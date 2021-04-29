#pragma once

#include <Uefi.h>

#include <Guid/GlobalVariable.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimplePointer.h>

#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
//#include <Library/FileHandleLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
//#include <Library/ShellLib.h>
#include <Library/TimerLib.h>
//#include <Protocol/UgaDraw.h>

//#include <time.h>


#pragma pack(1)

#define NSUCCEED(x) (x==N_SUCCESS)

// 错误码
#define N_SUCCESS					0
#define N_FILE_NOT_FOUND			2
#define N_PATH_NOT_FOUND			3
#define N_NOT_ENOUGH_MEMORY			8
#define N_FILE_EXISTS				80
#define N_INVALID_PARAMETER			87
#define N_METAFILE					0xC0000001
#define N_NOT_DIRECTORY				0xC0000002
#define N_FAIL						0xFFFFFFFF

// 分区或磁盘efi信息
typedef struct _EFI_DISK_PARAMETER_
{
	BOOLEAN					bHooked;
	BOOLEAN					Reserved[3];
	DWORD					MediaId;
	DWORD					BytesPerSector;
	DWORD					BytesPerPhysicalSector;
	ULONGLONG				BeginSector;
	ULONGLONG				TotalSectors;
	EFI_HANDLE				DiskHandle;
	EFI_BLOCK_READ			ReadBlocks;
	EFI_BLOCK_WRITE			WriteBlocks;
	EFI_BLOCK_FLUSH			FlushBlocks;
	EFI_BLOCK_IO_PROTOCOL *	BlockIo;
}EFI_DISK_PARAMETER;


// 文件句柄
typedef void* NFILE;

#define N_TYPE_FILE					1
#define N_TYPE_DIR					2

/* MAX_PATH definition was missing in ntfs-3g's headers. */
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

//目录项
typedef struct _DIR_ENTRY_
{
	DWORD type;				//文件或目录
	long long filesize;		//如果该项为文件的话，则是文件大小
	INT32 modifytime;		//修改时间 
	long  namelen;			//文件名长度，不包括空字符
	char name[MAX_PATH];			//文件或目录名
}DIR_ENTRY,*PDIR_ENTRY;

//列目录信息
typedef struct _DIR_LIST_
{
	long count;				//目录项的数目
	DIR_ENTRY	entry[1];	//目录项
}DIR_LIST,*PDIR_LIST;

//
// 分区初始化
// EFI_DISK_PARAMETER *efidisk - 分区相关信息
//
DWORD n_init(EFI_DISK_PARAMETER *efidisk, DWORD bReadOnly);

//
// 读写结束后，分区资源释放
// BOOL force - 是否强制释放
//
DWORD n_uninit(DWORD force);

//
// 新建目录，目录必须不存在，否则返回错误N_FILE_EXISTS
//           父目录必须存在，否则返回错误N_PATH_NOT_FOUND
// char* path - 全路径 \dir\file，最后字符不能为路径分符'\'
//
DWORD n_createdir(char* path);

//
// 删除目录，不存在，则返回N_PATH_NOT_FOUND
//           
// char* org_path - 全路径 /dir/subdir
//
DWORD n_deletedir(char* org_path);

//
// 列目录
// char* path - 全路径 /dir/subdir
// long long *psize - buf大小
// void* buf - 如果buf为NULL或buf size不足，则在psize中返回buf所需的大小，并返回N_NOT_ENOUGH_MEMORY
// 
DWORD n_listdir(char* path,long long *psize,void* buf);

//
// 打开文件，文件必须要存在，否则返回错误N_PATH_NOT_FOUND
// char* path - 全路径 \dir\file
// NFILE* file - 如果成功，file返回文件句柄，失败则NULL
//
DWORD n_openfile(char* path,NFILE* file);

//
// 新建文件，文件必须不存在，否则返回错误N_FILE_EXISTS
//           父目录必须存在，否则返回错误N_PATH_NOT_FOUND
// char* path - 全路径 \dir\file
// NFILE* file - 如果成功，file返回文件句柄，失败则NULL
//
DWORD n_createfile(char* path,NFILE* file);

//
// 删除文件，不存在，则返回N_PATH_NOT_FOUND
//           
// char* path - 全路径 \dir\file
//
DWORD n_deletefile(char* org_path);

//
// 关闭文件
// NFILE file - 文件句柄
//
DWORD n_closefile(NFILE file);

// 读取文件
// NFILE file - 文件句柄
// long long offset - 文件偏移
// long long size - 读取的内容大小，byte
// void* buf - 读入内容的缓存地址
// long long *readsize - 实际读取的大小
//
DWORD n_readfile(NFILE file,long long offset,long long size,void* buf,long long *readsize);

// 写入文件
// NFILE file - 文件句柄
// long long offset - 文件偏移
// long long size - 写入的内容大小，byte
// void* buf - 写入内容的缓存地址
// long long *writesize - 实际写入的大小
//
DWORD n_writefile(NFILE file,long long offset,long long size,void* buf,long long *writesize);

// 设置文件大小
// NFILE file - 文件句柄
// long long size - 文件的大小，byte
//
DWORD n_setfilesize(NFILE file,long long size);

// 获取文件大小
// NFILE file - 文件句柄
// long long *psize - 返回文件大小，byte
//
DWORD n_getfilesize(NFILE file,long long *psize);

//
// 返回ntfs卷是否只读
// void *vol - ntfs_volume*
//
DWORD n_check_vol_readonly(void *vol);

#pragma pack()
