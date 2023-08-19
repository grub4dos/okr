/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#include "okr.h"
// find the file storage address ,and copy it out .
//所有分区的休眠文件扇区列表在一开始一次性取出，以后直接使用就可以了
BYTE		g_UnusedListInit = FALSE;
BYTE		g_PartitionIndex;
WORD		g_FatLoadedCount;
ULONGLONG	g_FatLoadedSector;
DWORD		g_dwMaxUnusedListCount;
DWORD		g_dwUnusedListCount;
DWORD		g_dwCurrentUnusedListCount;
SIZE_L		*g_UnusedList;
SIZE_L		*g_CurrentUnusedList;
BYTE 		*g_FatBuffer = NULL;
FILE_RECORD_HEADER *MFT;

//假设系统分区最多有8个，那就有8个描述无用文件的描述符
UNUSED_FILE_DESC g_UnusedFile[MAX_UNUSED_FILES];

////////////////////////////////////////////////////////////////////////////////////
//ntfs
BOOL ReadLCN(ULONGLONG lcn, WORD count, PVOID buffer)
{
	lcn = MultU64x32(lcn, g_Partitions[g_PartitionIndex].SectorsPerClr);
	lcn += g_Partitions[g_PartitionIndex].BootBegin;
	count *= g_Partitions[g_PartitionIndex].SectorsPerClr;
	
	return DiskRead(g_Partitions[g_PartitionIndex].DiskNumber, lcn, count, buffer);
}

BOOL ReadExternalAttribute(NONRESIDENT_ATTRIBUTE *Attr, DWORD Vcn, WORD Number, PVOID Buffer)
{
	ULONGLONG lcn;
	ULONG runcount;
	DWORD count;
	BYTE *buf = (BYTE *)Buffer;
	BOOL bRet = FALSE;
	
	while(Number)
	{
		FindRun(Attr, Vcn, &lcn, &runcount);
		if(runcount > Number)
			count = Number;
		else
			count = (WORD)runcount;
		bRet = ReadLCN(lcn, count, buf);
		if(!bRet)
			break;
		Vcn += count;
		Number -= count;
		buf += count * g_Partitions[g_PartitionIndex].SectorsPerClr * SECTOR_SIZE;
	}
	
	return bRet;
}

BOOL ReadAttribute(PATTRIBUTE attr, DWORD offset, DWORD size, PVOID buffer)
{
	if (attr->Nonresident == FALSE) 
	{
		PRESIDENT_ATTRIBUTE rattr = (PRESIDENT_ATTRIBUTE)attr;
		CopyMem(buffer, (BYTE *)rattr + rattr->ValueOffset, rattr->ValueLength);
	}
	else 
	{
		PNONRESIDENT_ATTRIBUTE nrAttr= (PNONRESIDENT_ATTRIBUTE)attr;
		DWORD vcn = offset / (g_Partitions[g_PartitionIndex].SectorsPerClr * SECTOR_SIZE);
		DWORD count = size / (g_Partitions[g_PartitionIndex].SectorsPerClr * SECTOR_SIZE);
		if(count == 0)
			count = 1;
		ReadExternalAttribute((PNONRESIDENT_ATTRIBUTE)attr, vcn, (WORD)count, buffer);
	}
	return TRUE;
}

//;=======================================
//;Des : 读取一个文件的虚拟簇
//;Entry : pFileRecord - 文件记录(inode)
//;	attrtype - 属性类型
//;	vcn - 开始虚拟簇号
//;	count - 所要读取的个数
//;	buffer - 缓冲区
//;=======================================
BOOL ReadVCN(PFILE_RECORD_HEADER FileRecord, ATTRIBUTE_TYPE AttrType, 
						 ULONG Vcn, ULONG Count, PVOID Buffer)
{
	PNONRESIDENT_ATTRIBUTE attr;
	PBYTE buf = NULL;
	BOOL bRet = FALSE;
	
	attr = (PNONRESIDENT_ATTRIBUTE)FindAttribute(FileRecord, AttrType);
	if (!attr || (Vcn < attr->LowVcn || Vcn > attr->HignVcn)) 
	{
		PATTRIBUTE attrlist = FindAttribute(FileRecord, AttributeAttributeList);
		buf = myalloc(g_Partitions[g_PartitionIndex].SectorsPerFileRecord * SECTOR_SIZE);
		attr = (PNONRESIDENT_ATTRIBUTE)FindAttributeFromAttributeList(attrlist, TRUE, AttrType, Vcn, buf);
	}
	
	if(attr){
		bRet = ReadExternalAttribute(attr, Vcn, (WORD)Count, Buffer);
	}

	if(buf)
		myfree(buf);
		
	return bRet;
}

//;===============================================
//;Des:	从MFT中读取一个文件记录
//;Entry: index - 文件节点号
//;	buf - 文件记录存放地址偏移，段内！！
//;Retrun: 0 - failed, 1 - successed
//;===============================================
BOOL GetNtfsFileRecord(DWORD Inode, void * RecordBuffer)
{
	DWORD vcn;
	BYTE *tempbuffer;
	WORD buffersize, count, offset, secperfile, secperclr;
	BOOL bRet = FALSE;
	
	secperclr = g_Partitions[g_PartitionIndex].SectorsPerClr;
	secperfile = g_Partitions[g_PartitionIndex].SectorsPerFileRecord;
	if(g_Partitions[g_PartitionIndex].SectorsPerClr <= g_Partitions[g_PartitionIndex].SectorsPerFileRecord)
	{
		buffersize = secperfile * SECTOR_SIZE;
		count = secperfile / secperclr;
		offset = 0;
		
	}
	else
	{
		buffersize = g_Partitions[g_PartitionIndex].SectorsPerClr * SECTOR_SIZE;
		count = secperclr / secperfile;
		offset = (WORD)(Inode & (count - 1)) * secperfile * SECTOR_SIZE;
		count = 1;

	}
	tempbuffer = myalloc(buffersize);
	if (!tempbuffer)
		return FALSE;
	
	//;读一个文件记录
	vcn = (DWORD)muldiv(Inode, secperfile, secperclr);
	if(!ReadVCN((PFILE_RECORD_HEADER)MFT, AttributeData, vcn, count, tempbuffer))
	{
		MyLog(LOG_ERROR, L"ReadVCN failed\r\n");
		goto failed;
	}
	
	//;取得节点在MFT中的簇的偏移, 即读出的缓冲区的开始处,然后将之拷贝到目标缓冲区中
	CopyMem(RecordBuffer, tempbuffer+offset, secperfile * SECTOR_SIZE);
	FixupInode(RecordBuffer, secperfile);
	
	bRet = TRUE;
	
failed:
	
	if(tempbuffer)
		myfree(tempbuffer);
	
	return bRet;
}

//;==========================================================
//;Des: 从属性列表中查找一个属性的偏移
//;Entry: attrlist - 属性列表指针
//;       nostructhead - 是否需要跳过结构头
//;	attrtype - 属性类型
//;	vcn	- 虚拟簇号， if -1 表示不使用
//;	buf - 属性列表使用的缓冲区，存放所要查找的属性
//;Return:  eax - 所要查找属性的偏移, 0 if fail, ebx - 当前已找到的偏移
//;==========================================================
ATTRIBUTE *FindAttributeFromAttributeListEx(PATTRIBUTE AttrList, BOOL bAttrHeader, 
	ATTRIBUTE_TYPE AttrType, ULONG Vcn, PVOID Buffer, PWORD AttrOffset)
{
	DWORD offset, attrlistlen;
	PATTRIBUTE_LIST attrlist;
	PATTRIBUTE attr = NULL;
	PNONRESIDENT_ATTRIBUTE nrAttr;

	attrlistlen = 1024;
	attrlist = (PATTRIBUTE_LIST)AttrList;
	if(bAttrHeader)
	{
		if(AttrList->Nonresident == 1)
		{
			//;我们暂时处理不了属性列表也是非常驻属性的情况
			// - 属性列表也是非常驻属性的情况下应该放在该函数外面进行载入
			return NULL;
		}
		attrlistlen = AttrList->Length;
		offset = ((RESIDENT_ATTRIBUTE *)AttrList)->ValueOffset;
		attrlist = (PATTRIBUTE_LIST)((PBYTE)AttrList + offset);
	}	
	
	offset = 0;
	while( attrlist->AttributeType != AttrType )
	{
		if((attrlist->AttributeType == 0) 
			|| (attrlist->AttributeType == 0xffffffff)
			|| (attrlist->Length == 0))
			return NULL;
		attrlist = (PATTRIBUTE_LIST)((PBYTE)attrlist + attrlist->Length);
		offset += attrlist->Length;
		if(offset >= attrlistlen)
			return NULL;
	}
	
	while( attrlist->AttributeType == AttrType )
	{
		GetNtfsFileRecord((DWORD)attrlist->FileReferenceNumber, (PBYTE)Buffer);
		
		attr = FindAttribute((PFILE_RECORD_HEADER)Buffer, AttrType);
		nrAttr = (PNONRESIDENT_ATTRIBUTE)attr;
		if(!attr || Vcn < nrAttr->LowVcn || Vcn > nrAttr->HignVcn)
		{
			attrlist = (PATTRIBUTE_LIST)((PBYTE)attrlist + attrlist->Length);
		}
		else
			break;
	}
	
	if(attrlist->AttributeType != AttrType)
		return NULL;
		
	if(AttrOffset)
		*AttrOffset = (WORD)((BYTE*)attrlist - (BYTE*)AttrList);

	return attr;	
}

ATTRIBUTE *FindAttributeFromAttributeList(PATTRIBUTE AttrList, BOOL bAttrHeader, 
	ATTRIBUTE_TYPE AttrType, ULONG Vcn, PVOID Buffer)
{
	return FindAttributeFromAttributeListEx(AttrList, bAttrHeader, AttrType, Vcn, Buffer, NULL);
}

//;尝试从索引根里面查找文件,返回节点号
DWORD FindIndexRecordInRootAttribute(FILE_RECORD_HEADER * FileRecord, WCHAR *AttrName)
{
	PATTRIBUTE attr;
	BYTE *attrlistbuffer = NULL;
	DWORD inode, attrlen, offset;

	inode = 0;
	attr = FindAttribute(FileRecord, AttributeIndexRoot);
	if (!attr)	//   ; then try attributelist
	{
		attr = FindAttribute(FileRecord, AttributeAttributeList);
		if(attr)
		{
			WORD bytesPerFileRecord = g_Partitions[g_PartitionIndex].SectorsPerFileRecord * SECTOR_SIZE;
			attrlistbuffer = myalloc(bytesPerFileRecord);
			if(!attrlistbuffer)
				goto failed;
			attr = FindAttributeFromAttributeList(attr, TRUE, AttributeIndexRoot, 0, attrlistbuffer);
		}
	}
	if(attr)
	{
		DIRECTORY_ENTRY *dir;
		attrlen = attr->Length;
		offset = attr->NameOffset + attr->NameLength * 2 + sizeof(INDEX_ROOT);
		while(offset < attrlen)
		{
			if (offset + sizeof(DIRECTORY_ENTRY) >= attrlen)
				goto failed;
			
			dir = (DIRECTORY_ENTRY *)((BYTE *)attr + offset);
			if (dir->FileReferenceNumber)
			{
				if (dir->Name.NameLength == StrLength(AttrName))
				{
					if(mywcsnicmp(AttrName, dir->Name.Name, dir->Name.NameLength) == 0)
					{
						inode = (DWORD)dir->FileReferenceNumber;
						break;
					}
				}
			}
			
			//;检测下一个目录项
			offset += dir->Length;
		}
	}
	
failed:	
	if(attrlistbuffer)
		myfree(attrlistbuffer);
		
	return inode;
}

//;从目录缓冲区中查找一个文件，目录可能没有读完
DWORD FindFileEntryInBuffer(BYTE *Directory, WORD Length, WORD *FileName)
{
	WORD offset;
	DWORD inode = 0;
	DIRECTORY_ENTRY *dir;
	
	offset = 0;
	//如果剩余空间不足以容纳下一个目录项，则返回
	while (offset + sizeof(DIRECTORY_ENTRY) < Length)
	{
		dir = (DIRECTORY_ENTRY *)(Directory + offset);
		if(dir->Length && dir->FileReferenceNumber)
		{
			if (dir->Length >= sizeof(DIRECTORY_ENTRY))
			{
				if (dir->Name.NameLength == StrLength(FileName))
				{
					if(mywcsnicmp(FileName, dir->Name.Name, dir->Name.NameLength) == 0)
					{
						inode = (DWORD)dir->FileReferenceNumber;
						break;
					}
				}
			}
			//;检测下一个目录项
			if((DWORD)offset + dir->Length > (DWORD)Length)
				break;
			offset += dir->Length;
		}
		else
		{
			//;如果不是有效的目录项，则当前运行下的目录内容检查完毕
			break;
		}
	}
	
	return inode;
}

//;========================================
//;Des: 从根目录中查找匹配的文件
//;Entry: p_filename - 文件名指针（远指针）
//;Return: index - 找到的文件节点号, -1 if error.
//;========================================
DWORD FindInodeFromNameInRoot(char *FileName, FILE_RECORD_HEADER *InodeBuffer)
{
	PATTRIBUTE attr;
	BYTE *attrbuffer = NULL;
	BYTE *attrlistbuffer = NULL;
	INDEX_BLOCK_HEADER *index;
	DWORD inode, offset, attrlen;
	BYTE *dirent;
	WORD dirlen, bufsize, bytesPerIndxBlock;
	WORD wszFileName[128] = { 0 };
	
	if(!GetNtfsFileRecord(INODE_ROOT_DIRECTORY, InodeBuffer)){

		return 0;
		}
	//;将文件名转换成宽字符名
	Ascii2Unicode(wszFileName, FileName);

	//;NTFS目录是使用B+树组织的，所以首先需要查找其根节点上的文件是否是所要查找的；然后再查找子节点
	inode = FindIndexRecordInRootAttribute((FILE_RECORD_HEADER *)InodeBuffer, wszFileName);
	if(inode)
		return inode;

	attr = FindAttribute((FILE_RECORD_HEADER *)InodeBuffer, AttributeIndexAllocation);
	if (!attr) 	//we encounter attribute list
	{
		PATTRIBUTE attrlist;
		//;找属性列表
		attrlist = FindAttribute((FILE_RECORD_HEADER *)InodeBuffer, AttributeAttributeList);
		if(!attrlist)
			goto failed;

		//;从属性列表中取出索引分配信息
		bufsize = g_Partitions[g_PartitionIndex].SectorsPerFileRecord * SECTOR_SIZE;
		attrlistbuffer = myalloc(bufsize);
		if(!attrlistbuffer)
			goto failed;
		attr = FindAttributeFromAttributeList(attrlist, TRUE, AttributeIndexAllocation, 0, attrlistbuffer);
		if (!attr)
			goto failed;
	}

	//;读出根目录，查找指定文件
	bytesPerIndxBlock = g_Partitions[g_PartitionIndex].SectorsPerIndxBlock * SECTOR_SIZE;
	attrbuffer = myalloc(bytesPerIndxBlock);
	if(attr->Nonresident)
		attrlen = (DWORD)((NONRESIDENT_ATTRIBUTE *)attr)->AllocatedSize;
	else
		attrlen = ((RESIDENT_ATTRIBUTE *)attr)->ValueLength;
	offset = 0;
	while (offset < attrlen)
	{
		//;一个目录可能分几个INDX来存放，其内容需要分几次读完
		if(!ReadAttribute(attr, offset, bytesPerIndxBlock, attrbuffer))
		{
			//MyLog(LOG_ERROR, L"ReadAttribute failed, offset %x\r\n", offset);
			break;
		}
		if(!FixupInode(attrbuffer, g_Partitions[g_PartitionIndex].SectorsPerIndxBlock))
			break;
		
		//;找到目录项的偏移地址
		index = (INDEX_BLOCK_HEADER *)attrbuffer;
		dirent = (BYTE *)&index->DirectoryIndex + index->DirectoryIndex.EntriesOffset;	//指向目录项结构
		dirlen = (WORD)(bytesPerIndxBlock - (dirent - attrbuffer));
		inode = FindFileEntryInBuffer(dirent, dirlen, wszFileName);
		if(inode)
			break;
		offset += bytesPerIndxBlock;
	}
	
failed:	
	if(attrbuffer)
		myfree(attrbuffer);
	if(attrlistbuffer)
		myfree(attrlistbuffer);
	return inode;
}

//;载入MFT
//;return FALSE if failed, non-0 else.
BOOL LoadMFT(WORD PartitionIndex)
{
	DWORD retcode;
	WORD mftsize;
	ULONGLONG sector;

	sector = g_Partitions[PartitionIndex].BootBegin + g_Partitions[PartitionIndex].MftBeginSector;
	mftsize = g_Partitions[PartitionIndex].SectorsPerFileRecord * SECTOR_SIZE;
	MFT = (FILE_RECORD_HEADER *)myalloc(mftsize);
	if(!MFT)
	{
		MyLog(LOG_ERROR, L"failed to alloc mft(%x)\r\n", mftsize);
		return FALSE;
	}
		
	if(!DiskRead(g_Partitions[PartitionIndex].DiskNumber, sector, 
		g_Partitions[PartitionIndex].SectorsPerFileRecord, MFT))
	{
		MyLog(LOG_ERROR, L"failed to read mft %lx\r\n", sector);
		return FALSE;
	}
		
	if(!FixupInode(MFT, g_Partitions[PartitionIndex].SectorsPerFileRecord))
	{
		MyLog(LOG_ERROR, L"failed to fixup mft %x %x.\r\n", *(DWORD*)MFT, *(DWORD*)((PBYTE)MFT + 508));
		return FALSE;
	}
	
	return TRUE;
}

DWORD GetInodeVcnList(PFILE_RECORD_HEADER InodeBuffer, SIZE_L *pList, DWORD MaxListNumber)
{
	PATTRIBUTE attr;
	DWORD listcount;
	ULONGLONG lcn;
	ULONG vcn, count;
	
	//;从里面查找DATA属性，如果没有找到，说明存在属性列表，需要在属性列表里查找
	listcount = 0;
	attr = FindAttribute(InodeBuffer, AttributeData);	// /*AttributeData*/
	if(attr)
	{
		vcn = 0;
		while(listcount < MaxListNumber)
		{
			if(FindRun((PNONRESIDENT_ATTRIBUTE)attr, vcn, &lcn, &count))
			{
				pList->Begin = lcn;
				pList->Number = count;
				pList++;
				vcn += count;
				listcount++;
			}
			else{

				break;
			}
		}		
	}
	else
	{
		//;查找属性列表
		ATTRIBUTE * attrlist;
		BYTE *buf;
		BOOL bAttrHeader;
		DWORD attrlistlen;
		WORD offset, attroffset;
		
		attrlist = FindAttribute(InodeBuffer, AttributeAttributeList);
		if(attrlist == NULL)
			return 0;
		if(attrlist->Nonresident == 1)
		{
			//属性列表是非常住属性，需要从磁盘中读取出来
		}

		count = g_Partitions[g_PartitionIndex].SectorsPerFileRecord * SECTOR_SIZE;
		buf = myalloc(count);
		bAttrHeader = TRUE;
		vcn = 0;
		attrlistlen = attrlist->Length;
		offset = 0;
		while(offset < attrlistlen)
		{
			//;find AttributData in attribute list.
			attr = FindAttributeFromAttributeListEx(attrlist, bAttrHeader, AttributeData, vcn, buf, &attroffset);
			if(attr == NULL)
				break;
			while(listcount < MaxListNumber)
			{
				if(FindRun((PNONRESIDENT_ATTRIBUTE)attr, vcn, &lcn, &count))
				{
					pList->Begin = lcn;
					pList->Number = count;
					pList++;
					vcn += count;
					listcount++;
				}
				else{

					break;
				}
			}
			if(listcount >= MaxListNumber){

				break;
			}
			if(offset < attrlistlen)
			{
				ATTRIBUTE_LIST * pAttrList = (ATTRIBUTE_LIST *)((BYTE *)attrlist + attroffset);	//指向当前已经处理过的attrlist
				
				offset += attroffset + pAttrList->Length;
				attrlist = (ATTRIBUTE *)((PBYTE)pAttrList + pAttrList->Length);	//指向下一个要处理的attrlist
				bAttrHeader = FALSE;
			}
			else{

				break;
			}
		}
		
		if(buf)
			myfree(buf);
	}
	return listcount;
}

DWORD GetInodeSectorList(PFILE_RECORD_HEADER InodeBuffer, SIZE_L *pList, DWORD MaxListNumber)
{
	DWORD listcount, i;
	
	listcount = GetInodeVcnList(InodeBuffer, pList, MaxListNumber);
	
	//将簇号转换为sector
	for(i=0;i<listcount;i++)
	{
		pList[i].Begin = MultU64x32(pList[i].Begin, g_Partitions[g_PartitionIndex].SectorsPerClr);
		//pList[i].Begin += g_Partitions[g_PartitionIndex].BootBegin;
		//pList[i].Begin += g_Partitions[g_PartitionIndex].ProtectionBegin;
		pList[i].Number *= g_Partitions[g_PartitionIndex].SectorsPerClr;
	}
	
	return listcount;
}

//;entry: drive - which partition
//;		pfilename - file name begin with '\', such as "\win386.swp", 
  //;		plist - sectors list buffer
  //;		listsize - size of list(unit:SIZE_D)
//;return ax = seclist numbers
DWORD DumpFileSectorListNtfs(WORD PartitionIndex, char *FileName, SIZE_L *pList, DWORD MaxListNumber)
{
	DWORD inode = 0;
	DWORD listcount = 0, bytesPerFileRecord;
	PFILE_RECORD_HEADER inodebuffer = NULL;
	
	if(!LoadMFT(PartitionIndex)){
		goto failed;
	}

	bytesPerFileRecord = g_Partitions[PartitionIndex].SectorsPerFileRecord * SECTOR_SIZE;		
	inodebuffer = (PFILE_RECORD_HEADER)myalloc(bytesPerFileRecord);
	if(!inodebuffer){

		goto failed;
	}

	inode = FindInodeFromNameInRoot(FileName, inodebuffer);
	if(inode == 0)
		goto failed;
//MyLog(LOG_DEBUG, L"inode 0x%x.\r\n", inode);

	GetNtfsFileRecord(inode, inodebuffer);

	listcount = GetInodeSectorList(inodebuffer, pList, MaxListNumber);
//	MyLog(LOG_DEBUG, L" zwang GetInodeSectorList fail\n");
	
//disp_reg_32(22, 0, listcount, COLOR_BLUE, COLOR_WHITE);
//disp_reg_32(32, 0, MaxListNumber, COLOR_BLUE, COLOR_WHITE);
	
failed:
	
	if(inodebuffer)
		myfree(inodebuffer);
	if(MFT)
		myfree(MFT);
	MFT = NULL;

	return listcount;
}

////////////////////////////////////////////////////////////////////////////////////

//revert 8.3 filename to the filename of directory item.
BOOL RevertFileName(char *FileName, char *DosName)
{
	int i=0;
	
	while(*FileName)
	{
		if(i > 11) return FALSE; //filename is wrong
		if(*FileName != '.')
		{
			if((*FileName >= 'a')&&(*FileName <= 'z'))
				*DosName++ = (*FileName++ - 0x20);
			else *DosName++ = *FileName++;
			i++;
		}
		else
		{
			if(i == 0)
			{
				FileName++;
				if(*FileName == '.')  //parent dir.
				{
					*DosName++ = '.';
					*DosName++ = *FileName++;
					for(i=2;i<11;i++) *DosName++ = 0x20;
				}
				else if(*FileName == 0)
				{
					*DosName++ = '.';
					for(i=1;i<11;i++) *DosName++ = 0x20;
				}
				else
				{
					for(i=0;i<8;i++) *DosName++ = 0x20;
				}
			}
			else
			{
				FileName++;
				if(i < 8)
				{
					for(;i<8;i++) *DosName++ = 0x20;
				}
				else if(i > 8)
				{
					*DosName++ = '.';
					i++;
				}
			}
		}
	}
	
	while(i<11)
	{
		*DosName++ = 0x20;
		i++;
	}
	return TRUE;
}

//按指定的文件名在目录缓冲中查找相应的目录项，并返回该目录项指针。
//ENTRY: filename - 文件名
//               buff - 目录缓冲区
//               buffsize - 缓冲区大小
//RETURN:
//               0 - not found the matched file.
//               non 0 - 指向对应的目录项。
BYTE* GetDirectoryItem(char *FileName, BYTE * Buffer, WORD BufferSize)
{
	char name[12];
	
	RevertFileName((char*)FileName, name); //revert pathname to 8.3 format str.
	name[11]=0;
	while(BufferSize && *Buffer)  //scan buffer to end.
	{
		if(strnicmp(name, Buffer, 11)==0)
		{
			return Buffer;
		}
		Buffer += sizeof(FAT_DIRECTORY_ENTRY);
		BufferSize -= sizeof(FAT_DIRECTORY_ENTRY);
	}
	return NULL;
}

//;读取FAT表的一个扇区，返回ax=读出来的FAT表在TEMPBUFFER里的偏移地址
BYTE *GetFatTable(ULONGLONG FatSector)
{
	DWORD offset;
	offset = (DWORD)(FatSector - g_FatLoadedSector);
	if(offset >= (DWORD)g_FatLoadedCount)
	{
		//; 一次读128个扇区以缓冲磁盘IO
		g_FatLoadedSector = FatSector;
		offset = (DWORD)(FatSector - g_FatLoadedSector);
		
		FatSector = g_FatLoadedSector + g_Partitions[g_PartitionIndex].BootBegin;
		DiskRead(g_Partitions[g_PartitionIndex].DiskNumber, FatSector, 128, g_FatBuffer);
		//ReadWriteDiskSector(g_Partitions[g_PartitionIndex].DiskNumber, COMMAND_READ, 
		//	FatSector, MAX_SECTORS_IN_SEGMENT, g_FatBuffer);
		g_FatLoadedCount = 128;
	}
	
	return g_FatBuffer + offset * SECTOR_SIZE;
}

///////////////
//FUNCTION: 得到下一个簇
//ENTRY：	drive  -  0='c',1='d'.....
//			PrevClusterNumber - 当前簇
///
DWORD GetNextClusterNumber(DWORD Cluster)
{
	ULONGLONG fatsector;
	DWORD offset;
	
	if(g_Partitions[g_PartitionIndex].PartitionType == FAT16)
	{
		WORD *fatbuffer;
		fatsector = Cluster/256 + g_Partitions[g_PartitionIndex].FirstFatSector;
		fatbuffer = (WORD *)GetFatTable(fatsector);
		
		offset = (Cluster & 255);
		return (DWORD)(fatbuffer[offset]);
	}
	else
	{
		DWORD *fatbuffer;
		fatsector = Cluster/128 + g_Partitions[g_PartitionIndex].FirstFatSector;
		fatbuffer = (DWORD *)GetFatTable(fatsector);

		offset = (Cluster & 127);
		return (DWORD)(fatbuffer[offset]);
	}
}

//;get sector list from filename in FAT32
//;return eax = seclistsize if success,
//;             0 else.
DWORD DumpFileSectorListFAT(WORD PartitionIndex, char *FileName, SIZE_L *pList, DWORD MaxListNumber)
{
	FAT_DIRECTORY_ENTRY * dir;
	DWORD clr, nextclr, secperclr, count, total;
	ULONGLONG sector, databegin;
	WORD listcount = 0, i;
	PBYTE buf = NULL, fatbuffer = NULL;

	fatbuffer = myalloc(0x10000);
	if(!fatbuffer)
		return 0;
	//InitPartitionFSInfo(PartitionIndex, fatbuffer);
	g_FatBuffer = fatbuffer;
	g_FatLoadedSector = 0;
	g_FatLoadedCount = 0;
	
	dir = NULL;
	buf = myalloc(0x8000);
	if(!buf)
		goto failed;
		
	secperclr = g_Partitions[PartitionIndex].SectorsPerClr;
	if(g_Partitions[PartitionIndex].PartitionType == FAT16)
	{
		count = g_Partitions[PartitionIndex].FirstFatSector + 
					g_Partitions[PartitionIndex].SectorsPerFAT * g_Partitions[PartitionIndex].FatCopies;
		sector = g_Partitions[PartitionIndex].BootBegin + count;
		total = g_Partitions[PartitionIndex].RootSectors;
		while(total)
		{
			if(total > 0x8000/SECTOR_SIZE)
				count = 0x8000/SECTOR_SIZE;
			else
				count = total;
				
			DiskRead(g_Partitions[g_PartitionIndex].DiskNumber, sector, (WORD)count, buf);
			dir = (FAT_DIRECTORY_ENTRY *)GetDirectoryItem(FileName, buf, (WORD)count*SECTOR_SIZE);
			if(dir)
				break;
			total -= count;
			sector += count;
		}
		if(!dir)
			goto failed;
		clr = dir->firstclr;
	}
	else
	{
		count = g_Partitions[PartitionIndex].FirstFatSector + 
					g_Partitions[PartitionIndex].SectorsPerFAT * g_Partitions[PartitionIndex].FatCopies;
		databegin = g_Partitions[PartitionIndex].BootBegin + count;
		
		clr = g_Partitions[PartitionIndex].RootDirClr;
//Print(L"RootDirClr 0x%x, secperclr 0x%x\r\n", clr, secperclr);
		while(clr && clr != FAT32_EOF)
		{
			total = secperclr;
			sector = databegin + (clr-2)*secperclr;
			while(total)
			{
				if(total > 0x8000/SECTOR_SIZE)
					count = 0x8000/SECTOR_SIZE;
				else
					count = total;
					
				DiskRead(g_Partitions[g_PartitionIndex].DiskNumber, sector, (WORD)count, buf);
				//ReadWriteDiskSector(g_Partitions[PartitionIndex].DiskNumber, COMMAND_READ, 
				//	sector, (WORD)count, buf);
				dir = (FAT_DIRECTORY_ENTRY *)GetDirectoryItem(FileName, buf, (WORD)count*SECTOR_SIZE);
				if(dir)
					break;
				total -= count;
				sector += count;
			}
			if(dir)
				break;
			clr = GetNextClusterNumber(clr);
		}
		if(!dir)
			goto failed;
		clr = (DWORD)((DWORD)dir->firstclr_hi << 16) + dir->firstclr;
	}

	//初始化分区参数
	//;找出该文件的目录项,并写到目录缓冲区directoryItem里面
	listcount = 0;
	pList[listcount].Begin = clr;
	count = 1;
	while(listcount < MaxListNumber)
	{
		nextclr = GetNextClusterNumber(clr);
		if(g_Partitions[PartitionIndex].PartitionType == FAT16)
		{
			if(clr >= 0xfff0 || !nextclr)
				break;
		}
		else
		{
			if(clr >= 0x0ffffff0 || !nextclr)
				break;
		}
		if(nextclr == clr + 1)
		{
			count++;
		}
		else
		{
			pList[listcount].Number = count;
			listcount++;
			pList[listcount].Begin = clr;
			count = 1;
		}
	}
	pList[listcount].Number = count;
	listcount++;

	//listcount = GetClrListFromFirstClr(clr, pList, MaxListNumber);
	//将簇号转换为protection sector
	for(i=0;i<listcount;i++)
	{
		pList[i].Begin = MultU64x32(pList[i].Begin, g_Partitions[PartitionIndex].SectorsPerClr);
		//pList[i].Begin += g_Partitions[PartitionIndex].ProtectionBegin;	//g_Partition[PartitionIndex].BootBegin;
		pList[i].Number *= g_Partitions[PartitionIndex].SectorsPerClr;
	}

failed:
	if(fatbuffer)
		myfree(fatbuffer);
	if(buf)
		myfree(buf);
	return listcount;
}

////////////////////////////////////////////////////////////////////////////////////
BOOL RecordList(BYTE PartitionIndex, SIZE_L *pList, DWORD ListNumber)
{
	SIZE_L *newList;
	DWORD	newListCount, i, totalsize;
	
	//如果还没有分配内存，或者已经分配的不够容纳新增的扇区列表，则重新分配
	if(!g_UnusedList || ListNumber + g_dwUnusedListCount > g_dwMaxUnusedListCount)
	{
		if(!g_UnusedList)
		{
			g_dwMaxUnusedListCount = 0;
			g_dwUnusedListCount = 0;
		}
		newListCount = g_dwMaxUnusedListCount + ((ListNumber + 0x1fff) & ~0x1fff);
		newList = myalloc(newListCount * sizeof(SIZE_L));
		if(g_dwUnusedListCount)
		{
			//拷贝原来的数据到新分配的内存中，并释放原来的扇区列表
			CopyMem(newList, g_UnusedList, g_dwUnusedListCount*sizeof(SIZE_L));
			myfree(g_UnusedList);
		}
		g_UnusedList = newList;
		g_dwMaxUnusedListCount = newListCount;
	}
	
	newList = g_UnusedList + g_dwUnusedListCount;
	CopyMem(g_UnusedList + g_dwUnusedListCount, pList, ListNumber * sizeof(SIZE_L));
	g_dwUnusedListCount += ListNumber;
	
	//计算大小
	totalsize = 0;
	for(i=0;i<ListNumber;i++)
	{
		totalsize += pList[i].Number;
	}
	
	//记录unused file desc
	for(i=0;i<MAX_UNUSED_FILES;i++)
	{
		if(g_UnusedFile[i].PartitionIndex == PartitionIndex || !g_UnusedFile[i].ListCount)
		{
			if(!g_UnusedFile[i].ListCount)
			{
				g_UnusedFile[i].PartitionIndex = PartitionIndex;
				g_UnusedFile[i].pList = newList;
			}
			g_UnusedFile[i].ListCount += ListNumber;
			g_UnusedFile[i].TotalSize += totalsize;
			break;
		}
	}
	
	return TRUE;
}

//取出备份分区的pagefile.sys和hiberfil.sys文件的扇区位置及总大小,返回扇区大小
DWORD GetUnusedDataList(BYTE PartitionIndex)
{
	DWORD listCount, listSize, i, filesize;
	SIZE_L *pList;
	char *filename[] = {"hiberfil.sys", "pagefile.sys", "swapfile.sys"};
	
	filesize = 0;
	g_dwCurrentUnusedListCount = 0;
	g_CurrentUnusedList = NULL;
	g_PartitionIndex = PartitionIndex;
	
	if(g_UnusedListInit)
		goto completed;
	
	//至少有两个需要特殊处理的文件：休眠文件和虚拟内存文件
	listSize = 0x10000;
	pList = (SIZE_L *)myalloc(listSize);
	if(!pList)
		goto failed;
	mymemset(pList, 0, listSize);

	//if(g_Partitions[PartitionIndex].PartitionType == NTFS)
	//{
	//	MyLog(LOG_DEBUG, L"%d:%d, SectorsPerFileRecord:%d, MftBeginSector(%lx), BootBegin %lx\r\n", 
	//				g_Partitions[PartitionIndex].DiskNumber, PartitionIndex, 
	//				g_Partitions[PartitionIndex].SectorsPerFileRecord, 
	//				g_Partitions[PartitionIndex].MftBeginSector,
	//				g_Partitions[PartitionIndex].BootBegin
	//				);
	//}
	
	for(i=0;i<sizeof(filename)/sizeof(char*);i++)
	{
		if(g_Partitions[PartitionIndex].PartitionType == NTFS){
			listCount = DumpFileSectorListNtfs(PartitionIndex, filename[i], pList, listSize/sizeof(SIZE_L));
			MyLog(LOG_ERROR,L"in getlist.c, line %d listCount=%x filename[i]=%a\n", __LINE__,listCount,filename[i]); 
		}
		else if(g_Partitions[PartitionIndex].PartitionType == FAT32 || g_Partitions[PartitionIndex].PartitionType == FAT16){
			listCount = DumpFileSectorListFAT(PartitionIndex, filename[i], pList, listSize/sizeof(SIZE_L));
		}
		else 
			listCount = 0;
			
		if(listCount)
		{
			RecordList(PartitionIndex, pList, listCount);
			mymemset(pList, 0, listSize);
		}
	}
	
#if OKR_DEBUG
	//if(g_dwUnusedListCount)
	//{
		MyLog(LOG_DEBUG, L"%d:%d, listcount:%d, first(%x,%x)\r\n", 
			g_Partitions[PartitionIndex].DiskNumber, PartitionIndex, 
			g_dwUnusedListCount, g_UnusedList->Begin, g_UnusedList->Number);
	//	getch();
	//}
#endif	
	myfree(pList);

failed:
	
completed:
	for(i=0;i<MAX_UNUSED_FILES;i++)
	{
		if(g_UnusedFile[i].PartitionIndex == PartitionIndex)
		{
			filesize = g_UnusedFile[i].TotalSize;
			
			g_dwCurrentUnusedListCount = g_UnusedFile[i].ListCount;
			g_CurrentUnusedList = g_UnusedFile[i].pList;
			break;
		}
	}
	
	return filesize;
}

DWORD GetAllUnusedDataSize()
{
	DWORD i, totalsize;
	
	if(!g_UnusedListInit)
	{
		g_dwUnusedListCount = 0;
		g_dwMaxUnusedListCount = 0;
		g_dwUnusedListCount = 0;
		g_UnusedList = NULL;
		mymemset(&g_UnusedFile, 0, sizeof(g_UnusedFile));
		
    	for (i=0;i<g_dwTotalPartitions;i++) {
    	    if (g_Partitions[i].bIfBackup!=TRUE)
    	        continue;
			GetUnusedDataList(i);
    	}
	}
	
	totalsize = 0;
	for(i=0;i<MAX_UNUSED_FILES;i++)
	{
		totalsize += g_UnusedFile[i].TotalSize;
	}
	
	g_UnusedListInit = TRUE;
	
	return totalsize;
}

void ReleaseUnusedDataList()
{
	if(g_UnusedListInit)
	{
		if(g_UnusedList)
			myfree(g_UnusedList);
		g_UnusedList = NULL;
		
		g_UnusedListInit = FALSE;
	}
}
