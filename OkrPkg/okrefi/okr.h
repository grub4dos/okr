/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#include <Uefi.h>

#include <Protocol/BlockIo.h>   
#include <Protocol/BlockIo2.h>   
#include <Protocol/DevicePathToText.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/DevicePath.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
//#include <Library/ShellLib.h>
#include <Library/TimerLib.h>
#include <guid/FileInfo.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/Print.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/Decompress.h>

#include <foundation/protocol/consolecontrol/consolecontrol.h>

//去掉"结构或联合没有定义名称"的警告
#pragma warning(disable: 4101)  //unreferenced local variable
#pragma warning(disable: 4189)	//local variable is initialized but not referenced
#pragma warning(disable: 4201) 
#pragma warning(disable: 4996)
#pragma warning(disable: 4701)
#pragma warning(disable: 4164)
#pragma warning(disable: 4028)
#pragma warning(disable: 4013)

#pragma intrinsic(memset)
//#define DEFMEMSET  //定义memset和memcpy函数
#define NTFSLIB 
//#define _SNAPSHOT_SSD_MODULE_	//定义snapshot ssd模块

//是否定义了多线程读写硬盘
//#define OKR_ASYNC_IO	1

//定义OKR_DEBUG的目的是为了在release时将debug信息不编译进程序中
//#define OKR_DEBUG	 1 //define it in the  okrd.inf so if use debug mode build,add by wh
#define OKR_DEBUG_UI  1 // use debug all the UI　interface ，will change logic　add by wh
#define OKR_DEBUG_TEST_MSGUI  1 // use debug all the msgbox UI　interface ，will change logic　add by wh

//定义OKR_EVENT的目的是测试使用event进行线程同步的效率，发现恢复时这种方式的效率非常差，和单线程速度一样；
//备份时还好，但提升速度和不使用event一样，所以暂时不用event来进行多线程同步
//#define OKR_EVENT	1

//是否显示备份恢复的时间
#define SHOW_TIME 1

#define AUTO_MESSAGE_COUNT_DOWN_TIME	9

#include "commondef.h"
#ifdef NTFSLIB
#include "ntfs.h"
#endif

#include "vga.h"
#include "resource.h"
#include "utils.h"
#include "window.h"
#include "ntfshead.h"
#include "partition.h"

#include "../../include/format.h"

#include "func.h"
#include "utils.h"
#include "lz4.h"
#include "jpg.h"
 
#include "face.h"
#include "autoface.h"
#include "autofacefunc.h"
#include "getlist.h"

#include "lnk.h"

//extern EFI_LOADED_IMAGE_PROTOCOL *g_Loadedimage;
extern IMAGE_FILE_INFO g_Img_file_list[];

void *__cdecl memcpy( void *dest, const void *src, int count );
void *__cdecl memset(void *pTarget, int value, int cbTarget);
  
//
#define USER_COMMAND_NONE		0
#define USER_COMMAND_BACKUP		1
#define USER_COMMAND_RECOVER	2
#define USER_COMMAND_SSD		3
#define USER_COMMAND_DRIVER_INSTALL	4
#define USER_COMMAND_INIT_BACKUP	5

#define USER_COMMAND_FUNCTION_SELECT	10
#define USER_COMMAND_BACKUP_SELECT		11
#define USER_COMMAND_RECOVER_SELECT		12
#define USER_COMMAND_RECOVER_NEWHDD_SELECT		13
#define COMPRESS_MAGIC			'eazc'
#define UNCOMPRESS_MAGIC		'spxn'

#define OKR_ERROR_SUCCESS						0
#define OKR_ERROR_UNMATCHED_PARTITION			2
#define OKR_ERROR_INVALID_IMAGE_FILE			3
#define OKR_ERROR_DESTINATION_DISK_NOT_FOUND	4
#define OKR_ERROR_DISK_SPACE_TOO_SMALL			5
#define OKR_ERROR_READ_BITMAP					6
#define OKR_ERROR_READ_SIGNATURE				7
#define OKR_ERROR_INVALID_SIGNATURE				8
#define OKR_ERROR_UNIT_SIZE						9

#define OKR_ERROR_FILE_SYSTEM				10
#define OKR_ERROR_CREATE_DIRECTORY			11
#define OKR_ERROR_OPEN_FILE					12
#define OKR_ERROR_READ_FILE					13
#define OKR_ERROR_WRITE_FILE				14
#define OKR_ERROR_WRITE_DISK				15
#define OKR_ERROR_READ_GPT					16
#define OKR_ERROR_WRITE_GPT					17
#define OKR_ERROR_BACKUP_GPT				18
#define OKR_ERROR_WRITE_HEADER				19
#define OKR_ERROR_READ_DISK					20

#define OKR_ERROR_COMPRESS					20
#define OKR_ERROR_DECOMPRESS				21
#define OKR_ERROR_INVALID_COMPRESS_MAGIC	22
#define OKR_ERROR_COMPRESS_DATA_CORRUPTED	23
#define OKR_ERROR_READ_COMPRESS_UNIT		24

#define OKR_ERROR_ALLOC_MEM					30
#define OKR_ERROR_ALLOC_BITMAP				31
#define OKR_ERROR_USER_BREAK				32
#define SCANBUFFER_SIZE 0x10000
#define HEADLISTBUFFER (1024*1024)

//#define MAX_SUPPORED_HDD_NO 3  // include system disk
#define MAX_SUPPORED_HDD_NO 6  // include system disk//ysy 1230

typedef struct _GLOBAL_DATA_
{
	EFI_HANDLE	ImageHandle;
#if OKR_EVENT
	EFI_EVENT cosumer_event;
	EFI_EVENT producter_event;
#endif	
	EFI_DECOMPRESS_PROTOCOL * Decompress;
	
	char 	Cmd;			//当前用户选择的命令 = COMMAND_NONE;
	char 	BackupDiskNumber;
	char 	RecoverDiskNumber;
	char	ImageDiskNumber;		//镜像文件所在的硬盘，恢复时使用
	
	char	bCompleted;		//当前备份/恢复操作是否完毕，辅助线程通过这个标志来决定是否退出
	char	ThreadState;	//
	char	bForceRestorePartition;	//隐藏选项，当前分区表和备份分区表不一致时是否强制恢复分区表
	char	bRefeshDebugInfo;		//
	
	char	bMultiThread;	//是否支持多线程
	char	bAsyncIO;		//是否支持异步IO
	char	bDebug;			//是否处于debug模式
	char	LogLevel;		//日志输出level

	char	bHasFactoryBackup;	//是否已经存在工厂备份
	char	bHibernate;
	char	bExitToShell;		//退出到shell，不要重启
	char	oempartition;		//lenovo的服务分区的索引号

	char	RecursiveTimes;
	char	bFactoryRestore;	//是否工厂恢复模式，将恢复mbr，并且是恢复到初始备份点
	char	paddingdd[2];
	DWORD	TimeZone;			//本地时区，秒

	UINT64	TotalBackupSize;	//备份的数据的总大小
	UINT64  CurImageSyspartitionsize;
	//UINT64	currentsize;		//当前处理的数据大小
	//ULONGLONG readn;
	DWORD	BeginTime;	//开始时间
	DWORD	LastTime;	//上次显示时间
	DWORD	LastRemain;	//上次剩余时间
	DWORD	TotalTime;
	
	WORD	ImageFilesNumber;	//镜像文件的个数
	WORD	LogY;				//调试窗口的当前文本信息的纵坐标
	RECT	rWnd;				//主窗口的坐标
	float	fMinscale;			//窗口缩放比例   基准：3840*2160
	float	fFontScale;
	DWORD	scanbuffersize;
	DWORD	groupdescsize;
	PBYTE	scanbuffer;			//备份时扫描有效数据用的缓冲区
	PBYTE	groupdesc;			//
	PBYTE	databitmap;				//压缩或解压用到的bitbuffer
	WIN32_PARTITIONINFO *partition;			//当前正在备份或恢复的分区
	WIN32_PARTITIONINFO *partition_write;	//当前正在恢复的分区
	OKR_HEADER *fileheader;
	OKR_HEADER *images;		//检测到的所有镜像文件的文件头

    VOID  *Scratch;			//解压用到的临时缓冲区
    UINT32 ScratchSize;		//缓冲区大小
    VOID  *bmp;				//解压图形用的缓冲区, 1MB
    UINT32 bmpsize;			//解压图形用的缓冲区大小
	VOID  *dbgBackground;	//调试信息的背景
	VOID  *dbgbuf;			//调试信息（备份屏幕），1MB， 600×400×4=
	UINT32 dbgbufsize;		//调试信息缓冲区大小

	DWORD  MemoryPagesNumber;	//内存页的个数
	DWORD  CompressBlockSize;	//每个压缩块大小

	DWORD dwErrorCode;
	DWORD dwLineNumber;
	DWORD dwParameters[4];

	UINT32 logbufsize;			//log缓冲区的总大小，最大128KB
	UINT32 loglength;			//当前log的大小
	char*  logbuf;
	int   selecteddisk;
	int  comefromnewhddface ;	//是否恢复到新硬盘
	int  destdisktype ;
	ULONGLONG PartCBootBegin ; 
	ULONGLONG PartCTotalSector ; 
	int needlimit ;
	FILE_HANDLE_INFO hLog;		//日志文件的句柄
	ENUM_GSKU_SKIP_TYPE enumGSKUSkipType;
	BOOL bIsGSKU;
	BOOL bIsAutoFunc;
	DWORD BackupImgTime;
}GLOBAL_DATA, *PGLOBAL_DATA;

extern GLOBAL_DATA g_data;
extern BOOLEAN  IsLinuxSystem;


#define SET_ERROR(n, s) {if(!g_data.dwErrorCode) {g_data.dwLineNumber=n; g_data.dwErrorCode= s;};}
#define SET_ERROR_PARA(e1, e2, e3, e4) { \
	if(!g_data.dwErrorCode){ \
		g_data.dwParameters[0]=e1; \
		g_data.dwParameters[1]=e2; \
		g_data.dwParameters[2]=e3; \
		g_data.dwParameters[3]=e4; \
	};\
}

//
int GetFileNamelist(FILE_SYS_INFO *sysinfo, WORD Destpart, CHAR16 *filelist, INTN size);
char HasFactoryBackup();
//int DrawCopyrightDlg();
////int  DrawNewCopyrightDlg(); //  need show many info add by wh 2018-02-01

