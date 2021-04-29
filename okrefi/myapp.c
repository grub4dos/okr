
#include "okr.h"
 
#ifdef _SNAPSHOT_SSD_MODULE_
#include "ssd/ssd.h"
#endif

//EFI_LOADED_IMAGE_PROTOCOL *g_Loadedimage;
//CHAR16 g_CurrentWorkDir[128];
//EFI_GUID g_ocv_guid =EFI_OCV_GUID;
WCHAR g_EduLoader[] = L"EDU.efi";

GLOBAL_DATA g_data;

extern BYTE *g_seg_buffer;

extern BYTE *g_read_seg_buffer;




#if OKR_DEBUG
// 输入argv数组，和数组长度，输出解析到的字符串列表填入argv和argc,中间不允许有多个空格
int GetArgList(CHAR16 *commandline,CHAR16 **argv , INTN *argc)
{
    CHAR16 ch=commandline[0];
    int i=0;
    int getnum=0;
    argv[0]=commandline;
    while (ch!=0) {
        if (ch==L' ') {
            commandline[i]=0;
            getnum++;
            if (getnum==*argc)
                break;
            argv[getnum]=commandline+i+1;
        }

        i++;
        ch=commandline[i];

    }
    *argc=getnum;
    return 0;
}
#endif

extern BYTE * g_font_bitmap;

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
extern int _fltused;
int
main (
    IN int Argc,
    IN char **Argv
)
{
	EFI_STATUS	status = EFI_SUCCESS;
	EFI_MEMORY_DESCRIPTOR *memoryMap;
	UINTN memoryMapSize;
	UINTN mapKey;
	UINTN descriptorSize;
	UINT32 descriptorVersion;
	
	mymemset(&g_data, 0, sizeof(&g_data));
	
	
	mymemset(g_disklist, 0,  sizeof(g_disklist));

#if OKR_ASYNC_IO
	g_data.bAsyncIO = TRUE;
#endif

#if OKR_DEBUG
	g_data.bDebug = TRUE;
	g_data.LogLevel = LOG_DEBUG;

#else
	g_data.LogLevel = LOG_ERROR;
#endif
	//_asm int 3
	g_data.LogY = LOG_GRAPHIC_Y - 20;

	g_data.bHasFactoryBackup = -1;
	g_data.oempartition = -1;
	
#if OKR_EVENT
	status = gBS->CreateEvent( 0, TPL_NOTIFY, NULL, NULL, &g_data.cosumer_event);
	status = gBS->CreateEvent( 0, TPL_NOTIFY, NULL, NULL, &g_data.producter_event);
#endif
	
	status = gBS->LocateProtocol(&gEfiDecompressProtocolGuid, NULL, (VOID **)&g_data.Decompress);
	if (EFI_ERROR(status)) {
		g_data.Decompress = NULL;
		if(g_data.bDebug)
			MyLog(LOG_ERROR,L"EfiDecompressProtocol not found, status 0x%x\n", status);
		return -1;
	}
	KeyboardInit();

	g_data.bmpsize = 1024*1024;	//1MB
	g_data.bmp = myalloc(g_data.bmpsize);
	g_data.dbgbufsize = 1024*1024;
	g_data.dbgbuf = myalloc(g_data.dbgbufsize);
	g_data.dbgBackground = myalloc(g_data.dbgbufsize);

	g_data.logbufsize = 1024*64;
	g_data.logbuf = myalloc(g_data.logbufsize);
	mymemset(g_data.logbuf, 'X', g_data.logbufsize);

	memoryMapSize = g_data.bmpsize;
	memoryMap = g_data.bmp;
	status = gBS->GetMemoryMap(&memoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);

	//Print(L"GetMemoryMap returns 0x%x, mapsize (0x%x)0x%x, mapKey 0x%x, descriptorSize 0x%x, descriptorVersion 0x%x\n",
	//	status, sizeof(memoryMap), memoryMapSize, mapKey, descriptorSize, descriptorVersion);
	//getch();
	if (!EFI_ERROR(status)) {
		DWORD i;
		DWORD maxPages = 0;
		EFI_MEMORY_DESCRIPTOR *pMap = memoryMap;

		for(i=0;i<memoryMapSize/descriptorSize;i++) {
			//Print(L"%d: Type %x, Physical 0x%x, Virtual 0x%x, Pages 0x%x, Attr 0x%x\n",
			//	i, pMap->Type, pMap->PhysicalStart, pMap->VirtualStart, 
			//	pMap->NumberOfPages, pMap->Attribute);
			if(pMap->NumberOfPages > maxPages)
				maxPages = pMap->NumberOfPages;
			//if((i % 10) == 0)
			//	getch();
			pMap = (EFI_MEMORY_DESCRIPTOR *)((BYTE *)pMap + descriptorSize);
		}
		g_data.MemoryPagesNumber = maxPages;
	}

	{
		EFI_TIME  Time;
		gRT->GetTime (&Time, NULL);
		g_data.TimeZone = ((Time.TimeZone != EFI_UNSPECIFIED_TIMEZONE) ? (Time.TimeZone * 60) : 0);
		MyLog(LOG_DEBUG, L"TimeZone 0x%x\n", Time.TimeZone);
	}

	if( g_data.bDebug ) {
		Print(L"maxPages: 0x%x, Press any key to continue...\n", g_data.MemoryPagesNumber);
		getch();
	}

    return 0;
}

BOOL Cleanup()
{
	MouseRelease();
	UnloadResource();

	if(g_data.images)
    	myfree(g_data.images);
    g_data.images = NULL;
	
	if(g_data.scanbuffer)
		myfree(g_data.scanbuffer);
	g_data.scanbuffer = NULL;
	
	if(g_data.Scratch)
		myfree(g_data.Scratch);
	g_data.Scratch = NULL;
	
	//if(g_data.databitmap)
	//	myfree(g_data.databitmap);
	//g_data.databitmap = NULL;

	//if(g_data.groupdesc)
	//	myfree(g_data.groupdesc);
	//g_data.groupdesc = NULL;

	if(g_data.bmp)
		myfree(g_data.bmp);
	g_data.bmp = NULL;
	
	if(g_data.dbgbuf)
		myfree(g_data.dbgbuf);
	g_data.dbgbuf = NULL;

	if(g_data.dbgBackground)
		myfree(g_data.dbgBackground);
	g_data.dbgBackground = NULL;

	return TRUE;
}

BOOL FoundEduInFolder(FILE_SYS_INFO *sysinfo, WORD PartitionIndex, WCHAR *PathName, BYTE *Buffer)
{
	EFI_STATUS status;
	FILE_HANDLE_INFO filehandle;
	EFI_FILE_INFO *info;
	UINTN filesize, remainsize, bufsize;
	DWORD i;
	BOOL bFound = FALSE;
	int namelen;

	if(g_data.RecursiveTimes > 4)	//递归超过6层
		return FALSE;

	g_data.RecursiveTimes++;
	//Print(L"FoundEduInFolder:%s\n", PathName);

	if (!FileOpen(sysinfo, &filehandle, PathName, EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY,FALSE)) {
		g_data.RecursiveTimes--;
		return FALSE;
	}
	filesize = 0;
	bufsize = 64*1024;
	remainsize = bufsize;
	info = (EFI_FILE_INFO *)Buffer;
	while(remainsize) {
		if (!FileRead(&filehandle, &remainsize, (PBYTE)info)) {
			remainsize = 0;
			break;
		}
		if(remainsize == 0) {
			//Print(L"remainsize 0\n");
			break;
		}
		//Print(L"Size %d, %s\n", filesize, info->FileName);
		filesize += info->Size;		//作为目录，每次只能读取一个目录项
		remainsize = bufsize - filesize;
		info = (EFI_FILE_INFO *)((BYTE*)info + info->Size);
	}
	FileClose(&filehandle);

	i = 0;
	info = (EFI_FILE_INFO *)Buffer;
	//PrintBinary(info, 128);
	while (filesize > 0) {

		if(filesize < info->Size)
			break;
		MyLog(LOG_DEBUG, L"%d: %s\n", i, info->FileName);

		if(info->FileName[0] != '.') {

			if(info->Attribute & EFI_FILE_DIRECTORY) {
				namelen = StrLength(PathName);
				if(namelen > 1)
					SPrint(PathName + namelen, 256, L"\\%s", info->FileName);
				else
					SPrint(PathName + namelen, 256, L"%s", info->FileName);
				bFound = FoundEduInFolder(sysinfo, PartitionIndex, PathName, Buffer + 64*1024);
				PathName[namelen] = 0;
				if(bFound)
					break;
			}
			else {
				if (mywcsnicmp(info->FileName, g_EduLoader, sizeof(g_EduLoader)/sizeof(WCHAR)-1)==0) {
					bFound = TRUE;
					break;
				}
			}
		}

		i++;
		if(filesize <= info->Size)
			break;
		filesize -= info->Size;
		info = (EFI_FILE_INFO *)((BYTE*)info + info->Size);
	}

	g_data.RecursiveTimes--;
	return bFound;
}

BOOL HasEduLoaderFile(WORD PartitionIndex)
{
	FILE_SYS_INFO  sysinfo;
	BOOL bFound = FALSE;
	PBYTE buffer;
	WCHAR *pathName;
	
	buffer = (PBYTE)g_data.images;
	pathName = (WCHAR *)buffer;
	pathName[0] = L'\\';
	pathName[1] = 0;
	buffer += 4096;	//腾出4096字节

	if (FileSystemInit(&sysinfo,PartitionIndex,TRUE) == FALSE) 
		return FALSE;

	g_data.RecursiveTimes = 0;
	bFound = FoundEduInFolder(&sysinfo, PartitionIndex, pathName, buffer);

	FileSystemClose(&sysinfo);

	return bFound;
}

BOOL HasEduLoader()
{
	//遍历所有fat分区
	DWORD num = 0;
	DWORD maxnum = HEADLISTBUFFER/OKR_HEADER_MIN_SIZE;
	DWORD i, j;
	BYTE tmpimage[OKR_HEADER_MIN_SIZE] = { 0 };

	if(!g_data.images)
		g_data.images = myalloc(HEADLISTBUFFER);

	for (i=0;i<g_dwTotalPartitions;i++) {

		if ((g_Partitions[i].PartitionType!=FAT32)
			&&(g_Partitions[i].PartitionType!=FAT32_E)
			&&(g_Partitions[i].PartitionType!=FAT16)
			&&(g_Partitions[i].PartitionType!=FAT16_E))
			continue;
		MyLog(LOG_DEBUG, L"Vol %d:%s\n", i, g_Partitions[i].Label);
		if(mywcsnicmp(g_Partitions[i].Label, L"", 6) != 0
			&& mywcsnicmp(g_Partitions[i].Label, L"LRS_ES", 6) != 0) {
				continue;
		}

		if(HasEduLoaderFile(i))
			return TRUE;
	}

	return FALSE;
}

EFI_STATUS
EFIAPI
OkrMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS Status;
	MyLog(LOG_ERROR, L" OkrMain starting ....  .\n");
    //ShellAppMain(0, NULL);
    //应该是编译器问题，大概是这么一个流程：
    //1、如果直接指定入口地址为OkrMain，那么在/O1开关下进行优化时，没有用到的变量及函数会被优化掉，这样导致link时提示没有_fltused变量，从而出错；
    //2、如果在myapp里定义一个_fltused，那么就会导致重复定义错误
    //所以解决方案是在myapp里引用一下_fltused就可以了
    if(main(_fltused,NULL) < 0)
    	goto cleanup;

	g_data.ImageHandle = ImageHandle;

#if OKR_DEBUG
	MyLog(LOG_DEBUG,L"Loading resources...\n");
#endif
	// 出图形界面
	if (LoadResource()) {
		MyLog(LOG_ERROR, L"Failed to load resource.\n");
		goto cleanup;
	}

#if OKR_DEBUG
	MyLog(LOG_DEBUG,L"Analyzing disk...\n");
#endif
	GetDiskInfo();
#if OKR_DEBUG
	MyLog(LOG_DEBUG,L"Analyzing partition...\n");
#endif
	GetPartitionInfo();

//g_data.bExitToShell = TRUE;
	if(HasEduLoader()) {
		MyLog(LOG_DEBUG, L"EDU found.\n");
		goto cleanup;
	}

#if OKR_DEBUG
	MyLog(LOG_DEBUG,L"Initilizing screen, press any key to continue...\n");
	getch();
#endif
	if(!VgaInit()){
		logflush(TRUE);
		Print(L"VgaInit failed, press any key to continue...\r\n");
		getch();
		goto cleanup;
	}
#if OKR_DEBUG
	MyLog(LOG_DEBUG,L"Initilizing mouse...\n");
#endif
	MouseInit();

#if OKR_DEBUG
	MyLog(LOG_DEBUG,L"Enter main window...\n");
#endif
	
	if(g_seg_buffer == NULL)
	{
	  	g_seg_buffer = myalloc(MY_DEFINE_BUFFER_SIZE);
	  	
	  	if(g_seg_buffer==NULL)
	  	{
	  		 {
			       MyLog(LOG_DEBUG, L"Initilizing myalloc failed:%x.\n",(MY_DEFINE_BUFFER_SIZE));
		         goto cleanup;  		 	  
	  		 }
	  	}
	}

	if(g_read_seg_buffer == NULL)
	{
	  	g_read_seg_buffer = myalloc(MY_DEFINE_BUFFER_SIZE);
	  	
	  	if(g_read_seg_buffer==NULL)
	  	{
	  		 {
			       MyLog(LOG_DEBUG, L"Initilizing myalloc rea buf failed:%x.\n",(MY_DEFINE_BUFFER_SIZE));
		         goto cleanup;  		 	  
	  		 }
	  	}
	}
		
	Face();

#if OKR_DEBUG
	MyLog(LOG_DEBUG,L"Exiting...\n");
#endif
	MouseRelease();
	VgaRelease();

cleanup:
	Cleanup();
	
	if(g_seg_buffer)
	{
		myfree(g_seg_buffer);
		g_seg_buffer = NULL;
	}

	if(g_read_seg_buffer)
	{
		myfree(g_read_seg_buffer);
		g_read_seg_buffer = NULL;
	}
	
	//reboot
	if(!g_data.bExitToShell)
		gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);

    return EFI_SUCCESS;
}
