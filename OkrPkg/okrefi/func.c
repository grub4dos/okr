/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

//#include <stdio.h>
#include "okr.h"
#include "faceUIDefine.h"
 
#include <stdio.h>
#include<stdio.h>
#include<string.h>
#include "..\pbkdf2_sha256\sha2.h"

#define SHA256_LEN		32
#define PBKDF2P_LEN     32
#define SALT_LEN		32
#define ITER_LEN		 1

BOOLEAN IsNoDataPartFlag = FALSE; 

#define FILE_SHA256_SEG_NUMBER			17
#define FILE_PER_SECTOR_DATA			(10 * 1024 * 1024)
#define MULTI_FILE_PER_SECTOR_DATA		(5 * 1024 * 1024)

//0:WinRE is before OS partition
//1:WinRE is after Data partition
//2:WinRE is after OS partition
BYTE g_WinREState = 0;		
	
BYTE Windows_Data_TypeGuid[16]={0xa2,0xa0,0xd0,0xeb,0xe5,0xb9,0x33,0x44,0x87,0xc0,0x68,0xb6,0xb7,0x26,0x99,0xc7};
BYTE Window_Recovery_TypeGuid[16]={0xa4,0xbb,0x94,0xde,0xd1,0x06,0x40,0x4d,0xa1,0x6a,0xbf,0xd5,0x01,0x79,0xd6,0xac};
BYTE Uefi_System_TypeGuid[16]={0x28,0x73,0x2a,0xc1,0x1f,0xf8,0xd2,0x11,0xba,0x4b,0x0,0xa0,0xc9,0x3e,0xc9,0x3b};
BYTE Window_Reserved_TypeGuid[16]={0x16,0xe3,0xc9,0xe3,0x5c,0x0b,0xb8,0x4d,0x81,0x7d,0xf9,0x2d,0xf0,0x02,0x15,0xae};
EFI_GUID PARTITION_LINUX_SWAP_GUID =  { 0x0657fd6d, 0xa4ab, 0x43c4, {0x84, 0xe5, 0x09, 0x33, 0xc8, 0x4b, 0x4f, 0x4f}};
EFI_GUID PARTITION_LINUX_BOOT_GUID =  { 0x21686148, 0x6449, 0x6e6f, {0x74, 0x4e, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49}};
BYTE Uefi_ESP_TypeGuid[16]={0xC1,0x2A,0x73,0x28,0xF8,0x1F,0x11,0xD2, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B};
 	
PBYTE g_lz4_buffer=NULL;
INTN  g_lz4_size; 			//=0 first time;>0 compressed size; <0 error
ULONGLONG g_wireTotalSectors; 

UINT32 g_databitmapbufsize;

BOOLEAN  IsLinuxSystem = FALSE;
UINT32 gcount;

BOOL g_bUserBreak = FALSE;

extern void MyLogA(const char *Format, ...);


BOOLEAN CheckHandleValid(EFI_HANDLE handle)
{
 
	 EFI_STATUS						UsbStatus = EFI_SUCCESS;
	 EFI_BLOCK_IO_PROTOCOL		*BlockIOProtocol=NULL;
 
	 BOOLEAN RetStatus = FALSE;
 
	 
		 
		 UsbStatus = gBS->HandleProtocol(handle,&gEfiBlockIoProtocolGuid,(VOID **)&BlockIOProtocol);
 
		 if( UsbStatus == EFI_SUCCESS )
		 {
			 MyLog(LOG_DEBUG, L"CheckHandleValid  Step1.\n");
			  MyLog(LOG_DEBUG, L"RemovableMedia  true.\n");
			  if(BlockIOProtocol->Media->MediaPresent)
			   MyLog(LOG_DEBUG, L"MediaPresent   \n");
	  
			 if(TRUE == BlockIOProtocol->Media->MediaPresent)
				 RetStatus = TRUE;
				 
		 }
	 
 
	 return RetStatus;
 
}

void SetNTFSVolDirty(WORD disknum, UINT64 sectoroff)
{
	char *buffer = AllocatePool(512);
	char *bitmapbuffer = AllocatePool(1024);
	 
	UINT64 StartSec = 0;
	UINT64 StartClus = 0;
	
	UINT16 Attr_start = 0;
	UINT16 A_70_Attr_start = 0;
	UINT32 i =0;
	 
	UINT16 runlist_start = 0;
	
	DiskRead(disknum,sectoroff,1,buffer);
	
	if(buffer[3]!='N' || buffer[4]!='T' || buffer[5]!='F' || buffer[6]!='S')
	{
		MyLogA("SetNTFSVolDirty not ntfs\n");
		goto Done ;
	}
	MyLogA("SetNTFSVolDirty find ntfs\n");
	 
	
	StartClus =  *(UINT64*)&buffer[0x30];
	MyLog(LOG_ERROR,L"MFT_Header %llx\r\n", StartClus);
	StartSec  = sectoroff + MultU64x32(StartClus , 8) + 6;
	MyLog(LOG_ERROR,L"%llx\r\n", StartSec);
	if (DiskRead(disknum, StartSec,2, bitmapbuffer) == FALSE)
		{
			MyLog(LOG_ERROR,L"error read 2 sec\r\n");
			goto Done ;
		}
	
	if(bitmapbuffer[0] == 'F' && bitmapbuffer[1] == 'I' && bitmapbuffer[2] == 'L' && bitmapbuffer[3] == 'E' )
			MyLogA("bitmapbuffer header is FILE \n");
		else
		{
			MyLogA("bitmapbuffer header is NOT FILE \n");
			goto Done ;
		}
		
			Attr_start = *(UINT16*)&bitmapbuffer[0x14];
		MyLogA("Attr_start, %x\n",Attr_start);
		
		for(i= Attr_start;i< 1024 ; )
		{
			if(bitmapbuffer[i] == 0x70 || bitmapbuffer[i] == 0xFF || bitmapbuffer[i] == 0)
				break;
			else
			{
				MyLogA("Attr_[%d]=0x%x\n",i,bitmapbuffer[i]);
				i = i + *(UINT32*)&bitmapbuffer[i+4];
			}
		}
		
		if(bitmapbuffer[i] != 0x70)
		{
			MyLogA("Attr_70 not find\n");
			goto Done ;
		}
		else
			MyLogA("Attr_70  find i = %d\n",i);
		
		A_70_Attr_start = i;
	
	runlist_start = *(UINT16*)&bitmapbuffer[A_70_Attr_start+0x14];
	 runlist_start = runlist_start+A_70_Attr_start;
	bitmapbuffer[runlist_start + 8 +1 +1 ] = 0x01;
	bitmapbuffer[runlist_start + 8 +1 +1 + 1] = 0x0f;
	
	
	
	
	DiskWrite((WORD)disknum, StartSec,2, bitmapbuffer);
	
	Done:
	 
	if(buffer)
		FreePool(buffer);
	if(bitmapbuffer)
		FreePool(bitmapbuffer);
	return ;
}



BOOL UserBreak()
{
	//Auto function does not  support user break.
	if (g_data.bIsAutoFunc)
	{
		return FALSE;
	}

    EFI_INPUT_KEY key;
	EFI_STATUS status;

	status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
	if (status == EFI_SUCCESS) {
		if (key.ScanCode == SCAN_ESC) {
			////// press esc abort
			logflush(FALSE);
			if (g_data.Cmd == USER_COMMAND_BACKUP)
			{
				if (DrawMsgBoxEx(STRING_BACKUP_USER_BREAK, IDCANCELMAIN, IDOKSUB, TYPE_ENQUIRE) == IDOKSUB)
				{
					return TRUE;
				}
			}
			else
			{
				if (DrawMsgBoxEx(STRING_USER_BREAK, IDCANCELMAIN, IDOKSUB, TYPE_ENQUIRE) == IDOKSUB)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL MarkBit(ULONGLONG block,BYTE *bitmap)
{
    ULONGLONG byte=RShiftU64(block,3);//block/8;
    BYTE mask = 1<<((BYTE)block&7);//block%8
    //check capacity of bitmap?
    bitmap[byte] |= mask;
    return TRUE;
}

BOOL ReadBit(UINT64 block,BYTE *bitmap)
{
    ULONGLONG byte=RShiftU64(block,3);//block/8;
    BYTE mask=1<<((BYTE)block&7);//block%8
    
    if (mask & bitmap[byte])
        return TRUE;
    else
        return FALSE;
}

//设置分区数据bitmap位，每位为64个扇区
INTN GetDataToBitmap(UINT64 sector,UINT64 num, VOID *bitmap)
{
    ULONGLONG firstblk;
    ULONGLONG lastblk;

    //在此函数里过滤pagefile及hibernatefile,此时的num一般为一个簇或者8个簇，所以直接判断是否是pagefile或hiberfile
    //就可以了，不用判断交叉的情况
    if (g_dwCurrentUnusedListCount)
    {
        PSIZE_L pList = g_CurrentUnusedList;
        DWORD i;
        //如果文件存在
        for (i=0;i<g_dwCurrentUnusedListCount;i++)
        {
            if (sector >= pList->Begin && sector + num <= pList->Begin + pList->Number)
                return 0;
            pList++;
        }
    }

    firstblk=DivU64x32(sector,BLOCK_SIZE);
    lastblk=DivU64x32((sector+num-1),BLOCK_SIZE);
    while (firstblk<=lastblk) {

        MarkBit(firstblk,bitmap);
        firstblk++;
    }
    //检查是否溢出
    return 0;
}

int HasSystemFile(int partindex)
{
    BYTE buffer[1024];
    UINTN filesize;
    FILE_HANDLE_INFO filehandle;
    FILE_SYS_INFO  sysinfo;
    BOOL bFound = FALSE;

    if (FileSystemInit(&sysinfo,partindex,TRUE)==FALSE)
	{
		MyLog(LOG_DEBUG, L"FileSystemInit return -1.\n");
		return -1;
	}

    if (!FileOpen(&sysinfo,&filehandle,L"\\",EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE))
    	goto completed;

    while (!bFound) {
        filesize=1024;
        if (!FileRead(&filehandle,&filesize,buffer) || !filesize) {
        	break;
        }
        
        //   MyLog(LOG_DEBUG,L"HasSystemFile,found : %s\n", ((EFI_FILE_INFO *)buffer)->FileName);
        	
      //  if( IsExistDir_NTFS_FAT(&sysinfo,L"\\Program Files") == TRUE )
        if ( StrnCmp(((EFI_FILE_INFO *)buffer)->FileName,L"Program Files", 13) == 0 )
     //   if( IsExistFileInDir_NTFS(L"\\",L"Program Files") == TRUE || IsExistFileInDir_NTFS(L"\\",L"Program Files (x86)") == TRUE)
        {
        	  if( (IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\SAM") == TRUE||IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\sam") == TRUE) && (IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\SYSTEM") == TRUE|| IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\system") == TRUE)&&(IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\SECURITY") == TRUE||IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\security") == TRUE  )&& (IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\COMPONENTS") == TRUE||IsExistFileInDir_NTFS_FAT(&sysinfo,L"\\Windows\\System32\\config\\components") == TRUE))
            {
    	          MyLog(LOG_DEBUG,L"found system file windows，partindex:%d\n",partindex);

                bFound = TRUE;    	
            }
        }
        
       // if( IsExistFileInDir_NTFS(L"\\",L"Program Files") == TRUE || IsExistFileInDir_NTFS(L"\\",L"Program Files (x86)") == TRUE)
       // {
        	//  if( IsExistFileInDir_NTFS(L"\\Windows\\System32\\config\\",L"SAM") == TRUE && IsExistFileInDir_NTFS(L"\\Windows\\System32\\config\\",L"SYSTEM") == TRUE &&IsExistFileInDir_NTFS(L"\\Windows\\System32\\config\\",L"SECURITY") == TRUE && IsExistFileInDir_NTFS(L"\\Windows\\System32\\config\\",L"COMPONENTS") == TRUE)
    //        {
          //      bFound = TRUE;    	
    //        }
            //MyLog(LOG_ERROR,L"found windows: %s\n", ((EFI_FILE_INFO *)buffer)->FileName);
      // }
 
        else if (StrnCmp(((EFI_FILE_INFO *)buffer)->FileName,L"hiberfil", 8) == 0)
        {
            MyLog(LOG_ERROR,L"found %s\n", ((EFI_FILE_INFO *)buffer)->FileName);
            bFound = TRUE;
        }
    }
    
completed:
    FileClose(&filehandle);
    FileSystemClose(&sysinfo);
    
    return bFound;
}

//检测系统分区，并将之标记为待备份分区
//检测规则是：
//1、检测是否GPT分区，然后检测是否存在windows分区，如果存在，则认为是
//如果存在windows目录，则认为是系统分区
//2.如果没有找到系统分区，并且检测到存在Bitlocker分区，返回-2
//返回值：
//1： 成功
//0： 未找到系统分区
//-1：未找到系统分区，并且初始化文件系统时出错。
//-2： 没有找到系统分区，并且检测到存在Bitlocker分区。
//-3:  找到多个系统分区。
int ProbeSystemPartition()
{
	DWORD i, systemPartition = 0;
	int diskindex = -1;
	int ret = 0;
	int init_file_sys_err = 0;

	ULONGLONG systemBeginSector = 0;
	CHAR16 LenovoLabel[32] = { L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T' };//ysy
	g_data.BackupDiskNumber = -1;

	IsLinuxSystem = FALSE;
	MyLog(LOG_DEBUG, L"Detecting system partition(%d)\n", g_dwTotalPartitions);

	//get efi ,recovery , reserved partition,and first data partition
	for (i = 0; i < g_dwTotalPartitions; i++) 
	{
		if (g_Partitions[i].bGpt
			&& MyMemCmp(Uefi_System_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0)
		{ 
			//ESP 分区
			g_Partitions[i].bIfBackup = TRUE;
			diskindex = g_Partitions[i].DiskNumber;
			MyLog(LOG_DEBUG, L"Found efi system partition(%d:%d)\n", diskindex, i);
			break;
		}
	}

	int iOSNumber = 0;
	//查找出第一个有windows目录的分区出来
	for (i = 0; i < g_dwTotalPartitions; i++)
	{
		MyLog(LOG_DEBUG, L"Detecting system  %d,DiskNumber:%d\n", i, g_Partitions[i].DiskNumber);

		//if(g_disklist[destdisk].removable)

		if (g_Partitions[i].TotalSectors > 1024 * 2048
			&& g_Partitions[i].PartitionType == NTFS) 
		{
			//	&& (diskindex == -1 || diskindex == g_Partitions[i].DiskNumber)

			//系统分区必然大于1GB
			ret = HasSystemFile(i);
			MyLog(LOG_DEBUG, L"HasSystemFile ret: %d\n", ret);
			if (ret == TRUE)
			{
				g_Partitions[i].bIfBackup = TRUE;
				g_Partitions[i].HasWindows = TRUE;
				systemBeginSector = g_Partitions[i].BootBegin;
				systemPartition = i;
				diskindex = g_Partitions[i].DiskNumber;
				MyLog(LOG_DEBUG, L"Found windows partition(%d:%d)\n", diskindex, i);
				iOSNumber++;
				//		break;
			}
			if (ret < 0) 
			{
				init_file_sys_err++;
			}
		}
	}

	if (iOSNumber > 1)
	{
		MyLog(LOG_DEBUG, L"Found multi os partition.(Number: %d)\n", iOSNumber);
		return -3;
	}

	if (diskindex == -1 || !systemBeginSector) 
	{
		int candidate = -1;
		//可能是linux系统，那么查找出第一个linux大小超过20G的linux分区作为系统分区
		for (i = 0; i < g_dwTotalPartitions; i++)
		{
			if (g_Partitions[i].TotalSectors > 20 * 1024 * 2048
				&& (diskindex == -1 || diskindex == g_Partitions[i].DiskNumber)) 
			{
				if (g_Partitions[i].PartitionType == PARTITION_LINUX) 
				{
					g_Partitions[i].bIfBackup = TRUE;
					systemBeginSector = g_Partitions[i].BootBegin;
					systemPartition = i;
					diskindex = g_Partitions[i].DiskNumber;
					MyLog(LOG_DEBUG, L"Found linux partition(%d:%d)\n", diskindex, i);
					IsLinuxSystem = TRUE;
					break;
				}
				else if (g_Partitions[i].PartitionType == NTFS || g_Partitions[i].PartitionType == FAT32)
				{
					if (candidate == -1)
						candidate = i;
				}
			}
		}

		if (diskindex == -1 || !systemBeginSector)
		{
			//判断是否进行了BitLocker加密
			for (i = 0; i < g_dwTotalPartitions; i++)
			{
				//if(g_Partitions[i].TotalSectors == 0  
				   //   && g_Partitions[i].Handle== 0 ){
				   //   return -2;
				   //   }
				if (g_Partitions[i].PartitionType == SHD_BITLOCKER)
				{
#if OKR_DEBUG
					MyLog(LOG_DEBUG, L"Par %d disk %d BitLockerNum++.... SHD_BITLOCKER\r\n", i, g_Partitions[i].DiskNumber);
#endif
					return -2;
				}

				//
				if (g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
				{
#if OKR_DEBUG
					MyLog(LOG_DEBUG, L"Par %d disk %d BitLockerNum++ SHD_BITLOCKERTOGO....\r\n", i, g_Partitions[i].DiskNumber);
#endif
					return -2;
				}
			}
		}

		if (!systemBeginSector && candidate != -1) 
		{
			/*
						i = (DWORD)candidate;
						g_Partitions[i].bIfBackup = TRUE;
						systemBeginSector = g_Partitions[i].BootBegin;
						systemPartition = i;
						diskindex = g_Partitions[i].DiskNumber;
			*/
			diskindex = -1;
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"Assume first partition as system(%d:%d)\n", diskindex, i);
#endif
		}
	}

	if (diskindex == -1)
	{
		MyLog(LOG_DEBUG, L"No Operation System detected.\n");

		if (init_file_sys_err > 0)
		{
			return -1;
		}
		//return FALSE;
		return 0;
	}
	MyLog(LOG_DEBUG, L"System disk: %d.\n", diskindex);

	g_data.BackupDiskNumber = diskindex;

	//将系统分区之前的小分区都标记为需要备份

	for (i = 0; i < g_dwTotalPartitions; i++)
	{
		if (g_data.BackupDiskNumber != g_Partitions[i].DiskNumber)
			continue;
		if (g_Partitions[i].TotalSectors < 1 * 1024 * 2048 && g_Partitions[i].BootBegin < systemBeginSector)	//小于1GB的隐藏分区需要备份
			g_Partitions[i].bIfBackup = TRUE;
	}

	//将系统分区活动分区和ESP分区都标记为需要备份
	for (i = 0; i < g_dwTotalPartitions; i++)
	{
		if (g_data.BackupDiskNumber != g_Partitions[i].DiskNumber)
			continue;
		if (g_Partitions[i].bBootPartition == TRUE || (g_Partitions[i].bGpt && (MyMemCmp(Uefi_ESP_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0)))	//小于1GB的隐藏分区需要备份
			g_Partitions[i].bIfBackup = TRUE;
	}

	for (i = 0; i < g_dwTotalPartitions; i++)
	{
		if (g_data.BackupDiskNumber != g_Partitions[i].DiskNumber)
			continue;
		//if(( g_Partitions[i].bGpt &&  (CompareGuid(&PARTITION_LINUX_SWAP_GUID,g_Partitions[i].TypeGuid)) ))	//swap分区需要备份
		//	g_Partitions[i].bIfBackup = TRUE;

		if ((g_Partitions[i].bGpt && (CompareGuid(&PARTITION_LINUX_BOOT_GUID, (EFI_GUID*)g_Partitions[i].TypeGuid))))	//legacy boot 分区需要备份
			g_Partitions[i].bIfBackup = TRUE;
	}

	//
		//比较分区GUID，MS RESERVED PARTITION不需要备份
	for (i = 0; i < g_dwTotalPartitions; i++) 
	{
		GetVolumeName(i);
		if (diskindex != g_Partitions[i].DiskNumber)
			continue;

		if (MyMemCmp(Window_Recovery_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0) {

			//if(i < systemPartition)
			if (MyMemCmp((char*)g_Partitions[i].Label, (char*)LenovoLabel, 22) != 0)//ysy  1210
				g_Partitions[i].bIfBackup = TRUE;

			MyLog(LOG_DEBUG, L"Found MS Recovery Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);

		}
		else if (MyMemCmp(Window_Reserved_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0) {

			g_Partitions[i].bIfBackup = FALSE;
			MyLog(LOG_DEBUG, L"Found MS Reserved Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);

		}
		else if (MyMemCmp(Windows_Data_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0 && g_Partitions[i].HasWindows != TRUE) {


			g_Partitions[i].bIfBackup = FALSE;

			MyLog(LOG_DEBUG, L"Found MS Data Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);

		}
		else {
			MyLog(LOG_DEBUG, L"i:%x:bBackup %x,Type:%x,Id:%x,Label:%s\n", i, g_Partitions[i].bIfBackup, g_Partitions[i].PartitionType, g_Partitions[i].PartitionId, g_Partitions[i].Label);
		}
	}

	for (i = 0; i < g_dwTotalPartitions; i++)
	{
		if (MyMemCmp(Windows_Data_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0 && g_Partitions[i].HasWindows != TRUE)
			MyLog(LOG_DEBUG, L"Found zwang MS Data Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);
	}

	MyLog(LOG_DEBUG, L"End ProbeSystemPartition\n");
	//return TRUE;
	return 1;
}



//return bytes of backup data
UINT64 GetBackupTotaldata()
{
    UINT64 totaldata=0;
    DWORD i;
    
    for (i=0;i<g_dwTotalPartitions;i++) {
        if (g_Partitions[i].bIfBackup!=TRUE)
            continue;
            
        //skip pagefile swapfile... size
        totaldata += MultU64x32(g_Partitions[i].TotalSectors - g_Partitions[i].FreeSectors, 512);
    }
    
    totaldata -= MultU64x32(GetAllUnusedDataSize(), 512);
    
//MyLog(LOG_ERROR,L"totaldata 0x%x, GetAllUnusedDataSize() 0x%x\n", 
//	totaldata, GetAllUnusedDataSize());

    return totaldata;
}

//
DWORD g_show_total = 0;	//MB
DWORD g_shown_unit = 0;	//MB
DWORD g_shown_count = 0;	//KB
DWORD g_show_interval = 0;
DWORD g_show_percent = 0;
PCONTROL g_showctrl = NULL;

DWORD pre_show_percent = 0;

DWORD g_update_backup = 1;

int ProcessBarInit(PCONTROL pctl, UINT64 showtotal)
{
//	g_show_total = DivU64x32(showtotal, 1024*1024);	//MB
	g_show_total = ((showtotal/1024)/1024);	//MB

	g_show_interval = g_show_total / 100;
	g_shown_unit = 0;
	g_shown_count = 0;
	g_show_percent = 0;
	g_showctrl = pctl;
	g_data.BeginTime = GetTickCount()/1000;
	g_data.LastTime = 0;
	g_data.LastRemain = 0xffffffff;
	
	pre_show_percent = 0;
	g_update_backup = 1;
	
//MyLog(LOG_ERROR,L"ProBarInit total 0x%x,0x%x, 0x%x \n", 
//	showtotal, g_show_total, g_show_interval);
	if (pctl)
	{
		struct DM_MESSAGE dmessage;
		dmessage.message=WM_PROCESS_RUN;
    	dmessage.thex = 0;
    	pctl->sendmessage(pctl, &dmessage);
  }
    
	return 0;
}

void UpdateRemainTime(DWORD remain)
{
	if (remain < 0)
	{
		remain = 0;
	}

	CHAR16 text[64] = { 0 };

	SPrint(text, 64, STRING_REMAIN_TIME L"  %02d:%02d", remain / 60, remain % 60);
	MyLog(LOG_ERROR, L"UI remain: %d  %02d:%02d", remain, remain / 60, remain % 60);

	DWORD y;

	float fMinscale = g_data.fMinscale;

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	if (1 == g_update_backup)
	{
		dwWidth = (WHITE_BG_WIDTH - WHITE_BG_CORNER_SIZE) * fMinscale - 20;
		dwLeft = (g_WindowWidth - dwWidth) / 2;

		dwTop = REC_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
		dwHeight = BK_PAGE_REAMIN_TIME_VALUE_HEIGHT * fMinscale + 10;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (REC_PAGE_REAMIN_TIME_TEXT_TOP - 150) * fMinscale;
		}
		FillRect(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

		g_update_backup = 0;
	}

	//dwWidth = (WHITE_BG_WIDTH - WHITE_BG_CORNER_SIZE) * fMinscale - 20;	

	dwTop = REC_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
	dwHeight = BK_PAGE_REAMIN_TIME_VALUE_HEIGHT * fMinscale + 5;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_REAMIN_TIME_TEXT_TOP - 150) * fMinscale;
	}
	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(40); //(int)(40 * g_data.fFontScale);

	dwWidth = 0;
	GetStringLen(text, &stStringInfo, &dwWidth);
	dwLeft = (g_WindowWidth - dwWidth - 4) / 2;

	FillRect(dwLeft - 100, dwTop, dwWidth + 200, dwHeight, 0xFFFFFF);

	//dwLeft = (g_WindowWidth - dwWidth) / 2;
	//dwLeft = (g_WindowWidth - (BK_PAGE_REAMIN_TIME_TEXT_WIDTH + BK_PAGE_REAMIN_TIME_VALUE_WIDTH + BK_PAGE_REAMIN_TIME_TEXT_VALUE_SPAN) * fMinscale) / 2;
	dwTop = BK_PAGE_REAMIN_TIME_VALUE_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_REAMIN_TIME_VALUE_TOP - 150) * fMinscale;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, text, &stStringInfo, TRUE);
}

void UpdateComma(DWORD dotNum)
{
	//if (g_update_backup)
	//{
	//	//MyLog(LOG_DEBUG, L"g_update_backup %d\n", g_update_backup);
	//	return;
	//}

	CHAR16 text[8] = { 0 };

	for (DWORD i = 0; i < dotNum; i++)
	{
		text[i] = L'.';
	}

	float fMinscale = g_data.fMinscale;

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTZHJW;
	stStringInfo.iFontSize = CalcFontSize(90); 

	dwWidth = 0;
	GetStringLen(STRING_BACKUP_SYSTEM, &stStringInfo, &dwWidth);
	dwLeft = (g_WindowWidth - dwWidth) / 2;
	dwTop = BK_PAGE_SUB_TITLE_TOP * fMinscale;
		
	dwLeft = (g_WindowWidth - dwWidth) / 2 + dwWidth + 5;
	//MyLog(LOG_DEBUG, L"dwLeft %d\n", dwLeft);
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_SUB_TITLE_TOP - 150) * fMinscale;
	}
	FillRect(dwLeft, dwTop, stStringInfo.iFontSize * 3, stStringInfo.iFontSize, 0xFFFFFF);

	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, text, &stStringInfo, TRUE);
}

//为了方便调用者showcount是字节
int GoProcessBar(DWORD showcount)
{	
	if(g_showctrl)
	{
		//MyLog(LOG_DEBUG, L"g_showctrl HAS\n");
		DWORD percent = 0;

		if (showcount == -1)
		{
			//MyLog(LOG_DEBUG, L"showcount == -1 \n");
			percent = 100;
		}
		else
		{
			g_shown_count += showcount / 1024;	//KB
			//MyLog(LOG_DEBUG, L"g_shown_count =%lld , g_show_interval:%lld\n", g_shown_count / 1024, g_show_interval);
			if (g_shown_count / 1024 >= g_show_interval)
			{
				g_shown_unit += g_shown_count / 1024;

				MyLog(LOG_DEBUG, L"g_shown_unit %d:%d\n", g_shown_unit, g_shown_count);
				MyLog(LOG_DEBUG, L"g_show_interval %d:%d\n", g_show_interval, g_show_total);

				//g_shown_count = 0;
				//percent = (WORD)(((DWORD)g_shown_unit * 100) / (DWORD)g_show_total);
				percent = (WORD)(((DWORD)g_shown_unit * 100) / (DWORD)(g_show_interval * 100));

				if (percent > 0 && percent > g_show_percent)
				{
					g_shown_count = 0;
				}
				MyLog(LOG_DEBUG, L"percent %d:%d\n", percent, g_show_percent);

			}
		}    	
    	
    	if(percent > g_show_percent)
    	{
			//MyLog(LOG_DEBUG, L"percent %d g_show_percent %d\n", percent, g_show_percent);
		    struct DM_MESSAGE dmessage;
		    
		    if(percent > 100)
		    	percent = 100;

			g_show_percent = percent;
   			dmessage.message = WM_PROCESS_RUN;
   			dmessage.thex = percent;
   			g_showctrl->sendmessage(g_showctrl, &dmessage);

   			
   	//		//刷新标题条中的百分号
   	//		{
			 //   DWORD x, y, d[3], i;
   	//			DWORD digit[10] = {
   	//				IMG_FILE_0, IMG_FILE_1, IMG_FILE_2, IMG_FILE_3, IMG_FILE_4, 
   	//				IMG_FILE_5, IMG_FILE_6, IMG_FILE_7, IMG_FILE_8, IMG_FILE_9};
   	//			x = g_x_begin + 430;
		  //  	y = (g_WindowHeight / 3 - 60) /2 + 180;
				//if (g_WindowHeight <= 800)
				//{
				//	y -= 80;
				//}
				//
				//d[0] = percent/100;
		  //  	d[1] = (percent%100) / 10;
		  //  	d[2] = (percent%10);
		  //  	if(percent >= 100) i = 0;
		  //  	else if(percent >= 10) i = 1;
	   // 		else i = 2;
	   // 			
    //			DrawLine(x, y, 200, 60, COLOR_CLIENT);
		  //  	for(;i<3;i++)
		  //  	{
				//	DisplayImg(x, y, digit[d[i]], FALSE);
				//	x += 40;
   	//			}
				//DisplayImg(x, y, IMG_FILE_PERCENT, FALSE);
   	//		}
    	}
#ifdef SHOW_TIME    	
    	{
    		DWORD curtime = 0, remain = 0;
    		curtime = GetTickCount()/1000 - g_data.BeginTime;

    		if(curtime > g_data.LastTime)
    		{
				//MyLog(LOG_ERROR, L"curtime: %d  %02d:%02d", curtime, curtime / 60, curtime % 60);
				//MyLog(LOG_ERROR, L"g_data.LastTime: %d  %02d:%02d", g_data.LastTime, g_data.LastTime / 60, g_data.LastTime % 60);

				//SPrint(text, 64, L"%02d:%02d", curtime/60, curtime%60);
				//FillRect(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);
			//	DisplayString(dwLeft, dwTop, COLOR_TEXT_CONTENT, text);
				UpdateComma(curtime % 3 + 1);

				if (!percent)
				{
					percent = g_show_percent;
				}
				if (percent > 0)
				{
					remain = curtime * (100 - percent) / percent;
					//remain = curtime * (g_show_total - g_shown_unit) / g_shown_unit;

					if (1 == percent)
					{
						if (remain < 120)
						{
							///Calculation the least remain value.
							///Generally, 20 GB data / 5 minutes.
							int iLeastRemain = 300 * g_show_total / (20 * 1024);

							if (iLeastRemain > remain)
							{
								MyLog(LOG_DEBUG, L"remain(%d) may be too small, adjust it.(%d)", remain, iLeastRemain);

								remain = iLeastRemain;
							}
						}
					}

					if (percent > pre_show_percent)
					{
						pre_show_percent = percent;

						MyLog(LOG_ERROR, L"curtime: %d  %02d:%02d", curtime, curtime / 60, curtime % 60);
						MyLog(LOG_ERROR, L"remain: %d  %02d:%02d", remain, remain / 60, remain % 60);
						MyLog(LOG_ERROR, L"g_data.LastRemain: %d ", g_data.LastRemain);
						
						if (remain < g_data.LastRemain)
						{
							//防止进度跳动过大
							if (g_data.LastRemain != 0xffffffff)
							{
								if (percent < 100)
								{
									//进度均匀
									int iIntevalValue = g_data.LastRemain / (100 - percent);

									if (iIntevalValue > 0)
									{
										if (g_data.LastRemain > remain + iIntevalValue)
										{
											remain = g_data.LastRemain - iIntevalValue;
										}
									}
								}

								/////如果上次计算的数据比本次数据大20s，本次数据 = 上次数据 - 20s
 								//if (g_data.LastRemain > remain + 20)
								//{
								//	remain = g_data.LastRemain - 20;
								//}
							}

							UpdateRemainTime(remain);
							g_data.LastRemain = remain;
						}
						else
						{
							MyLog(LOG_ERROR, L"Skip remain (%d) > g_data.LastRemain (%d)", remain, g_data.LastRemain);
							
							//防止进度长时间不动
							if (percent < 100)
							{
								int iInteval = g_data.LastRemain / (100 - percent);
								iInteval = iInteval < 2 ? iInteval : 2;

								if (iInteval > 0 && g_data.LastRemain - iInteval > 0)
								{
									g_data.LastRemain -= iInteval;
									UpdateRemainTime(g_data.LastRemain);
								}
							}
						}
					}
				}
	   	    
    			g_data.LastTime = curtime;
    		}

			if (percent >= 100 && g_show_percent >= 100)
			{
				UpdateRemainTime(0);
			}
   	    }
#endif   	    
    }
	else
	{
		//MyLog(LOG_DEBUG, L"g_showctrl is NULL\n");
	}
    return 0;
}

int DestroyProcessBar()
{
	 g_show_total = 0;	//MB
   g_shown_unit = 0;	//MB
   g_shown_count = 0;	//KB
   g_show_interval = 0;
   g_show_percent = 0;
   
   
  g_show_total = 0;	//MB
	g_show_interval = 0;
	g_shown_unit = 0;
	g_shown_count = 0;
	g_show_percent = 0;
	g_showctrl = NULL;


  pre_show_percent = 0xffff;
  g_update_backup = 1;
//  g_data.LastTime = 0xFFFF;
	
	return 0;
}

////为了方便调用者showcount是字节
//int MyShowGoProcessBar(DWORD  showcount)
//{
//	if(g_showctrl)
//	{
//	    DWORD percent = 0;
//	    
//		if(percent == -1)
//		{
//			percent = 100;
//		}
//    else
//		{
//    		g_shown_count += showcount/1024;	//KB
//    		if(g_shown_count/1024 >= g_show_interval)
//    		{
//    			g_shown_unit += g_shown_count/1024;
//    			g_shown_count = 0;
//    			percent = (WORD)(((DWORD)g_shown_unit*100)/(DWORD)g_show_total);
//    		}
//    	}	
//    	if(percent > g_show_percent)
//    	{
//		    struct DM_MESSAGE dmessage;
//		    
//		    if(percent > 100)
//		    	percent = 100;
//   			dmessage.message = WM_PROCESS_RUN;
//   			dmessage.thex = percent;
//   			g_showctrl->sendmessage(g_showctrl, &dmessage);
//   			g_show_percent = percent;
//   			
//   	//		//刷新标题条中的百分号
//   	//		{
//			 //   WORD x, y, d[3], i;
//   	//			WORD digit[10] = {
//   	//				IMG_FILE_0, IMG_FILE_1, IMG_FILE_2, IMG_FILE_3, IMG_FILE_4, 
//   	//				IMG_FILE_5, IMG_FILE_6, IMG_FILE_7, IMG_FILE_8, IMG_FILE_9};
//   	//			x = g_x_begin + 320;
//		  //  	y = (g_WindowHeight / 3 - 60) /2 + 8;
//		  //  	d[0] = percent/100;
//		  //  	d[1] = (percent%100) / 10;
//		  //  	d[2] = (percent%10);
//		  //  	if(percent >= 100) i = 0;
//		  //  	else if(percent >= 10) i = 1;
//	   // 		else i = 2;
//	   // 			
//    //			DrawLine(x, y, 200, 60, COLOR_TITLE_L_BLUE);
//		  //  	for(;i<3;i++)
//		  //  	{
//				//	DisplayImg(x, y, digit[d[i]], FALSE);
//				//	x += 32;
//   	//			}
//				//DisplayImg(x, y, IMG_FILE_PERCENT, FALSE);
//   	//		}
//   			
//   			return 0;
//    	}
//#ifdef SHOW_TIME    	
//    	{
//    		DWORD curtime, remain;
//    		curtime = GetTickCount()/1000 - g_data.BeginTime;
//    		if(curtime > g_data.LastTime)
//    		{
//	    		CHAR16 text[64] = { 0 };
//    			SPrint(text, 64, L"%02d:%02d", curtime/60, curtime%60);
//    			FillRect(g_x_begin+320, g_y_begin+200, 40, 18, COLOR_CLIENT);
//// 	    		DisplayString(g_x_begin+320, g_y_begin+200,COLOR_BLACK,text);
// 	    		DisplayString(g_x_begin+320, g_y_begin+200,COLOR_TEXT,text);
//   	    		
//    			if(!percent)
//    				percent = g_show_percent;
//    			if(percent)
//    			{
//    				remain = curtime * (100 - percent) / percent;
//    				if(remain < g_data.LastRemain)
//    				{
//    					SPrint(text, 64, L"%02d:%02d", remain/60, remain%60);
//    					FillRect(g_x_begin+320, g_y_begin+218, 40, 18, COLOR_CLIENT);
////   	    			DisplayString(g_x_begin+320, g_y_begin+218, COLOR_BLACK,text);
//   	    			DisplayString(g_x_begin+320, g_y_begin+218, COLOR_TEXT,text);
//	   	    			g_data.LastRemain = remain;
//	   	    		}
//	   	    	}
//    			g_data.LastTime = curtime;
//    		}
//   	    }
//#endif   	    
//    }
//    return 0;
//}
//分BLOCK压缩
VOID CompressBuffer(PBUFFER_INFO buffinfo)
{
	  lZ4_BUFFER_HEAD *p_lz4_buf_head = (lZ4_BUFFER_HEAD *)g_lz4_buffer;
	  
    if (buffinfo->usedsize) {
	//	g_lz4_size = LZ4_compress(buffinfo->pointer, g_lz4_buffer+8, (int)buffinfo->usedsize);
		g_lz4_size = LZ4_compress(buffinfo->pointer, g_lz4_buffer+sizeof(lZ4_BUFFER_HEAD), (int)buffinfo->usedsize);
		if (g_lz4_size == 0) {
			g_lz4_size = -1;
			SET_ERROR(__LINE__, OKR_ERROR_COMPRESS);
			return ;
		}
		if (g_lz4_size > (INTN)buffinfo->usedsize) {// compress larger than original data
			CopyMem(g_lz4_buffer+sizeof(lZ4_BUFFER_HEAD),buffinfo->pointer,buffinfo->usedsize);
			g_lz4_size = buffinfo->usedsize;
			//*(DWORD *)g_lz4_buffer = UNCOMPRESS_MAGIC;	//no cmopress sig
      p_lz4_buf_head->compress_flag = UNCOMPRESS_MAGIC;
      
		} else {
		 //	*(DWORD *)g_lz4_buffer = COMPRESS_MAGIC;
		  
		 p_lz4_buf_head->compress_flag = COMPRESS_MAGIC;

		}
	//	*(DWORD *)(g_lz4_buffer+4) = (DWORD)g_lz4_size;

//	  g_lz4_size += 8;
	  p_lz4_buf_head->lz4_size = (DWORD)g_lz4_size;
	  g_lz4_size += sizeof(lZ4_BUFFER_HEAD);
    //4k对齐后的数据大小
    g_lz4_size = CalcAliginSize(g_lz4_size);
    
    if( (DWORD)g_lz4_size > (LZ4_COMPRESSBOUND(g_data.CompressBlockSize)+sizeof(lZ4_BUFFER_HEAD)+ALGIN_SIZE_BYTE) )
    {
 			g_lz4_size = -1;
			MyLog(LOG_ERROR, L"CompressBuffer mem align error\n");
			return ;   	
    }
	  //*(DWORD *)(g_lz4_buffer+8) = (DWORD)g_lz4_size;
 		
 		MyLog(LOG_DEBUG, L"CompressBuffer g_lz4_size:%x\n",g_lz4_size);

	  p_lz4_buf_head->align_lz4_size = (DWORD)g_lz4_size;

		
	} else {
		g_lz4_size = 0;
	}
    buffinfo->datavalid = FALSE;	//数据压缩完毕，压缩数据保存在g_lz4_buffer里
    return;
}

VOID DiskReadThread(struct BUFFERARRAY *bufferarray)
{
	struct OKR_HEADER *pheader;
    struct CONTINUE_RWDISK_INFO conti_info;
    PBUFFER_INFO buffinfo;
    UINT64 totaloriginsize=0;
    WORD i;
    DWORD sequence = 0;
    
    pheader = g_data.fileheader;
    Clean_BufferArray(bufferarray);
    
    for (i=0;i<g_dwTotalPartitions;i++) {
        VOID *bitbuffer;
        INTN   size;
        ULONGLONG blocks,currentblock;
        ULONGLONG beginsec;
        ULONGLONG readn;
		int ret;
        
        if (!g_Partitions[i].bIfBackup)
            continue;
        //scan disk
        blocks = DivU64x32((g_Partitions[i].TotalSectors+(BLOCK_SIZE-1)),BLOCK_SIZE);
        size = DivU64x32((blocks+7),8);
        
        //获得该分区的pagefile以及hibernate的扇区列表, 该函数第二次执行时就是简单的获取第一次的结果
        GetUnusedDataList(i);

		//等待上一个分区的bitbuffer被保存
		while(g_data.partition && g_data.partition_write != g_data.partition) {
			if(g_data.bCompleted)
				break;
			CpuPause();
		}
		if(g_data.bCompleted)
			break;
		
		bitbuffer = g_data.databitmap;
        SetMem(bitbuffer,size,0);
		
        ret = ScanPartitionData(i, GetDataToBitmap, bitbuffer, (PBYTE)&pheader->Magic);
		if(ret < 0) {
            pheader->Partition[i].BackupDataSize = 0;
			g_Partitions[i].BitmapValid = 0xff;
			MyLog(LOG_DEBUG, L"ScanPartitionData(%d) failed.\n", i);
            continue;
        }
		g_Partitions[i].BitmapValid = TRUE;

		//g_data.threadstate = 4;
        beginsec = g_Partitions[i].BootBegin;
        g_data.partition = &g_Partitions[i];
        ///////////////////////////////////////////////////////////////////

		SmartDiskIoInit((BYTE)g_data.BackupDiskNumber, COMMAND_READ, &conti_info);
        currentblock = 0;
        while (currentblock < blocks) {

            buffinfo = GetProducter(bufferarray);
			if(!buffinfo)
				break;

            readn = 0;
            while (currentblock < blocks) {
                if (ReadBit(currentblock, bitbuffer)) {
                    SmartDiskIo(&conti_info,MultU64x32(currentblock, BLOCK_SIZE)+beginsec,BLOCK_SIZE,buffinfo->pointer+readn);
                    readn += BLOCK_SIZE*512;

                    if (readn + BLOCK_SIZE*512 > buffinfo->totalsize) {
                        currentblock++;
                        break;
                    }
                }
                currentblock++;
            }
            SmartDiskIo(&conti_info, 0, 0, NULL);

            if( readn ) {
	            buffinfo->srcsize = readn;//maybe 0
    	        totaloriginsize += readn;
            	buffinfo->usedsize = readn;
            	CompressBuffer(buffinfo);
				if(g_lz4_size < 0) {
					//出错了
					SET_ERROR(__LINE__, OKR_ERROR_COMPRESS);
            		goto Done;
				}
            	CopyMem(buffinfo->pointer, g_lz4_buffer, g_lz4_size);
            	buffinfo->usedsize = g_lz4_size;
            	buffinfo->sequence = sequence;
            	sequence++;
            }
			buffinfo->datavalid = TRUE;	//数据有效
        }
    }
    
    g_data.fileheader->OriginalSize = totaloriginsize;

Done:
	g_data.bCompleted = TRUE;
	MyLog(LOG_DEBUG,L"DiskReadThread competed, totalsize=0x%x\n", totaloriginsize);

	return;    
}

//异步IO
int BackupWithAsyncIo(FILE_HANDLE_INFO *filehandle, OKR_HEADER *pheader, 
		struct BUFFERARRAY *bufferarray, struct MP_INFO *mpinfo)
{
    PBUFFER_INFO bufferinfo;
    WORD i = 0;
    int ret=-1;
    UINT64 filetotalsize = 0,after_alignlen = 0;	//记录整个文件的大小
	VOID *bitbuffer = NULL;
	ULONGLONG blocks = 0;
    INTN size = 0;
    
    bitbuffer = g_data.databitmap;

	//先清除分区数据bitmap有效标志位
	for (i=0;i<g_dwTotalPartitions;i++) {
		g_Partitions[i].BitmapValid = FALSE;
	}
    
	StartAP(mpinfo, DiskReadThread, bufferarray);

    for (i=0;i<g_dwTotalPartitions;i++) {
        ULONGLONG totalwrite = 0, total = 0, currentblock = 0;
        
        if (!g_Partitions[i].bIfBackup)
            continue;
        
        blocks = DivU64x32((g_Partitions[i].TotalSectors+(BLOCK_SIZE-1)),BLOCK_SIZE);
        size = DivU64x32((blocks+7),8);
        
        while(!g_Partitions[i].BitmapValid) {
			if(g_data.bCompleted)
				goto Done;
        	CpuPause();		//等待bitbuffer生效
        }
		if(g_Partitions[i].BitmapValid == 0xff || g_Partitions + i != g_data.partition)	{
			//有可能中间有分区无效，需要跳过
			MyLog(LOG_DEBUG, L"Invalid partition %d, bitmap 0x%x\n", i, g_Partitions[i].BitmapValid);
			pheader->Partition[i].bIfBackup = FALSE;
        	continue;
		}
        
        total = 0;
		currentblock = 0;
        for (currentblock=0;currentblock<blocks;currentblock++) {
            if (ReadBit(currentblock, bitbuffer))
                total += BLOCK_SIZE*512;
        }
        //WRITE bitmap
    //    if (!FileWrite(filehandle, size, g_data.databitmap)) {
		//	SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
    //        goto Done;
    //    }
        
        after_alignlen = CalcAliginSize(size);
        //计算对齐后大小是否大于原buffer的大小
		    if(after_alignlen > g_databitmapbufsize)
		    {
		    	  MyLog(LOG_ERROR,L"BackupAsyncIowrite,beyond align buffer\n");
            goto Done;
		    }	
		    size = after_alignlen;
        if (!FileWrite(filehandle,size,g_data.databitmap)) {
            //MyLog(LOG_ERROR,L"write bitmap error\n");
            goto Done;
        }
        g_data.partition_write = g_Partitions + i;	//当前正在备份该分区，此时bitbuffer已经保存

		totalwrite = size;
        filetotalsize += size;
        ///////////////////////////////////////////////////////////////////
		//压缩和读取数据放在diskreadthread线程里，本线程只处理保存文件
//#if OKR_DEBUG
//		MyLog(LOG_ERROR,L"backup partition %d, total 0x%x, state %d, blocks 0x%x\n", 
//			i, total, g_data.threadstate, blocks);
//#endif
        while (total) {
		    
            if((!g_data.bIsAutoFunc && UserBreak()) || g_lz4_size < 0)
                goto Done;

            bufferinfo = GetConsumer(bufferarray);
            if(!bufferinfo)
				goto Done;
//#if OKR_DEBUG
//            MyLog(LOG_ERROR,L"GetConsumer, seqence %d, g_lz4_size 0x%x, size 0x%x, src 0x%x, total 0x%x, totalbackupsize 0x%x\n",
//            	bufferinfo->sequence, g_lz4_size, bufferinfo->usedsize, bufferinfo->srcsize, total, g_data.TotalBackupSize);
//            MyLog(LOG_ERROR,L"buffer[0].seqence %d, ready %d, buffer[1].seqence %d, ready %d\n",
//            	bufferarray->buffer[0].sequence, bufferarray->buffer[0].ready, 
//            	bufferarray->buffer[1].sequence, bufferarray->buffer[1].ready);
//#endif
            if (bufferinfo->usedsize > 0) {
                if (!FileWrite(filehandle, bufferinfo->usedsize, bufferinfo->pointer)) {
					SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
                    goto Done;
                }
                totalwrite += bufferinfo->usedsize;
                filetotalsize += bufferinfo->usedsize;
                
                if(total > bufferinfo->srcsize)
                	total -= bufferinfo->srcsize;
                else
                	total = 0;
                
	            if(bufferinfo->srcsize > bufferinfo->usedsize)
    	        	GoProcessBar(bufferinfo->srcsize - bufferinfo->usedsize);
            }

			bufferinfo->datavalid = FALSE;	//数据使用完毕，清除有效标记
        }

        ///////////////////////////////////////////////////////////////////////////////
        pheader->Partition[i].BackupDataSize = totalwrite;
    }
    FileClose(filehandle);
    
	//UPDATE HEADER
    pheader->bComplete = TRUE;
    pheader->FileSize = filetotalsize;
    //pheader->OriginalSize = totaloriginsize;	//在diskreadthread里设置
    pheader->CompressBlockSize = g_data.CompressBlockSize;
    
    ret = 0;
    
    BackupLastProcess();

    GoProcessBar(-1);

	//delayEx(1000);

Done:
	g_data.bCompleted = TRUE;
    WaitToComplete(mpinfo);
	
	  DestroyProcessBar();
	  
    MyLog(LOG_DEBUG,L"Backup Compete, totalsize=0x%x\n", filetotalsize);

//	MyLog(LOG_ERROR,L"pause, partition %d, state %d, g_lz4_size 0x%x, readpartition %d\n", 
//		i, g_data.threadstate, g_lz4_size, g_data.readpartition);
//	MyLog(LOG_ERROR,L"g_data.partition %x, g_data.partition_write %x, g_dwTotalPartitions %d\n",
//    	g_data.partition, g_data.partition_write, g_dwTotalPartitions);
//	for (i=0;i<g_dwTotalPartitions;i++) {
//        if (!g_Partitions[i].bIfBackup)
//            continue;
//    	MyLog(LOG_ERROR,L"backup %d, pid %d, total 0x%x, bIfBackup %d, backup 0x%x\n", 
//    		i, g_Partitions[i].PartitionType, g_Partitions[i].TotalSectors, 
//    		g_Partitions[i].bIfBackup, g_Partitions[i].BackupDataSize);
//    }
//getch();
    return ret;
}

BOOL InitGlobalDataForBackup(OKR_HEADER *pheader)
{
	ULONGLONG blocks;
    INTN size, maxsize = 0, maxscansize = SCANBUFFER_SIZE, maxgroupdesc = 0;
    WORD i;
    
	//分配bitbuffer, 按最大的分区容量来计算bitbuffer的大小
	for (i=0;i<pheader->PartitionCount;i++) {
		if(!pheader->Partition[i].bIfBackup)
            continue;

		MyLog(LOG_DEBUG, L"Backup %d, PartitionType %d, TotalSize 0x%x\n", 
			i, pheader->Partition[i].PartitionType, pheader->Partition[i].TotalSectors);
//getch();

        blocks = DivU64x32((pheader->Partition[i].TotalSectors+(BLOCK_SIZE-1)),BLOCK_SIZE);
        size = DivU64x32((blocks+7),8);
        if(size > maxsize)
        	maxsize = size;
        	
       	if(pheader->Partition[i].PartitionType == PARTITION_LINUX) {
       		
			DWORD blocksize = pheader->Partition[i].SectorsPerClr * SECTOR_SIZE;
			size = (((pheader->Partition[i].blocks_per_group + 7)/8 + blocksize - 1) & ~(blocksize - 1));
	        if(size > maxscansize)
    	    	maxscansize = size;
    	    	
			size = pheader->Partition[i].groups_count * sizeof(EXT3_GROUP_DESC);
			size = ((size + 0xfff) & ~0xfff);
	        if(size > maxgroupdesc)
    	    	maxgroupdesc = size;
		}
	}
	
MyLog(LOG_DEBUG, L"alloc g_lz4_buffer 0x%x, 0x%x.\n", g_data.CompressBlockSize, LZ4_COMPRESSBOUND(g_data.CompressBlockSize)+8);
//getch();
	if(!g_lz4_buffer)
	{
	//	g_lz4_buffer = myalloc(LZ4_COMPRESSBOUND(g_data.CompressBlockSize)+8);
		
		g_lz4_buffer = myalloc(LZ4_COMPRESSBOUND(g_data.CompressBlockSize)+sizeof(lZ4_BUFFER_HEAD)+ALGIN_SIZE_BYTE);
		if (!g_lz4_buffer)
			return FALSE;
	}

MyLog(LOG_DEBUG, L"alloc bitmap 0x%x.\n", maxsize);
//getch();
	//if(!g_data.databitmap) 
	{
		//预计多分配4k数据，buffer按buffer大小能够满足内存对齐
		g_data.databitmap = myalloc(maxsize+ALGIN_SIZE_BYTE);
		if(!g_data.databitmap)
			return FALSE;
	}
	g_databitmapbufsize = maxsize+ALGIN_SIZE_BYTE;
	
	g_data.bCompleted = FALSE;
	g_data.partition = NULL;
	g_data.partition_write = NULL;
	g_data.fileheader = pheader;
MyLog(LOG_DEBUG, L"alloc scanbuffer 0x%x.\n", maxscansize);
//getch();
	//if(!g_data.scanbuffer) 
	{
		g_data.scanbuffersize = maxscansize;
		g_data.scanbuffer = myalloc(maxscansize);
		if(!g_data.scanbuffer)
			return FALSE;
	}
	
	if(maxgroupdesc) 	// && !g_data.groupdesc
	{
MyLog(LOG_DEBUG, L"alloc groupdesc 0x%x.\n", maxgroupdesc);
//getch();
		g_data.groupdescsize = maxgroupdesc;
		g_data.groupdesc = myalloc(maxgroupdesc);
		if(!g_data.groupdesc)
			return FALSE;
	}
	
	//g_data.currentsize = 0;
	return TRUE;
}

BOOL IsGptSystem(WORD DiskNumber)
{
	DWORD i;

	for (i=0;i<g_dwTotalPartitions;i++) {
		if(g_Partitions[i].DiskNumber == DiskNumber) {
			if(g_Partitions[i].bGpt)
				return TRUE;
			else
				return FALSE;
		}
	}
	return FALSE;
}


void AddAllHddSn(CHAR8 *Buffer)
{
	UINT8 i = 0;
	UINT8 totaldisknum= 0;
	CHAR8 *TempBuffer = Buffer;
	 
	for(i=0;i<MAX_DISK_NUM;i++)
	{
		if(g_disklist[i].disksn[0]!=0 || g_disklist[i].disksn[4]!=0|| g_disklist[i].disksn[8]!=0|| g_disklist[i].disksn[12]!=0|| g_disklist[i].disksn[16]!=0|| g_disklist[i].disksn[19]!=0)
			totaldisknum++;
	 
	}
	if(totaldisknum > MAX_SUPPORED_HDD_NO)
		totaldisknum = MAX_SUPPORED_HDD_NO;
	
	TempBuffer[0]= 'D';// sig DSN
	TempBuffer[1]= 'S';
	TempBuffer[2]= 'N';
	TempBuffer[3]= totaldisknum; // total sn number
	
	
	TempBuffer= TempBuffer + 12;
	
	if (g_data.BackupDiskNumber >= 0 && g_data.BackupDiskNumber < MAX_DISK_NUM)
	{
		//CopyMem(TempBuffer,g_disklist[g_data.BackupDiskNumber].disksn,20);       //length 20 system disk sn
		if (1 == g_disklist[g_data.BackupDiskNumber].isRAIDDisk)
		{
			MyLog(LOG_DEBUG, L"raidDisksn: %a\n", g_disklist[g_disknum].raidDisksn);
			CopyMem(TempBuffer, g_disklist[g_data.BackupDiskNumber].raidDisksn, 20);
		}
		else //if (0 == g_disklist[g_data.BackupDiskNumber].isRAIDDisk)
		{
			//MyLog(LOG_DEBUG, L"disksn: %a\n", g_disklist[g_disknum].disksn);
			CopyMem(TempBuffer, g_disklist[g_data.BackupDiskNumber].disksn, 20);
		}
	}

	//other disk num
	totaldisknum =0;
	
	for(i=0;i<MAX_DISK_NUM;i++)
	{
		
		if( i != g_data.BackupDiskNumber && totaldisknum< (MAX_SUPPORED_HDD_NO - 1))
		{
			if(g_disklist[i].disksn[0]!=0 || g_disklist[i].disksn[4]!=0|| g_disklist[i].disksn[8]!=0|| g_disklist[i].disksn[12]!=0|| g_disklist[i].disksn[16]!=0|| g_disklist[i].disksn[19]!=0)
			{
				TempBuffer = TempBuffer + 20;
				CopyMem(TempBuffer,g_disklist[i].disksn,20);   
				totaldisknum++;
			}
		}
	 
	}
 
}

OKR_HEADER *CreateImageHeader(CHAR16 *desc)
{
	OKR_HEADER *pheader;
	DWORD size;

	size = ((FIELD_OFFSET(OKR_HEADER, Partition) + g_dwTotalPartitions * sizeof(WIN32_PARTITIONINFO) + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1));
	pheader = myalloc(size);
	mymemset(pheader, 0, size);

	pheader->Magic			= OKR_IMAGE_MAGIC;
	pheader->Version		= OKR_VERSION;
	pheader->StructSize		= size;
	pheader->FileTime		= mytime(NULL);
	pheader->Time			= (DWORD)pheader->FileTime;
	g_data.BackupImgTime = pheader->Time;
	pheader->TimeZone		= g_data.TimeZone;
	if(IsGptSystem(g_data.BackupDiskNumber))
		pheader->bGpt		= TRUE;
	pheader->GptSize		= GPT_DATA_SIZE*512;
	pheader->Disksize		= g_disklist[g_data.BackupDiskNumber].totalsector;
	pheader->PartitionCount	= g_dwTotalPartitions;
	StrnCpy((CHAR16 *)pheader->Memo, desc, sizeof(pheader->Memo)/sizeof(CHAR16));
	AddAllHddSn((CHAR8*)pheader->Memo+100);
	CopyMem(pheader->Partition, g_Partitions, g_dwTotalPartitions * sizeof(WIN32_PARTITIONINFO));

	pheader->DataOffset		= (DWORD)(pheader->StructSize + pheader->GptSize);

	return pheader;
}

BYTE HasFactoryBackupInPartition(FILE_SYS_INFO  *SysInfo, WORD Destpart, PBYTE Buffer)
{
	BYTE bFound = FALSE;
	int filenum, namelen;
	CHAR16 * filename;
	FILE_HANDLE_INFO filehandle;

	filenum = GetFileNamelist(SysInfo, Destpart, (CHAR16*)Buffer, 1024*1024);
	if(filenum > 0)
	{
		filename = (CHAR16*)Buffer;
		while (filenum) {
			CHAR16 fullpath[256] = { 0 };
			GetBackupFileFullPath(SysInfo, Destpart, filename, 0, fullpath, 256);

			//if (IsLenovoPart(Destpart) == TRUE)
			//{
			//	SPrint(fullpath, 256, BACKUP_DIR L"\\%s", filename);
			//}
			//else
			//{
			//	SPrint(fullpath, 256, BACKUP_DIR_CUSTOMER L"\\%s", filename);
			//}
			namelen = StrLength(filename) + 1;
		
			MyLog(LOG_DEBUG, L"HasFactoryBackup,fullpath:%s.\n", fullpath);

			if (FileOpen(SysInfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, FALSE)) {
				UINTN filesize = OKR_HEADER_MIN_SIZE;
				OKR_HEADER *header = (OKR_HEADER *)(Buffer + 1024*1024);

				MyLog(LOG_DEBUG, L"Found:%s.\n", filename);

				if( FileRead(&filehandle, &filesize, (PBYTE)header)
					&& filesize == OKR_HEADER_MIN_SIZE
					&& header->Magic == OKR_IMAGE_MAGIC 
					&& header->bComplete == TRUE
					&& (header->StructSize & (SECTOR_SIZE - 1)) == 0
					&& header->StructSize <= OKR_HEADER_MAX_SIZE
					&& header->StructSize >= OKR_HEADER_MIN_SIZE
					&& header->bFactoryBackup == TRUE) 
			  {

						bFound = TRUE;
				}

				FileClose(&filehandle);

				if(bFound)
					break;
			}

			filename += namelen;
			filenum--;
		}
	}
	return bFound;
}

//判断是否工厂备份
char HasFactoryBackup()
{
	BYTE bFound = FALSE;
	DWORD i;
	FILE_SYS_INFO  sysinfo;
	PBYTE buffer;

	buffer = myalloc(1024*2048);
	if(!buffer) {
		MyLog(LOG_DEBUG, L"HasFactoryBackup, alloc memory failed.\n");
		return FALSE;
	}

//	MyLog(LOG_DEBUG, L"HasFactoryBackup,g_dwTotalPartitions:%d.\n",g_dwTotalPartitions);

	for (i=0;i<g_dwTotalPartitions;i++) {

//	MyLog(LOG_DEBUG, L"HasFactory,BackupDiskNumber:%d,DiskNumber:%d,i:%d\n",g_data.BackupDiskNumber,g_Partitions[i].DiskNumber,i);

	//	if(g_data.BackupDiskNumber != g_Partitions[i].DiskNumber)
	//		continue;
		if (g_Partitions[i].bIfBackup)
		{
			continue;
		}

		if ((g_Partitions[i].PartitionType!=FAT32)
			&&(g_Partitions[i].PartitionType!=FAT32_E)
			&&(g_Partitions[i].PartitionType!=NTFS))
			continue;

		if (DivU64x32(g_Partitions[i].TotalSectors, 2048 * 1024) < 4)	//分区大小小于4GB，可以忽略
		{
			continue;
		}

		//Factory backup store in LENOVO_PART part.
		if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) != 0)
		{
			continue;
		}

		if (FileSystemInit(&sysinfo, i, TRUE) != FALSE) {

			if(HasFactoryBackupInPartition(&sysinfo, i, buffer))
				bFound = TRUE;

			MyLog(LOG_DEBUG, L"HasFactoryBackupInPartition(%d) %d.\n", i, bFound);

			FileSystemClose(&sysinfo);
		} else {
			MyLog(LOG_DEBUG, L"HasFactoryBackup, FileSystemInit(%d) failed.\n", i);
		}

		if(bFound)
			break;
	}

	myfree(buffer);

	MyLog(LOG_DEBUG, L"HasFactoryBackup %d.\n", bFound);

	return bFound;
}

int BackupLastProcess()
{
	  if( g_show_percent < 96  )
    {
      
      DWORD i = 0;
 	//g_show_interval
    //	ProcessBarInit(pctl,100*1024*1024);

 //   	MyLog(LOG_DEBUG,L"progress  %d \n",g_show_percent);
/*
    	for( ;i<100 - g_show_percent-2; i++ )
    	{    		
    	  MyLog(LOG_DEBUG,L"progress i %d \n",i);

    	//	MyShowGoProcessBar((i)*1024*1024);   		
    	//	GoProcessBar((1)*1024*1024);
			//  GoProcessBar(BLOCK_SIZE*512);
			  GoProcessBar(((g_show_interval*2)*1024*1024+1*1024*1024));

    		//delayEx(1000);
    	}
*/    	
    	while(1)
    	{
   //   	MyLog(LOG_DEBUG,L"progress  %d \n",g_show_percent);

    		if(g_show_percent>96)
    		{
    			break;
    		}
    		GoProcessBar(((g_show_interval*2)*1024*1024+1*1024*1024));
    		//delayEx(1000);
    	}
    }
    
 //   GoProcessBar(-1);
    
    return 0;
}

 
int ReadBackupDir(FILE_SYS_INFO *sysinfo, WORD partindex)
{
    //////////
    EFI_STATUS status;
    int ret=0;
    BYTE buffer[1024] = { 0 };
    UINTN filesize;
    FILE_HANDLE_INFO filehandle;
	BOOL bIsLenovoPart =  IsLenovoPart(partindex);
	if (bIsLenovoPart == TRUE)
	{
		if (!FileOpen(sysinfo, &filehandle, BACKUP_DIR, EFI_FILE_MODE_READ, EFI_FILE_DIRECTORY, FALSE))
		{
			MyLog(LOG_DEBUG, L"ReadBackupDir,%s error\n", BACKUP_DIR);
			return -1;
		}
	}
	else
	{
		if (!FileOpen(sysinfo, &filehandle, BACKUP_DIR_CUSTOMER, EFI_FILE_MODE_READ, EFI_FILE_DIRECTORY, FALSE))
		{
			MyLog(LOG_DEBUG, L"ReadBackupDir,%s error\n", BACKUP_DIR_CUSTOMER);
			return -1;
		}
	}
      
    filesize = 1024;
    if (!FileRead(&filehandle,&filesize ,buffer)) 
    {
      
        FileClose(&filehandle);
            
		if (bIsLenovoPart)
		{
			MyLog(LOG_DEBUG, L"ReadBackupDir,read %s error\n", BACKUP_DIR);
		}
		else
		{
			MyLog(LOG_DEBUG, L"ReadBackupDir,read %s error\n", BACKUP_DIR_CUSTOMER);
		}

        return -1;
    }
    FileClose(&filehandle);
    
    return 0;
}        

void updatepartcsize (CHAR8* buffer,ULONGLONG value) //kang
{
	ULONGLONG *maxpartitionsize = (ULONGLONG *)(buffer + 4);
	*maxpartitionsize = value;
	
}
//备份程序主函数
int Backup(BOOL if_init_backup,WORD Destpart,CHAR16 *desc,PCONTROL pctl)
{
	MyLog(LOG_DEBUG, L"Backup: if_init_backup %d Destpart %d.\n", if_init_backup, Destpart);

	OKR_HEADER *pheader;
	CHAR16 name[100] = {0};
	UINT64 totaloriginsize = 0;
	UINT64 bigfiletotalsize = 0;//记录整个文件的大小
	struct CONTINUE_RWDISK_INFO conti_info;
	PBUFFER_INFO buffinfo;
	struct BUFFERARRAY buffarray;
	struct MP_INFO mpinfo;
	FILE_HANDLE_INFO filehandle;
	FILE_SYS_INFO  sysinfo;
	WORD i = 0;
	int ret = -1;
	BOOL bCreatDirError = FALSE;
	BOOL bFileCreated = FALSE, bShowError = TRUE;
	BYTE *align_buffer = NULL;
	UINT64 head_alignlen = 0, after_alignlen = 0;
	UINT64 alignbufsize = 8 * 1024 * 1024;
	ULONGLONG maxpartitionsize = 0;
	BYTE ntfsguid[512] = { 0 };

	align_buffer = myalloc(alignbufsize);

	if (align_buffer == NULL)
	{
		MyLog(LOG_DEBUG, L"Backup myalloc 1M failed\n");
		return ret;
	}
	/////////
	g_data.Cmd = USER_COMMAND_BACKUP;

	mymemset(&filehandle, 0, sizeof(filehandle));
	mymemset(&sysinfo, 0, sizeof(sysinfo));

	InitMp(&mpinfo);	//==FALSE)
	MyLog(LOG_DEBUG, L"InitMp completed, multithread %d.\n", g_data.bMultiThread);

	if (!Init_BufferArray(&buffarray))
		goto Done;

	MyLog(LOG_DEBUG, L"CompressUnitSize 0x%x MB, MaxPages 0x%x.\n",
		g_data.CompressBlockSize / (1024 * 1024), g_data.MemoryPagesNumber);
	pheader = CreateImageHeader(desc);
	if (!pheader)
		goto Done;

	if (!InitGlobalDataForBackup(pheader))
		goto Done;

	MyLog(LOG_DEBUG, L"FileSystemInit(%d).\n", Destpart);
	//初始化文件系统
	if (FileSystemInit(&sysinfo, Destpart, FALSE) == FALSE) {
		//DrawMsgBoxEx(STRING_FS_INIT_FAILED, IDOK, IDCANCEL, TYPE_INFO);
		//DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1,STRING_FS_INIT_FAILED_2,STRING_FS_INIT_FAILED_3,NULL,STRING_YES, STRING_NO);
		DrawMsgBoxEx(STRING_FS_INIT_FAILED_1, IDOK, -1, TYPE_INFO);
		bShowError = FALSE;
		goto Done;
	}

	//if(g_data.BackupDiskNumber == g_Partitions[Destpart].DiskNumber)
	pheader->bFactoryBackup = (g_data.bHasFactoryBackup > 0) ? FALSE : TRUE;

	if (if_init_backup)
	{
		pheader->bFactoryBackup = TRUE;
	}
	else
	{
		pheader->bFactoryBackup = FALSE;
	}
	MyLog(LOG_DEBUG, L"Backup: is Factory Backup %d.\n", pheader->bFactoryBackup);

	//创建.okr目录
	if (IsLenovoPart(Destpart) == TRUE)
	{
		StrnCpy(name, BACKUP_DIR, 100);
	}
	else
	{
		StrnCpy(name, BACKUP_DIR_CUSTOMER, 100);
	}

	MyLog(LOG_DEBUG, L"CreateDirectory(%s).\n", name);
	if (!FileOpen(&sysinfo, &filehandle, name, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY, FALSE))
	{
		SET_ERROR(__LINE__, OKR_ERROR_CREATE_DIRECTORY);
		goto Done;
	}
	FileClose(&filehandle);

	if (ReadBackupDir(&sysinfo, Destpart) < 0)
	{
		//   SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);       
		bCreatDirError = TRUE;
		goto Done;
	}

	//打开日志文件
	logopen(&sysinfo, Destpart);

	//SPrint(name, 100, BACKUP_DIR L"\\%x", (UINTN)pheader->Time);
	if (g_Partitions[Destpart].PartitionType == NTFS)
	{
		if (pheader->bFactoryBackup /*|| desc[0] >= 0x80*/)
		{
			SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%x-initial backup", (UINTN)pheader->Time);
		}
		else if (desc && desc[0] >= 0x80)
		{
			CHAR16 descText[100] = { 0 };
			SPrint(descText, 100, L"%s", desc);

			if (StrnCmp(desc, STRING_DEFAULT_BK_NAME, 4) == 0)
			{
				SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%x", (UINTN)pheader->Time);
			}
			else
			{
				SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%x-initial backup", (UINTN)pheader->Time);
			}
		}
		else if (desc && desc[0])
		{
			SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%x-%s", (UINTN)pheader->Time, desc);
		}
		else
		{
			SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%x", (UINTN)pheader->Time);
		}
	}
	else
	{
		if (IsLenovoPart(Destpart) == TRUE)
		{
			SPrint(name, 100, BACKUP_DIR L"\\%x", (UINTN)pheader->Time);
		}
		else
		{
			SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%x", (UINTN)pheader->Time);
		}
	}

	MyLog(LOG_DEBUG, L"CreateFile(%s).\n", name);
	if (!FileOpen(&sysinfo, &filehandle, name, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0, TRUE)) {
		SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
		goto Done;
	}
	bFileCreated = TRUE;

	MyLog(LOG_DEBUG, L"Creating image header(%d bytes).\n", pheader->StructSize);


	CopyMem(align_buffer, (PBYTE)pheader, pheader->StructSize);
	head_alignlen += pheader->StructSize;


	//   if (!FileWrite(&filehandle, pheader->StructSize, (PBYTE)pheader)) {
	//		SET_ERROR(__LINE__, OKR_ERROR_WRITE_HEADER);
	//       goto Done;
	//   }

	bigfiletotalsize += pheader->StructSize;

	//无论是否GPT分区，前34个扇区都进行备份
	//if(pheader->bGpt)
	{
		BYTE *buffer;	//[GPT_DATA_SIZE*512];
		buffer = buffarray.buffer[0].pointer;

		if (DiskRead((WORD)g_data.BackupDiskNumber, 0, pheader->GptSize / SECTOR_SIZE, buffer) == FALSE) {
			SET_ERROR(__LINE__, OKR_ERROR_READ_GPT);
			goto Done;
		}

		CopyMem(align_buffer + head_alignlen, buffer, pheader->GptSize);

		head_alignlen += pheader->GptSize;

		after_alignlen = CalcAliginSize(head_alignlen);

		if (after_alignlen > alignbufsize)
		{

			MyLog(LOG_DEBUG, L"Backup CalcAliginSize beyond buffer\n");

			goto Done;

		}

		MyLog(LOG_DEBUG, L"Backup after:%d,len:%d r\n", after_alignlen, head_alignlen);

		//一次性按4k对齐写入数据
		if (!FileWrite(&filehandle, after_alignlen, align_buffer)) {
			SET_ERROR(__LINE__, OKR_ERROR_BACKUP_GPT);
			goto Done;
		}


		//		if (!FileWrite(&filehandle, pheader->GptSize, buffer)) {
		//			SET_ERROR(__LINE__, OKR_ERROR_BACKUP_GPT);
		//			goto Done;
		//		}
		//		MyLog(LOG_DEBUG,L"Backup GPT %d bytes\n", pheader->GptSize);
		MyLog(LOG_DEBUG, L"Backup after_alignlen %d bytes\n", after_alignlen);

		bigfiletotalsize = after_alignlen;

		//	bigfiletotalsize += pheader->GptSize;
	}
	pheader->DataOffset = (DWORD)bigfiletotalsize;

	//支持硬盘异步IO
	if (g_data.bMultiThread && g_data.bAsyncIO && g_data.BackupDiskNumber != g_Partitions[Destpart].DiskNumber)
	{
		ProcessBarInit(pctl, g_data.TotalBackupSize);
		MyLog(LOG_DEBUG, L"BackupWithAsyncIo.\n");
		logflush(FALSE);
		ret = BackupWithAsyncIo(&filehandle, pheader, &buffarray, &mpinfo);
		if (ret == 0)
			goto updateheader;
		else
			goto Done;
	}

	logflush(FALSE);

	if (sysinfo.ifntfs)
	{
		MyLog(LOG_DEBUG, L"ifntfs ProcessAutoBarInit %lld.\n", g_data.TotalBackupSize + g_data.TotalBackupSize);
		ProcessBarInit(pctl, g_data.TotalBackupSize + g_data.TotalBackupSize);
	}
	else
	{
		UINT32 datatotalsize = 0;
		UINT64 backupdatatotalsize = g_data.TotalBackupSize + g_data.TotalBackupSize;

		////0x100000 * 0x400: 1073741824 Byte(1 GB)
		datatotalsize = ((g_data.TotalBackupSize / 0x100000) / 0x400);

		if (datatotalsize >= 5 && datatotalsize <= 40)
		{
			backupdatatotalsize = ((g_data.TotalBackupSize + g_data.TotalBackupSize)*(70 - ((datatotalsize - 5) / 2))) / 100;
		}
		else if (datatotalsize > 40)
		{
			backupdatatotalsize = ((g_data.TotalBackupSize + g_data.TotalBackupSize)*(66)) / 100;
		}


		//		MyLog(LOG_DEBUG,L"Backup TotalBackupSize %d M,%d\n",(g_data.TotalBackupSize/1024)/1024,backupdatatotalsize);
		 //   getch();

		ProcessBarInit(pctl, backupdatatotalsize);

		//	      ProcessBarInit(pctl, ((g_data.TotalBackupSize + g_data.TotalBackupSize)*50)/100);
	}
	MyLog(LOG_DEBUG, L"BackupDisk.guid= %x %x %x-%x %x %x\n", g_disklist[g_data.BackupDiskNumber].guid[0], g_disklist[g_data.BackupDiskNumber].guid[1], g_disklist[g_data.BackupDiskNumber].guid[2], g_disklist[g_data.BackupDiskNumber].guid[13], g_disklist[g_data.BackupDiskNumber].guid[14], g_disklist[g_data.BackupDiskNumber].guid[15]);

	for (i = 0; i < g_dwTotalPartitions; i++) {
		VOID *bitbuffer = NULL;
		INTN   size = 0;
		ULONGLONG totalblocks = 0, currentblock = 0, origin_datasize = 0, Lastblock = 0, Lastblock_1 = 0;
		ULONGLONG beginsec = 0;
		ULONGLONG totalwrite = 0, readn = 0, lastread = 0;


		if (g_data.BackupDiskNumber == g_Partitions[i].DiskNumber) // fix linux_swap bug
		{

			if (CompareGuid(&PARTITION_LINUX_SWAP_GUID, (EFI_GUID*)g_Partitions[i].TypeGuid))
			{
				MyLog(LOG_DEBUG, L"Found Linux Swap= %d\n", i);
				DiskRead((WORD)g_data.BackupDiskNumber, g_Partitions[i].BootBegin, 12, pheader->TempBuffer);
			}

			MyLog(LOG_DEBUG, L"g_Partitions[%d].PartitionType= %d\n", i, g_Partitions[i].PartitionType);

		}


		if (!g_Partitions[i].bIfBackup) {

			continue;
		}
		//scan disk
		totalblocks = DivU64x32((g_Partitions[i].TotalSectors + BLOCK_SIZE - 1), BLOCK_SIZE);
		size = DivU64x32((totalblocks + 7), 8);
		bitbuffer = g_data.databitmap;
		//SetMem(bitbuffer,size,0);
		SetMem(bitbuffer, g_databitmapbufsize, 0);

		//MyLog(LOG_DEBUG,L"scan  part %d, blocks %ld,bitbuffer=%x,size=%d\n",i,blocks,bitbuffer,size);

		//获得该分区的pagefile以及hibernate的扇区列表, 该函数第二次执行时就是简单的获取第一次的结果
		GetUnusedDataList(i);

		if (ScanPartitionData(i, GetDataToBitmap, bitbuffer, (PBYTE)&pheader->Magic) < 0) {
			pheader->Partition[i].BackupDataSize = 0;
			MyLog(LOG_DEBUG, L"Failed to scan filesystem %d\n", i);
			continue;
		}
		Lastblock = 0;
		Lastblock_1 = 0;
		//kang -- if(g_data.bDebug) 
		{
			origin_datasize = 0;
			for (currentblock = 0; currentblock < totalblocks; currentblock++) {
				if (ReadBit(currentblock, bitbuffer))
				{
					origin_datasize += BLOCK_SIZE * 512;
					Lastblock_1 = Lastblock;
					Lastblock = currentblock;
				}
			}
			//kang--	MyLog(LOG_DEBUG, L"Backup partition(%d), datasize 0x%lx bytes\n", i, origin_datasize);
		}
		//	memset(ntfsguid,0,512);//ysy 1226
		if (g_Partitions[i].HasWindows)
		{
			memset(ntfsguid, 0, 512);//ysy 1226
			if ((Lastblock - Lastblock_1) > 0x8000) {
				maxpartitionsize = Lastblock_1;
			}
			else {
				maxpartitionsize = Lastblock;
			}


			// 保存ntfs 系统分区guid
			DiskRead((WORD)g_data.BackupDiskNumber, g_Partitions[i].BootBegin, 1, ntfsguid);
			MyLog(LOG_DEBUG, L"ntfs g_Partitions[%d] ntfsguid= %x  %x  %x  %x\n", i, ntfsguid[0x48], ntfsguid[0x49], ntfsguid[0x4a], ntfsguid[0x4b]);

		}
		MyLog(LOG_DEBUG, L"Backup partition(%d),   maxpartitionsize = 0x%llx\n", i, maxpartitionsize);
		MyLog(LOG_DEBUG, L"Backup partition(%d), datasize 0x%llx bytes, Lastblock = 0x%llx,Lastblock_1 =0x%llx \n", i, origin_datasize, Lastblock, Lastblock_1);
		//WRITE bitmap
		//按4k写入整个Buffer


		after_alignlen = CalcAliginSize(size);
		//计算对齐后大小是否大于原buffer的大小
		if (after_alignlen > g_databitmapbufsize)
		{
			MyLog(LOG_ERROR, L"write bitmap,beyond align buffer\n");
			goto Done;
		}

		MyLog(LOG_DEBUG, L"Backup:%d,off:%d,after2:%d,size:%d \n", i, bigfiletotalsize, after_alignlen, size);
		//getch();
		size = after_alignlen;
		if (!FileWrite(&filehandle, size, bitbuffer)) {
			ret = -1;
			// bCreatDirError = TRUE;
			MyLog(LOG_ERROR, L"write bitmap error\n");
			goto Done;
		}
		//MyLog(LOG_DEBUG, L"2101 FileWrite FileWrite\n");
		//getch();
		//	MyLog(LOG_DEBUG, L"Backup FileWrite over\n");

  //      if (!FileWrite(&filehandle,size,bitbuffer)) {
  //          //MyLog(LOG_ERROR,L"write bitmap error\n");
  //          goto Done;
 //       }
		totalwrite = size;
		bigfiletotalsize += size;

		beginsec = g_Partitions[i].BootBegin;
		///////////////////////////////////////////////////////////////////
		logflush(FALSE);

		g_lz4_size = 8;
		Clean_BufferArray(&buffarray);
		currentblock = 0;
		SmartDiskIoInit((BYTE)g_data.BackupDiskNumber, COMMAND_READ, &conti_info);
		origin_datasize = 0;
		lastread = 0;
		while (1) {
			if (!g_data.bIsAutoFunc && UserBreak()) {


				bShowError = FALSE;
				goto Done;
			}

			if (g_data.bMultiThread)
			{
				//多线程串行IO, 在读扇区的时候进行压缩，等读取完毕，压缩也结束了，可以进行写盘操作
				buffinfo = GetConsumerAnyway(&buffarray);
				if (buffinfo->datavalid) {
					StartAP(&mpinfo, CompressBuffer, buffinfo);
				}
			}
			///////// read sector to buffer
			readn = 0;
			//MyLog(LOG_DEBUG, L"1 Read 0x%llx,off:%llx comp:0x%llx", readn, bigfiletotalsize, g_lz4_size);
			buffinfo = GetProducter(&buffarray);
			//MyLog(LOG_DEBUG, L"2 Read 0x%llx,off:%llx comp:0x%llx", readn, bigfiletotalsize, g_lz4_size);
			if (currentblock < totalblocks) {
				while (currentblock < totalblocks) {
					if (ReadBit(currentblock, bitbuffer)) {
						SmartDiskIo(&conti_info, MultU64x32(currentblock, BLOCK_SIZE) + beginsec, BLOCK_SIZE, buffinfo->pointer + readn);
						readn += BLOCK_SIZE * 512;
						GoProcessBar(BLOCK_SIZE * 512);
						if (readn == buffinfo->totalsize) {
							currentblock++;
							break;
						}
					}
					currentblock++;
				}
				SmartDiskIo(&conti_info, 0, 0, NULL);
			}
			buffinfo->usedsize = readn;//maybe 0
			origin_datasize += readn;
			totaloriginsize += readn;
			buffinfo->datavalid = TRUE;

			//MyLog(LOG_DEBUG, L"2160 g_data.bMultiThread:%d", g_data.bMultiThread);
			//等候数据压缩线程
			if (g_data.bMultiThread) {
				WaitToComplete(&mpinfo);
			}
			else {
				CompressBuffer(buffinfo);
			}

			//MyLog(LOG_DEBUG,L"wait compress  %d",g_lz4_size);
			if (!g_data.bIsAutoFunc && UserBreak()) {
				bShowError = FALSE;
				goto Done;
			}
			if (g_data.bDebug) 
			{
				MyLog(LOG_DEBUG, L"Read 0x%llx,off:%llx comp:0x%llx", readn, bigfiletotalsize, g_lz4_size);
			}

			// write compressed data to file
			if (g_lz4_size == 0) {
				MyLog(LOG_DEBUG, L"Partition(%d) backup completed.", i);
				break;
			}
			else if (g_lz4_size < 0) {
				//MyLog(LOG_DEBUG, L"compress error %d", g_lz4_size);
				break;
			}
			else if (g_lz4_size > 8) {
				if (!FileWrite(&filehandle, g_lz4_size, g_lz4_buffer)) {
					SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
					goto Done;
				}
				totalwrite += g_lz4_size;
				bigfiletotalsize += g_lz4_size;

				if ((INTN)lastread > g_lz4_size)
				{
					MyLog(LOG_DEBUG, L"backup  progress %d \n", (DWORD)(lastread - g_lz4_size));
					GoProcessBar((DWORD)(lastread - g_lz4_size));
				}
			}

			lastread = readn;	//在第二次读硬盘时才会写文件，所以需要用此变量记录上次读出的数据大小，以便显示进度
			g_lz4_size = 8;
		}

		///////////////////////////////////////////////////////////////////////////////
		//MyLog(LOG_DEBUG,L"part total write %ld bytes,orginal size %ld---\n",totalwrite,origin_datasize);
//#if OKR_DEBUG
//        origin_datasize=0;
//        for (currentblock=0;currentblock<blocks;currentblock++) {
//            if (ReadBit(currentblock,bitbuffer))
//                origin_datasize+=BLOCK_SIZE*512;
//        }
//        MyLog(LOG_DEBUG,L"after bitmap %ld---\n",origin_datasize);
//#endif
		pheader->Partition[i].BackupDataSize = totalwrite;
	}
	pheader->FileNumber = filehandle.currentfileseg + 1;

	FileClose(&filehandle);

	//UPDATE HEADER
	pheader->bComplete = TRUE;
	pheader->FileSize = bigfiletotalsize;
	pheader->OriginalSize = totaloriginsize;
	pheader->CompressBlockSize = g_data.CompressBlockSize;
	CopyMem(pheader->SourceDiskGUID, g_disklist[g_data.BackupDiskNumber].guid, 16);
	pheader->WinPartNtfsGuid[0] = ntfsguid[0x48];
	pheader->WinPartNtfsGuid[1] = ntfsguid[0x49];
	pheader->WinPartNtfsGuid[2] = ntfsguid[0x4a];
	pheader->WinPartNtfsGuid[3] = ntfsguid[0x4b];
	pheader->WinPartNtfsGuid[4] = ntfsguid[0x4c];
	pheader->WinPartNtfsGuid[5] = ntfsguid[0x4d];
	pheader->WinPartNtfsGuid[6] = ntfsguid[0x4e];
	pheader->WinPartNtfsGuid[7] = ntfsguid[0x4f];
	pheader->WinPartNtfsGuid[8] = 0;
	pheader->WinPartNtfsGuid[9] = 0;
	pheader->WinPartNtfsGuid[10] = 0;
	pheader->WinPartNtfsGuid[11] = 0;
	pheader->WinPartNtfsGuid[12] = 0;
	pheader->WinPartNtfsGuid[13] = 0;
	pheader->WinPartNtfsGuid[14] = 0;
	pheader->WinPartNtfsGuid[15] = 0;
	updatepartcsize(pheader->Memo + 100, maxpartitionsize);
	g_data.bHasFactoryBackup = TRUE;
    
updateheader:

	if (!FileOpen(&sysinfo, &filehandle, name, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0, FALSE)) {
		SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
		goto Done;
	}

	CopyMem(align_buffer, (PBYTE)pheader, pheader->StructSize);

	after_alignlen = CalcAliginSize(head_alignlen);
	//一次性按4k对齐写入数据
	if (!FileWrite(&filehandle, after_alignlen, align_buffer)) {
		SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
		goto Done;
	}

	/*

		if (!FileWrite(&filehandle, pheader->StructSize, (PBYTE)pheader)) {
			SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
			goto Done;
		}
	 */


	FileClose(&filehandle);

	//  MyLog(LOG_DEBUG,L"backup complete progress %d \n",g_show_percent);
	BackupLastProcess();

	GoProcessBar(-1);

	//延时1秒，让用户可以看到100%
	//delayEx(1000);

	DestroyProcessBar();
	g_data.BeginTime = GetTickCount() / 1000;
	g_data.LastTime = 0;

	//Update UI for valid image
	StringInfo stStringInfoREMAIN_TIME;
	stStringInfoREMAIN_TIME.enumFontType = FZLTHJW;
	stStringInfoREMAIN_TIME.iFontSize = CalcFontSize(40);

	WORD dwWidth = 0;
	GetStringLen(STRING_CALCU_IMAGE_VALUE, &stStringInfoREMAIN_TIME, &dwWidth);

	WORD dwLeft = (g_WindowWidth - dwWidth) / 2;
	float fMinscale = g_data.fMinscale;
	WORD dwTop = REC_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
	WORD dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_REAMIN_TIME_TEXT_TOP - 150) * fMinscale;
	}
	FillRect(dwLeft - 100, dwTop - 10, dwWidth + 200, dwHeight + 20, 0xFFFFFF);
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_CALCU_IMAGE_VALUE, &stStringInfoREMAIN_TIME, TRUE);

	////Caculate check value
	BOOL bRet = CalculateFileCheckValue(&sysinfo, name, pheader->FileNumber);
	if (FALSE == bRet)
	{
		if (g_bUserBreak)
		{
			MyLog(LOG_DEBUG, L"User break.\n");
			bShowError = FALSE;
			g_bUserBreak = FALSE;
			goto Done;
		}

		MyLog(LOG_DEBUG, L"CalculateFileCheckValue Error.\n");
		DrawMsgBoxEx(STRING_CALCULATE_CHECK_VALUE_ERROR, IDOK, -1, TYPE_INFO);
		bShowError = FALSE;
		goto Done;
	}

	ret = 0;

	//logflush(FALSE);

Done:

	if (align_buffer != NULL)
	{
		myfree(align_buffer);
		align_buffer = NULL;
	}
	if (g_data.databitmap)
		myfree(g_data.databitmap);
	g_data.databitmap = NULL;
	if (g_data.scanbuffer)
		myfree(g_data.scanbuffer);
	g_data.scanbuffer = NULL;
	if (g_data.groupdesc)
		myfree(g_data.groupdesc);
	g_data.groupdesc = NULL;

	if (g_lz4_buffer)
		myfree(g_lz4_buffer);
	g_lz4_buffer = NULL;

	if (pheader)
		myfree(pheader);
	Destroy_BufferArray(&buffarray);
	FileClose(&filehandle);

	ReleaseUnusedDataList();

	if (ret < 0)
	{

		MyLog(LOG_DEBUG, L"Backup failed with error %d, LineNumber %d\n", g_data.dwErrorCode, g_data.dwLineNumber);

		if (bFileCreated) {
			//备份失败，删除文件
			FileDelete(&sysinfo, name);
			for (i = 1; i <= (WORD)filehandle.currentfileseg; i++) {
				SPrint(name, 100, L"%s.%d", filehandle.filename, i);
				FileDelete(&sysinfo, name);
			}
		}
		logflush(FALSE);

		if (bCreatDirError)
		{
			//针对备份过程拔掉U盘 
			//DrawMsgBox(STRING_CREATE_DIR_ERROR, STRING_YES, NULL);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_BACKUP, 0, STRING_CREATE_DIR_ERROR);
			DestroyProcessBar();

			MyLog(LOG_DEBUG, L"Backup failed bCreatDirError error\n");
			FileClose(&g_data.hLog);
			FileSystemClose(&sysinfo);
			return ret;
		}
		if (bShowError) {
			WCHAR *szError = name;

			if (g_data.dwErrorCode == OKR_ERROR_WRITE_FILE) {
				szError = STRING_ERROR_WRITE_FILE;
			}
			else {
				SPrint(szError, 100, STRING_ERROR, g_data.dwErrorCode, g_data.dwLineNumber);
			}

			//DrawMsgBox(szError, STRING_YES, NULL);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_BACKUP, 0, szError);
		}

		DestroyProcessBar();

		//	MyLog(LOG_DEBUG, L"Backup failed bShowError %d \n",bShowError);
		//	MyLog(LOG_DEBUG, L"Backup failed bCreatDirError %d \n",bCreatDirError);
		if (bCreatDirError != FALSE)
		{
			FileClose(&g_data.hLog);
			FileSystemClose(&sysinfo);
			//  	if( bShowError != FALSE)  	
			return ret;
		}
	}
	else
	{
		MyLog(LOG_DEBUG, L"Backup successfully.\n");
		logflush(FALSE);
	}

	//关闭日志文件
	//FileClose(&g_data.hLog);
	FileSystemClose(&sysinfo);
	DestroyProcessBar();
	//if (g_data.bDebug)
	//{
	//	MyLog(LOG_DEBUG, L"Press any key to continue.\n");
	//	getch();
	//}

	return ret;
}

int GetFileNamelistFunc(FILE_SYS_INFO *sysinfo, CHAR16 *destFolder, CHAR16 **pfilelist, INTN *psize)
{
	//////////
	EFI_STATUS status;
	int ret = 0;
	BYTE buffer[1024] = { 0 };
	UINTN filesize;
	FILE_HANDLE_INFO filehandle;

	if (psize == NULL || pfilelist == NULL || *pfilelist == NULL)
	{
		MyLog(LOG_DEBUG, L"NULL point.\n");
		return -1;
	}

	if (!FileOpen(sysinfo, &filehandle, destFolder, EFI_FILE_MODE_READ, EFI_FILE_DIRECTORY, FALSE))
	{
		MyLog(LOG_DEBUG, L"GetFileNamelist,%s error\n", destFolder);
		return -1;
	}

	while (*psize > 0) {
		filesize = 1024;
		if (!FileRead(&filehandle, &filesize, buffer)) {
			FileClose(&filehandle);
			MyLog(LOG_DEBUG, L"GetFileNamelist,read %s error\n", destFolder);
			return -1;
		}

		if (filesize == 0)
			break;
		if (StrnCmp(((EFI_FILE_INFO *)buffer)->FileName, L".", 10) == 0)
			continue;
		if (StrnCmp(((EFI_FILE_INFO *)buffer)->FileName, L"..", 10) == 0)
			continue;

		MyLog(LOG_DEBUG, L"FileName:  %s\n", ((EFI_FILE_INFO *)buffer)->FileName);

		StrnCpy(*pfilelist, ((EFI_FILE_INFO *)buffer)->FileName, 100);
		*pfilelist += StrLength(((EFI_FILE_INFO *)buffer)->FileName) + 1;
		*psize -= StrLength(((EFI_FILE_INFO *)buffer)->FileName) + 1;
		ret++;
	}

	MyLog(LOG_DEBUG, L"scan %ld files", ret);

	FileClose(&filehandle);

	return ret;
}

int GetFileNamelist(FILE_SYS_INFO *sysinfo, WORD Destpart, CHAR16 *filelist, INTN size)
{
    int ret=0;

	BOOL bIsLenovoPart = IsLenovoPart(Destpart);
	if (TRUE == bIsLenovoPart)
	{
		int iRet = GetFileNamelistFunc(sysinfo, BACKUP_DIR, &filelist, &size);
		if (-1 != iRet)
		{
			ret = iRet;
		}
		//MyLog(LOG_DEBUG, L"BACKUP_DIR\n");
	}
	else
	{
		int iRet = -1;
		BOOL bRet = IsFileExistEx(sysinfo, BACKUP_DIR_CUSTOMER);
		if (bRet)
		{
			iRet = GetFileNamelistFunc(sysinfo, BACKUP_DIR_CUSTOMER, &filelist, &size);
			if (-1 != iRet)
			{
				ret = iRet;
			}
			//MyLog(LOG_DEBUG, L"BACKUP_DIR_CUSTOMER\n");
		}

		bRet = IsFileExistEx(sysinfo, BACKUP_DIR);
		if (bRet)
		{
			iRet = GetFileNamelistFunc(sysinfo, BACKUP_DIR, &filelist, &size);
			if (-1 != iRet)
			{
				ret += iRet;
			}
			//MyLog(LOG_DEBUG, L"BACKUP_DIR\n");
		}
	}
    
    return ret;
}

//获得备份点文件列表，同时删除未完成的备份点
int GetBackupFileList(int partindex, OKR_HEADER* pbackuplist, int maxnum, int *img_err)
{
	CHAR16 * buffer = NULL;
	CHAR16 * filename = NULL;
	BYTE	miniheader[OKR_HEADER_MIN_SIZE] = {0};
	OKR_HEADER *header = (OKR_HEADER *)miniheader;
	int filenum = 0, namelen = 0;
	int count = 0, img_count = 0;
	UINTN filesize = 0;
	//////////
	FILE_HANDLE_INFO filehandle;
	FILE_SYS_INFO  sysinfo;
	EFI_STATUS status;
	BOOL ret = FALSE, bReadOnly = FALSE, bDelete = FALSE;

	//BOOL bIsLenovoPart = IsLenovoPart(partindex);

	if (FileSystemInit(&sysinfo, partindex, FALSE) == FALSE)
	{
		bReadOnly = TRUE;
		if (FileSystemInit(&sysinfo, partindex, TRUE) == FALSE)
			return 0;
	}

	buffer = AllocatePool(1024 * 1024);
	if (!buffer)
	{
		goto cleanup;
	}
	
	if ((filenum = GetFileNamelist(&sysinfo, partindex, buffer, 1024 * 1024)) <= 0)//
		goto cleanup;

	filename = buffer;

	while (filenum) 
	{
		MyLog(LOG_DEBUG, L"filenum %d filename %s\n", filenum, filename);
		
		namelen = StrLength(filename) + 1;
		//MyLog(LOG_DEBUG, L"namelen %d\n", namelen);
		
		if (StrLength(filename) == 0)
		{
			MyLog(LOG_DEBUG, L"filename len is 0.\n");
			filename += namelen;
			filenum--;
			continue;
		}

		bDelete = FALSE;
		if (namelen <= sizeof(pbackuplist->FileName) / 2) {

			CHAR16 fullpath[256] = { 0 };
			GetBackupFileFullPath(&sysinfo, partindex, filename, 0, fullpath, 256);
			MyLog(LOG_DEBUG, L"Checking %s\n", fullpath);

			if (!FileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, FALSE)) {
				MyLog(LOG_ERROR, L"Open %s file error\n", fullpath);
			}
			else {
				filesize = OKR_HEADER_MIN_SIZE;
				ret = FileRead(&filehandle, &filesize, (PBYTE)header);
				//MyLog(LOG_DEBUG, L"FileRead ret %d filesize %lld \n", ret, filesize);
				if (!ret || filesize != OKR_HEADER_MIN_SIZE) {

					MyLog(LOG_DEBUG, L"Invalid image header(%s)\n", fullpath);

				}
				else {
					if (header->Magic == OKR_IMAGE_MAGIC
						&& (header->StructSize & (SECTOR_SIZE - 1)) == 0
						&& header->StructSize <= OKR_HEADER_MAX_SIZE
						&& header->StructSize >= OKR_HEADER_MIN_SIZE) {

						if (header->bComplete == TRUE) {

							header->PartitionIndex = partindex;
							CopyMem(&pbackuplist[count], header, OKR_HEADER_MIN_SIZE);
							CopyMem(&pbackuplist[count].FileName, filename, namelen * 2);

							MyLog(LOG_DEBUG, L"Found image %d:%s\n", (UINT64)pbackuplist[count].Time, pbackuplist[count].Memo);
							count++;
							if (count >= maxnum)
								break;
						}
						else {
							MyLog(LOG_DEBUG, L"Inconsistent image, bComplete = FALSE,%x", header->bFactoryBackup);
							if (!bReadOnly) {
								//删除所有不完整备份	if(header->bFactoryBackup == TRUE)
								bDelete = TRUE;
							}
							*img_err = ++img_count;

							//		MyLog(LOG_DEBUG,L"Found error image %d\n", *img_err);

						}
					}
					else if (header->Magic != OKR_IMAGE_MAGIC) {

						MyLog(LOG_DEBUG, L"Invalid magic data.");
					}
				}

				FileClose(&filehandle);


				//未完成备份的文件需要自动删除
				if (bDelete)
				{
					CHAR16 tmpname[256] = { 0 };

					DWORD i;
					UINT64 filesize;

					CopyMem(tmpname, fullpath, sizeof(fullpath));

					for (i = 1; i <= MAX_DELE_FILE_NUM; i++) 
					{
						GetBackupFileFullPath(&sysinfo, partindex, filename, i, fullpath, 256);
						//if (bIsLenovoPart)
						//{
						//	SPrint(fullpath, 256, BACKUP_DIR L"\\%s.%d", filename, i);
						//}
						//else
						//{
						//	SPrint(fullpath, 256, BACKUP_DIR_CUSTOMER L"\\%s.%d", filename, i);
						//}
						
						GetFileSizeByName(&sysinfo, fullpath, &filesize);
						if (FileDelete(&sysinfo, fullpath))
							g_Partitions[partindex].FreeSectors += filesize / SECTOR_SIZE;
					}

					GetFileSizeByName(&sysinfo, tmpname, &filesize);
					//删除备份点以后，就要增加这个文件所在的分区的剩余空间
					if (FileDelete(&sysinfo, tmpname))
						g_Partitions[partindex].FreeSectors += filesize / SECTOR_SIZE;

					MyLog(LOG_DEBUG, L"delete init back image success");
					(*img_err)--;
				}
			}
		}
		else {
			//文件名长度超过0x80字节，则失败
		}

		filename += namelen;
		filenum--;
	}

cleanup:
	if (buffer)
		FreePool(buffer);
	FileSystemClose(&sysinfo);

	return count;
}


//获取所有分区里的镜像文件的文件头，保存到g_data.images里
int GetAllBackupFiles(int *img_err_count)
{
	DWORD num = 0;
	DWORD maxnum = HEADLISTBUFFER/OKR_HEADER_MIN_SIZE;
	DWORD i, j;
	BYTE tmpimage[OKR_HEADER_MIN_SIZE] = {0};
	int img_count = 0,all_img_count = 0;
	
	if(!g_data.images)
		g_data.images = myalloc(HEADLISTBUFFER);
	
    for (i=0;i<g_dwTotalPartitions;i++) {
		//MyLog(LOG_DEBUG, L"Partition: %d\n", i);
        if ((g_Partitions[i].PartitionType!=FAT32)
        		&&(g_Partitions[i].PartitionType!=FAT32_E)
        		&&(g_Partitions[i].PartitionType!=NTFS))
            continue;
	    if (DivU64x32(g_Partitions[i].TotalSectors, 2048*1024) < 4)	//分区大小小于4GB，可以忽略
       	    continue;
      img_count = 0;
      
    	num += (DWORD)GetBackupFileList(i, g_data.images + num, maxnum - num,&img_count);
    	
    	all_img_count += img_count;
   
		//MyLog(LOG_DEBUG,L"GetAllBackupFiles img_count %d,all %d\n", img_count,all_img_count);
 	
		if (num >= maxnum)
		{
			MyLog(LOG_DEBUG, L"num(%d) >= maxnum(%d)\n", num, maxnum);
			break;
		}
	}
	g_data.ImageFilesNumber = num;

	//出厂备份需要置顶
	//检查备份是否存在lenovo_part,避免是从其他机器复制的初始备份。
	BOOL bHasFactoryImage = FALSE;
	for (i = 0; i < num; i++) 
	{
		if (g_data.images[i].bFactoryBackup &&
			StrnCmp(g_Partitions[g_data.images[i].PartitionIndex].Label, L"LENOVO_PART", 11) == 0 &&
			MyMemCmp(Window_Recovery_TypeGuid, g_Partitions[g_data.images[i].PartitionIndex].TypeGuid, 16) == 0)
		{
			if (i != 0 )
			{
				CopyMem(tmpimage, &g_data.images[0], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[0], &g_data.images[i], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[i], &tmpimage, OKR_HEADER_MIN_SIZE);
			}
			bHasFactoryImage = TRUE;
			break;
		}
	}

	//找到所有文件后，按时间从现在往前排序
	for (i = 0; i < num; i++)
	{
		if (0 == i && bHasFactoryImage)
		{
			continue;
		}

		for (j = i + 1; j < num; j++)
		{
			if(g_data.images[i].FileTime > g_data.images[j].FileTime) 
			{
				CopyMem(tmpimage, &g_data.images[i], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[i], &g_data.images[j], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[j], &tmpimage, OKR_HEADER_MIN_SIZE);
			}
		}
	}	
	
	if (NULL != img_err_count)
	{
		*img_err_count = all_img_count;
	}

    MyLog(LOG_DEBUG, L"GetAllBackupFiles.img_err_count:%d\n",all_img_count);
	
	return num;
}


int CheckAllBackupFiles()
{
	DWORD num = 0;
	DWORD filecount = 0;
	DWORD maxnum = HEADLISTBUFFER / OKR_HEADER_MIN_SIZE;
	DWORD i, j;
	CHAR16 FileNamebuffer[256];
	FILE_HANDLE_INFO filehandle;
	FILE_SYS_INFO  sysinfo;
	BOOL readonly = FALSE;
	int partindex = 0;
	int img_err_count = 0, img_err_del = 0;
	BYTE tmpimage[OKR_HEADER_MIN_SIZE] = { 0 };

	//   MyLog(LOG_DEBUG, L"CheckAllBackupFiles.FileNumber:%d\n",g_data.ImageFilesNumber);

	   //找到所有文件后
	for (i = 0; i < g_data.ImageFilesNumber; i++)
	{
		partindex = g_data.images[i].PartitionIndex;

		readonly = FALSE;

		//BOOL bIsLenovoPart = IsLenovoPart(partindex);

		if (FileSystemInit(&sysinfo, partindex, FALSE) == FALSE)
		{
			readonly = TRUE;
			if (FileSystemInit(&sysinfo, partindex, TRUE) == FALSE)
				continue;
		}
		filecount = 0;

		// MyLog(LOG_DEBUG, L"partindex:%d,i %d.FileNumber:%d\n",partindex, i,g_data.images[i].FileNumber);

		for (j = 0; j < g_data.images[i].FileNumber; j++)
		{
			GetBackupFileFullPath(&sysinfo, partindex, g_data.images[i].FileName, j, FileNamebuffer, 256);
			//if (bIsLenovoPart)
			//{
			//	if (j == 0)
			//	{
			//		SPrint(FileNamebuffer, 256, BACKUP_DIR L"\\%s", g_data.images[i].FileName);
			//	}
			//	else
			//	{
			//		SPrint(FileNamebuffer, 256, BACKUP_DIR L"\\%s.%d", g_data.images[i].FileName, j);
			//	}
			//}
			//else
			//{
			//	if (j == 0)
			//	{
			//		SPrint(FileNamebuffer, 256, BACKUP_DIR_CUSTOMER L"\\%s", g_data.images[i].FileName);
			//	}
			//	else
			//	{
			//		SPrint(FileNamebuffer, 256, BACKUP_DIR_CUSTOMER L"\\%s.%d", g_data.images[i].FileName, j);
			//	}
			//}

			if (IsExistFileInDir_NTFS_FAT(&sysinfo, FileNamebuffer) == TRUE)
			{
				filecount++;
			}
			//  MyLog(LOG_DEBUG, L"filecount %d.%s\n", filecount,FileNamebuffer);

		}


		if (g_data.images[i].FileNumber != filecount && readonly == FALSE && g_data.images[i].bFactoryBackup == TRUE)
		{
			CHAR16 fullpath[256] = { 0 };

			DWORD k;
			UINT64 filesize;



			for (k = 1; k <= MAX_DELE_FILE_NUM; k++)
			{
				GetBackupFileFullPath(&sysinfo, partindex, g_data.images[i].FileName, k, fullpath, 256);
				//if (bIsLenovoPart)
				//{
				//	SPrint(fullpath, 256, BACKUP_DIR L"\\%s.%d", g_data.images[i].FileName, k);
				//}
				//else
				//{
				//	SPrint(fullpath, 256, BACKUP_DIR_CUSTOMER L"\\%s.%d", g_data.images[i].FileName, k);
				//}				

				//  MyLog(LOG_DEBUG, L"%d,fullpath %s\n",k,fullpath);

				GetFileSizeByName(&sysinfo, fullpath, &filesize);
				if (FileDelete(&sysinfo, fullpath))
					g_Partitions[partindex].FreeSectors += filesize / SECTOR_SIZE;
			}

			GetBackupFileFullPath(&sysinfo, partindex, g_data.images[i].FileName, 0, fullpath, 256);
			//if (bIsLenovoPart)
			//{
			//	SPrint(fullpath, 256, BACKUP_DIR L"\\%s", g_data.images[i].FileName);
			//}
			//else
			//{
			//	SPrint(fullpath, 256, BACKUP_DIR_CUSTOMER L"\\%s", g_data.images[i].FileName);
			//}

			//   MyLog(LOG_DEBUG, L"fullpath %s,%d\n",fullpath,g_data.images[i].FileNumber);

			GetFileSizeByName(&sysinfo, fullpath, &filesize);

			if (FileDelete(&sysinfo, fullpath))
				g_Partitions[partindex].FreeSectors += filesize / SECTOR_SIZE;

			img_err_del++;

			//	memset(&g_data.images[i],0,sizeof(g_data.images[i]));

		}

		if (g_data.images[i].FileNumber != filecount)
		{
			img_err_count++;

			mymemset(&g_data.images[i], 0, sizeof(g_data.images[i]));

		}

		FileSystemClose(&sysinfo);
	}

	num = g_data.ImageFilesNumber;

	//出厂备份需要置顶
	//检查备份是否存在lenovo_part,避免是从其他机器复制的初始备份。
	BOOL bHasFactoryImage = FALSE;
	for (i = 0; i < num; i++)
	{
		if (g_data.images[i].bFactoryBackup &&
			StrnCmp(g_Partitions[g_data.images[i].PartitionIndex].Label, L"LENOVO_PART", 11) == 0 &&
			MyMemCmp(Window_Recovery_TypeGuid, g_Partitions[g_data.images[i].PartitionIndex].TypeGuid, 16) == 0)
		{
			if (i != 0)
			{
				CopyMem(tmpimage, &g_data.images[0], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[0], &g_data.images[i], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[i], &tmpimage, OKR_HEADER_MIN_SIZE);
			}
			bHasFactoryImage = TRUE;
			break;
		}
	}

	//找到所有文件后，按时间从现在往前排序
	for (i = 0; i < num; i++)
	{
		if (0 == i && bHasFactoryImage)
		{
			continue;
		}

		if (g_data.images[i].FileTime == 0)
		{
			for (j = i + 1; j < num; j++)
			{
				if (g_data.images[j].FileTime != 0)
				{
					//  memcpy(tmpimage, &g_data.images[i], OKR_HEADER_MIN_SIZE);
					CopyMem(&g_data.images[i], &g_data.images[j], OKR_HEADER_MIN_SIZE);


					mymemset(&g_data.images[j], 0, sizeof(g_data.images[j]));
					break;

					//	memcpy(&g_data.images[j], &tmpimage, OKR_HEADER_MIN_SIZE);
				}
			}
		}

	}


	g_data.ImageFilesNumber = g_data.ImageFilesNumber - img_err_del - img_err_count;

	MyLog(LOG_DEBUG, L"ImageFilesNumber:%d,%d,img_err_del %d\n", g_data.ImageFilesNumber, img_err_count, img_err_del);

	//	getch();

	return (img_err_count - img_err_del);
}

//检查是否需要恢复分区表
BOOL IfNeedRecoverPartTable_Linux(OKR_HEADER * head)
{
    DWORD i;
    for (i=0;i<head->PartitionCount;i++) {
        DWORD j;
        if (!head->Partition[i].bIfBackup)
            continue;

		//find bootbegin
        for (j=0;j<g_dwTotalPartitions;j++) {
            if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin 
				&& g_Partitions[j].DiskNumber == g_data.RecoverDiskNumber)
                break;
        }
        //Print(L"get %d ,%d",j,g_dwTotalPartitions);

        if (j == g_dwTotalPartitions)
            return TRUE;
        if (MyMemCmp(g_Partitions[j].UniGuid,head->Partition[i].UniGuid,16) != 0)
            return TRUE;
        if (g_Partitions[j].TotalSectors != head->Partition[i].TotalSectors)
            return TRUE;
		if (g_Partitions[j].PartitionTableBegin != head->Partition[i].PartitionTableBegin)
			return TRUE;
    }
    
    return FALSE;
}

BOOL IfPartTableChanged(OKR_HEADER * head)
{
    DWORD i;
	
	if( g_data.bForceRestorePartition == TRUE )
		return TRUE;
	
    for (i=0;i<head->PartitionCount;i++) {
        DWORD j;
        if (!head->Partition[i].bIfBackup)
            continue;


        for (j=0;j<g_dwTotalPartitions;j++) {
            if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin 
				&& g_Partitions[j].DiskNumber == g_data.RecoverDiskNumber)
                break;
        }
    //    MyLog(LOG_DEBUG, L"yyyyyyyyyyyyyget %d ,%d\n",j,g_dwTotalPartitions);

        if (j == g_dwTotalPartitions)
			{	
			
		//	MyLog(LOG_DEBUG, L"yyyy11: %d ,%d\n",j,g_dwTotalPartitions);
            return TRUE;		
			}
        if (MyMemCmp(g_Partitions[j].UniGuid,head->Partition[i].UniGuid,16) != 0)
			{//MyLog(LOG_DEBUG, L"yyyy22: %d,%d,%x ,%x\n",i,j,g_Partitions[j].UniGuid,head->Partition[i].UniGuid);
            return TRUE;}
        if (g_Partitions[j].TotalSectors != head->Partition[i].TotalSectors)
			{//MyLog(LOG_DEBUG, L"yyyy33: %,%d,%x ,%x\n",i,j,g_Partitions[j].TotalSectors,head->Partition[i].TotalSectors);
            return TRUE;}
		if (g_Partitions[j].PartitionTableBegin != head->Partition[i].PartitionTableBegin)
		{//	MyLog(LOG_DEBUG, L"yyyy44: %d ,%d,%x,%x\n",i,j,g_Partitions[j].PartitionTableBegin,head->Partition[i].PartitionTableBegin);
		return TRUE;}
    }
    
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
////是否恢复分区表
////1. 强制恢复
////2. 新硬盘恢复
////3. 备份点中标记为备份的分区，是否都可以找到
/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL IfNeedRecoverPartTable_delete(OKR_HEADER * head,int*klchange)
{
    DWORD i = 0;
	//MyLog(LOG_DEBUG, L"IfNeedRecoverPartTable_delete...\n");

	if (g_data.bForceRestorePartition == TRUE)
	{
		MyLog(LOG_DEBUG, L"bForceRestorePartition return TRUE.\n");
		return TRUE;
	}

	if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1)
	{
		MyLog(LOG_DEBUG, L"NEW_FOUND_DISK comefromnewhddface 1.Return TRUE.\n");
		return TRUE;
	}

#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"g_data.RecoverDiskNumber: %d.\n", g_data.RecoverDiskNumber);
#endif

	////查找备份点中标记为备份的分区，是否都可以找到。
	for (i = 0; i < head->PartitionCount; i++) {
		DWORD j = 0;
		if (!head->Partition[i].bIfBackup)
			continue;
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"head->Partition: %d BootBegin: %x.\n", i, head->Partition[i].BootBegin);
#endif
		for (j = 0; j < g_dwTotalPartitions; j++)
		{
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"g_Partitions: %d BootBegin: %x.disk: %d\n", j, g_Partitions[j].BootBegin, g_Partitions[j].DiskNumber);
#endif
			if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin
				&& g_Partitions[j].DiskNumber == g_data.RecoverDiskNumber)
			{
				break;
			}
		}
		//       MyLog(LOG_DEBUG, L"yyyyyyyyyyyyyaaaaget %d ,%d\n",j,g_dwTotalPartitions);

		if (j == g_dwTotalPartitions)
		{
			*klchange = 1;
			MyLog(LOG_DEBUG, L"Cannot find the par: %d (%d, %d)\n", j, g_dwTotalPartitions, klchange);
			return TRUE;
		}

	}
    
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//检查是否需要恢复分区表
////1. 强制恢复
////2. 新硬盘恢复
////3. 备份点中标记为备份的分区，是否都可以找到
////4. 分区UniGuid是否变化
////5. 分区TotalSectors是否变化
////6. 分区PartitionTableBegin是否变化
/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL IfNeedRecoverPartTable(OKR_HEADER * head,int*klchange)
{
    DWORD i = 0;

	if (g_data.bForceRestorePartition == TRUE)
	{
		return TRUE;
	}

	if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1)
	{
		return TRUE;
	}
	
	for (i = 0; i < head->PartitionCount; i++) {
		DWORD j = 0;
		if (!head->Partition[i].bIfBackup)
			continue;

		for (j = 0; j < g_dwTotalPartitions; j++) {
			if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin
				&& g_Partitions[j].DiskNumber == g_data.RecoverDiskNumber)
				break;
		}
		//  MyLog(LOG_DEBUG, L"yyyyyyyyyyyyyget %d ,%d\n",j,g_dwTotalPartitions);

		if (j == g_dwTotalPartitions)
		{
			*klchange = 1;
			MyLog(LOG_DEBUG, L"yyyy11: %d ,%d,%d\n", j, g_dwTotalPartitions, klchange);
			return TRUE;
		}
		if (MyMemCmp(g_Partitions[j].UniGuid, head->Partition[i].UniGuid, 16) != 0)
		{
			MyLog(LOG_DEBUG, L"yyyy22: %d,%d,%x ,%x\n", i, j, g_Partitions[j].UniGuid, head->Partition[i].UniGuid);
			return TRUE;
		}
		if (g_Partitions[j].TotalSectors != head->Partition[i].TotalSectors)
		{
			MyLog(LOG_DEBUG, L"yyyy33: %d,%d,%x ,%x\n", i, j, g_Partitions[j].TotalSectors, head->Partition[i].TotalSectors);
			return TRUE;
		}
		if (g_Partitions[j].PartitionTableBegin != head->Partition[i].PartitionTableBegin)
		{
			MyLog(LOG_DEBUG, L"yyyy44: %d ,%d,%x,%x\n", i, j, g_Partitions[j].PartitionTableBegin, head->Partition[i].PartitionTableBegin);
			return TRUE;
		}
	}
    
    return FALSE;
}

VOID DecompressBuffer(PBUFFER_INFO buffinfo)
{
	if (buffinfo->usedsize) {
		if (buffinfo->ifcompressed) {
			g_lz4_size = LZ4_decompress_safe(buffinfo->pointer, g_lz4_buffer, (int)buffinfo->usedsize, g_data.CompressBlockSize);
			if(g_lz4_size == 0) {
				g_lz4_size = -1;
				SET_ERROR(__LINE__, OKR_ERROR_DECOMPRESS);
			}
		} else {
			CopyMem(g_lz4_buffer, buffinfo->pointer, buffinfo->usedsize);
			g_lz4_size = buffinfo->usedsize;
		}
	} else {
		g_lz4_size = 0;
	}
    buffinfo->datavalid = FALSE;
}



BOOL GetDestinationDisk_Linux(OKR_HEADER * head)
{
	DWORD i, j;
	ULONGLONG datasize = 0;
	int backupdisk = 0;
	int destdisk = -1;
	int candidate = -1;
	
	for (i=0;i<head->PartitionCount;i++) {
		if (head->Partition[i].bIfBackup) {
			backupdisk = head->Partition[i].DiskNumber;
			break;
		}
	}
	//MyLog(LOG_DEBUG, L"backupdisk %d.\n", backupdisk);
	
	//计算分区总大小
    for (i=0;i<head->PartitionCount;i++) {
    	
		//遍历所有分区，检测到和备份时的分区信息完全一致时，可以根据它是否备份分区来判断它为backupdisk或datadisk。
       	for (j=0;j<g_dwTotalPartitions;j++) {

       	    if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin
       	    	&& g_Partitions[j].TotalSectors == head->Partition[i].TotalSectors
       	    	&& g_Partitions[j].PartitionType == head->Partition[i].PartitionType
       	    	&& MyMemCmp(g_Partitions[j].UniGuid, head->Partition[i].UniGuid,16) == 0
       	    	&& MyMemCmp((PBYTE)g_Partitions[j].Label, (PBYTE)head->Partition[i].Label, 32) == 0) {

				//MyLog(LOG_DEBUG, L"%d(%d): backup %d, disk %d(%d), destdisk %d .\n", 
				//	i, j, head->Partition[i].bIfBackup, head->Partition[i].DiskNumber, g_Partitions[j].DiskNumber, destdisk);

				if (head->Partition[i].bIfBackup) {
					if (backupdisk == head->Partition[i].DiskNumber && destdisk == -1) {
						//g_disklist[g_Partitions[j].DiskNumber].backupdisk = TRUE;
						destdisk = g_Partitions[j].DiskNumber;
						
						if(g_disklist[destdisk].removable)
						{
							destdisk = -1;
							//g_disklist[g_Partitions[j].DiskNumber].backupdisk = FALSE;
						
						}
					}
				} else {
					if (backupdisk != head->Partition[i].DiskNumber) {
						if(destdisk == g_Partitions[j].DiskNumber)	//error
							destdisk = -1;
        				g_disklist[g_Partitions[j].DiskNumber].datadisk = TRUE;
					}
				}
       	        break;
       	    }
        }

		if (head->Partition[i].bIfBackup)
	        datasize += head->Partition[i].TotalSectors;
    }
	//MyLog(LOG_DEBUG, L"destdisk %d, datasize %x.\n", destdisk, datasize);

	if(destdisk != -1) {

		//MyLog(LOG_DEBUG, L"totalsector %x.\n", g_disklist[destdisk].totalsector);

		//备份的数据大小超过硬盘大小
		if(datasize >= g_disklist[destdisk].totalsector)
			destdisk = -1;

		candidate = destdisk;
		//硬盘大小和备份时的相差8M，认为是不同的硬盘
		if(head->Disksize > g_disklist[destdisk].totalsector + 2048*8 
			|| g_disklist[destdisk].totalsector > head->Disksize + 2048*8)
			destdisk = -1;
	}
	//MyLog(LOG_DEBUG, L"check destdisk %d.\n", destdisk);
    
    //如果没有找到原来备份的硬盘，那么遍历所有硬盘，查找是否存在没有格式化的硬盘，如果有，就以这个盘作为目标盘
    //如果都是有效的硬盘，那么以第一个大小接近或超过备份镜像大小的硬盘作为目标盘。
	if(destdisk == -1) {
		for (i=0;i<g_disknum;i++) {

			if (g_disklist[i].removable || g_disklist[i].datadisk)
				continue;

			if (!g_disklist[i].formatted && (g_disklist[i].totalsector >= datasize ) && candidate == -1) {
				destdisk = i;
				break;
			}
			//检测到的第一个大小接近或超过备份镜像大小的硬盘, 并且和镜像文件所在的盘不是同一个盘的
			//作为候选目标盘
			if (g_disklist[i].totalsector == head->Disksize && candidate == -1)
			{
				candidate = i;
				break;
			}
		}
	}
	
		if(candidate == -1) {
		for (i=0;i<g_disknum;i++) {

			if (g_disklist[i].removable || g_disklist[i].datadisk)
				continue;

			if (!g_disklist[i].formatted && (g_disklist[i].totalsector >= datasize)) {
				destdisk = i;
				break;
			}
			//检测到的第一个大小接近或超过备份镜像大小的硬盘, 并且和镜像文件所在的盘不是同一个盘的
			//作为候选目标盘
			if (g_disklist[i].totalsector >= datasize && candidate == -1)
				candidate = i;
		}
	}
	
	if(destdisk == -1)
		destdisk = candidate;

	//MyLog(LOG_DEBUG, L"check destdisk %d, candidate %d.\n", destdisk, candidate);

	g_data.RecoverDiskNumber = destdisk;
    
    if (g_data.RecoverDiskNumber == -1) {
		//无法定位目标硬盘
		SET_ERROR(__LINE__, OKR_ERROR_DESTINATION_DISK_NOT_FOUND);
        MyLog(LOG_DEBUG, L"Destination disk not found.\n");
	
		DrawMsgBoxEx(STRING_PART_MODIFY_ERROR, IDOK, IDCANCEL, TYPE_WARNING);
	//	DrawMsgBox(STRING_DISK_TOO_SMALL, STRING_YES, STRING_NO);
	
        return FALSE;
    }
	MyLog(LOG_DEBUG, L"Destination disk %d.\n", g_data.RecoverDiskNumber);
    
    return TRUE;
}



#define EXTRA_SIZE1 0x2800000  // 20G 扇区
int destpartindex = -1;
int oldisk_c_small = 0; // 0 equ  , 1 small , 2 big
BOOL GetDestinationDisk(OKR_HEADER * head, int *ifolddisk)
{
	MyLog(LOG_DEBUG, L"GetDestinationDisk...\n");

	DWORD i = 0, j = 0;
	ULONGLONG datasize = 0;
	int backupdisk = -1;
	int backupdisk_sysdisk = -1;
	int destdisk = -1;
	int candidate = -1;
	*ifolddisk = -1;
	int findc = 0;  //是否找到这个分区
	oldisk_c_small = 0;
	for (i = 0; i < head->PartitionCount; i++) {
		if (head->Partition[i].bIfBackup) {
			backupdisk = head->Partition[i].DiskNumber;
			break;
		}
	}
	MyLog(LOG_DEBUG, L"backupdisk %d.\n", backupdisk);


	//Get system disk from backup point.
	for (i = 0; i < g_disknum; i++)
	{
		//从备份点中得到备份disk的信息
		if (g_disklist[i].curdisktype == DISK_SYSDISK)
		{
			backupdisk_sysdisk = i;
			break;
		}
	}

	MyLog(LOG_DEBUG, L"backupdisk_sysdisk %d.\n", backupdisk_sysdisk);

	//计算分区总大小
	for (i = 0; i < head->PartitionCount; i++)
	{
		if (head->Partition[i].bIfBackup)
			datasize += head->Partition[i].TotalSectors;
	}
	MyLog(LOG_DEBUG, L"datasize 0x%x.\n", datasize);

	///disk exist
	if (backupdisk != -1 || backupdisk_sysdisk != -1)
	{
		for (i = 0; i < head->PartitionCount; i++)
		{
			if (backupdisk != -1 &&
				head->Partition[i].DiskNumber != backupdisk &&
				head->Partition[i].bIfBackup)
			{
				MyLog(LOG_DEBUG, L"Par (%d) in backupfile is not backup par.\n", i);
				continue;
			}
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"Par in backupfile:\n par: %d: BootBegin %ld, TotalSectors %ld, UniGuid[0] %d, Label[0] %d.\n",
				i,
				head->Partition[i].BootBegin,
				head->Partition[i].TotalSectors,
				head->Partition[i].UniGuid[0],
				head->Partition[i].Label[0]);

#endif
			//遍历所有分区，检测到和备份时的分区信息完全一致时，可以根据它是否备份分区来判断它为backupdisk或datadisk。
			for (j = 0; j < g_dwTotalPartitions; j++)
			{
				if (backupdisk_sysdisk != -1 &&
					g_Partitions[j].DiskNumber != backupdisk_sysdisk)
				{
#if OKR_DEBUG
					MyLog(LOG_DEBUG, L"Par (%d) is not on disk %d.\n", j, backupdisk_sysdisk);
#endif
					continue;
				}
#if OKR_DEBUG
				if (i == 0)
				{
					MyLog(LOG_DEBUG, L"Par in disk:\n par: %d: BootBegin %ld, TotalSectors %ld, UniGuid[0] %d, Label[0] %d.\n",
						j,
						g_Partitions[j].BootBegin,
						g_Partitions[j].TotalSectors,
						g_Partitions[j].UniGuid[0],
						g_Partitions[j].Label[0]);
				}
#endif
				if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin
					&& g_Partitions[j].TotalSectors == head->Partition[i].TotalSectors
					&& g_Partitions[j].PartitionType == head->Partition[i].PartitionType
					&& MyMemCmp(g_Partitions[j].UniGuid, head->Partition[i].UniGuid, 16) == 0
					&& MyMemCmp((PBYTE)g_Partitions[j].Label, (PBYTE)head->Partition[i].Label, 32) == 0)
				{
					MyLog(LOG_DEBUG, L"GetDestinationDisk Find same partition!\n");
					*ifolddisk = 1;
					findc = 1;
					MyLog(LOG_DEBUG, L"%d(%d): backup %d, disk %d(%d), destdisk %d .\n",
						i, j, head->Partition[i].bIfBackup, head->Partition[i].DiskNumber, g_Partitions[j].DiskNumber, destdisk);

					if (head->Partition[i].bIfBackup)
					{
						if (destdisk == -1)
						{
							destdisk = g_Partitions[j].DiskNumber;
							destpartindex = j;
							if (g_disklist[destdisk].removable)
							{
								destdisk = -1;
							}
						}
					}

					//if (head->Partition[i].bIfBackup) {
					//	if (backupdisk == head->Partition[i].DiskNumber && destdisk == -1) {
					//		//g_disklist[g_Partitions[j].DiskNumber].backupdisk = TRUE;
					//		destdisk = g_Partitions[j].DiskNumber;
					//		destpartindex = j;
					//		if (g_disklist[destdisk].removable)
					//		{
					//			destdisk = -1;
					//			//g_disklist[g_Partitions[j].DiskNumber].backupdisk = FALSE;
					//		}
					//	}
					//}
					//else {
					//	if (backupdisk != head->Partition[i].DiskNumber)
					//	{
					//		MyLog(LOG_DEBUG, L"backupdisk != head->Partition[i].DiskNumber\n");
					//		if (destdisk == g_Partitions[j].DiskNumber)
					//		{
					//			MyLog(LOG_DEBUG, L"destdisk == g_Partitions[j].DiskNumber\n");
					//			//error
					//			destdisk = -1;
					//		}
					//		g_disklist[g_Partitions[j].DiskNumber].datadisk = TRUE;
					//	}
					//}
					break;
				}

				if (head->Partition[i].HasWindows == TRUE && findc == 0)
				{
					MyLog(LOG_DEBUG, L"findc = 0 i = %d j=%d \n", i, j);
					g_data.PartCBootBegin = head->Partition[i].BootBegin;
					g_data.PartCTotalSector = head->Partition[i].TotalSectors;
					MyLog(LOG_DEBUG, L"big-total :0x%llx 0x%llx\n", g_data.PartCBootBegin, g_data.PartCTotalSector);
				}

				if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin)
				{
					CHAR16 Buffr1[256] = { 0 };


					if (head->Partition[i].HasWindows == TRUE)
					{
						destpartindex = j;
						*ifolddisk = 1;
						destdisk = g_Partitions[j].DiskNumber;
						g_data.PartCBootBegin = g_Partitions[j].BootBegin;
						g_data.PartCTotalSector = g_Partitions[j].TotalSectors;
						MyLog(LOG_DEBUG, L"findc = 1 i = %d j=%d \n", i, j);
						MyLog(LOG_DEBUG, L"big-total :0x%llx 0x%llx\n", g_data.PartCBootBegin, g_data.PartCTotalSector);
						findc = 1;

						///当前系统分区大小 > 原系统分区大小
						if (g_Partitions[j].TotalSectors > head->Partition[i].TotalSectors)
							oldisk_c_small = 2;

						///当前系统分区大小 < 原系统分区大小
						if (g_Partitions[j].TotalSectors < head->Partition[i].TotalSectors)
							oldisk_c_small = 1;

						MyLog(LOG_DEBUG, L"i = %d j=%d \n", i, j);
						MyLog(LOG_DEBUG, L"Olddisk C :0x%llx 0x%llx\n", g_data.PartCBootBegin, g_data.PartCTotalSector);
						MyLog(LOG_DEBUG, L"partition C --image C :0x%llx 0x%llx\n", g_Partitions[j].TotalSectors, head->Partition[i].TotalSectors);

						///System part size change, check if bitlocker.
						if (
							(g_Partitions[j].TotalSectors < head->Partition[i].TotalSectors &&				      ///当前系统分区大小 < 原系统分区大小
							g_Partitions[j].TotalSectors < (head->Partition[i].TotalSectors - 0x8000)) ||         ///当前系统分区大小 < 原系统分区大小 - 16M
							(head->Partition[i].TotalSectors < g_Partitions[j].TotalSectors  &&					  ///原系统分区大小 < 当前系统分区大小
								head->Partition[i].TotalSectors < (g_Partitions[j].TotalSectors - 0x8000)))		  ///原系统分区大小 < 当前系统分区大小 - 16M
						{
							MyLog(LOG_DEBUG, L"System part size change.\n", i, j);

							if (g_Partitions[j].PartitionType == SHD_BITLOCKER ||
								g_Partitions[j].PartitionType == SHD_BITLOCKERTOGO)
							{
								MyLog(LOG_DEBUG, L"Par %d disk %d BitLocker ....\r\n", i, g_Partitions[i].DiskNumber);

								if (g_data.bForceRestorePartition == FALSE)
								{
									MyLog(LOG_DEBUG, L"bForceRestorePartition is false.\r\n");
																		
									//int iRet = DrawMsgBoxMultilLineEx(STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_1,
									//	STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_2,
									//	STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_3,
									//	STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_4,
									//	IDB_MSG_BTN_FORCE_RECOVERY, IDCANCEL, TYPE_ENQUIRE);

									int iRet = DrawMsgBoxEx(STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE, IDCANCEL, IDB_MSG_BTN_FORCE_RECOVERY, TYPE_ENQUIRE);
									if (iRet == IDB_MSG_BTN_FORCE_RECOVERY)
									{
										iRet = DrawMsgBoxEx(STRING_FORCE_RECOVERY_INFO, IDCANCEL, IDB_MSG_BTN_FORCE_RECOVERY, TYPE_ENQUIRE);
										if (iRet == IDB_MSG_BTN_FORCE_RECOVERY)
										{
											g_data.bForceRestorePartition = TRUE;
											//return FALSE;
										}
										else// if (iRet == IDCANCEL)
										{
											destdisk = -1;
											g_data.RecoverDiskNumber = destdisk;
											return FALSE;
										}
									}
									else// if (iRet == IDCANCEL)
									{
										destdisk = -1;
										g_data.RecoverDiskNumber = destdisk;
										return FALSE;
									}
								}
							}
						}

						///当前系统分区大小 < 原系统分区大小 - 16M
						if (g_Partitions[j].TotalSectors < (head->Partition[i].TotalSectors - 0x8000) && g_data.CurImageSyspartitionsize != 0)
						{
							UINT64	 NewAlignNum1 = MultU64x32(g_data.CurImageSyspartitionsize, BLOCK_SIZE);
							MyLog(LOG_DEBUG, L"Olddisk C sys disk CurImageSyspin sec:0x%llx\n", NewAlignNum1);

							if ((NewAlignNum1 + EXTRA_SIZE1) < head->Partition[i].TotalSectors && g_Partitions[j].TotalSectors < (NewAlignNum1 + EXTRA_SIZE1))
							{
								memset(Buffr1, 0, 256);
								if (g_data.bForceRestorePartition == FALSE)
								{
									//SPrint(Buffr1, 256, STRING_OLD_DISK_SYSDISK_TOO_SMALL, (NewAlignNum1 + EXTRA_SIZE1) / 2 / 1024 / 1024, g_Partitions[j].TotalSectors / 2 / 1024 / 1024);
									SPrint(Buffr1, 256, STRING_OLD_DISK_SYSDISK_TOO_SMALL, (NewAlignNum1 + EXTRA_SIZE1) / 2 / 1024 / 1024);
			
									DrawMsgBoxEx(Buffr1, IDOK, -1, TYPE_INFO);
									destdisk = -1;
									g_data.RecoverDiskNumber = destdisk;
									return FALSE;
								}
							}

							if (head->Partition[i].TotalSectors < (NewAlignNum1 + EXTRA_SIZE1))
							{
								memset(Buffr1, 0, 256);
								if (g_data.bForceRestorePartition == FALSE)
								{
									//SPrint(Buffr1, 256, STRING_OLD_DISK_SYSDISK_TOO_SMALL, (head->Partition[i].TotalSectors) / 2 / 1024 / 1024, g_Partitions[j].TotalSectors / 2 / 1024 / 1024);
									SPrint(Buffr1, 256, STRING_OLD_DISK_SYSDISK_TOO_SMALL, (head->Partition[i].TotalSectors) / 2 / 1024 / 1024);

									DrawMsgBoxEx(Buffr1, IDOK, -1, TYPE_INFO);
									destdisk = -1;
									g_data.RecoverDiskNumber = destdisk;
									return FALSE;
								}

							}
							g_data.needlimit = 1;
						}
					}
				}
			}

			//if (head->Partition[i].bIfBackup)
			//	datasize += head->Partition[i].TotalSectors;
		}
		MyLog(LOG_DEBUG, L"destdisk %d\n", destdisk);

		if (destdisk != -1)
		{
			//MyLog(LOG_DEBUG, L"totalsector %x.\n", g_disklist[destdisk].totalsector);

			//备份的数据大小超过硬盘大小
			if (datasize >= g_disklist[destdisk].totalsector)
			{
				destdisk = -1;
			}

			candidate = destdisk;
			//硬盘大小和备份时的相差8M，认为是不同的硬盘
			if (head->Disksize > g_disklist[destdisk].totalsector + 2048 * 8
				|| g_disklist[destdisk].totalsector > head->Disksize + 2048 * 8)
				destdisk = -1;
		}
		//MyLog(LOG_DEBUG, L"check destdisk %d.\n", destdisk);

		//如果没有找到原来备份的硬盘，且存在bitlocker分区
		if (g_data.bForceRestorePartition == FALSE &&
			destdisk == -1)
		{
			if (IfExistBitlockerOnDisk(backupdisk_sysdisk))
			{
				MyLog(LOG_DEBUG, L"BootBegin changed. Par %d disk %d BitLocker ....\r\n", i, g_Partitions[i].DiskNumber);

				//TODO id OK FORCE RECOVERY, IDCANCEL CANCEL
				//int iRet = DrawMsgBoxWithFlag(STRING_BITLOCKER_SYSTEM_PAR_BOOTPOS_CHANGE, STRING_YES, STRING_NO, 2);
				int iRet = DrawMsgBoxEx(STRING_BITLOCKER_SYSTEM_PAR_BOOTPOS_CHANGE, IDOK, IDCANCEL, TYPE_ENQUIRE);
				if (iRet == IDOK)
				{
					iRet = DrawMsgBoxEx(STRING_FORCE_RECOVERY_INFO, IDCANCEL, IDB_MSG_BTN_FORCE_RECOVERY, TYPE_ENQUIRE);
					if (iRet == IDB_MSG_BTN_FORCE_RECOVERY)
					{
						g_data.bForceRestorePartition = TRUE;
						//return FALSE;
					}
					else// if (iRet == IDCANCEL)
					{
						destdisk = -1;
						g_data.RecoverDiskNumber = destdisk;
						return FALSE;
					}
				}
				else// if (iRet == IDCANCEL)
				{
					destdisk = -1;
					g_data.RecoverDiskNumber = destdisk;
					return FALSE;
				}
			}
		}
	}

	//如果没有找到原来备份的硬盘，那么遍历所有硬盘，查找是否存在没有格式化的硬盘，如果有，就以这个盘作为目标盘
	//如果都是有效的硬盘，那么以第一个大小接近或超过备份镜像大小的硬盘作为目标盘。
	//如果是自动恢复过程，不支持恢复到其它硬盘
	if (!g_data.bIsAutoFunc)
	{
		if (destdisk == -1)
		{
			for (i = 0; i < g_disknum; i++)
			{

				if (g_disklist[i].removable || (g_disklist[i].curdisktype == DISK_DATA))
					continue;

				if (!g_disklist[i].formatted && (g_disklist[i].totalsector >= datasize)) {
					destdisk = i;
					break;
				}
				//检测到的第一个大小接近或超过备份镜像大小的硬盘, 并且和镜像文件所在的盘不是同一个盘的
				//作为候选目标盘
				if (g_disklist[i].totalsector >= datasize && candidate == -1)
					candidate = i;
			}
		}

		if (destdisk == -1)
		{
			destdisk = candidate;
		}
		//MyLog(LOG_DEBUG, L"check destdisk %d, candidate %d.\n", destdisk, candidate);
	}

	g_data.RecoverDiskNumber = destdisk;


	if (g_data.RecoverDiskNumber == -1) 
	{
		//无法定位目标硬盘
		SET_ERROR(__LINE__, OKR_ERROR_DESTINATION_DISK_NOT_FOUND);
		MyLog(LOG_DEBUG, L"Destination disk not found.\n");

		if (g_data.bIsAutoFunc)
		{
			DrawMsgBoxEx(STRING_AUTOBR_DISK_NOT_FOUND_ERROR, IDOK, -1, TYPE_INFO);
		}
		else
		{
			DrawMsgBoxEx(STRING_DISK_NOT_FOUND_ERROR, IDOK, -1, TYPE_INFO);
		}

		return FALSE;
	}
	else
		if (*ifolddisk == -1)
			*ifolddisk = 2;
	MyLog(LOG_DEBUG, L"Destination disk %d.\n", g_data.RecoverDiskNumber);


	return TRUE;
}




BOOL CheckPartitionStartPosition(OKR_HEADER * head)
{
	DWORD i, j;
 
 
    for (i=0;i<head->PartitionCount;i++)
		{
    	
		//遍历所有分区， 
       	for (j=0;j<g_dwTotalPartitions;j++)
			{	
				if(head->Partition[i].HasWindows == TRUE)
				{	
					if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin)
					{
				
						return TRUE;
			
					}
				}
			
			}

		 
		}
	 
	 
   
    return FALSE;
 
	
}


 
BOOL CheckDestinationDisk(OKR_HEADER * head)
{
	MyLog(LOG_DEBUG, L"CheckDestinationDisk...\n");

	DWORD i, j;
	ULONGLONG datasize = 0;
	int backupdisk = 0;
	int destdisk = -1;
 
	 
	destdisk = g_data.selecteddisk;
 
 
 

	g_data.RecoverDiskNumber = destdisk;
    
 
	MyLog(LOG_DEBUG, L"Destination disk %d.\n", g_data.RecoverDiskNumber);
    
    return TRUE;
}

VOID DiskWriteThread(struct BUFFERARRAY *bufferarray)
{
	PBUFFER_INFO bufferinfo;
    ULONGLONG maxblock, currentblock, total, bootbegin, number;
    struct CONTINUE_RWDISK_INFO conti_info;
    PBYTE p;
    WIN32_PARTITIONINFO *partition = NULL;	//当前正在备份或恢复的分区
    VOID *bitbuffer = NULL;
	BYTE partitionindex;
	OKR_HEADER *fileheader;

	//g_data.threadstate = 1;
	fileheader = g_data.fileheader;
	total = 0;
	
    for(;;) 
    {
		//首先是解压数据
		bufferinfo = GetConsumer(bufferarray);
		if(!bufferinfo)
			break;

		//更新bitbuffer
		partitionindex = bufferinfo->partitionindex;
		if(partition != &fileheader->Partition[partitionindex]) {

			if(bitbuffer)
				myfree(bitbuffer);
			partition = g_data.partition;
			bitbuffer = g_data.databitmap;
    		maxblock = DivU64x32((partition->TotalSectors + BLOCK_SIZE - 1), BLOCK_SIZE);
			bootbegin = partition->BootBegin;
    		SmartDiskIoInit((BYTE)g_data.RecoverDiskNumber, COMMAND_WRITE, &conti_info);
    		g_data.partition_write = partition;
    		
			if(g_data.bDebug) {
				total = 0;
				for (currentblock=0;currentblock<maxblock;currentblock++) {
					if (ReadBit(currentblock,bitbuffer))
						total += BLOCK_SIZE*512;
				}
			}
			currentblock = 0;
		}
		
		//g_data.threadstate = 3;
		//file eof
		DecompressBuffer(bufferinfo);
		if(g_lz4_size < 0)
    	    break;

		//开始写盘
        p = g_lz4_buffer;
        number = g_lz4_size;
		if(g_data.bDebug)
			total -= number;

        while (number) {
            if (currentblock >= maxblock) 
                break;
            
            if (ReadBit(currentblock, bitbuffer)) {
                DWORD count = BLOCK_SIZE;
                if (currentblock == (maxblock-1)) {//LAST BLOCK
                    count = (DWORD)(partition->TotalSectors - MultU64x32(currentblock, BLOCK_SIZE));
                }
                SmartDiskIo(&conti_info, MultU64x32(currentblock,BLOCK_SIZE) + bootbegin, count, p);
                p += BLOCK_SIZE*512;
                number -= BLOCK_SIZE*512;
            }
            currentblock++;
        }
        SmartDiskIo(&conti_info, 0, 0, NULL);
	}
	
	if(bitbuffer)
		myfree(bitbuffer);
	//g_data.threadstate = 0;
	g_data.bCompleted = TRUE;
	return;
}

//双线程并行IO方式下的恢复
int RecoverWithAsyncIo(FILE_HANDLE_INFO *filehandle, OKR_HEADER *fileheader,
	    struct BUFFERARRAY *buffarray, struct MP_INFO *mpinfo)
{
    int ret=-1;
    BOOL bRet = 0;
    DWORD i = 0;
    PBUFFER_INFO buffinfo;
    VOID *bitbuffer = NULL;
	DWORD sequence = 0;
  UINT64 after_alignlen = 0;
  
	Clean_BufferArray(buffarray);
	sequence = 0;
	g_data.partition = NULL;
	g_data.partition_write = NULL;
	
	StartAP(mpinfo, DiskWriteThread, buffarray);

    for (i=0;i<fileheader->PartitionCount;i++) {
        UINTN   size, readn;
        ULONGLONG totalblocks,totaldatasize,currentblock,currentdata;

        if (!fileheader->Partition[i].bIfBackup || !fileheader->Partition[i].BackupDataSize)
            continue;

        MyLog(LOG_DEBUG,L"Resotring partition %d.\n",i);

        //calc bitmap size
        totalblocks = DivU64x32((fileheader->Partition[i].TotalSectors + BLOCK_SIZE - 1), BLOCK_SIZE);
        size = DivU64x32((totalblocks+7),8);
         //对齐后的数据大小
		    after_alignlen = CalcAliginSize(size);
		    
        bitbuffer = myalloc(after_alignlen);
        if (!bitbuffer) {
			SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
            goto  Done;
        }
        totaldatasize = fileheader->Partition[i].BackupDataSize - after_alignlen/*size*/;
        
		//先读取bitmap, bitmap表因为整体写入，所以不会出现跨分段文件的情况
		MyLog(LOG_DEBUG,L"RecoveAsyncIo bitmap(size:%ld) of partition %d\n", size, i);


        readn = size = after_alignlen;
        if (!FileRead(filehandle, &readn, bitbuffer) || readn != size) {
			SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
            goto  Done;
        }

        //统计数据总大小
		if(g_data.bDebug) {
			currentdata = 0;
			for (currentblock=0;currentblock<totalblocks;currentblock++) {
				if (ReadBit(currentblock,bitbuffer))
					currentdata += BLOCK_SIZE*512;
			}
			MyLog(LOG_DEBUG, L"RecoveAsyncIo RestoringData: 0x%x -> 0x%x bytes\n", totaldatasize, currentdata);
		}

		//检查DiskWriteThread是否开始处理上一个分区，如果没有，则需要等待他开始处理上一个分区才能继续
		while(g_data.partition && g_data.partition_write != g_data.partition) {
			if(g_data.bCompleted) 
				goto Done;
			CpuPause();
		}
		g_data.partition = &fileheader->Partition[i];
        g_data.databitmap = bitbuffer;
		bitbuffer = NULL;	//设置为null，交给解压线程释放
        ////////////////////////////////////
        currentdata = 0;
        while (currentdata < totaldatasize) {
            DWORD readsize = 0;
            lZ4_BUFFER_HEAD lz4_buffer_head;
            if(!g_data.bIsAutoFunc && UserBreak())
            	goto Done;
            	
            //readn = 4;
            readn = sizeof(lZ4_BUFFER_HEAD);
            if (!FileRead(filehandle,&readn,(PBYTE)&lz4_buffer_head)||readn < sizeof(lZ4_BUFFER_HEAD))
           // if (!FileRead(filehandle, &readn, (PBYTE)&signature) || readn < 4) 
           {
				SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
                goto  Done;
            }
           
            
			buffinfo = GetProducter(buffarray);
			if(!buffinfo)
				goto Done;

                 
           if (lz4_buffer_head.compress_flag == COMPRESS_MAGIC || lz4_buffer_head.compress_flag == 0x676a6763)
                    buffinfo->ifcompressed = TRUE;
           else if (lz4_buffer_head.compress_flag == UNCOMPRESS_MAGIC || lz4_buffer_head.compress_flag == 0x676a6762)
                    buffinfo->ifcompressed = FALSE;
           else 
           	{
					       MyLog(LOG_DEBUG, L"RecoveAsyncIo compress_flag error 0x%x\n", lz4_buffer_head.compress_flag);
					       SET_ERROR(__LINE__, OKR_ERROR_INVALID_SIGNATURE);
                    goto  Done;
            }
    
			      buffinfo->partitionindex = (BYTE)i;
            readsize = readn = lz4_buffer_head.align_lz4_size - sizeof(lZ4_BUFFER_HEAD);                
            if( readn > g_data.CompressBlockSize + ALGIN_SIZE_BYTE )
            {
                MyLog(LOG_ERROR,L"RecoveAsyncIo Compress size beyond error\n");
                goto Done;
            }
            
          //  readsize = readn = lz4_buffer_head.align_lz4_size - sizeof(lZ4_BUFFER_HEAD);

            if (!FileRead(filehandle,&readn,buffinfo->pointer) || readsize != readn) {
				//必须完整的块，因为写入时，整块写入，不会出现分割点在一块压缩数据内的情况。
				SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
                goto  Done;
            }
            currentdata += readsize + sizeof(lZ4_BUFFER_HEAD);
            buffinfo->sequence = sequence++;
            buffinfo->usedsize = lz4_buffer_head.lz4_size;	
            buffinfo->datavalid = TRUE;
            //数据成功读取到buffer中，唤醒消费者将数据恢复到硬盘上

			MyLog(LOG_DEBUG, L"RecoveAsyncIo producer: compressed %d, size %d, current 0x%x,total 0x%x, state %d\n",
            	buffinfo->ifcompressed, readn, currentdata, totaldatasize, g_data.ThreadState);
		}
    }

    g_data.bCompleted = TRUE;

	WaitToComplete(mpinfo);
    
    BackupLastProcess();
    
    GoProcessBar(-1);

	//delayEx(1000);

    ret = 0;
Done:

	//MyLog(LOG_DEBUG, L"Recover completed with status %d\n", ret);

  //DestroyProcessBar();

    g_data.bCompleted = TRUE;
	if(bitbuffer)
		myfree(bitbuffer);
	
    return ret;
}

ULONGLONG GetFirstPartitionBeginSector(WORD DiskNumber)
{
	DWORD i;
	for( i=0;i<g_dwTotalPartitions;i++)
	{
		if(g_Partitions[i].DiskNumber == DiskNumber)
		{
			return g_Partitions[i].BootBegin;
		}
	}
	return 0;
}

BOOL IsBackup(OKR_HEADER *header, DWORD BootBegin, DWORD TotalSector)
{
	DWORD i;

	for (i=0; i<header->PartitionCount; i++) {
		if (!header->Partition[i].bIfBackup || !header->Partition[i].BackupDataSize)
			continue;
		if(header->Partition[i].BootBegin == (ULONGLONG)BootBegin) {
			if(header->Partition[i].TotalSectors == (ULONGLONG)TotalSector)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL RecheckPartitionTable_Linux(OKR_HEADER *header, BYTE *Mbr, BYTE *BackupMbr)
{
	//判断位置是否发生变化
	PARTITION_ENTRY *entry1, *entry2;
	DWORD i;

	entry1 = (PARTITION_ENTRY *)(BackupMbr + 0x1be);
	entry2 = (PARTITION_ENTRY *)(Mbr + 0x1be);
	for(i=0;i<4;i++) {
		if(IsBackup(header, entry1->frondsectortotal, entry1->totalsector)) {
			if(entry1->frondsectortotal != entry2->frondsectortotal)
				return TRUE;
			if(entry1->totalsector != entry2->totalsector)
				return TRUE;
		}
		entry1++;
		entry2++;
	}

	return FALSE;
}
BOOL RecheckPartitionTable(OKR_HEADER *header, BYTE *Mbr, BYTE *BackupMbr)
{
	//判断位置是否发生变化
	PARTITION_ENTRY *entry1, *entry2;
	DWORD i;

	if( g_data.bForceRestorePartition == TRUE )
	return TRUE;
	
	entry1 = (PARTITION_ENTRY *)(BackupMbr + 0x1be);
	entry2 = (PARTITION_ENTRY *)(Mbr + 0x1be);
	for(i=0;i<4;i++) {
		if(IsBackup(header, entry1->frondsectortotal, entry1->totalsector)) {
			if(entry1->frondsectortotal != entry2->frondsectortotal)
				return TRUE;
			if(entry1->totalsector != entry2->totalsector)
				return TRUE;
		}
		entry1++;
		entry2++;
	}

	return FALSE;
}

int find_lenght(UINT8* buffer,UINT64 startindex,UINT64 TotalLength,UINT64 requiredlength)
{
	UINT64 i = startindex ;
	UINT64 j = 0;
	for( ;i< TotalLength; i++)
	{
		if(buffer[i]!=0)
		{
			if(j >= requiredlength)
				break;
		}
		else
		j++;
	}
	
	if(j >= requiredlength)
		return 1;
	else
		return 0;
			
	
}

 
extern int resize_main(int argc, char **argv);

 
void FixSmallDiskCSize_Bitmap(char disknum,ULONGLONG BootBegin, ULONGLONG tempTotalSector)
{
	FILE_SYS_INFO  sysinfo;
	
	if(FileSystemInit(&sysinfo, destpartindex, FALSE)==FALSE)
	{
		MyLog(LOG_ERROR,L"destpartindex %d\r\n", destpartindex);
		MyLog(LOG_DEBUG, L"System destpartindex: %d.\n", destpartindex);
		////2021/12/07  Does not show this message.
		//DrawMsgBoxEx(STRING_FS_INIT_FAILED_1, IDOK, -1, TYPE_INFO);
		return;
	}
	resize_main(0,NULL);
	
	FileSystemClose(&sysinfo);
	return ;
}

void FixSmallDiskCSize(char disknum,ULONGLONG BootBegin, ULONGLONG tempTotalSector)
{
	BYTE *buffer = NULL ;	 
	//ULONGLONG *size =0 ;	 
	int j=0;
	
	ULONGLONG TotalSector= tempTotalSector;
	
	if(disknum < 0 || BootBegin == 0 || TotalSector == 0)
	{
		MyLog(LOG_DEBUG, L"FixSmallDiskCSize, disknum %d bootbegin:%llx TotalSector=%llx\n", disknum, BootBegin, TotalSector);
		//DrawMsgBox(STRING_CREATE_DIR_ERROR1, STRING_YES, STRING_NO);
		//RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR1);
		return ;
	}
	
	buffer = AllocatePool(512);	
	if(buffer == NULL)
		return ;
	
	MyLog(LOG_DEBUG,L"FixSmallDiskCSize,%llx TotalSector=%llx\n",BootBegin,TotalSector);
	
	TotalSector = (g_disklist[disknum].totalsector )&0xffffffffffffff00;
	
	TotalSector = (TotalSector- BootBegin + 1)&0xffffffffffffffc0 ;
	
	MyLog(LOG_DEBUG,L"FixSmallDiskCSize_recount TotalSector=%llx\n",TotalSector);
	if (DiskRead((WORD)disknum, BootBegin,1, buffer) == FALSE)
		{
		//DrawMsgBox(STRING_CREATE_DIR_ERROR2, STRING_YES, STRING_NO);
		RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR2);
		if(buffer)
			FreePool(buffer);
		return ;
		}
		//size = (ULONGLONG*)&buffer[0x28];
		//*size = TotalSector;
		for(j=0;j<8; j++)
		{
			buffer[0x28+j] = (TotalSector >> j*8)&0xff ;
		}
	if (DiskWrite((WORD)disknum, BootBegin,1, buffer) == FALSE)
		{
		//DrawMsgBox(STRING_CREATE_DIR_ERROR3, STRING_YES, STRING_NO);
		RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR3);
		
		}
		 
		if(buffer)
			FreePool(buffer);
		return ;
}
		
extern  int rewrite_upcase(void);
int last_upcase = 0;	
void FixPartCSize(char disknum,ULONGLONG BootBegin, ULONGLONG myTotalSector)
{
	BYTE *buffer = NULL ;	 
	//ULONGLONG *size =0 ;	 
	int j=0;
	last_upcase = 0;
	ULONGLONG TotalSector = myTotalSector;
	if(disknum < 0 || BootBegin == 0 || TotalSector == 0)
	{
		MyLog(LOG_DEBUG, L"FixPartCSize, disknum %d bootbegin:%llx TotalSector=%llx\n", disknum, BootBegin, TotalSector);
		//DrawMsgBox(STRING_CREATE_DIR_ERROR4, STRING_YES, STRING_NO);
		//RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR4);
		return ;
	}
	
	buffer = AllocatePool(512);	
	if(buffer == NULL)
		return ;
	
	//MyLog(LOG_DEBUG,L"FixPartCSize,%llx TotalSector=%llx\n",BootBegin,TotalSector);
	
	if (DiskRead((WORD)disknum, BootBegin,1, buffer) == FALSE)
		{
		DrawMsgBoxEx(STRING_DISK_READ_ERROR_TIPS, IDOK, -1, TYPE_INFO);
		if(buffer)
			FreePool(buffer);
		return ;
		}
		//size = (ULONGLONG*)&buffer[0x28];
		//*size = TotalSector;
		for(j=0;j<8; j++)
		{
			buffer[0x28+j] = (TotalSector >> j*8)&0xff ;
		}
	if (DiskWrite((WORD)disknum, BootBegin,1, buffer) == FALSE)
		{
		DrawMsgBoxEx(STRING_DISK_WRITE_ERROR_TIPS, IDOK, -1, TYPE_INFO);
		
		}
		if(buffer)
			FreePool(buffer);
		last_upcase = 1;
		return ;
}

void UpdateOKRHeaderInfo(OKR_HEADER *okrheader, ULONGLONG OsBootBegin)
{
	//查找系统分区
	for (int i = 0; i < okrheader->PartitionCount; i++)
	{
		if (MyMemCmp(Window_Recovery_TypeGuid, okrheader->Partition[i].TypeGuid, 16) == 0)
		{
			//更新备份文件中winre分区的起始地址信息
			okrheader->Partition[i].BootBegin = OsBootBegin;
			break;
		}
	}
}

int fixSmallc = 0;

#define GPT_SIZE 16896
/////
///diskknum: recovery disk number
///bootBegin:  C partition boot begin
///TotalSector: C partition total sector
///////////////////////////////////////////////////////////////////////
///对应的磁盘layout： ESP   MSR   OS    WINRE   DATA    
///定位到OS分区，之后查找OS分区之后的分区，处理WinRE分区的属性。
///
void FixNoDataPartTable(char disknum, ULONGLONG BootBegin, ULONGLONG TotalSector, OKR_HEADER *okrheader)
{
	BYTE *buffer = NULL;
	BYTE *ntfsbuffer = NULL;
	UINT64 Random1 = 0;
	UINT64 BootDSecBegin = 0;
	UINT32 Crc = 0;
	int i = 0, j = 0;
	BYTE *head = 0;

	MyLog(LOG_DEBUG, L"FixNoDataPartTable :%d 0x%llx  0x%llx\n", disknum, BootBegin, TotalSector);
	if (disknum < 0 || BootBegin == 0 || TotalSector == 0)
	{
		DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING1, IDOK, IDCANCEL, TYPE_INFO);
		return;
	}

	buffer = AllocatePool(GPT_SIZE);
	ntfsbuffer = AllocatePool(512);
	if (buffer == NULL)
	{
		MyLog(LOG_DEBUG, L"AllocatePool return NULL.\n");
		return;
	}

	if (IsGPT(disknum))
	{
		/////////////////GPT////////////////////////////

		//读取新硬盘分区表，从1扇区，读取到33扇区
		if (DiskRead((WORD)disknum, 1, GPT_SIZE / 512, buffer) == FALSE)
		{
			DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING2, IDOK, IDCANCEL, TYPE_INFO);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//判断硬盘头是否为EFI PART
		if (buffer[0] != 'E' || buffer[1] != 'F' || buffer[2] != 'I' || buffer[3] != ' ' || buffer[4] != 'P' || buffer[5] != 'A' || buffer[6] != 'R' || buffer[7] != 'T')
		{
			MyLog(LOG_DEBUG, L"wrong efi header efi part\n");
			return;
		}

		buffer[0x10] = 0;
		buffer[0x11] = 0;
		buffer[0x12] = 0;
		buffer[0x13] = 0;

		MyLog(LOG_DEBUG, L"HD GUID :%02x %02x %02x %02x\n", buffer[0x38], buffer[0x39], buffer[0x3a], buffer[0x3b]);

		//update 
		// 20～27	8	EFI信息区(GPT头)备份位置的扇区号，也就是EFI区域结束扇区号。通常是整个磁盘最末一个扇区。
		*(UINT64*)(&buffer[0x20]) = (g_disklist[disknum].totalsector - 1);

		// 30～37	8	GPT分区区域的结束扇区号，通常是倒数第34扇区。
		*(UINT64*)(&buffer[0x30]) = (g_disklist[disknum].totalsector - 0x22);

		//更新硬盘 GUID
		AsmRdRand64(&Random1);	//Generates a 64-bit random number through RDRAND instruction
		//*(UINT64*)(&buffer[0x38]) = Random1;
		for (j = 0; j < 8; j++)
		{
			//buffer[0x38+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG, L"HD GUID :%llx\n", Random1);

		//Generates a 64-bit random number through RDRAND instruction
		AsmRdRand64(&Random1);
		//*(UINT64*)(&buffer[0x40]) = Random1;
		for (j = 0; j < 8; j++)
		{
			//buffer[0x40+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG, L"HD GUID :%llx\n", Random1);

		//更新分区表
		//查找分区表中，OS分区所在位置
		for (i = 512; i < GPT_SIZE;)
		{
			head = &buffer[i];
			MyLog(LOG_DEBUG, L"HD Part BootBegin :%llx\n", *(UINT64*)(&head[0x20]));
			MyLog(LOG_DEBUG, L"HD Part End :%llx\n", *(UINT64*)(&head[0x28]));
			if (*(UINT64*)(&head[0x20]) == BootBegin)
			{
				break;
			}

			i = i + 128;
		}

		if (i > 4096)
		{
			MyLog(LOG_DEBUG, L"fixnew hdd Not Find partc\n");
			return;
		}

		MyLog(LOG_DEBUG, L"====g_disklist[disknum].totalsector :%llx \n", g_disklist[disknum].totalsector);
		MyLog(LOG_DEBUG, L"====BootBegin + TotalSector + g_wireTotalSectors :%llx \n", BootBegin + TotalSector + g_wireTotalSectors);
		MyLog(LOG_DEBUG, L"====BootBegin + TotalSector + g_wireTotalSectors + 0x64000 :%llx \n", BootBegin + TotalSector + g_wireTotalSectors + 0x64000);
		//if (g_disklist[disknum].totalsector > (BootBegin + TotalSector + g_wireTotalSectors + 0x64000)) //判断剩余空间，大于多少200MB(0x64000)创建新分区
		if (g_disklist[disknum].totalsector > (BootBegin + TotalSector + g_wireTotalSectors)) //判断剩余空间，大于多少200MB(0x64000)创建新分区
		{
			MyLog(LOG_DEBUG, L"====Space is enough to create par.\n");
			//memcpy(&buffer[i+128],&buffer[i],128);

			if (g_WinREState != 0)
			{
				//os的下一个分区，即WinRE分区。
				i = i + 128;

				//Update winre part table

				//更新分区GUID
				AsmRdRand64(&Random1);
				//*(UINT64*)(&buffer[i+0x10]) = Random1;
				for (j = 0; j < 8; j++)
				{
					//	buffer[0x10+i+j] = (Random1 >> j*8)&0xff ;
				}
				//	MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);

				AsmRdRand64(&Random1);
				//*(UINT64*)(&buffer[i+0x18]) = Random1;
				for (j = 0; j < 8; j++)
				{
					//	buffer[0x18+i+j] = (Random1 >> j*8)&0xff ;
				}
				//	MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);

				//更新分区起始地址和结束
				//20～27 8bit 该分区的起始扇区，用LBA值表示
				*(UINT64*)(&buffer[i + 0x20]) = BootDSecBegin = (BootBegin + TotalSector + 0xF) & 0xfffffffffffffff0;

				//28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (BootDSecBegin + g_wireTotalSectors) & 0xffffffffffffff00;

				//00～0F 16bit 用GUID表示的分区类型
				CopyMem(&buffer[i], Window_Recovery_TypeGuid, 16);
				*(UINT64*)(&buffer[i + 0x30]) = 0x8000000000000001;

				UpdateOKRHeaderInfo(okrheader, *(UINT64*)(&buffer[i + 0x20]));

				//WinRE不需要进行处理，会直接恢复数据。
			   //格式化分区第一个扇区。防止出现错误的文件系统
				if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				{
					MyLog(LOG_DEBUG, L"read ntfsbuffer error\n");
				}
				SetMem(ntfsbuffer, 512, 0);

				if (DiskWrite((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				{
					MyLog(LOG_DEBUG, L"write ntfsbuffer error\n");
				}

				if (ntfsbuffer)
				{
					FreePool(ntfsbuffer);
				}
			}

			fixSmallc = 0;
		}
		else
		{
			MyLog(LOG_DEBUG, L"====Space is not enough to create par.\n");
			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区结束地址

			//00～0F 16bit 用GUID表示的分区类型
			//OS 分区type guid不是Window_Recovery_TypeGuid
			//CopyMem(&buffer[i], Window_Recovery_TypeGuid, 16);

			if (g_WinREState != 0)
			{
				TotalSector = g_disklist[disknum].totalsector - (BootBegin + g_wireTotalSectors) - 0x22;
				MyLog(LOG_DEBUG, L"====C Drive size %d MB (%llx sectors).\n", TotalSector / 2 / 1024, TotalSector);

				//TODO:Check if disk size is enough

				//28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (BootBegin + TotalSector) & 0xffffffffffffff00;
				MyLog(LOG_DEBUG, L"====OS finish sector: %llx\n", *(UINT32*)(&buffer[0x28]));

				//os的下一个分区，即WinRE分区。
				i = i + 128;

				//Update winre part table

				//更新分区GUID
				AsmRdRand64(&Random1);
				//*(UINT64*)(&buffer[i+0x10]) = Random1;
				for (j = 0; j < 8; j++)
				{
					//	buffer[0x10+i+j] = (Random1 >> j*8)&0xff ;
				}
				//	MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);

				AsmRdRand64(&Random1);
				//*(UINT64*)(&buffer[i+0x18]) = Random1;
				for (j = 0; j < 8; j++)
				{
					//	buffer[0x18+i+j] = (Random1 >> j*8)&0xff ;
				}
				//	MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);

				//更新分区起始地址和结束
				//20～27 8bit 该分区的起始扇区，用LBA值表示
				*(UINT64*)(&buffer[i + 0x20]) = BootDSecBegin = (BootBegin + TotalSector) & 0xfffffffffffffff0;
				MyLog(LOG_DEBUG, L"====WinRE boot sector: %llx\n", *(UINT32*)(&buffer[i + 0x20]));

				//28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (BootDSecBegin + g_wireTotalSectors) & 0xffffffffffffff00;
				MyLog(LOG_DEBUG, L"====WinRE finish sector: %llx\n", *(UINT32*)(&buffer[i + 0x28]));

				//00～0F 16bit 用GUID表示的分区类型
				CopyMem(&buffer[i], Window_Recovery_TypeGuid, 16);
				*(UINT64*)(&buffer[i + 0x30]) = 0x8000000000000001;

				UpdateOKRHeaderInfo(okrheader, *(UINT64*)(&buffer[i + 0x20]));

				//WinRE不需要进行处理，会直接恢复数据。
			   //格式化分区第一个扇区。防止出现错误的文件系统
				if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				{
					MyLog(LOG_DEBUG, L"read ntfsbuffer error\n");
				}
				SetMem(ntfsbuffer, 512, 0);

				if (DiskWrite((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				{
					MyLog(LOG_DEBUG, L"write ntfsbuffer error\n");
				}

				if (ntfsbuffer)
				{
					FreePool(ntfsbuffer);
				}
			}
			else
			{
				TotalSector = g_disklist[disknum].totalsector - BootBegin - 0x22;
				MyLog(LOG_DEBUG, L"====C Drive size %d MB (%llx sectors).\n", TotalSector / 2 / 1024, TotalSector);

				//28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (BootBegin + TotalSector) & 0xffffffffffffff00;
				MyLog(LOG_DEBUG, L"====OS finish sector: %llx\n", *(UINT32*)(&buffer[0x28]));
			}

			fixSmallc = 1;
		}

		i = i + 128;

		for (; i < GPT_SIZE; i++)
		{
			buffer[i] = 0;
		}

		//58～5B 4bit 分区表CRC校验和
		gBS->CalculateCrc32(buffer + 512, 16384, &Crc);
		*(UINT32*)(&buffer[0x58]) = Crc;

		*(UINT32*)(&buffer[0x10]) = 0;

		//5C～*  *bit 保留，通常是全零填充
		gBS->CalculateCrc32(buffer, 0x5c, &Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;

		if (DiskWrite((WORD)disknum, 1, GPT_SIZE / 512, buffer) == FALSE)
		{
			//DrawMsgBox(STRING_CREATE_DIR_ERROR5, STRING_YES, STRING_NO);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//update backup sector 
		BYTE *backup = NULL;
		//更新分区表备份位置信息  LBA2-LBA34
		backup = buffer + 512;
		if (DiskWrite((WORD)disknum, g_disklist[disknum].totalsector - 33, 32, backup) == FALSE)
		{
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//更新1号扇区备份位置信息  LBA1
		//交换0x18与0x20的值。
		backup = buffer;
		UINT64 temp = *(UINT64*)(&buffer[0x20]);
		*(UINT64*)(&buffer[0x20]) = *(UINT64*)(&buffer[0x18]);
		*(UINT64*)(&buffer[0x18]) = temp;

		*(UINT64*)(&buffer[0x48]) = *(UINT64*)(&buffer[0x30]) + 0x1;

		//gBS->CalculateCrc32(buffer + 512, 16384, &Crc);
		//*(UINT32*)(&buffer[0x58]) = Crc;

		*(UINT32*)(&buffer[0x10]) = 0;

		MyLog(LOG_DEBUG, L"crc : %llx\n", *(UINT32*)(&buffer[0x10]));

		gBS->CalculateCrc32(buffer, 0x5c, &Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;

		MyLog(LOG_DEBUG, L"crc : %llx\n", *(UINT32*)(&buffer[0x10]));

		if (DiskWrite((WORD)disknum, g_disklist[disknum].totalsector - 1, 1, backup) == FALSE)
		{
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		for (int j = 512; j < GPT_SIZE; j = j + 128)
		{
			head = &buffer[j];
			if (*(UINT64*)(&head[0x20]))
			{
				MyLog(LOG_DEBUG, L"par[%d] bb:%llx end:%llx\n", (j - 512) / 128, *(UINT64*)(&head[0x20]), *(UINT64*)(&head[0x28]));
				MyLog(LOG_DEBUG, L"TypeGUID : %llx\n", *(UINT64*)(&head));
				MyLog(LOG_DEBUG, L"PartGUID : %llx\n", *(UINT64*)(&head[0x10]));
				MyLog(LOG_DEBUG, L"attributes   : %llx\n", *(UINT64*)(&head[0x30]));
				MyLog(LOG_DEBUG, L"label: %s\n", &head[0x38]);
			}
		}

	}
	else
	{
		/////////////////MBR////////////////////////////
		if (DiskRead((WORD)disknum, 0, 1, buffer) == FALSE)
		{
			//DrawMsgBox(STRING_MOUNT_SYS_ERROR_3, STRING_YES, STRING_NO);
			MyLog(LOG_DEBUG, L"DiskRead hdd mbr read error\n");
			if (buffer)
				FreePool(buffer);
			goto Done;
		}
		if (buffer[0x1fe] != 0x55 || buffer[0x1ff] != 0xaa)
		{
			MyLog(LOG_DEBUG, L"wrong mbr header !\n");
			FreePool(buffer);
			goto Done;
		}

		//**************asdf
		head = &buffer[0x1c6];
		//寻找c分区
		for (i = 0; i < 4; i++)
		{
			MyLog(LOG_DEBUG, L"HD BootBegin :%llx\n", *(UINT32*)(&head[i * 16]));
			if (*(UINT32*)(&head[i * 16]) == (UINT32)BootBegin)
			{
				MyLog(LOG_DEBUG, L"mbr find part c", *(UINT32*)(&head[i * 16]));
				break;
			}
		}

		if (i == 4)
		{
			MyLog(LOG_DEBUG, L"fixnew hdd mbr Not Find partc\n");
			goto Done;
		}

		if (g_disklist[disknum].totalsector > (BootBegin + TotalSector + 0x64000) && i < 3) //判断剩余空间，大于多少20G创建新分区
		{
			i++;

			buffer[0x1be + 16 * i + 0] = 0;
			buffer[0x1be + 16 * i + 1] = 0xfe;
			buffer[0x1be + 16 * i + 2] = 0xff;
			buffer[0x1be + 16 * i + 3] = 0xff;
			buffer[0x1be + 16 * i + 4] = 7;
			buffer[0x1be + 16 * i + 5] = 0xfe;
			buffer[0x1be + 16 * i + 6] = 0xff;
			buffer[0x1be + 16 * i + 7] = 0xff;
			//buffer[0x1be+16*i + 8] = ;
			//buffer[0x1be+16*i + 9] = ;
			//buffer[0x1be+16*i + 10] = ;
			//buffer[0x1be+16*i + 11] = ;
			//buffer[0x1be+16*i + 12] = ;
			//buffer[0x1be+16*i + 13] = ;
			//buffer[0x1be+16*i + 14] = ;
			//buffer[0x1be+16*i + 15] = ;

			MyLog(LOG_DEBUG, L"NEW PART GUID :%llx\n", Random1);
			//更新分区起始地址和结束
			*(UINT32*)(&buffer[0x1be + i * 16 + 8]) = BootDSecBegin = (UINT32)((BootBegin + TotalSector + 0xff) & 0xffffff00);
			*(UINT32*)(&buffer[0x1be + i * 16 + 12]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin - TotalSector - 0xff) & 0xfffffff0);

			MyLog(LOG_DEBUG, L"i =%d 0x1be + i*16 = %d\n", i, 0x1be + i * 16);


			//格式化分区第一个扇区。防止出现错误的文件系统
			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
			{
				MyLog(LOG_DEBUG, L"read ntfsbuffer error\n");
			}
			SetMem(ntfsbuffer, 512, 0);
			if (DiskWrite((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
			{
				MyLog(LOG_DEBUG, L"write ntfsbuffer error\n");
			}
			if (ntfsbuffer)
			{
				FreePool(ntfsbuffer);
			}

			fixSmallc = 0;
		}
		else
		{
			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区起始地址和结束
		//	*(UINT32*)(&buffer[0x1ca]) = (g_disklist[disknum].totalsector )&0xffffffffffffff00;
			*(UINT32*)(&buffer[0x1ca + i * 16]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin) & 0xfffffff0);
			fixSmallc = 1;
			MyLog(LOG_DEBUG, L"i =%d 0x1ca + i*16 = %d\n", i, 0x1ca + i * 16);
		}

		if (i < 3)
		{
			SetMem((buffer + 0x1be + i * 16 + 16), 16 * (3 - i), 0); //后面分区清0
			MyLog(LOG_DEBUG, L"SetMem i =%d 0x1be + i*16 = %d\n", i, 0x1be + i * 16);
		}
		//**************asdf

		if (DiskWrite((WORD)disknum, 0, 1, buffer) == FALSE)
		{
			MyLog(LOG_DEBUG, L"DiskWrite hdd mbr read error\n");
			if (buffer)
			{
				FreePool(buffer);
			}
			goto Done;
		}
	}

Done:
	if (buffer)
	{
		FreePool(buffer);
	}
}

INT64 volume_size = 0;
INT64 get_volume_size(void)
{
	return volume_size;
}

///计算WinRE分区与OS分区之间的空间
void IsFixNewHddWay(OKR_HEADER *okrheader)
{
	int wirecount, i, t;
	ULONGLONG SizeMined = 0;
	CHAR16 LenovoLabel[32] = { L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T' };
	MyLog(LOG_DEBUG, L"FALSE wirecountZWANG okrheader->PartitionCount  %x \n", okrheader->PartitionCount);

	for (i = 0; i < okrheader->PartitionCount; i++) 
	{
		//Find WinRE Par: GUID is recovery GUID, label is not Lenovo part
		if (MyMemCmp(Window_Recovery_TypeGuid, okrheader->Partition[i].TypeGuid, 16) == 0 && MyMemCmp(okrheader->Partition[i].Label, LenovoLabel, 22) != 0) {
			wirecount = i; //wire count is ESP to Wire total count
			MyLog(LOG_DEBUG, L"wirecount: %d.\n", wirecount);
			break;
		}
	}

	for (i = 0; i < okrheader->PartitionCount; i++) 
	{
		//Find OS Par: isbackup and HasWindows
		if (okrheader->Partition[i].bIfBackup == TRUE && okrheader->Partition[i].HasWindows == TRUE) 
		{
			t = i;
			MyLog(LOG_DEBUG, L"ENTER T here\n");
			MyLog(LOG_DEBUG, L"os: %d.\n", t);
			break;
		}
	}

	MyLog(LOG_DEBUG, L"wire bb 0x%llx, os bb: 0x%llx os total: %lld GB.\n", okrheader->Partition[wirecount].BootBegin, okrheader->Partition[t].BootBegin, okrheader->Partition[t].TotalSectors / 2 / 1024 / 1024);
	   
	if (okrheader->Partition[wirecount].BootBegin >= okrheader->Partition[t].BootBegin)
	{
		SizeMined = okrheader->Partition[wirecount].BootBegin - okrheader->Partition[t].BootBegin - okrheader->Partition[t].TotalSectors;
	}
	else
	{
		g_WinREState = 0;
		SizeMined = okrheader->Disksize - okrheader->Partition[t].BootBegin - okrheader->Partition[t].TotalSectors;
	}

	MyLog(LOG_DEBUG, L"wire tt  %lld MB, SizeMined: %lld MB.disk size %lld MB\n", okrheader->Partition[wirecount].TotalSectors / 2 / 1024, SizeMined / 2 / 1024, okrheader->Disksize / 2 / 1024);

	///0x1000: 2MB
	if (SizeMined < 0x1000)
	{
		IsNoDataPartFlag = TRUE;
		g_WinREState = 2;
	}
	else
	{
		IsNoDataPartFlag = FALSE;
		g_WinREState = 1;
	}

	MyLog(LOG_DEBUG, L"IsNoDataPartFlag:  %d \n", IsNoDataPartFlag);
}


void  GetWirePartSector(OKR_HEADER *okrheader)
{
	int wirecount, i;
	CHAR16 LenovoLabel[32] = { L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T' };
	for (i = 0; i < okrheader->PartitionCount; i++) {

		if (MyMemCmp(Window_Recovery_TypeGuid, okrheader->Partition[i].TypeGuid, 16) == 0 && MyMemCmp(okrheader->Partition[i].Label, LenovoLabel, 22) != 0) {
			wirecount = i; //wire count is ESP to Wire total count
			MyLog(LOG_DEBUG, L"wirecount: %x\n", wirecount);
			break;
		}
	}
	g_wireTotalSectors = okrheader->Partition[wirecount].TotalSectors;


	MyLog(LOG_DEBUG, L"g_wireTotalSectors 0x%llx\n", g_wireTotalSectors);

}

/////
///diskknum: recovery disk number
///bootBegin:  C partition boot begin
///TotalSector: C partition total sector
void FixNewHDD(char disknum, ULONGLONG BootBegin, ULONGLONG TotalSector)
{
	BYTE *buffer = NULL;
	BYTE *ntfsbuffer = NULL;
	UINT64 Random1 = 0;
	UINT64 BootDSecBegin = 0;
	UINT32 Crc = 0;
	int i = 0, j = 0;
	BYTE *head = 0;
	int m = 0;
	UINT64 WinrePartGUID1 = 0x4d4006d1de94bba4; // this is winre part GUID   WX++

	MyLog(LOG_DEBUG, L"FixNewHDD :%d 0x%llx  0x%llx\n", disknum, BootBegin, TotalSector);
	if (disknum < 0 || BootBegin == 0 || TotalSector == 0)
	{
		DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING1, IDOK, IDCANCEL, TYPE_INFO);
		return;
	}

	buffer = AllocatePool(GPT_SIZE);
	ntfsbuffer = AllocatePool(512);
	if (buffer == NULL)
	{
		MyLog(LOG_DEBUG, L"AllocatePool return NULL.\n");
		return;
	}

	if (IsGPT(disknum))
	{
		/////////////////GPT////////////////////////////

		//读取新硬盘分区表，从1扇区，读取到33扇区
		if (DiskRead((WORD)disknum, 1, GPT_SIZE / 512, buffer) == FALSE)
		{
			DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING2, IDOK, IDCANCEL, TYPE_INFO);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//判断硬盘头是否为EFI PART
		if (buffer[0] != 'E' || buffer[1] != 'F' || buffer[2] != 'I' || buffer[3] != ' ' || buffer[4] != 'P' || buffer[5] != 'A' || buffer[6] != 'R' || buffer[7] != 'T')
		{
			MyLog(LOG_DEBUG, L"wrong efi header efi part\n");
			return;
		}

		buffer[0x10] = 0;
		buffer[0x11] = 0;
		buffer[0x12] = 0;
		buffer[0x13] = 0;

		MyLog(LOG_DEBUG, L"HD GUID :%02x %02x %02x %02x\n", buffer[0x38], buffer[0x39], buffer[0x3a], buffer[0x3b]);

		//update 
		// 20～27	8	EFI信息区(GPT头)备份位置的扇区号，也就是EFI区域结束扇区号。通常是整个磁盘最末一个扇区。
		*(UINT64*)(&buffer[0x20]) = (g_disklist[disknum].totalsector - 1);

		// 30～37	8	GPT分区区域的结束扇区号，通常是倒数第34扇区。
		*(UINT64*)(&buffer[0x30]) = (g_disklist[disknum].totalsector - 0x22);

		//更新硬盘 GUID
		AsmRdRand64(&Random1);	//Generates a 64-bit random number through RDRAND instruction
		//*(UINT64*)(&buffer[0x38]) = Random1;
		for (j = 0; j < 8; j++)
		{
			//buffer[0x38+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG, L"HD GUID :%llx\n", Random1);

		//Generates a 64-bit random number through RDRAND instruction
		AsmRdRand64(&Random1);
		//*(UINT64*)(&buffer[0x40]) = Random1;
		for (j = 0; j < 8; j++)
		{
			//buffer[0x40+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG, L"HD GUID :%llx\n", Random1);
		
		//更新分区表

		//查找分区表中，OS分区所在位置
		for (i = 512; i < GPT_SIZE;)
		{
			head = &buffer[i];
			MyLog(LOG_DEBUG, L"HD Part BootBegin :%llx\n", *(UINT64*)(&head[0x20]));
			MyLog(LOG_DEBUG, L"HD Part End :%llx\n", *(UINT64*)(&head[0x28]));
			if (*(UINT64*)(&head[0x20]) == BootBegin)
			{
				break;
			}

			i = i + 128;
		}

		if (i > 4096)
		{
			MyLog(LOG_DEBUG, L"fixnew hdd Not Find partc\n");
			return;
		}

		MyLog(LOG_DEBUG, L"===os part i : %d %d\n", i, (i - 512) / 128);

		//查找分区表中，winre分区所在位置
		for (m = 512; m < GPT_SIZE;)
		{
			////head = &buffer[m];
			//MyLog(LOG_DEBUG, L"buffer m : %d  %d\n", m, (m - 512) / 128);
			//MyLog(LOG_DEBUG, L"TypeGUID : %llx\n", *(UINT64*)(&buffer[m]));
			//MyLog(LOG_DEBUG, L"PartGUID : %llx\n", m, *(UINT64*)(&buffer[m + 0x10]));
			//MyLog(LOG_DEBUG, L"BootSecBegin : %llx\n", *(UINT64*)(&buffer[m + 0x20]));
			//MyLog(LOG_DEBUG, L"BootSecEnd   : %llx\n", *(UINT64*)(&buffer[m + 0x28]));
			//MyLog(LOG_DEBUG, L"attributes   : %llx\n", *(UINT64*)(&buffer[m + 0x30]));
			//MyLog(LOG_DEBUG, L"label: %s\n", &buffer[m + 0x38]);

			if (*(UINT64*)(&buffer[m]) == WinrePartGUID1) {
				break;
			}

			m = m + 128;
		}

		MyLog(LOG_DEBUG, L"===Winre part m : %d  %d\n", m, (m - 512) / 128);

		if (g_disklist[disknum].totalsector > (BootBegin + TotalSector + g_wireTotalSectors + 0x64000)) //判断剩余空间，大于多少20G创建新分区
		{
			//以os分区表的内容为基础，创建新分区。
			memcpy(&buffer[i + 128], &buffer[i], 128);

			i = i + 128;

			//更新分区GUID
			AsmRdRand64(&Random1);
			*(UINT64*)(&buffer[i+0x10]) = Random1;
			for (j = 0; j < 8; j++)
			{
				buffer[0x10+i+j] = (Random1 >> j*8)&0xff ;
				//buffer[0x10 + i + j] = 0;
			}
			MyLog(LOG_DEBUG, L"NEW PART GUID :%llx\n", Random1);

			AsmRdRand64(&Random1);
			*(UINT64*)(&buffer[i+0x18]) = Random1;
			for (j = 0; j < 8; j++)
			{
				buffer[0x18+i+j] = (Random1 >> j*8)&0xff ;
				//buffer[0x18 + i + j] = 0;
			}
			MyLog(LOG_DEBUG, L"NEW PART GUID :%llx\n", Random1);

			//更新分区起始地址和结束
			////20～27 8bit 该分区的起始扇区，用LBA值表示
			*(UINT64*)(&buffer[i + 0x20]) = BootDSecBegin = (BootBegin + TotalSector + 0xF) & 0xfffffffffffffff0;
			MyLog(LOG_DEBUG, L"BootDSecBegin :%llx\n", *(UINT64*)(&buffer[i + 0x20]));
			if (g_WinREState != 0)
			{
				////28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (g_disklist[disknum].totalsector - g_wireTotalSectors - 0x22 - 1) & 0xffffffffffffff00;
			}
			else
			{
				////28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (g_disklist[disknum].totalsector - 0x22) & 0xfffffffffffffff0;
			}
			MyLog(LOG_DEBUG, L"BootDSecEnd :%llx\n", *(UINT64*)(&buffer[i + 0x28]));

			//格式化分区第一个扇区。防止出现错误的文件系统
			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
			{
				MyLog(LOG_DEBUG, L"read ntfsbuffer error\n");
			}
			SetMem(ntfsbuffer, 512, 0);
			if (DiskWrite((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
			{
				MyLog(LOG_DEBUG, L"write ntfsbuffer error\n");
			}

			if (ntfsbuffer)
			{
				FreePool(ntfsbuffer);
			}

			fixSmallc = 0;
		}
		else
		{
			MyLog(LOG_DEBUG, L"==No data par.\n");

			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区起始地址和结束
			//28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
			//*(UINT64*)(&buffer[i + 0x28]) = (g_disklist[disknum].totalsector - 0x22);// & 0xffffffffffffff00;

			if (g_WinREState != 0)
			{
				////28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (g_disklist[disknum].totalsector - g_wireTotalSectors - 0x22 - 1) & 0xffffffffffffff00;
			}
			else
			{
				////28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				*(UINT64*)(&buffer[i + 0x28]) = (g_disklist[disknum].totalsector - 0x22) & 0xffffffffffffff00;
			}

			fixSmallc = 1;
		}

		i = i + 128;
		if (g_WinREState != 0)
		{
			if (m >= i)
			{
				MyLog(LOG_DEBUG, L"m >= i.\n");

				memcpy(&buffer[i], &buffer[m], 128);
				//////0x22: GPT 分区表扇区数，34.
				////更新分区起始地址和结束
				////20～27 8bit 该分区的起始扇区，用LBA值表示
				//*(UINT64*)(&buffer[i + 0x20]) = (g_disklist[disknum].totalsector - g_wireTotalSectors - 0x22);// &0xfffffffffffffff0;
				////28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
				//*(UINT64*)(&buffer[i + 0x28]) = (g_disklist[disknum].totalsector - 0x22);// &0xffffffffffffff00;
				//*(UINT64*)(&buffer[i + 0x30]) = 0x8000000000000001;
				//
				//更新备份文件中winre分区的起始地址信息
				//okrheader->Partition[i].BootBegin = *(UINT64*)(&buffer[i + 0x20]);
				i = i + 128;
			}
		}

		///清除分区表中其余的分区信息。包括WinRE被清除。
		for (; i < GPT_SIZE; i++)
		{
			buffer[i] = 0;
		}

		//58～5B 4bit 分区表CRC校验和
		gBS->CalculateCrc32(buffer + 512, 16384, &Crc);
		*(UINT32*)(&buffer[0x58]) = Crc;

		*(UINT32*)(&buffer[0x10]) = 0;

		//5C～*  *bit 保留，通常是全零填充
		gBS->CalculateCrc32(buffer, 0x5c, &Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;



		if (DiskWrite((WORD)disknum, 1, GPT_SIZE / 512, buffer) == FALSE)
		{
			//DrawMsgBox(STRING_CREATE_DIR_ERROR5, STRING_YES, STRING_NO);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);
			if (buffer)
				FreePool(buffer);
			return;
		}

		for (int j = 512; j < GPT_SIZE; j = j + 128)
		{
			head = &buffer[j];
			if (*(UINT64*)(&head[0x20]))
			{
				MyLog(LOG_DEBUG, L"par[%d] bb:%llx end:%llx\n", (j - 512) / 128, *(UINT64*)(&head[0x20]), *(UINT64*)(&head[0x28]));
				MyLog(LOG_DEBUG, L"TypeGUID : %llx\n", *(UINT64*)(&head));
				MyLog(LOG_DEBUG, L"PartGUID : %llx\n", *(UINT64*)(&head[0x10]));
				MyLog(LOG_DEBUG, L"attributes   : %llx\n", *(UINT64*)(&head[0x30]));
				MyLog(LOG_DEBUG, L"label: %s\n", &head[0x38]);
			}
		}
	}
	else
	{
		/////////////////MBR////////////////////////////
		if (DiskRead((WORD)disknum, 0, 1, buffer) == FALSE)
		{
			//DrawMsgBox(STRING_MOUNT_SYS_ERROR_3, STRING_YES, STRING_NO);
			MyLog(LOG_DEBUG, L"DiskRead hdd mbr read error\n");
			if (buffer)
				FreePool(buffer);
			goto Done;
		}
		if (buffer[0x1fe] != 0x55 || buffer[0x1ff] != 0xaa)
		{
			MyLog(LOG_DEBUG, L"wrong mbr header !\n");
			FreePool(buffer);
			goto Done;
		}

		//**************asdf
		head = &buffer[0x1c6];
		//寻找c分区
		for (i = 0; i < 4; i++)
		{

			MyLog(LOG_DEBUG, L"HD BootBegin :%llx\n", *(UINT32*)(&head[i * 16]));
			if (*(UINT32*)(&head[i * 16]) == (UINT32)BootBegin)
			{
				MyLog(LOG_DEBUG, L"mbr find part c", *(UINT32*)(&head[i * 16]));
				break;
			}
		}

		if (i == 4)
		{
			MyLog(LOG_DEBUG, L"fixnew hdd mbr Not Find partc\n");
			goto Done;
		}

		if (g_disklist[disknum].totalsector > (BootBegin + TotalSector + 0x64000) && i < 3) //判断剩余空间，大于多少20G创建新分区
		{
			i++;

			buffer[0x1be + 16 * i + 0] = 0;
			buffer[0x1be + 16 * i + 1] = 0xfe;
			buffer[0x1be + 16 * i + 2] = 0xff;
			buffer[0x1be + 16 * i + 3] = 0xff;
			buffer[0x1be + 16 * i + 4] = 7;
			buffer[0x1be + 16 * i + 5] = 0xfe;
			buffer[0x1be + 16 * i + 6] = 0xff;
			buffer[0x1be + 16 * i + 7] = 0xff;
			//buffer[0x1be+16*i + 8] = ;
			//buffer[0x1be+16*i + 9] = ;
			//buffer[0x1be+16*i + 10] = ;
			//buffer[0x1be+16*i + 11] = ;
			//buffer[0x1be+16*i + 12] = ;
			//buffer[0x1be+16*i + 13] = ;
			//buffer[0x1be+16*i + 14] = ;
			//buffer[0x1be+16*i + 15] = ;

			MyLog(LOG_DEBUG, L"NEW PART GUID :%llx\n", Random1);
			//更新分区起始地址和结束
			*(UINT32*)(&buffer[0x1be + i * 16 + 8]) = BootDSecBegin = (UINT32)((BootBegin + TotalSector + 0xff) & 0xffffff00);
			*(UINT32*)(&buffer[0x1be + i * 16 + 12]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin - TotalSector - 0xff) & 0xfffffff0);

			MyLog(LOG_DEBUG, L"i =%d 0x1be + i*16 = %d\n", i, 0x1be + i * 16);


			//格式化分区第一个扇区。防止出现错误的文件系统
			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
			{
				MyLog(LOG_DEBUG, L"read ntfsbuffer error\n");
			}
			SetMem(ntfsbuffer, 512, 0);
			if (DiskWrite((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
			{
				MyLog(LOG_DEBUG, L"write ntfsbuffer error\n");
			}
			if (ntfsbuffer)
			{
				FreePool(ntfsbuffer);
			}

			fixSmallc = 0;
		}
		else
		{
			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区起始地址和结束
		//	*(UINT32*)(&buffer[0x1ca]) = (g_disklist[disknum].totalsector )&0xffffffffffffff00;
			*(UINT32*)(&buffer[0x1ca + i * 16]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin) & 0xfffffff0);
			fixSmallc = 1;
			MyLog(LOG_DEBUG, L"i =%d 0x1ca + i*16 = %d\n", i, 0x1ca + i * 16);
		}

		if (i < 3)
		{
			SetMem((buffer + 0x1be + i * 16 + 16), 16 * (3 - i), 0); //后面分区清0
			MyLog(LOG_DEBUG, L"SetMem i =%d 0x1be + i*16 = %d\n", i, 0x1be + i * 16);
		}
		//**************asdf

		if (DiskWrite((WORD)disknum, 0, 1, buffer) == FALSE)
		{
			MyLog(LOG_DEBUG, L"DiskWrite hdd mbr read error\n");
			if (buffer)
				FreePool(buffer);
			goto Done;
		}
	}

Done:
	if (buffer)
	{
		FreePool(buffer);
	}

	MyLog(LOG_DEBUG, L"FixNewHDD end.\n");
	//DrawMsgBoxEx(L"FixNewHDD end.", IDOK, -1, 1);
}


///////////////////////////////////////////////////////////////////////
///对应的磁盘layout： ESP   MSR   OS    DATA    WINRE
///定位到OS分区，之后查找OS分区之后的分区，处理WinRE分区的属性。
///disknum: 恢复磁盘号
///okrheader：存储备份文件的信息
void FixNewHDDPartitionTable(char disknum, OKR_HEADER *okrheader)
{
	BYTE *buffer = NULL;
	BYTE *ntfsbuffer = NULL;
	UINT32 Crc = 0;
	UINT64 BootDSecBegin = 0;
	int i = 0, j = 0, partcCount = 0;
	BYTE *head = 0;
	CHAR16 LenovoLabel[32] = { L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T' };//ysy

	gcount = 0xff;
	buffer = AllocatePool(GPT_SIZE);
	if (buffer == NULL)
	{
		return;
	}

	ntfsbuffer = AllocatePool(512);
	if (ntfsbuffer == NULL)
	{
		return;
	}

	if (IsGPT(disknum)) 
	{
		/////////////////GPT////////////////////////////

		//读取新硬盘分区表，从1扇区，读取到33扇区
		if (DiskRead((WORD)disknum, 1, GPT_SIZE / 512, buffer) == FALSE) 
		{
			DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING2, IDOK, IDCANCEL, TYPE_INFO);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//判断硬盘头是否为EFI PART
		if (buffer[0] != 'E' || buffer[1] != 'F' || buffer[2] != 'I' || buffer[3] != ' ' || buffer[4] != 'P' || buffer[5] != 'A' || buffer[6] != 'R' || buffer[7] != 'T')
		{
			MyLog(LOG_DEBUG, L"wrong efi header efi part\n");
			return;
		}

		buffer[0x10] = 0;
		buffer[0x11] = 0;
		buffer[0x12] = 0;
		buffer[0x13] = 0;

		//查找系统分区
		for (i = 0; i < okrheader->PartitionCount; i++)
		{
			if (MyMemCmp(Windows_Data_TypeGuid, okrheader->Partition[i].TypeGuid, 16) == 0 && okrheader->Partition[i].HasWindows == TRUE)
			{
				partcCount = i;

				MyLog(LOG_DEBUG, L"partcCount(OS par) IS %X\n", partcCount);
				break;
			}
		}


		//查找OS之后的分区
		for (i = partcCount; i < okrheader->PartitionCount; i++) 
		{
			MyLog(LOG_DEBUG, L"par i :%d\n", i);

			MyLog(LOG_DEBUG, L"partition %d, BootBegin 0x%llx, TotalSectors %lld MB.\n",
				i, okrheader->Partition[i].BootBegin, okrheader->Partition[i].TotalSectors/2/1024);
			MyLog(LOG_DEBUG, L"Partition[%d].bIfBackup %x.\n", i, okrheader->Partition[i].bIfBackup);
			MyLog(LOG_DEBUG, L"partcCount %x.\n", partcCount);
			MyLog(LOG_DEBUG, L"Label %s.\n", okrheader->Partition[i].Label);

			if (i > partcCount) 
			{
				MyLog(LOG_DEBUG, L"enter FixNewpartition HDD \n");
				//在当前分区，通过bootbegin，查找备份文件中os之后的分区内容。
				for (j = 512; j < GPT_SIZE; j = j + 128)
				{
					head = &buffer[j];
					if (*(UINT64*)(&head[0x20]))
					{
						MyLog(LOG_DEBUG, L"NewHdd par[%d] BootBegin :%llx end: %llx\n", (j - 512) / 128, *(UINT64*)(&head[0x20]), *(UINT64*)(&head[0x28]));
					}

					///
					if (*(UINT64*)(&head[0x20]) == okrheader->Partition[i].BootBegin
						|| *(UINT64*)(&head[0x20]) == okrheader->Partition[i].BootBegin + 0x20)
					{
						MyLog(LOG_DEBUG, L"find the par. \n");
						break;
					}
				}

				MyLog(LOG_DEBUG, L"j :%llx\n", j);
				
				if (j < GPT_SIZE) 
				{
					//找到WinRE分区，更新分区的信息
					//目前进不到这个分支，winre信息已经清除。

					//判断是否为winre分区
					if (okrheader->Partition[i].bIfBackup &&
						MyMemCmp(Window_Recovery_TypeGuid, okrheader->Partition[i].TypeGuid, 16) == 0)
					{
						MyLog(LOG_DEBUG, L"wire is %x okrheader->Partition[i].TotalSectors %x \n", i, okrheader->Partition[i].TotalSectors);
						CopyMem(head, Window_Recovery_TypeGuid, 16);
						////0x22: GPT 分区表扇区数，34.

						//更新分区起始地址和结束
						//20～27 8bit 该分区的起始扇区，用LBA值表示
						*(UINT64*)(&head[0x20]) = BootDSecBegin = (g_disklist[disknum].totalsector - g_wireTotalSectors - 0x22) & 0xfffffffffffffff0;

						//28～2F 8bit 该分区的结束扇区(包含)，用LBA值表示，通常是奇数。
						*(UINT64*)(&head[0x28]) = (g_disklist[disknum].totalsector - 0x22) & 0xffffffffffffff00;

						*(UINT64*)(&head[0x30]) = 0x8000000000000001;

						//更新备份文件中winre分区的起始地址信息
						okrheader->Partition[i].BootBegin = *(UINT64*)(&head[0x20]);

						//格式化分区第一个扇区。防止出现错误的文件系统
						if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
						{
							MyLog(LOG_DEBUG, L"read ntfsbuffer error\n");
						}

						SetMem(ntfsbuffer, 512, 0);
						if (DiskWrite((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
						{
							MyLog(LOG_DEBUG, L"write ntfsbuffer error\n");
						}

						for (j = j + 128; j < GPT_SIZE; j++) 
						{
							buffer[j] = 0;
						}
						break;
					}
					else 
					{
						MyLog(LOG_DEBUG, L"==par[%d] bb:%llx end:%llx\n", (j - 512) / 128, *(UINT64*)(&head[0x20]), *(UINT64*)(&head[0x28]));
						
						//*(UINT64*)(&head[0x20]) = 0;
						//*(UINT64*)(&head[0x28]) = 0;
					}
				}
				else 
				{
					//当前磁盘分区表，没找到WinRE分区，有空余空间，新建分区

					//判断是否为winre分区
					if (okrheader->Partition[i].bIfBackup &&
						MyMemCmp(Window_Recovery_TypeGuid, okrheader->Partition[i].TypeGuid, 16) == 0)
					{
						//MyLog(LOG_DEBUG, L"okrheader->Partition[%d].bIfBackup  %x \n", i, okrheader->Partition[i].bIfBackup);

						//查找分区表的结尾位置？
						for (j = 512; j < GPT_SIZE; j = j + 128) {
							head = &buffer[j];
							MyLog(LOG_DEBUG, L"partition BootBegin :%llx  j %x \n", *(UINT64*)(&head[0x20], j));

							if (*(UINT64*)(&head[0x20]) == 0 || *(UINT64*)(&head[0x28]) == 0) {
								break;
							}
						}

						if (j < GPT_SIZE) 
						{
							CopyMem(head, Window_Recovery_TypeGuid, 16);
							MyLog(LOG_DEBUG, L"Window_Recovery_TypeGuid okrheader->Partition[i].TotalSectors %llx ", okrheader->Partition[i].TotalSectors);
							*(UINT64*)(&head[0x20]) = BootDSecBegin = (g_disklist[disknum].totalsector - g_wireTotalSectors - 0x22) & 0xfffffffffffffff0;
							*(UINT64*)(&head[0x28]) = (g_disklist[disknum].totalsector - 0x22) & 0xffffffffffffff00;
							*(UINT64*)(&head[0x30]) = 0x8000000000000001;

							//更新备份文件中winre分区的起始地址信息
							okrheader->Partition[i].BootBegin = *(UINT64*)(&head[0x20]);

							//格式化分区第一个扇区。防止出现错误的文件系统
							if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
							{
								MyLog(LOG_DEBUG, L"read ntfsbuffer error\n");
							}

							SetMem(ntfsbuffer, 512, 0);
							if (DiskWrite((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
							{
								MyLog(LOG_DEBUG, L"write ntfsbuffer error\n");
							}

							MyLog(LOG_DEBUG, L"j  %x ntfsbuffer \n", j);

							for (j = j + 128; j < GPT_SIZE; j++)
							{
								buffer[j] = 0;
							}

							break;
						}
					}
				}
			}
		}

		gBS->CalculateCrc32(buffer + 512, 16384, &Crc);
		*(UINT32*)(&buffer[0x58]) = Crc;

		*(UINT32*)(&buffer[0x10]) = 0;

		gBS->CalculateCrc32(buffer, 0x5c, &Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;

		//更新磁盘分区表的数据
		if (DiskWrite((WORD)disknum, 1, GPT_SIZE / 512, buffer) == FALSE)
		{
			//DrawMsgBox(STRING_CREATE_DIR_ERROR5, STRING_YES, STRING_NO);
		  //MyLog(LOG_DEBUG, L"DiskWrite failed 4\n");
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//update backup sector 
		BYTE *backup = NULL;
		//更新分区表备份位置信息  LBA2-LBA34
		backup = buffer + 512;
		if (DiskWrite((WORD)disknum, g_disklist[disknum].totalsector - 33, 32, backup) == FALSE)
		{
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//更新1号扇区备份位置信息  LBA1
		//交换0x18与0x20的值。
		backup = buffer;
		UINT64 temp = *(UINT64*)(&buffer[0x20]);
		*(UINT64*)(&buffer[0x20]) = *(UINT64*)(&buffer[0x18]);
		*(UINT64*)(&buffer[0x18]) = temp;

		*(UINT64*)(&buffer[0x48]) = *(UINT64*)(&buffer[0x30]) + 0x1;

		//gBS->CalculateCrc32(buffer + 512, 16384, &Crc);
		//*(UINT32*)(&buffer[0x58]) = Crc;

		*(UINT32*)(&buffer[0x10]) = 0;

		MyLog(LOG_DEBUG, L"crc : %llx\n", *(UINT32*)(&buffer[0x10]));

		gBS->CalculateCrc32(buffer, 0x5c, &Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;

		MyLog(LOG_DEBUG, L"crc : %llx\n", *(UINT32*)(&buffer[0x10]));

		if (DiskWrite((WORD)disknum, g_disklist[disknum].totalsector - 1, 1, backup) == FALSE)
		{
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);
			if (buffer)
			{
				FreePool(buffer);
			}
			return;
		}

		//输出log
		for (j = 512; j < GPT_SIZE; j = j + 128)
		{
			head = &buffer[j];
			if (*(UINT64*)(&head[0x20]))
			{
				MyLog(LOG_DEBUG, L"par[%d] bb:%llx end:%llx\n", (j - 512) / 128, *(UINT64*)(&head[0x20]), *(UINT64*)(&head[0x28]));
			}
		}
	}
	
	if (buffer)
	{
		FreePool(buffer);
	}

	if (ntfsbuffer)
	{
		FreePool(ntfsbuffer);
	}
	
	//DrawMsgBoxEx(L"FixNewHDDPartitionTable end.", IDOK, -1, 1);
}	

///////////////////////////////////////////////////////////////////////
//iDestDiskNum - 恢复硬盘索引
//fileindex - 镜像文件的头结构的索引号 g_data.images[fileindex]
//Return value: 
//0: recovery; 
//-1: STRING_RAID_BK_NO_RAID_DISK_RECOVERY_ERROR
//-2: STRING_RAID_DISK_NO_RAID_BK_RECOVERY_ERROR
//-3: STRING_RAID_BK_INFO_NO_MATCH_RECOVERY_ERROR
int CheckRAIDDiskRecovery(WORD iDestDiskNum, DWORD fileindex)
{
	MyLog(LOG_DEBUG, L"CheckRAIDDiskRecovery partindex: %d, fileindex: %d.\n", iDestDiskNum, fileindex);

	int iRet = 0;
	OKR_HEADER *Header = NULL;
	CHAR8 *TempBuffer = NULL;
	CHAR8  systemDisksn[20] = { 0 };

	BOOL bIsRAIDBackupFile = FALSE;

	Header = &(g_data.images[fileindex]);
	TempBuffer = (CHAR8 *)Header->Memo + 100 + 12;
	for (int j = 0; j < 20; j++)
	{
		systemDisksn[j] = TempBuffer[j];
	}

	MyLog(LOG_DEBUG, L"systemDisksn: [%a].\n", systemDisksn);

	if (systemDisksn[0] == 'R' &&
		systemDisksn[1] == 'A' &&
		systemDisksn[2] == 'I' &&
		systemDisksn[3] == 'D')
	{
		bIsRAIDBackupFile = TRUE;
	}

	MyLog(LOG_DEBUG, L"isRAIDDisk: %d\n", g_disklist[iDestDiskNum].isRAIDDisk);

	if (bIsRAIDBackupFile)
	{
		if (1 == g_disklist[iDestDiskNum].isRAIDDisk)
		{
			//RAID mode
			MyLog(LOG_DEBUG, L"CompareHddSN raidDisksn: %a, systemDisksn: %a\n", g_disklist[iDestDiskNum].raidDisksn, systemDisksn);
			if (1 != CompareHddSN(g_disklist[iDestDiskNum].raidDisksn, systemDisksn))
			{
				iRet = -3;
			}
			else
			{
				iRet = 0;
			}
		}
		else
		{
			//Error, can not recovery
			iRet = -1;
		}
	}
	else
	{
		if (1 == g_disklist[iDestDiskNum].isRAIDDisk)
		{
			//Error, can not recovery
			iRet = -2;
		}
		else
		{
			//Normal
			iRet = 0;
		}
	}

	MyLog(LOG_DEBUG, L"CheckRAIDDiskRecovery ret: %d.\n", iRet);
	return iRet;
}

////////////////////////////////////////////////////////////////////////////////		
extern int sysdisktype_in_backupfile ;
//恢复的时候使用多线程，一个线程读文件，一个线程写硬盘
//partindex - 镜像文件所在的分区index
//fileindex - 镜像文件的头结构的索引号 g_data.images[fileindex]
extern DWORD	OldStructSize ;
extern int OldImage  ;			////备份文件是否为旧版本。1：旧版本（0x09000810，AppVer: 9.2.0）。 0：新版本（0x09000811）。
int Recover(WORD partindex, DWORD fileindex, PCONTROL pctl)
{
	MyLog(LOG_DEBUG, L"Recover...\n");

	OKR_HEADER *header = NULL;
	CHAR16 fullpath[128] = { 0 };
	CHAR16 *filename = NULL;
	UINT64 filesize = 0;
	int ret = -1;

	int olddisk = 0;			///是否恢复到原硬盘。1：是  2：不是  -1：未获取
	int klchange = 0;			///备份点中标记为备份的分区，是否都可以找到。1：不是，磁盘有变动  0：是

	BOOL bRet = 0, bPartitionChanged = 0, bShowError = TRUE;
	BOOL bPartModify = FALSE;

	DWORD i = 0;
	struct CONTINUE_RWDISK_INFO conti_info;
	PBUFFER_INFO buffinfo;
	struct BUFFERARRAY buffarray;
	struct MP_INFO mpinfo;
	FILE_HANDLE_INFO filehandle;
	FILE_SYS_INFO  sysinfo;
	VOID *bitbuffer = NULL;
	UINT64 after_alignlen = 0;
	UINT64 curlen = 0;
	BYTE ntfsbuff[512] = { 0 };

	OldImage = 0;

	if (!CheckHandleValid(g_Partitions[partindex].Handle))
	{
		SET_ERROR(__LINE__, OKR_ERROR_FILE_SYSTEM);
		DrawMsgBoxEx(STRING_MEDIA_ERROR, IDOK, -1, TYPE_INFO);
		return -1;
	}

	g_data.Cmd = USER_COMMAND_RECOVER;
	g_data.bCompleted = FALSE;
	g_data.ImageDiskNumber = g_Partitions[partindex].DiskNumber;
	g_data.dwErrorCode = 0;
	filename = (CHAR16 *)g_data.images[fileindex].FileName;

	InitMp(&mpinfo);

	header = myalloc(OKR_HEADER_MAX_SIZE);
	if (!header)
	{
		SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
		goto Done;
	}

	if (FileSystemInit(&sysinfo, partindex, FALSE) == FALSE)
	{
		if (FileSystemInit(&sysinfo, partindex, TRUE) == FALSE)
		{
			SET_ERROR(__LINE__, OKR_ERROR_FILE_SYSTEM);
			goto  Done;
		}
	}
	else
	{
		logopen(&sysinfo, partindex);
	}

	MyLog(LOG_DEBUG, L"Recover index:%d,filenum %d\n", fileindex, g_data.images[fileindex].FileNumber);

	GetBackupFileFullPath(&sysinfo, partindex, filename, 0, fullpath, 128);
	
	//针对多个fat32备份文件拷贝NTFS分区
	if (g_data.images[fileindex].FileNumber > 1 && sysinfo.ifntfs)
	{
		if (!SegFileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE))
		{
			SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
			goto  Done;
		}
	}
	else
	{
		if (!FileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE))
		{
			SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
			goto  Done;
		}
	}

	// curlen += filesize;
	filesize = OKR_HEADER_MAX_SIZE;
	bRet = FileRead(&filehandle, (UINTN*)&filesize, (PBYTE)header);
	if (!bRet || filesize != OKR_HEADER_MAX_SIZE)
	{
		SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
		goto  Done;
	}

	if (header->Magic != OKR_IMAGE_MAGIC || header->bComplete != TRUE)
	{
		SET_ERROR(__LINE__, OKR_ERROR_INVALID_IMAGE_FILE);
		goto  Done;
	}

	MyLog(LOG_DEBUG, L"Read0 Partition(TotalSectors:%ld), partition %d\n", header->Partition[1].TotalSectors, 1);
	g_data.CompressBlockSize = header->CompressBlockSize;
	FileClose(&filehandle);

	//恢复时Init_BufferArray分配的buffer大小最小为CompressBlockSize
	if (Init_BufferArray(&buffarray) == FALSE)
	{
		SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
		goto Done;
	}

	g_lz4_buffer = myalloc(g_data.CompressBlockSize + ALGIN_SIZE_BYTE);
	if (!g_lz4_buffer)
	{
		SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
		goto Done;
	}

	MyLog(LOG_DEBUG, L"Restoring(multithread %d)...\n", g_data.bMultiThread);
	MyLog(LOG_DEBUG, L"CompressUnitSize 0x%x, DataOffset 0x%x\n",
		g_data.CompressBlockSize, header->DataOffset);

	MyLog(LOG_DEBUG, L"Checking partitions(num:%d)...\n", header->PartitionCount);

	//检查目标硬盘是否有足够的空间进行恢复，如果目标硬盘太小，不够存储所有分区信息，则返回失败
	//g_data.selecteddisk = -1;
	//g_data.comefromnewhddface = 0;
	destpartindex = -1;
	if (g_data.comefromnewhddface == 0)
	{
		if (!GetDestinationDisk(header, &olddisk))
		{
			MyLog(LOG_DEBUG, L"GetDestinationDisk return FALSE.\n");

			bPartModify = TRUE;
			goto Done;
		}
	}
	else
	{
		if (!CheckDestinationDisk(header))
		{
			MyLog(LOG_DEBUG, L"CheckDestinationDisk return FALSE.\n");
			bPartModify = TRUE;
			goto Done;
		}
	}

	int nRet = CheckRAIDDiskRecovery(g_data.RecoverDiskNumber, fileindex);
	if (-1 == nRet)
	{
		DrawMsgBoxEx(STRING_RAID_BK_NO_RAID_DISK_RECOVERY_ERROR, IDOK, -1, TYPE_WARNING);
		ret = -1;
		bShowError = FALSE;
		MyLog(LOG_DEBUG, L"STRING_RAID_BK_NO_RAID_DISK_RECOVERY_ERROR, No go on \n");
		goto Done;
	}
	else if (-2 == nRet)
	{
		DrawMsgBoxEx(STRING_RAID_DISK_NO_RAID_BK_RECOVERY_ERROR, IDOK, -1, TYPE_WARNING);
		ret = -1;
		bShowError = FALSE;
		MyLog(LOG_DEBUG, L"STRING_RAID_DISK_NO_RAID_BK_RECOVERY_ERROR, No go on \n");
		goto Done;
	}
	else if (-3 == nRet)
	{
		DrawMsgBoxEx(STRING_RAID_BK_INFO_NO_MATCH_RECOVERY_ERROR, IDOK, -1, TYPE_WARNING);
		ret = -1;
		bShowError = FALSE;
		MyLog(LOG_DEBUG, L"STRING_RAID_BK_INFO_NO_MATCH_RECOVERY_ERROR, No go on \n");
		goto Done;
	}

	if (g_data.bIsGSKU && (0 == GetRecoverableDiskNumber()))
	{
		MyLog(LOG_DEBUG, L"Destination disk not found.\n");
		if (g_data.bIsAutoFunc)
		{
			DrawMsgBoxEx(STRING_AUTOBR_DISK_NOT_FOUND_ERROR, IDOK, -1, TYPE_INFO);
		}
		else
		{
			DrawMsgBoxEx(STRING_DISK_NOT_FOUND_ERROR, IDOK, -1, TYPE_INFO);
		}

		ret = -1;
		bShowError = FALSE;
		MyLog(LOG_DEBUG, L"STRING_DISK_NOT_FOUND_ERROR, No go on \n");
		goto Done;
	}

	if (sysdisktype_in_backupfile == 2 && !g_data.bIsAutoFunc)  //sata 备份到sata盘
	{
		if (g_disklist[g_data.RecoverDiskNumber].disksn[0] != 0 && g_disklist[g_data.RecoverDiskNumber].disksn[0] != 0x20) //nvme  恢复到nvme盘
		{
			// if(g_data.comefromnewhddface)//ysy 0110
			if (DrawMsgBoxEx(STRING_NVME_TIPS, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK)//nvme 提示
			{
				bShowError = FALSE;
				MyLog(LOG_DEBUG, L"STRING_NVME_TIPS ,No go on \n");
				goto Done;
			}
		}
	}

	//检查是否存在Bitlocker加密数据分区
	if (g_data.comefromnewhddface == 0)
	{
		if (TRUE == IfDataParExistBitlocker(header, fileindex) && !g_data.bForceRestorePartition)
		{

			if (g_data.bIsAutoFunc)
			{
				if (DrawMsgBoxEx(STRING_AUTOBR_BITLOCKER_DATA_PAR, IDB_AUTO_REBOOT, IDB_AUTO_CONTINUE, TYPE_INFO) != IDB_AUTO_CONTINUE) //提示解密Bitlocker
				//if (DrawMsgBoxTwoLine(STRING_AUTOBR_BITLOCKER_DATA_PAR, STRING_CLOSE_BITLOCKER, IDB_AUTO_REBOOT, IDB_AUTO_CONTINUE, TYPE_INFO) != IDB_AUTO_CONTINUE) //提示解密Bitlocker
				{
					bShowError = FALSE;
					MyLog(LOG_DEBUG, L"Data par has Bitlocker, no go on.\n");
					goto Done;
				}
				else
				{
					if (DrawMsgBoxExWithQRCode(STRING_AUTOBR_BITLOCKER_FIND_KEY, IDB_CONTINUE, IDCANCEL, TYPE_INFO, IMG_FILE_FIND_BITLOCKER_KEY, 200) != IDB_CONTINUE) //提示查找Bitlocker Key
					{
						bShowError = FALSE;
						MyLog(LOG_DEBUG, L"Find Key Cancel, no go on.\n");
						goto Done;
					}
					else
					{
						g_data.bForceRestorePartition = TRUE;
						MyLog(LOG_DEBUG, L"[AUTOBR]Set g_data.bForceRestorePartition = TRUE\n");
					}
				}
			}
			else
			{
				if (DrawMsgBoxTwoLine(STRING_BITLOCKER_DATA_PAR, STRING_CLOSE_BITLOCKER, IDCANCEL, IDB_CONTINUE, TYPE_INFO) != IDB_CONTINUE) //提示解密Bitlocker
				{
					bShowError = FALSE;
					MyLog(LOG_DEBUG, L"Data par has Bitlocker, no go on.\n");
					goto Done;
				}
				else if (DrawMsgBoxExWithQRCode(STRING_BITLOCKER_FIND_KEY, IDB_RUN_RESTORE, IDCANCEL, TYPE_INFO, IMG_FILE_FIND_BITLOCKER_KEY, 200) != IDB_RUN_RESTORE) //提示查找Bitlocker Key
				{
					bShowError = FALSE;
					MyLog(LOG_DEBUG, L"Find Key Cancel, no go on.\n");
					goto Done;
				}
			}
		}
	}

	//	MyLog(LOG_DEBUG,L"g_data.destdisktype = %d\n", g_data.destdisktype);
	//	MyLog(LOG_DEBUG,L"hddface =%d\n", g_data.comefromnewhddface);	
	MyLog(LOG_DEBUG, L"olddisk final = %d..\n", olddisk);
	//	 MyLog(LOG_DEBUG,L"g_data.bForceRestorePartition = %r..\n", g_data.bForceRestorePartition);
		//检查是否需要恢复分区表
	if (g_data.bFactoryRestore) 
	{

		//工厂恢复模式则强制恢复分区表
		bPartitionChanged = TRUE;
		g_data.bForceRestorePartition = TRUE;
	}
	else {

		bPartitionChanged = IfNeedRecoverPartTable_delete(header, &klchange);
		MyLog(LOG_DEBUG, L"IfNeedRecoverPartTable_delete bPartitionChanged =%d.\n", bPartitionChanged);
		if (!g_data.bForceRestorePartition)
		{
			MyLog(LOG_DEBUG,L"klchange  %d\n",klchange);	

			if (klchange)
			{
				if (g_data.bIsAutoFunc)
				{					
					DrawMsgBoxEx(STRING_AUTOBR_PARTITION_CHANGED, IDOK, -1, TYPE_INFO);
				}
				else
				{
					DrawMsgBoxEx(STRING_WELCOME, IDOK, -1, TYPE_INFO);
				}
				//SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
				bShowError = FALSE;
				goto Done;

			}
			else
			{
				bPartitionChanged = IfNeedRecoverPartTable(header, &klchange);
				MyLog(LOG_DEBUG, L"IfNeedRecoverPartTable bPartitionChanged =%d.\n", bPartitionChanged);
			}

		}

	}
	MyLog(LOG_DEBUG, L"\n bPartitionChanged =%d!\n", bPartitionChanged);
	if (g_data.comefromnewhddface == 0 && (olddisk == 2 || olddisk == 1))
	{

		MyLog(LOG_DEBUG, L"Done GUID   OldImage =%d\n", OldImage);
		//kang ++++>
		if (OldImage == 0)
		{
			MyLog(LOG_DEBUG, L"Done GUID   OldImage 2=%d\n", OldImage);
			if (g_data.bForceRestorePartition == FALSE)
			{
				for (i = 0; i < g_dwTotalPartitions; i++)
				{
					if (g_Partitions[i].HasWindows && g_Partitions[i].DiskNumber == g_data.RecoverDiskNumber)
						break;//hdd guid 或者分区头起始地址变化，导致c盘恢复后与分区表不一致（用户重新安装系统）,会死机，提示无法恢复

				}
				if (i < g_dwTotalPartitions)
				{

					MyLog(LOG_DEBUG, L"Done GUID   i =%d\n", i);
					MyLog(LOG_DEBUG, L"recovery disk find windows !\n");

					if (DiskRead((WORD)g_data.RecoverDiskNumber, g_Partitions[i].BootBegin, 1, ntfsbuff) == FALSE) {
						goto  Stepgoon;
					}

					if (header->WinPartNtfsGuid[0] == ntfsbuff[0x48] && \
						header->WinPartNtfsGuid[1] == ntfsbuff[0x49] && \
						header->WinPartNtfsGuid[2] == ntfsbuff[0x4a] && \
						header->WinPartNtfsGuid[3] == ntfsbuff[0x4b] && \
						header->WinPartNtfsGuid[4] == ntfsbuff[0x4c] && \
						header->WinPartNtfsGuid[5] == ntfsbuff[0x4d] && \
						header->WinPartNtfsGuid[6] == ntfsbuff[0x4e] && \
						header->WinPartNtfsGuid[7] == ntfsbuff[0x4f])

					{
						MyLog(LOG_DEBUG, L"Done GUID   i = equ \n");
					}
					else {
						if (g_data.bIsAutoFunc)
						{
							DrawMsgBoxEx(STRING_AUTOBR_PARTITION_CHANGED, IDOK, -1, TYPE_INFO);
						}
						else
						{
							DrawMsgBoxEx(STRING_UUID_ERROR, IDOK, -1, TYPE_INFO);
						}
						bShowError = FALSE;
						ret = -1;

						MyLog(LOG_DEBUG, L"Done GUID PartC Serial Num changed ,No go on \n");
						goto Done;

					}
				}
			}
		}

	Stepgoon:
		//kang ++++<
		if (bPartitionChanged)
		{
			if (g_data.bForceRestorePartition == FALSE)
			{

				if (OldImage == 0)
				{

					if (CheckPartitionStartPosition(header) == FALSE || !CompareGuid((EFI_GUID*)header->SourceDiskGUID, (EFI_GUID*)g_disklist[g_data.RecoverDiskNumber].guid)) //hdd guid 或者分区头起始地址变化，导致c盘恢复后与分区表不一致（用户重新安装系统）,会死机，提示无法恢复
					{

						MyLog(LOG_DEBUG, L"header->SourceDiskGUID %x %x %x-%x %x %x\n", header->SourceDiskGUID[0], header->SourceDiskGUID[1], header->SourceDiskGUID[2], header->SourceDiskGUID[13], header->SourceDiskGUID[14], header->SourceDiskGUID[15]);
						MyLog(LOG_DEBUG, L"DestDisk.guid= %x %x %x-%x %x %x\n", g_disklist[g_data.RecoverDiskNumber].guid[0], g_disklist[g_data.RecoverDiskNumber].guid[1], g_disklist[g_data.RecoverDiskNumber].guid[2], g_disklist[g_data.RecoverDiskNumber].guid[13], g_disklist[g_data.RecoverDiskNumber].guid[14], g_disklist[g_data.RecoverDiskNumber].guid[15]);

						if (g_data.bIsAutoFunc)
						{
							DrawMsgBoxEx(STRING_AUTOBR_PARTITION_CHANGED, IDOK, -1, TYPE_INFO);
						}
						else
						{
							DrawMsgBoxEx(STRING_UUID_ERROR, IDOK, -1, TYPE_INFO);
						}
						bShowError = FALSE;
						ret = -1;

						MyLog(LOG_DEBUG, L"Done GUID PartC Start Address changed ,No go on \n");
						goto Done;
					}
				}

				//if (DrawMsgBoxEx(STRING_C_PART_CHANGE_GOON, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK)
				//{
				//	bShowError = FALSE;
				//	ret = -1;
				//	MyLog(LOG_DEBUG, L"Done PartC changed ,No go on \n");
				//	goto Done;
				//}
			}

			//if (DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK) //免责提示
			//{
			//	bShowError = FALSE;
			//	MyLog(LOG_DEBUG, L"Done PartC changed,agree? ,No go on \n");
			//	goto Done;
			//}

		}

		//		if(DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK)//免责声明
			//	{
		//			MyLog(LOG_DEBUG,L"Done PartC changed warning,No go on \n");
		//			bShowError = FALSE;
		//			goto Done;
		//		}

	}

	if (g_data.comefromnewhddface == 0)
	{
		if (FALSE == g_data.bForceRestorePartition && !g_data.bIsAutoFunc)
		{
			if (DrawMsgBoxEx(STRING_R_SYS_HINT, IDOK, IDCANCEL, TYPE_WARNING) != IDOK)
			{
				bShowError = FALSE;
				goto Done;
			}
		}
	}
	else
	{
		if (1 == g_data.comefromnewhddface)
		{
			if (NEW_FOUND_DISK == g_data.destdisktype)
			{
				if (DrawMsgBoxEx(STRING_RECOVER_NEW_DISK_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK) //免责提示
				{
					bShowError = FALSE;
					goto Done;
				}
			}

			if (DISK_SYSDISK == g_data.destdisktype && !g_data.bIsAutoFunc)
			{
				if (DrawMsgBoxEx(STRING_R_SYS_HINT, IDOK, IDCANCEL, TYPE_WARNING) != IDOK)
				{
					bShowError = FALSE;
					goto Done;
				}
			}
		}
	}

	//////Update UI for valid image
	g_data.BeginTime = GetTickCount() / 1000;
	g_data.LastTime = 0;

	StringInfo stStringInfoREMAIN_TIME;
	stStringInfoREMAIN_TIME.enumFontType = FZLTHJW;
	stStringInfoREMAIN_TIME.iFontSize = CalcFontSize(40);

	WORD dwWidth = 0;
	GetStringLen(STRING_VALID_IMAGE_FILE, &stStringInfoREMAIN_TIME, &dwWidth);

	WORD dwLeft = (g_WindowWidth - dwWidth) / 2;
	float fMinscale = g_data.fMinscale;
	WORD dwTop = REC_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
	WORD dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_REAMIN_TIME_TEXT_TOP - 150) * fMinscale;
	}
	FillRect(dwLeft - 100, dwTop - 10, dwWidth + 200, dwHeight + 20, 0xFFFFFF);
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_VALID_IMAGE_FILE, &stStringInfoREMAIN_TIME, TRUE);

	//Check File value
	BOOL bCheckRet = ValidImageCheckValue(&sysinfo, fullpath, g_data.images[fileindex].FileNumber);
	if (FALSE == bCheckRet)
	{
		if (g_bUserBreak)
		{
			MyLog(LOG_DEBUG, L"User break.\n");
			bShowError = FALSE;
			g_bUserBreak = FALSE;
			goto Done;
		}

		MyLog(LOG_DEBUG, L"ValidImageCheckValue Error.\n");
		DrawMsgBoxEx(STRING_VALID_IMAGE_CHECK_VALUE_ERROR, IDOK, -1, TYPE_INFO);
		bShowError = FALSE;
		goto  Done;
	}

	if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1) //kang 
	{
		bPartitionChanged = TRUE;
		g_data.bForceRestorePartition = TRUE;
	}
	//MyLog(LOG_DEBUG,L"2\n");
	if (g_data.destdisktype == DISK_SYSDISK && g_data.comefromnewhddface == 1) //kang 
	{
		bPartitionChanged = FALSE;
	}
	
	///////////////////////////////Start recovery///////////////////////////////////////
	//针对多个fat32备份文件拷贝NTFS分区
	if (g_data.images[fileindex].FileNumber > 1 && sysinfo.ifntfs)
	{
		if (!SegFileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE))
		{
			SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
			goto  Done;
		}
	}
	else
	{
		if (!FileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE))
		{
			SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
			goto  Done;
		}
	}

	//if (bPartitionChanged == TRUE || g_data.bForceRestorePartition) {
	{
		//不管怎样，都要恢复Grub
		BOOL needrecover = FALSE;
		BYTE *databuffer = g_lz4_buffer;
		BYTE *mbr = g_lz4_buffer + 0x100000;	//+1MB, GPT_DATA_SIZE*SECTOR_SIZE;
		ULONGLONG firstsector = GetFirstPartitionBeginSector(g_data.RecoverDiskNumber);

		filesize = header->GptSize;

		if (OldImage == 1)
		{
			if (!FileSetPos(&filehandle, OldStructSize))
			{
				goto  Done;
			}
		}
		else
		{
			if (!FileSetPos(&filehandle, header->StructSize))
			{
				goto  Done;
			}
		}

		bRet = FileRead(&filehandle, (UINTN*)&filesize, databuffer);
		if (!bRet || filesize != header->GptSize) {	//GPT_DATA_SIZE * 512
			SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
			goto  Done;
		}

		if (DiskRead((WORD)g_data.RecoverDiskNumber, 0, 1, mbr) == FALSE) {
			SET_ERROR(__LINE__, OKR_ERROR_READ_DISK);
			goto  Done;
		}

		if (!bPartitionChanged && !header->bGpt) {
			bPartitionChanged = RecheckPartitionTable(header, mbr, databuffer);
		}

		if (bPartitionChanged)
		{
			MyLog(LOG_DEBUG, L"bPartitionChanged true\n");
		}
		else
		{
			MyLog(LOG_DEBUG, L"bPartitionChanged false \n");
		}

		if (header->bGpt)
		{
			MyLog(LOG_DEBUG, L"bGpt true\n");
		}
		else
		{
			MyLog(LOG_DEBUG, L"bGpt false\n");
		}

		//gBS->Stall(0x1000000);


		if (bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 0) {


			if (g_data.bForceRestorePartition == FALSE)
			{
				//如果分区表发生变化，但是没有允许强制恢复分区表，则弹出提示，不允许恢复
				MyLog(LOG_DEBUG, L"Done bForceRestorePartition %d\n", g_data.bForceRestorePartition);
				if (g_data.bIsAutoFunc)
				{
					DrawMsgBoxEx(STRING_AUTOBR_PARTITION_CHANGED, IDOK, -1, TYPE_INFO);
				}
				else
				{
					DrawMsgBoxEx(STRING_PARTITION_CHANGED, IDOK, -1, TYPE_INFO);
				}
				SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
				bShowError = FALSE;
				goto Done;
			}
			else
			{
				if (DrawMsgBoxEx(STRING_PARTITION_CHANGED_FORCE_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK)
				{
					SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
					bShowError = FALSE;
					goto Done;
				}
			}
		}

		//如果分区发生变化，则需要用户确认是否恢复；分区信息没变化的话，则只恢复grub信息
		//如果是工厂恢复模式强制恢复分区表; 
		if (bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1)
		{
			needrecover = FALSE; //(BOOL)DrawMsgBoxEx(STRING_HINTPARTTABLE, IDOK, IDCANCEL, TYPE_ENQUIRE);;
		}
		else //if(g_data.bFactoryRestore)
		{
			needrecover = TRUE;
		}

		if (!IsGptSystem(g_data.RecoverDiskNumber) && header->bGpt)
		{
			needrecover = TRUE;
		}

		if (IsGptSystem(g_data.RecoverDiskNumber) && !(header->bGpt))
		{
			needrecover = TRUE;
		}

		//ctrl-f  强制恢复分区表，并不对其他分区更改

		if (g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
		{
			needrecover = TRUE;
		}


		if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1) //kang 
		{
			needrecover = TRUE;
		}

		if (g_data.destdisktype == DISK_SYSDISK && g_data.comefromnewhddface == 1) //kang 
		{
			needrecover = FALSE;

		}

		// MyLog(LOG_DEBUG,L"4\n");

		if (g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
		{
			bPartitionChanged = TRUE;
		}

		if (needrecover) {
			MyLog(LOG_DEBUG, L"====Recovery Disk Partition Table.\n");

			//恢复分区表或GRUB信息
			UINT64 sector = 0;
			DWORD number = 0;

			number = header->GptSize / SECTOR_SIZE;
			if (!bPartitionChanged && !g_data.bFactoryRestore) {

				//如果分区没变化，并且不是工厂恢复模式，则表示只恢复grub，
				//此时需要判断是否为GPT分区格式，如果是，则不要恢复grub
				if (header->bGpt || IsGptSystem(g_data.RecoverDiskNumber))
					number = 0;
				//	Print(L"number = %d \n",number);
				if (number)
				{
					CopyMem(databuffer + 0x1be, mbr + 0x1be, 0x40);
					//需要计算一下第一个分区的起始扇区位置
					if (firstsector && number > firstsector)
						number = firstsector;
				}
			}
			if (number) {
				MyLog(LOG_ERROR, L"====DiskWrite disk:%d sector: %llx num: %d \r\n", g_data.RecoverDiskNumber, sector, number, databuffer);
				if (DiskWrite((WORD)g_data.RecoverDiskNumber, sector, number, databuffer) == FALSE) 
				{
					SET_ERROR(__LINE__, OKR_ERROR_WRITE_GPT);
					goto  Done;
				}
			}
		}
		else {
			//MyLog(LOG_DEBUG,L"User select to not restore partition\n");
			//kang -- if(g_data.destdisktype == DISK_SYSDISK  && g_data.comefromnewhddface == 1) //kang 
			//kang -- ;
			//kang -- else
			//kang -- {
			//kang -- bShowError = FALSE;
			//kang -- 
			//kang -- goto  Done;
			//kang -- }
		}
	}

	fixSmallc = 0;
	int  index, disksrcnum, srcdisknume = 0;
	disksrcnum = g_data.ImageDiskNumber;
	srcdisknume = g_data.RecoverDiskNumber;

	if (g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE) 
	{
		MyLog(LOG_DEBUG, L"g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE\n");
		goto ForceComefrom0_0;
	}

	if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1) //kang 
	{
		MyLog(LOG_DEBUG, L"g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1\n");

		MyLog(LOG_DEBUG, L"g_data.BackupDiskNumber.totalsector  %llx g_data.RecoverDiskNumbe %llx \n", g_disklist[disksrcnum].totalsector, g_disklist[srcdisknume].totalsector);
		MyLog(LOG_DEBUG, L"g_data.RecoverDiskNumbe %llx \n", g_disklist[srcdisknume].totalsector);

		IsFixNewHddWay(header);
		MyLog(LOG_DEBUG, L"IsNoDataPartFlag ZWANG  %x \n", IsNoDataPartFlag);
		GetWirePartSector(header);
		if (TRUE == IsNoDataPartFlag)
		{
			//GetWirePartSector(header);

			FixNoDataPartTable(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector, header);
		}
		else
		{
			FixNewHDD(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector);

			if (g_WinREState != 0)
			{
				FixNewHDDPartitionTable(g_data.RecoverDiskNumber, header);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (g_data.comefromnewhddface == 0 && olddisk == 2)
	{
		MyLog(LOG_DEBUG, L"g_data.comefromnewhddface == 0 && olddisk == 2\n");
		FixNewHDD(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector);
	}

	//MyLog(LOG_DEBUG, L"5\n");

ForceComefrom0_0:
	MyLog(LOG_DEBUG, L"Resotring DataOffset %d.\n", header->DataOffset);
	//确认每个恢复分区，都在正确的位置。
	if (!FileSetPos(&filehandle, header->DataOffset ? header->DataOffset : OKR_HEADER_MAX_SIZE + GPT_OLD_DATA_SIZE * 512))
		goto  Done;

	//MyLog(LOG_DEBUG, L"6\n");

	filesize = header->FileSize;
	g_data.fileheader = header;

	curlen = header->DataOffset;

	if (g_data.bMultiThread && g_data.bAsyncIO && g_data.RecoverDiskNumber != g_Partitions[partindex].DiskNumber)
	{
		ProcessBarInit(pctl, filesize);
		logflush(FALSE);

		ret = RecoverWithAsyncIo(&filehandle, header, &buffarray, &mpinfo);
		goto Done;
	}

	logflush(FALSE);
	//	ProcessBarInit(pctl, header->OriginalSize + header->FileSize);

	if (g_data.images[fileindex].FileNumber > 1 && sysinfo.ifntfs == FALSE)
	{
		UINT32 datatotalsize = 0;
		UINT64 backupdatatotalsize = header->OriginalSize + header->FileSize;

		datatotalsize = ((backupdatatotalsize / 0x100000) / 0x400) / 2;

		if (datatotalsize >= 4 && datatotalsize <= 36)
		{
			backupdatatotalsize = (backupdatatotalsize*(68 - ((datatotalsize - 4) / 2))) / 100;
		}
		else if (datatotalsize > 36)
		{
			backupdatatotalsize = (backupdatatotalsize * 62) / 100;
		}

		ProcessBarInit(pctl, backupdatatotalsize);
	}
	else
	{
		ProcessBarInit(pctl, header->OriginalSize + header->FileSize);
	}

	MyLog(LOG_DEBUG, L"Resotring OriginalSize %d G, file %d G\n", (header->OriginalSize / 0x100000) / 0x400, (header->FileSize / 0x100000) / 0x400);

	for (i = 0; i < header->PartitionCount; i++) {
		INTN   size = 0;
		ULONGLONG totalblocks = 0, totaldatasize = 0, currentblock = 0, currentdata = 0, beginsec = 0, Lastblock_1 = 0;
		ULONGLONG origin_datasize = 0;
		ULONGLONG totalsec = 0;

		MyLog(LOG_DEBUG, L"Resotring partition %d, backup %d, size 0x%x.\n",
			i, header->Partition[i].bIfBackup, header->Partition[i].BackupDataSize);

		if (!header->Partition[i].bIfBackup || !header->Partition[i].BackupDataSize)
			continue;



		MyLog(LOG_DEBUG, L"Read Partition(TotalSectors:%ld), partition %d\n", header->Partition[i].TotalSectors, i);
		//calc bitmap size
		totalblocks = DivU64x32((header->Partition[i].TotalSectors + (BLOCK_SIZE - 1)), BLOCK_SIZE);
		size = DivU64x32((totalblocks + 7), 8);
		//对齐后的数据大小
		after_alignlen = CalcAliginSize(size);
		MyLog(LOG_DEBUG, L"Read bitmap(size:%ld) %ld,%ld\n", size, i, after_alignlen);
		//   getch();
		bitbuffer = myalloc(after_alignlen);
		if (bitbuffer == NULL) {
			SET_ERROR(__LINE__, OKR_ERROR_ALLOC_BITMAP);
			goto  Done;
		}
		totaldatasize = header->Partition[i].BackupDataSize - after_alignlen/*size*/;

		//先读取bitmap

		MyLog(LOG_DEBUG, L"Resbitmap curlen  %ld\n", curlen);


		//filesize = size;
		filesize = after_alignlen;
		bRet = FileRead(&filehandle, (UINTN*)&filesize, bitbuffer);//bitmap表因为整体写入，所以不会出现跨分段文件的情况
		if (!bRet || filesize != after_alignlen) {
			SET_ERROR(__LINE__, OKR_ERROR_READ_BITMAP);
			goto  Done;
		}
		curlen += after_alignlen;
		MyLog(LOG_DEBUG, L"Restoring curlen %ld\n", curlen);
		//统计数据总大小
		if (g_data.bDebug) {
			currentdata = 0;
			for (currentblock = 0; currentblock < totalblocks; currentblock++) {
				if (ReadBit(currentblock, bitbuffer))
					currentdata += BLOCK_SIZE * 512;
			}
		}


		beginsec = header->Partition[i].BootBegin;

		MyLog(LOG_DEBUG, L"Restor size:0x%lx - 0x%lx\n", totaldatasize, currentdata);
		origin_datasize = 0;
		logflush(FALSE);

		g_lz4_size = 8;
		currentblock = 0;
		currentdata = 0;
		Clean_BufferArray(&buffarray);
		SmartDiskIoInit((BYTE)g_data.RecoverDiskNumber, COMMAND_WRITE, &conti_info);
		while (1) {
			DWORD signature, readsize;
			lZ4_BUFFER_HEAD lz4_buffer_head;

			UINTN readn;

			if (!g_data.bIsAutoFunc && UserBreak()) {
				g_data.dwErrorCode = OKR_ERROR_USER_BREAK;
				bShowError = FALSE;
				goto Done;
			}

			if (g_data.bMultiThread) {
				buffinfo = GetConsumerAnyway(&buffarray);
				if (buffinfo->datavalid) {
					StartAP(&mpinfo, DecompressBuffer, buffinfo);
				}
			}
			buffinfo = GetProducter(&buffarray);

			if (currentdata < totaldatasize) {
				// readn = 4;
				readn = sizeof(lZ4_BUFFER_HEAD);
				MyLog(LOG_DEBUG, L"Restoring1 curlen  %ld\n", curlen);

				if (!FileRead(&filehandle, &readn, (PBYTE)&lz4_buffer_head)) {
					MyLog(LOG_DEBUG, L"Res FileRead1 error\n");

					SET_ERROR(__LINE__, OKR_ERROR_READ_SIGNATURE);
					bShowError = TRUE;

					goto  Done;
				}
			}
			else {
				readn = 0;
			}


			curlen += readn;
			if (readn) {
				if (lz4_buffer_head.compress_flag == COMPRESS_MAGIC || lz4_buffer_head.compress_flag == 0x676a6763)
					buffinfo->ifcompressed = TRUE;
				else if (lz4_buffer_head.compress_flag == UNCOMPRESS_MAGIC || lz4_buffer_head.compress_flag == 0x676a6762)
					buffinfo->ifcompressed = FALSE;
				else {
					MyLog(LOG_DEBUG, L"compress_flag error 0x%x\n", lz4_buffer_head.compress_flag);
					SET_ERROR(__LINE__, OKR_ERROR_INVALID_SIGNATURE);
					goto  Done;
				}
				if (lz4_buffer_head.lz4_size > buffinfo->totalsize)
				{
					SET_ERROR(__LINE__, OKR_ERROR_UNIT_SIZE);
					goto  Done;
				}
				/*
				readn = 4;
				if (!FileRead(&filehandle,&readn, (PBYTE)&readsize) || readsize > buffinfo->totalsize) {
					SET_ERROR(__LINE__, OKR_ERROR_UNIT_SIZE);
					goto  Done;
				}
				*/
				readsize = readn = lz4_buffer_head.align_lz4_size - sizeof(lZ4_BUFFER_HEAD);
				if (readn > g_data.CompressBlockSize + ALGIN_SIZE_BYTE)
				{
					MyLog(LOG_ERROR, L"Compress size beyond error:%ld,%ld\n", readn, g_data.CompressBlockSize);
					goto Done;
				}
				/*
						 curlen  = curlen - sizeof(lZ4_BUFFER_HEAD);

						 if (!FileSetPos(&filehandle,curlen))
							  goto  Done;

						 readsize = readn = lz4_buffer_head.align_lz4_size;

						 MyLog(LOG_DEBUG, L"Res2 curlen 0x%lx,readn 0x%lx\n", curlen,readn);

					   SetMem(TmpBuffer,g_data.CompressBlockSize + ALGIN_SIZE_BYTE,0);

							   if (!FileRead(&filehandle, &readn, TmpBuffer) || readsize != readn )
							   {

									  MyLog(LOG_DEBUG, L"Res FileRead2 error\n");

								   //必须完整的块，因为写入时，整块写入，不会出现分割点在一块压缩数据内的情况。
								   SET_ERROR(__LINE__, OKR_ERROR_READ_COMPRESS_UNIT);
								   goto  Done;
					   }

					   CopyMem(buffinfo->pointer,TmpBuffer+sizeof(lZ4_BUFFER_HEAD),lz4_buffer_head.align_lz4_size - sizeof(lZ4_BUFFER_HEAD));
				*/
				MyLog(LOG_DEBUG, L"Res2 curlen 0x%lx,readn 0x%lx\n", curlen, readn);
				if (!FileRead(&filehandle, &readn, buffinfo->pointer) || readsize != readn) {

					MyLog(LOG_DEBUG, L"Res FileRead2 error\n");

					//必须完整的块，因为写入时，整块写入，不会出现分割点在一块压缩数据内的情况。
					SET_ERROR(__LINE__, OKR_ERROR_READ_COMPRESS_UNIT);
					goto  Done;
				}

				//   MyLog(LOG_DEBUG, L"Res3 curlen,readn \n");

				curlen += readn;

				readn = lz4_buffer_head.align_lz4_size;
				currentdata += readn;
				//	buffinfo->usedsize = readn;
				buffinfo->usedsize = lz4_buffer_head.lz4_size;
				MyLog(LOG_DEBUG, L"read compress %ld, size=%ld ,last=%ld\n", buffinfo->ifcompressed, readn, totaldatasize - currentdata);

			}
			else {//end

		 //	MyLog(LOG_DEBUG,L"read 0 bytes\n");
				buffinfo->usedsize = 0;
			}

			//     MyLog(LOG_DEBUG, L"Res4 curlen,readn \n");

			buffinfo->datavalid = TRUE;
			if (g_data.bMultiThread)
				WaitToComplete(&mpinfo);
			else
				DecompressBuffer(buffinfo);

			//  MyLog(LOG_DEBUG, L"Res5 curlen,readn \n");

			origin_datasize += g_lz4_size;
			MyLog(LOG_DEBUG, L"decompressed bytes %ld,total size %ld\n", g_lz4_size, origin_datasize);
			if (!g_data.bIsAutoFunc && UserBreak()) {
				g_data.dwErrorCode = OKR_ERROR_USER_BREAK;
				bShowError = FALSE;
				goto Done;
			}

			if (g_lz4_size == 0) {

				//   MyLog(LOG_DEBUG, L"Res g_lz4_size is 0\n");
						//completed.
				break;
			}
			else if (g_lz4_size < 0) {
				SET_ERROR(__LINE__, OKR_ERROR_DECOMPRESS);
				goto  Done;
			}
			else if (g_lz4_size > 8) {

				UINT64 datasize = g_lz4_size;
				PBYTE p = g_lz4_buffer;
				int iRet = -1;
				while (datasize > 0) {
					if (currentblock >= totalblocks) {
						SET_ERROR(__LINE__, OKR_ERROR_COMPRESS_DATA_CORRUPTED);
						goto  Done;
					}
					if (ReadBit(currentblock, bitbuffer)) {
						DWORD secnum = BLOCK_SIZE;
						if (currentblock == totalblocks - 1) {//LAST BLOCK
							secnum = (DWORD)(header->Partition[i].TotalSectors - MultU64x32(currentblock, BLOCK_SIZE));
							//MyLog(LOG_DEBUG,L"lastblock=%d, filesize=%ld, totalblocks=%ld, index=%ld", secnum, filesize, totalblocks, currentblock);
						}


						if (g_data.needlimit == 1 && header->Partition[i].HasWindows) //kang +++
						{
							if (currentblock < (g_data.CurImageSyspartitionsize + 1)) //kang +++ 
							{
								iRet = SmartDiskIo(&conti_info, MultU64x32(currentblock, BLOCK_SIZE) + beginsec, secnum, p);
								if (FALSE == iRet)
								{
									ret = -1;
									MyLog(LOG_DEBUG, L"SmartDiskIo failed ret: %ld\n", iRet);
									SET_ERROR(__LINE__, OKR_ERROR_WRITE_GPT);
									goto  Done;
								}
							}
						}
						else
						{
							iRet = SmartDiskIo(&conti_info, MultU64x32(currentblock, BLOCK_SIZE) + beginsec, secnum, p);
							if (FALSE == iRet)
							{
								ret = -1;
								MyLog(LOG_DEBUG, L"SmartDiskIo failed ret: %ld\n", iRet);
								SET_ERROR(__LINE__, OKR_ERROR_WRITE_GPT);
								goto  Done;
							}
						}

						totalsec += secnum;
						p += BLOCK_SIZE * 512;
						datasize -= BLOCK_SIZE * 512;

						GoProcessBar(BLOCK_SIZE * 512);
					}
					currentblock++;
				}
				iRet = SmartDiskIo(&conti_info, 0, 0, NULL);
				if (FALSE == iRet)
				{
					ret = -1;
					MyLog(LOG_DEBUG, L"SmartDiskIo failed ret: %ld\n", iRet);
					SET_ERROR(__LINE__, OKR_ERROR_WRITE_GPT);

					goto  Done;
				}
			}

			g_lz4_size = 8;
		}

		//NextPart:
		MyLog(LOG_DEBUG, L"RestorePartition completed, total: 0x%lx\n", totalsec);

		myfree(bitbuffer);
		bitbuffer = NULL;
	}

	//MyLog(LOG_DEBUG,L"comefromnewhddface %d bForceRestore %d disktype:%d \n", g_data.comefromnewhddface, g_data.bForceRestorePartition, g_data.destdisktype);
	if (g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
	{
		MyLog(LOG_DEBUG, L"g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE\n");
		goto ForceComefrom0_1;
	}

	if (g_data.destdisktype == DISK_SYSDISK && g_data.comefromnewhddface == 1) //kang 
	{
		MyLog(LOG_DEBUG, L"g_data.destdisktype == DISK_SYSDISK && g_data.comefromnewhddface == 1\n");
		FixPartCSize(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector);
	}

	if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1) //kang 
	{
		MyLog(LOG_DEBUG, L"g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1\n");
		if (fixSmallc == 1)
			FixSmallDiskCSize(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector);
	}

	if (bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 2) //kang 
	{
		MyLog(LOG_DEBUG, L"bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 2\n");
		if (fixSmallc == 1)
			FixSmallDiskCSize(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector);
	}

	MyLog(LOG_DEBUG, L"check:b=%d,come=%d,olddisk=%d,oldisk_c_small=%d\n", bPartitionChanged, g_data.comefromnewhddface, olddisk, oldisk_c_small);
	BackupLastProcess();

ForceComefrom0_1:
	MyLog(LOG_DEBUG, L"Restore Done\n");

	// logflush(FALSE);
	  //FileClose(&filehandle);
	 //FileClose(&g_data.hLog);
	 //FileSystemClose(&sysinfo);

	if (g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
	{
		MyLog(LOG_DEBUG, L"g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE\n");
		goto ForceComefrom0_2;
	}

	if (bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1 && oldisk_c_small == 2) //kang 
	{
		MyLog(LOG_DEBUG, L"bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1 && oldisk_c_small == 2\n");
		//kang -- MyLog(LOG_DEBUG, L"Fix hdd  bitmap =0x%llx 0x%llx\n",g_data.PartCBootBegin, g_data.PartCTotalSector);
		volume_size = (INT64)MultU64x32(g_data.PartCTotalSector, 512);
		FixSmallDiskCSize_Bitmap(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector);
		SetNTFSVolDirty(g_data.RecoverDiskNumber, g_data.PartCBootBegin);

		DonotShowWaitMsgFace();

		DrawMsgBoxEx(STRING_DISK_CHECK_TIPS, IDOK, -1, TYPE_INFO);

	}

	if (bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1) //kang 
	{
		MyLog(LOG_DEBUG, L"bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1\n");
		FixPartCSize(g_data.RecoverDiskNumber, g_data.PartCBootBegin, g_data.PartCTotalSector);
	}

ForceComefrom0_2:
	DiskFlush(g_data.RecoverDiskNumber);
	GoProcessBar(-1);

	//kang -- MyLog(LOG_DEBUG, L"Fix hdd  begin =0x%llx 0x%llx\n",g_data.PartCBootBegin, g_data.PartCTotalSector);
		
	//延时1秒，让用户可以看到100%
	//delayEx(1000);
	//////////////////////////////////////////////////////////////////		
	if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1 && g_data.dwErrorCode == OKR_ERROR_SUCCESS) //kang 
	{
		//DrawMsgBox(STRING_RECOVER_NEW_DISK_SUC, STRING_YES, NULL);
		DrawMsgBoxEx(STRING_RECOVER_NEW_DISK_SUC, IDOK, -1, TYPE_INFO);
	}
	ret = 0;

Done:
	OldImage = 0;
	MyLog(LOG_DEBUG, L"g_data.destdisktype=%d.  \n ", g_data.destdisktype);
	MyLog(LOG_DEBUG, L"g_data.comefromnewhddface=%d\n", g_data.comefromnewhddface);
	MyLog(LOG_DEBUG, L"g_data.dwErrorCode=%d\n", g_data.dwErrorCode);
	/*   if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1 && g_data.dwErrorCode == OKR_ERROR_SUCCESS) //kang
	   {
		 //DrawMsgBox(STRING_RECOVER_NEW_DISK_SUC, STRING_YES, NULL);
		 DrawMsgBoxEx(STRING_RECOVER_NEW_DISK_SUC, IDOK, -1, TYPE_INFO);
	   }
	*/

	//	MyLog(LOG_DEBUG, L"Restore Done.\n");
	
	g_data.bForceRestorePartition = FALSE;

	if (bitbuffer)
		myfree(bitbuffer);
	g_data.databitmap = NULL;	//异步恢复时bitbuffer是临时分配的，不是放在全局变量里
	if (header)
		myfree(header);
	if (g_lz4_buffer)
		myfree(g_lz4_buffer);
	g_lz4_buffer = NULL;

	Destroy_BufferArray(&buffarray);

	//	MyLog(LOG_DEBUG, L"Restore Done 1\n");

	 //asdf  FileClose(&filehandle);

	if (ret < 0) 
	{
		FileClose(&filehandle);

		logflush(FALSE);
		FileClose(&g_data.hLog);
		FileSystemClose(&sysinfo);

		//kang -- MyLog(LOG_DEBUG, L"Restore failed with error %d, LineNumber %d\n", g_data.dwErrorCode, g_data.dwLineNumber);
	}
	else 
	{
		MyLog(LOG_DEBUG, L"Restore successfully\n");
	}
	//asdf	logflush(FALSE);
	//asdf	FileClose(&g_data.hLog);
	 //asdf   FileSystemClose(&sysinfo);

	if (ret < 0 && bShowError) 
	{
		WCHAR *szError = fullpath;
		SPrint(szError, 100, STRING_ERROR, g_data.dwErrorCode, g_data.dwLineNumber);
		//	DrawMsgBox(szError, STRING_YES, NULL);

		if (bPartModify == FALSE)
		{
			//todo: check if disk is exist.
			//DrawMsgBox(STRING_ERROR_READ_FILE, STRING_YES, NULL);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_ERROR_READ_FILE);
		}

	}

	if (bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1 && oldisk_c_small == 2)
	{
		//MyLog(LOG_DEBUG, L"sssssss\n");
	}
	else
	{
		FileClose(&filehandle);

		logflush(FALSE);
		FileClose(&g_data.hLog);
		FileSystemClose(&sysinfo);

	}

	//FileClose(&filehandle);
	//FileSystemClose(&sysinfo);
	DestroyProcessBar();

	//if (g_data.bDebug) 
	//{
	//	//MyLog(LOG_DEBUG, L"Press any key to continue.\n");
	//	//getch();
	//}
	g_data.Cmd = USER_COMMAND_NONE;

	return ret;
}

//恢复的时候使用多线程，一个线程读文件，一个线程写硬盘
//partindex - 镜像文件所在的分区index
//fileindex - 镜像文件的头结构的索引号 g_data.images[fileindex]
int Recover_Linux(WORD partindex, DWORD fileindex, PCONTROL pctl)
{
	MyLog(LOG_DEBUG, L"Recover_Linux...\n");

    OKR_HEADER *header = NULL;
    CHAR16 fullpath[128] = { 0 };
	CHAR16 *filename =NULL;
    UINT64 filesize = 0;
    int ret=-1;
    BOOL bRet = 0, bPartitionChanged = 0, bShowError = TRUE;
    BOOL bPartModify = FALSE;

    DWORD i = 0;
    struct CONTINUE_RWDISK_INFO conti_info;
    PBUFFER_INFO buffinfo;
    struct BUFFERARRAY buffarray;
    struct MP_INFO mpinfo;
    FILE_HANDLE_INFO filehandle;
    FILE_SYS_INFO  sysinfo;
    VOID *bitbuffer = NULL;
    UINT64 after_alignlen = 0;
    UINT64 curlen = 0;
    
	
	
	if(!CheckHandleValid( g_Partitions[partindex].Handle))
	{
		SET_ERROR(__LINE__, OKR_ERROR_FILE_SYSTEM);
		DrawMsgBoxEx(STRING_MEDIA_ERROR, IDOK, -1, TYPE_INFO);
		return -1;
	}
	
	
	
	
    g_data.Cmd = USER_COMMAND_RECOVER;
	g_data.bCompleted = FALSE;
    g_data.ImageDiskNumber = g_Partitions[partindex].DiskNumber;
	filename = (CHAR16 *)g_data.images[fileindex].FileName;

    InitMp(&mpinfo);

	header = myalloc(OKR_HEADER_MAX_SIZE);
	if (!header) {
		SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
		goto Done;
	}


	
	if (FileSystemInit(&sysinfo,partindex,FALSE) == FALSE) {
		if (FileSystemInit(&sysinfo,partindex,TRUE) == FALSE) {
			SET_ERROR(__LINE__, OKR_ERROR_FILE_SYSTEM);
			goto  Done;
		}
	} else {
		logopen(&sysinfo, partindex);
	}

  	MyLog(LOG_DEBUG,L"Recover index:%d,filenum %d\n",fileindex,g_data.images[fileindex].FileNumber);

	GetBackupFileFullPath(&sysinfo, partindex, filename, 0, fullpath, 128);
	//BOOL bIsLenovoPart = IsLenovoPart(partindex);
	//if (IsLenovoPart(partindex) == TRUE)
	//{
	//	SPrint(fullpath, 128, BACKUP_DIR L"\\%s", filename);
	//}
	//else
	//{
	//	SPrint(fullpath, 128, BACKUP_DIR_CUSTOMER L"\\%s", filename);
	//}

    //针对多个fat32备份文件拷贝NTFS分区
    if( g_data.images[fileindex].FileNumber > 1 && sysinfo.ifntfs )
    {
    	  if (!SegFileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE)) 
    	  {
		        SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
            goto  Done;
        }    	
    }
    else
    {
    	  if (!FileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE)) 
    	  {
		        SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
            goto  Done;
        }
    }

   // curlen += filesize;
    filesize = OKR_HEADER_MAX_SIZE;
    bRet = FileRead(&filehandle, (UINTN*)&filesize ,(PBYTE)header);
    if (!bRet || filesize != OKR_HEADER_MAX_SIZE) {
		SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
        goto  Done;
    }

    if (header->Magic != OKR_IMAGE_MAGIC || header->bComplete != TRUE) {
		SET_ERROR(__LINE__, OKR_ERROR_INVALID_IMAGE_FILE);
        goto  Done;
    }
    
    MyLog(LOG_DEBUG,L"Read0 Partition(TotalSectors:%ld), partition %d\n", header->Partition[1].TotalSectors, 1);
	g_data.CompressBlockSize = header->CompressBlockSize;
	//恢复时Init_BufferArray分配的buffer大小最小为CompressBlockSize
	if (Init_BufferArray(&buffarray) == FALSE) {
		SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
		goto Done;
	}
	g_lz4_buffer = myalloc(g_data.CompressBlockSize+ALGIN_SIZE_BYTE);
	if (!g_lz4_buffer) {
		SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
		goto Done;
	}
	MyLog(LOG_DEBUG,L"Restoring(multithread %d)...\n", g_data.bMultiThread);
	MyLog(LOG_DEBUG,L"CompressUnitSize 0x%x, DataOffset 0x%x\n",
		g_data.CompressBlockSize, header->DataOffset);

	MyLog(LOG_DEBUG,L"Checking partitions(num:%d)...\n", header->PartitionCount);

	//检查目标硬盘是否有足够的空间进行恢复，如果目标硬盘太小，不够存储所有分区信息，则返回失败
	if(!GetDestinationDisk_Linux(header))
  {
      	bPartModify = TRUE;
  	  	goto Done;
  }
    //检查是否需要恢复分区表
	if(g_data.bFactoryRestore) {
		//工厂恢复模式则强制恢复分区表
		bPartitionChanged = TRUE;
		g_data.bForceRestorePartition = TRUE;
	} else {
		bPartitionChanged = IfNeedRecoverPartTable_Linux(header);
	}
    //if (bPartitionChanged == TRUE || g_data.bForceRestorePartition) {
	{
		//不管怎样，都要恢复Grub
		BOOL needrecover = FALSE;
		BYTE *databuffer = g_lz4_buffer;
		BYTE *mbr = g_lz4_buffer + 0x100000;	//+1MB, GPT_DATA_SIZE*SECTOR_SIZE;
		ULONGLONG firstsector = GetFirstPartitionBeginSector(g_data.RecoverDiskNumber);

		filesize = header->GptSize;
		if (!FileSetPos(&filehandle, header->StructSize))
			goto  Done;
		bRet = FileRead(&filehandle, (UINTN*)&filesize, databuffer);
		if (!bRet || filesize != header->GptSize) {	//GPT_DATA_SIZE * 512
			SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
			goto  Done;
		}
		if (DiskRead((WORD)g_data.RecoverDiskNumber, 0, 1, mbr)==FALSE) {
			SET_ERROR(__LINE__, OKR_ERROR_READ_DISK);
			goto  Done;
		}
		if(!bPartitionChanged && !header->bGpt) { 
			bPartitionChanged = RecheckPartitionTable_Linux(header, mbr, databuffer);
		}

		if(bPartitionChanged && !g_data.bForceRestorePartition) {
			//如果分区表发生变化，但是没有允许强制恢复分区表，则弹出提示，不允许恢复
			//MyLog(LOG_DEBUG,L"bForceRestorePartition %d\n", g_data.bForceRestorePartition);
			DrawMsgBoxEx(STRING_PARTITION_CHANGED, IDOK, -1, TYPE_INFO);
			SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
			bShowError = FALSE;
			goto Done;
		}

		//如果分区发生变化，则需要用户确认是否恢复；分区信息没变化的话，则只恢复grub信息
		//如果是工厂恢复模式强制恢复分区表; 
		if(bPartitionChanged)
			needrecover = (BOOL)DrawMsgBoxEx(STRING_HINTPARTTABLE, IDOK, IDCANCEL, TYPE_ENQUIRE);
		else //if(g_data.bFactoryRestore)
			needrecover = TRUE;
		if (needrecover) {

        	//恢复分区表或GRUB信息
			UINT64 sector = 0;
			DWORD number = 0;

			number = header->GptSize/SECTOR_SIZE;
			if(!bPartitionChanged && !g_data.bFactoryRestore) {
				//如果分区没变化，并且不是工厂恢复模式，则表示只恢复grub，
				//此时需要判断是否为GPT分区格式，如果是，则不要恢复grub
				if(header->bGpt || IsGptSystem(g_data.RecoverDiskNumber))
					number = 0;
				if(number)
				{
					CopyMem(databuffer + 0x1be, mbr + 0x1be, 0x40);
					//需要计算一下第一个分区的起始扇区位置
					if(firstsector && number > firstsector)
						number = firstsector;
				}
			}
			if(number) {
				if (DiskWrite((WORD)g_data.RecoverDiskNumber, sector, number, databuffer)==FALSE) {
					SET_ERROR(__LINE__, OKR_ERROR_WRITE_GPT);
					MyLog(LOG_DEBUG, L"DiskWrite failed 2\n");
					goto  Done;
				}
			}
		} else {
			//MyLog(LOG_DEBUG,L"User select to not restore partition\n");
			bShowError = FALSE;
            goto  Done;
		}
	}

		MyLog(LOG_DEBUG,L"Resotring DataOffset %d.\n",header->DataOffset);
    //确认每个恢复分区，都在正确的位置。
	if (!FileSetPos(&filehandle, header->DataOffset ? header->DataOffset : OKR_HEADER_MAX_SIZE + GPT_OLD_DATA_SIZE*512))
        goto  Done;
    
	filesize = header->FileSize;
	g_data.fileheader = header;

  curlen = header->DataOffset;

	if(g_data.bMultiThread && g_data.bAsyncIO && g_data.RecoverDiskNumber != g_Partitions[partindex].DiskNumber)
	{
    	ProcessBarInit(pctl, filesize);
		logflush(FALSE);
    	
		ret = RecoverWithAsyncIo(&filehandle, header, &buffarray, &mpinfo);
	 	goto Done;
	}

	logflush(FALSE);
   //	ProcessBarInit(pctl, header->OriginalSize + header->FileSize);

   if( g_data.images[fileindex].FileNumber > 1 && sysinfo.ifntfs == FALSE )
   {
   		  UINT32 datatotalsize =  0;
	  	  UINT64 backupdatatotalsize = header->OriginalSize + header->FileSize;
	  	
	  	  datatotalsize = ((backupdatatotalsize/0x100000)/0x400)/2;
	  	  
	  	  if( datatotalsize >= 4 && datatotalsize <= 36 )
	  	  {
	  	  	backupdatatotalsize = (backupdatatotalsize*(68-( (datatotalsize - 4)/2 )))/100;	  	  	
	  	  }
	  	  else if ( datatotalsize > 36 )
	  	  {
	 	  	  	backupdatatotalsize = (backupdatatotalsize*62)/100;	  	  	 	   
	  	  }
	  	  
 	    	ProcessBarInit(pctl,backupdatatotalsize);
   }
   else
   {
   	   	ProcessBarInit(pctl, header->OriginalSize + header->FileSize);
   }
   
		MyLog(LOG_DEBUG,L"Resotring OriginalSize %d G, file %d G\n",(header->OriginalSize/0x100000)/0x400,(header->FileSize/0x100000)/0x400);

    for (i=0; i<header->PartitionCount; i++) {
        INTN   size = 0;
        ULONGLONG totalblocks = 0,totaldatasize = 0,currentblock = 0,currentdata = 0,beginsec = 0;
        ULONGLONG origin_datasize = 0;
        ULONGLONG totalsec=0;

		MyLog(LOG_DEBUG,L"Resotring partition %d, backup %d, size 0x%x.\n",
			i, header->Partition[i].bIfBackup, header->Partition[i].BackupDataSize);


	 
			 //if (DiskRead((WORD)g_data.RecoverDiskNumber, 0, 1, mbr)==FALSE) {
			if(CompareGuid(&PARTITION_LINUX_SWAP_GUID,(EFI_GUID*)header->Partition[i].TypeGuid))
			{
				MyLog(LOG_DEBUG, L"Found Linux Swap= %d\n", i);
				//DiskRead((WORD)g_data.BackupDiskNumber, g_Partitions[i].BootBegin, 4, pheader->TempBuffer);
				DiskWrite((WORD)g_data.RecoverDiskNumber, header->Partition[i].BootBegin , 12, header->TempBuffer);
			}
				
			 
						 
		 
		
		
        if (!header->Partition[i].bIfBackup || !header->Partition[i].BackupDataSize)
            continue;

        MyLog(LOG_DEBUG,L"Read Partition(TotalSectors:%ld), partition %d\n", header->Partition[i].TotalSectors, i);
		//calc bitmap size
        totalblocks = DivU64x32((header->Partition[i].TotalSectors+(BLOCK_SIZE-1)),BLOCK_SIZE);
        size = DivU64x32((totalblocks+7),8);
        //对齐后的数据大小
		    after_alignlen = CalcAliginSize(size);
        MyLog(LOG_DEBUG,L"Read bitmap(size:%ld) %ld,%ld\n", size, i,after_alignlen);
     //   getch();
        bitbuffer = myalloc(after_alignlen);
        if (bitbuffer == NULL) {
			SET_ERROR(__LINE__, OKR_ERROR_ALLOC_BITMAP);
            goto  Done;
        }
        totaldatasize = header->Partition[i].BackupDataSize - after_alignlen/*size*/;

		//先读取bitmap
       
        MyLog(LOG_DEBUG, L"Resbitmap curlen  %ld\n", curlen);  


		    //filesize = size;
		    filesize = after_alignlen;
        bRet = FileRead(&filehandle, (UINTN*)&filesize ,bitbuffer);//bitmap表因为整体写入，所以不会出现跨分段文件的情况
        if (!bRet || filesize != after_alignlen) {
			SET_ERROR(__LINE__, OKR_ERROR_READ_BITMAP);
            goto  Done;
        }
        curlen += after_alignlen;
		   MyLog(LOG_DEBUG, L"Restoring curlen %ld\n", curlen);
        //统计数据总大小
		if(g_data.bDebug) {
			currentdata = 0;
			for (currentblock=0; currentblock<totalblocks; currentblock++) {
				if (ReadBit(currentblock,bitbuffer))
					currentdata += BLOCK_SIZE*512;
			}
		}
        
        ////////////////////////////////////
        beginsec = header->Partition[i].BootBegin;

		MyLog(LOG_DEBUG, L"Restor size:0x%lx - 0x%lx\n", totaldatasize, currentdata);
        origin_datasize = 0;
		logflush(FALSE);

        g_lz4_size = 8;
		currentblock = 0;
		currentdata = 0;
        Clean_BufferArray(&buffarray);
        SmartDiskIoInit((BYTE)g_data.RecoverDiskNumber, COMMAND_WRITE, &conti_info);
        while (1) {
            DWORD signature,readsize;
            lZ4_BUFFER_HEAD lz4_buffer_head;

            UINTN readn;
            
			if(!g_data.bIsAutoFunc && UserBreak()) {
				bShowError = FALSE;
            	goto Done;
			}
            
            if(g_data.bMultiThread) {
            	buffinfo = GetConsumerAnyway(&buffarray);
            	if (buffinfo->datavalid) {
            	    StartAP(&mpinfo, DecompressBuffer, buffinfo);
            	}
            }
            buffinfo = GetProducter(&buffarray);

            if (currentdata < totaldatasize) {
               // readn = 4;
                readn = sizeof(lZ4_BUFFER_HEAD);
                MyLog(LOG_DEBUG, L"Restoring1 curlen  %ld\n", curlen);  
     
                if (!FileRead(&filehandle,&readn,(PBYTE)&lz4_buffer_head)) {
      						   MyLog(LOG_DEBUG, L"Res FileRead1 error\n");

					SET_ERROR(__LINE__, OKR_ERROR_READ_SIGNATURE);
  				          bShowError = TRUE;  

                    goto  Done;
                }
            } else {
                readn = 0;
            }
            curlen += readn;
            if ( readn ) {
                if (lz4_buffer_head.compress_flag == COMPRESS_MAGIC || lz4_buffer_head.compress_flag == 0x676a6763)
                    buffinfo->ifcompressed = TRUE;
                else if (lz4_buffer_head.compress_flag == UNCOMPRESS_MAGIC || lz4_buffer_head.compress_flag == 0x676a6762)
                    buffinfo->ifcompressed = FALSE;
                else {
					MyLog(LOG_DEBUG, L"compress_flag error 0x%x\n", lz4_buffer_head.compress_flag);
					SET_ERROR(__LINE__, OKR_ERROR_INVALID_SIGNATURE);
                    goto  Done;
                }
                if(lz4_buffer_head.lz4_size > buffinfo->totalsize)
                {
					          SET_ERROR(__LINE__, OKR_ERROR_UNIT_SIZE);
                    goto  Done;               
                }
                /*
                readn = 4;
                if (!FileRead(&filehandle,&readn, (PBYTE)&readsize) || readsize > buffinfo->totalsize) {
					SET_ERROR(__LINE__, OKR_ERROR_UNIT_SIZE);
                    goto  Done;
                }
                */
                 readsize = readn = lz4_buffer_head.align_lz4_size - sizeof(lZ4_BUFFER_HEAD);                
                 if( readn > g_data.CompressBlockSize + ALGIN_SIZE_BYTE )
                 {
                		MyLog(LOG_ERROR,L"Compress size beyond error:%ld,%ld\n",readn,g_data.CompressBlockSize);
                    goto Done;
                 }
 /*               
          curlen  = curlen - sizeof(lZ4_BUFFER_HEAD);
               
          if (!FileSetPos(&filehandle,curlen))
               goto  Done;
               
          readsize = readn = lz4_buffer_head.align_lz4_size;
          
          MyLog(LOG_DEBUG, L"Res2 curlen 0x%lx,readn 0x%lx\n", curlen,readn);
        
        SetMem(TmpBuffer,g_data.CompressBlockSize + ALGIN_SIZE_BYTE,0);  
        
 				if (!FileRead(&filehandle, &readn, TmpBuffer) || readsize != readn ) 
 				{
					
					   MyLog(LOG_DEBUG, L"Res FileRead2 error\n");

					//必须完整的块，因为写入时，整块写入，不会出现分割点在一块压缩数据内的情况。
					SET_ERROR(__LINE__, OKR_ERROR_READ_COMPRESS_UNIT);
                    goto  Done;
        }
        
        CopyMem(buffinfo->pointer,TmpBuffer+sizeof(lZ4_BUFFER_HEAD),lz4_buffer_head.align_lz4_size - sizeof(lZ4_BUFFER_HEAD));
 */      
                MyLog(LOG_DEBUG, L"Res2 curlen 0x%lx,readn 0x%lx\n", curlen,readn);                      
				if (!FileRead(&filehandle, &readn, buffinfo->pointer) || readsize != readn ) {
					
					   MyLog(LOG_DEBUG, L"Res FileRead2 error\n");

					//必须完整的块，因为写入时，整块写入，不会出现分割点在一块压缩数据内的情况。
					SET_ERROR(__LINE__, OKR_ERROR_READ_COMPRESS_UNIT);
                    goto  Done;
                }

             //   MyLog(LOG_DEBUG, L"Res3 curlen,readn \n");

                curlen += readn;

        readn = lz4_buffer_head.align_lz4_size ;
				currentdata += readn;
			//	buffinfo->usedsize = readn;
				buffinfo->usedsize = lz4_buffer_head.lz4_size;
				MyLog(LOG_DEBUG, L"read compress %ld, size=%ld ,last=%ld\n",buffinfo->ifcompressed,readn,totaldatasize-currentdata);

			} else {//end

			//	MyLog(LOG_DEBUG,L"read 0 bytes\n");
				buffinfo->usedsize = 0;
            }

       //     MyLog(LOG_DEBUG, L"Res4 curlen,readn \n");

            buffinfo->datavalid = TRUE;
            if(g_data.bMultiThread)
            	WaitToComplete(&mpinfo);
            else 
            	DecompressBuffer(buffinfo);
         
      //  MyLog(LOG_DEBUG, L"Res5 curlen,readn \n");

			origin_datasize += g_lz4_size;
			MyLog(LOG_DEBUG,L"decompressed bytes %ld,total size %ld\n",g_lz4_size, origin_datasize);
			if(!g_data.bIsAutoFunc && UserBreak()) {
				bShowError = FALSE;
				goto Done;
			}

			if (g_lz4_size == 0) {

		//   MyLog(LOG_DEBUG, L"Res g_lz4_size is 0\n");
				//completed.
				break;
			} else if (g_lz4_size < 0) {
				SET_ERROR(__LINE__, OKR_ERROR_DECOMPRESS);
                goto  Done;
            } else if (g_lz4_size > 8) {

                UINT64 datasize = g_lz4_size;
                PBYTE p = g_lz4_buffer;

                while (datasize > 0) {
                    if (currentblock >= totalblocks) {
						SET_ERROR(__LINE__, OKR_ERROR_COMPRESS_DATA_CORRUPTED);
                        goto  Done;
                    }
                    if (ReadBit(currentblock, bitbuffer)) {
                        DWORD secnum = BLOCK_SIZE;
                        if (currentblock == totalblocks - 1) {//LAST BLOCK
                            secnum = (DWORD)(header->Partition[i].TotalSectors - MultU64x32(currentblock,BLOCK_SIZE));
							//MyLog(LOG_DEBUG,L"lastblock=%d, filesize=%ld, totalblocks=%ld, index=%ld", secnum, filesize, totalblocks, currentblock);
                        }
                        SmartDiskIo(&conti_info, MultU64x32(currentblock, BLOCK_SIZE) + beginsec, secnum, p);
                        totalsec += secnum;
                        p += BLOCK_SIZE*512;
                        datasize -= BLOCK_SIZE*512;
                        
                       	GoProcessBar(BLOCK_SIZE*512);
                    }
                    currentblock++;
                }
                SmartDiskIo(&conti_info, 0, 0, NULL);
            }
            
            g_lz4_size = 8;
        }

//NextPart:
		MyLog(LOG_DEBUG,L"RestorePartition completed, total: 0x%lx\n", totalsec);

		myfree(bitbuffer);
        bitbuffer = NULL;
    }

	//	MyLog(LOG_DEBUG,L"Restore BackupLastProcess\n");
    
    BackupLastProcess();
    
    GoProcessBar(-1);
	DiskFlush(g_data.RecoverDiskNumber);
	//延时1秒，让用户可以看到100%
	//delayEx(1000);

    ret = 0;
Done:

 //	MyLog(LOG_DEBUG, L"Restore Done.\n");

	if(bitbuffer)
		myfree(bitbuffer);
	g_data.databitmap = NULL;	//异步恢复时bitbuffer是临时分配的，不是放在全局变量里
	if(header)
		myfree(header);
	if(g_lz4_buffer)
    	myfree(g_lz4_buffer);
    g_lz4_buffer = NULL;
    
    Destroy_BufferArray(&buffarray);

 //	MyLog(LOG_DEBUG, L"Restore Done 1\n");
    
    FileClose(&filehandle);

	if(ret < 0) {
		MyLog(LOG_DEBUG, L"Restore failed with error %d, LineNumber %d\n", g_data.dwErrorCode, g_data.dwLineNumber);
	} else {
		MyLog(LOG_DEBUG, L"Restore successfully.\n");
	}
	logflush(FALSE);
	FileClose(&g_data.hLog);
    FileSystemClose(&sysinfo);

	if(ret < 0 && bShowError) {
		WCHAR *szError = fullpath;
		SPrint(szError, 100, STRING_ERROR, g_data.dwErrorCode, g_data.dwLineNumber);
	//	DrawMsgBox(szError, STRING_YES, NULL);
	
		if (bPartModify == FALSE)
		{
			//DrawMsgBox(STRING_ERROR_READ_FILE, STRING_YES, NULL);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_ERROR_READ_FILE);
		}
		
		
	}

  DestroyProcessBar();

	if(g_data.bDebug) {
		//MyLog(LOG_DEBUG, L"Press any key to continue.\n");
		//getch();
	}


	g_data.Cmd = USER_COMMAND_NONE;
	
    return ret;
}


BOOL IsExistFileInDir_NTFS_FAT(FILE_SYS_INFO  *sysinfo,CHAR16* fullpath)
{
	 BOOL ret = FALSE;
	 FILE_HANDLE_INFO filehandle;
	 
   //if (!FileOpen(sysinfo, &filehandle, BACKUP_DIR, EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE)) 


 //  if(!FileOpen(sysinfo,&filehandle,fullpath,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE))
 //   {
 //  	  FileClose(&filehandle);
   	     	  
 //  	  return TRUE;
 //  }
   
   
   if (!FileOpen(sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE))
   {
		  return ret;
   }
  
   FileClose(&filehandle);

  
   return TRUE;
	 
}


BOOL IsExistDir_NTFS_FAT(FILE_SYS_INFO  *sysinfo,CHAR16* fullpath)
{
	 BOOL ret = FALSE;
	 FILE_HANDLE_INFO filehandle;
	 
   //if (!FileOpen(sysinfo, &filehandle, BACKUP_DIR, EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE)) 


   if(!FileOpen(sysinfo,&filehandle,fullpath,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE))
   {
   	  FileClose(&filehandle);
   	     	  
   	  return TRUE;
   }
   
   
   FileClose(&filehandle);

  
   return TRUE;
	 
}

////Check if LENOVO_PART\okr\GSKU exist, if it exists, read the content, get the skip type.
////[in]int partindex: lenovo part index.
////[out]ENUM_GSKU_SKIP_TYPE * iSkipType: skip type. 
////	 if the LENOVO_PART\okr\GSKU file does not exist, default value is NOT_GSKU.
////	 if the LENOVO_PART\okr\GSKU file exists, default value is DISABLE_SATA_TO_NVME.
////Return Value: TRUE: find the flag file. FALSE: does not find the file.
BOOL HasGSKUFile(int partindex, ENUM_GSKU_SKIP_TYPE * iSkipType)
{
	FILE_HANDLE_INFO filehandle;
	FILE_SYS_INFO  sysinfo;
	BOOL bFound = FALSE;
	*iSkipType = NOT_GSKU;

	if (FileSystemInit(&sysinfo, partindex, TRUE) == TRUE)
	{
		if (!FileOpen(&sysinfo, &filehandle, L"\\okr\\GSKU", EFI_FILE_MODE_READ, 0, FALSE))
		{
			MyLog(LOG_DEBUG, L"HasGSKUFile FileOpen FAILED.\n");
		}
		else
		{
			MyLog(LOG_DEBUG, L"found GSKU flag file\n");

			*iSkipType = DISABLE_SATA_TO_NVME;

			bFound = TRUE;
			UINTN uiSize = 1024;
			CHAR8 buffer[1024] = { 0 };
			CHAR8 Type[4] = { 0 };

			EFI_STATUS Status = (filehandle.filehandle)->Read(filehandle.filehandle, &uiSize, (VOID*)buffer);
			if (!EFI_ERROR(Status))
			{
				//MyLog(LOG_DEBUG, L"uiSize: %d\n", uiSize);
				//MyLog(LOG_DEBUG, L"buffer: %s\n", (CHAR16*)buffer);
				if (uiSize < 1024 && uiSize > 0)
				{
					UINTN uiFlag = uiSize;
					while (uiFlag >= 1)
					{
						if (buffer[uiFlag - 1] >= L'0' && buffer[uiFlag - 1] <= L'9')
						{
							//MyLog(LOG_DEBUG, L"uiFlag - 1: %d buffer[uiFlag - 1] : %d\n", uiFlag - 1, buffer[uiFlag - 1]);

							Type[0] = buffer[uiFlag - 1];

							*iSkipType = StrDecimalToUintn(Type);
							MyLog(LOG_DEBUG, L"HasGSKUFile SkipType in file: %d\n", *iSkipType);
							break;
						}

						uiFlag--;
					}
				}
			}
			else
			{
				MyLog(LOG_DEBUG, L"HasGSKUFile FileRead FAILED\n");
			}
		}

		FileClose(&filehandle);
		FileSystemClose(&sysinfo);
	}
	else
	{
		MyLog(LOG_DEBUG, L"HasGSKUFile FileSystemInit FAILED\n");
	}
	MyLog(LOG_DEBUG, L"HasGSKUFile SkipType: %d\n", *iSkipType);
	return bFound;
}

BOOL CheckIfAllDataParBitlocker(WORD diskindex)
{
	BOOL bIsAllDataParBitlocker = FALSE;
	for (int i = 0; i < g_dwTotalPartitions; i++)
	{
		if (diskindex != g_Partitions[i].DiskNumber)
			continue;

		////所有数据分区是否都是BitLocker加密？
		if (MyMemCmp(Windows_Data_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0 && 
			g_Partitions[i].bIfBackup == FALSE)
		{
			//MyLog(LOG_DEBUG, L"Par %d type %x\r\n", i, g_Partitions[i].PartitionType);

			if (g_Partitions[i].PartitionType == UNKNOWN_PARTITION)
			{
				continue;
			}

			if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
			{
				MyLog(LOG_DEBUG, L"Par %d disk %d BITLOCKER\r\n", i, g_Partitions[i].DiskNumber);
				bIsAllDataParBitlocker = TRUE; 
			}
			else
			{
				bIsAllDataParBitlocker = FALSE;
				break;
			}
		}
	}
	MyLog(LOG_DEBUG, L"IsAllDataParBitlocker %d diskindex: %d\n", bIsAllDataParBitlocker, diskindex);
	return bIsAllDataParBitlocker;
}

BOOL CheckIfAllDataParBitlockerInAllDisk()
{
	BOOL bIsAllDataParBitlocker = FALSE;
	for (int i = 0; i < g_dwTotalPartitions; i++)
	{
		////所有数据分区是否都是BitLocker加密？
		if (MyMemCmp(Windows_Data_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0 &&
			g_Partitions[i].bIfBackup == FALSE)
		{
			//MyLog(LOG_DEBUG, L"Par %d type %x\r\n", i, g_Partitions[i].PartitionType);

			if (g_Partitions[i].PartitionType == UNKNOWN_PARTITION)
			{
				continue;
			}

			if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
			{
				MyLog(LOG_DEBUG, L"Par %d disk %d BITLOCKER\r\n", i, g_Partitions[i].DiskNumber);
				bIsAllDataParBitlocker = TRUE;
			}
			else
			{
				bIsAllDataParBitlocker = FALSE;
				MyLog(LOG_DEBUG, L"Par %d disk %d not BITLOCKER\r\n", i, g_Partitions[i].DiskNumber);
				break;
			}
		}
	}
	MyLog(LOG_DEBUG, L"IsAllDataParBitlocker %d\n", bIsAllDataParBitlocker);
	return bIsAllDataParBitlocker;
}

BOOL IfExistBitlocker()
{
	BOOL bIfExistBitlocker = FALSE;
	for (int i = 0; i < g_dwTotalPartitions; i++)
	{
		if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
		{
			bIfExistBitlocker = TRUE;
			break;
		}
	}

	return bIfExistBitlocker;
}

BOOL IfExistBitlockerOnDisk(int iDiskNum)
{
	BOOL bIfExistBitlocker = FALSE;
	for (int i = 0; i < g_dwTotalPartitions; i++)
	{
		if (iDiskNum != g_Partitions[i].DiskNumber)
			continue;

		if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
		{
			bIfExistBitlocker = TRUE;
			MyLog(LOG_DEBUG, L"iDiskNum %d, par %d is bitlocker.\n", iDiskNum, g_Partitions[i].PartitionIndex);
			break;
		}
	}
	MyLog(LOG_DEBUG, L"IfExistBitlockerOnDisk ret: %d\n", bIfExistBitlocker);
	return bIfExistBitlocker;
}

BOOL IfDataParExistBitlocker(OKR_HEADER *header, int fileindex)
{
	BOOL bDataParExistBitlocker = FALSE;
	BOOL bIfExistBitlocker = FALSE;
	int iBitLockerPartIndex = 0;
	int iBitLockerPartCount = 0;
	for (int i = 0; i < g_dwTotalPartitions; i++)
	{
		MyLog(LOG_DEBUG, L"i: %d\n", i);
		MyLog(LOG_DEBUG, L"guid:%x %x %x %x-%x %x %x %x\n",  g_Partitions[i].TypeGuid[0], g_Partitions[i].TypeGuid[1], g_Partitions[i].TypeGuid[2], g_Partitions[i].TypeGuid[3], g_Partitions[i].TypeGuid[12], g_Partitions[i].TypeGuid[13], g_Partitions[i].TypeGuid[14], g_Partitions[i].TypeGuid[15]);
		MyLog(LOG_DEBUG, L"Type: %x\n", g_Partitions[i].PartitionType);
		MyLog(LOG_DEBUG, L"Label: %s\n", g_Partitions[i].Label);
		if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
		{
			bIfExistBitlocker = TRUE;
			iBitLockerPartIndex = i;
			iBitLockerPartCount++;
		}
	}

	MyLog(LOG_DEBUG, L"bIfExistBitlocker: %d, iBitLockerPartIndex:%d, iBitLockerPartCount:%d\n", bIfExistBitlocker, iBitLockerPartIndex, iBitLockerPartCount);
	
	if (bIfExistBitlocker)
	{
		if (iBitLockerPartCount > 1)
		{
			//如果有两个以上分区存在BitLocker，认为一定有数据分区
			bDataParExistBitlocker = TRUE;
		}
		else if (iBitLockerPartCount == 1)
		{
			//如果只有一个分区BitLocer，有可能是系统分区，需要判断
			if (NULL != header)
			{
				MyLog(LOG_DEBUG, L"header->PartitionCount:%d\n", header->PartitionCount);
				for (int iImgPartIndex = 0; iImgPartIndex < header->PartitionCount; iImgPartIndex++)
				{
					MyLog(LOG_DEBUG, L"iImgPartIndex: %d\n", iImgPartIndex);
					if (header->Partition[iImgPartIndex].HasWindows)
					{
						MyLog(LOG_DEBUG, L"iImgPartIndex:%d HasWindows\n", iImgPartIndex);
						if (g_Partitions[iBitLockerPartIndex].BootBegin != header->Partition[iImgPartIndex].BootBegin)
						{
							MyLog(LOG_DEBUG, L"BootBegin Not Match. Data Par Exist Bitlocker\n");
							//镜像中存储的系统分区起始点，与当前存在BitLocker的分区起始点相同，认为是系统分区存在BitLocker
							//否则认为存在BitLocker的分区是数据分区
							bDataParExistBitlocker = TRUE;
						}
						break;
					}
				}
			}
			
		}
		
	}

	return bDataParExistBitlocker;
}

BOOL IsLenovoPart(int partindex)
{
	BOOL bIsLenovoPart = FALSE;
	if (StrnCmp(g_Partitions[partindex].Label, L"LENOVO_PART", 11) == 0 &&
		MyMemCmp(Window_Recovery_TypeGuid, g_Partitions[partindex].TypeGuid, 16) == 0 &&
		g_Partitions[partindex].PartitionType == FAT32)
	{
		bIsLenovoPart = TRUE;
	}
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"%d IsLenovoPart %d\n", partindex, bIsLenovoPart);
#endif
	return bIsLenovoPart;
}

int GetBackupFileFullPath(PFILE_SYS_INFO info, int partindex, CHAR16* fileName, int iFileNameIndex, CHAR16* fileFullPath, int iFileFullPathLen)
{
	int iRet = -1;

	if (fileName == NULL || fileFullPath == NULL)
	{
		return -1;
	}
	//MyLog(LOG_DEBUG, L"GetBackupFileFullPath...\n");
	SetMem(fileFullPath, iFileFullPathLen, 0);

	if (IsLenovoPart(partindex) == TRUE)
	{
		if (iFileNameIndex == 0)
		{
			SPrint(fileFullPath, iFileFullPathLen, BACKUP_DIR L"\\%s", fileName);
		}
		else
		{
			SPrint(fileFullPath, iFileFullPathLen, BACKUP_DIR L"\\%s.%d", fileName, iFileNameIndex);
		}

		//MyLog(LOG_DEBUG, L"full path: %s\n", fileFullPath);

		iRet = 0;
		return iRet;
	}
	else
	{
		BOOL bRet = IsFileExistEx(info, BACKUP_DIR_CUSTOMER);
		if (bRet)
		{
			if (iFileNameIndex == 0)
			{
				SPrint(fileFullPath, iFileFullPathLen, BACKUP_DIR_CUSTOMER L"\\%s", fileName);
			}
			else
			{
				SPrint(fileFullPath, iFileFullPathLen, BACKUP_DIR_CUSTOMER L"\\%s.%d", fileName, iFileNameIndex);
			}

			bRet = IsFileExistEx(info, fileFullPath);
			if (TRUE == bRet)
			{
				MyLog(LOG_DEBUG, L"full path: %s\n", fileFullPath);
				iRet = 0;
				return iRet;
			}
		}

		bRet = IsFileExistEx(info, BACKUP_DIR);
		if (TRUE == bRet)
		{
			SetMem(fileFullPath, iFileFullPathLen, 0);
			if (iFileNameIndex == 0)
			{
				SPrint(fileFullPath, iFileFullPathLen, BACKUP_DIR L"\\%s", fileName);
			}
			else
			{
				SPrint(fileFullPath, iFileFullPathLen, BACKUP_DIR L"\\%s.%d", fileName, iFileNameIndex);
			}

			bRet = IsFileExistEx(info, fileFullPath);
			if (TRUE == bRet)
			{
				MyLog(LOG_DEBUG, L"full path: %s\n", fileFullPath);
				iRet = 0;
				return iRet;
			}
		}
	}

	return iRet;
}

BOOL IsFileExist(int partindex, CHAR16* fileName)
{
	FILE_HANDLE_INFO filehandle;
	FILE_SYS_INFO  sysinfo;
	BOOL bFound = FALSE;

	if (FileSystemInit(&sysinfo, partindex, TRUE) == TRUE)
	{
		if (TRUE == FileOpen(&sysinfo, &filehandle, fileName, EFI_FILE_MODE_READ, 0, FALSE))
		{
			bFound = TRUE;
		}

		FileClose(&filehandle);
		FileSystemClose(&sysinfo);
	}
	else
	{
		MyLog(LOG_DEBUG, L"IsFileExist FileSystemInit FAILED\n");
	}

	return bFound;
}

BOOL IsFileExistEx(PFILE_SYS_INFO info, CHAR16* fileName)
{
	FILE_HANDLE_INFO filehandle;
	BOOL bFound = FALSE;

	if (fileName != NULL && info != NULL)
	{
		if (TRUE == FileOpen(info, &filehandle, fileName, EFI_FILE_MODE_READ, 0, FALSE))
		{
			bFound = TRUE;
			FileClose(&filehandle);
		}
	}
	//MyLog(LOG_DEBUG, L"IsFileExistEx bFound %d path: %s\n", bFound, fileName);
	return bFound;
}

BOOL IsRAIDBackupFile(DWORD backupfileID)
{
	BOOL bIsRAIDBackupFile = FALSE;
	
	if (AsciiStrnCmp(g_data.images[backupfileID].Memo + 100 + 12, "RAID", 4) == 0)
	{
		bIsRAIDBackupFile = TRUE;
	}
	MyLog(LOG_DEBUG, L"IsRAIDBackupFile: %d.\n", bIsRAIDBackupFile);
	return bIsRAIDBackupFile;
}

// 转化函数
int sthvalue(char c)
{
	int value;
	if ((c >= '0') && (c <= '9'))
		value = 48;
	else if ((c >= 'a') && (c <= 'f'))
		value = 87;
	else if ((c >= 'A') && (c <= 'F'))
		value = 55;
	else {
		printf("invalid data %c", c);
		return -1;
	}
	return value;
}

int strtohex(char *str, char *data)
{
	int len = 0;
	int sum = 0;
	int high = 0;
	int low = 0;
	int value = 0;
	int j = 0;
	len = strlen(str);
	printf("%d\n", len);
	for (int i = 0; i < len; i++)
	{
		value = sthvalue(str[i]);
		high = (((str[i] - value) & 0xF) << 4);
		value = sthvalue(str[i + 1]);
		low = ((str[i + 1] - value) & 0xF);
		sum = high | low;
		j = i / 2;
		data[j] = sum;
		i = i + 1;
	}
	return len;
}

void PrintValue(unsigned char *ucSha256Value, int bufferSize)
{
#if OKR_DEBUG
	CHAR16 buffer[100] = { 0 };

	for (int i = 0; i < bufferSize && i < 100; ++i)
	{
		SPrint(buffer + i * sizeof(CHAR16), 100, L"%02x", ucSha256Value[i]);
	}
	MyLog(LOG_DEBUG, L"%s \n", buffer);
#endif
}

BOOL _CalFileSHA256Value(PFILE_HANDLE_INFO filehandle, myc_sha256_ctx *ctx, BYTE *pData,
	size_t BufferSize, BOOL bIsCheckValue)
{
	BOOL bRet = FALSE;

	if (NULL == filehandle || NULL == ctx || NULL == pData || 0 == BufferSize)
	{
		MyLog(LOG_DEBUG, L"Invalid argument.\n");
		return bRet;
	}

	do
	{
		memset(pData, 0, BufferSize);

		myc_sha256_ctx ctxSec;
		myc_sha256_init(&ctxSec);

		unsigned char pucTmpSha256[SHA256_LEN] = { 0 };
		int iTmpSha256Length = SHA256_LEN;

		UINT64 filesize = 0;
		FileGetSize(filehandle, &filesize);
		UINT64 llSectorReadSize = 0;
		UINT64 llReadSize = 0;
		DWORD dwReadSize = 0;
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"CalculateFileSHA256Value FileGetSize %lld.\n", filesize);
#endif
		FileSetPos(filehandle, 0);
		int iIndex = 0;
		do
		{
			DWORD curtime = 0;
			curtime = GetTickCount() / 1000 - g_data.BeginTime;
			if (curtime > g_data.LastTime)
			{
				UpdateComma(curtime % 3 + 1);
				g_data.LastTime = curtime;
			}

			if (!g_data.bIsAutoFunc && UserBreak()) 
			{
#if OKR_DEBUG
				MyLog(LOG_DEBUG, L"_CalFileSHA256Value UserBreak true.\n");
#endif
				g_bUserBreak = TRUE;
				bRet = FALSE;
				break;
			}

			if (filesize - llReadSize > 8192)
			{
				dwReadSize = 8192;
			}
			else
			{
				dwReadSize = filesize - llReadSize;
#if OKR_DEBUG
				MyLog(LOG_DEBUG, L"dwReadSize %lld.\n", dwReadSize);
#endif
				if (TRUE == bIsCheckValue)
				{
					dwReadSize -= PBKDF2P_LEN;
				}
#if OKR_DEBUG
				MyLog(LOG_DEBUG, L"dwReadSize after %lld.\n", dwReadSize);
#endif
			}

			if (!NormalFileRead(filehandle, &dwReadSize, pData))
			{
				MyLog(LOG_DEBUG, L"read file error\n");
				break;
			}

			//MyLog(LOG_DEBUG, L"pData: %d\n", dwReadSize);
			//PrintValue(pData, dwReadSize);
			//MyLog(LOG_DEBUG, L"******iIndex %d*******\n", iIndex);
			//PrintValue(pData, 32);
			//PrintValue(pData + 8192 - 32, 32);
			//MyLog(LOG_DEBUG, L"*************\n");
			//getch();

			//hash some data
			//myc_sha256(pData, BufferSize, ucSha256Value);
			if (dwReadSize != BufferSize && TRUE == bIsCheckValue)
			{
#if OKR_DEBUG
				MyLog(LOG_DEBUG, L"******iIndex %d*******\n", iIndex);
				PrintValue(pData, 32);
				MyLog(LOG_DEBUG, L"pData + dwReadSize - 32:\n");
				PrintValue(pData + dwReadSize - 32, 32);

				//dwReadSize -= PBKDF2P_LEN;
				
				//MyLog(LOG_DEBUG, L"pData + dwReadSize:\n");
				//PrintValue(pData + dwReadSize, 32);
				MyLog(LOG_DEBUG, L"*************\n");
				MyLog(LOG_DEBUG, L"llReadSize %lld, dwReadSize %lld.\n", llReadSize, dwReadSize);
#endif
			}

			if (0 != dwReadSize)
			{
				llReadSize += dwReadSize;
				llSectorReadSize += dwReadSize;
				//MyLog(LOG_DEBUG, L"llReadSize %lld, dwReadSize %lld.\n", llReadSize, dwReadSize);
				myc_sha256_update(&ctxSec, pData, dwReadSize);

				if (llSectorReadSize >= 2 * 1024 * 1024 || !(dwReadSize == BufferSize && llReadSize < filesize))
				{
					//MyLog(LOG_DEBUG, L"******iIndex %d*******\n", iIndex);
					//PrintValue(pData, 32);
					//PrintValue(pData + 8192 - 32, 32);
					//MyLog(LOG_DEBUG, L"*************\n");
					//getch();

					myc_sha256_final(&ctxSec, pucTmpSha256);
					//PrintValue(pucTmpSha256, iTmpSha256Length);

					myc_sha256_update(ctx, pucTmpSha256, iTmpSha256Length);

					myc_sha256_init(&ctxSec);
					memset(pucTmpSha256, 0, iTmpSha256Length);

					llSectorReadSize = 0;
				}
			}

			memset(pData, 0, BufferSize);

			FileSetPos(filehandle, llReadSize);
			iIndex++;

		} while (dwReadSize == BufferSize && llReadSize < filesize);

#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"llReadSize %lld\n", llReadSize);
#endif

		if (g_bUserBreak)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	//MyLog(LOG_DEBUG, L"_CalFileSHA256Value return %d.\n", bRet);
	return bRet;
}

BOOL _CalculateFileSHA256ValueBySegData(PFILE_HANDLE_INFO filehandle, myc_sha256_ctx *ctx, BYTE *pData,
	size_t BufferSize, BOOL bIsCheckValue, DWORD FileNumber)
{
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"_CalculateFileSHA256ValueBySegData...\n");
#endif

	BOOL bRet = FALSE;

	if (NULL == filehandle || NULL == ctx || NULL == pData || 0 == BufferSize)
	{
		MyLog(LOG_DEBUG, L"Invalid argument.\n");
		return bRet;
	}

	do
	{
		memset(pData, 0, BufferSize);

		myc_sha256_ctx ctxSec;
		myc_sha256_init(&ctxSec);

		unsigned char pucTmpSha256[SHA256_LEN] = { 0 };
		int iTmpSha256Length = SHA256_LEN;

		UINT64 filesize = 0;
		FileGetSize(filehandle, &filesize);
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"FileGetSize %lld.\n", filesize);
#endif
		UINT64 llSectorDataReadSize = 0;
		UINT64 llSectorReadSize = 0;
		UINT64 llReadSize = 0;
		DWORD dwReadSize = 0;

		UINT64 llPerSectorDataLimit = FILE_PER_SECTOR_DATA;
		if (FileNumber > 1)
		{
			llPerSectorDataLimit = MULTI_FILE_PER_SECTOR_DATA;
		}

		FileSetPos(filehandle, 0);

		int iSegIndex = 0;
		int iSegTotalNumber = FILE_SHA256_SEG_NUMBER;
		UINT64 llSegFileSize = (UINT64)filesize / iSegTotalNumber;

		int iIndex = 0;
		do
		{
			DWORD curtime = 0;
			curtime = GetTickCount() / 1000 - g_data.BeginTime;
			if (curtime > g_data.LastTime)
			{
				UpdateComma(curtime % 3 + 1);
				g_data.LastTime = curtime;
			}

			if (!g_data.bIsAutoFunc && UserBreak())
			{
				MyLog(LOG_DEBUG, L"_CalculateFileSHA256ValueBySegData UserBreak true.\n");
				g_bUserBreak = TRUE;
				bRet = FALSE;
				break;
			}

			dwReadSize = 8192;

			if (!NormalFileRead(filehandle, &dwReadSize, pData))
			{
				MyLog(LOG_DEBUG, L"read file error\n");
				break;
			}

			if (0 != dwReadSize)
			{
				llReadSize += dwReadSize;
				llSectorReadSize += dwReadSize;
				llSectorDataReadSize += dwReadSize;

				//MyLog(LOG_DEBUG, L"llReadSize %lld, dwReadSize %lld.\n", llReadSize, dwReadSize);
				myc_sha256_update(&ctxSec, pData, dwReadSize);

				if (llSectorReadSize >= 2 * 1024 * 1024)
				{
					//MyLog(LOG_DEBUG, L"******iIndex %d*******\n", iIndex);
					//MyLog(LOG_DEBUG, L"llReadSize %lld, dwReadSize %lld\n", llReadSize, dwReadSize);
					//MyLog(LOG_DEBUG, L"llSectorReadSize %lld llSectorDataReadSize %lld\n", llSectorReadSize, llSectorDataReadSize);
					//PrintValue(pData, 32);
					//PrintValue(pData + 8192 - 32, 32);
					//MyLog(LOG_DEBUG, L"*************\n");
					//getch();

					myc_sha256_final(&ctxSec, pucTmpSha256);
					//PrintValue(pucTmpSha256, iTmpSha256Length);

					myc_sha256_update(ctx, pucTmpSha256, iTmpSha256Length);
					memset(pucTmpSha256, 0, iTmpSha256Length);

					myc_sha256_init(&ctxSec);
					llSectorReadSize = 0;
				}
			}

			if (llSectorDataReadSize >= llPerSectorDataLimit)
			{
				//MyLog(LOG_DEBUG, L"******iIndex %d*******\n", iIndex);
				//PrintValue(pData, 32);
				//PrintValue(pData + 8192 - 32, 32);
				//MyLog(LOG_DEBUG, L"*************\n");
				//getch();

				iSegIndex++;
				if (iSegIndex < iSegTotalNumber)
				{
					llReadSize = iSegIndex * llSegFileSize;
					//MyLog(LOG_DEBUG, L"=SegIndex %d llReadSize %lld\r\n", iSegIndex, llReadSize);
					FileSetPos(filehandle, llReadSize);
				}
				else if (iSegIndex == iSegTotalNumber)
				{
					if (bIsCheckValue)
					{
						llReadSize = filesize - llPerSectorDataLimit - 32;
					}
					else
					{
						llReadSize = filesize - llPerSectorDataLimit;
					}
					//MyLog(LOG_DEBUG, L"==SegIndex %d llReadSize %lld\r\n", iSegIndex, llReadSize);
					FileSetPos(filehandle, llReadSize);
				}
				else
				{
					//MyLog(LOG_DEBUG, L"break  SegIndex %d llReadSize %lld\r\n", iSegIndex, llReadSize);
					myc_sha256_final(&ctxSec, pucTmpSha256);
					PrintValue(pucTmpSha256, iTmpSha256Length);

					myc_sha256_update(ctx, pucTmpSha256, iTmpSha256Length);
					memset(pucTmpSha256, 0, iTmpSha256Length);
					break;
				}

				//MyLog(LOG_DEBUG, L"SegIndex %d llReadSize %lld\r\n", iSegIndex, llReadSize);
				llSectorDataReadSize = 0;
			}
			else
			{
				FileSetPos(filehandle, llReadSize);
			}

			memset(pData, 0, BufferSize);
			
			iIndex++;

		} while (dwReadSize == BufferSize && llReadSize < filesize);

		//MyLog(LOG_DEBUG, L"llReadSize %lld\n", llReadSize);

		if (g_bUserBreak)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	//MyLog(LOG_DEBUG, L"_CalculateFileSHA256ValueBySegData return %d.\n", bRet);
	return bRet;
}

BOOL CalculateFileSHA256Value(PFILE_HANDLE_INFO filehandle, CHAR16 *filename, DWORD FileNumber,
	unsigned char *ucSha256Value, int iSha256Length, BOOL bIsCheckValue)
{
	//MyLog(LOG_DEBUG, L"CalculateFileSHA256Value...\n");

	BOOL bRet = FALSE;
	BYTE *pData = NULL;

	if (NULL == filehandle || NULL == ucSha256Value)
	{
		MyLog(LOG_DEBUG, L"Invalid argument.\n");
		return bRet;
	}

	do
	{
		size_t BufferSize = 8192;
		pData = myalloc(BufferSize);
		if (NULL == pData)
		{
			MyLog(LOG_DEBUG, L"myalloc error.\n");
			break;
		}
		memset(pData, 0, BufferSize);

		UINT64 filesize = 0;

		myc_sha256_ctx ctx;
		myc_sha256_init(&ctx);

		if (FileNumber >= 0 && FileNumber <= 1)
		{			
			FileGetSize(filehandle, &filesize);
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"FileGetSize %lld.\n", filesize);
#endif
			if (filesize > FILE_SHA256_SEG_NUMBER * 20 * 1024 * 1024)
			{
				if (!_CalculateFileSHA256ValueBySegData(filehandle, &ctx, pData, BufferSize, TRUE, FileNumber))
				{
					bRet = FALSE;
					if (g_bUserBreak)
					{
						break;
					}
					SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
					MyLog(LOG_DEBUG, L"_CalculateFileSHA256ValueBySegData Error.\n");
					break;
					}
			}
			else
			{
				if (!_CalFileSHA256Value(filehandle, &ctx, pData, BufferSize, TRUE))
				{
					bRet = FALSE;
					if (g_bUserBreak)
					{
						break;
					}
					SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
					MyLog(LOG_DEBUG, L"_CalFileSHA256Value Error.\n");
					break;
				}
			}
		}
		else if (FileNumber > 1)
		{
			BOOL bCalSegFile = FALSE;
			//MyLog(LOG_DEBUG, L"FileNumber %d\n", FileNumber);

			for (int iFileNameIndex = 0; iFileNameIndex < FileNumber; iFileNameIndex++)
			{
				bCalSegFile = FALSE;

				FILE_HANDLE_INFO segFilehandle;
				mymemset(&segFilehandle, 0, sizeof(segFilehandle));

				if (iFileNameIndex != FileNumber - 1)
				{
					CHAR16 fileFullPath[256] = { 0 };
					if (0 == iFileNameIndex)
					{
						SPrint(fileFullPath, 256, L"%s", filename, iFileNameIndex);
#if OKR_DEBUG
						MyLog(LOG_DEBUG, L"fileFullPath %s\n", fileFullPath);
#endif
					}
					else
					{
						SPrint(fileFullPath, 256, L"%s.%d", filename, iFileNameIndex);
#if OKR_DEBUG
						MyLog(LOG_DEBUG, L"fileFullPath %s\n", fileFullPath);
#endif
					}

					if (!FileOpen(filehandle->psys, &segFilehandle, fileFullPath, EFI_FILE_MODE_READ, 0, TRUE))
					{
						SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
						MyLog(LOG_DEBUG, L"FileOpen Error.\n");
						break;
					}

					FileGetSize(&segFilehandle, &filesize);
#if OKR_DEBUG
					MyLog(LOG_DEBUG, L"FileGetSize %lld.\n", filesize);
#endif

					if (filesize > FILE_SHA256_SEG_NUMBER * 20 * 1024 * 1024)
					{
						if (!_CalculateFileSHA256ValueBySegData(&segFilehandle, &ctx, pData, BufferSize, FALSE, FileNumber))
						{
							bRet = FALSE;
							FileClose(&segFilehandle);
							if (g_bUserBreak)
							{
								break;
							}

							SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
							MyLog(LOG_DEBUG, L"_CalculateFileSHA256ValueBySegData Error.\n");
							break;
						}
					}
					else
					{
						if (!_CalFileSHA256Value(&segFilehandle, &ctx, pData, BufferSize, FALSE))
						{
							bRet = FALSE;
							FileClose(&segFilehandle);
							if (g_bUserBreak)
							{
								break;
							}

							SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
							MyLog(LOG_DEBUG, L"_CalFileSHA256Value Error.\n");
							break;
						}
					}

					FileClose(&segFilehandle);
				}
				else
				{
#if OKR_DEBUG
					MyLog(LOG_DEBUG, L"iFileNameIndex %d\n", iFileNameIndex);
#endif
					FileGetSize(filehandle, &filesize);
#if OKR_DEBUG
					MyLog(LOG_DEBUG, L"FileGetSize %lld.\n", filesize);
#endif

					if (filesize > FILE_SHA256_SEG_NUMBER * 20 * 1024 * 1024)
					{
						if (!_CalculateFileSHA256ValueBySegData(filehandle, &ctx, pData, BufferSize, TRUE, FileNumber))
						{
							bRet = FALSE;
							if (g_bUserBreak)
							{
								break;
							}

							SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
							MyLog(LOG_DEBUG, L"_CalFileSHA256Value Error.\n");
							break;
						}
					}
					else
					{
						if (!_CalFileSHA256Value(filehandle, &ctx, pData, BufferSize, TRUE))
						{
							bRet = FALSE;
							if (g_bUserBreak)
							{
								break;
							}

							SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
							MyLog(LOG_DEBUG, L"_CalFileSHA256Value Error.\n");
							break;
						}
					}
				}

				bCalSegFile = TRUE;
			}
			if (FALSE == bCalSegFile)
			{
				bRet = FALSE;
				break;
			}
		}

		myc_sha256_final(&ctx, ucSha256Value);

		bRet = TRUE;

#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"CalculateFileSHA256Value:\n");
		PrintValue(ucSha256Value, iSha256Length);
#endif
	} while (FALSE);

	if (pData != NULL)
	{
		myfree(pData);
		pData = NULL;
	}

#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"CalculateFileSHA256Value return %d.\n", bRet);
#endif

	return bRet;
}


BOOL _GetSaltValue(PFILE_HANDLE_INFO filehandle, 
	unsigned char *ucSlatValue, int iSlatLength, int *iterations)
{
	//MyLog(LOG_DEBUG, L"GetSaltValue...\n");

	BOOL bRet = FALSE;
	BYTE *pData = NULL;

	if (NULL == filehandle || NULL == ucSlatValue || NULL == iterations)
	{
		MyLog(LOG_DEBUG, L"Invalid argument.\n");
		return bRet;
	}

	do
	{
		UINT64 filesize = 0;
		FileGetSize(filehandle, &filesize);
		//MyLog(LOG_DEBUG, L"GetSaltValue FileGetSize %lld.\n", filesize);

		FileSetPos(filehandle, filesize / 2);
		DWORD dwReadSize = iSlatLength;
		if (!NormalFileRead(filehandle, &dwReadSize, ucSlatValue))
		{
			MyLog(LOG_DEBUG, L"read file error\n");
			break;
		}

#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"SlatValue:\n");
		PrintValue(ucSlatValue, iSlatLength);
#endif

		unsigned char ucIterValue[4] = { 0 };
		UINT64 iIterLength = 4;
		FileSetPos(filehandle, filesize / 3);
		if (!NormalFileRead(filehandle, &iIterLength, ucIterValue))
		{
			MyLog(LOG_DEBUG, L"read file error\n");
			break;
		}

#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"ucIterValue %d  %d.\n", ucIterValue[0], (int)ucIterValue[0]);
#endif
		*iterations = (int)ucIterValue[0];

		bRet = TRUE;

	} while (FALSE);

	if (pData != NULL)
	{
		myfree(pData);
		pData = NULL;
	}

	//MyLog(LOG_DEBUG, L"GetSaltValue return %d.\n", bRet);
	return bRet;
}

BOOL GetSaltValue(PFILE_HANDLE_INFO filehandle, CHAR16 *filename, DWORD FileNumber,
	unsigned char *ucSlatValue, int iSlatLength, int *iterations)
{
	BOOL bRet = FALSE;
	BYTE *pData = NULL;

	if (NULL == filehandle || NULL == ucSlatValue || NULL == filename || NULL == iterations)
	{
		MyLog(LOG_DEBUG, L"Invalid argument.\n");
		return bRet;
	}

	do
	{
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"FileNumber %d\n", FileNumber);
#endif

		if (FileNumber >= 0 && FileNumber <= 1)
		{
			if (!_GetSaltValue(filehandle, ucSlatValue, iSlatLength, iterations))
			{
				bRet = FALSE;
				MyLog(LOG_DEBUG, L"_GetSaltValue Error.\n");
				break;
			}
		}
		else if (FileNumber > 1)
		{
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"filename %s\n", filename);
#endif

			FILE_HANDLE_INFO segFilehandle;
			mymemset(&segFilehandle, 0, sizeof(segFilehandle));

			if (!FileOpen(filehandle->psys, &segFilehandle, filename, EFI_FILE_MODE_READ, 0, TRUE))
			{
				SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
				MyLog(LOG_DEBUG, L"FileOpen Error.\n");
				break;
			}

			if (!_GetSaltValue(&segFilehandle, ucSlatValue, iSlatLength, iterations))
			{
				FileClose(&segFilehandle);
				break;
			}
			FileClose(&segFilehandle);
		}

		bRet = TRUE;
	} while (FALSE);

	//MyLog(LOG_DEBUG, L"GetSaltValue return %d.\n", bRet);
	return bRet;
}

BOOL SaveZeroDataToFileEndForCheckValue(PFILE_HANDLE_INFO filehandle)
{
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"***SaveZeroDataToFileEndForCheckValue..\n");
#endif

	BOOL bRet = FALSE;

	do
	{
		if (NULL == filehandle)
		{
			MyLog(LOG_DEBUG, L"Invalid argument.\n");
			break;
		}

		UINT64 filesize = 0;
		FileGetSize(filehandle, &filesize);
		FileSetPos(filehandle, filesize);

		unsigned char pucCheckValue[PBKDF2P_LEN] = { '0' };
		int iValueLength = PBKDF2P_LEN;
		for (int i = 0; i < iValueLength; i++)
		{
			pucCheckValue[i] = '0';
		}
		pucCheckValue[PBKDF2P_LEN - 1] = '\0';
		if (!NormalFileWrite(filehandle, &iValueLength, pucCheckValue))
		{
			MyLog(LOG_DEBUG, L"Write Placeholder Error.\n");
			break;
		}

		FileGetSize(filehandle, &filesize);
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"SaveZeroDataToFileEndForCheckValue Get FileSize %lld\n", filesize);
#endif
		bRet = TRUE;

	} while (FALSE);

	return bRet;
}

BOOL SaveFileCheckValue(PFILE_HANDLE_INFO filehandle, unsigned char *pucCheckValue, int iValueLength)
{
	BOOL bRet = FALSE;

	do
	{	
		if (NULL == filehandle || NULL == pucCheckValue)
		{
			MyLog(LOG_DEBUG, L"Invalid argument.\n");
			break;
		}

		UINT64 filesize = 0;
		FileGetSize(filehandle, &filesize);
		if (filesize > 0)
		{
			filesize -= SALT_LEN;
		}
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"SaveFileCheckValue FileGetSize %lld.\n", filesize);
#endif
		FileSetPos(filehandle, filesize);
		if (NormalFileWrite(filehandle, &iValueLength, pucCheckValue))
		{
			bRet = TRUE;
			MyLog(LOG_DEBUG, L"NormalFileWrite Suc.\n");
		}
	} while (FALSE);

	return bRet;
}

BOOL GetFileCheckValue(PFILE_HANDLE_INFO filehandle, unsigned char *pucCheckValue, int iValueLength)
{
	BOOL bRet = FALSE;

	do
	{
		if (NULL == filehandle || NULL == pucCheckValue)
		{
			MyLog(LOG_DEBUG, L"Invalid argument.\n");
			break;
		}

		UINT64 filesize = 0;
		FileGetSize(filehandle, &filesize);
		//MyLog(LOG_DEBUG, L"GetFileCheckValue FileGetSize %lld.\n", filesize);
		if (filesize > 0)
		{
			filesize -= SALT_LEN;
		}
		//MyLog(LOG_DEBUG, L"GetFileCheckValue after %lld.\n", filesize);

		FileSetPos(filehandle, filesize);
		if (NormalFileRead(filehandle, &iValueLength, pucCheckValue))
		{
			bRet = TRUE;
			//MyLog(LOG_DEBUG, L"FileRead Suc.\n");
		}
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"GetFileCheckValue:\n");
		PrintValue(pucCheckValue, iValueLength);
#endif
	} while (FALSE);

	return bRet;
}

BOOL CalCheckValue(PFILE_HANDLE_INFO filehandle, CHAR16 *filename, DWORD FileNumber, 
	unsigned char *pucCheckValue, int iValueLength, BOOL bIsCheckValue)
{
	BOOL bRet = FALSE;

	do
	{
		if (NULL == filehandle || NULL == pucCheckValue || NULL == filename)
		{
			MyLog(LOG_DEBUG, L"Invalid argument.\n");
			break;
		}

		////CalculateFileSHA256Value
		unsigned char ucSha256Value[SHA256_LEN] = { 0 };
		int iSha256Length = SHA256_LEN;
		bRet = CalculateFileSHA256Value(filehandle, filename, FileNumber, ucSha256Value, iSha256Length, bIsCheckValue);
		if (FALSE == bRet)
		{
			if (g_bUserBreak)
			{
				break;
			}
			MyLog(LOG_DEBUG, L"CalculateFileSHA256Value Error.\n");
			break;
		}

		//GetSaltValue
		unsigned char ucSlatValue[SALT_LEN] = { 0 };
		int iSlatLength = SALT_LEN;
		int iterations = 100;
		bRet = GetSaltValue(filehandle, filename, FileNumber, ucSlatValue, iSlatLength, &iterations);
		if (FALSE == bRet)
		{
			MyLog(LOG_DEBUG, L"GetSaltValue Error.\n");
			break;
		}

		myc_pbkdf2_sha256(ucSha256Value, iSha256Length, ucSlatValue, iSlatLength, 3000 + iterations, pucCheckValue, iValueLength);
#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"CalCheckValue:\n");
		PrintValue(pucCheckValue, iValueLength);
#endif

		bRet = TRUE;

	} while (FALSE);
	
	//MyLog(LOG_DEBUG, L"CalCheckValue ret: %d\n", bRet);
	return bRet;
}

BOOL CalculateFileCheckValue(PFILE_SYS_INFO sysinfo, CHAR16 *filename, DWORD FileNumber)
{
	MyLogShowTime(LOG_DEBUG, L"***CalculateFileCheckValue..\n");

	BOOL bRet = FALSE;
	FILE_HANDLE_INFO filehandle;
	mymemset(&filehandle, 0, sizeof(filehandle));

	do
	{
		if (FileNumber >= 0 && FileNumber <= 1)
		{
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"filename %s\n", filename);
#endif
			if (!FileOpen(sysinfo, &filehandle, filename, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0, TRUE))
			{
				SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
				MyLog(LOG_DEBUG, L"FileOpen Error.\n");
				break;
			}
		}
		else if (FileNumber > 1)
		{
			CHAR16 fileFullPath[256] = { 0 };
			SPrint(fileFullPath, 256, L"%s.%d", filename, FileNumber - 1);
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"fileFullPath %s\n", fileFullPath);
#endif
			if (!FileOpen(sysinfo, &filehandle, fileFullPath, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0, TRUE))
			{
				SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
				MyLog(LOG_DEBUG, L"FileOpen Error.\n");
				break;
			}
		}
		
		SaveZeroDataToFileEndForCheckValue(&filehandle);

		//pbkdf2
		unsigned char pucCheckValue[PBKDF2P_LEN] = { 0 };
		int iValueLength = PBKDF2P_LEN;
		memset(pucCheckValue, 0, iValueLength);

		bRet = CalCheckValue(&filehandle, filename, FileNumber, pucCheckValue, iValueLength, TRUE);
		if (FALSE == bRet)
		{
			if (g_bUserBreak)
			{
				break;
			}
			MyLog(LOG_DEBUG, L"CalCheckValue Error.\n");
			break;
		}

		//Store the value
		bRet = SaveFileCheckValue(&filehandle, pucCheckValue, iValueLength);
		if (FALSE == bRet)
		{
			MyLog(LOG_DEBUG, L"SaveFileCheckValue Error.\n");
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	FileClose(&filehandle);

	MyLogShowTime(LOG_DEBUG, L"CalculateFileCheckValue ret: %d\n", bRet);
	return bRet;
}

BOOL ValidImageCheckValue(PFILE_SYS_INFO sysinfo, CHAR16 *filename, DWORD FileNumber)
{
	//return TRUE;

	MyLogShowTime(LOG_DEBUG, L"***ValidImageCheckValue..filename: %s FileNumber %d\n", filename, FileNumber);

	BOOL bRet = FALSE;
	FILE_HANDLE_INFO filehandle;
	mymemset(&filehandle, 0, sizeof(filehandle));

	do
	{
		if (FileNumber >= 0 && FileNumber <= 1)
		{
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"filename %s\n", filename);
#endif
			if (!FileOpen(sysinfo, &filehandle, filename, EFI_FILE_MODE_READ, 0, TRUE))
			{
				SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
				MyLog(LOG_DEBUG, L"FileOpen Error.\n");
				break;
			}
		}
		else if (FileNumber > 1)
		{
			CHAR16 fileFullPath[256] = { 0 };
			SPrint(fileFullPath, 256, L"%s.%d", filename, FileNumber - 1);
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"fileFullPath %s\n", fileFullPath);
#endif
			if (!FileOpen(sysinfo, &filehandle, fileFullPath, EFI_FILE_MODE_READ, 0, TRUE))
			{
				SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
				MyLog(LOG_DEBUG, L"FileOpen Error.\n");
				break;
			}
		}

		//pbkdf2
		unsigned char pucCheckValue[PBKDF2P_LEN] = { 0 };
		int iValueLength = PBKDF2P_LEN;
		memset(pucCheckValue, 0, iValueLength);

		bRet = CalCheckValue(&filehandle, filename, FileNumber, pucCheckValue, iValueLength, TRUE);
		if (FALSE == bRet)
		{
			if (g_bUserBreak)
			{
				break;
			}

			MyLog(LOG_DEBUG, L"CalCheckValue Error.\n");
			break;
		}

		///Get value from image file
		unsigned char pucCheckValueInFile[PBKDF2P_LEN] = { 0 };
		int iValueLengthInFile = PBKDF2P_LEN;

		bRet = GetFileCheckValue(&filehandle, pucCheckValueInFile, iValueLengthInFile);
		if (FALSE == bRet)
		{
			MyLog(LOG_DEBUG, L"GetFileCheckValue Error.\n");
			break;
		}

		//Compare value
		if (MyMemCmp(pucCheckValueInFile, pucCheckValue, iValueLengthInFile) == 0)
		{
			bRet = TRUE;			
		}
		else
		{
			bRet = FALSE;
		}

	} while (FALSE);

	FileClose(&filehandle);

	MyLogShowTime(LOG_DEBUG, L"ValidImageCheckValue ret: %d\n", bRet);
	return bRet;
}