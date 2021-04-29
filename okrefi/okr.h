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

//ȥ��"�ṹ������û�ж�������"�ľ���
#pragma warning(disable: 4101)  //unreferenced local variable
#pragma warning(disable: 4189)	//local variable is initialized but not referenced
#pragma warning(disable: 4201) 
#pragma warning(disable: 4996)
#pragma warning(disable: 4701)
#pragma warning(disable: 4164)
#pragma warning(disable: 4028)
#pragma warning(disable: 4013)

#pragma intrinsic(memset)
//#define DEFMEMSET  //����memset��memcpy����
#define NTFSLIB 
//#define _SNAPSHOT_SSD_MODULE_	//����snapshot ssdģ��

//�Ƿ����˶��̶߳�дӲ��
//#define OKR_ASYNC_IO	1

//����OKR_DEBUG��Ŀ����Ϊ����releaseʱ��debug��Ϣ�������������
//#define OKR_DEBUG	 1 //define it in the  okrd.inf so if use debug mode build,add by wh
#define OKR_DEBUG_UI  1 // use debug all the UI��interface ��will change logic��add by wh
#define OKR_DEBUG_TEST_MSGUI  1 // use debug all the msgbox UI��interface ��will change logic��add by wh

//����OKR_EVENT��Ŀ���ǲ���ʹ��event�����߳�ͬ����Ч�ʣ����ָֻ�ʱ���ַ�ʽ��Ч�ʷǳ���͵��߳��ٶ�һ����
//����ʱ���ã��������ٶȺͲ�ʹ��eventһ����������ʱ����event�����ж��߳�ͬ��
//#define OKR_EVENT	1

//�Ƿ���ʾ���ݻָ���ʱ��
#define SHOW_TIME 1

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
	
	char 	Cmd;			//��ǰ�û�ѡ������� = COMMAND_NONE;
	char 	BackupDiskNumber;
	char 	RecoverDiskNumber;
	char	ImageDiskNumber;		//�����ļ����ڵ�Ӳ�̣��ָ�ʱʹ��
	
	char	bCompleted;		//��ǰ����/�ָ������Ƿ���ϣ������߳�ͨ�������־�������Ƿ��˳�
	char	ThreadState;	//
	char	bForceRestorePartition;	//����ѡ���ǰ������ͱ��ݷ�����һ��ʱ�Ƿ�ǿ�ƻָ�������
	char	bRefeshDebugInfo;		//
	
	char	bMultiThread;	//�Ƿ�֧�ֶ��߳�
	char	bAsyncIO;		//�Ƿ�֧���첽IO
	char	bDebug;			//�Ƿ���debugģʽ
	char	LogLevel;		//��־���level

	char	bHasFactoryBackup;	//�Ƿ��Ѿ����ڹ�������
	char	bHibernate;
	char	bExitToShell;		//�˳���shell����Ҫ����
	char	oempartition;		//lenovo�ķ��������������

	char	RecursiveTimes;
	char	bFactoryRestore;	//�Ƿ񹤳��ָ�ģʽ�����ָ�mbr�������ǻָ�����ʼ���ݵ�
	char	paddingdd[2];
	
	DWORD	TimeZone;			//����ʱ������

	UINT64	TotalBackupSize;	//���ݵ����ݵ��ܴ�С
	UINT64  CurImageSyspartitionsize;
	//UINT64	currentsize;		//��ǰ��������ݴ�С
	//ULONGLONG readn;
	DWORD	BeginTime;	//��ʼʱ��
	DWORD	LastTime;	//�ϴ���ʾʱ��
	DWORD	LastRemain;	//�ϴ�ʣ��ʱ��
	DWORD	TotalTime;
	
	WORD	ImageFilesNumber;	//�����ļ��ĸ���
	WORD	LogY;				//���Դ��ڵĵ�ǰ�ı���Ϣ��������
	RECT	rWnd;				//�����ڵ�����
	
	DWORD	scanbuffersize;
	DWORD	groupdescsize;
	PBYTE	scanbuffer;			//����ʱɨ����Ч�����õĻ�����
	PBYTE	groupdesc;			//
	PBYTE	databitmap;				//ѹ�����ѹ�õ���bitbuffer
	WIN32_PARTITIONINFO *partition;			//��ǰ���ڱ��ݻ�ָ��ķ���
	WIN32_PARTITIONINFO *partition_write;	//��ǰ���ڻָ��ķ���
	OKR_HEADER *fileheader;
	OKR_HEADER *images;		//��⵽�����о����ļ����ļ�ͷ

    VOID  *Scratch;			//��ѹ�õ�����ʱ������
    UINT32 ScratchSize;		//��������С
    VOID  *bmp;				//��ѹͼ���õĻ�����, 1MB
    UINT32 bmpsize;			//��ѹͼ���õĻ�������С
	VOID  *dbgBackground;	//������Ϣ�ı���
	VOID  *dbgbuf;			//������Ϣ��������Ļ����1MB�� 600��400��4=
	UINT32 dbgbufsize;		//������Ϣ��������С

	DWORD  MemoryPagesNumber;	//�ڴ�ҳ�ĸ���
	DWORD  CompressBlockSize;	//ÿ��ѹ�����С

	DWORD dwErrorCode;
	DWORD dwLineNumber;
	DWORD dwParameters[4];

	UINT32 logbufsize;			//log���������ܴ�С�����128KB
	UINT32 loglength;			//��ǰlog�Ĵ�С
	char*  logbuf;
	int   selecteddisk;
	int  comefromnewhddface ;
	int  destdisktype ;
	ULONGLONG PartCBootBegin ; 
	ULONGLONG PartCTotalSector ; 
	int needlimit ;
	FILE_HANDLE_INFO hLog;		//��־�ļ��ľ��

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
int GetFileNamelist(FILE_SYS_INFO *sysinfo, CHAR16 *filelist, INTN size);
char HasFactoryBackup();
int DrawCopyrightDlg();
int  DrawNewCopyrightDlg(); //  need show many info add by wh 2018-02-01

