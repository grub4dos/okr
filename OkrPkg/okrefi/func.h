/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#ifndef _FUNC_H_
#define _FUNC_H_

//备份时，以64扇区作为基本单位备份
#define BLOCK_SIZE 64
//多少字节压缩一次，必须被缓冲区大小整除
#define MAX_COMPRESS_BLOCK_SIZE (500*1024*1024)
#define MIN_COMPRESS_BLOCK_SIZE (100*1024*1024)

#define TMP_FILE_NAME L"\\tmp.dat"

#define MAX_DELE_FILE_NUM  0x20


#define RESREV_SIZE  32

#pragma pack(1)

typedef struct _lZ4_BUFFER_HEAD
{
    DWORD  compress_flag;
    DWORD  lz4_size;
    DWORD  align_lz4_size;
    BYTE   reserv[4]; 
//    BYTE   reserv[4084];       
      
}lZ4_BUFFER_HEAD,*P_lZ4_BUFFER_HEAD;


typedef enum _ENUM_GSKU_SKIP_TYPE
{
	DISABLE_NVME_TO_SATA = 0,
	DISABLE_SATA_TO_NVME,
	DISABLE_DIFFERENT_TYPE, //both DISABLE_NVME_TO_SATA and DISABLE_SATA_TO_NVME
	DISABLE_NEW_HDD_FUNC,
	DISABLE_NVME_TO_NVME,
	DISABLE_SATA_TO_SATA,
	DISABLE_SAME_TYPE,		//both DISABLE_NVME_TO_NVME and DISABLE_SATA_TO_SATA
	NOT_GSKU
}ENUM_GSKU_SKIP_TYPE;

#pragma pack()

int ProbeSystemPartition();
struct CONTROL;
//int Backup(WORD Destpart,CHAR16 *desc,struct CONTROL *pctl);
int Backup(BOOL if_init_backup,WORD Destpart,CHAR16 *desc,PCONTROL pctl);

int CheckRAIDDiskRecovery(WORD partindex, DWORD fileindex);
//int Recover(WORD partindex,DWORD backupfileID,struct CONTROL *pctl);
int Recover(WORD partindex, DWORD fileindex, PCONTROL pctl);
//int GetBackupFileList(int partindex,OKR_HEADER* pbackuplist,int maxnum,int *);
int GetBackupFileList(int partindex, OKR_HEADER* pbackuplist, int maxnum, int *img_err);
int GetAllBackupFiles(int *);
UINT64 GetBackupTotaldata();

int GoProcessBar(DWORD showcount);

BOOL IsExistFileInDir_NTFS_FAT(FILE_SYS_INFO  *sysinfo,CHAR16* fullpath);
BYTE HasFactoryBackupInPartition(FILE_SYS_INFO  *SysInfo, WORD Destpart, PBYTE Buffer);
BOOL IsExistDir_NTFS_FAT(FILE_SYS_INFO  *sysinfo,CHAR16* fullpath);

int CheckAllBackupFiles();
int BackupLastProcess();
int DestroyProcessBar();
INTN GetDataToBitmap(UINT64 sector, UINT64 num, VOID *bitmap);
VOID CompressBuffer(PBUFFER_INFO buffinfo);
BOOL CheckIfAllDataParBitlocker(WORD diskindex);
BOOL CheckIfAllDataParBitlockerInAllDisk();
BOOL IfExistBitlocker();
BOOL IfExistBitlockerOnDisk(int iDiskNum);
BOOL IfDataParExistBitlocker();
BOOL IsLenovoPart(int partindex);
int GetBackupFileFullPath(PFILE_SYS_INFO info, int partindex, CHAR16* fileName, int iFileNameIndex, CHAR16* fileFullPath, int iFileFullPathLen);
BOOL IsFileExist(int partindex, CHAR16* fileName);
BOOL IsFileExistEx(PFILE_SYS_INFO info, CHAR16* fileName);
BOOL HasGSKUFile(int partindex, ENUM_GSKU_SKIP_TYPE * iSkipType);

BOOL IsRAIDBackupFile(DWORD backupfileID);

BOOL SaveFileCheckValue(PFILE_HANDLE_INFO filehandle, unsigned char *pucCheckValue, int iValueLength);
BOOL CalculateFileCheckValue(PFILE_SYS_INFO sysinfo, CHAR16 *filename, DWORD FileNumber);
BOOL ValidImageCheckValue(PFILE_SYS_INFO sysinfo, CHAR16 *filename, DWORD FileNumber);

//2G
//#define SPLIT_SIZE (1500000)
#define SPLIT_SIZE (1510612736)  
                                  
//#define SPLIT_SIZE (100*1024*1024)  

typedef struct QUEUE_CACHE{
	BYTE  bValid;
	BYTE  padding[2];
	DWORD Number;
	DWORD BufferSize;
	PBYTE Buffer;
}QUEUE_CACHE, *PQUEUE_CACHE;
#define MAX_NUM_QUEUE_CACHE 32

extern WORD  g_x_begin;	//窗口的x坐标
extern WORD  g_y_begin;	//窗口的y坐标

#endif


