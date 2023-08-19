/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#include "okr.h"

#include "lnkbuf.h"
#include <Protocol/NvmExpressPassthru.h >
EFI_GUID gEfiDiskInfoNvmeInterfaceGuid = {0x3ab14680, 0x5d3f, 0x4a4d, {0xbc, 0xdc, 0xcc, 0x38, 0x0, 0x18, 0xc7, 0xf7 }};

BYTE *g_seg_buffer = NULL;
BYTE *g_read_seg_buffer = NULL;


//定义内存分配释放函数，以便将来进行优化，防止碎片等
PVOID myalloc(DWORD Length)
{
    if (Length < 0x1000)
        Length = 0x1000;
    return AllocatePool(Length);
}

UINT64 CalcAliginSize( UINT64 num )
{
	UINT64	 NewAlignNum = 0;

	NewAlignNum = DivU64x32 ((num + ALGIN_SIZE_BYTE - 1), ALGIN_SIZE_BYTE);
	NewAlignNum = MultU64x32 (NewAlignNum, ALGIN_SIZE_BYTE);
	
	return NewAlignNum;
}

CHAR16 *
Ascii2Unicode (
    OUT CHAR16         *UnicodeStr,
    IN  CHAR8          *AsciiStr
)
/*++

Routine Description:
  Converts ASCII characters to Unicode.

Arguments:
  UnicodeStr - the Unicode string to be written to. The buffer must be large enough.
  AsciiStr   - The ASCII string to be converted.

Returns:
  The address to the Unicode string - same as UnicodeStr.

--*/
{
    CHAR16  *Str = NULL;
	int i =0;

	if (NULL == UnicodeStr || NULL == AsciiStr)
	{
		return NULL;
	}

    Str = UnicodeStr;

    while (AsciiStr[i]) {

        UnicodeStr[i] = (CHAR16)AsciiStr[i];
		i++;
    }
	UnicodeStr[i] = 0;

	return UnicodeStr;
}

// read file to memory (allocated),call freefilemem() later
// return NULL ,if read failed;
// return pointer of file content in memory, and readsize
//void * LoadFileToMem(EFI_HANDLE Device,CHAR16 *FilePath,UINT64 attr, UINTN *readsize)
//{
//    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Vol;
//    EFI_FILE_PROTOCOL *RootFs;
//    EFI_STATUS status;
//    EFI_FILE_PROTOCOL *filehandle;
//    EFI_FILE_INFO *fileinfo;
//    UINT64 filesize;
//    VOID *pmem;
//    BYTE buffer[1024];
//
//    *readsize=0;
//    status=gBS->HandleProtocol(Device,&gEfiSimpleFileSystemProtocolGuid,&Vol);
//    if (EFI_ERROR(status)) {
//        MyLog(LOG_ERROR,L"open fs protocol error %x",status);
//
//        return NULL;
//    }
//
//    status=Vol->OpenVolume(Vol,&RootFs);
//    if (EFI_ERROR(status)) {
//        MyLog(LOG_ERROR,L"open volume error %x",status);
//        return NULL;
//    }
//
//    status=RootFs->Open(RootFs,&filehandle,FilePath,EFI_FILE_MODE_READ,attr);
//    if (EFI_ERROR(status)) {
//        MyLog(LOG_ERROR,L"Open %s file error %x\n",FilePath,status);
//        return NULL;
//    }
//    filesize=sizeof(buffer);
//    status=RootFs->GetInfo(filehandle,&gEfiFileInfoGuid,&filesize,buffer);
//    if (EFI_ERROR(status)) {
//        MyLog(LOG_ERROR,L"get file info error %x\n",status);
//        return NULL;
//    }
//    fileinfo=(EFI_FILE_INFO *)buffer;
//    filesize=fileinfo->FileSize;
//    pmem=AllocatePool(filesize);
////  MyLog(LOG_VERBOSE,L"load file %s ,to mem=%x ,bytes:%d ...",FilePath,pmem,filesize);
//
//    status=RootFs->Read(filehandle,&filesize ,pmem);
//    if (EFI_ERROR(status)) {
//        FreePool(pmem);
//        MyLog(LOG_ERROR,L"read file  error %x\n",status);
//
//        pmem=NULL;
//    }
////  MyLog(LOG_VERBOSE,L"get %d bytes \n",filesize);
//
//    status=RootFs->Close(filehandle);
//
//    *readsize=filesize;
//    return pmem;
//}

void FreeFileMem(void *buffer)
{
    FreePool(buffer);
}
//#if OKR_DEBUG
CONST CHAR8 Hex[] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'
};


/**
  Dump some hexadecimal data to the screen.

  @param[in] Indent     How many spaces to indent the output.
  @param[in] Offset     The offset of the printing.
  @param[in] DataSize   The size in bytes of UserData.
  @param[in] UserData   The data to print out.
**/
VOID
DumpHex (
    IN UINTN        Indent,
    IN UINTN        Offset,
    IN UINTN        DataSize,
    IN VOID         *UserData
)
{
    UINT8 *Data;

    CHAR8 Val[50] = { 0 };

    CHAR8 Str[20] = { 0 };

    UINT8 TempByte;
    UINTN Size;
    UINTN Index;

    Data = UserData;
    while (DataSize != 0) {
        Size = 16;
        if (Size > DataSize) {
            Size = DataSize;
        }

        for (Index = 0; Index < Size; Index += 1) {
            TempByte            = Data[Index];
            Val[Index * 3 + 0]  = Hex[TempByte >> 4];
            Val[Index * 3 + 1]  = Hex[TempByte & 0xF];
            Val[Index * 3 + 2]  = (CHAR8) ((Index == 7) ? '-' : ' ');
            Str[Index]          = (CHAR8) ((TempByte < ' ' || TempByte > 'z') ? '.' : TempByte);
        }

        Val[Index * 3]  = 0;
        Str[Index]      = 0;
        if (g_data.LogLevel == LOG_DEBUG)
        {
        	 //   Print( L"%*a%08X: %-48a *%a*\r\n", Indent, "", Offset, Val, Str);
             	 MyLog(LOG_DEBUG,L"%*a%08X: %-48a *%a*\r\n", Indent, "", Offset, Val, Str);

        }
        Data += Size;
        Offset += Size;
        DataSize -= Size;
    }
}
//#endif

CHAR16 getch()
{
    EFI_INPUT_KEY key;
    UINTN index;

    gBS->WaitForEvent(1,&gST->ConIn->WaitForKey,&index);
    gST->ConIn->ReadKeyStroke(gST->ConIn,&key);
    return key.UnicodeChar;
}

//模仿windows下获得当前的tickcount，以毫秒为单位
DWORD GetTickCount()
{
	static EFI_TIME	begintime = {0};
	EFI_STATUS	status;
	EFI_TIME	time;
	DWORD		seconds;
	ULONGLONG	microSeconds;

	status = gRT->GetTime(&time, NULL);
	if(begintime.Year == 0 && begintime.Month == 0)
	{
		CopyMem(&begintime, &time, sizeof(EFI_TIME));
		return 0;
	}

	if(time.Hour < begintime.Hour)
		seconds = time.Hour + 24 - begintime.Hour;
	else
		seconds = time.Hour - begintime.Hour;
	seconds *= 60;
	seconds = seconds + time.Minute - begintime.Minute;
	seconds *= 60;
	seconds = seconds + time.Second - begintime.Second;
	microSeconds = (ULONGLONG)seconds * 1000 * 1000;
	microSeconds = microSeconds + time.Nanosecond - begintime.Nanosecond;
	microSeconds = DivU64x32(microSeconds, 1000);
	return (DWORD)microSeconds;
}

//按传统的bios来计算，1秒=18.2个tickcount，这样一个tickcount就约等于55毫秒
DWORD delayEx(DWORD TickCounts)
{
	DWORD begin;
	EFI_INPUT_KEY key = {0};
	EFI_STATUS status;

	begin = GetTickCount();
	while((begin + TickCounts) > GetTickCount())
	{
		status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
		if (status == EFI_SUCCESS)
			break;
		gBS->Stall(200);	//MicroSecondDelay(200);
	}
	return key.UnicodeChar;
}

BOOL logopen(FILE_SYS_INFO *sysinfo, WORD partindex)
{
	CHAR16 name[128] = BACKUP_DIR L"\\okr.log";
	GetBackupFileFullPath(sysinfo, partindex, L"\\okr.log", 0, name, 128);
	//if (IsLenovoPart(partindex) == TRUE)
	//{
	//	SPrint(name, 128, BACKUP_DIR L"\\%s", L"\\okr.log");
	//}
	//else
	//{
	//	SPrint(name, 128, BACKUP_DIR_CUSTOMER L"\\%s", L"\\okr.log");
	//}
	return FileOpen(sysinfo, &g_data.hLog, name, EFI_FILE_MODE_CREATE|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ,0,FALSE);
}

void logflush(BOOL bOpenFile)
{
	FILE_SYS_INFO  sysinfo;
	BOOL bOpen = FALSE;

	if(!g_data.hLog.filehandle && bOpenFile) {
		//日志文件写到服务分区里
		if(g_data.oempartition != -1) {
			if (FileSystemInit(&sysinfo,g_data.oempartition,FALSE)) {
				if(logopen(&sysinfo, g_data.oempartition))
					bOpen = TRUE;
			}
		} else {
			//找一个有lenovo.okr目录的分区
			DWORD i;
			for (i=0;i<g_dwTotalPartitions;i++) {

				if ((g_Partitions[i].PartitionType!=FAT32)
					&&(g_Partitions[i].PartitionType!=FAT32_E)
					&&(g_Partitions[i].PartitionType!=NTFS))
					continue;
				if (FileSystemInit(&sysinfo,i,FALSE)) {
					if(logopen(&sysinfo, i)) {
						bOpen = TRUE;
						break;
					}
				}
			}
		}
	}

	if(g_data.hLog.filehandle) {
		DWORD len = ((g_data.loglength + 511) & ~511);
		FileSetPos(&g_data.hLog, 0);
		FileWrite(&g_data.hLog, len, g_data.logbuf);

		if(bOpen) {
			FileClose(&g_data.hLog);
			FileSystemClose(&sysinfo);
		}
	}
}

struct StringInfo;
VOID MyLogString(CHAR16 *String)
{
	StringInfo stStringInfo;

	if(!g_data.bDebug)
		return;

	if (g_vga) {

		if(g_data.bRefeshDebugInfo || g_data.LogY == LOG_GRAPHIC_Y - 20) {
			//第一次，需要画背景
			GetImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
			DrawSinkableRectangle(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, COLOR_SHADOW_GREY, COLOR_BLACK);
		}

		if (g_data.LogY >= (LOG_GRAPHIC_HEIGHT + LOG_GRAPHIC_Y - 20)) {
			PutImgEx(LOG_GRAPHIC_X+4, LOG_GRAPHIC_Y+2, LOG_GRAPHIC_WIDTH-6, LOG_GRAPHIC_HEIGHT - 20 - 4, 
				(BYTE*)g_data.dbgbuf + sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * (LOG_GRAPHIC_WIDTH - 6)* 20);
		} else {
			if(g_data.bRefeshDebugInfo && g_data.LogY != LOG_GRAPHIC_Y - 20) {
				PutImgEx(LOG_GRAPHIC_X+4, LOG_GRAPHIC_Y+2, LOG_GRAPHIC_WIDTH-6, LOG_GRAPHIC_HEIGHT - 20 - 4, g_data.dbgbuf);
			}
			if(String)
				g_data.LogY += 20;
		}
		if(String) {
			if(!g_data.bRefeshDebugInfo)
				FillRect(LOG_GRAPHIC_X+4, g_data.LogY, LOG_GRAPHIC_WIDTH - 6, 20, COLOR_BLACK);
			//DisplayStringInWidth(LOG_GRAPHIC_X+4, g_data.LogY, LOG_GRAPHIC_WIDTH - 6, COLOR_WHITE, String);

			stStringInfo.enumFontType = FZLTHJW;
			stStringInfo.iFontSize = 15; // CalcFontSize(15);

			SetTextBKColor(0x000000);
			DisplayStringInWidthEx(LOG_GRAPHIC_X + 4, g_data.LogY, COLOR_WHITE, String, &stStringInfo, LOG_GRAPHIC_WIDTH - 6, 0, TRUE);

			GetImgEx(LOG_GRAPHIC_X+4, LOG_GRAPHIC_Y+2, LOG_GRAPHIC_WIDTH-6, LOG_GRAPHIC_HEIGHT-4, g_data.dbgbuf);
		}

		g_data.bRefeshDebugInfo = FALSE;

	} else {
		if(String)
			Print(String);
	}
}

void PrintLogBuffer()
{
	char *log = g_data.logbuf;
	char c;
	int i, lines;

	g_data.logbuf[g_data.loglength] = 0;
	i = 0;
	for(lines=0;lines<1000;lines++) {
		while(log[i] != '\n' && log[i] != 0) i++;
		c = log[i];
		log[i] = 0;
		Print(L"%a\n", log);
		log[i] = c;
		if(c == 0)
			break;
		log += i + 1;
	}
	g_data.logbuf[g_data.loglength] = 'X';
}

#define _sntprintf UnicodeSPrint
#define TCHAR CHAR16
#define _T(x) L##x
BOOL IsPrintChar(TCHAR c)
{
	if (c >= 0x20 && c <= 0x7e)
		return TRUE;
	else
		return FALSE;
}

VOID OutputToLogBuffer(CHAR16 *Buffer)
{
	DWORD len;
	len = (DWORD)StrLength(Buffer);
	if(len + g_data.loglength + 2 < g_data.logbufsize) {
		Unicode2Ascii(g_data.logbuf + g_data.loglength, Buffer);
		g_data.loglength += len;
		if(Buffer[len-1] != '\n'){
			g_data.logbuf[g_data.loglength++] = '\r';
			g_data.logbuf[g_data.loglength++] = '\n';
		}
	}
}

VOID MyLog(WORD loglevel, IN  CONST CHAR16  *Format,  ...)
{
    CHAR16 buffer[1024] = { 0 };
    UINT32 len;
    VA_LIST Marker;

    VA_START (Marker, Format);
    len = UnicodeVSPrint (buffer, 1024, Format, Marker);
    VA_END (Marker);

	OutputToLogBuffer(buffer);

	if (loglevel > g_data.LogLevel)
		return;

	MyLogString(buffer);
}

VOID MyLogShowTime(WORD loglevel, IN  CONST CHAR16  *Format, ...)
{
	CHAR16 buffer[1060] = { 0 };
	CHAR16 bufferTmp[1024] = { 0 };
	UINT32 len;
	VA_LIST Marker;

	EFI_TIME  Time;
	gRT->GetTime(&Time, NULL);
	SPrint(buffer, 20, L"%02d:%02d:%02d   ", Time.Hour, Time.Minute, Time.Second);

	VA_START(Marker, Format);
	len = UnicodeVSPrint(bufferTmp, 1024, Format, Marker);
	VA_END(Marker);

	CopyMem(buffer + (DWORD)StrLength(buffer), bufferTmp, 1024 * sizeof(CHAR16));

	OutputToLogBuffer(buffer);

	if (loglevel > g_data.LogLevel)
		return;

	MyLogString(buffer);
}

//VOID MyLogA(WORD loglevel, IN CONST char *Format, ...)
void MyLogA(const char *Format, ...)
{
	CHAR16 wString[1024]={0};
	 
    CHAR8  buffer[1024]={0};
    UINTN   Return;
    VA_LIST Marker;

    //if (loglevel > g_data.LogLevel)
    //    return;
        
    VA_START (Marker, Format);
	Return = AsciiVSPrint (buffer, 1024, Format, Marker);
    VA_END (Marker);
	
	Ascii2Unicode(wString, buffer);
	
	OutputToLogBuffer(wString);
	
	MyLogString(wString);
}

VOID MyLogBinary(PVOID Buffer, DWORD Length)
{
	CHAR16 line[512] = { 0 };
	BYTE  *pb = (BYTE*)Buffer;
	CHAR16 *p;
	UINTN  i,j;

	UINTN nlines = (UINTN)(Length/16);
	for (i=0;i<nlines;i++)
	{
		p = line;
		p += _sntprintf(p, 512, _T("%04x    "), i*16);
		for (j=0;j<16;j++)
		{
			p += _sntprintf(p, 512, _T("%02X "), pb[i*16+j]);
		}
		*p++ = 0x20;
		for (j=0;j<16;j++)
		{
			p += _sntprintf(p, 512, _T("%c"), IsPrintChar(pb[i*16+j])?pb[i*16+j]:'.');
		}
		p += _sntprintf(p, 512, _T("\r\n"));
		OutputToLogBuffer(line);
	}

	if ((Length % 16) != 0)
	{
		p = line;
		p += _sntprintf(p, 512, _T("%04x    "), i*16);
		for (j=0;j<(Length % 16);j++)
		{
			p += _sntprintf(p, 512, _T("%02X "), pb[i*16+j]);
		}
		p += _sntprintf(p, 512, _T("\r\n"));
		OutputToLogBuffer(line);
	}
}

DISKINFO g_disklist[MAX_DISK_NUM];
WORD g_disknum=0;

int GetBindingDriver(EFI_HANDLE ctlhandler,EFI_HANDLE *drvlist, INTN *drvnum,EFI_HANDLE *parentlist, INTN *prtnum)
{
    INTN i=0;
    INTN get=0;
    INTN get1=0;
    INTN HandleCount = 0;
    EFI_HANDLE *HandleBuffer = NULL;
    EFI_GUID                            **ProtocolGuidArray;
    UINTN                               ArrayCount;
    EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
    UINTN protocolindex;
    UINTN                               OpenInfoCount;
    UINTN                               OpenInfoIndex;

    gBS->LocateHandleBuffer (
        AllHandles,
        NULL,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    for (i=0;i<HandleCount;i++) {
        if (gBS->ProtocolsPerHandle (
                    HandleBuffer[i],
                    &ProtocolGuidArray,
                    &ArrayCount
                )!=0)
            continue;
        for (protocolindex=0;protocolindex<ArrayCount;protocolindex++) {
            if (gBS->OpenProtocolInformation (
                        HandleBuffer[i],
                        ProtocolGuidArray[protocolindex],
                        &OpenInfo,
                        &OpenInfoCount
                    )!=0)
                continue;
            //find handler=ctlhandler 's openinfo
            for (OpenInfoIndex=0;OpenInfoIndex<OpenInfoCount;OpenInfoIndex++) {
                if ((OpenInfo[OpenInfoIndex].ControllerHandle==ctlhandler)) {
                    //  	(OpenInfo[OpenInfoIndex].Attributes&EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER)){
                    if (drvnum!=NULL)
                        if (get<*drvnum) {
                            drvlist[get]=OpenInfo[OpenInfoIndex].AgentHandle;
                            get++;
                            // 	Print(L"get drv index %x",OpenInfo[OpenInfoIndex].AgentHandle);
                        }
                    if (prtnum!=NULL)
                        if (get1<*prtnum) {
                            parentlist[get1]=HandleBuffer[i];
                            get1++;
                            //  	Print(L"get Parent index %x",HandleBuffer[i]);

                        }
                }

            }
            FreePool(OpenInfo);
        }
        FreePool(ProtocolGuidArray);
    }

    FreePool(HandleBuffer);
	if (drvnum != NULL)
	{
		*drvnum = get;
	}
	if (prtnum != NULL)
	{
		*prtnum = get1;
	}
    return 0;

}



EFI_STATUS
NvmeIdentifyController (
  IN EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL    *NvmePassThruProtocol,
  IN UINT32                                NamespaceId,
  IN NVME_ADMIN_CONTROLLER_DATA            *NvmeAdminControllerData
  )
{
  EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET CommandPacket;
  EFI_NVM_EXPRESS_COMMAND                  Command;
  EFI_NVM_EXPRESS_COMPLETION               Completion;
  EFI_STATUS                               Status;

  ZeroMem (&CommandPacket, sizeof(EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET));
  ZeroMem (&Command, sizeof(EFI_NVM_EXPRESS_COMMAND));
  ZeroMem (&Completion, sizeof(EFI_NVM_EXPRESS_COMPLETION));

  Command.Cdw0.Opcode = NVME_ADMIN_IDENTIFY_CMD;

  //
  // According to Nvm Express 1.1 spec Figure 38, When not used, the field shall be cleared to 0h.
  // For the Identify command, the Namespace Identifier is only used for the Namespace data structure.
  //
  Command.Nsid = 0;

  CommandPacket.NvmeCmd        = &Command;
  CommandPacket.NvmeCompletion = &Completion;
  CommandPacket.TransferBuffer = NvmeAdminControllerData;
  CommandPacket.TransferLength = sizeof (NVME_ADMIN_CONTROLLER_DATA);
  CommandPacket.CommandTimeout = EFI_TIMER_PERIOD_SECONDS (5);
  CommandPacket.QueueType      = NVME_ADMIN_QUEUE;
  //
  // Set bit 0 (Cns bit) to 1 to identify a controller
  //
  Command.Cdw10                = 1;
  Command.Flags                = CDW10_VALID;

  Status = NvmePassThruProtocol->PassThru (
                                   NvmePassThruProtocol,
                                   NamespaceId,
                                   &CommandPacket,
                                   NULL
                                   );

  return Status;
}
  
VOID
TruncateLastNode (
  IN OUT EFI_DEVICE_PATH_PROTOCOL   *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL    *LastNode = NULL;
  EFI_DEVICE_PATH_PROTOCOL    *DevPath;

  DevPath = DevicePath;

  while (!IsDevicePathEnd(DevPath)) {
    LastNode = DevPath;
    DevPath  = NextDevicePathNode (DevPath);
  }

  if (LastNode != NULL) {
    CopyMem( LastNode, DevPath, sizeof(EFI_DEVICE_PATH_PROTOCOL));
  }

  return;
}

EFI_DEVICE_PATH_PROTOCOL* GetDevicePathLastNode(EFI_DEVICE_PATH_PROTOCOL* DevPath)
{
	EFI_DEVICE_PATH_PROTOCOL    *LastNode = NULL;
	EFI_DEVICE_PATH_PROTOCOL	*pDevPath = DevPath;
	while (!IsDevicePathEnd(pDevPath))
	{
		LastNode = pDevPath;
		pDevPath = NextDevicePathNode(pDevPath);
	}

	return LastNode;
}

EFI_STATUS
GetParentHandle (
  EFI_HANDLE      ChildHandle,
  EFI_HANDLE      *ParentHandle
  )
{
  EFI_STATUS                    Status = EFI_NOT_FOUND;
  EFI_HANDLE                    Handle;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL      *TmpDevPath;
  EFI_DEVICE_PATH_PROTOCOL      *OrgDevPath;

  Status = gBS->HandleProtocol (ChildHandle, &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);

  if (EFI_ERROR(Status)) {
    return Status;
  }

  OrgDevPath = DuplicateDevicePath (DevicePath);
  TmpDevPath = OrgDevPath;

  if (TmpDevPath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }


  TruncateLastNode (TmpDevPath);
  gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &TmpDevPath, &Handle);

  if (IsDevicePathEnd(TmpDevPath)) {
    *ParentHandle = Handle;
    Status = EFI_SUCCESS;
  }


  gBS->FreePool (OrgDevPath);


  return Status;
}
 void get_nvme_device_info(EFI_HANDLE diskhandle,CHAR8* Buffer)
{
	 //MyLog(LOG_DEBUG, L"get_nvme_device_info....\r\n");

	EFI_STATUS Status;

    EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL *nvme_device;
    EFI_HANDLE PassthrHandle =NULL;
 
 
    NVME_ADMIN_CONTROLLER_DATA            *NvmeAdminControllerData;
	
  
	Status = GetParentHandle(diskhandle,&PassthrHandle);
        

           // Print(L"step1 cn: %r \n",Status);

    if(Status != EFI_SUCCESS)
      {
		 
		 return ;
      } 
	   
	  
	   Status=gBS->HandleProtocol (
                       PassthrHandle,
                       &gEfiNvmExpressPassThruProtocolGuid,
                       (void **)&nvme_device);

           // Print(L"step1 cn: %r \n",Status);

    if(Status != EFI_SUCCESS)
      {
		 
          
		 return ;
      } 
	 
	  
    if ((Status == EFI_SUCCESS) && (nvme_device != NULL)) 
	 {
		    		
		 
		NvmeAdminControllerData = (NVME_ADMIN_CONTROLLER_DATA *)AllocateZeroPool (sizeof(NVME_ADMIN_CONTROLLER_DATA));
		Status = NvmeIdentifyController ( nvme_device, 0,NvmeAdminControllerData);	 
		
		if (Status == EFI_SUCCESS)
			CopyMem(Buffer,NvmeAdminControllerData->Sn,20);

		MyLog(LOG_DEBUG, L"SN    :%a\r\n", NvmeAdminControllerData->Sn);
		MyLog(LOG_DEBUG, L"Mn    :%a\r\n", NvmeAdminControllerData->Mn);
		//MyLog(LOG_DEBUG, L"Fr    :%a\r\n", NvmeAdminControllerData->Fr);
		//MyLog(LOG_DEBUG, L"Cntlid    :%a\r\n", NvmeAdminControllerData->Cntlid);
		//MyLog(LOG_DEBUG, L"VendorData    :%a\r\n", NvmeAdminControllerData->VendorData);

		FreePool(NvmeAdminControllerData);
	 }
 
	 
}
void modifysatasn(CHAR8* Snbuffer)
{
	UINT16 lenght = 20;
	UINT16 i = 0 , j = 0;
	CHAR8 tempbuffer[20];
	CHAR8  value = 0;
	
	
	memset(tempbuffer,0,lenght);
	CopyMem(tempbuffer,Snbuffer,20);//memcpy(tempbuffer,Snbuffer,lenght);
	
	for(j=0; j < lenght ; j = j + 2)
	{
		value   = tempbuffer[j];
		tempbuffer[j] = tempbuffer[j+1];
		tempbuffer[j+1] = value;
	}
	
	
	for(i = 0;i < lenght ; )
	{
		if(tempbuffer[i] == 0x20)
			i++;
		else
			break;
	}
	
	for(j=0; i < lenght ; j++,i++)
	{
		Snbuffer [j] = tempbuffer[i];
	}
	j++;
	for(; j < lenght ; j++ )
	{
		Snbuffer [j] = 0;
	}
	
	//memcpy(Snbuffer,tempbuffer,lenght);
}

BYTE IsRAIDDisk(int iDiskNum)
{
	BYTE bIsRAIDDisk = 0;
	CHAR8 *Result = NULL;	

	if (iDiskNum <= g_disknum)
	{
		if (1 != checkIfSnZero(g_disklist[iDiskNum].disksn))
		{
			CHAR8   disksn[20] = { 0 };
			for (int i = 0; i < 20; i = i + 2)
			{
				disksn[i] = g_disklist[iDiskNum].disksn[i + 1];
				disksn[i + 1] = g_disklist[iDiskNum].disksn[i];
			}

			//MyLog(LOG_DEBUG, L"disksn: %a\r\n", disksn);
			Upper(disksn);
			//MyLog(LOG_DEBUG, L"after upper disksn: %a\r\n", disksn);

			Result = AsciiStrStr(disksn, "RAID");
			if (NULL != Result)
			{
				bIsRAIDDisk = 1;
				//MyLog(LOG_DEBUG, L"AsciiStrStr ret: %a\r\n", Result);
			}
		}
	}

	MyLog(LOG_DEBUG, L"IsRAIDDisk ret: %d\r\n", bIsRAIDDisk);

	return bIsRAIDDisk;
}

void GetDiskSn(EFI_HANDLE diskhandle,CHAR8* Buffer)
{
	MyLog(LOG_DEBUG, L"GetDiskSn....\r\n");
	
	EFI_DISK_INFO_PROTOCOL *DiskInfo;
	EFI_STATUS Status;
	UINT32 DataSize = 512;
	UINT8 *databuffer = NULL;

	if (Buffer == NULL || diskhandle == NULL)
		return;
	Status = gBS->HandleProtocol(diskhandle, &gEfiDiskInfoProtocolGuid, (VOID **)&DiskInfo);

	if (Status == EFI_SUCCESS)
	{

		gBS->AllocatePool(EfiBootServicesData, DataSize, (VOID **)&databuffer);
		Status = DiskInfo->Identify(DiskInfo, databuffer, &DataSize);
		if (Status == EFI_SUCCESS)
		{
			CopyMem(Buffer, ((IDENTIFY_DATA*)databuffer)->Serial_Number_10, 20);
			// modifysatasn(Buffer);
		}
		else
		if (Status == EFI_BUFFER_TOO_SMALL)
		{
			if (DataSize == 4096) //nvme
				get_nvme_device_info(diskhandle, Buffer);
		}

#if OKR_DEBUG
		ATA_IDENTIFY_DATA *Stru = NULL;
		Stru = (ATA_IDENTIFY_DATA *)(databuffer);
		MyLog(LOG_DEBUG, L"SeriaNo    :%a\r\n", Stru->SerialNo);
		MyLog(LOG_DEBUG, L"FirmwareVer:%a\r\n", Stru->FirmwareVer);
		MyLog(LOG_DEBUG, L"ModleName  :%a\r\n", Stru->ModelName);
#endif

		if (databuffer)
			FreePool(databuffer);

	}
	else
		MyLog(LOG_DEBUG, L"diskhandle not support diskinfo\n");
	return;
}

//get device name or return NULL
CHAR16 * GetDiskName(EFI_HANDLE diskhandle)
{
    EFI_STATUS Status;
    INTN NumHandles = 0;
    EFI_HANDLE *HandleBuffer = NULL;
    EFI_COMPONENT_NAME_PROTOCOL *componentname;
    EFI_COMPONENT_NAME2_PROTOCOL *componentname2;
    CHAR16 *buffer=NULL;
    //  BYTE Buffer[512];
    INTN i,j,parentnum,drivernum;
    /// get parent device driver
    EFI_HANDLE *parentlist;
    EFI_HANDLE *driverlist;
    CHAR8  *SupportedLanguages = NULL;
    
    parentlist=AllocatePool(sizeof(EFI_HANDLE)*512);
    parentnum=512;
    driverlist=AllocatePool(sizeof(EFI_HANDLE)*512);
    drivernum=512;
    
 
     // step 1 ,find parents handler of diskhandler, and call these binding drivers componentname2
    parentnum=512;
    GetBindingDriver(diskhandle,NULL,NULL,parentlist,&parentnum);
    for (i=0;i<parentnum;i++) {
        drivernum=512;
        GetBindingDriver(parentlist[i],driverlist,&drivernum,NULL,NULL);
        for (j=0;j<drivernum;j++) {
            Status=gBS->HandleProtocol (
                       driverlist[j],
                       &gEfiComponentName2ProtocolGuid,
                       (void **)&componentname2);

           // Print(L"step1 cn: %r \n",Status);

            if(Status != EFI_SUCCESS)
            {
            	continue;
            } 
            
            //if (Status)
           //     continue;
            buffer=NULL;
            SupportedLanguages = componentname2->SupportedLanguages;
            
            //	Status=componentname->GetDriverName(componentname,"eng",&buffer);
            Status=componentname2->GetControllerName(componentname2,parentlist[i],diskhandle,SupportedLanguages/*"en-US"*/,&buffer);
            //if (Status)
            //    continue;
            //Print(L"step1 get: %r \n",Status);

            if(Status != EFI_SUCCESS)
            {
            	continue;
            } 
            
            if (buffer==NULL)
                continue;

           // Print(L"step1 get name %s. \n",buffer);
          //  getch();

#if OKR_DEBUG
            MyLog(LOG_DEBUG,L"\r\nstep1 get name %s. \n",buffer);
#endif
            goto Done;
        }
    }
       
    //STEP 1 FIND Binding driver of diskhandler and call componentname
    GetBindingDriver(diskhandle,driverlist,&drivernum,NULL,NULL);
    for (i=0;i<drivernum;i++) {
        Status=gBS->HandleProtocol (
                   driverlist[i],
                   &gEfiComponentNameProtocolGuid,
                   (void **)&componentname);
     //  Print(L"step2 cn: %r \n",Status);
       if(Status != EFI_SUCCESS)
       {
         	continue;
       } 
       // if (Status)
       //     continue;
        buffer=NULL;
        //	Status=componentname->GetDriverName(componentname,"eng",&buffer);

        SupportedLanguages = componentname->SupportedLanguages;

        Status=componentname->GetControllerName(componentname,diskhandle,NULL,SupportedLanguages/*"eng"*/,&buffer);
       // Print(L"step2 get: %r \n",Status);
        if(Status != EFI_SUCCESS)
        {
          	continue;
        } 
       // if (Status)
       //     continue;
        if (buffer==NULL)
            continue;
      //  Print(L"step2 get name: %s. \n",buffer);

       

#if OKR_DEBUG
        MyLog(LOG_DEBUG,L"step2 get name: %s. \n",buffer);
#endif
        goto Done;
    }

Done:
    FreePool(driverlist);
    FreePool(parentlist);
    return buffer;
}

EFI_STATUS GetDiskType(EFI_HANDLE diskhandle, UINT8 *uiDevicePathProtocolType, UINT8 *uiDevicePathProtocolSubType)
{
	EFI_STATUS				Status;

	*uiDevicePathProtocolType = 0;
	*uiDevicePathProtocolSubType = 0;

	EFI_DEVICE_PATH_PROTOCOL *DevicePath;
	EFI_DEVICE_PATH_PROTOCOL *TmpDevPath;
	EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* Device2TextProtocol = 0;

	Status = gBS->LocateProtocol(
		&gEfiDevicePathToTextProtocolGuid,
		NULL,
		(VOID**)&Device2TextProtocol
	);

	gBS->HandleProtocol(
		diskhandle,
		&gEfiDevicePathProtocolGuid,
		&DevicePath
	);

	if (Status != 0) {
		MyLog(LOG_ALERT, L"Can't get device path\n");
		return EFI_SUCCESS;
	}

	CHAR16*  TextDevicePath = 0;
	TextDevicePath = Device2TextProtocol->ConvertDevicePathToText(DevicePath, FALSE, TRUE);
	MyLog(LOG_DEBUG, L"DevicePath: %s\n", TextDevicePath);
	if (TextDevicePath)
	{
		gBS->FreePool(TextDevicePath);
	}

	TmpDevPath = DuplicateDevicePath(DevicePath);
	if (TmpDevPath == NULL) 
	{
		return EFI_OUT_OF_RESOURCES;
	}

	TmpDevPath = GetDevicePathLastNode(TmpDevPath);

	*uiDevicePathProtocolType = TmpDevPath->Type;
	*uiDevicePathProtocolSubType = TmpDevPath->SubType;
	MyLog(LOG_DEBUG, L"(%d %d)", *uiDevicePathProtocolType, *uiDevicePathProtocolSubType);

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CheckWhetherSsdDisk(
	IN	EFI_DISK_INFO_PROTOCOL		*DiskInfo,
	OUT	SSD_DISK_TYPE	            *Type
)
{
	EFI_STATUS				Status;
	EFI_IDENTIFY_DATA		IdentifyData;
// 	EFI_DISK_INFO_IDENTIFY IdentifyData;
	UINT32					IdentifyDataSize = sizeof(IdentifyData);


	if(DiskInfo == NULL || Type == NULL)
	{
		MyLog(LOG_DEBUG,L"NULL PARAMETER.\n");
		return EFI_INVALID_PARAMETER;
	}

	//This place can use DiskInfo->Interface to idenfity current device is ATA or NVMe controller.
	if(CompareGuid(&DiskInfo->Interface,&gEfiDiskInfoAhciInterfaceGuid))
	{
		Status = DiskInfo->Identify(DiskInfo,&IdentifyData,&IdentifyDataSize);
		if(EFI_ERROR(Status))
		{
			MyLog(LOG_DEBUG,L"DisInfo->Identify(satassd) error.Error-code:%r.\n",Status);
			return Status;
		}
	 // if(IdentifyData.AtaData.nv_cache_read_speed == 1)
		if(IdentifyData.AtaData.nominal_media_rotation_rate == 1)
		{
			*Type = SataSSd;
		}
		else
		{
			*Type = NoSSd;
		}
		return EFI_SUCCESS;
	}
		// for nvme Disk controller IdentifyData need to be sure which is the best one to identify
	if(CompareGuid(&DiskInfo->Interface,&gEfiDiskInfoNvmeInterfaceGuid))
	{
		*Type = NvmeSSd;	
		return EFI_SUCCESS;
	}
	*Type = NoType;
	return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
GetDiskInfoByBlockIo( EFI_BLOCK_IO *BlockIo,EFI_DISK_INFO_PROTOCOL	**DiskInfo )
{
	EFI_STATUS			Status;
	
	UINTN				HandleNumbers = 0;
	EFI_HANDLE			*HandleBuffer = NULL;
	UINTN				HandleIndex		= 0;
	EFI_DISK_INFO_PROTOCOL		*DiskInfoQuire;	
	EFI_BLOCK_IO				*BlockIoQuire = NULL;

	if(BlockIo == NULL)
	{
		MyLog(LOG_DEBUG,L"BlockIo is NULL.\n");
		return EFI_INVALID_PARAMETER;
	}
	
	if(DiskInfo == NULL)
	{
		MyLog(LOG_DEBUG,L"BlockIo is NULL.\n");
		return EFI_INVALID_PARAMETER;
	}

	if(BlockIo->Media->RemovableMedia != FALSE || BlockIo->Media->LogicalPartition != FALSE)
	{
		MyLog(LOG_DEBUG,L"BlockIo should not removableMedia and not logicalparttition.\n");
		return EFI_INVALID_PARAMETER;
	}
	
	//Get the Handles of the specifies Protocol
	Status	= gBS->LocateHandleBuffer(ByProtocol,&gEfiBlockIoProtocolGuid,NULL,&HandleNumbers,&HandleBuffer);
	if(EFI_ERROR(Status))
	{
		MyLog(LOG_DEBUG,L"Can not find the handle of protocol EFI_BLOCK_IO_PROTOCOL.\n");
		return Status;
	}


	//Find the handle match the protocol.
	for(HandleIndex = 0;HandleIndex < HandleNumbers;HandleIndex++)
	{
		Status = gBS->HandleProtocol(HandleBuffer[HandleIndex],&gEfiBlockIoProtocolGuid,&BlockIoQuire);
		if(EFI_ERROR(Status))
			continue;
		if(BlockIo == BlockIoQuire && BlockIoQuire->Media->RemovableMedia == FALSE && BlockIoQuire->Media->LogicalPartition == FALSE)
		{
			Status = gBS->HandleProtocol(HandleBuffer[HandleIndex],&gEfiDiskInfoProtocolGuid,&DiskInfoQuire);
			if(EFI_ERROR(Status))
			{
				MyLog(LOG_DEBUG,L"The No%d BlockIo not found diskinfo protocol.\n",HandleIndex);
				continue;
			}

			*DiskInfo = DiskInfoQuire;
			break;
		}
	}
	
	if(HandleIndex >= HandleNumbers)
	{
		MyLog(LOG_DEBUG,L"Can't find blockio instance.\n");
		if(HandleBuffer != NULL)
			FreePool(HandleBuffer);
		return EFI_NOT_FOUND;
	}

	if(HandleBuffer != NULL)
		FreePool(HandleBuffer);

	Status = EFI_SUCCESS;
	return Status;
}


//todo freePool
//MAX_DISK_NUM 16
INTN GetDiskInfo()
{
    EFI_STATUS Status;
    INTN NumHandles = 0;
    EFI_HANDLE *HandleBuffer = NULL;
    BYTE Buffer[512] = { 0 };
    INTN i = 0;
    CHAR16 *name = NULL;
	CHAR8 SnBuffer[20] = { 0 }; //disk sn 
	//EFI_DISK_INFO_PROTOCOL *pDiskInfo;   //Not use
  	SSD_DISK_TYPE	DiskType = NoType;
	CHAR8  totalSector[16] = { 0 };

	//Get all gEfiBlockIoProtocolGuid device
    Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiBlockIoProtocolGuid,
                 NULL,
                 &NumHandles,   //device number
                 &HandleBuffer  //device handle list
             );

    if (Status != EFI_SUCCESS) 
	{
        MyLog( LOG_ALERT,L"Not found block device\n");
        return -1;
    }

    g_disknum = 0;
    for ( i = 0; i < NumHandles; i++) 
	{
        EFI_BLOCK_IO_PROTOCOL *BlkIo;
        //EFI_BLOCK_IO2_PROTOCOL *BlkIo2;
        //EFI_DEVICE_PATH_PROTOCOL *DevicePath;
        //EFI_COMPONENT_NAME_PROTOCOL *cn;
        EFI_BLOCK_IO_MEDIA *Media;    //read only, device attr info
        //EFI_SIMPLE_TEXT_INPUT_PROTOCOL * Input;

		// open device
        Status=gBS->HandleProtocol (
                   HandleBuffer[i],
                   &gEfiBlockIoProtocolGuid,
                   (void **)&BlkIo);

		if (Status != 0)
		{
			continue;
		}

        //gBS->HandleProtocol (
        //    HandleBuffer[i],
        //    &gEfiDevicePathProtocolGuid,
        //    &DevicePath
        //);
        //if (Status!=0) {
        //    MyLog(LOG_ALERT,L"Can't get device path\n");
        //    continue;
        //}

        //Status=gBS->HandleProtocol (
        //           HandleBuffer[i],
        //           &gEfiBlockIo2ProtocolGuid,
        //           (void **)&BlkIo2);

//#if OKR_DEBUG
//        if (Status==0)
//            MyLog(LOG_DEBUG,L"get block io2 protocol!\n");
//#endif

        Media = BlkIo->Media;

		if (Media->BlockSize != 512)
		{
			continue;
		}

		//设备中是否有介质
		if (Media->MediaPresent != TRUE)
		{
			continue;
		}

		//介质是否为分区TRUE，还是整个设备FALSE
        if (Media->LogicalPartition == TRUE)
        {
//#if OKR_DEBUG
//        MyLog(LOG_DEBUG, L"LogicalPartition %d: handle=%x, blkio=%x, status=%x, blocksize=%d, \
//Removable=%d, Present=%d, Logicalpart=%d, Readonly=%d, lastblock=%x LogicalBlocksPerPhysicalBlock=%x\n",
//	  		i,
//	  		HandleBuffer[i], BlkIo, Status, Media->BlockSize,
//            Media->RemovableMedia,
//            Media->MediaPresent,
//            Media->LogicalPartition,
//            Media->ReadOnly,
//            Media->LastBlock,
//			Media->LogicalBlocksPerPhysicalBlock);
//#endif
            continue;
        }

        //todo check cdrom
        Status=BlkIo->ReadBlocks(
                   BlkIo,
                   Media->MediaId,
                   (EFI_LBA)0,
                   Media->BlockSize,  //The intrinsic block size of the device
                   Buffer);

		if (Status != 0)
		{
			continue;
		}

		//Media->LastBlock: The last logical block address on the device.
		if (Media->LastBlock < 8 * 2048)// cap < 8M  //??
		{
			continue;
		}

        if (g_disknum == MAX_DISK_NUM) 
		{
            MyLog(LOG_ALERT,L"Too Many Disks\n");
            break;
        }

		UINT8	uiDevicePathProtocolType = 0;
		UINT8	uiDevicePathProtocolSubType = 0;
		GetDiskType(HandleBuffer[i], &uiDevicePathProtocolType, &uiDevicePathProtocolSubType);
		if (g_data.bIsAutoFunc && 3 == uiDevicePathProtocolType && 5 == uiDevicePathProtocolSubType)
		{
			continue;
		}

		g_disklist[g_disknum].uiDevicePathProtocolType = uiDevicePathProtocolType;
		g_disklist[g_disknum].uiDevicePathProtocolSubType = uiDevicePathProtocolSubType;

        name = GetDiskName(HandleBuffer[i]);
		
        if (name==NULL) 
		{
            SPrint(g_disklist[g_disknum].diskname,sizeof(g_disklist[g_disknum].diskname),L"Disk #%d",(UINTN)g_disknum);
        } 
		else 
		{
            StrnCpy(g_disklist[g_disknum].diskname,name,sizeof(g_disklist[g_disknum].diskname)/sizeof(CHAR16));
        }
		
		SetMem(g_disklist[g_disknum].disksn,20,0);
		SetMem(SnBuffer,20,0);
		MyLog(LOG_DEBUG, L"disk %d Media->RemovableMedia %d\n", i, Media->RemovableMedia);
		if (Media->RemovableMedia == FALSE)
		{
			MyLog(LOG_DEBUG, L"disk %d Media->RemovableMedia == FALSE\n", i);
			GetDiskSn(HandleBuffer[i], SnBuffer);
		}
		CopyMem(g_disklist[g_disknum].disksn,SnBuffer,20);
		MyLog(LOG_ERROR, L"Disk%d sn =%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",g_disknum,SnBuffer[0],SnBuffer[1],SnBuffer[2],SnBuffer[3],SnBuffer[4],SnBuffer[5],SnBuffer[6],SnBuffer[7],SnBuffer[8],SnBuffer[9],SnBuffer[10],SnBuffer[11],SnBuffer[12],SnBuffer[13],SnBuffer[14],SnBuffer[15],SnBuffer[16],SnBuffer[17],SnBuffer[18],SnBuffer[19]);
		MyLog(LOG_ERROR, L"Disk%d sn =[%a]\n", g_disknum, SnBuffer);

        g_disklist[g_disknum].devhandle = HandleBuffer[i];
        g_disklist[g_disknum].devblkIo = BlkIo;
        g_disklist[g_disknum].removable = Media->RemovableMedia;
        g_disklist[g_disknum].totalsector = Media->LastBlock + 1;  //??
        
		SetMem(g_disklist[g_disknum].raidDisksn, 20, 0);
		g_disklist[g_disknum].isRAIDDisk = IsRAIDDisk(g_disknum);
		//MyLog(LOG_DEBUG, L"isRAIDDisk: %x\n", g_disklist[g_disknum].isRAIDDisk);
		if (1 == g_disklist[g_disknum].isRAIDDisk)
		{
			SetMem(g_disklist[g_disknum].raidDisksn, 20, 0);
			CopyMem(g_disklist[g_disknum].raidDisksn, "RAID                ", 20);

			SetMem(totalSector, 16, 0);
			AsciiSPrint(totalSector, 16, "%lld", g_disklist[g_disknum].totalsector);

			for (int iIndex = 0; iIndex < 16; iIndex++)
			{
				if ('\0' == totalSector[iIndex])
				{
					break;
				}
				g_disklist[g_disknum].raidDisksn[4 + iIndex] = totalSector[iIndex];
			}

			MyLog(LOG_DEBUG, L"raidDisksn: %a\n", g_disklist[g_disknum].raidDisksn);
		}

#if OKR_DEBUG
        MyLog(LOG_DEBUG, L"Block %d: device handle=%x, blk_handle=%x read=%d,block size=%d, \
Removable=%d, Present=%d, IoAlign=%d, Logicalpart=%d, Readonly=%d, lastblock=%x name:%s, g_disklist[g_disknum].totalsector:%d, \
DevicePathProtocolType=%d, DevicePathProtocolSubType=%d\n",
	  		g_disknum,
	  		HandleBuffer[i] ,BlkIo,Status,Media->BlockSize,\
            Media->RemovableMedia,
            Media->MediaPresent,
			Media->IoAlign,
            Media->LogicalPartition,
            Media->ReadOnly,
            Media->LastBlock,
            g_disklist[g_disknum].diskname,
			g_disklist[g_disknum].totalsector,
			g_disklist[g_disknum].uiDevicePathProtocolType,
			g_disklist[g_disknum].uiDevicePathProtocolSubType);
		getch();
#endif            
        g_disknum++;
    }

	if (HandleBuffer != NULL) 
	{
		FreePool(HandleBuffer);
	}

	if (g_disknum == 0)
	{
		return -1;
	}
    return 0;
}

//diskindex	：	磁盘号
//sector	：	读取设备/分区起始块的LBA地址
//num		:	读取的扇区数。扇区数 *512 = 读取的字节数。
//buffer	：	读取数据缓冲区
BOOL DiskRead(WORD diskindex, UINT64 sector, DWORD num, VOID *buffer)
{
	//MyLog(LOG_DEBUG, L"DiskRead  diskindex %d. sector %d num %d\n", diskindex, sector, num);

	EFI_STATUS Status;
	if (diskindex >= g_disknum)
	{
		MyLog(LOG_DEBUG, L"diskindex >= g_disknum\n");
		return FALSE;
	}

	Status = g_disklist[diskindex].devblkIo->ReadBlocks(
		g_disklist[diskindex].devblkIo,
		g_disklist[diskindex].devblkIo->Media->MediaId,
		(EFI_LBA)sector,
		512 * num,
		buffer);
	if (Status == 0)
	{
		return TRUE;
	}
	else 
	{
		MyLog(LOG_ERROR, L"DiskRead(%d,%ld,%d) failed with error 0x%lx\r\n", (DWORD)diskindex, sector, num, Status);
		MyLog(LOG_ERROR, L"%ld\r\n", sector);
		MyLog(LOG_ERROR, L"%d\r\n", num);
		return FALSE;
	}
}

BOOL DiskWrite(WORD diskindex,UINT64 sector, DWORD num,VOID *buffer)
{
	//MyLog(LOG_ERROR, L"DiskWrite disk:%d sector: %llx num: %d \r\n", diskindex, sector, num);

    EFI_STATUS Status;
	if (diskindex >= g_disknum)
	{
		MyLog(LOG_ERROR, L"DiskWrite failed. diskindex (%d) >= g_disknum (%d) \r\n", diskindex, g_disknum);
		return FALSE;
	}

    Status=g_disklist[diskindex].devblkIo->WriteBlocks(
               g_disklist[diskindex].devblkIo,
               g_disklist[diskindex].devblkIo->Media->MediaId,
               (EFI_LBA)sector,
               512*num,
               buffer);
    if (Status==0)
        return TRUE;
    else {
        MyLog(LOG_ERROR,L"DiskWrite(%d, %ld) failed with error 0x%lx\r\n",(DWORD)diskindex,sector,Status);
		MyLog(LOG_ERROR,L"%ld\r\n", sector);
		MyLog(LOG_ERROR,L"%d\r\n", num);
        return FALSE;
    }
}

BOOL DiskFlush(WORD diskindex)
{
    EFI_STATUS Status;
    if (diskindex>=g_disknum)
        return FALSE;

    Status=g_disklist[diskindex].devblkIo->FlushBlocks( g_disklist[diskindex].devblkIo);
    if (Status==0)
        return TRUE;
    else {
        MyLog(LOG_ERROR,L"FlushBlocks(%d, %ld) failed with error \r\n",(DWORD)diskindex,Status);
        return FALSE;
    }
}

int mywcsnicmp(CHAR16 *src, CHAR16 *dst, int len)
{
    while (*dst && len)
    {
        if (*src != *dst)
        {
            if (*src > *dst && *src != *dst + 0x20)
                break;
            if (*src < *dst && *src != *dst - 0x20)
                break;
        }
        ++src, ++dst, --len;
    }

    if (len && *dst != *src)
        return (*src - *dst);
    return 0;
}

int strnicmp(char *str1, char * str2, int len)
{
    while (((*str1 == *str2)||(*str1 == *str2+0x20)||(*str1 == *str2-0x20))
            &&(len != 0))
    {
        if ((*str1 == 0)||(*str2 == 0))
            break;
        if (*str1 > *str2)
        {
            if ((*str1 < 'a')||(*str1 > 'z'))
                return 1;
        }
        else if (*str1 < *str2)
        {
            if ((*str1 < 'A')||(*str1 > 'Z'))
                return -1;
        }
        str1++;
        str2++;
        len--;
    }
    if (len == 0)
        return 0;

    if (*str1 > *str2)
        return 1;
    else if (*str1 == *str2)
        return 0;
    else
        return -1;
}

  
 
/////////////////////////////////////
/**
  Function to verify and convert a string to its numerical value.

  If Hex it must be preceeded with a 0x, 0X, or has ForceHex set TRUE.

  @param[in] String       The string to evaluate.
  @param[out] Value       Upon a successful return the value of the conversion.
  @param[in] ForceHex     TRUE - always assume hex.
  @param[in] StopAtSpace  FALSE to skip spaces.

  @retval EFI_SUCCESS             The conversion was successful.
  @retval EFI_INVALID_PARAMETER   String contained an invalid character.
  @retval EFI_NOT_FOUND           String was a number, but Value was NULL.
**/
EFI_STATUS
EFIAPI
itoa64(
    IN CONST CHAR16   *String,
    IN CONST BOOLEAN  Hex,
    OUT   UINT64   *Value
)
{
	*Value = StrHexToUint64(String);
	return 0;
}

INTN
EFIAPI
MyMemCmp (
    IN       BYTE              *FirstString,
    IN       BYTE              *SecondString,
    INTN  num
)
{
    INTN i;
    //
    // ASSERT both strings are less long than PcdMaximumUnicodeStringLength
    //
    for (i=0;i<num;i++) {
        if (*FirstString != *SecondString)
            return 1;
        FirstString++;
        SecondString++;
    }
    return 0;
}
////////////////p//////////////////////////////
//
// -- Time Management Routines --
//

#define IsLeap(y)   (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define SECSPERMIN  (60)
#define SECSPERHOUR (60 * 60)
#define SECSPERDAY  (24 * SECSPERHOUR)

//
//  The arrays give the cumulative number of days up to the first of the
//  month number used as the index (1 -> 12) for regular and leap years.
//  The value at index 13 is for the whole year.
//
UINTN CumulativeDays[2][14] = {
    {
        0,
        0,
        31,
        31 + 28,
        31 + 28 + 31,
        31 + 28 + 31 + 30,
        31 + 28 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
    },
    {
        0,
        0,
        31,
        31 + 29,
        31 + 29 + 31,
        31 + 29 + 31 + 30,
        31 + 29 + 31 + 30 + 31,
        31 + 29 + 31 + 30 + 31 + 30,
        31 + 29 + 31 + 30 + 31 + 30 + 31,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
    }
};

time_t mytime (time_t *timer)
{
    EFI_TIME  Time;
    UINTN     Year;
	time_t		curtime;

    //
    // Get the current time and date information
    //
    gRT->GetTime (&Time, NULL);

    //
    // Years Handling
    // UTime should now be set to 00:00:00 on Jan 1 of the current year.
    //
    for (Year = 1970, curtime = 0; Year != Time.Year; Year++) {
        curtime = curtime + (time_t)(CumulativeDays[IsLeap(Year)][13] * SECSPERDAY);
    }

    //
    // Add in number of seconds for current Month, Day, Hour, Minute, Seconds, and TimeZone adjustment
    //
    curtime = curtime +
             (time_t)((Time.TimeZone != EFI_UNSPECIFIED_TIMEZONE) ? (Time.TimeZone * 60) : 0) +
             (time_t)(CumulativeDays[IsLeap(Time.Year)][Time.Month] * SECSPERDAY) +
             (time_t)(((Time.Day > 0) ? Time.Day - 1 : 0) * SECSPERDAY) +
             (time_t)(Time.Hour * SECSPERHOUR) +
             (time_t)(Time.Minute * 60) +
             (time_t)Time.Second;
	if(timer)
		*timer = curtime;

    return curtime;
}

//
// Convert a time value from type time_t to struct tm.
//
struct mytm * mygmtime (const time_t *timer, struct mytm * tm)
{
    struct mytm  *GmTime;
    UINT32     DayNo;
    UINT32     DayRemainder;
    time_t     Year;
    time_t     YearNo;
    UINT32     TotalDays;
    UINT32     MonthNo;

    if (!timer || !tm) {
        return NULL;
    }

    GmTime = tm;

    SetMem ((VOID *) GmTime, (UINTN) sizeof (struct mytm),0);

    DayNo        = (UINT32) (*timer / SECSPERDAY);
    DayRemainder = (UINT32) (*timer % SECSPERDAY);

    GmTime->tm_sec  = (int) (DayRemainder % SECSPERMIN);
    GmTime->tm_min  = (int) ((DayRemainder % SECSPERHOUR) / SECSPERMIN);
    GmTime->tm_hour = (int) (DayRemainder / SECSPERHOUR);
    GmTime->tm_wday = (int) ((DayNo + 4) % 7);

    for (Year = 1970, YearNo = 0; DayNo > 0; Year++) {
        TotalDays = (UINT32) (IsLeap (Year) ? 366 : 365);
        if (DayNo >= TotalDays) {
            DayNo -= TotalDays;
            YearNo++;
        } else {
            break;
        }
    }

    GmTime->tm_year = (int) (YearNo + (1970 - 1900));
    GmTime->tm_yday = (int) DayNo;

    for (MonthNo = 12; MonthNo > 1; MonthNo--) {
        if (DayNo >= CumulativeDays[IsLeap(Year)][MonthNo]) {
            DayNo -= (UINT32) (CumulativeDays[IsLeap(Year)][MonthNo]);
            break;
        }
    }

    GmTime->tm_mon  = (int) MonthNo;
    GmTime->tm_mday = (int) DayNo + 1;

    GmTime->tm_isdst  = 0;
    GmTime->tm_gmtoff = 0;
    GmTime->tm_zone   = NULL;

    return GmTime;
}

DWORD GetSeconds()
{
	static EFI_TIME	begintime = {0};
	EFI_STATUS	status;
	EFI_TIME	time;
	DWORD		seconds;
	
	status = gRT->GetTime(&time, NULL);
	if(begintime.Year == 0 && begintime.Month == 0)
	{
		CopyMem(&begintime, &time, sizeof(EFI_TIME));
		return 0;
	}
	
	if(time.Hour < begintime.Hour)
		seconds = time.Hour + 24 - begintime.Hour;
	else
		seconds = time.Hour - begintime.Hour;
	seconds *= 60;
	seconds = seconds + time.Minute - begintime.Minute;
	seconds *= 60;
	seconds = seconds + time.Second - begintime.Second;
	return (DWORD)seconds;
}

//根据磁盘HANDLE找到每个Partition对应子HANDLE,如果支持文件系统，则把handle填入到m_partitions[]
int CheckPartitionHandle(WORD disknumber)
{
	EFI_HANDLE diskhandle = g_disklist[disknumber].devhandle;
	EFI_STATUS status;
	UINTN i, j;
	EFI_GUID                            **ProtocolGuidArray;
	UINTN                               ArrayCount;
	EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
	UINTN                               OpenInfoCount;
	UINTN                               ChildIndex;
	EFI_DEVICE_PATH_PROTOCOL *DevicePath;
	EFI_BLOCK_IO_PROTOCOL *Blkio;
	BYTE *buffer;	//[16*512];
	BYTE *buffer1;	//[16*512];

	buffer = (BYTE*)g_data.bmp;
	buffer1 = (BYTE*)g_data.bmp + 16 * 512;

	//如果这个硬盘没有格式化就不用检查了，肯定没有可用的分区
	if (g_disklist[disknumber].formatted != TRUE)
		return -1;

#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"check disk %d child handle\n", disknumber);
#endif    
	/// find child handle of diskhandle
	status = gBS->ProtocolsPerHandle(
		diskhandle,
		&ProtocolGuidArray,
		&ArrayCount
	);
	if (status != 0)
		return -1;
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"CheckPartitionHandle, count %d\n", ArrayCount);
#endif    
	for (i = 0; i < ArrayCount; i++) {
		status = gBS->OpenProtocolInformation(
			diskhandle,
			ProtocolGuidArray[i],
			&OpenInfo,
			&OpenInfoCount
		);
		if (status != 0)
			continue;

		for (ChildIndex = 0; ChildIndex < OpenInfoCount; ChildIndex++) {
			EFI_HANDLE childctlhandle = OpenInfo[ChildIndex].ControllerHandle;
			EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Vol;

			if (childctlhandle == 0)
				continue;
			if (childctlhandle == diskhandle)
				continue;
			if (!(OpenInfo[ChildIndex].Attributes&EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER))
				continue;
			status = gBS->HandleProtocol(
				childctlhandle,
				&gEfiDevicePathProtocolGuid,
				&DevicePath
			);
			//		   if (DevicePath!=0)
			//		       MyLog(LOG_DEBUG,L"child: %x, %s \n",childctlhandle,DevicePathToStr(DevicePath));
			//	       else
			//	       	MyLog(LOG_DEBUG,L"child: %x,no path\n",childctlhandle);
						//       status=gBS->HandleProtocol(childctlhandle,&gEfiSimpleFileSystemProtocolGuid,&Vol);
						//		if (EFI_ERROR(status))// no filesystem
						//		   	continue;
						//read boot sector 16 sector
			status = gBS->HandleProtocol(childctlhandle, &gEfiBlockIoProtocolGuid, (void **)&Blkio);
			if (status != 0)
				continue;
			status = Blkio->ReadBlocks(Blkio, Blkio->Media->MediaId, 0, 512 * 16, buffer);
			if (status != 0)
				continue;
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"OpenInfoCount %d, ChildIndex %d\n",
				OpenInfoCount, ChildIndex);
#endif    
			//check partitions bpb
			for (j = 0;; j++) {
				BYTE pid;
				int partindex = GetPartitionIndex(disknumber, (DWORD)j);
				if (partindex < 0)
					break;
				pid = g_Partitions[partindex].PartitionType;
				if ((pid != NTFS)/* && (pid != FAT16) && (pid != FAT32) && (pid != FAT16_E) && (pid != FAT32_E)*/)//only support fat and ntfs write file
					continue;
				//					 if (IsUnSupportedPartition(g_Partitions[partindex].PartitionType))
				//					 	continue;
#if OKR_DEBUG
				MyLog(LOG_DEBUG, L"DiskRead(%d,%d), partindex %d\n",
					disknumber, g_Partitions[partindex].BootBegin, partindex);
#endif    
				//  Print(L"DiskRead(%d,%d), partindex %d\n",disknumber,g_Partitions[partindex].BootBegin, partindex);

				if (DiskRead(disknumber, g_Partitions[partindex].BootBegin, 16, buffer1) == FALSE)
					continue;
				if (MyMemCmp(buffer, buffer1, 16 * 512) == 0)
				{
					g_Partitions[partindex].Handle = childctlhandle;
					MyLog(LOG_DEBUG, L" get partition index %d \n", partindex);
					//	MyLog(LOG_DEBUG,L" get partition index %d \n",partindex);
					break;
				}
			}
		}

		FreePool(OpenInfo);
	}

	FreePool(ProtocolGuidArray);
	return 0;
}

int CheckFatPartitionHandle(WORD disknumber)
{
    EFI_HANDLE diskhandle = g_disklist[disknumber].devhandle;
    EFI_STATUS status;
    UINTN i, j;
    EFI_GUID                            **ProtocolGuidArray;
    UINTN                               ArrayCount;
    EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
 //   UINTN                               OpenInfoCount;
 //   UINTN                               ChildIndex;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    EFI_BLOCK_IO_PROTOCOL *Blkio;
    BYTE *buffer;	//[16*512];
    EFI_HANDLE childctlhandle;
    
    INTN               HandleFileCount,HandleFileIndex=0;   //HandleFileCount表示个数
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Sfs;           //将协议加载到Sfs上
    EFI_HANDLE                        *Files=NULL;          //Files用来接收容器
    EFI_FILE_PROTOCOL         *Root;          //Root是用来打开卷的

    BYTE *buffer1;	//[16*512];

	buffer = (BYTE*)g_data.bmp;
	buffer1 = (BYTE*)g_data.bmp + 16*512;

	//如果这个硬盘没有格式化就不用检查了，肯定没有可用的分区
	if(g_disklist[disknumber].formatted != TRUE)
		return -1;
  
  // Print(L"CheckPartitionHandle\n");

 
   status=gST->BootServices->LocateHandleBuffer(
   ByProtocol,
   &gEfiSimpleFileSystemProtocolGuid,
   NULL,
   &HandleFileCount,
   &Files);
  
   if(!EFI_ERROR(status))
   {
   	  // Print(L"CheckPartitionHandle-1,HandleFileCount:%d\n",HandleFileCount);

       for(HandleFileIndex=HandleFileCount-1; HandleFileIndex > -1; --HandleFileIndex)
       {
          // 打开EFI_SIMPLE_FILE_SYSTEM_PROTOCOL 
           //将协议加载到Sfs上，这样Sfs才能打开卷
          status = gST->BootServices->HandleProtocol(
              Files[HandleFileIndex],
              &gEfiSimpleFileSystemProtocolGuid,
              (VOID**)&Sfs);

          if(status == EFI_SUCCESS)
          {
          	 childctlhandle = Files[HandleFileIndex];
             status = gBS->HandleProtocol (childctlhandle, &gEfiBlockIoProtocolGuid, (void **)&Blkio);
            if (status != 0)
                continue;
            status = Blkio->ReadBlocks( Blkio,Blkio->Media->MediaId,0,512*16,buffer);
            if (status != 0)
            	 continue;
          
            for (j=0;;j++)
            {
				        BYTE pid;
                int partindex = GetPartitionIndex(disknumber,(DWORD)j);
                if (partindex < 0)
                    break;
                pid = g_Partitions[partindex].PartitionType;
                if (/*(pid != NTFS) &&*/ (pid != FAT16) && (pid != FAT32) && (pid != FAT16_E) && (pid != FAT32_E))//only support fat
                    continue;

             //   Print(L"DiskRead(%d,%d), partindex %d\n",disknumber,g_Partitions[partindex].BootBegin, partindex);
					
                if (DiskRead(disknumber,g_Partitions[partindex].BootBegin,16,buffer1)==FALSE)
                    continue;
                if (MyMemCmp(buffer,buffer1,16*512)==0) 
                {
                    g_Partitions[partindex].Handle=childctlhandle;
                    //	MyLog(LOG_DEBUG,L" get partition index %d \n",partindex);
             //       Print(L" get partition index %d \n",partindex);
                    break;
                }
            }
          }
       }
   }  
   
   if(Files!=NULL)
   {
   	  gBS->FreePool(Files);
   }
    return 0;
}


/**
  Converting a given device to an unicode string.

  This function will dependent on gEfiDevicePathToTextProtocolGuid, if protocol
  does not installed, then return unknown device path L"?" directly.

  @param    DevPath     Given device path instance

  @return   Converted string from given device path.
  @retval   L"?"  Can not locate gEfiDevicePathToTextProtocolGuid protocol for converting.
**/
CHAR16 *
DevicePathToStr (
    IN EFI_DEVICE_PATH_PROTOCOL     *DevPath
)
{
    EFI_STATUS                       Status;
    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *DevPathToText;
    CHAR16                           *ToText = L"";

    if (DevPath == NULL) {
        return L"";
    }

    Status = gBS->LocateProtocol (
                 &gEfiDevicePathToTextProtocolGuid,
                 NULL,
                 (VOID **) &DevPathToText
             );
    if (!EFI_ERROR (Status)) {
        ToText = DevPathToText->ConvertDevicePathToText (
                     DevPath,
                     FALSE,
                     TRUE
                 );
        ASSERT (ToText != NULL);
        //  Print(L"!!!%s!!!!",ToText);
        return ToText;
    } else {
        MyLog(LOG_ERROR,L"DevicePathToTextProtocolGuid NOT Support! %d\n",Status);
    }

    return L"\\/";
}

void SmartDiskIoInit(BYTE DiskNumber, BYTE readorwrite,struct CONTINUE_RWDISK_INFO *info)
{
	mymemset(info, 0, sizeof(struct CONTINUE_RWDISK_INFO));
    info->Func = (BYTE)readorwrite;
    info->DiskNumber = DiskNumber;
}

BOOL SmartDiskIo(struct CONTINUE_RWDISK_INFO *info, UINT64 Sector, DWORD Number, BYTE *Buffer)
{
    if (!info->Number) {

        info->Sector = Sector;
        info->Number = Number;
        info->Buffer = Buffer;

	} else {

        if (Number && Sector == info->Sector + info->Number) {

            info->Number += Number;

        } else {

			DWORD count;
			BOOL ret;

			while(info->Number)
			{
				if(info->Number > 2048)
					count = 2048;
				else
					count = info->Number;

				if (info->Func == COMMAND_READ)
					ret = DiskRead((WORD)info->DiskNumber, info->Sector, count, info->Buffer);
				else if(info->Func == COMMAND_WRITE)
					ret = DiskWrite((WORD)info->DiskNumber, info->Sector, count, info->Buffer);

				if (!ret)
					break;
				info->Sector += count;
				info->Number -= count;
				info->Buffer += count * 512;
			}

            info->Sector = Sector;
            info->Number = Number;
            info->Buffer = Buffer;

			if (!ret)
			{
				//MyLog(LOG_DEBUG, L"SmartDiskIo return FALSE.\n");
				return FALSE;
			}
        }
    }

    return TRUE;
}


 

BOOL FileSystemInit(OUT PFILE_SYS_INFO info,WORD partindex,BOOL readonly)
{
	//MyLog(LOG_DEBUG, L"FileSystemInit...\n");

    EFI_STATUS status;
    SetMem(info,sizeof(FILE_SYS_INFO),0);
 
    if (g_Partitions[partindex].PartitionType==NTFS) {
        //if (g_Partitions[partindex].Handle==0)
        //    return FALSE;
        info->ifntfs=TRUE;
    } else {
        info->ifntfs=FALSE;
        if (g_Partitions[partindex].Handle==0) {//efi not support
            MyLog(LOG_DEBUG,L"FileSystemInit(%d): Not supported.\n", partindex);
            return FALSE;
        }
    }
 
    if (info->ifntfs) {
		EFI_BLOCK_IO_PROTOCOL	*blockIo = NULL;
        DWORD ret=0;
      
#ifdef NTFSLIB
		if(g_Partitions[partindex].Handle)
		{
        	status=gBS->HandleProtocol(g_Partitions[partindex].Handle, &gEfiBlockIoProtocolGuid, &blockIo);
        	if (EFI_ERROR(status))
        	{
				MyLog(LOG_DEBUG, L"HandleProtocol gEfiBlockIoProtocolGuid Error. status=%d\n", status);
        		return FALSE;
        	}
        }
        else
        {
       		WORD diskindex = g_Partitions[partindex].DiskNumber;
       		blockIo = g_disklist[diskindex].devblkIo;
        }
		 
        info->partinfo.MediaId = blockIo->Media->MediaId;
        info->partinfo.BytesPerSector = blockIo->Media->BlockSize;
        info->partinfo.BytesPerPhysicalSector = blockIo->Media->BlockSize * blockIo->Media->LogicalBlocksPerPhysicalBlock;
        if(g_Partitions[partindex].Handle)
        {
        	info->partinfo.BeginSector = 0;
	        info->partinfo.TotalSectors = blockIo->Media->LastBlock;
        }
        else
        {
        	info->partinfo.BeginSector = g_Partitions[partindex].BootBegin;
	        info->partinfo.TotalSectors = g_Partitions[partindex].TotalSectors;
        }
		 
        info->partinfo.DiskHandle = g_Partitions[partindex].Handle;
        info->partinfo.ReadBlocks = blockIo->ReadBlocks;
        info->partinfo.WriteBlocks = blockIo->WriteBlocks;
        info->partinfo.FlushBlocks = blockIo->FlushBlocks;
        info->partinfo.BlockIo = blockIo;
        
		MyLog(LOG_DEBUG, L"call n_init\n");
		ret = n_init(&info->partinfo,readonly);
		MyLog(LOG_DEBUG, L"n_init return: %d\n", ret);

        if (ret != N_SUCCESS) {
            MyLog(LOG_DEBUG,L"FileSystemInit(%d) failed with error %d.\n", (DWORD)partindex, (DWORD)ret);
            return FALSE;
        } 
		else
		{
			//MyLog(LOG_DEBUG, L"FileSystemInit return TRUE. \n");
			return TRUE;
		}
#else
        return FALSE;
#endif
	
    } else {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Vol;
		 
        status=gBS->HandleProtocol(g_Partitions[partindex].Handle,&gEfiSimpleFileSystemProtocolGuid,&Vol);
        if (EFI_ERROR(status)) {
			MyLog(LOG_DEBUG,L"FileSystemInit(FAT:%d) failed with error %d.\n", (DWORD)partindex, (DWORD)status);
            return FALSE;
        }
		 
        status=Vol->OpenVolume(Vol,&info->RootFs);
		 
        if (EFI_ERROR(status)) {
            MyLog(LOG_ERROR,L"open volume error %x",status);
            return FALSE;
        }
        return TRUE;
    }
	 
}

void FileSystemClose(PFILE_SYS_INFO info)
{
#ifdef NTFSLIB

	if (NULL == info)
	{
		return;
	}

    if (info->ifntfs) {
        n_uninit(TRUE);
    }
#endif
}

//fill ntfshandle or filehandle
BOOL _FileOpen(PFILE_HANDLE_INFO Handle, CHAR16 *FileName, UINT64 OpenMode, UINT64 FileAttribute)
{
	BOOL bRet = TRUE;

    if (Handle->psys->ifntfs) {
#ifdef NTFSLIB
		DWORD status = 0;
        char asciifilename[256] = { 0 };
        
        Handle->cachesize = DEFAULT_FILE_CACHE_SIZE;
        Handle->cachedatasize = 0;
		Handle->cachedatapos = 0;
		Handle->cachedirty = FALSE;
	    Handle->cache = myalloc(Handle->cachesize);		//分配1MB大小的cache
	    
        Unicode2Ascii(asciifilename, FileName);
        if (FileAttribute & EFI_FILE_DIRECTORY) {//dir
			if (EFI_FILE_MODE_CREATE & OpenMode){
				status = n_createdir(asciifilename);
				if ((status != N_FILE_EXISTS) && (status != N_SUCCESS)){

					MyLog(LOG_DEBUG,L"ntfs_create_dir failed with error 0x%x",status);
					bRet = FALSE;
				}
			}
            Handle->ifdir = 1;
        } else {//nomal file
            status = n_openfile(asciifilename, &Handle->filehandle);
            if ((status == N_FILE_NOT_FOUND)&&(OpenMode&EFI_FILE_MODE_CREATE)) {
                if (n_createfile(asciifilename, &Handle->filehandle)!=N_SUCCESS) 
                {
                    //chenzg1
                	/* MyLog(LOG_DEBUG,L"n_createfile_unicode,%s",FileName);
                	 getch();
                   if(n_createfile_unicode(asciifilename,FileName,StrLength(FileName),&Handle->filehandle)!=N_SUCCESS)
					         {
					         	 MyLog(LOG_DEBUG,L"ntfs_create_file failed.%s",FileName);
                     bRet = FALSE;
                   }
                  
                  {
                  
                  	 unsigned char *tmp="ab123.lnk";
                  	unsigned char test3[] = {0xef,0xbb,0xbf,0xe4,0xb8,0xa5,0x2e,0x65,0x78,0x65,0x00 };
                  	unsigned char test6[] = {0xc1,0xeb,0x2e,0x65,0x78,0x65,0x00 };
                  	unsigned char test10[] = {0xef,0xbb,0xbf,0xe8,0x81,0x94,0x2e,0x65,0x78,0x65,0x00 };
                  	unsigned char test4[] = {0x61,0x61,0x62,0x31,0x32,0x33,0x2e,0x65,0x78,0x65,0x00 };
                  	unsigned char tempfile11[]={0x61,0xef,0xbb,0xbf,0x61,0x62,0x32,0x33,0x34,0x2e,0x6c,0x6e,0x6b,0x00};
                  		DWORD ret=N_SUCCESS;
	NFILE file=NULL;

	ret=n_createfile_unicode((L"\\Users\\联想软件与驱动安装.lnk"),sizeof(L"\\Users\\联想软件与驱动安装.lnk")/2-1,&file);
	if(NSUCCEED(ret)&&file)
	{
		MyLog(LOG_DEBUG,L"n_createfile1 sucess.");
		n_closefile(file);
	}
	else
	{
		MyLog(LOG_DEBUG,L"create file1 failed,ret=0x%lx,%lu\n",ret,ret);
	//	Print(L"create file failed,ret=0x%lx,%lu\n",ret,ret);
	}



                  }
                  */
                }
            } else if (status != N_SUCCESS) {
                MyLog(LOG_DEBUG,L"ntfs_open_file failed with error 0x%x",status);
				bRet = FALSE;
            }
        }
#else
		bRet = FALSE;
#endif
    } else {
		EFI_STATUS status;
        EFI_FILE_PROTOCOL * RootFs=Handle->psys->RootFs;

        status = RootFs->Open(RootFs, &Handle->filehandle, FileName, OpenMode, FileAttribute);
        if (EFI_ERROR(status)) {
			MyLog(LOG_DEBUG, L"OpenFile failed with error 0x%x", status);
			bRet = FALSE;
        }
    }
	//MyLog(LOG_DEBUG, L"_FileOpen: %d\n", bRet);
	return bRet;
}

BOOL FileDelete(PFILE_SYS_INFO info, CHAR16 *filename)
{
	char asciifilename[256] = { 0 };
	
	if (info->ifntfs) {
		Unicode2Ascii(asciifilename,filename);
		if(n_deletefile(asciifilename) == 0)
			return TRUE;
	} else {
		FILE_HANDLE_INFO filehandle;
		if (FileOpen(info,&filehandle,filename,EFI_FILE_MODE_CREATE|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ,0,TRUE)) {
			EFI_STATUS status;
			EFI_FILE_PROTOCOL * RootFs = filehandle.psys->RootFs;
			status = RootFs->Delete(filehandle.filehandle);
			if (!EFI_ERROR(status)) {
				return TRUE;
			} else {
				FileClose(&filehandle);
			}
		}
	}
	
	return FALSE;
}

BOOL FileGetSize(PFILE_HANDLE_INFO handle, UINT64 *FileSize)
{
	if (handle->psys->ifntfs) {
		if(n_getfilesize(handle->filehandle, FileSize) == N_SUCCESS)
			return TRUE;
	} else {
		BYTE buffer[1024] = { 0 };
		EFI_STATUS status;
		EFI_FILE_PROTOCOL * RootFs = handle->psys->RootFs;
		EFI_FILE_INFO *info = (EFI_FILE_INFO *)buffer;
		UINTN infosize = 1024;

		//,EFI_FILE_INFO_ID
		status = RootFs->GetInfo(handle->filehandle, &gEfiFileInfoGuid, &infosize, info);
		if (!EFI_ERROR(status)) {
			*FileSize = info->FileSize;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GetFileSizeByName(IN PFILE_SYS_INFO info,CHAR16 *filename,UINT64 *FileSize)
{
	FILE_HANDLE_INFO handle;
	BOOL bRet = FALSE;

	if(FileSize)
		*FileSize = 0;	//先清0

	if (!FileOpen(info, &handle, filename, EFI_FILE_MODE_READ, 0, FALSE)) {
		return FALSE;
	}

	bRet = FileGetSize(&handle, FileSize);

	FileClose(&handle);

	return bRet;
}

//support directry,create file or dir.attribute use EFI system define
//ifneedseg, true=support file segments
BOOL FileOpen(IN PFILE_SYS_INFO info,OUT PFILE_HANDLE_INFO handle,CHAR16 *filename,UINT64 openmode,UINT64 attribute,BOOL ifneedseg)
{
    EFI_STATUS status;

    SetMem(handle,sizeof(FILE_HANDLE_INFO),0);
    handle->psys=info;
    
    if (handle->psys->ifntfs)
    	handle->ifneedseg= FALSE;
    else
    	handle->ifneedseg=ifneedseg;

    StrnCpy(handle->filename,filename,sizeof(handle->filename)/sizeof(CHAR16));

    return _FileOpen(handle,filename,openmode,attribute);
}

BOOL SegFileOpen(IN PFILE_SYS_INFO info,OUT PFILE_HANDLE_INFO handle,CHAR16 *filename,UINT64 openmode,UINT64 attribute,BOOL ifneedseg)
{
    EFI_STATUS status;

    SetMem(handle,sizeof(FILE_HANDLE_INFO),0);
    handle->psys=info;
    
  //  if (handle->psys->ifntfs)
  //  	handle->ifneedseg= FALSE;
  //  else
  //  	handle->ifneedseg=ifneedseg;

   	handle->ifneedseg=ifneedseg;

    StrnCpy(handle->filename,filename,sizeof(handle->filename)/sizeof(CHAR16));

    return _FileOpen(handle,filename,openmode,attribute);
}



BOOL SegFileRead( PFILE_HANDLE_INFO handle,OUT UINTN * size, PBYTE buffer )
{
	  //MY_DEFINE_BUFFER_SIZE
	  UINTN index = 0;
	  UINTN buffersize = 0;
	  EFI_STATUS status;
 
	  if( (*size) < MY_DEFINE_BUFFER_SIZE )
	  {
	  	  status = handle->psys->RootFs->Read(handle->filehandle,size,buffer);        
        if (EFI_ERROR(status)) 
        {
            MyLog(LOG_ERROR,L"SegFileRead failed with error 0x%x.",status);
            return FALSE;
        } else
            return TRUE;
	  }
	  
	  for(index=0;index < (*size);)
	  {
	  	  buffersize = MY_DEFINE_BUFFER_SIZE;
	  	  
	  	  if( (index+MY_DEFINE_BUFFER_SIZE) >= (*size) )
	  	  {
	  	  	 buffersize = (*size) - index;
	  	  }
	  	  	  	  
	  		status = handle->psys->RootFs->Read(handle->filehandle,&buffersize,g_read_seg_buffer);        
        if (EFI_ERROR(status)) 
        {
            (*size) = index;     
            MyLog(LOG_ERROR,L"SegFileRead failed error 0x%x.",status);
            return FALSE;
        } 
	  	  
 	  	  CopyMem(buffer+index,g_read_seg_buffer,buffersize);

	  	  index += buffersize;
	  }
	  
	  return TRUE;	  
	  
}

BOOL _FileRead(PFILE_HANDLE_INFO handle, OUT UINTN * size, PBYTE buffer)
{
    EFI_STATUS status;
    if (handle->psys->ifntfs) {
#ifdef NTFSLIB
		UINT64 total = *size;
        UINT64 readsize = *size;
        DWORD status;
        if (handle->ifdir) {
			DIR_LIST * listbuffer = NULL;
            EFI_FILE_INFO *fileinfo = (EFI_FILE_INFO *)buffer;

           	listbuffer = (DIR_LIST *)handle->cache;
			if(!handle->cachedatasize)
			{
	            char asciifilename[256] = { 0 };
	            UINT64 buffersize = DEFAULT_FILE_CACHE_SIZE;
	            
            	Unicode2Ascii(asciifilename,handle->filename);
            	status = n_listdir(asciifilename, &buffersize, listbuffer);
            	if (status!=N_SUCCESS) {
#if OKR_DEBUG
                	MyLog(LOG_DEBUG,L"list dir error %x",status);
#endif
					*size=0;
            	    return FALSE;
            	}
            	
            	handle->cachedatasize = buffersize;
            }

            if (listbuffer->count <= handle->filepos) {//end
                *size=0;
                return TRUE;
            }
            fileinfo->FileSize = listbuffer->entry[handle->filepos].filesize;
            Ascii2Unicode(fileinfo->FileName, listbuffer->entry[handle->filepos].name);
            *size = sizeof(EFI_FILE_INFO) + StrLength(fileinfo->FileName);

            handle->filepos++;
            return TRUE;

        } else {
			UINT64 offset = 0;
			UINT64 count = 0;
			
			*size = 0;
			while(total)
			{
				if(handle->filepos & 511)
				{
					//MyLog(LOG_DEBUG,L"list filepos %x",handle->filepos);
					if(handle->cachedatapos > handle->filepos 
						|| handle->cachedatapos + handle->cachedatasize < handle->filepos)
					{
						//不在cache中，读取文件到cache里
						handle->cachedatapos = (handle->filepos & ~0xfff);	//对齐4K

  	        MyLog(LOG_DEBUG,L"n_readfile - 1");

						status = n_readfile(handle->filehandle, handle->cachedatapos, 
									handle->cachesize, handle->cache, &count);
		    	        if (status != N_SUCCESS|| count == 0xFFFFFFFFFFFFFFFF) {
        			        MyLog(LOG_DEBUG,L"ReadFile failed with error %x",status);
		    	            return FALSE;
		    	        }
        				  	  	        MyLog(LOG_DEBUG,L"n_readfile - 1 - over");

						handle->cachedatasize = count;
		    	        if(count == 0)
		    	        	break;
						GoProcessBar(count);
					}
				}
				
				if(handle->cachedatapos <= handle->filepos 
						&& handle->cachedatapos + handle->cachedatasize > handle->filepos)
				{
					offset = handle->filepos - handle->cachedatapos;
					count = handle->cachedatasize - offset;
					count = (total > count) ? count : total;
					CopyMem(buffer, handle->cache + offset, count);
					*size += count;
					buffer += count;
					handle->filepos += count;
					total -= count;
				}
				
				if(total)
				{
       				readsize = (total & ~(handle->cachesize - 1));	//因为cache大小最大64K，所以可以对齐64K
       				while(readsize)
       				{
       					if(readsize > 16*1024*1024)
       						count = 16*1024*1024;
       					else
       						count = readsize;
       						

        				status = n_readfile(handle->filehandle, handle->filepos, count, buffer, &count);
		    	        if ( status!=N_SUCCESS || count == 0xFFFFFFFFFFFFFFFF ) {
        			        MyLog(LOG_ERROR,L"ReadFile2 failed with error %x",status);
		    	            return FALSE;
		    	        }
		    	         
     
		    	        if(count == 0)
		    	        {
		    	        	total = 0;
		    	        	break;
		    	        }
        
       					GoProcessBar(count);
      
  				
        				buffer += count;
	    			    handle->filepos += count;
        				readsize -= count;
    				    total -= count;
        				*size += count;
        			}
        			
        			if(total)
        			{
        				

						status = n_readfile(handle->filehandle, handle->filepos, 
									handle->cachesize, handle->cache, &count);
		    	        if (status!=N_SUCCESS|| count == 0xFFFFFFFFFFFFFFFF) {
        			        MyLog(LOG_DEBUG,L"ReadFile3 failed with error %x",status);
		    	            return FALSE;
		    	        }

						handle->cachedatasize = count;
						handle->cachedatapos = handle->filepos;
						//handle->filepos += total;, 此时数据还没有拷贝到buffer中，所以filepos不能改，会再次循环，从而修正filepos
		    	        if(count == 0)
		    	        	break;
						GoProcessBar(count);
        			}
        		}
        	}
            return TRUE;
        }
#else
        return FALSE;
#endif
    } else {

    
      	return SegFileRead(handle,size,buffer);
/*
        status = handle->psys->RootFs->Read(handle->filehandle,size,buffer);

        if (EFI_ERROR(status)) {
            MyLog(LOG_DEBUG,L"read file error %x",status);
            return FALSE;
        } else
            return TRUE;
*/   
    }
}


BOOL SegFileWrite( PFILE_HANDLE_INFO handle,OUT UINTN * size, PBYTE buffer )
{
	  //MY_DEFINE_BUFFER_SIZE
	  UINTN index = 0;
	  UINTN buffersize = 0;
	  EFI_STATUS status;
 
	  if( (*size) < MY_DEFINE_BUFFER_SIZE )
	  {
	  	  status = handle->psys->RootFs->Write(handle->filehandle,size,buffer);        
        if (EFI_ERROR(status)) 
        {
            MyLog(LOG_ERROR,L"SegFileWrite failed with error 0x%x.",status);
            return FALSE;
        } else
            return TRUE;
	  }
	  
	  for(index=0;index < (*size);)
	  {
	  	  buffersize = MY_DEFINE_BUFFER_SIZE;
	  	  
	  	  if( (index+MY_DEFINE_BUFFER_SIZE) >= (*size) )
	  	  {
	  	  	 buffersize = (*size) - index;
	  	  }
	  	  
	  	  CopyMem(g_seg_buffer,buffer+index,buffersize);
	  	  
	  		status = handle->psys->RootFs->Write(handle->filehandle,&buffersize,g_seg_buffer);        
        if (EFI_ERROR(status)) 
        {
            (*size) = index;     
            MyLog(LOG_ERROR,L"SegFileWrite failed error 0x%x.",status);
            return FALSE;
        } 
	  	  index += buffersize;
	  }
	  
	  return TRUE;	  
	  
}

BOOL _FileWrite(PFILE_HANDLE_INFO handle,OUT UINTN * size, PBYTE buffer)
{
    EFI_STATUS status;
    if (handle->psys->ifntfs) {
#ifdef NTFSLIB
        UINT64 total = *size;
        UINT64 writesize = total;
        UINT64 count;
        DWORD status;
        
        if (handle->ifdir)
            return FALSE;
		*size = 0;
        if(handle->cachedatasize)
        {
			//如果当前文件指针超出cache范围，先将当前cache中的数据写入文件中
			if(handle->filepos != handle->cachedatapos + handle->cachedatasize) {


				status = n_writefile(handle->filehandle,handle->cachedatapos, handle->cachedatasize, handle->cache, &writesize);
	    	if (status!=N_SUCCESS|| writesize == 0xFFFFFFFFFFFFFFFF) 
	      {
    	      MyLog(LOG_ERROR,L"_FileWrite0 failed with error %x",status);
        		return FALSE;
        }
            	
				handle->cachedatasize = 0;
				handle->cachedatapos = handle->filepos;
				writesize = total;
			}

        	if(writesize > handle->cachesize - handle->cachedatasize)
        		writesize = handle->cachesize - handle->cachedatasize;
        	CopyMem(handle->cache + handle->cachedatasize, buffer, writesize);
        	handle->cachedatasize += writesize;
       		total -= writesize;
       		buffer += writesize;
       		*size += writesize;
        	if(handle->cachedatasize == handle->cachesize)
        	{

        		status = n_writefile(handle->filehandle, handle->cachedatapos, handle->cachedatasize, handle->cache, &writesize);
	    	    if (status!=N_SUCCESS|| writesize == 0xFFFFFFFFFFFFFFFF) {
    	    	    MyLog(LOG_ERROR,L"_FileWrite failed with error %x",status);
            		return FALSE;
            	}

    				handle->cachedatapos += handle->cachedatasize;
        		handle->filepos = handle->cachedatapos;
        		handle->cachedatasize = 0;
        		handle->cachedirty = FALSE;
        	}
        	else
        	{
        		handle->cachedirty = TRUE;
				handle->filepos += writesize;
        		return TRUE;
        	}
        }
        
       	writesize = (total & ~(handle->cachesize - 1));	//因为cache大小最大64K，所以可以对齐64K
       	while(writesize)
       	{
       		if(writesize > 16*1024*1024)
       			count = 16*1024*1024;
       		else
       			count = writesize;


        	status = n_writefile(handle->filehandle, handle->filepos, count, buffer, &count);
	    	if (status!=N_SUCCESS|| count == 0xFFFFFFFFFFFFFFFF) {
    	        MyLog(LOG_ERROR,L"_FileWrite2 failed with error %x",status);
            	return FALSE;
            }

        	writesize -= count;
       		GoProcessBar(count);
        	
        	buffer += count;
	        handle->filepos += count;
    	    total -= count;
        	*size += count;
        }

        if(total)
        {
        	CopyMem(handle->cache, buffer, total);
        	handle->cachedatasize = total;
			handle->cachedatapos = handle->filepos;
			handle->filepos += total;
       		handle->cachedirty = TRUE;
	        *size += total;
        }
		return TRUE;
#else
        return FALSE;
#endif
    } else {
    	
    	
    	return SegFileWrite(handle,size,buffer);
//        status = handle->psys->RootFs->Write(handle->filehandle,size,buffer);        
//        if (EFI_ERROR(status)) 
//        {
//            MyLog(LOG_ERROR,L"_FileWrite3 failed with error 0x%x.",status);
//            return FALSE;
//        } else
//            return TRUE;
    }
}

//当遇到文件末尾时，可能出现size不等于初始值，或为0的情况(分段文件可能不等，但不会0)
//其余都会相同。或返回FALSE
BOOL FileRead_kang_del(PFILE_HANDLE_INFO handle,OUT UINTN * size, PBYTE buffer)
{
    EFI_STATUS status;
    UINTN readsize=*size;
    if (handle->ifeof) {
        *size=0;
        return TRUE;
    }
    if (_FileRead(handle,&readsize,buffer)==FALSE)
        return FALSE;

    if (handle->ifneedseg&&(readsize==0)) {
		//read next file
        CHAR16 namebuffer[100] = { 0 };
        handle->currentfileseg++;
        SPrint(namebuffer,200,L"%s.%d",handle->filename,(UINTN)handle->currentfileseg);
        FileClose(handle);
#if OKR_DEBUG
        MyLog(LOG_DEBUG,L"read next file %s\n",namebuffer);
#endif
        if (_FileOpen(handle,namebuffer,EFI_FILE_MODE_READ,0)==FALSE) {
            handle->ifeof=TRUE;
            *size=0;
            return TRUE;
        }
        readsize=*size;
        if (_FileRead(handle,&readsize,buffer)==FALSE)
            return FALSE;
        *size=readsize;
    } else {
        *size=readsize;
    }
    return TRUE;

}
DWORD	OldStructSize = 0;
int OldImage = 0;			////备份文件是否为旧版本。1：旧版本（0x09000810，AppVer: 9.2.0）。 0：新版本（0x09000811）。
BOOL FileRead(PFILE_HANDLE_INFO handle,OUT UINTN * size, PBYTE buffer)
{
	EFI_STATUS status;
	UINTN readsize = *size;
	PBYTE Buff = NULL;
	OKR_HEADER_810 *OldHeader = NULL;
	OKR_HEADER *NewHeader = NULL;
    if (handle->ifeof) {
        *size=0;
        return TRUE;
    }
    if (_FileRead(handle,&readsize,buffer)==FALSE)
        return FALSE;

    if (handle->ifneedseg&&(readsize==0)) {
		//read next file
        CHAR16 namebuffer[100] = { 0 };
        handle->currentfileseg++;
        SPrint(namebuffer,200,L"%s.%d",handle->filename,(UINTN)handle->currentfileseg);
        FileClose(handle);
#if OKR_DEBUG
        MyLog(LOG_DEBUG,L"read next file %s\n",namebuffer);
#endif
        if (_FileOpen(handle,namebuffer,EFI_FILE_MODE_READ,0)==FALSE) {
            handle->ifeof=TRUE;
            *size=0;
            return TRUE;
        }
        readsize=*size;
        if (_FileRead(handle,&readsize,buffer)==FALSE)
            return FALSE;
        *size=readsize;
    } else {
        *size=readsize;
    }
	OldStructSize = 0;
	OldImage =0;
	//kang+++++++>
	
	/*
typedef struct OKR_HEADER {     //old

    DWORD	Magic;				//0, 'okr9'
    DWORD	Version;			//4, 
    DWORD	StructSize;			//8, 本结构大小，包括可变长度的分区信息
    DWORD	Time;				//0xC
    BYTE	Memo[240];			//0x10
	BYTE	bOkrPE;				//0x100, 是否PE下的备份
    BYTE	reserved[3];		//0x101
	DWORD	TimeZone;			//0x104, 时区，多少秒
    UINT64	FileSize;			//0x108, 所有备份文件大小（如果没有切分，就等于本文件大小；否则是所有切分的文件大小之和），字节数
    UINT64	Disksize;			//0x110, 备份的硬盘大小
//kang--	BYTE    SourceDiskGUID[16];
//kang--	BYTE	WinPartNtfsGuid[16];		//0x180 12 个扇区
//kang--	BYTE	Reserved_2[1024];		//0x180 12 个扇区
    WORD	PartitionIndex;		//0x118, 备份文件所在分区的index，runtime用
    WORD	PartitionCount;		//0x11A, 备份时检测到的有效分区个数，即本结构中可变长度成员变量Partition的个数
    DWORD	GptSize;			//0x11C, GPT分区表备份区域大小, 0-34扇区，IN BYTE
    BYTE	bGpt;				//0x120
    BYTE	bComplete;			//0x121, 备份结束后值1
    BYTE	bCompress;			//0x122, 数据区是否压缩
    BYTE	bFactoryBackup;		//0x123, 是否工厂备份, 必然在服务分区里，如果没有工厂备份，则第一次备份为工厂备份，工厂备份不可被删除
    DWORD	CompressBlockSize;	//0x124, 每100M压缩一次，必须被缓冲区大小整除
    UINT64	OriginalSize;		//0x128 - 0x130, 原始备份数据区字节数，备份后生成精确,压缩前的大小
	DWORD	DataOffset;			//0x130,	备份数据偏移地址，以字节为单位
	DWORD	FileNumber;			//0x134, 备份的镜像文件的个数
	ULONGLONG FileTime;			//0x138, 文件的日期
    BYTE	Reserved[0x40];		//0x140
	BYTE	FileName[0x80];		//0x180
//kang--	BYTE	TempBuffer[6144];		//0x180 12 个扇区
    WIN32_PARTITIONINFO Partition[1];	//0x200 + 0xb0*64
}OKR_HEADER, *POKR_HEADER;	


typedef struct OKR_HEADER {

    DWORD	Magic;				//0, 'okr9'
    DWORD	Version;			//4, 
    DWORD	StructSize;			//8, 本结构大小，包括可变长度的分区信息
    DWORD	Time;				//0xC
    BYTE	Memo[240];			//0x10
	BYTE	bOkrPE;				//0x100, 是否PE下的备份
    BYTE	reserved[3];		//0x101
	DWORD	TimeZone;			//0x104, 时区，多少秒
    UINT64	FileSize;			//0x108, 所有备份文件大小（如果没有切分，就等于本文件大小；否则是所有切分的文件大小之和），字节数
    UINT64	Disksize;			//0x110, 备份的硬盘大小
	BYTE    SourceDiskGUID[16];
	BYTE	WinPartNtfsGuid[16];		//0x180 12 个扇区
	BYTE	Reserved_2[1024];		//0x180 12 个扇区
    WORD	PartitionIndex;		//0x118, 备份文件所在分区的index，runtime用
    WORD	PartitionCount;		//0x11A, 备份时检测到的有效分区个数，即本结构中可变长度成员变量Partition的个数
    DWORD	GptSize;			//0x11C, GPT分区表备份区域大小, 0-34扇区，IN BYTE
    BYTE	bGpt;				//0x120
    BYTE	bComplete;			//0x121, 备份结束后值1
    BYTE	bCompress;			//0x122, 数据区是否压缩
    BYTE	bFactoryBackup;		//0x123, 是否工厂备份, 必然在服务分区里，如果没有工厂备份，则第一次备份为工厂备份，工厂备份不可被删除
    DWORD	CompressBlockSize;	//0x124, 每100M压缩一次，必须被缓冲区大小整除
    UINT64	OriginalSize;		//0x128 - 0x130, 原始备份数据区字节数，备份后生成精确,压缩前的大小
	DWORD	DataOffset;			//0x130,	备份数据偏移地址，以字节为单位
	DWORD	FileNumber;			//0x134, 备份的镜像文件的个数
	ULONGLONG FileTime;			//0x138, 文件的日期
    BYTE	Reserved[0x40];		//0x140
	BYTE	FileName[0x80];		//0x180
	BYTE	TempBuffer[6144];		//0x180 12 个扇区
    WIN32_PARTITIONINFO Partition[1];	//0x200 + 0xb0*64
}OKR_HEADER, *POKR_HEADER;



	*/
	if(((OKR_HEADER_810 *) buffer)->Magic == OKR_IMAGE_MAGIC &&  ((OKR_HEADER_810 *)buffer)->Version == 0x09000810 )
	{
		OldImage = 1;
		
		
		 Buff=AllocateZeroPool(OKR_HEADER_MAX_SIZE);
		 OldHeader =(OKR_HEADER_810 *) buffer;
		 NewHeader = (OKR_HEADER *)Buff ;
		NewHeader->Magic = OldHeader->Magic;				//0, 'okr9'
    NewHeader->Version 		= OKR_VERSION    ;//4, 
    NewHeader->StructSize 	= ((FIELD_OFFSET(OKR_HEADER, Partition) + OldHeader->PartitionCount * sizeof(WIN32_PARTITIONINFO) + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1)); 	//8, 本结构大小，包括可变长度的分区信息
  
	 OldStructSize = OldHeader->StructSize;
		     MyLog(LOG_DEBUG,L"\r\nOld StructSize =%x \n",OldHeader->StructSize);
            MyLog(LOG_DEBUG,L"\r\nnew StructSize =%x \n",NewHeader->StructSize);
		//	gBS->Stall(0x100000);
	 
		
  NewHeader->Time 		= OldHeader->Time   ;	//0xC
    CopyMem(NewHeader->Memo, OldHeader->Memo, 240); //  BYTE	Memo[240];			//0x10
	NewHeader->bOkrPE       = OldHeader->bOkrPE;		//0x100, 是否PE下的备份
    CopyMem(NewHeader->reserved, OldHeader->reserved, 3);//BYTE	reserved[3];		//0x101
	NewHeader->TimeZone     = OldHeader->TimeZone  ;//DWORD	TimeZone;			//0x104, 时区，多少秒
    NewHeader->FileSize	 = OldHeader->FileSize  ;		//0x108, 所有备份文件大小（如果没有切分，就等于本文件大小；否则是所有切分的文件大小之和），字节数
     NewHeader->Disksize	 = OldHeader->Disksize  ;	//0x110, 备份的硬盘大小
//	BYTE    SourceDiskGUID[16];
//	BYTE	WinPartNtfsGuid[16];		//0x180 12 个扇区
//	BYTE	Reserved_2[1024];		//0x180 12 个扇区
    NewHeader->PartitionIndex =   OldHeader->PartitionIndex ;		//0x118, 备份文件所在分区的index，runtime用
    NewHeader->PartitionCount =	OldHeader->PartitionCount;		//0x11A, 备份时检测到的有效分区个数，即本结构中可变长度成员变量Partition的个数
    NewHeader->GptSize =		OldHeader->GptSize;			//0x11C, GPT分区表备份区域大小, 0-34扇区，IN BYTE
    NewHeader->bGpt 	= OldHeader->bGpt    ;			//0x120
    NewHeader->bComplete  = OldHeader->bComplete    ;			//0x121, 备份结束后值1
    NewHeader->bCompress  = OldHeader->bCompress    ;			//0x122, 数据区是否压缩
    NewHeader->bFactoryBackup  = OldHeader->bFactoryBackup    ;		//0x123, 是否工厂备份, 必然在服务分区里，如果没有工厂备份，则第一次备份为工厂备份，工厂备份不可被删除
    NewHeader->CompressBlockSize  = OldHeader->CompressBlockSize    ;	//0x124, 每100M压缩一次，必须被缓冲区大小整除
    NewHeader->OriginalSize  = OldHeader->OriginalSize    ;		//0x128 - 0x130, 原始备份数据区字节数，备份后生成精确,压缩前的大小
	NewHeader->DataOffset  = OldHeader->DataOffset    ;			//0x130,	备份数据偏移地址，以字节为单位
	NewHeader->FileNumber  = OldHeader->FileNumber    ;			//0x134, 备份的镜像文件的个数
	NewHeader->FileTime  = OldHeader->FileTime    ;			//0x138, 文件的日期
     CopyMem(NewHeader->Reserved, OldHeader->Reserved, 0x40);// Reserved[0x40];		//0x140
	 CopyMem(NewHeader->FileName, OldHeader->FileName, 0x80); //NewHeader->FileName[0x80];		//0x180
//	BYTE	TempBuffer[6144];		//0x180 12 个扇区
 
	for(int i=0 ; i< NewHeader->PartitionCount ; i++)  // WIN32_PARTITIONINFO Partition[1];	//0x200 + 0xb0*64
	{
		CopyMem(&(NewHeader->Partition[i]), &(OldHeader->Partition[i]), sizeof(WIN32_PARTITIONINFO));
	}
	
	CopyMem(buffer, Buff, *size);
	if(Buff)
	FreePool(Buff);
	MyLog(LOG_DEBUG,L"\r\n OKR_HEADER_MAX_SIZE   =%x \n",OKR_HEADER_MAX_SIZE);
	MyLog(LOG_DEBUG,L"\r\n copy Size =%x \n",*size);
	
	}
	
	
	
	//kang+++++++<
	
	
    return TRUE;

}

//一次写完才会分段
BOOL FileWrite(PFILE_HANDLE_INFO handle, UINTN size, PBYTE buffer)
{
    UINTN writesize=size;
    EFI_STATUS status;
    if (_FileWrite(handle,&writesize,buffer)==FALSE)
    {
        //  MyLog(LOG_DEBUG,L"FileWrite Error\n");   	
    	    return FALSE;
    }
    if (writesize!=size)
    {
       //   MyLog(LOG_DEBUG,L"FileWrite Error writesize:%lx,%lx\n",writesize,size);   	    	
    	    return FALSE;
    }
    handle->currentfilesize+=writesize;
    if (handle->ifneedseg) {
        //MyLog(LOG_DEBUG,L"write %d %d",SPLIT_SIZE,handle->currentfilesize);
        if (handle->currentfilesize > SPLIT_SIZE) {//new file
            CHAR16 name[200] = { 0 };

            FileClose(handle);
            handle->currentfilesize=0;
            handle->currentfileseg++;
            SPrint(name,200,L"%s.%d",handle->filename,(UINTN)handle->currentfileseg);
           // MyLog(LOG_DEBUG,L"create next file %s\n",name);
            if (_FileOpen(handle,name,EFI_FILE_MODE_CREATE|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ,0)==FALSE)
                return FALSE;

        }
    }
    return TRUE;
}

BOOL FileClose(PFILE_HANDLE_INFO handle)
{
	if (NULL == handle)
	{
		return TRUE;
	}
	EFI_STATUS status;
	if (handle->psys->ifntfs) {
#ifdef NTFSLIB
		if (handle->filehandle) {
			if(handle->cachedirty && handle->cachedatasize)
			{
				UINT64 writesize = 0;
				status = n_writefile(handle->filehandle,handle->cachedatapos, handle->cachedatasize, handle->cache, &writesize);
			}
			handle->cachedirty = FALSE;

			n_closefile(handle->filehandle);
			handle->filehandle = NULL;
#endif
		} 

		handle->cachesize = 0;
		handle->cachedatasize = 0;
		if(handle->cache)
			myfree(handle->cache);		//分配64K大小的cache
		handle->cache = NULL;
		handle->ifdir = 0;
		handle->filepos = 0;

	} else {
		if (handle->filehandle) {
			status = handle->psys->RootFs->Close(handle->filehandle);
			handle->filehandle = NULL;
		}
	}
	return TRUE;
}

BOOL FileSetPos(PFILE_HANDLE_INFO handle,UINT64 pos)
{
    EFI_STATUS status;
    handle->ifeof=FALSE;

    if (handle->psys->ifntfs) {
#ifdef NTFSLIB
        handle->filepos=pos;
        return TRUE;
#else
        return FALSE;
#endif
    } else {
        status=handle->psys->RootFs->SetPosition(handle->filehandle,pos);
        if (EFI_ERROR(status)) {
            MyLog(LOG_ERROR,L"file seek error %x\n",status);
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

//////////////////// 双进程缓冲区管理，乒乓缓冲区原理
//两个缓冲区。一个用于不断读取原始数据，一个用于压缩/解压的原始数据。
//（压缩/解压后的数据存放，是属于压缩的独有缓冲）
BOOL Init_BufferArray(struct BUFFERARRAY *pbuffarray)
{
	PBYTE buffer = NULL;
	DWORD i;
	DWORD size = MAX_COMPRESS_BLOCK_SIZE;

    SetMem(pbuffarray, sizeof(struct BUFFERARRAY), 0);

	if(!g_data.MemoryPagesNumber)
		size = MIN_COMPRESS_BLOCK_SIZE;

	if(g_data.bMultiThread) {
		if(g_data.bAsyncIO) {
			pbuffarray->number = 4;
			size = MAX_COMPRESS_BLOCK_SIZE / 2;
		} else {
			pbuffarray->number = 2;
		}
	} else {
		pbuffarray->number = 1;
	}

	//如果是备份，则可以定义任意大小的block；如果是恢复，则block大小需要和备份保持一致
	if(g_data.Cmd == USER_COMMAND_RECOVER) {
		size = g_data.CompressBlockSize;
	}

	for(i=0;i<256;i++) {
		if(size * (pbuffarray->number+1) / 4096 < g_data.MemoryPagesNumber || !g_data.MemoryPagesNumber) {
			buffer = myalloc(size * pbuffarray->number + (pbuffarray->number*ALGIN_SIZE_BYTE));
			if(buffer)
				break;
		}
		if(g_data.Cmd == USER_COMMAND_RECOVER) {
			//恢复的时候size不可变化，只能改number个数
			pbuffarray->number /= 2;
			if(pbuffarray->number == 0)
				break;
		} else {
			//其他情况下可以改变size
			size -= MIN_COMPRESS_BLOCK_SIZE;
			if(size < MIN_COMPRESS_BLOCK_SIZE)
				break;
		}
	}
	if(!buffer)
		return FALSE;
	g_data.CompressBlockSize = size;
	if(pbuffarray->number == 1)
		g_data.bMultiThread = FALSE;

    pbuffarray->buffer[0].pointer = buffer;
    pbuffarray->buffer[0].totalsize = size/*+ALGIN_SIZE_BYTE*/;

	for(i=1;i<pbuffarray->number;i++)
	{
    	pbuffarray->buffer[i].pointer = buffer + size * i+ALGIN_SIZE_BYTE*i;
    	pbuffarray->buffer[i].totalsize = size;
  }

   return TRUE;
}

void Clean_BufferArray(struct BUFFERARRAY *pbuffarray)
{
	DWORD i;
	for(i=0;i<MAX_BUFFER_COUNT;i++) {
		pbuffarray->buffer[i].datavalid = FALSE;
		pbuffarray->buffer[i].usedsize= 0;
		pbuffarray->buffer[i].sequence= 0;
	}
	pbuffarray->producer = 0;
	pbuffarray->consumer = 0;
}

//获取生产者缓冲区
PBUFFER_INFO GetProducter(struct BUFFERARRAY *pbuffarray)
{
	PBUFFER_INFO buf;

	//获得生产者，其datavalid必须为FALSE
	if(g_data.bMultiThread) {
	    buf = &pbuffarray->buffer[pbuffarray->producer++];
		if(pbuffarray->producer >= pbuffarray->number)
			pbuffarray->producer = 0;
	}
	else
		buf = &pbuffarray->buffer[0];

	//生产者的buffer中必须是没有数据的
	while(buf->datavalid) {
		//说明没有空闲的buffer，需要等待写文件线程将缓冲区写到文件里，然后释放缓冲区
		if(g_data.bCompleted) {
			if(pbuffarray->producer == 0)
				pbuffarray->producer = pbuffarray->number - 1;
			else
				pbuffarray->producer--;
			return NULL;
		}
		CpuPause();
	}
	return buf;
}

//获取消费者缓冲区
PBUFFER_INFO GetConsumer(struct BUFFERARRAY *pbuffarray)
{
	PBUFFER_INFO buf;

	if(g_data.bMultiThread) {
		buf = &pbuffarray->buffer[pbuffarray->consumer++];
		if(pbuffarray->consumer >= pbuffarray->number)
			pbuffarray->consumer = 0;
	}
	else
		buf = &pbuffarray->buffer[0];
	
	//消费者的buffer中必须是有数据的，轮询等待
	while(!buf->datavalid) {
		if(g_data.bCompleted) {
			//如果用户强制退出或其他异常，则返回空，返回之前注意恢复consumer
			if(pbuffarray->consumer == 0)
				pbuffarray->consumer = pbuffarray->number - 1;
			else
				pbuffarray->consumer--;
			return NULL;
		}
		CpuPause();
	}
	return buf;
}

PBUFFER_INFO GetConsumerAnyway(struct BUFFERARRAY *pbuffarray)
{
	PBUFFER_INFO buf;

	if(g_data.bMultiThread) {
		buf = &pbuffarray->buffer[pbuffarray->consumer];
		if(buf->datavalid) {
			pbuffarray->consumer++;
			if(pbuffarray->consumer >= pbuffarray->number)
				pbuffarray->consumer = 0;
		}
	}
	else
		buf = &pbuffarray->buffer[0];
	return buf;
}

void Destroy_BufferArray(struct BUFFERARRAY *pbuffarray)
{
    if (pbuffarray->buffer[0].pointer)
    	myfree(pbuffarray->buffer[0].pointer);
    mymemset(pbuffarray, 0, sizeof(struct BUFFERARRAY));
}

BOOL InitMp(struct MP_INFO *mpinfo)
{
    INTN NumProc,NumEnabled,i;
    EFI_PROCESSOR_INFORMATION CpuInfo;
    INTN BSP=-1,BSP_Core;
    EFI_STATUS Status;

    SetMem(mpinfo,sizeof(struct MP_INFO),0);
    mpinfo->AP = -1;
    g_data.bMultiThread = FALSE;
    
    if(1)
    {
    	return FALSE;
    }
    Status = gBS->LocateProtocol( &gEfiMpServiceProtocolGuid, NULL, &mpinfo->MpProto);
    if (EFI_ERROR(Status)) {
        MyLog(LOG_DEBUG, L"Unable to locate the Mp procotol: %d\n", Status);
        //getch();
        return FALSE;
    }
    
	// Get Number of Processors and Number of Enabled Processors
    Status = mpinfo->MpProto->GetNumberOfProcessors( mpinfo->MpProto, &NumProc, &NumEnabled);
    if (EFI_ERROR(Status)) {
		MyLog(LOG_DEBUG, L"GetNumberOfProcessors failed with error %d\n", Status);
        return FALSE;
    }

  	MyLog(LOG_DEBUG,L"CPU number: %d %d\n", NumProc, NumEnabled);

	// Get Processor Health and Location information
    for (i=0;i<NumProc;i++) {

		MyLog(LOG_DEBUG,L"GetProcessorInfo(%d)...\n", i);

        Status = mpinfo->MpProto->GetProcessorInfo( mpinfo->MpProto, i, &CpuInfo);
        if (EFI_ERROR(Status)) {
		    MyLog(LOG_DEBUG,L"GetProcessorInfo failed with error: 0x%x\n", Status);
            continue;
        }
        if (CpuInfo.StatusFlag==7) {
            BSP=i;
            BSP_Core=CpuInfo.Location.Core;
        }
        if ((BSP!=-1)&&(CpuInfo.StatusFlag==6)&&(CpuInfo.Location.Core!=BSP_Core)) {//find next core
            mpinfo->AP=i;
            break;
        }
        //	Print(L"GET CPU %d : id=%x ,StatusFlag=%x , package=%d, core=%d,thread=%d\n",i,Tcb.ProcessorId,Tcb.StatusFlag,Tcb.Location.Package,Tcb.Location.Core,Tcb.Location.Thread);
    }
    if (mpinfo->AP==-1) {
        MyLog(LOG_DEBUG, L"Can't find available cpu.");
		//if (NumProc > 1)
		//{
		//	MyLog(LOG_ERROR,L"Force cpu to %d\n", NumProc - 1);
		//	mpinfo->AP = NumProc - 1;
		//    getch();
		//	return TRUE;
		//}
		if(g_data.bDebug)
			getch();
        return FALSE;
    }
#if OKR_DEBUG
    MyLog(LOG_DEBUG,L"find available cpu %d.",mpinfo->AP);
#endif
	g_data.bMultiThread = TRUE;

    return TRUE;
}

BOOL StartAP(struct MP_INFO *mpinfo,EFI_AP_PROCEDURE  Procedure, VOID  *ProcedureArgument)
{
    EFI_STATUS Status;
    if (mpinfo->status == 1)
        return FALSE;
    Status = gBS->CreateEvent( 0, TPL_NOTIFY, NULL, NULL, &mpinfo->ap_event);
    if (EFI_ERROR(Status)) {
        return FALSE;
    }
    Status = mpinfo->MpProto->StartupThisAP( mpinfo->MpProto, Procedure, mpinfo->AP, mpinfo->ap_event, 0, ProcedureArgument, NULL);
    if (EFI_ERROR(Status)) {
        return FALSE;
    }
    mpinfo->status = 1;
    return TRUE;
}

BOOL WaitToComplete(struct MP_INFO *mpinfo)
{
    UINTN eventindex;
    if (mpinfo->status != 1)
        return FALSE;
    gBS->WaitForEvent(1,&mpinfo->ap_event,&eventindex);
    gBS->CloseEvent(mpinfo->ap_event);
    mpinfo->status=0;
    return TRUE;
}

#if OKR_EVENT
void WaitForConsumer()
{
	UINTN eventindex;
	gBS->WaitForEvent(1, &g_data.cosumer_event, &eventindex);
}

void SignalConsumer()
{
	//consumer是等候在producter_event上的，所以是唤醒事件producter_event
	gBS->SignalEvent(g_data.producter_event);
}

void WaitForProducter()
{
	UINTN eventindex;
	gBS->WaitForEvent(1, &g_data.producter_event, &eventindex);
}

void SignalProducter()
{
	//producter是等候在cosumer_event上的，所以是唤醒事件cosumer_event
	gBS->SignalEvent(g_data.cosumer_event);
}
#endif

CHAR8 *
Unicode2Ascii (
    OUT CHAR8          *AsciiStr,
    IN  CHAR16         *UnicodeStr
)
{
    CHAR8 *Str;

    Str = AsciiStr;

    while (TRUE) {

        *AsciiStr = (CHAR8) *(UnicodeStr++);

        if (*(AsciiStr++) == '\0') {
            return Str;
        }
    }
}


void  mymemset(void *pTarget, UINT8 value, UINTN cbTarget)
{
    SetMem( pTarget, (UINTN)cbTarget, (UINT8)value );
    return ;
}

#ifdef DEFMEMSET
/*
#pragma function(mymemset)

void * __cdecl mymemset(void *pTarget, int value, int cbTarget)
{
    SetMem( pTarget, (UINTN)cbTarget, (UINT8)value );
    return NULL;
}

#pragma function(memcpy)
void *__cdecl memcpy( void *dest, const void *src, int count )
{
    CopyMem(dest, src, (UINTN)count);
    return NULL;
}
*/
#endif

PBYTE Decompress(IN VOID *Source, IN UINT32 SourceSize, 
		IN OUT VOID *Destination, IN UINT32 *lpdwDestinationSize
		)
{
    EFI_STATUS	status = EFI_SUCCESS;
    VOID *scratch;
    UINT32 scratchSize;
    UINT32 destinationSize;
    VOID *dest = NULL;
    
    if(!g_data.Decompress)
    	return NULL;
    
	if(Source && SourceSize)
	{
		status = g_data.Decompress->GetInfo(g_data.Decompress, Source, SourceSize, 
					&destinationSize, &scratchSize);
		if (EFI_ERROR(status) || destinationSize > *lpdwDestinationSize)
		{
			MyLog(LOG_ERROR,L"GetInfo(%p, %d) failed with error 0x%x, destsize 0x%x, *destsize 0x%x, buf %x,%x\n",
				Source, SourceSize, status, destinationSize, *lpdwDestinationSize,
				*(DWORD*)Source, *(DWORD*)((BYTE*)Source+4));
			return NULL;
		}
		
		if(!g_data.Scratch || scratchSize > g_data.ScratchSize)
		{
			if(g_data.Scratch)
				myfree(g_data.Scratch);
			g_data.Scratch = myalloc(scratchSize);
			g_data.ScratchSize = scratchSize;
			if(!g_data.Scratch)
			{
				MyLog(LOG_ERROR,L"failed to alloc Scratch buffer\n");
				return NULL;
			}
		}
		scratch = g_data.Scratch;
		
		if(destinationSize > *lpdwDestinationSize)
		{
			//需要分配内存
			dest = myalloc(destinationSize);
			if(!dest)
				return NULL;
		}
		else
			dest = Destination;

		status = g_data.Decompress->Decompress(g_data.Decompress, Source, SourceSize, dest, destinationSize,
			scratch, scratchSize);
		if (!EFI_ERROR(status))
		{
			*lpdwDestinationSize = destinationSize;
		}
		else
		{
			if(dest != Destination)
				myfree(dest);
			return NULL;
		}
	}
	
	return dest;
}



UINTN StrLength(IN CHAR16 * Str)
{
	UINTN	i = 0;
	for(i=0;*Str++ != L'\0';i++);
	return i;
}
EFI_STATUS StrCopy(OUT CHAR16 * Dest,IN CHAR16 * Source , UINTN Length)
{
	if( StrLength(Source) > Length)
		return EFI_BUFFER_TOO_SMALL;
	while( L'\0' != (*Dest ++ = * Source++) );
	return EFI_SUCCESS;
}


//fill ntfshandle or filehandle
BOOL _FileOpenA(PFILE_HANDLE_INFO Handle, char *FileName, UINT64 OpenMode, UINT64 FileAttribute)
{
    BOOL bRet = TRUE;

    if (Handle->psys->ifntfs) {
#ifdef NTFSLIB
        DWORD status;
        
        Handle->cachesize = DEFAULT_FILE_CACHE_SIZE;
        Handle->cachedatasize = 0;
        Handle->cachedatapos = 0;
        Handle->cachedirty = FALSE;
        Handle->cache = myalloc(Handle->cachesize);     //分配1MB大小的cache
        
        if (FileAttribute & EFI_FILE_DIRECTORY) {//dir
            if (EFI_FILE_MODE_CREATE & OpenMode){
                status = n_createdir(FileName);
                if ((status != N_FILE_EXISTS) && (status != N_SUCCESS)){

                    MyLog(LOG_DEBUG,L"ntfs_create_dir failed with error 0x%x",status);
                    bRet = FALSE;
                }
            }
            Handle->ifdir = 1;
        } else {//nomal file
            status = n_openfile(FileName, &Handle->filehandle);
            if ((status == N_FILE_NOT_FOUND)&&(OpenMode&EFI_FILE_MODE_CREATE)) {
                if (n_createfile(FileName, &Handle->filehandle)!=N_SUCCESS) {

                    MyLog(LOG_DEBUG,L"ntfs_create_file failed.");
                    bRet = FALSE;
                }
            } else if (status != N_SUCCESS) {
                MyLog(LOG_DEBUG,L"ntfs_open_file failed with error 0x%x",status);
                bRet = FALSE;
            }
        }
#else
        bRet = FALSE;
#endif
    } 
    /*else {
        EFI_STATUS status;
        EFI_FILE_PROTOCOL * RootFs=Handle->psys->RootFs;

        status = RootFs->Open(RootFs, &Handle->filehandle, FileName, OpenMode, FileAttribute);
        if (EFI_ERROR(status)) {
            MyLog(LOG_DEBUG, L"OpenFile failed with error 0x%x", status);
            bRet = FALSE;
        }
    }*/

    return bRet;
}


//support directry,create file or dir.attribute use EFI system define
//ifneedseg, true=support file segments
BOOL FileOpenA(IN PFILE_SYS_INFO info,OUT PFILE_HANDLE_INFO handle,char *filename,UINT64 openmode,UINT64 attribute,BOOL ifneedseg)
{
    EFI_STATUS status;

    SetMem(handle,sizeof(FILE_HANDLE_INFO),0);
    handle->psys=info;
    
    if (handle->psys->ifntfs)
        handle->ifneedseg=FALSE;
    else
        handle->ifneedseg=ifneedseg;

   // StrnCpy(handle->filename,filename,sizeof(handle->filename)/sizeof(CHAR16));

    return _FileOpenA(handle,filename,openmode,attribute);
}

int Upper(CHAR8   *String)
{
	int i = 0;
	if (NULL != String)
	{
		for (i = 0; *String != '\0'; i++)
		{
			if (*String >= 'a' && *String <= 'z')
			{
				*String -= 32;
			}
			*String++;
		}
	}

	return i;
}


BOOL NormalFileRead(PFILE_HANDLE_INFO handle, UINTN * size, PBYTE buffer)
{
	//MyLog(LOG_DEBUG, L"NormalFileRead...");

	EFI_STATUS status;

	if (handle->ifeof)
	{
		*size = 0;
		return TRUE;
	}

	if (handle->psys->ifntfs)
	{
		//MyLog(LOG_DEBUG, L"handle->psys->ifntfs...");

#ifdef NTFSLIB
		UINT64 total = *size;
		UINT64 readsize = *size;
		DWORD status;
		if (handle->ifdir)
		{
			//MyLog(LOG_DEBUG, L"handle->ifdir...");

			DIR_LIST * listbuffer = NULL;
			EFI_FILE_INFO *fileinfo = (EFI_FILE_INFO *)buffer;

			listbuffer = (DIR_LIST *)handle->cache;
			if (!handle->cachedatasize)
			{
				char asciifilename[256] = { 0 };
				UINT64 buffersize = DEFAULT_FILE_CACHE_SIZE;

				Unicode2Ascii(asciifilename, handle->filename);
				status = n_listdir(asciifilename, &buffersize, listbuffer);
				if (status != N_SUCCESS)
				{
#if OKR_DEBUG
					MyLog(LOG_DEBUG, L"list dir error %x", status);
#endif
					*size = 0;
					return FALSE;
				}

				handle->cachedatasize = buffersize;
			}

			if (listbuffer->count <= handle->filepos)
			{
				//end
				*size = 0;
				return TRUE;
			}
			fileinfo->FileSize = listbuffer->entry[handle->filepos].filesize;
			Ascii2Unicode(fileinfo->FileName, listbuffer->entry[handle->filepos].name);
			*size = sizeof(EFI_FILE_INFO) + StrLength(fileinfo->FileName);

			handle->filepos++;
			return TRUE;

		}
		else
		{
			//MyLog(LOG_DEBUG, L"No handle->ifdir...");

			UINT64 offset = 0;
			UINT64 count = 0;

			*size = 0;
			while (total)
			{
				if (handle->filepos & 511)
				{
					//MyLog(LOG_DEBUG, L"list filepos %x", handle->filepos);
					if (handle->cachedatapos > handle->filepos
						|| handle->cachedatapos + handle->cachedatasize < handle->filepos)
					{
						//不在cache中，读取文件到cache里
						handle->cachedatapos = (handle->filepos & ~0xfff);	//对齐4K

						//MyLog(LOG_DEBUG, L"n_readfile - 1");

						status = n_readfile(handle->filehandle, handle->cachedatapos,
							handle->cachesize, handle->cache, &count);
						if (status != N_SUCCESS || count == 0xFFFFFFFFFFFFFFFF)
						{
							MyLog(LOG_DEBUG, L"ReadFile failed with error %x", status);
							return FALSE;
						}
						//MyLog(LOG_DEBUG, L"n_readfile - 1 - over");

						handle->cachedatasize = count;
						if (count == 0)
							break;
						//GoProcessBar(count);
					}
				}

				if (handle->cachedatapos <= handle->filepos
					&& handle->cachedatapos + handle->cachedatasize > handle->filepos)
				{
					offset = handle->filepos - handle->cachedatapos;
					count = handle->cachedatasize - offset;
					count = (total > count) ? count : total;
					CopyMem(buffer, handle->cache + offset, count);
					*size += count;
					buffer += count;
					handle->filepos += count;
					total -= count;
				}

				if (total)
				{
					readsize = (total & ~(handle->cachesize - 1));	//因为cache大小最大64K，所以可以对齐64K
					while (readsize)
					{
						if (readsize > 16 * 1024 * 1024)
						{
							count = 16 * 1024 * 1024;
						}
						else
						{
							count = readsize;
						}

						status = n_readfile(handle->filehandle, handle->filepos, count, buffer, &count);
						if (status != N_SUCCESS || count == 0xFFFFFFFFFFFFFFFF)
						{
							MyLog(LOG_ERROR, L"ReadFile2 failed with error %x", status);
							return FALSE;
						}

						if (count == 0)
						{
							total = 0;
							break;
						}
						//GoProcessBar(count);

						buffer += count;
						handle->filepos += count;
						readsize -= count;
						total -= count;
						*size += count;
					}

					if (total)
					{
						status = n_readfile(handle->filehandle, handle->filepos,
							handle->cachesize, handle->cache, &count);
						if (status != N_SUCCESS || count == 0xFFFFFFFFFFFFFFFF)
						{
							MyLog(LOG_DEBUG, L"ReadFile3 failed with error %x", status);
							return FALSE;
						}

						handle->cachedatasize = count;
						handle->cachedatapos = handle->filepos;
						//handle->filepos += total;, 此时数据还没有拷贝到buffer中，所以filepos不能改，会再次循环，从而修正filepos
						if (count == 0)
						{
							break;
						}
						//GoProcessBar(count);
					}
				}
			}
			return TRUE;
		}
#else
		return FALSE;
#endif
	}
	else
	{
		return SegFileRead(handle, size, buffer);

		//status = handle->psys->RootFs->Read(handle->filehandle, size, buffer);
		//if (EFI_ERROR(status))
		//{
		//	MyLog(LOG_DEBUG, L"read file error %x", status);
		//	return FALSE;
		//}
		//else
		//{
		//	return TRUE;
		//}
	}

	return TRUE;
}

BOOL NormalFileWrite(PFILE_HANDLE_INFO handle, UINTN  *size, PBYTE buffer)
{
	EFI_STATUS status;
	if (handle->psys->ifntfs)
	{
#ifdef NTFSLIB
		UINT64 total = *size;
		UINT64 writesize = total;
		UINT64 count;
		DWORD status;

		if (handle->ifdir)
		{
			return FALSE;
		}
		*size = 0;
		if (handle->cachedatasize)
		{
			//如果当前文件指针超出cache范围，先将当前cache中的数据写入文件中
			if (handle->filepos != handle->cachedatapos + handle->cachedatasize)
			{
				status = n_writefile(handle->filehandle, handle->cachedatapos, handle->cachedatasize, handle->cache, &writesize);
				if (status != N_SUCCESS || writesize == 0xFFFFFFFFFFFFFFFF)
				{
					MyLog(LOG_ERROR, L"_FileWrite0 failed with error %x", status);
					return FALSE;
				}

				handle->cachedatasize = 0;
				handle->cachedatapos = handle->filepos;
				writesize = total;
			}

			if (writesize > handle->cachesize - handle->cachedatasize)
			{
				writesize = handle->cachesize - handle->cachedatasize;
			}
			CopyMem(handle->cache + handle->cachedatasize, buffer, writesize);
			handle->cachedatasize += writesize;
			total -= writesize;
			buffer += writesize;
			*size += writesize;
			if (handle->cachedatasize == handle->cachesize)
			{
				status = n_writefile(handle->filehandle, handle->cachedatapos, handle->cachedatasize, handle->cache, &writesize);
				if (status != N_SUCCESS || writesize == 0xFFFFFFFFFFFFFFFF)
				{
					MyLog(LOG_ERROR, L"_FileWrite failed with error %x", status);
					return FALSE;
				}

				handle->cachedatapos += handle->cachedatasize;
				handle->filepos = handle->cachedatapos;
				handle->cachedatasize = 0;
				handle->cachedirty = FALSE;
			}
			else
			{
				handle->cachedirty = TRUE;
				handle->filepos += writesize;
				return TRUE;
			}
		}

		writesize = (total & ~(handle->cachesize - 1));	//因为cache大小最大64K，所以可以对齐64K
		while (writesize)
		{
			if (writesize > 16 * 1024 * 1024)
			{
				count = 16 * 1024 * 1024;
			}
			else
			{
				count = writesize;
			}

			status = n_writefile(handle->filehandle, handle->filepos, count, buffer, &count);
			if (status != N_SUCCESS || count == 0xFFFFFFFFFFFFFFFF)
			{
				MyLog(LOG_ERROR, L"_FileWrite2 failed with error %x", status);
				return FALSE;
			}

			writesize -= count;
			//GoProcessBar(count);

			buffer += count;
			handle->filepos += count;
			total -= count;
			*size += count;
		}

		if (total)
		{
			CopyMem(handle->cache, buffer, total);
			handle->cachedatasize = total;
			handle->cachedatapos = handle->filepos;
			handle->filepos += total;
			handle->cachedirty = TRUE;
			*size += total;
		}
		return TRUE;
#else
		return FALSE;
#endif
	}
	else
	{
		return SegFileWrite(handle, size, buffer);
		//status = handle->psys->RootFs->Write(handle->filehandle, size, buffer);
		//if (EFI_ERROR(status))
		//{
		//	MyLog(LOG_ERROR, L"_FileWrite3 failed with error 0x%x.", status);
		//	return FALSE;
		//}
		//else
		//{
		//	return TRUE;
		//}
	}
	return TRUE;
}
