#include "okr.h"

#define Padd(p, n) ((PBYTE)p + n)

PEXT3_GROUP_DESC m_group_desc;

ULONGLONG	m_BootBegin;
DWORD		m_SectorsPerCluster;

BYTE		m_PartitionType;
BYTE		m_PartitionIndex;
BYTE		m_DiskNumber;
BYTE		m_bLBAMode;

SIZE_L		m_BitmapSectorList[0x10];
PBYTE		m_FatBuffer;
DWORD		m_dwFatBufferSize;

DWORD		m_FatSize;
DWORD		m_RecursiveTimes;
	
DWORD				g_dwTotalPartitions;			//所有硬盘的总分区数
WIN32_PARTITIONINFO g_Partitions[MAX_NUM_PARTITIONS];//所有硬盘的全部分区信息，如果超过64个，就不支持了

int IsUnSupportedPartition(WORD pid)
{
	switch(pid)
	{
	case NTFS:
	case FAT16:
	case FAT32:
	case FAT16_E:
	case FAT32_E:
	case PARTITION_LINUX:
		//支持的分区
		return FALSE;
		
	case PARTITION_LINUX_SWAP:
	case PARTITION_DOS_12:
	case PARTITION_XENIX_ROOT:
	case PARTITION_XENIX_USR:
	case PARTITION_FAT_16:
	case PARTITION_LDM:
	case PLAN9_PARTITION:
	case PARTITION_PREP:
	case DM6_AUX1PARTITION:
	case DM6_AUX3PARTITION:
	case DM6_PARTITION:
	case EZD_PARTITION:
	case PARTITION_UNIX:
	case PARTITION_NOVELL_NETWARE:
	case MINIX_PARTITION:
	case LINUX_EXTENDED_PARTITION:
	case PARTITION_NTFS_VOL_SET:
	case PARTITION_BSD_386:
	case PARTITION_OPENBSD:
	case NETBSD_PARTITION:
	case BSDI_PARTITION:
	case LINUX_RAID_PARTITION:
		//能够识别但不支持
		return 1;
		
	default:
		//不能识别
		return 2;
	}
}
////////////////////////////////////////////////////////////////////////////////
void InitPartition()
{
	m_group_desc = NULL;
	m_FatBuffer = NULL;
	m_dwFatBufferSize = 0;
	m_RecursiveTimes = 0;
	SetMem(m_BitmapSectorList, 0, sizeof(m_BitmapSectorList));
    g_dwTotalPartitions = 0;
	SetMem(g_Partitions, sizeof(g_Partitions), 0);
}

void CleanPartition()
{
	if (m_FatBuffer)
		FreePool(m_FatBuffer);
	m_FatBuffer = NULL;
}

ULONGLONG GetPartTotalClr(WIN32_PARTITIONINFO *Partition)
{
	ULONGLONG totalClrs = 0;
	ULONGLONG fatSector	= Partition->FirstFatSector + Partition->BootBegin;
	DWORD k = 0, total, i, count, remainSectors;
	DWORD	groupindex, blocksize;
	DWORD buffersize = 0, bitmapsize = 0;

	m_group_desc = NULL;
	total = Partition->SectorsPerFAT;
	buffersize = m_dwFatBufferSize;
	
	// if fat16, one fat sector have 512/2 clust
	// if fat32, one fat sector have 512/4 clust
	// if NTFS, one bitmap sector have 512*8 clust
	if (m_PartitionType == NTFS)
	{
		total = (DWORD)(DivU64x32(DivU64x32(Partition->TotalSectors, m_SectorsPerCluster),(512*8)) + 1);
		fatSector = m_BitmapSectorList[0].Begin + m_BootBegin;
		remainSectors = (DWORD)m_BitmapSectorList[0].Number;
		k++;
	}
	else if(m_PartitionType == PARTITION_LINUX)
	{
		groupindex = 0;
		blocksize = m_SectorsPerCluster * SECTOR_SIZE;
		buffersize = (((Partition->blocks_per_group + 7)/8 + blocksize - 1) & ~(blocksize - 1));
		bitmapsize = (buffersize + SECTOR_SIZE - 1)/SECTOR_SIZE;
		total = Partition->groups_count * bitmapsize;

		//MyLog(LOG_DEBUG, L"bitmapsize 0x%lx, total 0x%lx, groups_count %d\n",
		//	bitmapsize, total, Partition->groups_count);
	}
	
	if (!m_FatBuffer || buffersize > m_dwFatBufferSize)
	{
		if (m_FatBuffer)
			FreePool(m_FatBuffer);

		if(buffersize > 0x10000)
			m_dwFatBufferSize = ((buffersize + 0xfff) & ~0xfff);
		else
			m_dwFatBufferSize = 0x10000;
		m_FatBuffer = AllocatePool(m_dwFatBufferSize);
		if (!m_FatBuffer)
			return 0;
	}

	while (total > 0)
	{
		if (total > m_dwFatBufferSize/SECTOR_SIZE)
			count = m_dwFatBufferSize/SECTOR_SIZE;
		else
			count = total;

		if (m_PartitionType == NTFS)
		{
			if (count > remainSectors)
				count = remainSectors;
		}
		else if(m_PartitionType == PARTITION_LINUX)
		{
			count = bitmapsize;
			fatSector = GetExt3BitmapSector(Partition, groupindex);
			//MyLog(LOG_DEBUG, L"groupindex %d, fatsector 0x%llx, total 0x%x\n",
			//	groupindex, fatSector, total);
			if(fatSector == 0)
			{
				//没有初始化，无效的
				total -= count;
				groupindex++;
				continue;
			}
			else
				fatSector += m_BootBegin;
		}
	    DiskRead(m_DiskNumber, fatSector, count, m_FatBuffer);

		total -= count;
		fatSector += count;
		
		if(m_FatSize == 2) // fat16
			count *= 256;
		else if(m_FatSize == 4)
			count *= 128;
		else if(m_PartitionType == NTFS)
		{
			remainSectors -= count;
			if (remainSectors == 0)
			{
				fatSector = m_BitmapSectorList[k].Begin + m_BootBegin;
				remainSectors = (DWORD)m_BitmapSectorList[k].Number;
				k++;
				total = remainSectors;
			}
			count *= 512;
		}
		else if(m_PartitionType == PARTITION_LINUX)
		{
			if(total == 0)
				count = ((Partition->TotalSectors / m_SectorsPerCluster) % Partition->blocks_per_group);
			else
				count = Partition->blocks_per_group;
			count = (count + 7) / 8;
		}

		for ( i=0; i<count; i++)
		{
			if(m_FatBuffer[i*m_FatSize] != 0)
			{
				if(m_FatSize == 1)
					totalClrs += 8;
				else
					totalClrs++;
			}
			else
			{
				if(m_FatSize == 2) // fat16
				{
					if(*(WORD*)(m_FatBuffer + i*m_FatSize) != 0)
						totalClrs++;
				}
				else if(m_FatSize == 4)
				{
					if(*(DWORD*)(m_FatBuffer + i*m_FatSize) != 0)
						totalClrs++;
				}
			}
		}
		groupindex++;
	}

	m_group_desc = NULL;

	return totalClrs;
}

BOOL InitPartParamF16(WIN32_PARTITIONINFO *Partition)
{
	m_FatSize  = 2;
	m_DiskNumber = Partition->DiskNumber;
	m_BootBegin = Partition->BootBegin;
	return TRUE;
}

BOOL InitPartParamF32(WIN32_PARTITIONINFO *Partition)
{
	m_FatSize  = 4;
	m_DiskNumber = Partition->DiskNumber;
	m_BootBegin = Partition->BootBegin;
	return TRUE;
}

PATTRIBUTE  FindAttribute(PFILE_RECORD_HEADER FileRecord, ATTRIBUTE_TYPE AttrType)
{
	PATTRIBUTE attr = (PATTRIBUTE)Padd(FileRecord, FileRecord->AttributesOffset);
	while (attr->AttributeType != -1)
	{
		if (attr->AttributeType == AttrType)
			return attr;

		attr = (PATTRIBUTE)Padd(attr, attr->Length);
	}
	return NULL;
}

BOOL  FixupInode(PVOID Inode, DWORD InodeSize)
{
	PFILE_RECORD_HEADER file = (PFILE_RECORD_HEADER)Inode;
	PUSHORT usa = (PUSHORT)Padd(file, file->Ntfs.UsaOffset);
    DWORD i;
    PUSHORT sector;
//	InodeSize /= 512;
//	MyLog(LOG_DEBUG,L" USACOUNT %d,INODESIZE %d",file->Ntfs.UsaCount,InodeSize);
	if ((file->Ntfs.UsaCount > (InodeSize + 1)) || (file->Ntfs.UsaCount == 0))
		return FALSE;

	sector = (PUSHORT)file;
	for ( i=1; i<file->Ntfs.UsaCount;i++)
	{
		sector[255] = usa[i];
		sector += 256;
	}
	return TRUE;	
}

ULONG  RunLength(PBYTE run)
{
	return (*run & 0xf) + ((*run >>4) & 0xf) + 1;
}

ULONGLONG  RunLCN(PBYTE run)
{
	BYTE n1 = *run & 0xf;
	BYTE n2 = (*run >>4) & 0xf;
	ULONGLONG lcn = (n2 == 0) ? 0 : (char)(run[n1 + n2]);
    DWORD i;
	for (i = n1+n2-1; i>n1; i--)
		lcn = (lcn<< 8) + run[i];

	return lcn;
}

ULONG  RunCount(PBYTE run)
{
	BYTE n = *run & 0xf;
	ULONGLONG count = 0;
    DWORD i;
	for ( i=n; i>0; i--)
		count = (count << 8) + run[i];

	return (ULONG)count;
}

BOOL  FindRun (PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, PULONGLONG lcn, PULONG count)
{
	ULONGLONG base;
	PBYTE run;
	
	if (vcn < attr->LowVcn || vcn >attr->HignVcn)
		return FALSE;

	*lcn = 0;
	 base = attr->LowVcn;
	 run = (PBYTE)Padd(attr, attr->RunArrayOffset);
	while (*run)
	{
		*lcn += RunLCN(run);
		*count = RunCount(run);

		if(vcn >= base && vcn < base + *count)
		{
			*lcn = RunLCN(run)==0 ? 0 : *lcn + vcn - base;
			*count -= (ULONG)(vcn - base);
			return TRUE;
		}
		else
		{
			base += *count;
		}
		run += RunLength(run);
	}
	return FALSE;
}

ULONGLONG GetBitmapFileRecordSector(WIN32_PARTITIONINFO *Partition, PVOID Buffer)
{
	DWORD recordsize = Partition->SectorsPerFileRecord;
	DWORD secperclr = Partition->SectorsPerClr;
	ULONGLONG bitmapsector = 0;
	PBYTE attr ;
	DWORD count, offset, i;
	ULONGLONG vcn = 0, lcn;

	//读取MFT表
	DiskRead(m_DiskNumber, Partition->BootBegin + Partition->MftBeginSector, recordsize, Buffer);
	if (!FixupInode(Buffer, recordsize * SECTOR_SIZE))
	{
		return 0;
	}
	//找到data属性
	attr = (PBYTE)FindAttribute((PFILE_RECORD_HEADER)Buffer, AttributeData);
	if (!attr)
		return 0;

	offset = INODE_BITMAP * recordsize;
	for (i=0; i<3; i++)	//inode bitmap是第6个文件，前3个簇必然包含了此文件
	{
		if (FindRun((PNONRESIDENT_ATTRIBUTE)attr, vcn, &lcn, &count))
		{
			if(offset < count * secperclr)
			{
				bitmapsector = lcn * secperclr + offset;
				break;
			}
			offset -= count * secperclr;
			vcn += count;
		}
		else
			return 0;
	}

	return bitmapsector;
}

BOOL InitNTFS(WIN32_PARTITIONINFO *Partition)
{
	DWORD recordsize = Partition->SectorsPerFileRecord;
	DWORD secperclr = Partition->SectorsPerClr;
	BOOL bRet = FALSE;
	DWORD count;
	ULONGLONG vcn = 0, lcn;
	DWORD i;
	PBYTE attr ;
	ULONGLONG bitmapsector;
	PBYTE buf;
	
	m_DiskNumber = Partition->DiskNumber;
	m_FatSize  = 1;
	m_BootBegin = Partition->BootBegin;
	SetMem(m_BitmapSectorList, sizeof(m_BitmapSectorList),0);

	if((secperclr & (secperclr - 1)) != 0)
		return FALSE;

	if(recordsize > 32)
		return FALSE;
	
	//bitmapfile position in mftfile is 6, 0 - based.
	if(g_data.scanbuffer)
		buf = g_data.scanbuffer;
	else
		buf = (PBYTE)AllocatePool( recordsize * SECTOR_SIZE);
	if(!buf)
		return FALSE;

	//bitmapsector = Partition->BootBegin + Partition->MftBeginSector + INODE_BITMAP * recordsize;
	bitmapsector = GetBitmapFileRecordSector(Partition, buf);
	if(!bitmapsector)
		goto cleanup;
	bitmapsector += m_BootBegin;
	DiskRead(m_DiskNumber, bitmapsector, recordsize, buf);
	if (!FixupInode(buf, recordsize))
		goto cleanup;

	//找到data属性
	attr = (PBYTE)FindAttribute((PFILE_RECORD_HEADER)buf, AttributeData);
	if (!attr)
		goto cleanup;


	for ( i=0; i<sizeof(m_BitmapSectorList)/sizeof(SIZE_L); i++)
	{
		if (FindRun((PNONRESIDENT_ATTRIBUTE)attr, vcn, &lcn, &count))
		{
			m_BitmapSectorList[i].Begin  = lcn * secperclr;
			m_BitmapSectorList[i].Number = count * secperclr;
			vcn += count;
		}
		else
			break;
	}

	bRet = TRUE;

cleanup:
	if(buf != g_data.scanbuffer)
		FreePool(buf);

	return bRet;
}

//扫描用了已用扇区，剩余扇区
void  CalcPartitionSize(WIN32_PARTITIONINFO *Partition)
{
	ULONGLONG validsector = 0;
	BOOL bRet = FALSE;

	switch(Partition->PartitionType)
	{
	case FAT16:
		bRet = InitPartParamF16(Partition);
		break;
	case FAT32:
		bRet = InitPartParamF32(Partition);
		break;
	case NTFS:
		bRet = InitNTFS(Partition);
		break;
	case PARTITION_LINUX:
		m_FatSize = 1;
		m_DiskNumber = Partition->DiskNumber;
		m_BootBegin = Partition->BootBegin;
		bRet = TRUE;
		break;
	default:
		break;
	}
	
	if(bRet)
	{
		ULONGLONG	totalUsedClusters;
		totalUsedClusters = GetPartTotalClr(Partition);
		validsector = MultU64x32(totalUsedClusters, m_SectorsPerCluster);
		if(validsector < Partition->TotalSectors)
			Partition->FreeSectors = Partition->TotalSectors - validsector;
		else
			Partition->FreeSectors = 0;

		//if(Partition->PartitionType == PARTITION_LINUX)
		//{
		//	MyLog(LOG_DEBUG, L"boot 0x%llx, total 0x%llx, UsedClusters 0x%llx, valid 0x%llx, free 0x%llx.\n",
		//		m_BootBegin, Partition->TotalSectors, totalUsedClusters, validsector, Partition->FreeSectors);
		//	getch();
		//}
	}
}

///////////////////////////////////
//FUNCTION    :   verify partition.
//prt         :   partition table
//beginsector :   startting sector of partition
//whichdisk   :   disk value based 0x80
//RESULT      :   Return TRUE - the partition valid
//            :          FALSE - the partition invalid.
BOOL  VerifyPartition(PPARTITION_ENTRY  PartitionTableEntry, ULONGLONG BeginSector, WORD DiskNumber)
{
	ULONGLONG TotalSector ;
	int nSupported;
	if ( PartitionTableEntry->pid == 0 )
		return FALSE;   //invalid partition symbol

    TotalSector = g_disklist[DiskNumber].totalsector;
	if ( BeginSector >= TotalSector )
		return FALSE;

	nSupported = IsUnSupportedPartition(PartitionTableEntry->pid);
	if (nSupported == 2)
	{
		//;如果是不认识的分区类型，那么其起始扇区，磁头不是标准的都可以认为是无效的分区
		if((PartitionTableEntry->beginhead != 0)&&(PartitionTableEntry->beginhead != 1))
			return FALSE;

		if((PartitionTableEntry->begin_cylnAndsector & 0x3f) != 1)
			return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////
//FUNCTION   :   filled the Partitions struct 
//driveNo    :   partition  number
//bufffar    :   buffer which stored partition boot sector
//RETURN     :   0
BOOL  CheckPartitionType(BYTE* Buffer, BOOL bGetFreeClr, WIN32_PARTITIONINFO *Partition)
{
	PNTFS_BPB pNt  = (PNTFS_BPB)Buffer;
	PFAT16_BPB p16 = (PFAT16_BPB)Buffer;
	PFAT32_BPB p32 = (PFAT32_BPB)Buffer;
	PEXT3_SUPER_BLOCK pExt3 = (PEXT3_SUPER_BLOCK)(Buffer + 1024);
	
	m_PartitionType = Partition->PartitionType;
	
	if (*(WORD*)(Buffer+0x1fe) != 0xaa55)
	{
		if(pExt3->s_magic == 0xEF53)
			m_PartitionType = PARTITION_LINUX;
		else
			return FALSE;
	}
		
	if(strnicmp((CHAR8*)pNt->OemID, "NTFS", 4) == 0)
		m_PartitionType = NTFS;

	if(strnicmp((CHAR8*)p32->FileSystem, "FAT3", 4) == 0)
		m_PartitionType = FAT32;

	if(strnicmp((CHAR8*)p16->FileSystem, "FAT1", 4) == 0)
		m_PartitionType = FAT16;

	if(m_PartitionType != NTFS && m_PartitionType != FAT32 && m_PartitionType != FAT16)
	{
		if((p16->RootEntry == 0)||(p16->SecPerFat == 0))
		{
			//可能是FAT32或NTFS分区
			if(p32->BigTotalSec == 0)
				m_PartitionType = NTFS;
			else
				m_PartitionType = FAT32;
		}
		else
		{
			if(p16->TotalSec != 0)
			{
				m_PartitionType = FAT12;
			}
			else
				m_PartitionType = FAT16;
		}
	}

	//判读扇区字节数是否为512字节的整数倍
	if(p16->SecInByte == 0 || (p16->SecInByte & 0x1ff) || p16->SecPerClr == 0)
		m_PartitionType = PARTITION_UNUSED;

	//判断SecInByte是否为2的n次方
	if((p16->SecInByte & (p16->SecInByte - 1)) != 0)
		m_PartitionType = PARTITION_UNUSED;
	//判断secperclr是否为2的n次方
	if((p16->SecPerClr & (p16->SecPerClr - 1)) != 0)
		m_PartitionType = PARTITION_UNUSED;
		
	if(m_PartitionType == PARTITION_UNUSED && pExt3->s_magic == 0xEF53)
		m_PartitionType = PARTITION_LINUX;

	m_SectorsPerCluster = p16->SecPerClr;
	Partition->SectorsPerClr = (BYTE)m_SectorsPerCluster;
	
    if (m_PartitionType == NTFS)
	{
		DWORD bytesPerFileRecord;

		Partition->PartitionType = NTFS;
		if(Partition->TotalSectors != pNt->TotalSectors + 1)
			Partition->TotalSectors = pNt->TotalSectors;
		Partition->MftBeginSector = MultU64x32(pNt->MftStartLcn , m_SectorsPerCluster);

		bytesPerFileRecord = pNt->ClustersPerFileRecord;
		if(bytesPerFileRecord > 0x80)
		{
			bytesPerFileRecord = (1 << (0x100 - bytesPerFileRecord));	//bytes 
			Partition->SectorsPerFileRecord = (WORD)(bytesPerFileRecord / pNt->BytesPerSector);
		}
		else
			Partition->SectorsPerFileRecord = (WORD)(pNt->ClustersPerFileRecord * m_SectorsPerCluster);
		Partition->SectorsPerIndxBlock = (WORD)(pNt->ClustersPerIndexBlock * m_SectorsPerCluster);
		
		if(bGetFreeClr)
			CalcPartitionSize(Partition);
	}
	else if(m_PartitionType == FAT32)
	{
		Partition->TotalSectors = p32->BigTotalSec;
		Partition->FirstFatSector = p32->ResSec;
		Partition->SectorsPerFAT = p32->BigSecPerFat;
		Partition->FatCopies = p32->NumOfFat;
		Partition->RootDirClr = p32->RootCluster;
		Partition->PartitionType = FAT32;
		if(bGetFreeClr)
		{
			DWORD freeclr;
			freeclr = *(DWORD*)(Buffer+0x3e8);
			if(freeclr == 0xffffffff)
				CalcPartitionSize(Partition);
			else
				Partition->FreeSectors = (ULONGLONG)m_SectorsPerCluster * freeclr;
		}
	}
	else if(m_PartitionType == FAT16)
	{
		if ( m_SectorsPerCluster > 128 )
		{
			m_PartitionType = PARTITION_UNUSED;
			Partition->PartitionType = PARTITION_UNUSED;
		}
		else
		{
			Partition->TotalSectors = p16->BigTotalSec;
			Partition->FirstFatSector = p16->ResSec;
			Partition->SectorsPerFAT = p16->SecPerFat;
			Partition->FatCopies = p16->NumOfFat;
			Partition->RootSectors = p16->RootEntry*32/512;
			Partition->PartitionType = FAT16;
			if(bGetFreeClr)
				CalcPartitionSize(Partition);
		}
	}
	else if(m_PartitionType == PARTITION_LINUX)
	{
		MyLogBinary(pExt3, 512);

		m_SectorsPerCluster = (1 << (pExt3->s_log_block_size + 1));
		Partition->SectorsPerClr = m_SectorsPerCluster;
		
		Partition->PartitionType = PARTITION_LINUX;
		
		
		Partition->TotalSectors = (ULONGLONG)pExt3->s_blocks_count * m_SectorsPerCluster;
		if(m_SectorsPerCluster <= 2)
			Partition->group_desc_sector = m_SectorsPerCluster * 2;	//
		else
			Partition->group_desc_sector = m_SectorsPerCluster;
		Partition->blocks_per_group = pExt3->s_blocks_per_group;
		Partition->groups_count = 
			(pExt3->s_blocks_count + pExt3->s_blocks_per_group - 1)/pExt3->s_blocks_per_group;

		Partition->FreeSectors = pExt3->s_free_blocks_count * m_SectorsPerCluster;
		if(Partition->FreeSectors > Partition->TotalSectors)
			Partition->FreeSectors = 0;
		if(bGetFreeClr)
			CalcPartitionSize(Partition);
	}
	
	return TRUE;
}

///////////////////////////////////////////////
//FUNCTION          :   Parse the master partition
//prt               :   buffer which stored partition table
//currentpartition  :   current drive number which based 2
//                      C = 2, D = 3, ...
//beginsector       :   begin sector of partition table of partition
//whichdisk         :   disk based 0x80
//bufffar           :   buffer for readding partition boot sector
//RETURN            :   TRUE = success, FALSE = failed
BOOL  ParseMasterPartition(PPARTITION_ENTRY PartitionTableEntry, DWORD PartitionIndex, ULONGLONG BeginSector, ULONGLONG TotalSectors, WORD DiskNumber, BYTE* Buffer)
{
	ULONGLONG bpbsector;
    BOOL bRet = TRUE;

	if(PartitionTableEntry)
	{
		bRet = VerifyPartition(PartitionTableEntry, BeginSector, DiskNumber);
		if(!bRet)
		{
			bpbsector = BeginSector + PartitionTableEntry->frondsectortotal;
			if(DiskRead(DiskNumber, bpbsector, 1, Buffer))
			{
				if(*(WORD*)(Buffer+0x1fe) == 0xAA55)
					bRet = TRUE;
			}
		}
	}

	if ( bRet )
	{
		g_Partitions[PartitionIndex].DiskNumber = DiskNumber;
		g_Partitions[PartitionIndex].DriveLetter = 0;	//Drive + 'C';
		if(PartitionTableEntry)
		{
			g_Partitions[PartitionIndex].PartitionTableBegin = (DWORD)BeginSector;
			g_Partitions[PartitionIndex].BootBegin = BeginSector + PartitionTableEntry->frondsectortotal;
			g_Partitions[PartitionIndex].PartitionType = PartitionTableEntry->pid;
			g_Partitions[PartitionIndex].PartitionId = PartitionTableEntry->pid;
			g_Partitions[PartitionIndex].TotalSectors = PartitionTableEntry->totalsector;
			if ( PartitionTableEntry->id == 0x80 )
				g_Partitions[PartitionIndex].bBootPartition = TRUE;
			else
				g_Partitions[PartitionIndex].bBootPartition = FALSE;
		}
		else
		{
			g_Partitions[PartitionIndex].PartitionTableBegin = 0;
			g_Partitions[PartitionIndex].BootBegin = BeginSector;
			g_Partitions[PartitionIndex].PartitionType = UNKNOWN_PARTITION;
			g_Partitions[PartitionIndex].PartitionId = UNKNOWN_PARTITION;
			g_Partitions[PartitionIndex].TotalSectors = TotalSectors;
		}

		//////////////////////////////////////////////////////////////////////////
		bpbsector = g_Partitions[PartitionIndex].BootBegin;
		//读取4个扇区，因为linux分区的superblock始终是位于从1024字节开始的两个扇区里
		DiskRead(DiskNumber, bpbsector, 4, Buffer);
		if (!CheckPartitionType(Buffer, TRUE, &g_Partitions[PartitionIndex])){ //boot sector invalid
			//g_Partitions[PartitionIndex].PartitionId = UNKNOWN_PARTITION;;
			m_PartitionType = UNKNOWN_PARTITION;
		}
		if(m_PartitionType == PARTITION_UNUSED)
			bRet = FALSE;
		else
			g_disklist[DiskNumber].formatted = TRUE;
	}

	return bRet;
}

///////////////////////////////////////////////
//FUNCTION          :   Parse the extend partition
//prt               :   buffer which stored partition table
//currentpartition  :   current drive number which based 2
//                      C = 2, D = 3, ...
//beginsector       :   begin sector of partition table of partition
//whichdisk         :   disk based 0x80
//bufffar           :   buffer for readding partition boot sector
//RETURN            :   currentpartition is returned
DWORD ParseExtendPartition(PPARTITION_ENTRY  PartitionTableEntry, DWORD PartitionIndex, 
		ULONGLONG BeginSector, WORD whichdisk, BYTE* bufffar)
{
	BYTE       nextprt_beginhead;
	BYTE       nextprt_sector;
	PPARTITION_ENTRY  prt;
	DWORD       nextprt_begincyln;
	DWORD      nextprt_beginsec;
	DWORD		partitionNumber;
	DWORD		count;
    int i;
    
    m_RecursiveTimes++;
	if ( m_RecursiveTimes >= 32 )
		return 0;

	partitionNumber = 0;
	nextprt_beginsec = (DWORD)(PartitionTableEntry->frondsectortotal + BeginSector);
	if ( BeginSector == 0 )
		BeginSector = nextprt_beginsec;

	if(!DiskRead(whichdisk, nextprt_beginsec, 1, bufffar))
		return 0;

	if ( *(WORD* )(bufffar+0x1fe) != 0xaa55 )
		return 0;

	prt = (PPARTITION_ENTRY)(bufffar+0x1be);
	for( i=0; i<4; i++)
	{
		if ( prt->pid == EXTEND_PARTITION || prt->pid == BIG_EXTEND_PARTITION )
		{
			count = ParseExtendPartition( prt, PartitionIndex, BeginSector, whichdisk, bufffar+512 );
			partitionNumber += count;
			PartitionIndex += partitionNumber;
		}
		else if ( prt->pid != 0 )
		{
			if(ParseMasterPartition( prt, PartitionIndex, nextprt_beginsec, prt->totalsector, whichdisk, bufffar+512 ))
			{
				partitionNumber++;
				PartitionIndex++;
			}
		}
		if ( PartitionIndex >= 32 )  // not support exceed 26 partitions
			break;
        prt++;
	}

	return partitionNumber;
}

BOOL  IsGPT(WORD DiskNumber);
DWORD  ParseGPTInfo(WORD DiskNumber, PBYTE Buffer);
BOOL  SortPartitionInfo();
VOID GetVolumeName(WORD partindex);

//////////////////////////////////////////////////////
//FUNCTION    :   get all system partition info for one harddisk                because 98 drives order isn't same with NT 
//whichdisk   :   Harddisk number
//result      :   partition number;  0 or 100 means failed
DWORD  GetPartitionInfo()
{
	PPARTITION_ENTRY	prt;
	BYTE	*buffer;
	DWORD	count;
	WORD	i, j;
	
	InitPartition();

	buffer = (unsigned char *)AllocatePool(64*1024);
	for(i=0; i<g_disknum; i++)
	{
		if ( g_dwTotalPartitions >= MAX_NUM_PARTITIONS )
			break;

		if(IsGPT(i))
		{
			ParseGPTInfo(i, buffer);
		}
		else
		{
			m_bLBAMode = FALSE;
			if(!DiskRead(i,0, 1, buffer))
			{
				MyLog(LOG_VERBOSE,L"GetPartitionInfo::Read(0,1) failed with error.\r\n");
				continue;
			}

			if(ParseMasterPartition(NULL, g_dwTotalPartitions, 0, 0, i, buffer))
			{
				g_dwTotalPartitions++;
			}
			else
			{
				prt = (PPARTITION_ENTRY)(buffer+0x1be);
				for(j=0; j<4; j++)
				{
					if ( prt->pid == EXTEND_PARTITION || prt->pid == BIG_EXTEND_PARTITION)
					{
						if (( prt->pid == BIG_EXTEND_PARTITION ) || (g_disklist[i].totalsector >= 0x1000000))
							m_bLBAMode = TRUE;

						count = ParseExtendPartition(prt, g_dwTotalPartitions, 0, i, buffer+512);
						g_dwTotalPartitions += count;
						m_bLBAMode = FALSE;
					}
					else if( prt->pid != PARTITION_UNUSED )
					{
						if(ParseMasterPartition( prt, g_dwTotalPartitions, 0, prt->totalsector, i, buffer+512 ))
							g_dwTotalPartitions++;
					}

					if ( g_dwTotalPartitions >= MAX_NUM_PARTITIONS )
						break;
					prt++;
				}
			}
		}
		//每个硬盘的分区都要按先后顺序排序，并进行校验，避免有交叉或其它不合法分区的存在
		SortPartitionInfo();
	}
	FreePool(buffer);

	for (i=0;i<g_disknum;i++)
  {
  	CheckPartitionHandle(i);
  	
  	CheckFatPartitionHandle(i);
  }      
	
	for(i=0;i<g_dwTotalPartitions;i++) {
		if(g_Partitions[i].PartitionId == LENOVO_PART)
			g_data.oempartition = i;
		GetVolumeName(i);
	}

	for(i=0;i<g_dwTotalPartitions;i++) {

		MyLog(LOG_ERROR, L"Partition %d:%d, type=%x, begin=%x, total=%x, free %x, handle %x.\r\n", 
			i, g_Partitions[i].DiskNumber, g_Partitions[i].PartitionType, g_Partitions[i].BootBegin, 
			g_Partitions[i].TotalSectors,
			g_Partitions[i].FreeSectors, 
			g_Partitions[i].Handle);
		if(g_Partitions[i].PartitionType == FAT32) {

			MyLog(LOG_DEBUG, L"FirstFatSector %d, RootDirClr %d, SectorsPerFAT %d.\r\n", 
				g_Partitions[i].FirstFatSector,  g_Partitions[i].RootDirClr, 
				g_Partitions[i].SectorsPerFAT);
		}
		//	MyLog(LOG_DEBUG,L"Type guid: ");
		//	DumpHex(0,0,16,g_Partitions[i].TypeGuid);
		//	MyLog(LOG_DEBUG,L"\nUnique guid: ");
		//	DumpHex(0,0,16,g_Partitions[i].UniGuid);
		//	MyLog(LOG_DEBUG,L"\n");
	}

	if(g_dwTotalPartitions == 0)
		MyLog(LOG_DEBUG, L"Failed to analysis partition information, i %d.\r\n", i);
	//if(g_data.bDebug) {
	//	getch();
	//}

//	MyLog(LOG_DEBUG,L"GetPartitionInfo::totalpartiton=%d", g_dwTotalPartitions);
//	for(i=0;i<g_dwTotalPartitions;i++)
//	{
//	//	driveLetter = (char)g_Partitions[i].DriveLetter;
//	//	if(driveLetter == 0)
//	//		driveLetter = '0';
//		MyLog(LOG_DEBUG,L"Partition %d, begin=%ld, total=%ld, type=%d", i, g_Partitions[i].BootBegin, g_Partitions[i].TotalSectors, g_Partitions[i].PartitionType);
//	}

	CleanPartition();

	return g_dwTotalPartitions;
}

int  GetPartitionType(BYTE *Buf)
{
	PNTFS_BPB pNt = (PNTFS_BPB)Buf;
	PFAT16_BPB p16 = (PFAT16_BPB)Buf;
	PFAT32_BPB p32 = (PFAT32_BPB)Buf;

	if(!strnicmp((char*)pNt->OemID, "NTFS", 4))
		return NTFS;
	else if(!strnicmp((char*)p16->FileSystem,"FAT16",5))
		return FAT16;
	else if(!strnicmp((char*)p32->FileSystem,"FAT32",5))
		return FAT32;
	return 0;
}

BOOL  SortPartitionInfo()
{
	//对分区信息进行排序
	WIN32_PARTITIONINFO w32partition;
	WORD i,j;
	for( i=0;i<g_dwTotalPartitions;i++)
	{
		for(j=i+1;j<g_dwTotalPartitions;j++)
		{
			if(g_Partitions[i].BootBegin > g_Partitions[j].BootBegin && g_Partitions[i].DiskNumber == g_Partitions[j].DiskNumber)
			{
				CopyMem(&w32partition, &g_Partitions[i], sizeof(WIN32_PARTITIONINFO));
				CopyMem(&g_Partitions[i], &g_Partitions[j], sizeof(WIN32_PARTITIONINFO));
				CopyMem(&g_Partitions[j], &w32partition, sizeof(WIN32_PARTITIONINFO));
			}
		}
	}
	return TRUE;
}

BOOL IsGPT(WORD DiskNumber)
{
	BOOL bGpt = FALSE;
	PBYTE buffer;
	WORD i;

	//GPT分区一共可以描述128个分区信息，共占用32个扇区，16K字节大小; 如果加上MBR和GPT Header,则一共是34个扇区
	//这里分配32K字节， 以防一个扇区不止512字节，因为至少要占用2个扇区+128项

	//分析GPT分区
//	if (!m_Disk.Open(DiskNumber))
//	{
//		logprintf("IsGPT, Failed to open disk(%d) with error 0x%x.\r\n", DiskNumber, ::GetLastError());
//		return FALSE;
//	}
	buffer = (PBYTE)AllocatePool( 4*1024);
	if(!buffer)
	{
		MyLog(LOG_ERROR,L"IsGPT, failed to allocate memory\r\n");
		return FALSE;
	}

	//BYTE  buffer[0x1000];
	if(!DiskRead(DiskNumber,0, 1, buffer))
	{
		MyLog(LOG_ERROR,L"IsGPT::Read(%d,0,1) failed with error .\r\n", DiskNumber);
	}
	else
	{
		if (*(WORD*)(buffer+0x1fe) == 0xAA55)
		{
			PPARTITION_ENTRY entry = (PPARTITION_ENTRY)(buffer+0x1be);
			for ( i=0;i<4;i++)
			{
				if (entry->pid == GPT_PROTECTIVE_PARTITION || entry->pid == GPT_SYSTEM_PARTITION) 
				{
					bGpt = TRUE;
					break;
				}
				entry++;
			}
		}
		else
		{
			//奇怪，有问题，输出信息到日志文件中
#if OKR_DEBUG
			MyLog(LOG_DEBUG,L"IsGPT(%d). not 55aa\r\n", DiskNumber);
#endif
		}
	}
    if (!bGpt)
	{
    	SetMem(g_disklist[DiskNumber].guid,16,0);
    	CopyMem((PBYTE)g_disklist[DiskNumber].guid+12,buffer+0x1b8 ,4);
    	
    }
	if(buffer)
	{
		FreePool(buffer);
	}
	return bGpt;
}

DWORD  ParseGPTInfo(WORD DiskNumber, PBYTE Buffer)
{
	EFI_PARTITION_TABLE_HEADER *header;
    DWORD i,num;
	EFI_PARTITION_ENTRY *entry;
	BOOL bValidEntry;

	if(!DiskRead(DiskNumber,0, 64, Buffer))
	{
		MyLog(LOG_ERROR,L"ParseGPTInfo::Read(%d,0,64) failed with error .\r\n", DiskNumber);
	}
	header = (EFI_PARTITION_TABLE_HEADER *)(Buffer + 512);

	if(strnicmp((CHAR8 *)&header->Header.Signature, "EFI PART", 8) != 0)
	{
		MyLog(LOG_VERBOSE,L"Invalid GPT signature.\r\n");
		//logbinary(LOG_LEVEL_INFO, (PBYTE)&header->Header.Signature, 32);
		//logprintf("Mbr:\r\n");
		//logbinary(LOG_LEVEL_INFO, (PBYTE)Buffer, 512);
		return 0;
	}
	CopyMem(g_disklist[DiskNumber].guid,(VOID *)&header->DiskGUID,16);

    num = header->NumberOfPartitionEntries;
	if(num > 128)
		num = 128;

	entry = (EFI_PARTITION_ENTRY *)((char*)header + MultU64x32(header->PartitionEntryLBA - header->MyLBA, SECTOR_SIZE));
//	MyLog(LOG_DEBUG,L"Buffer %p, Entry %p, PartitionLba 0x%lx, mylba 0x%ld", Buffer, entry, header->PartitionEntryLBA, header->MyLBA);

	for( i=0;i<num;i++)
	{
		bValidEntry = TRUE;

		if(entry->PartitionTypeGUID.Data1 == 0 && entry->PartitionTypeGUID.Data2 == 0 && entry->PartitionTypeGUID.Data3 == 0)
			bValidEntry = FALSE;

		if(entry->StartingLBA == 0 || entry->EndingLBA == 0)
			bValidEntry = FALSE;

	//	MyLog(LOG_DEBUG,L"ParseGPTInfo %d, bValid %d", i, bValidEntry);
		if(bValidEntry)
		{
		//	MyLog(LOG_DEBUG,L"ParseGPTInfo, index %d, start %ld, total %ld", g_dwTotalPartitions, entry->StartingLBA, entry->EndingLBA - entry->StartingLBA);
			if(ParseMasterPartition(NULL, g_dwTotalPartitions, entry->StartingLBA, entry->EndingLBA - entry->StartingLBA + 1, DiskNumber, Buffer + 0x8000)){
				//Print(L" g_Partitions[g_dwTotalPartitions].TotalSectors=%d ",g_Partitions[g_dwTotalPartitions].TotalSectors);
				CopyMem(g_Partitions[g_dwTotalPartitions].TypeGuid, &entry->PartitionTypeGUID, 16);
				CopyMem(g_Partitions[g_dwTotalPartitions].UniGuid, &entry->UniquePartitionGUID, 16);
				g_Partitions[g_dwTotalPartitions].bGpt = TRUE;
				g_dwTotalPartitions++;
			}
		}
		entry = (EFI_PARTITION_ENTRY*)((PBYTE)entry + header->SizeOfPartitionEntry);
	}

	return g_dwTotalPartitions;
}

int  GetPartitionIndex(WORD DiskNumber, DWORD Index)
{
	WORD i;
	for ( i=0; i<g_dwTotalPartitions; i++)
	{
		if (g_Partitions[i].DiskNumber == DiskNumber)
		{
			if (Index == 0)
				return i;
			Index--;
		}
	}
	return -1;
}

VOID GetVolumeName_bad(WORD partindex)
{
	BYTE type = g_Partitions[partindex].PartitionType;
	WORD disknum = g_Partitions[partindex].DiskNumber;
	UINT64 beginsec = g_Partitions[partindex].BootBegin;
	BYTE buffer[512] = { 0 } ;
	BYTE label[64] = { 0 };
	PBYTE buf = NULL;

	SetMem(label,64,0);
	SetMem(g_Partitions[partindex].Label,sizeof(g_Partitions[partindex].Label),0);
	if (type == FAT32 || type == FAT32_E){

		PFAT32_BPB p = (PFAT32_BPB )buffer;
		//beginsec += g_Partitions[partindex].SectorsPerFAT*2+g_Partitions[partindex].FirstFatSector;

  	DiskRead(disknum, g_Partitions[partindex].BootBegin, 1, buffer);
	
  	beginsec = g_Partitions[partindex].BootBegin+p->ResSec + g_Partitions[partindex].SectorsPerFAT*2+(p->SecPerClr)*(p->RootCluster-2);
		DiskRead(disknum, beginsec, 1, buffer);
		CopyMem(label,buffer,11);
		Ascii2Unicode(g_Partitions[partindex].Label, label);

	}else if (type == NTFS){

		RESIDENT_ATTRIBUTE *attr;
		UINT64 volsector;
		DWORD recordsize = g_Partitions[partindex].SectorsPerFileRecord;

		//bitmapfile position in mftfile is 6, 0 - based.
		buf = (PBYTE)AllocatePool( recordsize * SECTOR_SIZE);
		if(!buf)
			return ;

		volsector = beginsec + g_Partitions[partindex].MftBeginSector + INODE_VOLUME * recordsize;
			
		DiskRead(disknum,volsector, recordsize, buf);

		if (!FixupInode(buf, recordsize)){
			MyLog(LOG_ALERT,L"fixupinodex error");
			goto cleanup;
	    }
		//找到data属性
		attr = (RESIDENT_ATTRIBUTE *)FindAttribute((PFILE_RECORD_HEADER)buf, AttributeVolumeName);
		if (!attr){
			MyLog(LOG_ALERT,L"Can't find data of bitmap");
			goto cleanup;
		}
		if (attr->ValueLength > 64)
			attr->ValueLength = 64;
		CopyMem( g_Partitions[partindex].Label,(PBYTE)attr+sizeof(RESIDENT_ATTRIBUTE),attr->ValueLength);

	}else if (type == FAT16 || type == FAT16_E) {

		PFAT16_BPB p = (PFAT16_BPB)buffer;
		DiskRead(disknum, beginsec, 1, buffer);
		CopyMem(label,p->DISKLabel,11);
		Ascii2Unicode(g_Partitions[partindex].Label, label);
	}

cleanup:	
	if (buf) 
		FreePool(buf);
	//MyLog(LOG_DEBUG,L"part %d:label:%s--- \n",partindex,g_Partitions[partindex].Label,sizeof(RESIDENT_ATTRIBUTE));
}


VOID GetVolumeName(WORD partindex)
{
	BYTE type = g_Partitions[partindex].PartitionType;
	WORD disknum = g_Partitions[partindex].DiskNumber;
	UINT64 beginsec = g_Partitions[partindex].BootBegin;
	BYTE buffer[512];
	BYTE label[64];
	PBYTE buf = NULL;
	//kang++++++>
	int i=0;
	DWORD RootStartSec = 0;
	WORD  ReservedSec =0;
    BYTE ClusterSize = 0;
	BYTE  FAT_Num =0;
	DWORD   FAT_Size =0;
	DWORD   Root_StartCluster =0;
	//kang++++++<
	
	SetMem(label,64,0);
	SetMem(g_Partitions[partindex].Label,sizeof(g_Partitions[partindex].Label),0);
	if (type == FAT32 || type == FAT32_E){

		PFAT32_BPB p = (PFAT32_BPB )buffer;
		beginsec += g_Partitions[partindex].SectorsPerFAT*2+g_Partitions[partindex].FirstFatSector;
		DiskRead(disknum, beginsec, 1, buffer);
		
		//kang++++++>
		
		ReservedSec = *((WORD*)&buffer[0xe]);
		ClusterSize =   buffer[0xd] ;
		FAT_Num    =   buffer[0x10] ;
		FAT_Size   = *((DWORD*)&buffer[0x24]);
		Root_StartCluster  = *((DWORD*)&buffer[0x2c]);
		
		RootStartSec =  ReservedSec + FAT_Num *FAT_Size + (Root_StartCluster -2)*ClusterSize;
		
		
		
		DiskRead(disknum, beginsec + RootStartSec , 1, buffer);
		
		for(i = 0 ;i< 512 ; i=i+32)
		{
			if(buffer[0xb + i]  ==8)
				break;
		}
		if(i<512)
			CopyMem(label,&buffer[i],11);
		else //kang++++++<
		CopyMem(label,buffer,11);
		Ascii2Unicode(g_Partitions[partindex].Label, (CHAR8*)label);

	}else if (type == NTFS){

		RESIDENT_ATTRIBUTE *attr;
		UINT64 volsector;
		DWORD recordsize = g_Partitions[partindex].SectorsPerFileRecord;

		//bitmapfile position in mftfile is 6, 0 - based.
		buf = (PBYTE)AllocatePool( recordsize * SECTOR_SIZE);
		if(!buf)
			return ;

		volsector = beginsec + g_Partitions[partindex].MftBeginSector + INODE_VOLUME * recordsize;
			
		DiskRead(disknum,volsector, recordsize, buf);

		if (!FixupInode(buf, recordsize)){
			MyLog(LOG_ALERT,L"fixupinodex error.\r\n");
			goto cleanup;
	    }
		//找到data属性
		attr = (RESIDENT_ATTRIBUTE *)FindAttribute((PFILE_RECORD_HEADER)buf, AttributeVolumeName);
		if (!attr){
			MyLog(LOG_ALERT,L"Can't find data of bitmap.\r\n");
			goto cleanup;
		}
		if (attr->ValueLength > 64)
			attr->ValueLength = 64;
		CopyMem( g_Partitions[partindex].Label,(PBYTE)attr+sizeof(RESIDENT_ATTRIBUTE),attr->ValueLength);

	}else if (type == FAT16 || type == FAT16_E) {

		PFAT16_BPB p = (PFAT16_BPB)buffer;
		DiskRead(disknum, beginsec, 1, buffer);
		CopyMem(label,p->DISKLabel,11);
		Ascii2Unicode(g_Partitions[partindex].Label, (CHAR8*)label);
	}

cleanup:	
	if (buf) 
		FreePool(buf);
	//MyLog(LOG_DEBUG,L"part %d:label:%s--- \n",partindex,g_Partitions[partindex].Label,sizeof(RESIDENT_ATTRIBUTE));
}











//return block index
DWORD GetExt3BitmapSector(WIN32_PARTITIONINFO *Partition, DWORD GroupIndex)
{
	PEXT3_GROUP_DESC group_desc;
	
	if(!m_group_desc)
	{
		ULONGLONG sector;
		DWORD desc_size = Partition->groups_count * sizeof(EXT3_GROUP_DESC);
		desc_size = ((desc_size + 0xfff) & ~0xfff);
		if(!g_data.groupdesc || desc_size > g_data.groupdescsize)
		{
			if(g_data.groupdesc)
				myfree(g_data.groupdesc);
			g_data.groupdescsize = desc_size;
			g_data.groupdesc = myalloc(g_data.groupdescsize);
		}
		m_group_desc = (PEXT3_GROUP_DESC)g_data.groupdesc;
		sector = Partition->group_desc_sector + Partition->BootBegin;
		DiskRead(Partition->DiskNumber, sector, desc_size/SECTOR_SIZE, m_group_desc);
	}

//group_desc = m_group_desc + 100000;
//MyLog(LOG_ERROR, L"desc %p, m_group_desc %p, size %d, bitmap %d\n", 
//	  group_desc, m_group_desc, g_data.groupdescsize, group_desc->bg_block_bitmap);
	
	group_desc = m_group_desc + GroupIndex;
	if(group_desc->bg_flags & EXT4_BG_BLOCK_UNINIT)	//没有初始化，不需要备份
		return 0;
	return (DWORD)(group_desc->bg_block_bitmap * (DWORD)Partition->SectorsPerClr);
}

extern EFI_GUID PARTITION_LINUX_SWAP_GUID ;
extern EFI_GUID PARTITION_LINUX_BOOT_GUID ;
//////////////////////////////////////////////////////////////////// 扫描数据代码，重构CalcPartitionSize过程。
//当扫描到数据块是有效数据时，调用函数指针
int ScanPartitionData(WORD partindex ,FUNCGetData getdatafunc,VOID *private,PBYTE test)
{
	ULONGLONG total = 0;
	DWORD k = 0, i,count, remainSectors;
    BYTE *Buffer;
    DWORD buffersize = 0, bitmapsize = 0;
	ULONGLONG databeginsector;
	DWORD	groupindex, blocksize;
    ULONGLONG cluster, fatSector;
    ULONGLONG totalsector;
    BOOL bRet = TRUE; 
	BOOL bUsed;
	BOOL RawSec = FALSE;
#if OKR_DEBUG
    ULONGLONG backupsectors, groupsize;
#endif
	
    m_group_desc = NULL;
	m_PartitionType = g_Partitions[partindex].PartitionType;
	m_PartitionIndex = (BYTE)partindex;
	m_BootBegin = g_Partitions[partindex].BootBegin;
	m_DiskNumber = g_Partitions[partindex].DiskNumber;
	m_SectorsPerCluster = g_Partitions[partindex].SectorsPerClr;
	totalsector = g_Partitions[partindex].TotalSectors;
	
#if OKR_DEBUG
	backupsectors = 0;
	groupsize = 0;
#endif	


	//	if(( g_Partitions[partindex].bGpt &&  (CompareGuid(&PARTITION_LINUX_SWAP_GUID,g_Partitions[partindex].TypeGuid)) ))	//swap分区需要备份
	//	{
	//		RawSec = TRUE;
	//		goto Linux_Swap_Biosboot ;
	//	}
			
		if(( g_Partitions[partindex].bGpt &&  (CompareGuid(&PARTITION_LINUX_BOOT_GUID,g_Partitions[partindex].TypeGuid) ) ))	//legacy boot 分区需要备份
		{
			RawSec = TRUE;
			goto Linux_Swap_Biosboot ;
		}
	

	if (IsUnSupportedPartition(m_PartitionType)==TRUE){
#if OKR_DEBUG
		MyLog(LOG_DEBUG,L"Scan Data Unsupport this filesystem type %x\n",m_PartitionType);
		 
#endif
		return -1;
    }
Linux_Swap_Biosboot:
	switch(m_PartitionType)
	{
	case FAT16_E:
	case FAT16:
		m_FatSize = 2;
		fatSector = g_Partitions[partindex].FirstFatSector + m_BootBegin;
		total = g_Partitions[partindex].SectorsPerFAT;
		databeginsector = g_Partitions[partindex].FirstFatSector + g_Partitions[partindex].SectorsPerFAT*2+32;
		getdatafunc(0,databeginsector,private);
		break;
		
	case FAT32:
	case FAT32_E:
		m_FatSize = 4;
	    total = g_Partitions[partindex].SectorsPerFAT;
		fatSector = g_Partitions[partindex].FirstFatSector + m_BootBegin;
		databeginsector = g_Partitions[partindex].FirstFatSector+g_Partitions[partindex].SectorsPerFAT*2;
        getdatafunc(0,g_Partitions[partindex].FirstFatSector,private);
        getdatafunc(g_Partitions[partindex].FirstFatSector,g_Partitions[partindex].SectorsPerFAT*2,private);
		break;
		
	case NTFS:
		bRet = InitNTFS(&g_Partitions[partindex]);
		if (bRet){
			fatSector = m_BitmapSectorList[0].Begin + m_BootBegin;
			total = (DWORD)(DivU64x32(DivU64x32(totalsector, m_SectorsPerCluster),(512*8)) + 1);
        	databeginsector=0;
		} else {
#if OKR_DEBUG
			MyLog(LOG_ALERT,L"ScanPartition can't scan this ntfs, ScanPartition can't scan this ntfs");
#endif
		}
		break;
		
	case PARTITION_LINUX:
		m_FatSize = 1;
		groupindex = 0;
		blocksize = m_SectorsPerCluster * SECTOR_SIZE;
		buffersize = (((g_Partitions[partindex].blocks_per_group + 7)/8 + blocksize - 1) & ~(blocksize - 1));
		bitmapsize = (buffersize + SECTOR_SIZE - 1)/SECTOR_SIZE;
		total = g_Partitions[partindex].groups_count * bitmapsize;
#if OKR_DEBUG
	 	MyLog(LOG_DEBUG,L"bitmapsize:%d, groups_count %d, total %d\n", 
		 	buffersize/SECTOR_SIZE, g_Partitions[partindex].groups_count, total);
#endif
		break;
		
	default:
//#if OKR_DEBUG		
//		MyLog(LOG_DEBUG,L"ScanPartition don't support filesystem, %d, pid %d", partindex, pid);
//#endif
		//kang --bRet=FALSE;
		//raw backup ---------->
		
		m_FatSize = 77;
		groupindex = 0;
		 
		buffersize = (g_Partitions[partindex].TotalSectors + 63)/64;
		total = (buffersize+7)/8 ;
	 
		SetMem(private,total,0xff);
		
		
		
		
		
		bRet=TRUE;
		//raw backup ----------<
		return 0;
	}
	
	if (!bRet) {
		return -1;
    }
//	 MyLog(LOG_DEBUG,L"part:%d ,fatsector/bitmap=%ld ,databegin=%ld, secperclr=%ld\n",partindex,fatSector,databeginsector,m_SectorsPerCluster);

    ///////////////////////////////////////////////////////////////////////
    if(buffersize < SCANBUFFER_SIZE)
    	buffersize = SCANBUFFER_SIZE;
	Buffer = g_data.scanbuffer;

	// if fat16, one fat sector have 512/2 clust
	// if fat32, one fat sector have 512/4 clust
	// if NTFS, one bitmap sector have 512*8 clust

	if (m_PartitionType == NTFS)
	{
		remainSectors = (DWORD)m_BitmapSectorList[0].Number;
		k++;
	}

	cluster = 0;
	while (total > 0)
	{
#if OKR_DEBUG
		groupsize = 0;
#endif		
		if(m_PartitionType == PARTITION_LINUX)
		{
			count = bitmapsize;
			fatSector = GetExt3BitmapSector(&g_Partitions[partindex], groupindex);
			if(fatSector == 0)
			{
				total -= count;
				groupindex++;
				continue;
			}
			else
				fatSector += m_BootBegin;
		}
		else
		{
			if (total > buffersize/SECTOR_SIZE)
				count = buffersize/SECTOR_SIZE;
			else
				count =(DWORD)total;
			if (m_PartitionType == NTFS)
			{
				if (count > remainSectors)
					count = remainSectors;
			}
		}
		
	    DiskRead(m_DiskNumber, fatSector, count, Buffer);

		total -= count;
		fatSector += count;
		
		if(m_FatSize == 2) // fat16
			count *= 256;
		else if(m_FatSize == 4)
			count *= 128;
		else if(m_PartitionType == NTFS)
		{
			remainSectors -= count;
			if (remainSectors == 0)
			{
				fatSector = m_BitmapSectorList[k].Begin + m_BootBegin;
				remainSectors = (DWORD)m_BitmapSectorList[k].Number;
				k++;
				total = remainSectors;
			}
			count *= 512;
		}
		else if(m_PartitionType == PARTITION_LINUX)
		{
			cluster = groupindex * g_Partitions[partindex].blocks_per_group / 8;

			if(total == 0)
				count = ((totalsector / m_SectorsPerCluster) % g_Partitions[partindex].blocks_per_group);
			else
				count = g_Partitions[partindex].blocks_per_group;
			count = (count + 7) / 8;
		}
		 

		for ( i=0; i<count; i++)
		{
			if(Buffer[i*m_FatSize] != 0)
			{
				bUsed = TRUE;
			}
			else
			{
				bUsed = FALSE;
				if(m_FatSize == 2) // fat16
				{
					if(*(WORD*)(Buffer + i*m_FatSize) != 0)
						bUsed = TRUE;
				}
				else if(m_FatSize == 4)
				{
					if(*(DWORD*)(Buffer + i*m_FatSize) != 0)
						bUsed = TRUE;
				}
			}
			if (bUsed) 
			{
				if (m_FatSize == 1){//ntfs
					if (cluster*8*m_SectorsPerCluster < totalsector)
					{
						getdatafunc(cluster*8*m_SectorsPerCluster, m_SectorsPerCluster*8, private);
#if OKR_DEBUG
						backupsectors += 8*m_SectorsPerCluster;
						groupsize += 8*m_SectorsPerCluster;
#endif						
					}
				}else{
					if (cluster >= 2){
						if(((cluster-2)*m_SectorsPerCluster)<=totalsector){
							getdatafunc(databeginsector+(cluster-2)*m_SectorsPerCluster, m_SectorsPerCluster,private);
#if OKR_DEBUG
							backupsectors += m_SectorsPerCluster;
#endif							
						}
				   }
				   
				  
				}
			}
        	cluster++;
		}
//#if OKR_DEBUG
//	    MyLog(LOG_DEBUG,L"ScanPartitionData(%d), %d: fatSector 0x%x, backup %d, groupsize %d, count %d\n",
//    		partindex, groupindex, fatSector, backupsectors, groupsize, count);
//    	getch();
//#endif		
		groupindex++;
	}
	
#if OKR_DEBUG
    MyLog(LOG_DEBUG,L"ScanPartitionData(%d), backup %d, free %d, total %d\n",
    	partindex, backupsectors, g_Partitions[partindex].FreeSectors, 
    	g_Partitions[partindex].TotalSectors);
	//getch();
	//totalsector = g_Partitions[partindex].TotalSectors - g_Partitions[partindex].FreeSectors;
	//if(backupsectors > totalsector + 2048*500)
	//{
	//	return -1;
	//}
#endif
	
	//g_Partitions[partindex].BackupDataSize = backupsectors;
     
    m_group_desc = NULL;

    return 0;
}

