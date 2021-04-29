//#include <stdio.h>
#include "okr.h"

BOOLEAN IsNoDataPartFlag = FALSE; 
	
BYTE Windows_Data_TypeGuid[16]={0xa2,0xa0,0xd0,0xeb,0xe5,0xb9,0x33,0x44,0x87,0xc0,0x68,0xb6,0xb7,0x26,0x99,0xc7};
BYTE Window_Recovery_TypeGuid[16]={0xa4,0xbb,0x94,0xde,0xd1,0x06,0x40,0x4d,0xa1,0x6a,0xbf,0xd5,0x01,0x79,0xd6,0xac};
BYTE Uefi_System_TypeGuid[16]={0x28,0x73,0x2a,0xc1,0x1f,0xf8,0xd2,0x11,0xba,0x4b,0x0,0xa0,0xc9,0x3e,0xc9,0x3b};
BYTE Window_Reserved_TypeGuid[16]={0x16,0xe3,0xc9,0xe3,0x5c,0x0b,0xb8,0x4d,0x81,0x7d,0xf9,0x2d,0xf0,0x02,0x15,0xae};
EFI_GUID PARTITION_LINUX_SWAP_GUID =  { 0x0657fd6d, 0xa4ab, 0x43c4, {0x84, 0xe5, 0x09, 0x33, 0xc8, 0x4b, 0x4f, 0x4f}};
EFI_GUID PARTITION_LINUX_BOOT_GUID =  { 0x21686148, 0x6449, 0x6e6f, {0x74, 0x4e, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49}};
BYTE Uefi_ESP_TypeGuid[16]={0xC1,0x2A,0x73,0x28,0xF8,0x1F,0x11,0xD2, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B};
 	
PBYTE g_lz4_buffer=NULL;
INTN  g_lz4_size; 			//=0 first time;>0 compressed size; <0 error
ULONGLONG gwireTotalSectors; 

UINT32 g_databitmapbufsize;

BOOLEAN  IsLinuxSystem = FALSE;
UINT32 gcount;


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
    EFI_INPUT_KEY key;
	EFI_STATUS status;

	status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
	if (status == EFI_SUCCESS) {
		if (key.ScanCode == SCAN_ESC) {
			////// press esc abort
			logflush(FALSE);
			if(g_data.Cmd == USER_COMMAND_BACKUP)
			{			
			  	if(DrawMsgBox(STRING_BACKUP_USER_BREAK, STRING_YES, STRING_NO) == IDOK)
				     return TRUE;			
			}
			else
		  {
		  	if(DrawMsgBox(STRING_USER_BREAK, STRING_YES, STRING_NO) == IDOK)
				  return TRUE;
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
        return -1;

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
//
int ProbeSystemPartition()
{
    DWORD i, systemPartition = 0;
    int diskindex = -1;
    int ret = 0;
    int init_file_sys_err = 0;
    
	ULONGLONG systemBeginSector = 0;
	CHAR16 LenovoLabel[32] = {L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T'};//ysy
    g_data.BackupDiskNumber = -1;

	IsLinuxSystem  = FALSE ;
	MyLog(LOG_DEBUG, L"Detecting system partition(%d)\n", g_dwTotalPartitions);

    //get efi ,recovery , reserved partition,and first data partition
	for (i=0;i<g_dwTotalPartitions;i++) {
		if (g_Partitions[i].bGpt 
			&& MyMemCmp(Uefi_System_TypeGuid,g_Partitions[i].TypeGuid,16) == 0) { //ESP 分区

			g_Partitions[i].bIfBackup = TRUE;
			diskindex = g_Partitions[i].DiskNumber;
			MyLog(LOG_DEBUG, L"Found efi system partition(%d:%d)\n", diskindex, i);
			break;
		}
	}

	//查找出第一个有windows目录的分区出来
	for(i=0;i<g_dwTotalPartitions;i++) 
	{
	   MyLog(LOG_DEBUG, L"Detecting system  %d,DiskNumber:%d\n",i, g_Partitions[i].DiskNumber);
		
		//					if(g_disklist[destdisk].removable)

		if(g_Partitions[i].TotalSectors > 1024*2048 
			&& g_Partitions[i].PartitionType == NTFS ) {
		//	&& (diskindex == -1 || diskindex == g_Partitions[i].DiskNumber)
	
			//系统分区必然大于1GB
			ret = HasSystemFile(i);
			if( ret == TRUE )
		  {	
				g_Partitions[i].bIfBackup = TRUE;
				g_Partitions[i].HasWindows = TRUE;
				systemBeginSector = g_Partitions[i].BootBegin;
				systemPartition = i;
				diskindex = g_Partitions[i].DiskNumber;
				MyLog(LOG_DEBUG, L"Found windows partition(%d:%d)\n", diskindex, i);
	
		//		break;
			}
			if( ret<0 )
			   init_file_sys_err++;	
		}
	}

	if (diskindex == -1 || !systemBeginSector) {
		int candidate = -1;
		//可能是linux系统，那么查找出第一个linux大小超过20G的linux分区作为系统分区
		for(i=0;i<g_dwTotalPartitions;i++) {
			if(g_Partitions[i].TotalSectors > 20*1024*2048
				&& (diskindex == -1 || diskindex == g_Partitions[i].DiskNumber)) {

				if(g_Partitions[i].PartitionType == PARTITION_LINUX) {
					g_Partitions[i].bIfBackup = TRUE;
					systemBeginSector = g_Partitions[i].BootBegin;
					systemPartition = i;
					diskindex = g_Partitions[i].DiskNumber;
					MyLog(LOG_DEBUG, L"Found linux partition(%d:%d)\n", diskindex, i);
					IsLinuxSystem = TRUE ;
					break;
				} else if (g_Partitions[i].PartitionType == NTFS || g_Partitions[i].PartitionType == FAT32) {
					if(candidate == -1)
						candidate = i;
				}
			}
		}


		if(!systemBeginSector && candidate != -1) {
/*
			i = (DWORD)candidate;
			g_Partitions[i].bIfBackup = TRUE;
			systemBeginSector = g_Partitions[i].BootBegin;
			systemPartition = i;
			diskindex = g_Partitions[i].DiskNumber;
*/
      diskindex = -1;
			MyLog(LOG_DEBUG, L"Assume first partition as system(%d:%d)\n", diskindex, i);
		}
	}


	if (diskindex == -1) {
		MyLog(LOG_DEBUG, L"No Operation System detected.\n");
		
		if( init_file_sys_err > 0)
		{
			return -1;
		}	
		return FALSE;
	}
	MyLog(LOG_DEBUG, L"System disk: %d.\n", diskindex);

	g_data.BackupDiskNumber = diskindex;

	//将系统分区之前的小分区都标记为需要备份

	for(i=0;i<g_dwTotalPartitions;i++)
	{
		if (g_data.BackupDiskNumber != g_Partitions[i].DiskNumber)
			continue;
		if(g_Partitions[i].TotalSectors < 1*1024*2048 && g_Partitions[i].BootBegin < systemBeginSector)	//小于1GB的隐藏分区需要备份
			g_Partitions[i].bIfBackup = TRUE;
	}

	//将系统分区活动分区和ESP分区都标记为需要备份
	for(i=0;i<g_dwTotalPartitions;i++)
	{
		if (g_data.BackupDiskNumber != g_Partitions[i].DiskNumber)
			continue;
		if(g_Partitions[i].bBootPartition == TRUE || ( g_Partitions[i].bGpt &&  (MyMemCmp(Uefi_ESP_TypeGuid,g_Partitions[i].TypeGuid,16) == 0) ))	//小于1GB的隐藏分区需要备份
			g_Partitions[i].bIfBackup = TRUE;
	}
	
	for(i=0;i<g_dwTotalPartitions;i++)
	{
		if (g_data.BackupDiskNumber != g_Partitions[i].DiskNumber)
			continue;
		//if(( g_Partitions[i].bGpt &&  (CompareGuid(&PARTITION_LINUX_SWAP_GUID,g_Partitions[i].TypeGuid)) ))	//swap分区需要备份
		//	g_Partitions[i].bIfBackup = TRUE;
			
		if(( g_Partitions[i].bGpt &&  (CompareGuid(&PARTITION_LINUX_BOOT_GUID,g_Partitions[i].TypeGuid)) ))	//legacy boot 分区需要备份
			g_Partitions[i].bIfBackup = TRUE;
	}

//
    //比较分区GUID，MS RESERVED PARTITION不需要备份
    for (i=0;i<g_dwTotalPartitions;i++) {


	  GetVolumeName(i);
        if (diskindex != g_Partitions[i].DiskNumber)
            continue;

        if (MyMemCmp(Window_Recovery_TypeGuid,g_Partitions[i].TypeGuid,16) == 0) {
		
			//if(i < systemPartition)
					if(MyMemCmp( g_Partitions[i].Label, LenovoLabel,22) != 0)//ysy  1210
				g_Partitions[i].bIfBackup = TRUE;
			
			MyLog(LOG_DEBUG, L"Found MS Recovery Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);

        } else if (MyMemCmp(Window_Reserved_TypeGuid,g_Partitions[i].TypeGuid,16) == 0) {

            g_Partitions[i].bIfBackup = FALSE;
            MyLog(LOG_DEBUG, L"Found MS Reserved Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);

        } else if (MyMemCmp(Windows_Data_TypeGuid,g_Partitions[i].TypeGuid,16) == 0 && g_Partitions[i].HasWindows != TRUE) {

           
           g_Partitions[i].bIfBackup = FALSE;
           
            MyLog(LOG_DEBUG, L"Found MS Data Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);

        }
		else { 
			MyLog(LOG_DEBUG, L"i:%x:bBackup %x,Type:%x,Id:%x,Label:%s\n", i, g_Partitions[i].bIfBackup,g_Partitions[i].PartitionType,g_Partitions[i].PartitionId,g_Partitions[i].Label);
		}
    }
MyLog(LOG_DEBUG, L"End ProbeSystemPartition\n");

	for (i=0;i<g_dwTotalPartitions;i++){

           
           if (MyMemCmp(Windows_Data_TypeGuid,g_Partitions[i].TypeGuid,16) == 0 && g_Partitions[i].HasWindows != TRUE)
		   	
           MyLog(LOG_DEBUG, L"Found zwang MS Data Partition %d, bBackup %d\n", i, g_Partitions[i].bIfBackup);
		}


    return TRUE;
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


//为了方便调用者showcount是字节
int GoProcessBar(DWORD showcount)
{

	if(g_showctrl)
	{
	  DWORD percent = 0;
	     
		if(showcount == -1)
		{
			percent = 100;
		}
		else
		{
    		g_shown_count += showcount/1024;	//KB
    		if(g_shown_count/1024 >= g_show_interval)
    		{
    			g_shown_unit += g_shown_count/1024;
  		  
  		    MyLog(LOG_DEBUG, L"g_shown_unit %d:%d\n",g_shown_unit, g_shown_count);
  		    MyLog(LOG_DEBUG, L"g_show_interval %d:%d\n",g_show_interval, g_show_total);
  		    			
    			g_shown_count = 0;
    			percent = (WORD)(((DWORD)g_shown_unit*100)/(DWORD)g_show_total);
     		
     		  MyLog(LOG_DEBUG, L"percent %d:%d\n",percent, g_show_percent);
  			    			
    		}
    	}
    	
    	
    	if(percent > g_show_percent)
    	{
		    struct DM_MESSAGE dmessage;
		    
		    if(percent > 100)
		    	percent = 100;
   			dmessage.message = WM_PROCESS_RUN;
   			dmessage.thex = percent;
   			g_showctrl->sendmessage(g_showctrl, &dmessage);
   			g_show_percent = percent;
   			
   			//刷新标题条中的百分号
   			{
			    DWORD x, y, d[3], i;
   				DWORD digit[10] = {
   					IMG_FILE_0, IMG_FILE_1, IMG_FILE_2, IMG_FILE_3, IMG_FILE_4, 
   					IMG_FILE_5, IMG_FILE_6, IMG_FILE_7, IMG_FILE_8, IMG_FILE_9};
   				x = g_x_begin + 430;
		    	y = (g_WindowHeight / 3 - 60) /2 + 180;
				if (g_WindowHeight <= 800)
				{
					y -= 80;
				}
				
				d[0] = percent/100;
		    	d[1] = (percent%100) / 10;
		    	d[2] = (percent%10);
		    	if(percent >= 100) i = 0;
		    	else if(percent >= 10) i = 1;
	    		else i = 2;
	    			
    			DrawLine(x, y, 200, 60, COLOR_CLIENT);
		    	for(;i<3;i++)
		    	{
					DisplayImg(x, y, digit[d[i]], FALSE);
					x += 40;
   				}
				DisplayImg(x, y, IMG_FILE_PERCENT, FALSE);
   			}
    	}
#ifdef SHOW_TIME    	
    	{
    		DWORD curtime, remain;
    		curtime = GetTickCount()/1000 - g_data.BeginTime;
    		if(curtime > g_data.LastTime)
    		{
				DWORD y;
				y = (g_WindowHeight / 3 - 60) / 2;
				if (g_WindowHeight <= 800)
				{
					y -= 80;
				}

				CHAR16 text[64] = { 0 };
    			SPrint(text, 64, L"%02d:%02d", curtime/60, curtime%60);
    			FillRect(g_x_begin+440, y + 350, 40+40, 18, COLOR_CLIENT);
// 	    		DisplayString(g_x_begin+320, g_y_begin+200,COLOR_BLACK,text);
 	    		DisplayString(g_x_begin+440, y + 350, COLOR_SEC_BLUE, text);
   	    		
    			if(!percent)
    				percent = g_show_percent;
    			if(percent)
    			{
    				remain = curtime * (100 - percent) / percent;
    				
   // 				if(1)
//    				if(remain < g_data.LastRemain)
    				if( percent > pre_show_percent )
    				{
				    	pre_show_percent = percent;

    					SPrint(text, 64, L"%02d:%02d", remain/60, remain%60);
    					FillRect(g_x_begin+440, y + 380, 40+40, 18, COLOR_CLIENT);
//   	    			DisplayString(g_x_begin+320, g_y_begin+218, COLOR_BLACK,text);
   	    			DisplayString(g_x_begin+440, y + 380, COLOR_SEC_BLUE, text);
	   	    			g_data.LastRemain = remain;
	   	    	}    	
	   	    }
	   	    
	   	    
    			g_data.LastTime = curtime;
    		}
   	    }
#endif   	    
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

//  g_data.LastTime = 0xFFFF;
	
	return 0;
}
//为了方便调用者showcount是字节
int MyShowGoProcessBar(DWORD  showcount)
{
	if(g_showctrl)
	{
	    DWORD percent = 0;
	    
		if(percent == -1)
		{
			percent = 100;
		}
    else
		{
    		g_shown_count += showcount/1024;	//KB
    		if(g_shown_count/1024 >= g_show_interval)
    		{
    			g_shown_unit += g_shown_count/1024;
    			g_shown_count = 0;
    			percent = (WORD)(((DWORD)g_shown_unit*100)/(DWORD)g_show_total);
    		}
    	}	
    	if(percent > g_show_percent)
    	{
		    struct DM_MESSAGE dmessage;
		    
		    if(percent > 100)
		    	percent = 100;
   			dmessage.message = WM_PROCESS_RUN;
   			dmessage.thex = percent;
   			g_showctrl->sendmessage(g_showctrl, &dmessage);
   			g_show_percent = percent;
   			
   			//刷新标题条中的百分号
   			{
			    WORD x, y, d[3], i;
   				WORD digit[10] = {
   					IMG_FILE_0, IMG_FILE_1, IMG_FILE_2, IMG_FILE_3, IMG_FILE_4, 
   					IMG_FILE_5, IMG_FILE_6, IMG_FILE_7, IMG_FILE_8, IMG_FILE_9};
   				x = g_x_begin + 320;
		    	y = (g_WindowHeight / 3 - 60) /2 + 8;
		    	d[0] = percent/100;
		    	d[1] = (percent%100) / 10;
		    	d[2] = (percent%10);
		    	if(percent >= 100) i = 0;
		    	else if(percent >= 10) i = 1;
	    		else i = 2;
	    			
    			DrawLine(x, y, 200, 60, COLOR_TITLE_L_BLUE);
		    	for(;i<3;i++)
		    	{
					DisplayImg(x, y, digit[d[i]], FALSE);
					x += 32;
   				}
				DisplayImg(x, y, IMG_FILE_PERCENT, FALSE);
   			}
   			
   			return 0;
    	}
#ifdef SHOW_TIME    	
    	{
    		DWORD curtime, remain;
    		curtime = GetTickCount()/1000 - g_data.BeginTime;
    		if(curtime > g_data.LastTime)
    		{
	    		CHAR16 text[64] = { 0 };
    			SPrint(text, 64, L"%02d:%02d", curtime/60, curtime%60);
    			FillRect(g_x_begin+320, g_y_begin+200, 40, 18, COLOR_CLIENT);
// 	    		DisplayString(g_x_begin+320, g_y_begin+200,COLOR_BLACK,text);
 	    		DisplayString(g_x_begin+320, g_y_begin+200,COLOR_TEXT,text);
   	    		
    			if(!percent)
    				percent = g_show_percent;
    			if(percent)
    			{
    				remain = curtime * (100 - percent) / percent;
    				if(remain < g_data.LastRemain)
    				{
    					SPrint(text, 64, L"%02d:%02d", remain/60, remain%60);
    					FillRect(g_x_begin+320, g_y_begin+218, 40, 18, COLOR_CLIENT);
//   	    			DisplayString(g_x_begin+320, g_y_begin+218, COLOR_BLACK,text);
   	    			DisplayString(g_x_begin+320, g_y_begin+218, COLOR_TEXT,text);
	   	    			g_data.LastRemain = remain;
	   	    		}
	   	    	}
    			g_data.LastTime = curtime;
    		}
   	    }
#endif   	    
    }
    return 0;
}
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
        blocks = DivU64x32((g_Partitions[i].TotalSectors+63),64);
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
        
        blocks = DivU64x32((g_Partitions[i].TotalSectors+63),64);
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
		    
            if(UserBreak() || g_lz4_size < 0)
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

        blocks = DivU64x32((pheader->Partition[i].TotalSectors+63),64);
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
	
	CopyMem(TempBuffer,g_disklist[g_data.BackupDiskNumber].disksn,20);       //length 20 system disk sn
	
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

	filenum = GetFileNamelist(SysInfo, (CHAR16*)Buffer, 1024*1024);
	if(filenum > 0)
	{
		filename = (CHAR16*)Buffer;
		while (filenum) {
			CHAR16 fullpath[256] = { 0 };
			SPrint(fullpath, 256, BACKUP_DIR L"\\%s", filename);
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
			continue;

		if ((g_Partitions[i].PartitionType!=FAT32)
			&&(g_Partitions[i].PartitionType!=FAT32_E)
			&&(g_Partitions[i].PartitionType!=NTFS))
			continue;

		if (DivU64x32(g_Partitions[i].TotalSectors, 2048*1024) < 4)	//分区大小小于4GB，可以忽略
			continue;

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

    		delayEx(1000);
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
    		delayEx(1000);
    	}
    }
    
 //   GoProcessBar(-1);
    
    return 0;
}

 
int ReadBackupDir(FILE_SYS_INFO *sysinfo)
{
    //////////
    EFI_STATUS status;
    int ret=0;
    BYTE buffer[1024] = { 0 };
    UINTN filesize;
    FILE_HANDLE_INFO filehandle;

    if (!FileOpen(sysinfo, &filehandle, BACKUP_DIR, EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE)) 
    {
    	  MyLog(LOG_DEBUG, L"ReadBackupDir,%s error\n",BACKUP_DIR);
        return -1;
    }
      
    filesize = 1024;
    if (!FileRead(&filehandle,&filesize ,buffer)) 
    {
      
        FileClose(&filehandle);
            
        MyLog(LOG_DEBUG, L"ReadBackupDir,read %s error\n",BACKUP_DIR);
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
	OKR_HEADER *pheader;
	CHAR16 name[100];
	UINT64 totaloriginsize = 0;
	UINT64 bigfiletotalsize = 0;//记录整个文件的大小
	struct CONTINUE_RWDISK_INFO conti_info;
	PBUFFER_INFO buffinfo;
	struct BUFFERARRAY buffarray;
	struct MP_INFO mpinfo;
	FILE_HANDLE_INFO filehandle;
	FILE_SYS_INFO  sysinfo;
	WORD i;
	int ret = -1;
	BOOL bCreatDirError = FALSE;
	BOOL bFileCreated = FALSE, bShowError = TRUE;
  BYTE *align_buffer = NULL;
  UINT64 head_alignlen = 0,after_alignlen = 0;
  UINT64 alignbufsize = 8*1024*1024;
  ULONGLONG maxpartitionsize = 0;
  BYTE ntfsguid[512]={0};
  
  
  align_buffer = myalloc(alignbufsize);
  
  if(align_buffer==NULL)
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
		g_data.CompressBlockSize/(1024*1024), g_data.MemoryPagesNumber);
	pheader = CreateImageHeader(desc);
	if(!pheader)
		goto Done;

	if(!InitGlobalDataForBackup(pheader))
		goto Done;

	MyLog(LOG_DEBUG, L"FileSystemInit(%d).\n", Destpart);
    //初始化文件系统
    if (FileSystemInit(&sysinfo, Destpart, FALSE) == FALSE) {
  //  	DrawMsgBox(STRING_FS_INIT_FAILED, STRING_YES, STRING_NO);
  		DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1,STRING_FS_INIT_FAILED_2,STRING_FS_INIT_FAILED_3,NULL,STRING_YES, STRING_NO);

		  bShowError = FALSE;
        goto Done;
    }

	//if(g_data.BackupDiskNumber == g_Partitions[Destpart].DiskNumber)
		pheader->bFactoryBackup = (g_data.bHasFactoryBackup > 0) ? FALSE : TRUE;
    
    if(if_init_backup)
    {
    	 pheader->bFactoryBackup = TRUE;
    }
    else
    {
    	 pheader->bFactoryBackup = FALSE;    	
    }
    
    //创建.okr目录
	StrnCpy(name, BACKUP_DIR, 100);
	MyLog(LOG_DEBUG, L"CreateDirectory(%s).\n", name);
    if (!FileOpen(&sysinfo,&filehandle,name,EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ|EFI_FILE_MODE_CREATE,EFI_FILE_DIRECTORY,FALSE))
    {
		SET_ERROR(__LINE__, OKR_ERROR_CREATE_DIRECTORY);
        goto Done;
    }
    FileClose(&filehandle);

   if( ReadBackupDir(&sysinfo) < 0 )
   {
 		//   SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);       
       bCreatDirError = TRUE;
       goto Done;  	  
   }

	//打开日志文件
	logopen(&sysinfo);

    //SPrint(name, 100, BACKUP_DIR L"\\%x", (UINTN)pheader->Time);
	if (g_Partitions[Destpart].PartitionType == NTFS) {
		if(pheader->bFactoryBackup || desc[0] >= 0x80)
			SPrint(name, 100, BACKUP_DIR L"\\%x-initial backup", (UINTN)pheader->Time);
		else if(desc && desc[0])
			SPrint(name, 100, BACKUP_DIR L"\\%x-%s", (UINTN)pheader->Time, desc);
		else
			SPrint(name, 100, BACKUP_DIR L"\\%x", (UINTN)pheader->Time);
	} else {
		SPrint(name, 100, BACKUP_DIR L"\\%x", (UINTN)pheader->Time);
	}
    MyLog(LOG_DEBUG,L"CreateFile(%s).\n",name);
    if (!FileOpen(&sysinfo,&filehandle,name,EFI_FILE_MODE_CREATE|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ,0,TRUE)) {
		SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
        goto Done;
    }
	bFileCreated = TRUE;

	 MyLog(LOG_DEBUG, L"Creating image header(%d bytes).\n", pheader->StructSize);
	 
	
	 CopyMem(align_buffer, (PBYTE)pheader,pheader->StructSize);
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

		if (DiskRead((WORD)g_data.BackupDiskNumber, 0, pheader->GptSize/SECTOR_SIZE, buffer) == FALSE) {
			SET_ERROR(__LINE__, OKR_ERROR_READ_GPT);
			goto Done;
		}
		
  	CopyMem(align_buffer+head_alignlen, buffer,pheader->GptSize);
  	
		head_alignlen += pheader->GptSize;
		
    after_alignlen = CalcAliginSize(head_alignlen);
    
    if( after_alignlen > alignbufsize )
    {
  		  
  		  MyLog(LOG_DEBUG,L"Backup CalcAliginSize beyond buffer\n");
  	
    	  goto Done;
    	      	
    }
    
    MyLog(LOG_DEBUG,L"Backup after:%d,len:%d r\n",after_alignlen,head_alignlen);

    //一次性按4k对齐写入数据
		if (!FileWrite(&filehandle,after_alignlen, align_buffer)) {
			SET_ERROR(__LINE__, OKR_ERROR_BACKUP_GPT);
			goto Done;
		}
		

//		if (!FileWrite(&filehandle, pheader->GptSize, buffer)) {
//			SET_ERROR(__LINE__, OKR_ERROR_BACKUP_GPT);
//			goto Done;
//		}
//		MyLog(LOG_DEBUG,L"Backup GPT %d bytes\n", pheader->GptSize);
		MyLog(LOG_DEBUG,L"Backup after_alignlen %d bytes\n",after_alignlen);

    bigfiletotalsize = after_alignlen;
    
	//	bigfiletotalsize += pheader->GptSize;
	}
	pheader->DataOffset = (DWORD)bigfiletotalsize;
    
	//支持硬盘异步IO
	if(g_data.bMultiThread && g_data.bAsyncIO && g_data.BackupDiskNumber != g_Partitions[Destpart].DiskNumber)
    {
	    ProcessBarInit(pctl, g_data.TotalBackupSize);
		MyLog(LOG_DEBUG, L"BackupWithAsyncIo.\n");
		logflush(FALSE);
    	ret = BackupWithAsyncIo(&filehandle, pheader, &buffarray, &mpinfo);
    	if(ret == 0)
    		goto updateheader;
    	else
    		goto Done;
    }

	logflush(FALSE);
	
	   if( sysinfo.ifntfs )
	  {
	       ProcessBarInit(pctl, g_data.TotalBackupSize + g_data.TotalBackupSize);		
  	}
	  else
	  {	
	  	  UINT32 datatotalsize =  0;
	  	  UINT64 backupdatatotalsize = g_data.TotalBackupSize + g_data.TotalBackupSize;
	  	
	  	  datatotalsize = ((g_data.TotalBackupSize/0x100000)/0x400);
	  	  
	  	  if( datatotalsize >= 5 && datatotalsize <= 40 )
	  	  {
	  	  	backupdatatotalsize = ((g_data.TotalBackupSize + g_data.TotalBackupSize)*(70-( (datatotalsize - 5)/2 )))/100;	  	  	
	  	  }
	  	  else if ( datatotalsize > 40 )
	  	  {
	 	  	  	backupdatatotalsize = ((g_data.TotalBackupSize + g_data.TotalBackupSize)*(66))/100;	  	  	 	   
	  	  }
	  	  	
	  	  	  	  
  	//		MyLog(LOG_DEBUG,L"Backup TotalBackupSize %d M,%d\n",(g_data.TotalBackupSize/1024)/1024,backupdatatotalsize);
     //   getch();

	      ProcessBarInit(pctl, backupdatatotalsize);
         
//	      ProcessBarInit(pctl, ((g_data.TotalBackupSize + g_data.TotalBackupSize)*50)/100);
    }
	MyLog(LOG_DEBUG, L"BackupDisk.guid= %x %x %x-%x %x %x\n",g_disklist[g_data.BackupDiskNumber].guid[0],g_disklist[g_data.BackupDiskNumber].guid[1],g_disklist[g_data.BackupDiskNumber].guid[2],g_disklist[g_data.BackupDiskNumber].guid[13],g_disklist[g_data.BackupDiskNumber].guid[14],g_disklist[g_data.BackupDiskNumber].guid[15]); 

    for (i=0;i<g_dwTotalPartitions;i++) {
        VOID *bitbuffer = NULL;
        INTN   size = 0;
        ULONGLONG totalblocks = 0,currentblock = 0,origin_datasize = 0,Lastblock = 0,Lastblock_1 = 0;
        ULONGLONG beginsec = 0;
        ULONGLONG totalwrite = 0,readn = 0, lastread = 0;
		
		
		if (g_data.BackupDiskNumber == g_Partitions[i].DiskNumber) // fix linux_swap bug
		{
			 
			if(CompareGuid(&PARTITION_LINUX_SWAP_GUID,g_Partitions[i].TypeGuid))
			{
				MyLog(LOG_DEBUG, L"Found Linux Swap= %d\n", i);
				DiskRead((WORD)g_data.BackupDiskNumber, g_Partitions[i].BootBegin, 12, pheader->TempBuffer);
			}
				
			MyLog(LOG_DEBUG, L"g_Partitions[%d].PartitionType= %d\n", i,g_Partitions[i].PartitionType);
						 
		}
			
		
        if (!g_Partitions[i].bIfBackup)
            continue;

        //scan disk
        totalblocks = DivU64x32((g_Partitions[i].TotalSectors+BLOCK_SIZE-1),BLOCK_SIZE);
        size = DivU64x32((totalblocks + 7), 8);
        bitbuffer = g_data.databitmap;
		    //SetMem(bitbuffer,size,0);
        SetMem(bitbuffer,g_databitmapbufsize,0);
        
        //MyLog(LOG_DEBUG,L"scan  part %d, blocks %ld,bitbuffer=%x,size=%d\n",i,blocks,bitbuffer,size);
        
        //获得该分区的pagefile以及hibernate的扇区列表, 该函数第二次执行时就是简单的获取第一次的结果
        GetUnusedDataList(i);

        if (ScanPartitionData(i,GetDataToBitmap,bitbuffer,(PBYTE)&pheader->Magic) < 0) {
            pheader->Partition[i].BackupDataSize = 0;
			MyLog(LOG_DEBUG, L"Failed to scan filesystem %d\n", i);
            continue;
        }
		Lastblock =0;
		Lastblock_1 =0;
		//kang -- if(g_data.bDebug) 
		{
			origin_datasize=0;
			for (currentblock=0;currentblock<totalblocks;currentblock++) {
			    if (ReadBit(currentblock,bitbuffer))
				{
			        origin_datasize+=BLOCK_SIZE*512;
					Lastblock_1 = Lastblock;
					Lastblock = currentblock;
				}
			}
		//kang--	MyLog(LOG_DEBUG, L"Backup partition(%d), datasize 0x%lx bytes\n", i, origin_datasize);
		}
			//	memset(ntfsguid,0,512);//ysy 1226
		if(g_Partitions[i].HasWindows)
		{
		memset(ntfsguid,0,512);//ysy 1226
			if((Lastblock - Lastblock_1)> 0x8000)
				maxpartitionsize = Lastblock_1;
			else
				maxpartitionsize = Lastblock;
			
			
			// 保存ntfs 系统分区guid
			DiskRead((WORD)g_data.BackupDiskNumber, g_Partitions[i].BootBegin, 1, ntfsguid);	
			MyLog(LOG_DEBUG, L"ntfs g_Partitions[%d] ntfsguid= %x  %x  %x  %x\n", i,ntfsguid[0x48],ntfsguid[0x49],ntfsguid[0x4a],ntfsguid[0x4b]);
			
		}
		MyLog(LOG_DEBUG, L"Backup partition(%d),   maxpartitionsize = 0x%llx\n", i, maxpartitionsize);
		MyLog(LOG_DEBUG, L"Backup partition(%d), datasize 0x%llx bytes, Lastblock = 0x%llx,Lastblock_1 =0x%llx \n", i, origin_datasize,Lastblock,Lastblock_1);
		//WRITE bitmap
		//按4k写入整个Buffer

	   
		    after_alignlen = CalcAliginSize(size);
        //计算对齐后大小是否大于原buffer的大小
		    if(after_alignlen > g_databitmapbufsize)
		    {
		    	  MyLog(LOG_ERROR,L"write bitmap,beyond align buffer\n");
            goto Done;
		    }	
		    
		    MyLog(LOG_DEBUG,L"Backup:%d,off:%d,after2:%d,size:%d \n",i,bigfiletotalsize,after_alignlen,size);
		   // getch();
		    size = after_alignlen;
        if (!FileWrite(&filehandle,size,bitbuffer)) {
        	  ret = -1;
        	 // bCreatDirError = TRUE;
            MyLog(LOG_ERROR,L"write bitmap error\n");
            goto Done;
        }		    
 
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
        currentblock=0;
        SmartDiskIoInit((BYTE)g_data.BackupDiskNumber, COMMAND_READ, &conti_info);
        origin_datasize=0;
        lastread = 0;
        while (1) {
			if(UserBreak()) {
				
				
				bShowError = FALSE;
				goto Done;
			}
            
            if(g_data.bMultiThread)
            {
				//多线程串行IO, 在读扇区的时候进行压缩，等读取完毕，压缩也结束了，可以进行写盘操作
            	buffinfo = GetConsumerAnyway(&buffarray);
            	if (buffinfo->datavalid) {
            	    StartAP(&mpinfo, CompressBuffer, buffinfo);
            	}
            }
            ///////// read sector to buffer
			readn = 0;
			buffinfo = GetProducter(&buffarray);
			if(currentblock < totalblocks) {

				while (currentblock < totalblocks) {
					if (ReadBit(currentblock,bitbuffer)) {
						SmartDiskIo(&conti_info,MultU64x32(currentblock, BLOCK_SIZE)+beginsec,BLOCK_SIZE,buffinfo->pointer+readn);
						readn += BLOCK_SIZE*512;
						GoProcessBar(BLOCK_SIZE*512);

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

			//等候数据压缩线程
            if(g_data.bMultiThread) {
            	WaitToComplete(&mpinfo);
            } else {
            	CompressBuffer(buffinfo);
            }

            //MyLog(LOG_DEBUG,L"wait compress  %d",g_lz4_size);
			if(UserBreak()) {
				bShowError = FALSE;
				goto Done;
			}
			if(g_data.bDebug){
				MyLog(LOG_DEBUG, L"Read 0x%llx,off:%llx comp:0x%llx", readn,bigfiletotalsize, g_lz4_size);
			}

			// write compressed data to file
			if(g_lz4_size == 0) {
				MyLog(LOG_DEBUG, L"Partition(%d) backup completed.", i);
				break;
			} else if (g_lz4_size < 0) {
                //MyLog(LOG_DEBUG, L"compress error %d", g_lz4_size);
                break;
            } else if (g_lz4_size > 8) {
                if (!FileWrite(&filehandle,g_lz4_size,g_lz4_buffer)) {
					SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
                    goto Done;
                }
                totalwrite += g_lz4_size;
                bigfiletotalsize += g_lz4_size;
                
	            if((INTN)lastread > g_lz4_size)
    	        {
    	           MyLog(LOG_DEBUG,L"backup  progress %d \n",(DWORD)(lastread - g_lz4_size));
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
	  pheader->FileNumber = filehandle.currentfileseg+1;

    FileClose(&filehandle);
    
	//UPDATE HEADER
    pheader->bComplete = TRUE;
    pheader->FileSize = bigfiletotalsize;
    pheader->OriginalSize = totaloriginsize;
    pheader->CompressBlockSize = g_data.CompressBlockSize;
	CopyMem( pheader->SourceDiskGUID,g_disklist[g_data.BackupDiskNumber].guid,16);
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
	updatepartcsize(pheader->Memo + 100 , maxpartitionsize);
	  g_data.bHasFactoryBackup = TRUE;
    
updateheader:

    if (!FileOpen(&sysinfo,&filehandle,name,EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ,0,FALSE)) {
		SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
        goto Done;
    }
 	
  	CopyMem(align_buffer, (PBYTE)pheader,pheader->StructSize);
  
    after_alignlen = CalcAliginSize(head_alignlen);
    //一次性按4k对齐写入数据
		if (!FileWrite(&filehandle,after_alignlen, align_buffer)) {
			SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
			goto Done;
		}
/*  

    if (!FileWrite(&filehandle, pheader->StructSize, (PBYTE)pheader)) {
		SET_ERROR(__LINE__, OKR_ERROR_WRITE_FILE);
        goto Done;
    }
 */    
    ret = 0;
    
  //  MyLog(LOG_DEBUG,L"backup complete progress %d \n",g_show_percent);
    
    BackupLastProcess();
   
    
    GoProcessBar(-1);

	//延时1秒，让用户可以看到100%
	delayEx(1000);

Done:
	
	if(align_buffer!= NULL)
	{
		myfree(align_buffer);	
		align_buffer = NULL;
	}
	if(g_data.databitmap)
		myfree(g_data.databitmap);
	g_data.databitmap = NULL;
	if(g_data.scanbuffer)
		myfree(g_data.scanbuffer);
	g_data.scanbuffer = NULL;
	if(g_data.groupdesc)
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
    
    if(ret < 0) {

		MyLog(LOG_DEBUG, L"Backup failed with error %d, LineNumber %d\n", g_data.dwErrorCode, g_data.dwLineNumber);

		if(bFileCreated) {
			//备份失败，删除文件
			FileDelete(&sysinfo, name);
			for(i=1;i<=(WORD)filehandle.currentfileseg;i++) {
				SPrint(name,100,L"%s.%d",filehandle.filename, i);
				FileDelete(&sysinfo, name);
			}
		}
		logflush(FALSE);

    if(bCreatDirError)
    {
    	//针对备份过程拔掉U盘 
  		DrawMsgBox(STRING_CREATE_DIR_ERROR, STRING_YES, NULL);
    	DestroyProcessBar();  	
  
  		MyLog(LOG_DEBUG, L"Backup failed bCreatDirError error\n");

  	 	return ret;
    }	
		if( bShowError ){
			WCHAR *szError = name;

			if(g_data.dwErrorCode == OKR_ERROR_WRITE_FILE) {
				szError = STRING_ERROR_WRITE_FILE;
			} else {
				SPrint(szError, 100, STRING_ERROR, g_data.dwErrorCode, g_data.dwLineNumber);
			}

			DrawMsgBox(szError, STRING_YES, NULL);
		}
			
  	DestroyProcessBar();
 
	//	MyLog(LOG_DEBUG, L"Backup failed bShowError %d \n",bShowError);
	//	MyLog(LOG_DEBUG, L"Backup failed bCreatDirError %d \n",bCreatDirError);
  	if( bCreatDirError != FALSE)  	
//  	if( bShowError != FALSE)  	
  	   return ret;

	} else {
		MyLog(LOG_DEBUG, L"Backup successfully.\n");
		logflush(FALSE);
	}

	//关闭日志文件
	FileClose(&g_data.hLog);


  FileSystemClose(&sysinfo);

	if(g_data.bDebug) {
		//MyLog(LOG_DEBUG, L"Press any key to continue.\n");
		//getch();
	}

	return ret;
}


int GetFileNamelist(FILE_SYS_INFO *sysinfo, CHAR16 *filelist, INTN size)
{
    //////////
    EFI_STATUS status;
    int ret=0;
    BYTE buffer[1024] = { 0 };
    UINTN filesize;
    FILE_HANDLE_INFO filehandle;

    if (!FileOpen(sysinfo, &filehandle, BACKUP_DIR, EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE)) 
    {
    	  MyLog(LOG_DEBUG, L"GetFileNamelist,%s error\n",BACKUP_DIR);
        return -1;
    }

    while (size>0) {
        filesize = 1024;
        if (!FileRead(&filehandle,&filesize ,buffer)) {
            FileClose(&filehandle);
            
         	  MyLog(LOG_DEBUG, L"GetFileNamelist,read %s error\n",BACKUP_DIR);
            return -1;
        }

        if (filesize == 0)
            break;
        if (StrnCmp(((EFI_FILE_INFO *)buffer)->FileName,L".",10)==0)
            continue;
        if (StrnCmp(((EFI_FILE_INFO *)buffer)->FileName,L"..",10)==0)
            continue;
        StrnCpy(filelist,((EFI_FILE_INFO *)buffer)->FileName,100);
        filelist += StrLength(((EFI_FILE_INFO *)buffer)->FileName)+1;
        size -= StrLength(((EFI_FILE_INFO *)buffer)->FileName)+1;
        ret++;
    }
    
    MyLog(LOG_DEBUG,L"scan %ld files",ret);
    
    FileClose(&filehandle);
    
    return ret;
}

//获得备份点文件列表，同时删除未完成的备份点
int GetBackupFileList(int partindex, OKR_HEADER* pbackuplist,int maxnum,int *img_err)
{
    CHAR16 * buffer = NULL;
    CHAR16 * filename;
    BYTE	miniheader[OKR_HEADER_MIN_SIZE];
    OKR_HEADER *header = (OKR_HEADER *)miniheader;
    int filenum, namelen;
    int count = 0,img_count = 0;
    UINTN filesize;
    //////////
    FILE_HANDLE_INFO filehandle;
    FILE_SYS_INFO  sysinfo;
    EFI_STATUS status;
    BOOL ret, bReadOnly = FALSE, bDelete;

    if (FileSystemInit(&sysinfo,partindex,FALSE)==FALSE) 
	{
		bReadOnly = TRUE;
		if (FileSystemInit(&sysinfo,partindex,TRUE)==FALSE) 
    		return 0;
	}
    
    buffer = AllocatePool(1024*1024);
    if(!buffer)
    	goto cleanup;
    if ((filenum = GetFileNamelist(&sysinfo, buffer, 1024*1024)) <= 0)//
        goto cleanup;

    filename = buffer;
    while (filenum) {
        CHAR16 fullpath[256] = { 0 };
        SPrint(fullpath, 256, BACKUP_DIR L"\\%s", filename);
		namelen = StrLength(filename) + 1;
		MyLog(LOG_DEBUG, L"Checking %s\n", fullpath);

		bDelete = FALSE;
		if(namelen <= sizeof(pbackuplist->FileName)/2)	{

			if (!FileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, FALSE)) {
				MyLog(LOG_ERROR,L"Open %s file error\n", fullpath);

			} else {
				filesize = OKR_HEADER_MIN_SIZE;
				ret = FileRead(&filehandle, &filesize, (PBYTE)header);
				if (!ret || filesize != OKR_HEADER_MIN_SIZE) {

					MyLog(LOG_DEBUG, L"Invalid image header(%s)\n", fullpath);

				} else {
					if (header->Magic == OKR_IMAGE_MAGIC 
						&& (header->StructSize & (SECTOR_SIZE - 1)) == 0
						&& header->StructSize <= OKR_HEADER_MAX_SIZE
						&& header->StructSize >= OKR_HEADER_MIN_SIZE) {

							if(header->bComplete == TRUE) {

								header->PartitionIndex = partindex;
								CopyMem(&pbackuplist[count], header, OKR_HEADER_MIN_SIZE);
								CopyMem(&pbackuplist[count].FileName, filename, namelen*2);

								MyLog(LOG_DEBUG,L"Found image %d:%s\n", (UINT64)pbackuplist[count].Time, pbackuplist[count].Memo);
								count++;
								if (count >= maxnum)
									break;
							} else {
								MyLog(LOG_DEBUG,L"Inconsistent image, bComplete = FALSE,%x",header->bFactoryBackup);
								if(!bReadOnly) {
								//删除所有不完整备份	if(header->bFactoryBackup == TRUE)
									   bDelete = TRUE;							
								}
								*img_err = ++img_count;
								
   					//		MyLog(LOG_DEBUG,L"Found error image %d\n", *img_err);

							}
					} else if (header->Magic != OKR_IMAGE_MAGIC) {

						MyLog(LOG_DEBUG,L"Invalid magic data.");
					}
				}

				FileClose(&filehandle);


				//未完成备份的文件需要自动删除
				if(bDelete) 
				{
				  CHAR16 tmpname[256] = {0};

					DWORD i;
					UINT64 filesize;
					
					CopyMem(tmpname,fullpath,sizeof(fullpath));
					
					for(i=1;i<=MAX_DELE_FILE_NUM;i++) {
						SPrint(fullpath, 256, BACKUP_DIR L"\\%s.%d", filename, i);
						GetFileSizeByName(&sysinfo, fullpath, &filesize);
						if(FileDelete(&sysinfo, fullpath))
							g_Partitions[partindex].FreeSectors += filesize/SECTOR_SIZE;
					}
		
				  GetFileSizeByName(&sysinfo, tmpname, &filesize);
					//删除备份点以后，就要增加这个文件所在的分区的剩余空间
					if(FileDelete(&sysinfo, tmpname))
						g_Partitions[partindex].FreeSectors += filesize/SECTOR_SIZE;
									
	      	MyLog(LOG_DEBUG,L"delete init back image success");			
					(*img_err) --;
				}
			}
		} else {
			//文件名长度超过0x80字节，则失败
		}

		filename += namelen;
		filenum--;
    }
    
cleanup:
	if(buffer)
    	FreePool (buffer);
    FileSystemClose(&sysinfo);
    
    return count;
}

//获取所有分区里的镜像文件的文件头，保存到g_data.images里
int GetAllBackupFiles(int *img_err_count)
{
	DWORD num = 0;
	DWORD maxnum = HEADLISTBUFFER/OKR_HEADER_MIN_SIZE;
	DWORD i, j;
	BYTE tmpimage[OKR_HEADER_MIN_SIZE];
	int img_count = 0,all_img_count = 0;
	
	if(!g_data.images)
		g_data.images = myalloc(HEADLISTBUFFER);
	
    for (i=0;i<g_dwTotalPartitions;i++) {

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
 	
    	if(num >= maxnum)
    		break;
	}
	g_data.ImageFilesNumber = num;

	//找到所有文件后，按时间从现在往前排序
	for(i=0;i<num;i++) {
		for(j=i+1;j<num;j++){
			if(g_data.images[i].FileTime < g_data.images[j].FileTime) {
				CopyMem(tmpimage, &g_data.images[i], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[i], &g_data.images[j], OKR_HEADER_MIN_SIZE);
				CopyMem(&g_data.images[j], &tmpimage, OKR_HEADER_MIN_SIZE);
			}
		}
	}
	
	*img_err_count = all_img_count;

    MyLog(LOG_DEBUG, L"GetAllBackupFiles.img_err_count:%d\n",all_img_count);
	
	return num;
}


int CheckAllBackupFiles()
{
	DWORD num = 0;
	DWORD filecount = 0;
	DWORD maxnum = HEADLISTBUFFER/OKR_HEADER_MIN_SIZE;
	DWORD i, j;
  CHAR16 FileNamebuffer[256]; 
  FILE_HANDLE_INFO filehandle;
  FILE_SYS_INFO  sysinfo;
  BOOL readonly = FALSE;
  int partindex = 0;
  int img_err_count = 0,img_err_del = 0;

 //   MyLog(LOG_DEBUG, L"CheckAllBackupFiles.FileNumber:%d\n",g_data.ImageFilesNumber);
    
	//找到所有文件后
	for(i=0;i<g_data.ImageFilesNumber;i++) 
  {
  	partindex = g_data.images[i].PartitionIndex;
  	
  	readonly = FALSE;
  	
  	if (FileSystemInit(&sysinfo,partindex,FALSE)==FALSE)
  	{
  		 readonly = TRUE;
  		 if (FileSystemInit(&sysinfo,partindex,TRUE)==FALSE)
        continue;
    } 
    filecount = 0;
 
   // MyLog(LOG_DEBUG, L"partindex:%d,i %d.FileNumber:%d\n",partindex, i,g_data.images[i].FileNumber);
   
		for(j=0;j<g_data.images[i].FileNumber;j++)
		{
      if( j == 0)
      {
         SPrint(FileNamebuffer, 256, BACKUP_DIR L"\\%s", g_data.images[i].FileName);   	
      }
      else
      {
      	 SPrint(FileNamebuffer, 256, BACKUP_DIR L"\\%s.%d", g_data.images[i].FileName,j);
      }
      
      if(  IsExistFileInDir_NTFS_FAT(&sysinfo,FileNamebuffer) == 	TRUE )
      {
      	  filecount++;
      }
    //  MyLog(LOG_DEBUG, L"filecount %d.%s\n", filecount,FileNamebuffer);

		}
		

		if(g_data.images[i].FileNumber!=filecount && readonly == FALSE && g_data.images[i].bFactoryBackup == TRUE)
		{
	       CHAR16 fullpath[256] = { 0 };
				 
					DWORD k;
					UINT64 filesize;
		      


					for(k=1;k<=MAX_DELE_FILE_NUM;k++) 
					{
						SPrint(fullpath, 256, BACKUP_DIR L"\\%s.%d", g_data.images[i].FileName, k);
  
  		    //  MyLog(LOG_DEBUG, L"%d,fullpath %s\n",k,fullpath);

						GetFileSizeByName(&sysinfo, fullpath, &filesize);
						if(FileDelete(&sysinfo, fullpath))
							g_Partitions[partindex].FreeSectors += filesize/SECTOR_SIZE;
					}
				  
				  SPrint(fullpath, 256, BACKUP_DIR L"\\%s", g_data.images[i].FileName);   	

		   //   MyLog(LOG_DEBUG, L"fullpath %s,%d\n",fullpath,g_data.images[i].FileNumber);
	
					GetFileSizeByName(&sysinfo, fullpath, &filesize);

					if(FileDelete(&sysinfo, fullpath))
						g_Partitions[partindex].FreeSectors += filesize/SECTOR_SIZE;
						
					img_err_del++;
					
				//	memset(&g_data.images[i],0,sizeof(g_data.images[i]));
				
		}
		
		if(g_data.images[i].FileNumber!=filecount )
		{
			img_err_count++;
			
    	mymemset(&g_data.images[i],0,sizeof(g_data.images[i]));

		}	
		
    FileSystemClose(&sysinfo);
	}
	
  num = g_data.ImageFilesNumber;
  
	//找到所有文件后，按时间从现在往前排序
	for(i=0;i<num;i++)
  {
  	if( g_data.images[i].FileTime == 0 )
  	{
 	   	for(j=i+1;j<num;j++)
 	   	{
			  if( g_data.images[j].FileTime != 0)
			  {
				//  memcpy(tmpimage, &g_data.images[i], OKR_HEADER_MIN_SIZE);
			   	CopyMem(&g_data.images[i], &g_data.images[j], OKR_HEADER_MIN_SIZE);
					
						
					mymemset(&g_data.images[j],0,sizeof(g_data.images[j]));
					break;

			  //	memcpy(&g_data.images[j], &tmpimage, OKR_HEADER_MIN_SIZE);
			  }
		  } 		
  	}

	}	

	
  g_data.ImageFilesNumber = g_data.ImageFilesNumber - img_err_del - img_err_count;

  MyLog(LOG_DEBUG, L"ImageFilesNumber:%d,%d,img_err_del %d\n",g_data.ImageFilesNumber,img_err_count,img_err_del);
	
//	getch();
	
	return (img_err_count-img_err_del);
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
BOOL IfNeedRecoverPartTable_delete(OKR_HEADER * head,int*klchange)
{
    DWORD i;

	if( g_data.bForceRestorePartition == TRUE )
		return TRUE;
	if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1) 
		{
 //  MyLog(LOG_DEBUG, L"yyyyaaaa\n");
		return TRUE;}
	
    for (i=0;i<head->PartitionCount;i++) {
        DWORD j;
        if (!head->Partition[i].bIfBackup)
            continue;


        for (j=0;j<g_dwTotalPartitions;j++) {
            if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin 
				&& g_Partitions[j].DiskNumber == g_data.RecoverDiskNumber)
                break;
        }
 //       MyLog(LOG_DEBUG, L"yyyyyyyyyyyyyaaaaget %d ,%d\n",j,g_dwTotalPartitions);

        if (j == g_dwTotalPartitions)
			{	
			*klchange=1;
//			MyLog(LOG_DEBUG, L"yyyyaaaa11: %d ,%d,%d\n",j,g_dwTotalPartitions,klchange);
            return TRUE;		
			}
      
    }
    
    return FALSE;
}






//////////////////////////////////////////////////////////////////////////////////////////////////

//检查是否需要恢复分区表
BOOL IfNeedRecoverPartTable(OKR_HEADER * head,int*klchange)
{
    DWORD i;

	if( g_data.bForceRestorePartition == TRUE )
		return TRUE;
	if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1) 
		{
  // MyLog(LOG_DEBUG, L"yyyy\n");
		return TRUE;}
	
    for (i=0;i<head->PartitionCount;i++) {
        DWORD j;
        if (!head->Partition[i].bIfBackup)
            continue;


        for (j=0;j<g_dwTotalPartitions;j++) {
            if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin 
				&& g_Partitions[j].DiskNumber == g_data.RecoverDiskNumber)
                break;
        }
      //  MyLog(LOG_DEBUG, L"yyyyyyyyyyyyyget %d ,%d\n",j,g_dwTotalPartitions);

        if (j == g_dwTotalPartitions)
			{	
			*klchange=1;
	//		MyLog(LOG_DEBUG, L"yyyy11: %d ,%d,%d\n",j,g_dwTotalPartitions,klchange);
            return TRUE;		
			}
        if (MyMemCmp(g_Partitions[j].UniGuid,head->Partition[i].UniGuid,16) != 0)
			{//MyLog(LOG_DEBUG, L"yyyy22: %d,%d,%x ,%x\n",i,j,g_Partitions[j].UniGuid,head->Partition[i].UniGuid);
            return TRUE;}
        if (g_Partitions[j].TotalSectors != head->Partition[i].TotalSectors)
			{//MyLog(LOG_DEBUG, L"yyyy33: %d,%d,%x ,%x\n",i,j,g_Partitions[j].TotalSectors,head->Partition[i].TotalSectors);
            return TRUE;}
		if (g_Partitions[j].PartitionTableBegin != head->Partition[i].PartitionTableBegin)
		{//	MyLog(LOG_DEBUG, L"yyyy44: %d ,%d,%x,%x\n",i,j,g_Partitions[j].PartitionTableBegin,head->Partition[i].PartitionTableBegin);
		return TRUE;}
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
						g_disklist[g_Partitions[j].DiskNumber].backupdisk = TRUE;
						destdisk = g_Partitions[j].DiskNumber;
						
						if(g_disklist[destdisk].removable)
						{
							destdisk = -1;
							g_disklist[g_Partitions[j].DiskNumber].backupdisk = FALSE;
						
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
	
		DrawMsgBox(STRING_PART_MODIFY_ERROR, STRING_YES, STRING_NO);
	//	DrawMsgBox(STRING_DISK_TOO_SMALL, STRING_YES, STRING_NO);
	
        return FALSE;
    }
	MyLog(LOG_DEBUG, L"Destination disk %d.\n", g_data.RecoverDiskNumber);
    
    return TRUE;
}



#define EXTRA_SIZE1 0x2800000  // 20G 扇区
int destpartindex = -1;
int oldisk_c_small = 0; // 0 equ  , 1 small , 2 big
BOOL GetDestinationDisk(OKR_HEADER * head,int *ifolddisk)
{
	DWORD i, j;
	ULONGLONG datasize = 0;
	int backupdisk = 0;
	int destdisk = -1;
	int candidate = -1;
	*ifolddisk =-1;
	int findc = 0;
	oldisk_c_small =0;
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

				MyLog(LOG_DEBUG, L"GetDestinationDisk Find same partition!\n");
				*ifolddisk = 1;
				findc = 1 ;
				//MyLog(LOG_DEBUG, L"%d(%d): backup %d, disk %d(%d), destdisk %d .\n", 
				//	i, j, head->Partition[i].bIfBackup, head->Partition[i].DiskNumber, g_Partitions[j].DiskNumber, destdisk);

				if (head->Partition[i].bIfBackup) {
					if (backupdisk == head->Partition[i].DiskNumber && destdisk == -1) {
						g_disklist[g_Partitions[j].DiskNumber].backupdisk = TRUE;
						destdisk = g_Partitions[j].DiskNumber;
						destpartindex = j;
						if(g_disklist[destdisk].removable)
						{
							destdisk = -1;
							g_disklist[g_Partitions[j].DiskNumber].backupdisk = FALSE;
						
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
			
			if(head->Partition[i].HasWindows == TRUE && findc == 0)
				{
				MyLog(LOG_DEBUG,L"findc = 0 i = %d j=%d \n",i,j);
				g_data.PartCBootBegin =  head->Partition[i].BootBegin;
				g_data.PartCTotalSector = head->Partition[i].TotalSectors;
				MyLog(LOG_DEBUG,L"big-total :0x%llx 0x%llx\n",g_data.PartCBootBegin,g_data.PartCTotalSector);
				}
				
			if (g_Partitions[j].BootBegin == head->Partition[i].BootBegin)
			{
				CHAR16 Buffr1[256] = {0};
				
				
				if(head->Partition[i].HasWindows == TRUE)
				{
				destpartindex = j;
				*ifolddisk = 1;
				destdisk = g_Partitions[j].DiskNumber;
				g_data.PartCBootBegin =  g_Partitions[j].BootBegin;
				g_data.PartCTotalSector = g_Partitions[j].TotalSectors;
				MyLog(LOG_DEBUG,L"findc = 1 i = %d j=%d \n",i,j);
				MyLog(LOG_DEBUG,L"big-total :0x%llx 0x%llx\n",g_data.PartCBootBegin,g_data.PartCTotalSector);
				findc = 1 ;
				
				if(g_Partitions[j].TotalSectors > head->Partition[i].TotalSectors)
				oldisk_c_small = 2;
			
				if(g_Partitions[j].TotalSectors < head->Partition[i].TotalSectors)
				oldisk_c_small = 1;
			
				MyLog(LOG_DEBUG,L"i = %d j=%d \n",i,j);
				MyLog(LOG_DEBUG,L"Olddisk C :0x%llx 0x%llx\n",g_data.PartCBootBegin,g_data.PartCTotalSector);
				MyLog(LOG_DEBUG,L"partition C --image C :0x%llx 0x%llx\n",g_Partitions[j].TotalSectors,head->Partition[i].TotalSectors);
				if( g_Partitions[j].TotalSectors <( head->Partition[i].TotalSectors - 0x8000) && g_data.CurImageSyspartitionsize!=0) 
				{	
				UINT64	 NewAlignNum1 = MultU64x32(g_data.CurImageSyspartitionsize, BLOCK_SIZE );
				MyLog(LOG_DEBUG,L"Olddisk C sys disk CurImageSyspin sec:0x%llx\n",NewAlignNum1);
         if((NewAlignNum1 + EXTRA_SIZE1)< head->Partition[i].TotalSectors && g_Partitions[j].TotalSectors <(NewAlignNum1 + EXTRA_SIZE1))
           {
					   memset(Buffr1, 0, 256);
						if( g_data.bForceRestorePartition == FALSE )
						{
							SPrint(Buffr1, 256, STRING_OLD_DISK_SYSDISK_TOO_SMALL, g_Partitions[j].TotalSectors/2/1024/1024,(NewAlignNum1 + EXTRA_SIZE1)/2/1024/1024);
							DrawMsgBox(Buffr1, STRING_YES, NULL);
							destdisk = -1;
							g_data.RecoverDiskNumber = destdisk;
							return FALSE;
						}
 
					}



         
          if( head->Partition[i].TotalSectors < (NewAlignNum1 + EXTRA_SIZE1))
					{
					   memset(Buffr1, 0, 256);
						if( g_data.bForceRestorePartition == FALSE )
						{
							SPrint(Buffr1, 256, STRING_OLD_DISK_SYSDISK_TOO_SMALL, g_Partitions[j].TotalSectors/2/1024/1024,(head->Partition[i].TotalSectors)/2/1024/1024);
							DrawMsgBox(Buffr1, STRING_YES, NULL);
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
	
		DrawMsgBox(STRING_DISK_NOT_FOUND_ERROR, STRING_YES, STRING_NO);
	//	DrawMsgBox(STRING_DISK_TOO_SMALL, STRING_YES, STRING_NO);
	
        return FALSE;
    }
	 else
	 	if(*ifolddisk == -1)
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
            if(UserBreak())
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

    ret = 0;
Done:

	//MyLog(LOG_DEBUG, L"Recover completed with status %d\n", ret);

  DestroyProcessBar();

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
		 DrawMsgBox(STRING_FS_INIT_FAILED_1, STRING_YES, STRING_NO);
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
		DrawMsgBox(STRING_CREATE_DIR_ERROR1, STRING_YES, STRING_NO);
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
		DrawMsgBox(STRING_CREATE_DIR_ERROR2, STRING_YES, STRING_NO);
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
		DrawMsgBox(STRING_CREATE_DIR_ERROR3, STRING_YES, STRING_NO);
		
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
		DrawMsgBox(STRING_CREATE_DIR_ERROR4, STRING_YES, STRING_NO);
		return ;
	}
	
	buffer = AllocatePool(512);	
	if(buffer == NULL)
		return ;
	
	//MyLog(LOG_DEBUG,L"FixPartCSize,%llx TotalSector=%llx\n",BootBegin,TotalSector);
	
	if (DiskRead((WORD)disknum, BootBegin,1, buffer) == FALSE)
		{
		DrawMsgBox(STRING_DISK_READ_ERROR_TIPS, STRING_YES, NULL);
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
		DrawMsgBox(STRING_DISK_WRITE_ERROR_TIPS, STRING_YES, NULL);
		
		}
		if(buffer)
			FreePool(buffer);
		last_upcase = 1;
		return ;
}
		
int fixSmallc = 0;

#define GPT_SIZE 16896
void FixNoDataPartTable(char disknum,ULONGLONG BootBegin, ULONGLONG TotalSector)
{
	BYTE *buffer = NULL ;	
	BYTE *ntfsbuffer = NULL ;
	UINT64 Random1 = 0;
	UINT64 BootDSecBegin = 0;
	UINT32 Crc = 0;
	int i =0,j=0;
	BYTE *head =0;
	
	MyLog(LOG_DEBUG,L"FixNewHDD :%d 0x%llx  0x%llx\n",disknum,BootBegin,TotalSector);
	if(disknum < 0 || BootBegin == 0 || TotalSector == 0)
	{
		DrawMsgBox(STRING_RECOVER_OLD_DISK_WARNING1, STRING_YES, STRING_NO);
		return ;
	}
	
	buffer = AllocatePool(GPT_SIZE);	
	ntfsbuffer = AllocatePool(512);	
	if(buffer == NULL)
		return ;
	
	if(IsGPT(disknum))
	{
		if (DiskRead((WORD)disknum, 1, GPT_SIZE/512, buffer) == FALSE)
		{
			DrawMsgBox(STRING_RECOVER_OLD_DISK_WARNING2, STRING_YES, STRING_NO);
			if(buffer)
			FreePool(buffer);
			return ;
		}
		
		if( buffer[0]!='E' ||  buffer[1]!='F' ||  buffer[2]!='I'||  buffer[3]!=' '||  buffer[4]!='P'||  buffer[5]!='A'||  buffer[6]!='R'||  buffer[7]!='T' )
		{
			MyLog(LOG_DEBUG,L"wrong efi header efi part\n");
			return ;
		}
		
		buffer[0x10] = 0;
		buffer[0x11] = 0;
		buffer[0x12] = 0;
		buffer[0x13] = 0;
		
		MyLog(LOG_DEBUG,L"HD GUID :%02x %02x %02x %02x\n",buffer[0x38],buffer[0x39],buffer[0x3a],buffer[0x3b]);
		//更新硬盘 GUID
		AsmRdRand64(&Random1);
		//*(UINT64*)(&buffer[0x38]) = Random1;
		for(j=0;j<8; j++)
		{
			//buffer[0x38+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG,L"HD GUID :%llx\n",Random1);
		AsmRdRand64(&Random1);
		//*(UINT64*)(&buffer[0x40]) = Random1;
		for(j=0;j<8; j++)
		{
			//buffer[0x40+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG,L"HD GUID :%llx\n",Random1);
		
		
		
		
		//更新分区表
		for(i= 512 ;i< GPT_SIZE ;)
		{
			head = &buffer[i];
			MyLog(LOG_DEBUG,L"HD BootBegin :%llx\n",*(UINT64*)(&head[0x20]) ); 
			if( *(UINT64*)(&head[0x20]) == BootBegin)
			break;
		
			 i = i+128 ;
		}
		
		if(i > 4096)
		{
			
			MyLog(LOG_DEBUG,L"fixnew hdd Not Find partc\n" ); 
			return ;
			
		}
		
		if( g_disklist[disknum].totalsector > (BootBegin + TotalSector + 0x64000)) //判断剩余空间，大于多少20G创建新分区
		{
			memcpy(&buffer[i+128],&buffer[i],128);
			
			i = i+128;
			//更新分区GUID
			AsmRdRand64(&Random1);
			//*(UINT64*)(&buffer[i+0x10]) = Random1;
			for(j=0;j<8; j++)
			{
				buffer[0x10+i+j] = (Random1 >> j*8)&0xff ;
			}
			MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);
			
			AsmRdRand64(&Random1);
			//*(UINT64*)(&buffer[i+0x18]) = Random1;
			for(j=0;j<8; j++)
			{
				buffer[0x18+i+j] = (Random1 >> j*8)&0xff ;
			}
			MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);
			//更新分区起始地址和结束
		
			*(UINT64*)(&buffer[i+0x20]) = BootDSecBegin =(BootBegin + TotalSector)&0xfffffffffffffff0;
			
			*(UINT64*)(&buffer[i+0x28]) = (BootDSecBegin+gwireTotalSectors)&0xffffffffffffff00;
	          CopyMem(&buffer[i], Window_Recovery_TypeGuid, 16);

			MyLog(LOG_DEBUG,L"1111111111111\n");
			//格式化分区第一个扇区。防止出现错误的文件系统
			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"read ntfsbuffer error\n");
   
		
			SetMem(ntfsbuffer,512,0);

		
			if (DiskWrite((WORD)disknum, BootDSecBegin,1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"write ntfsbuffer error\n");
		
		
			if(ntfsbuffer)
			FreePool(ntfsbuffer);
		
			fixSmallc = 0;
		}
		else
		{
			MyLog(LOG_DEBUG,L"222222222222\n");
			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区起始地址和结束
			 CopyMem(&buffer[i], Window_Recovery_TypeGuid, 16);
			*(UINT64*)(&buffer[i+0x28]) = (BootDSecBegin+gwireTotalSectors)&0xffffffffffffff00;
			 fixSmallc = 1 ;
		}
		
		i = i+128;
		
		for( ;i< GPT_SIZE ; i++)
		{
		buffer[i] = 0;
		}
		
		
		gBS->CalculateCrc32(buffer+512,16384,&Crc);
		*(UINT32*)(&buffer[0x58]) = Crc;
		
		
		gBS->CalculateCrc32(buffer,0x5c,&Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;
		
		
		
		if (DiskWrite((WORD)disknum, 1,GPT_SIZE/512,buffer) == FALSE)
		{
		DrawMsgBox(STRING_CREATE_DIR_ERROR5, STRING_YES, STRING_NO);
		if(buffer)
			FreePool(buffer);
		return ;
		}


		
	}
	else
	{
		if (DiskRead((WORD)disknum, 0, 1, buffer) == FALSE)
		{
			//DrawMsgBox(STRING_MOUNT_SYS_ERROR_3, STRING_YES, STRING_NO);
			MyLog(LOG_DEBUG,L"DiskRead hdd mbr read error\n" ); 
			if(buffer)
			FreePool(buffer);
			goto Done ;
		}
		if( buffer[0x1fe]!=0x55 ||  buffer[0x1ff]!=0xaa )
		{
			MyLog(LOG_DEBUG,L"wrong mbr header !\n");
			FreePool(buffer);
			goto Done ;
		}
		
		
		//**************asdf
		head = &buffer[0x1c6];
		//寻找c分区
		for(i= 0 ;i< 4 ; i++)
		{
			
			MyLog(LOG_DEBUG,L"HD BootBegin :%llx\n",*(UINT32*)(&head[i*16]) ); 
			if( *(UINT32*)(&head[i*16]) == (UINT32)BootBegin)
			{
				MyLog(LOG_DEBUG,L"mbr find part c",*(UINT32*)(&head[i*16]) ); 
				break;
			}
		
			 
		}
		
		if(i ==4)
		{
			
			MyLog(LOG_DEBUG,L"fixnew hdd mbr Not Find partc\n" ); 
			goto Done ;
			
		}
		
		if( g_disklist[disknum].totalsector > (BootBegin + TotalSector + 0x64000)  && i<3) //判断剩余空间，大于多少20G创建新分区
		{
			i++;
			 
			buffer[0x1be+16*i + 0] = 0;
			buffer[0x1be+16*i + 1] = 0xfe;
			buffer[0x1be+16*i + 2] = 0xff;
			buffer[0x1be+16*i + 3] = 0xff;
			buffer[0x1be+16*i + 4] = 7;
			buffer[0x1be+16*i + 5] = 0xfe;
			buffer[0x1be+16*i + 6] = 0xff;
			buffer[0x1be+16*i + 7] = 0xff;
			//buffer[0x1be+16*i + 8] = ;
			//buffer[0x1be+16*i + 9] = ;
			//buffer[0x1be+16*i + 10] = ;
			//buffer[0x1be+16*i + 11] = ;
			//buffer[0x1be+16*i + 12] = ;
			//buffer[0x1be+16*i + 13] = ;
			//buffer[0x1be+16*i + 14] = ;
			//buffer[0x1be+16*i + 15] = ;
		 
			MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);
			//更新分区起始地址和结束
			*(UINT32*)(&buffer[0x1be + i*16 + 8]) = BootDSecBegin = (UINT32)((BootBegin + TotalSector + 0xff)&0xffffff00);
			*(UINT32*)(&buffer[0x1be + i*16 + 12]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin - TotalSector - 0xff)&0xfffffff0);
			
			MyLog(LOG_DEBUG,L"i =%d 0x1be + i*16 = %d\n",i,0x1be + i*16);
			
			
			//格式化分区第一个扇区。防止出现错误的文件系统
			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"read ntfsbuffer error\n");
			SetMem(ntfsbuffer,512,0);
			if (DiskWrite((WORD)disknum, BootDSecBegin,1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"write ntfsbuffer error\n");
			if(ntfsbuffer)
			FreePool(ntfsbuffer);
		
		
			fixSmallc = 0;
		}
		else
		{
			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区起始地址和结束
		//	*(UINT32*)(&buffer[0x1ca]) = (g_disklist[disknum].totalsector )&0xffffffffffffff00;
			*(UINT32*)(&buffer[0x1ca + i*16]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin)&0xfffffff0);
			 fixSmallc = 1 ;
			 MyLog(LOG_DEBUG,L"i =%d 0x1ca + i*16 = %d\n",i,0x1ca + i*16);
			 
		}
		
		 
		
		if(i<3)
		{
			SetMem( (buffer+ 0x1be +i*16 + 16),16*(3-i),0); //后面分区清0
			MyLog(LOG_DEBUG,L"SetMem i =%d 0x1be + i*16 = %d\n",i,0x1be + i*16);
		}
		//**************asdf
		
			if (DiskWrite((WORD)disknum, 0,1, buffer) == FALSE)
		{
			MyLog(LOG_DEBUG,L"DiskWrite hdd mbr read error\n" ); 
			if(buffer)
			FreePool(buffer);
			goto Done ;
		}
		
		
		
	}
	Done :
	 
	if(buffer)
	   FreePool(buffer);
		
}

INT64 volume_size = 0;
INT64 get_volume_size(void)
{
	return volume_size;
}


void IsFixNewHddWay(OKR_HEADER *okrheader)
{
      int wirecount,i,t;
      ULONGLONG SizeMined = 0;
      	CHAR16 LenovoLabel[32] = {L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T'};
	  MyLog(LOG_DEBUG,L"FALSE wirecountZWANG okrheader->PartitionCount  %x \n",okrheader->PartitionCount);

     for (i=0; i<okrheader->PartitionCount; i++) {

	        if(MyMemCmp(Window_Recovery_TypeGuid,okrheader->Partition[i].TypeGuid,16) == 0 && MyMemCmp( okrheader->Partition[i].Label, LenovoLabel,22) != 0 ){
                 wirecount = i; //wire count is ESP to Wire total count
                 break;

	        }
 
     	}


     for (i=0; i<okrheader->PartitionCount; i++) {

	        if( okrheader->Partition[i].bIfBackup == TRUE&& okrheader->Partition[i].HasWindows == TRUE){
                t = i;
				MyLog(LOG_DEBUG,L"ENTER T here\n");
                 break;

	        }
 
     	}


	 SizeMined = okrheader->Partition[wirecount].BootBegin - okrheader->Partition[t].BootBegin -  okrheader->Partition[t].TotalSectors;


     
	 
	MyLog(LOG_DEBUG,L"okrheader->Partition[wirecount]   0x%llx\n",okrheader->Partition[wirecount] .TotalSectors);
	
	 
			      if(SizeMined < 0x1000){
				    IsNoDataPartFlag = TRUE;
					 MyLog(LOG_DEBUG,L"FALSE IsNoDataPartFlag i ZWANG  %x \n",i);

			      	}else{
                    IsNoDataPartFlag = FALSE;
			      }
	    
		
	 


}


void  GetWirePartSector(OKR_HEADER *okrheader)
	{
		  int wirecount,i;
	      	CHAR16 LenovoLabel[32] = {L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T'};
		 for (i=0; i<okrheader->PartitionCount; i++) {
	
				if(MyMemCmp(Window_Recovery_TypeGuid,okrheader->Partition[i].TypeGuid,16) == 0 && MyMemCmp( okrheader->Partition[i].Label, LenovoLabel,22) != 0 ){
					 wirecount = i; //wire count is ESP to Wire total count
					 MyLog(LOG_DEBUG,L"entergwireTotalSectors %x\n",wirecount);
					 break;
	
				}
	 
			}
		gwireTotalSectors = okrheader->Partition[wirecount].TotalSectors ;
	
	
	MyLog(LOG_DEBUG,L"gwireTotalSectors 0x%llx\n",gwireTotalSectors);

	}


void FixNewHDD(char disknum,ULONGLONG BootBegin, ULONGLONG TotalSector)
{
	BYTE *buffer = NULL ;	
	BYTE *ntfsbuffer = NULL ;
	UINT64 Random1 = 0;
	UINT64 BootDSecBegin = 0;
	UINT32 Crc = 0;
	int i =0,j=0;
	BYTE *head =0;
	
	MyLog(LOG_DEBUG,L"FixNewHDD :%d 0x%llx  0x%llx\n",disknum,BootBegin,TotalSector);
	if(disknum < 0 || BootBegin == 0 || TotalSector == 0)
	{
		DrawMsgBox(STRING_RECOVER_OLD_DISK_WARNING1, STRING_YES, STRING_NO);
		return ;
	}
	
	buffer = AllocatePool(GPT_SIZE);	
	ntfsbuffer = AllocatePool(512);	
	if(buffer == NULL)
		return ;
	
	if(IsGPT(disknum))
	{
		if (DiskRead((WORD)disknum, 1, GPT_SIZE/512, buffer) == FALSE)
		{
			DrawMsgBox(STRING_RECOVER_OLD_DISK_WARNING2, STRING_YES, STRING_NO);
			if(buffer)
			FreePool(buffer);
			return ;
		}
		
		if( buffer[0]!='E' ||  buffer[1]!='F' ||  buffer[2]!='I'||  buffer[3]!=' '||  buffer[4]!='P'||  buffer[5]!='A'||  buffer[6]!='R'||  buffer[7]!='T' )
		{
			MyLog(LOG_DEBUG,L"wrong efi header efi part\n");
			return ;
		}
		
		buffer[0x10] = 0;
		buffer[0x11] = 0;
		buffer[0x12] = 0;
		buffer[0x13] = 0;
		
		MyLog(LOG_DEBUG,L"HD GUID :%02x %02x %02x %02x\n",buffer[0x38],buffer[0x39],buffer[0x3a],buffer[0x3b]);
		//更新硬盘 GUID
		AsmRdRand64(&Random1);
		//*(UINT64*)(&buffer[0x38]) = Random1;
		for(j=0;j<8; j++)
		{
			//buffer[0x38+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG,L"HD GUID :%llx\n",Random1);
		AsmRdRand64(&Random1);
		//*(UINT64*)(&buffer[0x40]) = Random1;
		for(j=0;j<8; j++)
		{
			//buffer[0x40+j] = (Random1 >> j*8)&0xff ;
		}
		MyLog(LOG_DEBUG,L"   :%llx\n",Random1);
		
		
		
		
		//更新分区表
		for(i= 512 ;i< GPT_SIZE ;)
		{
			head = &buffer[i];
			MyLog(LOG_DEBUG,L"HD BootBegin :%llx\n",*(UINT64*)(&head[0x20]) ); 
			if( *(UINT64*)(&head[0x20]) == BootBegin)
			break;
		
			 i = i+128 ;
		}
		
		if(i > 4096)
		{
			
			MyLog(LOG_DEBUG,L"fixnew hdd Not Find partc\n" ); 
			return ;
			
		}
		
		if( g_disklist[disknum].totalsector > (BootBegin + TotalSector + 0x64000)) //判断剩余空间，大于多少20G创建新分区
		{
			memcpy(&buffer[i+128],&buffer[i],128);
			
			i = i+128;
			//更新分区GUID
			AsmRdRand64(&Random1);
			//*(UINT64*)(&buffer[i+0x10]) = Random1;
			for(j=0;j<8; j++)
			{
				buffer[0x10+i+j] = (Random1 >> j*8)&0xff ;
			}
			MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);
			
			AsmRdRand64(&Random1);
			//*(UINT64*)(&buffer[i+0x18]) = Random1;
			for(j=0;j<8; j++)
			{
				buffer[0x18+i+j] = (Random1 >> j*8)&0xff ;
			}
			MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);
			//更新分区起始地址和结束
			*(UINT64*)(&buffer[i+0x20]) = BootDSecBegin =(BootBegin + TotalSector)&0xfffffffffffffff0;
			*(UINT64*)(&buffer[i+0x28]) = (g_disklist[disknum].totalsector )&0xffffffffffffff00;
	

		
			//格式化分区第一个扇区。防止出现错误的文件系统
			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"read ntfsbuffer error\n");
			SetMem(ntfsbuffer,512,0);
			if (DiskWrite((WORD)disknum, BootDSecBegin,1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"write ntfsbuffer error\n");
			
			if(ntfsbuffer)
			FreePool(ntfsbuffer);
		
			fixSmallc = 0;
		}
		else
		{
			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区起始地址和结束
			*(UINT64*)(&buffer[i+0x28]) = (g_disklist[disknum].totalsector )&0xffffffffffffff00;
			 fixSmallc = 1 ;
		}
		
		i = i+128;
		
		for( ;i< GPT_SIZE ; i++)
		{
		buffer[i] = 0;
		}
		
		
		gBS->CalculateCrc32(buffer+512,16384,&Crc);
		*(UINT32*)(&buffer[0x58]) = Crc;
		
		
		gBS->CalculateCrc32(buffer,0x5c,&Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;
		
		
		
		if (DiskWrite((WORD)disknum, 1,GPT_SIZE/512, buffer) == FALSE)
		{
		DrawMsgBox(STRING_CREATE_DIR_ERROR5, STRING_YES, STRING_NO);
		if(buffer)
			FreePool(buffer);
		return ;
		}
		
	}
	else
	{
		if (DiskRead((WORD)disknum, 0, 1, buffer) == FALSE)
		{
			//DrawMsgBox(STRING_MOUNT_SYS_ERROR_3, STRING_YES, STRING_NO);
			MyLog(LOG_DEBUG,L"DiskRead hdd mbr read error\n" ); 
			if(buffer)
			FreePool(buffer);
			goto Done ;
		}
		if( buffer[0x1fe]!=0x55 ||  buffer[0x1ff]!=0xaa )
		{
			MyLog(LOG_DEBUG,L"wrong mbr header !\n");
			FreePool(buffer);
			goto Done ;
		}
		
		
		//**************asdf
		head = &buffer[0x1c6];
		//寻找c分区
		for(i= 0 ;i< 4 ; i++)
		{
			
			MyLog(LOG_DEBUG,L"HD BootBegin :%llx\n",*(UINT32*)(&head[i*16]) ); 
			if( *(UINT32*)(&head[i*16]) == (UINT32)BootBegin)
			{
				MyLog(LOG_DEBUG,L"mbr find part c",*(UINT32*)(&head[i*16]) ); 
				break;
			}
		
			 
		}
		
		if(i ==4)
		{
			
			MyLog(LOG_DEBUG,L"fixnew hdd mbr Not Find partc\n" ); 
			goto Done ;
			
		}
		
		if( g_disklist[disknum].totalsector > (BootBegin + TotalSector + 0x64000)  && i<3) //判断剩余空间，大于多少20G创建新分区
		{
			i++;
			 
			buffer[0x1be+16*i + 0] = 0;
			buffer[0x1be+16*i + 1] = 0xfe;
			buffer[0x1be+16*i + 2] = 0xff;
			buffer[0x1be+16*i + 3] = 0xff;
			buffer[0x1be+16*i + 4] = 7;
			buffer[0x1be+16*i + 5] = 0xfe;
			buffer[0x1be+16*i + 6] = 0xff;
			buffer[0x1be+16*i + 7] = 0xff;
			//buffer[0x1be+16*i + 8] = ;
			//buffer[0x1be+16*i + 9] = ;
			//buffer[0x1be+16*i + 10] = ;
			//buffer[0x1be+16*i + 11] = ;
			//buffer[0x1be+16*i + 12] = ;
			//buffer[0x1be+16*i + 13] = ;
			//buffer[0x1be+16*i + 14] = ;
			//buffer[0x1be+16*i + 15] = ;
		 
			MyLog(LOG_DEBUG,L"NEW PART GUID :%llx\n",Random1);
			//更新分区起始地址和结束
			*(UINT32*)(&buffer[0x1be + i*16 + 8]) = BootDSecBegin = (UINT32)((BootBegin + TotalSector + 0xff)&0xffffff00);
			*(UINT32*)(&buffer[0x1be + i*16 + 12]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin - TotalSector - 0xff)&0xfffffff0);
			
			MyLog(LOG_DEBUG,L"i =%d 0x1be + i*16 = %d\n",i,0x1be + i*16);
			
			
			//格式化分区第一个扇区。防止出现错误的文件系统
			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"read ntfsbuffer error\n");
			SetMem(ntfsbuffer,512,0);
			if (DiskWrite((WORD)disknum, BootDSecBegin,1, ntfsbuffer) == FALSE)
				MyLog(LOG_DEBUG,L"write ntfsbuffer error\n");
			if(ntfsbuffer)
			FreePool(ntfsbuffer);
		
		
			fixSmallc = 0;
		}
		else
		{
			//新磁盘空间小，c盘后无分区，更改c分区大小
			//更新C分区起始地址和结束
		//	*(UINT32*)(&buffer[0x1ca]) = (g_disklist[disknum].totalsector )&0xffffffffffffff00;
			*(UINT32*)(&buffer[0x1ca + i*16]) = (UINT32)((g_disklist[disknum].totalsector - BootBegin)&0xfffffff0);
			 fixSmallc = 1 ;
			 MyLog(LOG_DEBUG,L"i =%d 0x1ca + i*16 = %d\n",i,0x1ca + i*16);
			 
		}
		
		 
		
		if(i<3)
		{
			SetMem( (buffer+ 0x1be +i*16 + 16),16*(3-i),0); //后面分区清0
			MyLog(LOG_DEBUG,L"SetMem i =%d 0x1be + i*16 = %d\n",i,0x1be + i*16);
		}
		//**************asdf
		
			if (DiskWrite((WORD)disknum, 0,1, buffer) == FALSE)
		{
			MyLog(LOG_DEBUG,L"DiskWrite hdd mbr read error\n" ); 
			if(buffer)
			FreePool(buffer);
			goto Done ;
		}
		
		
		
	}
	Done :
	 
	if(buffer)
	   FreePool(buffer);
		
}

///////////////////////////////////////////////////////////////////////
void FixNewHDDPartitionTable(char disknum, OKR_HEADER *okrheader)
{
	BYTE *buffer = NULL ;	
	BYTE *ntfsbuffer = NULL ;
	UINT32 Crc = 0;
	UINT64 BootDSecBegin = 0;
	int i =0,j=0,partcCount = 0;
	BYTE *head =0;
	CHAR16 LenovoLabel[32] = {L'L',L'E',L'N',L'O',L'V',L'O',L'_',L'P',L'A',L'R',L'T'};//ysy

    gcount = 0xff;
	buffer = AllocatePool(GPT_SIZE);	
	if(buffer == NULL)
		return ;
      ntfsbuffer = AllocatePool(512);	
	if(ntfsbuffer == NULL)
		return ;

	if(IsGPT(disknum)){
		if (DiskRead((WORD)disknum, 1, GPT_SIZE/512, buffer) == FALSE){
			DrawMsgBox(STRING_RECOVER_OLD_DISK_WARNING2, STRING_YES, STRING_NO);
			if(buffer)
			FreePool(buffer);
			return ;
		}
		
		if( buffer[0]!='E' ||  buffer[1]!='F' ||  buffer[2]!='I'||  buffer[3]!=' '||  buffer[4]!='P'||  buffer[5]!='A'||  buffer[6]!='R'||  buffer[7]!='T' )
		{
			MyLog(LOG_DEBUG,L"wrong efi header efi part\n");
			return ;
		}
		
		buffer[0x10] = 0;
		buffer[0x11] = 0;
		buffer[0x12] = 0;
		buffer[0x13] = 0;

		for (i=0; i<okrheader->PartitionCount; i++) {
			 
			      if (MyMemCmp(Windows_Data_TypeGuid,okrheader->Partition[i].TypeGuid,16 ) == 0 && okrheader->Partition[i].HasWindows == TRUE)
		         {
			           partcCount = i;
		
					  MyLog(LOG_DEBUG,L"partcCount IS %X\n",partcCount); 
					break;
			      
			      }
			     
			}


    for (i=partcCount; i<okrheader->PartitionCount; i++) {
  		MyLog(LOG_DEBUG,L"Recover partition %d, BootBegin 0x%llx,  TotalSectors 0x%llx.\n",
  			i, okrheader->Partition[i].BootBegin, okrheader->Partition[i].TotalSectors);
		MyLog(LOG_DEBUG,L"Recover okrheader->Partition[i].bIfBackup %x.\n",okrheader->Partition[i].bIfBackup);
		MyLog(LOG_DEBUG,L"partcCount %x.\n",partcCount);


    if(i>partcCount){
	

MyLog(LOG_DEBUG,L"enter FixNewpartition HDD \n" ); 
        for(j= 512 ;j< GPT_SIZE ; j = j+128)
    		{
    			head = &buffer[j];
          if( *(UINT64*)(&head[0x20]))
    			  MyLog(LOG_DEBUG,L"New Hdd partition[%d] BootBegin :%llx\n",(j - 512)/128, *(UINT64*)(&head[0x20]) ); 
    			if( *(UINT64*)(&head[0x20]) == okrheader->Partition[i].BootBegin 
            ||  *(UINT64*)(&head[0x20]) == okrheader->Partition[i].BootBegin + 0x20)
    			  break;  		
    		}

        MyLog(LOG_DEBUG,L"j :%llx\n",j); 

        if(j< GPT_SIZE){
          if (okrheader->Partition[i].bIfBackup ){
		  		MyLog(LOG_DEBUG,L"wire is %x okrheader->Partition[i].TotalSectors %x \n",i,okrheader->Partition[i].TotalSectors); 
			   	  CopyMem(head, Window_Recovery_TypeGuid, 16);
            *(UINT64*)(&head[0x20]) = (g_disklist[disknum].totalsector - okrheader->Partition[i].TotalSectors - 0x20) & 0xfffffffffffffff0;
            *(UINT64*)(&head[0x28]) = (g_disklist[disknum].totalsector - 0x20) & 0xffffffffffffff00;

            okrheader->Partition[i].BootBegin = (g_disklist[disknum].totalsector - okrheader->Partition[i].TotalSectors - 0x20) & 0xfffffffffffffff0;

            //格式化分区第一个扇区。防止出现错误的文件系统
      			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
      				MyLog(LOG_DEBUG,L"read ntfsbuffer error\n");
      			SetMem(ntfsbuffer,512,0);
      			if (DiskWrite((WORD)disknum, BootDSecBegin,1, ntfsbuffer) == FALSE)
      				MyLog(LOG_DEBUG,L"write ntfsbuffer error\n");

            for( j = j+128; j< GPT_SIZE ; j++){
              buffer[j] = 0;
            }
            break;
          }else {
            *(UINT64*)(&head[0x20]) = 0;
            *(UINT64*)(&head[0x28]) = 0;
          }
          
        }else{
          if(okrheader->Partition[i].bIfBackup  ){
			  MyLog(LOG_DEBUG,L"okrheader->Partition[i].bIfBackup  %x \n",i); 
            for(j= 512 ;j< GPT_SIZE ; j = j+128){
        			head = &buffer[j];
        			MyLog(LOG_DEBUG,L"partition BootBegin :%llx  j %x \n",*(UINT64*)(&head[0x20],j) ); 

        			if( *(UINT64*)(&head[0x20]) == 0 || *(UINT64*)(&head[0x28]) == 0 ){
                break;
              }
            }

            if(j< GPT_SIZE){
		   	      CopyMem(head, Window_Recovery_TypeGuid, 16);
	MyLog(LOG_DEBUG,L"Window_Recovery_TypeGuid okrheader->Partition[i].TotalSectors %llx ",okrheader->Partition[i].TotalSectors); 
              *(UINT64*)(&head[0x20]) = BootDSecBegin = (g_disklist[disknum].totalsector - okrheader->Partition[i].TotalSectors -0x20) & 0xfffffffffffffff0;
              *(UINT64*)(&head[0x28]) = (g_disklist[disknum].totalsector - 0x20) & 0xffffffffffffff00;

              okrheader->Partition[i].BootBegin = (g_disklist[disknum].totalsector - okrheader->Partition[i].TotalSectors - 0x20)&0xfffffffffffffff0;

              //格式化分区第一个扇区。防止出现错误的文件系统
        			if (DiskRead((WORD)disknum, BootDSecBegin, 1, ntfsbuffer) == FALSE)
        				MyLog(LOG_DEBUG,L"read ntfsbuffer error\n");
        			SetMem(ntfsbuffer,512,0);
        			if (DiskWrite((WORD)disknum, BootDSecBegin,1, ntfsbuffer) == FALSE)
        				MyLog(LOG_DEBUG,L"write ntfsbuffer error\n");
        			
          			MyLog(LOG_DEBUG,L"j  %x ntfsbuffer \n",j);

              for( j = j+128; j< GPT_SIZE ; j++){
                buffer[j] = 0;
              }

              break;
      		  }
          }
        }
      }
    }

		gBS->CalculateCrc32(buffer+512,16384,&Crc);
		*(UINT32*)(&buffer[0x58]) = Crc;
		
		gBS->CalculateCrc32(buffer,0x5c,&Crc);
		*(UINT32*)(&buffer[0x10]) = Crc;
		
		
		if (DiskWrite((WORD)disknum, 1,GPT_SIZE/512, buffer) == FALSE)
		{
		  DrawMsgBox(STRING_CREATE_DIR_ERROR5, STRING_YES, STRING_NO);
		  if(buffer)
			  FreePool(buffer);
		  return ;
		}
		
	}
		 
	if(buffer)
	   FreePool(buffer);
	if(ntfsbuffer)
  	FreePool(ntfsbuffer);
		
}	

///////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////		
extern int sysdisktype_in_backupfile ;
//恢复的时候使用多线程，一个线程读文件，一个线程写硬盘
//partindex - 镜像文件所在的分区index
//fileindex - 镜像文件的头结构的索引号 g_data.images[fileindex]
extern DWORD	OldStructSize ;
extern int OldImage  ;
int Recover(WORD partindex, DWORD fileindex, PCONTROL pctl)
{
    OKR_HEADER *header = NULL;
    CHAR16 fullpath[128] = { 0 };
	CHAR16 *filename =NULL;
    UINT64 filesize = 0;
    int ret=-1;
	int olddisk = 0;
	int klchange=0;
    BOOL bRet = 0, bPartitionChanged = 0,bShowError = TRUE;
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
	BYTE ntfsbuff[512]={0};
   
	OldImage = 0;
 
	if(!CheckHandleValid( g_Partitions[partindex].Handle))
	{
		SET_ERROR(__LINE__, OKR_ERROR_FILE_SYSTEM);
		DrawMsgBox(STRING_MEDIA_ERROR, STRING_YES, NULL);
		return -1;
	}
	
    g_data.Cmd = USER_COMMAND_RECOVER;
	g_data.bCompleted = FALSE;
    g_data.ImageDiskNumber = g_Partitions[partindex].DiskNumber;
	g_data.dwErrorCode = 0;
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
		logopen(&sysinfo);
	}
 
  	MyLog(LOG_DEBUG,L"Recover index:%d,filenum %d\n",fileindex,g_data.images[fileindex].FileNumber);

    SPrint(fullpath, 128, BACKUP_DIR L"\\%s", filename);
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
	
	//g_data.selecteddisk = -1;
	//g_data.comefromnewhddface = 0;
	destpartindex = -1;		 
	if(g_data.comefromnewhddface == 0)	
	{
		 
		if(!GetDestinationDisk(header,&olddisk))
		{
		 
			bPartModify = TRUE;
			goto Done;
		}
	}
	else
	{

	
	   	
	 
		if(!CheckDestinationDisk(header))
		{
			 
			bPartModify = TRUE;
			goto Done;
		}
	}

   	if(sysdisktype_in_backupfile == 2)  //sata 备份到sata盘
	{
		if( g_disklist[g_data.RecoverDiskNumber].disksn[0] !=0  &&  g_disklist[g_data.RecoverDiskNumber].disksn[0] !=0x20) //nvme  恢复到nvme盘
		{
			// if(g_data.comefromnewhddface)//ysy 0110
			if(DrawMsgBox(STRING_NVME_TIPS,STRING_YES,STRING_NO) == IDCANCEL)//nvme 提示
			{
				bShowError = FALSE;
				MyLog(LOG_DEBUG,L"STRING_NVME_TIPS ,No go on \n");
				goto Done;
			}
		}
	}

	

//	MyLog(LOG_DEBUG,L"g_data.destdisktype = %d\n", g_data.destdisktype);
//	MyLog(LOG_DEBUG,L"hddface =%d\n", g_data.comefromnewhddface);	
	MyLog(LOG_DEBUG,L"olddisk final = %d..\n", olddisk);	
//	 MyLog(LOG_DEBUG,L"g_data.bForceRestorePartition = %r..\n", g_data.bForceRestorePartition);
    //检查是否需要恢复分区表
	if(g_data.bFactoryRestore) {
	
		//工厂恢复模式则强制恢复分区表
		bPartitionChanged = TRUE;
		g_data.bForceRestorePartition = TRUE;
	} else {
            
		bPartitionChanged = IfNeedRecoverPartTable_delete(header,&klchange);
             if(!g_data.bForceRestorePartition)
	   	{  
		
//	MyLog(LOG_DEBUG,L"aaaaaaa  %d\n",klchange);	
		
	       	if(klchange)
			{
//				MyLog(LOG_DEBUG,L"bbbbbbb\n");	
	                   DrawMsgBox(STRING_WELCOME, STRING_YES, NULL);
				//SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
				bShowError = FALSE;
				goto Done;

			}
			else
				{
                      bPartitionChanged = IfNeedRecoverPartTable(header,&klchange);

			}

	       }

	}
 MyLog(LOG_DEBUG, L"\n bPartitionChanged =%d !\n",bPartitionChanged); 
	if( g_data.comefromnewhddface == 0 && (olddisk == 2 || olddisk == 1) )	
	{
		
		MyLog(LOG_DEBUG,L"Done GUID   OldImage =%d\n",OldImage);
		//kang ++++>
		if(OldImage == 0)
		{
			MyLog(LOG_DEBUG,L"Done GUID   OldImage 2=%d\n",OldImage);
		  if( g_data.bForceRestorePartition == FALSE )
		  {
			for(i=0;i<g_dwTotalPartitions;i++) 
				{
					if(g_Partitions[i].HasWindows && g_Partitions[i].DiskNumber == g_data.RecoverDiskNumber)
							break;//hdd guid 或者分区头起始地址变化，导致c盘恢复后与分区表不一致（用户重新安装系统）,会死机，提示无法恢复
					
				}
				if(i < g_dwTotalPartitions)
				{
					
					MyLog(LOG_DEBUG,L"Done GUID   i =%d\n",i);
					MyLog(LOG_DEBUG, L"recovery disk find windows !\n"); 
					
					if (DiskRead((WORD)g_data.RecoverDiskNumber, g_Partitions[i].BootBegin, 1, ntfsbuff)==FALSE) {
					goto  Stepgoon;
					}
					
					if(header->WinPartNtfsGuid[0] == ntfsbuff[0x48] && \
					header->WinPartNtfsGuid[1] == ntfsbuff[0x49] && \
					header->WinPartNtfsGuid[2] == ntfsbuff[0x4a] && \
					header->WinPartNtfsGuid[3] == ntfsbuff[0x4b] && \
					header->WinPartNtfsGuid[4] == ntfsbuff[0x4c] && \
					header->WinPartNtfsGuid[5] == ntfsbuff[0x4d] && \
					header->WinPartNtfsGuid[6] == ntfsbuff[0x4e] && \
					header->WinPartNtfsGuid[7] == ntfsbuff[0x4f]  )
					
					{
					MyLog(LOG_DEBUG,L"Done GUID   i = equ \n" );
					}
					else{
					DrawMsgBox(STRING_SYS_UUID_ERROR,STRING_YES,NULL);
					bShowError = FALSE;
					ret = -1;
					
					MyLog(LOG_DEBUG,L"Done GUID PartC Serial Num changed ,No go on \n");
					goto Done;
						
					}
				}
		  }
		}
				Stepgoon:
		 	
		//kang ++++<
		if(bPartitionChanged )
		{
			if( g_data.bForceRestorePartition == FALSE )
			{
				
				if(OldImage == 0)
				{
				
				    if(CheckPartitionStartPosition(header) == FALSE || !CompareGuid(header->SourceDiskGUID,g_disklist[g_data.RecoverDiskNumber].guid)) //hdd guid 或者分区头起始地址变化，导致c盘恢复后与分区表不一致（用户重新安装系统）,会死机，提示无法恢复
				    {
				    	
				    	MyLog(LOG_DEBUG, L"header->SourceDiskGUID %x %x %x-%x %x %x\n",header->SourceDiskGUID[0],header->SourceDiskGUID[1],header->SourceDiskGUID[2],header->SourceDiskGUID[13],header->SourceDiskGUID[14],header->SourceDiskGUID[15]); 
				    	MyLog(LOG_DEBUG, L"DestDisk.guid= %x %x %x-%x %x %x\n",g_disklist[g_data.RecoverDiskNumber].guid[0],g_disklist[g_data.RecoverDiskNumber].guid[1],g_disklist[g_data.RecoverDiskNumber].guid[2],g_disklist[g_data.RecoverDiskNumber].guid[13],g_disklist[g_data.RecoverDiskNumber].guid[14],g_disklist[g_data.RecoverDiskNumber].guid[15]); 
				    
				    
				    	DrawMsgBox(STRING_UUID_ERROR,STRING_YES,NULL);
				    	bShowError = FALSE;
				    	ret = -1;
				    	
				    	MyLog(LOG_DEBUG,L"Done GUID PartC Start Address changed ,No go on \n");
				    	goto Done;
				    }
				}
				
		
				if(DrawMsgBox(STRING_C_PART_CHANGE_GOON,STRING_YES,STRING_NO) == IDCANCEL)
				{
					bShowError = FALSE;
					ret = -1;
					MyLog(LOG_DEBUG,L"Done PartC changed ,No go on \n");
					goto Done;
				}
			}
			
			if(DrawMsgBox(STRING_RECOVER_OLD_DISK_WARNING, STRING_YES, STRING_NO) == IDCANCEL) //免责提示
			{
				bShowError = FALSE;
				MyLog(LOG_DEBUG,L"Done PartC changed,agree? ,No go on \n");
				goto Done;
			}
			
		}
		
		 
	//		if(DrawMsgBox(STRING_RECOVER_OLD_DISK_WARNING,STRING_YES,STRING_NO) == IDCANCEL)//免责声明
		//	{
	//			MyLog(LOG_DEBUG,L"Done PartC changed warning,No go on \n");
	//			bShowError = FALSE;
	//			goto Done;
	//		}
		 
	}
	//MyLog(LOG_DEBUG,L"1\n");
	
	if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1) //kang 
	{
		bPartitionChanged = TRUE;
		g_data.bForceRestorePartition = TRUE;
	}
	//MyLog(LOG_DEBUG,L"2\n");
	if(g_data.destdisktype == DISK_SYSDISK  && g_data.comefromnewhddface == 1) //kang 
	{
		bPartitionChanged = FALSE;
		 
	}

    //if (bPartitionChanged == TRUE || g_data.bForceRestorePartition) {
	{
		//不管怎样，都要恢复Grub
		BOOL needrecover = FALSE;
		BYTE *databuffer = g_lz4_buffer;
		BYTE *mbr = g_lz4_buffer + 0x100000;	//+1MB, GPT_DATA_SIZE*SECTOR_SIZE;
		ULONGLONG firstsector = GetFirstPartitionBeginSector(g_data.RecoverDiskNumber);
              
		filesize = header->GptSize;
		
		if(OldImage == 1)
		{
			if (!FileSetPos(&filehandle, OldStructSize))
			goto  Done;
		}
		else
		{		
		if (!FileSetPos(&filehandle, header->StructSize))
			goto  Done;
		}
		 
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
			bPartitionChanged = RecheckPartitionTable(header, mbr, databuffer);
		}
		 
		 if(bPartitionChanged)
			MyLog(LOG_DEBUG,L"bPartitionChanged true\n");
		else
			MyLog(LOG_DEBUG,L"bPartitionChanged false \n");
		
		if(header->bGpt)
			MyLog(LOG_DEBUG,L"bGpt true\n");
		else
			MyLog(LOG_DEBUG,L"bGpt false\n");
				
 //gBS->Stall(0x1000000);
	
 
		if(bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 0 ) {
			
			
			if(g_data.bForceRestorePartition == FALSE)
			{
			//如果分区表发生变化，但是没有允许强制恢复分区表，则弹出提示，不允许恢复
			MyLog(LOG_DEBUG,L"Done bForceRestorePartition %d\n", g_data.bForceRestorePartition);
			DrawMsgBox(STRING_PARTITION_CHANGED, STRING_YES, NULL);
			SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
			bShowError = FALSE;
			goto Done;
			}
			else 
			{
				if(DrawMsgBox(STRING_PARTITION_CHANGED_FORCE_WARNING,STRING_YES,STRING_NO)== IDCANCEL)
				{
					SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
					bShowError = FALSE;
					goto Done;
				}
			}
		}
  
		//如果分区发生变化，则需要用户确认是否恢复；分区信息没变化的话，则只恢复grub信息
		//如果是工厂恢复模式强制恢复分区表; 
		if(bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1)
			needrecover = FALSE ; //(BOOL)DrawMsgBox(STRING_HINTPARTTABLE,STRING_YES,STRING_NO);
		else //if(g_data.bFactoryRestore)
			needrecover = TRUE;
			
		if(!IsGptSystem(g_data.RecoverDiskNumber) && header->bGpt)
			needrecover = TRUE;
		
		if(IsGptSystem(g_data.RecoverDiskNumber) && !(header->bGpt))
			needrecover = TRUE;
			
		//ctrl-f  强制恢复分区表，并不对其他分区更改
		
		if(g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
			needrecover = TRUE;
			
			
		if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1) //kang 
		{
			needrecover = TRUE;
		}
		
		if(g_data.destdisktype == DISK_SYSDISK  && g_data.comefromnewhddface == 1) //kang 
		{
			needrecover = FALSE;
			
		}

	// MyLog(LOG_DEBUG,L"4\n");
	 
	if(g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
		bPartitionChanged = TRUE;
	
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
			//	Print(L"number = %d \n",number);
				if(number)
				{
					//Print(L"3 \n");
					CopyMem(databuffer + 0x1be, mbr + 0x1be, 0x40);
					//需要计算一下第一个分区的起始扇区位置
					if(firstsector && number > firstsector)
						number = firstsector;
				}
			}
			if(number) {
				//Print(L"3 1 number =%d\n",number);
				if (DiskWrite((WORD)g_data.RecoverDiskNumber, sector, number, databuffer)==FALSE) {
					//Print(L"4 \n");
					SET_ERROR(__LINE__, OKR_ERROR_WRITE_GPT);
					goto  Done;
				}
			}
			//Print(L"5 \n");
		} else {
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
     int  index,disksrcnum,srcdisknume = 0;
	disksrcnum = g_data.ImageDiskNumber;
	srcdisknume = g_data.RecoverDiskNumber;
	

 
	if(g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
		goto ForceComefrom0_0;
	
		if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1) //kang 
		{
                                                                                                                                                                                                                             
		    MyLog(LOG_DEBUG,L"g_data.BackupDiskNumber.totalsector  %llx g_data.RecoverDiskNumbe %llx \n",g_disklist[disksrcnum].totalsector ,g_disklist[srcdisknume] .totalsector);
                   MyLog(LOG_DEBUG,L"g_data.RecoverDiskNumbe %llx \n",g_disklist[srcdisknume] .totalsector);
               
                     IsFixNewHddWay(header);
					 MyLog(LOG_DEBUG,L"IsNoDataPartFlag ZWANG  %x \n",IsNoDataPartFlag);

                  if( TRUE ==  IsNoDataPartFlag)
                     {
                     	   GetWirePartSector(header);
						   
                    	   FixNoDataPartTable(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);
						    MyLog(LOG_DEBUG,L"GetWirePartSector\n");
	
                    	}
						else{
                         
                         FixNewHDD(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);

          
			

		                 FixNewHDDPartitionTable(g_data.RecoverDiskNumber, header);
                                      MyLog(LOG_DEBUG,L"FixNewHDDPartitionTable\n");

                      }

   }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( g_data.comefromnewhddface == 0 && olddisk == 2)
			FixNewHDD(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);
			
MyLog(LOG_DEBUG,L"5\n");
		
ForceComefrom0_0:
		MyLog(LOG_DEBUG,L"Resotring DataOffset %d.\n",header->DataOffset);
    //确认每个恢复分区，都在正确的位置。
	if (!FileSetPos(&filehandle, header->DataOffset ? header->DataOffset : OKR_HEADER_MAX_SIZE + GPT_OLD_DATA_SIZE*512))
        goto  Done;
 
	
	
	
	
     MyLog(LOG_DEBUG,L"6\n");
 
	
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
        ULONGLONG totalblocks = 0,totaldatasize = 0,currentblock = 0,currentdata = 0,beginsec = 0,Lastblock_1 = 0;
        ULONGLONG origin_datasize = 0;
        ULONGLONG totalsec=0;

		MyLog(LOG_DEBUG,L"Resotring partition %d, backup %d, size 0x%x.\n",
			i, header->Partition[i].bIfBackup, header->Partition[i].BackupDataSize);

        if (!header->Partition[i].bIfBackup || !header->Partition[i].BackupDataSize)
            continue;

 
						
        MyLog(LOG_DEBUG,L"Read Partition(TotalSectors:%ld), partition %d\n", header->Partition[i].TotalSectors, i);
		//calc bitmap size
        totalblocks = DivU64x32((header->Partition[i].TotalSectors+63),64);
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
            
			if(UserBreak()) {
				g_data.dwErrorCode = OKR_ERROR_USER_BREAK;
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
			if(UserBreak()) {
				g_data.dwErrorCode = OKR_ERROR_USER_BREAK;
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
						
					 	if(g_data.needlimit == 1 && header->Partition[i].HasWindows) //kang +++
					 	{
					 		if(currentblock < (g_data.CurImageSyspartitionsize + 1)) //kang +++ 
					 		SmartDiskIo(&conti_info, MultU64x32(currentblock, BLOCK_SIZE) + beginsec, secnum, p);
					 	}
					 	else
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
     	if(g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
		goto ForceComefrom0_1;
	
 	 if(g_data.destdisktype == DISK_SYSDISK  && g_data.comefromnewhddface == 1) //kang 
	 	FixPartCSize(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);
	
	
		
	if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1) //kang 
	{
		if(fixSmallc == 1)
	 	FixSmallDiskCSize(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);
	}		
		
	 if(bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 2) //kang 
	{
		if(fixSmallc == 1)
	 	FixSmallDiskCSize(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);
	}	

	MyLog(LOG_DEBUG, L"check:b=%d,come=%d,olddisk=%d,oldisk_c_small=%d\n",bPartitionChanged, g_data.comefromnewhddface ,   olddisk,   oldisk_c_small  );
    BackupLastProcess();
	
	ForceComefrom0_1 :
	MyLog(LOG_DEBUG, L"Restore Done\n");
	
   // logflush(FALSE);
	 //FileClose(&filehandle);
	//FileClose(&g_data.hLog);
	//FileSystemClose(&sysinfo);
	
	if(g_data.comefromnewhddface == 0 && g_data.bForceRestorePartition == TRUE)
		goto ForceComefrom0_2;
	if(bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1 && oldisk_c_small == 2) //kang 
	{
		//kang -- MyLog(LOG_DEBUG, L"Fix hdd  bitmap =0x%llx 0x%llx\n",g_data.PartCBootBegin, g_data.PartCTotalSector);
		volume_size = (INT64)MultU64x32(g_data.PartCTotalSector, 512); 
	 	FixSmallDiskCSize_Bitmap(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);
		SetNTFSVolDirty(g_data.RecoverDiskNumber,g_data.PartCBootBegin);
		DrawMsgBox(STRING_DISK_CHECK_TIPS, STRING_YES, NULL);
		 
	}
	
		 if(bPartitionChanged && g_data.comefromnewhddface == 0 && olddisk == 1) //kang 
	  	FixPartCSize(g_data.RecoverDiskNumber,g_data.PartCBootBegin, g_data.PartCTotalSector);
	
	ForceComefrom0_2 :
	DiskFlush(g_data.RecoverDiskNumber);
    GoProcessBar(-1);

	//kang -- MyLog(LOG_DEBUG, L"Fix hdd  begin =0x%llx 0x%llx\n",g_data.PartCBootBegin, g_data.PartCTotalSector);

	
 
		
	//延时1秒，让用户可以看到100%
	delayEx(1000);

//////////////////////////////////////////////////////////////////		
  if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1 && g_data.dwErrorCode == OKR_ERROR_SUCCESS) //kang 
   {
  	 DrawMsgBox(STRING_RECOVER_NEW_DISK_SUC, STRING_YES, NULL);
   }
    ret = 0;
Done:
OldImage = 0;
 MyLog(LOG_DEBUG, L"g_data.destdisktype=%d.  \n ",g_data.destdisktype);
  MyLog(LOG_DEBUG, L"g_data.comefromnewhddface=%d\n",g_data.comefromnewhddface);
   MyLog(LOG_DEBUG, L"g_data.dwErrorCode=%d\n",g_data.dwErrorCode);
/*   if(g_data.destdisktype == NEW_FOUND_DISK  && g_data.comefromnewhddface == 1 && g_data.dwErrorCode == OKR_ERROR_SUCCESS) //kang 
   {
  	 DrawMsgBox(STRING_RECOVER_NEW_DISK_SUC, STRING_YES, NULL);
   }
*/  
	 
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
    
  //asdf  FileClose(&filehandle);

	if(ret < 0) {
		
		
	FileClose(&filehandle);

	logflush(FALSE);
	FileClose(&g_data.hLog);
    FileSystemClose(&sysinfo);
	
	
		;//kang -- MyLog(LOG_DEBUG, L"Restore failed with error %d, LineNumber %d\n", g_data.dwErrorCode, g_data.dwLineNumber);
	} else {
		//kang --MyLog(LOG_DEBUG, L"Restore successfully\n");
	}
//asdf	logflush(FALSE);
//asdf	FileClose(&g_data.hLog);
 //asdf   FileSystemClose(&sysinfo);

 
	
	 
		
 
	if(ret < 0 && bShowError) {
		WCHAR *szError = fullpath;
		SPrint(szError, 100, STRING_ERROR, g_data.dwErrorCode, g_data.dwLineNumber);
	//	DrawMsgBox(szError, STRING_YES, NULL);
	
	if(bPartModify == FALSE)
		  DrawMsgBox(STRING_ERROR_READ_FILE, STRING_YES, NULL);
		
		
	}

  DestroyProcessBar();

	if(g_data.bDebug) {
		//MyLog(LOG_DEBUG, L"Press any key to continue.\n");
		//getch();
	}


	g_data.Cmd = USER_COMMAND_NONE;
	
    return ret;
}



//恢复的时候使用多线程，一个线程读文件，一个线程写硬盘
//partindex - 镜像文件所在的分区index
//fileindex - 镜像文件的头结构的索引号 g_data.images[fileindex]
int Recover_Linux(WORD partindex, DWORD fileindex, PCONTROL pctl)
{
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
		DrawMsgBox(STRING_MEDIA_ERROR, STRING_YES, NULL);
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
		logopen(&sysinfo);
	}

  	MyLog(LOG_DEBUG,L"Recover index:%d,filenum %d\n",fileindex,g_data.images[fileindex].FileNumber);

    SPrint(fullpath, 128, BACKUP_DIR L"\\%s", filename);
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
			DrawMsgBox(STRING_PARTITION_CHANGED, STRING_YES, NULL);
			SET_ERROR(__LINE__, OKR_ERROR_UNMATCHED_PARTITION);
			bShowError = FALSE;
			goto Done;
		}

		//如果分区发生变化，则需要用户确认是否恢复；分区信息没变化的话，则只恢复grub信息
		//如果是工厂恢复模式强制恢复分区表; 
		if(bPartitionChanged)
			needrecover = (BOOL)DrawMsgBox(STRING_HINTPARTTABLE,STRING_YES,STRING_NO);
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
			if(CompareGuid(&PARTITION_LINUX_SWAP_GUID,header->Partition[i].TypeGuid))
			{
				MyLog(LOG_DEBUG, L"Found Linux Swap= %d\n", i);
				//DiskRead((WORD)g_data.BackupDiskNumber, g_Partitions[i].BootBegin, 4, pheader->TempBuffer);
				DiskWrite((WORD)g_data.RecoverDiskNumber, header->Partition[i].BootBegin , 12, header->TempBuffer);
			}
				
			 
						 
		 
		
		
        if (!header->Partition[i].bIfBackup || !header->Partition[i].BackupDataSize)
            continue;

        MyLog(LOG_DEBUG,L"Read Partition(TotalSectors:%ld), partition %d\n", header->Partition[i].TotalSectors, i);
		//calc bitmap size
        totalblocks = DivU64x32((header->Partition[i].TotalSectors+63),64);
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
            
			if(UserBreak()) {
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
			if(UserBreak()) {
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
	delayEx(1000);

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
	
	if(bPartModify == FALSE)
		  DrawMsgBox(STRING_ERROR_READ_FILE, STRING_YES, NULL);
		
		
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
