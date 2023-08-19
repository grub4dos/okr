/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#ifndef _OKR_UTILS_H_
#define _OKR_UTILS_H_


#include <uefi.h>
#include <Protocol/BlockIo.h>   
#include <Protocol/BlockIo2.h>   
#include <Protocol/simplefilesystem.h>
#include <Protocol/DiskInfo.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Scsi.h>

#include <Protocol/PciIo.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/AtaPassThru.h>
#include <Protocol/ScsiPassThruExt.h>

typedef
VOID
(EFIAPI *EFI_AP_PROCEDURE) (
  IN  VOID                              *Buffer
  );

#include <Protocol/MPService.h>
#include "commondef.h"

#define muldiv(x,y,z) DivU64x32(MultU64x32((UINT64)x,(UINT32)y),(UINT32)z)
CHAR16 *
Ascii2Unicode (
  OUT CHAR16         *UnicodeStr,
  IN  CHAR8          *AsciiStr
  );

//void * LoadFileToMem(EFI_HANDLE Device,CHAR16 *FilePath,UINT64 attr,UINTN *readsize);

void FreeFileMem(void *buffer);

#define MY_DEFINE_BUFFER_SIZE (20*1024*1024) //20M

#define ALGIN_SIZE_BYTE  (4*1024)

typedef enum _SSD_DISK_TYPE{
	NoType = 0,			//Not init.
	NoSSd,
	SataSSd,			//Sata interface ssd.
	NvmeSSd				//Nvme interface ssd.
}SSD_DISK_TYPE;

//
#define COMMAND_INVALID 0
#define COMMAND_READ  2
#define COMMAND_WRITE 3
struct CONTINUE_RWDISK_INFO {
	BYTE		Func;
	BYTE		DiskNumber;
	WORD		reserved;
	DWORD		Number;
	UINT64		Sector;
	BYTE *		Buffer;
};
void SmartDiskIoInit(BYTE DiskNumber, BYTE readorwrite,struct CONTINUE_RWDISK_INFO *info);
BOOL SmartDiskIo(struct CONTINUE_RWDISK_INFO *info,UINT64 beginsec,DWORD continum,BYTE *buffer);

typedef struct _FILE_SYS_INFO{
	/////////////// efi handle
    EFI_FILE_PROTOCOL *RootFs;
	///////////// ntfs handle
#ifdef NTFSLIB
	EFI_DISK_PARAMETER partinfo;
#endif
	///////////// options
	BOOL ifntfs;
} FILE_SYS_INFO, *PFILE_SYS_INFO;

typedef struct {
	PFILE_SYS_INFO psys;
	EFI_FILE_PROTOCOL *filehandle;
#ifdef NTFSLIB
	UINT64 filepos;
#endif
	////////////////////////
	CHAR16 filename[64];
	DWORD currentfileseg;
	UINT64 currentfilesize;
	///////////// options
	BOOL	ifdir;
	BOOL	ifneedseg;
	BOOL	ifeof;
	BOOL	cachedirty;		//cache中的数据需要刷新到硬盘中
	DWORD	cachesize;
	DWORD	cachedatasize;	//cache中有效数据大小
	UINT64 	cachedatapos;	//cache中有效数据的偏移
	char	*cache;
} FILE_HANDLE_INFO, *PFILE_HANDLE_INFO;
#define DEFAULT_FILE_CACHE_SIZE	0x100000

BOOL FileSystemInit(OUT PFILE_SYS_INFO info, WORD partindex, BOOL  readonly);
void FileSystemClose(PFILE_SYS_INFO info);
//support directry,create file or dir.attribute&openmode use EFI system define
//ifneedseg, true=support file segments,only support read only or write only
BOOL FileDelete(PFILE_SYS_INFO info, CHAR16 *filename);
BOOL FileOpen(IN PFILE_SYS_INFO info, OUT PFILE_HANDLE_INFO handle, CHAR16 *filename, UINT64 openmode, UINT64 attribute, BOOL ifneedseg);
BOOL FileRead(PFILE_HANDLE_INFO handle, OUT UINTN * size, PBYTE buffer);
BOOL FileWrite(PFILE_HANDLE_INFO handle, UINTN  size, PBYTE buffer);
BOOL FileClose(PFILE_HANDLE_INFO handle);
BOOL FileSetPos(PFILE_HANDLE_INFO handle, UINT64 pos);
BOOL GetFileSizeByName(IN PFILE_SYS_INFO info, CHAR16 *filename, UINT64 *FileSize);
BOOL FileGetSize(PFILE_HANDLE_INFO handle, UINT64 *FileSize);

BOOL NormalFileRead(PFILE_HANDLE_INFO handle, UINTN * size, PBYTE buffer);
BOOL NormalFileWrite(PFILE_HANDLE_INFO handle, UINTN  *size, PBYTE buffer);

VOID DumpHex(IN UINTN        Indent, IN UINTN        Offset, IN UINTN        DataSize, IN VOID         *UserData);
CHAR16 getch();


#define SPrint UnicodeSPrint

BOOL logopen(FILE_SYS_INFO *sysinfo, WORD partindex);
void logflush(BOOL bOpenFile);
void PrintLogBuffer();
VOID MyLog(WORD loglevel,  IN  CONST CHAR16  *Format,  ...);
VOID MyLogShowTime(WORD loglevel, IN  CONST CHAR16  *Format, ...);
VOID MyLogString(CHAR16 *String);

#define LOG_DEBUG 5
#define LOG_VERBOSE 4
#define LOG_MESSAGE  3
#define LOG_ALERT   2
#define LOG_ERROR  1

#define LOG_GRAPHIC_X        16
#define LOG_GRAPHIC_Y        40
#define LOG_GRAPHIC_WIDTH    400
#define LOG_GRAPHIC_HEIGHT   640

#define MAX_DISK_NUM 16

#define DISK_DATA 0
#define DISK_SYSDISK 1
#define NEW_FOUND_DISK 2
#define RAID_DISK 3
 


typedef struct {
	EFI_HANDLE devhandle;
    EFI_BLOCK_IO_PROTOCOL *devblkIo;
	BYTE        guid[16];
	CHAR16		diskname[256];
	UINT64		totalsector;
	BYTE		removable;
	BYTE		formatted;	//是否格式化了
	BYTE		datadisk;	//是否为数据盘，通过和备份镜像里的分区信息比较来判断
	//BYTE		backupdisk;	//是否为上次备份的系统盘，通过和备份镜像里的分区信息比较来判断
    CHAR8       disksn[20]; 
	CHAR8       raidDisksn[20];
	CHAR8       curdisktype;  // DISK_DATA 0 数据磁盘，DISK_SYSDISK 1 系统所在磁盘 在备份文件中将系统所在磁盘默认为第一个disksn， NEW_FOUND_DISK 2 新发现硬盘
	UINT8		uiDevicePathProtocolType; 
	UINT8		uiDevicePathProtocolSubType;
	BYTE		isRAIDDisk;	//是否为RAID盘。1: RAID disk. 0: No RAID disk
//  SSD_DISK_TYPE disktype;
} DISKINFO ,* PDISKINFO;


typedef struct _IDENTIFY_DATA{ 

	UINT16	General_Config_0;
	UINT16	Reserved_1;
	UINT16	Special_Config_2;
	UINT16	Reserved_3;
	UINT16	Reserved_4;
	UINT16	Reserved_5;
	UINT16	Reserved_6;
	UINT16	Reserved_7;
	UINT16	Reserved_8;
	UINT16	Reserved_9;
	UINT8	Serial_Number_10[20];
	UINT16	Reserved_20;
	UINT16	Reserved_21;
	UINT16	Reserved_22;
	UINT8	Firmware_Revision_23[8];
	UINT8	Model_Number_27[40];
	UINT16	Maximum_Sector_Multiple_Command_47;
	UINT16	Trusted_Computing_Support;
	UINT16	Capabilities_49;
	UINT16	Capabilities_50;
	UINT16	PIO_Mode_51;
	UINT16	Reserved_52;
	UINT16	Valid_Bits_53;
	UINT16	Reserved_54_58[5];
	UINT16	Valid_Bits_59;
	UINT32	Addressable_Sector_60;
	UINT16	SingleWord_DMA_62;
	UINT16	MultiWord_DMA_63;
	UINT16	PIO_Mode_64;
	UINT16	Min_Multiword_DMA_timing_65;
	UINT16	Manuf_Multiword_DMA_timing_66;
	UINT16	Min_PIO_Mode_timing_67;
	UINT16	Min_PIO_Mode_timing_68;
	UINT16	Reserved_69_74[6];
	UINT16	Queue_Depth_75;
	UINT16	Reserved_76_79[4];
	UINT16	Major_Revision_80;
	UINT16	Minor_Revision_81;
	UINT16	Command_Set_Supported_82;
	UINT16	Command_Set_Supported_83;
	UINT16	Command_Set_Supported_84;
	UINT16	Command_Set_Enabled_85;
	UINT16	Command_Set_Enabled_86;
	UINT16	Command_Set_Enabled_87;
	UINT16	UDMA_Mode_88;
	UINT16	Time_security_Earse_89;
	UINT16	Time_Esecurity_Earse_90;
	UINT16	Current_Power_Level_91;
	UINT16	Master_Password_Rev_92;
	UINT16	Hard_Reset_Value_93;
	UINT16	Acoustic_Level_94;
	UINT16	Reserved_95_99[5];
	UINT64	LBA_48;
	UINT16	Reserved_104_126[23];
	UINT16	Status_Notification_127;
	UINT16	Security_Status_128;
	UINT16	Reserved_129_159[31];
	UINT16	CFA_Power_Mode_160;
	UINT16	Reserved_161_175[15];
	UINT16	Media_Serial_Number_176_205[30];
	UINT16	Reserved_206_254[49];
	UINT16	Checksum_255;
} IDENTIFY_DATA;

typedef struct {
    UINT16      MP;                 // Bits 15:0
    UINT8       Reserved1;          // Bits 23:16
    UINT8       MPS : 1;            // Bits 24
    UINT8       NOPS : 1;           // Bits 25
    UINT8       Reserved2 : 6;      // Bits 31:26
    UINT32      ENLAT;              // Bits 63:32
    UINT32      EXLAT;              // Bits 95:64
    UINT8       RRT : 5;            // Bits 100:96
    UINT8       Reserved3 : 3;      // Bits 103:101
    UINT8       RRL : 5;            // Bits 108:104
    UINT8       Reserved4 : 3;      // Bits 111:109
    UINT8       RWT : 5;            // Bits 116:112
    UINT8       Reserved5 : 3;      // Bits 119:117
    UINT8       RWL : 5;            // Bits 124:120
    UINT8       Reserved6 : 3;      // Bits 127:125
    UINT16      IDLP;               // Bits 143:128
    UINT8       Reserved7 : 6;      // Bits 149:144
    UINT8       IPS :2;             // Bits 151:150
    UINT8       Reserved8;          // Bits 159:152
    UINT16      ACTP;               // Bits 175:160
    UINT8       APW : 3;            // Bits 178:176
    UINT8       Reserved9 : 3;      // Bits 181:179
    UINT8       APS : 2;            // Bits 183:182
    UINT8       Reserved10[9];      // Bits 255:184
} POWER_STATE_DESCRIPTOR;


typedef struct {
    UINT16                  VID;                    // Offset 01:00                
    UINT16                  SSVID;                  // Offset 03:02
    UINT8                   SerialNumber[20];       // Offset 23:04
    UINT8                   ModelNumber[40];        // Offset 63:24
    UINT8                   FirmwareRevision[8];    // Offset 71:64
    UINT8                   ArbitrationBurst;       // Offset 72
    UINT8                   IEEEOUIIdentifier[3];   // Offset 75:73
    UINT8                   CMIC;                   // Offset 76
    UINT8                   MDTS;                   // Offset 77
    UINT16                  ControllerID;           // Offset 79:78
    UINT32                  VER;                    // Offset 83:80
    UINT32                  RTD3R;                  // Offset 87:84
    UINT32                  RTD3E;                  // Offset 91:88
    UINT32                  OAES;                   // Offset 95:92
    UINT8                   Reserved1[160];         // Offset 255:96
    UINT16                  OACS;                   // Offset 257:256
    UINT8                   ACL;                    // Offset 258
    UINT8                   AERL;                   // Offset 259
    UINT8                   FRMW;                   // Offset 260
    UINT8                   LPA;                    // Offset 261
    UINT8                   ELPE;                   // Offset 262
    UINT8                   NPSS;                   // Offset 263
    UINT8                   AVSCC;                  // Offset 264
    UINT8                   APSTA;                  // Offset 265
    UINT16                  WCTEMP;                 // Offset 267:266
    UINT16                  CCTEMP;                 // Offset 269:268
    UINT16                  MTFA;                   // Offset 271:270
    UINT32                  HMPRE;                  // Offset 275:272
    UINT32                  HMMIN;                  // Offset 279:276
    UINT8                   TNVMCAP[16];            // Offset 295:280
    UINT8                   UNVMCAP[16];            // Offset 311:296
    UINT32                  RPMBS;                  // Offset 315:312
    UINT8                   Reserved2[196];         // Offset 511:316
    UINT8                   SQES;                   // Offset 512
    UINT8                   CQES;                   // Offset 513
    UINT16                  Reserved3;              // Offset 515:514
    UINT32                  NN;                     // Offset 519:516
    UINT16                  ONCS;                   // Offset 521:520
    UINT16                  FUSES;                  // Offset 523:522
    UINT8                   FNA;                    // Offset 524
    UINT8                   VWC;                    // Offset 525
    UINT16                  AWUN;                   // Offset 527:526
    UINT16                  AWUPF;                  // Offset 529:528
    UINT8                   NVSCC;                  // Offset 530
    UINT8                   Reserved4;              // Offset 531
    UINT16                  ACWU;                   // Offset 533:532
    UINT16                  Reserved5;              // Offset 535:536
    UINT32                  SGLS;                   // Offset 539:536
    UINT8                   Reserved6[164];         // Offset 703:540
    UINT8                   Reserved7[1344];        // Offset 2047:704
    POWER_STATE_DESCRIPTOR  psdd[32];               // Offset 3071:2048
    UINT8                   VS[1024];               // Offset 4095:3072
} IDENTIFY_CONTROLLER_DATA;


typedef struct {
  UINT16 Mp;                /* Maximum Power */
  UINT8  Rsvd1;             /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Mps:1;             /* Max Power Scale */
  UINT8  Nops:1;            /* Non-Operational State */
  UINT8  Rsvd2:6;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Enlat;             /* Entry Latency */
  UINT32 Exlat;             /* Exit Latency */
  UINT8  Rrt:5;             /* Relative Read Throughput */
  UINT8  Rsvd3:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rrl:5;             /* Relative Read Leatency */
  UINT8  Rsvd4:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rwt:5;             /* Relative Write Throughput */
  UINT8  Rsvd5:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rwl:5;             /* Relative Write Leatency */
  UINT8  Rsvd6:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rsvd7[16];         /* Reserved as of Nvm Express 1.1 Spec */
} NVME_PSDESCRIPTOR;

typedef struct {
  //
  // Controller Capabilities and Features 0-255
  //
  UINT16 Vid;                 /* PCI Vendor ID */
  UINT16 Ssvid;               /* PCI sub-system vendor ID */
  UINT8  Sn[20];              /* Product serial number */

  UINT8  Mn[40];              /* Proeduct model number */
  UINT8  Fr[8];               /* Firmware Revision */
  UINT8  Rab;                 /* Recommended Arbitration Burst */
  UINT8  Ieee_oui[3];         /* Organization Unique Identifier */
  UINT8  Cmic;                /* Multi-interface Capabilities */
  UINT8  Mdts;                /* Maximum Data Transfer Size */
  UINT8  Cntlid[2];           /* Controller ID */
  UINT8  Rsvd1[176];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // Admin Command Set Attributes
  //
  UINT16 Oacs;                /* Optional Admin Command Support */
    #define NAMESPACE_MANAGEMENT_SUPPORTED  BIT3
    #define FW_DOWNLOAD_ACTIVATE_SUPPORTED  BIT2
    #define FORMAT_NVM_SUPPORTED            BIT1
    #define SECURITY_SEND_RECEIVE_SUPPORTED BIT0
  UINT8  Acl;                 /* Abort Command Limit */
  UINT8  Aerl;                /* Async Event Request Limit */
  UINT8  Frmw;                /* Firmware updates */
  UINT8  Lpa;                 /* Log Page Attributes */
  UINT8  Elpe;                /* Error Log Page Entries */
  UINT8  Npss;                /* Number of Power States Support */
  UINT8  Avscc;               /* Admin Vendor Specific Command Configuration */
  UINT8  Apsta;               /* Autonomous Power State Transition Attributes */
  UINT8  Rsvd2[246];          /* Reserved as of Nvm Express 1.1 Spec */

  //
  // NVM Command Set Attributes
  //
  UINT8  Sqes;                /* Submission Queue Entry Size */
  UINT8  Cqes;                /* Completion Queue Entry Size */
  UINT16 Rsvd3;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Nn;                  /* Number of Namespaces */
  UINT16 Oncs;                /* Optional NVM Command Support */
  UINT16 Fuses;               /* Fused Operation Support */
  UINT8  Fna;                 /* Format NVM Attributes */
  UINT8  Vwc;                 /* Volatile Write Cache */
  UINT16 Awun;                /* Atomic Write Unit Normal */
  UINT16 Awupf;               /* Atomic Write Unit Power Fail */
  UINT8  Nvscc;               /* NVM Vendor Specific Command Configuration */
  UINT8  Rsvd4;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT16 Acwu;                /* Atomic Compare & Write Unit */
  UINT16 Rsvd5;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Sgls;                /* SGL Support  */
  UINT8  Rsvd6[164];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // I/O Command set Attributes
  //
  UINT8 Rsvd7[1344];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // Power State Descriptors
  //
  NVME_PSDESCRIPTOR PsDescriptor[32];

  UINT8  VendorData[1024];    /* Vendor specific data */
} NVME_ADMIN_CONTROLLER_DATA;


#define NVME_ADMIN_DEIOSQ_CMD                0x00
#define NVME_ADMIN_CRIOSQ_CMD                0x01
#define NVME_ADMIN_GET_LOG_PAGE_CMD          0x02
#define NVME_ADMIN_DEIOCQ_CMD                0x04
#define NVME_ADMIN_CRIOCQ_CMD                0x05
#define NVME_ADMIN_IDENTIFY_CMD              0x06
#define NVME_ADMIN_ABORT_CMD                 0x08
#define NVME_ADMIN_SET_FEATURES_CMD          0x09


extern DISKINFO g_disklist[MAX_DISK_NUM];
extern WORD g_disknum;

PVOID myalloc(DWORD Length);
#define myfree(Buffer)	FreePool(Buffer);

CHAR16 * Ascii2Unicode (OUT CHAR16         *UnicodeStr, IN  CHAR8          *AsciiStr);

BOOL DiskWrite(WORD diskindex,UINT64 sector, DWORD num,VOID *buffer);
BOOL DiskRead(WORD diskindex,UINT64 sector, DWORD num,VOID *buffer);
BOOL DiskFlush(WORD diskindex);
//void * mymemset (void *dest, char ch, unsigned int count);
void  mymemset(void *pTarget, UINT8 value, UINTN cbTarget);

void * memcpy (void *dest, const void *src, unsigned int count);
CHAR16 * DevicePathToStr (  IN EFI_DEVICE_PATH_PROTOCOL     *DevPath  );
int  CheckPartitionHandle(WORD disknumber);

int strnicmp(char *str1, char * str2, int len);
int mywcsnicmp(CHAR16 *src, CHAR16 *dst, int len);
//char *stristr(char *srcstring, char *strCharSet);
UINTN StrLength(IN CHAR16 * Str);
 
INTN GetDiskInfo();

EFI_STATUS
EFIAPI
itoa64(
  IN CONST CHAR16   *String,
  IN CONST BOOLEAN  Hex,
     OUT   UINT64   *Value);
 
 INTN
EFIAPI
MyMemCmp (
  IN       BYTE              *FirstString,
  IN       BYTE              *SecondString,
  INTN  num
  );
  
typedef ULONGLONG time_t;

struct mytm {
  int   tm_sec;     /* seconds after the minute [0-60] */
  int   tm_min;     /* minutes after the hour [0-59] */
  int   tm_hour;    /* hours since midnight [0-23] */
  int   tm_mday;    /* day of the month [1-31] */
  int   tm_mon;     /* months since January [0-11] */
  int   tm_year;    /* years since 1900 */
  int   tm_wday;    /* days since Sunday [0-6] */
  int   tm_yday;    /* days since January 1 [0-365] */
  int   tm_isdst;   /* Daylight Savings Time flag */
  long  tm_gmtoff;  /* offset from CUT in seconds */
  char  *tm_zone;   /* timezone abbreviation */
};
time_t mytime (time_t *timer);
struct mytm * mygmtime (const time_t *timer, struct mytm * tm);
DWORD GetTickCount();
DWORD delayEx(DWORD TickCounts);

//
#define MAX_BUFFER_COUNT	4
typedef struct BUFFER_INFO{
	PBYTE	pointer;
	DWORD	totalsize;
	DWORD	usedsize;
	DWORD	srcsize;
	BYTE	datavalid;		//是否有数据在buffer里等待处理
	BYTE	partitionindex;	//该buffer属于哪个分区的数据，恢复时用到
	BYTE	ifcompressed;
	BYTE	padding;
	DWORD	sequence;		//顺序号，用来判断当前那个buffer是可用的consumer
} * PBUFFER_INFO;

struct BUFFERARRAY{
	DWORD  number;			//有效buffer的个数, 不同的算法逻辑需要使用不同个数的buffer，支持硬盘异步IO时双缓冲可能会有等待的现象，可以使用4块buffer
	WORD   consumer;			//当前正在处理的消费者索引号
	WORD   producer;			//当前正在处理的生产者索引号
	struct BUFFER_INFO buffer[MAX_BUFFER_COUNT];
};

BOOL Init_BufferArray(struct BUFFERARRAY *pbuffarray);
VOID  Clean_BufferArray(struct BUFFERARRAY *pbuffarray);

PBUFFER_INFO GetProducter(struct BUFFERARRAY *pbuffarray);
PBUFFER_INFO GetConsumer(struct BUFFERARRAY *pbuffarray);
PBUFFER_INFO GetConsumerAnyway(struct BUFFERARRAY *pbuffarray);
void Destroy_BufferArray(struct BUFFERARRAY *pbuffarray);

struct MP_INFO{
	EFI_MP_SERVICES_PROTOCOL *MpProto;
	INTN AP;
 	EFI_EVENT ap_event;
	int status;//=1running
};

BOOL InitMp(struct MP_INFO *mpinfo);
BOOL StartAP(struct MP_INFO *mpinfo,EFI_AP_PROCEDURE  Procedure, VOID  *ProcedureArgument);
BOOL WaitToComplete(struct MP_INFO *mpinfo);

void WaitForConsumer();
void SignalConsumer();
void WaitForProducter();
void SignalProducter();

CHAR16 *
Ascii2Unicode (
  OUT CHAR16         *UnicodeStr,
  IN  CHAR8          *AsciiStr
  );
  
CHAR8 *
Unicode2Ascii (
  OUT CHAR8          *AsciiStr,
  IN  CHAR16         *UnicodeStr
  );

BOOL IsPrintChar(CHAR16 c);
VOID MyLogBinary(PVOID Buffer, DWORD Length);
PBYTE Decompress(IN VOID *Source, IN UINT32 SourceSize, 
		IN OUT VOID *Destination, IN UINT32 *lpdwDestinationSize
		);
void printhexbuffer(unsigned char *buffer,int len);

int CheckFatPartitionHandle(WORD disknumber);
		
UINT64 CalcAliginSize( UINT64 num );
BOOL SegFileOpen(IN PFILE_SYS_INFO info,OUT PFILE_HANDLE_INFO handle,CHAR16 *filename,UINT64 openmode,UINT64 attribute,BOOL ifneedseg);

BOOL FileOpenA(IN PFILE_SYS_INFO info,OUT PFILE_HANDLE_INFO handle,char *filename,UINT64 openmode,UINT64 attribute,BOOL ifneedseg);
BOOL _FileOpenA(PFILE_HANDLE_INFO Handle, char *FileName, UINT64 OpenMode, UINT64 FileAttribute);
int Upper(CHAR8   *String);

#endif
