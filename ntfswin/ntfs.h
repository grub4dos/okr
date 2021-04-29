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

// ������
#define N_SUCCESS					0
#define N_FILE_NOT_FOUND			2
#define N_PATH_NOT_FOUND			3
#define N_NOT_ENOUGH_MEMORY			8
#define N_FILE_EXISTS				80
#define N_INVALID_PARAMETER			87
#define N_METAFILE					0xC0000001
#define N_NOT_DIRECTORY				0xC0000002
#define N_FAIL						0xFFFFFFFF

// ���������efi��Ϣ
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


// �ļ����
typedef void* NFILE;

#define N_TYPE_FILE					1
#define N_TYPE_DIR					2

/* MAX_PATH definition was missing in ntfs-3g's headers. */
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

//Ŀ¼��
typedef struct _DIR_ENTRY_
{
	DWORD type;				//�ļ���Ŀ¼
	long long filesize;		//�������Ϊ�ļ��Ļ��������ļ���С
	INT32 modifytime;		//�޸�ʱ�� 
	long  namelen;			//�ļ������ȣ����������ַ�
	char name[MAX_PATH];			//�ļ���Ŀ¼��
}DIR_ENTRY,*PDIR_ENTRY;

//��Ŀ¼��Ϣ
typedef struct _DIR_LIST_
{
	long count;				//Ŀ¼�����Ŀ
	DIR_ENTRY	entry[1];	//Ŀ¼��
}DIR_LIST,*PDIR_LIST;

//
// ������ʼ��
// EFI_DISK_PARAMETER *efidisk - ���������Ϣ
//
DWORD n_init(EFI_DISK_PARAMETER *efidisk, DWORD bReadOnly);

//
// ��д�����󣬷�����Դ�ͷ�
// BOOL force - �Ƿ�ǿ���ͷ�
//
DWORD n_uninit(DWORD force);

//
// �½�Ŀ¼��Ŀ¼���벻���ڣ����򷵻ش���N_FILE_EXISTS
//           ��Ŀ¼������ڣ����򷵻ش���N_PATH_NOT_FOUND
// char* path - ȫ·�� \dir\file������ַ�����Ϊ·���ַ�'\'
//
DWORD n_createdir(char* path);

//
// ɾ��Ŀ¼�������ڣ��򷵻�N_PATH_NOT_FOUND
//           
// char* org_path - ȫ·�� /dir/subdir
//
DWORD n_deletedir(char* org_path);

//
// ��Ŀ¼
// char* path - ȫ·�� /dir/subdir
// long long *psize - buf��С
// void* buf - ���bufΪNULL��buf size���㣬����psize�з���buf����Ĵ�С��������N_NOT_ENOUGH_MEMORY
// 
DWORD n_listdir(char* path,long long *psize,void* buf);

//
// ���ļ����ļ�����Ҫ���ڣ����򷵻ش���N_PATH_NOT_FOUND
// char* path - ȫ·�� \dir\file
// NFILE* file - ����ɹ���file�����ļ������ʧ����NULL
//
DWORD n_openfile(char* path,NFILE* file);

//
// �½��ļ����ļ����벻���ڣ����򷵻ش���N_FILE_EXISTS
//           ��Ŀ¼������ڣ����򷵻ش���N_PATH_NOT_FOUND
// char* path - ȫ·�� \dir\file
// NFILE* file - ����ɹ���file�����ļ������ʧ����NULL
//
DWORD n_createfile(char* path,NFILE* file);

//
// ɾ���ļ��������ڣ��򷵻�N_PATH_NOT_FOUND
//           
// char* path - ȫ·�� \dir\file
//
DWORD n_deletefile(char* org_path);

//
// �ر��ļ�
// NFILE file - �ļ����
//
DWORD n_closefile(NFILE file);

// ��ȡ�ļ�
// NFILE file - �ļ����
// long long offset - �ļ�ƫ��
// long long size - ��ȡ�����ݴ�С��byte
// void* buf - �������ݵĻ����ַ
// long long *readsize - ʵ�ʶ�ȡ�Ĵ�С
//
DWORD n_readfile(NFILE file,long long offset,long long size,void* buf,long long *readsize);

// д���ļ�
// NFILE file - �ļ����
// long long offset - �ļ�ƫ��
// long long size - д������ݴ�С��byte
// void* buf - д�����ݵĻ����ַ
// long long *writesize - ʵ��д��Ĵ�С
//
DWORD n_writefile(NFILE file,long long offset,long long size,void* buf,long long *writesize);

// �����ļ���С
// NFILE file - �ļ����
// long long size - �ļ��Ĵ�С��byte
//
DWORD n_setfilesize(NFILE file,long long size);

// ��ȡ�ļ���С
// NFILE file - �ļ����
// long long *psize - �����ļ���С��byte
//
DWORD n_getfilesize(NFILE file,long long *psize);

//
// ����ntfs���Ƿ�ֻ��
// void *vol - ntfs_volume*
//
DWORD n_check_vol_readonly(void *vol);

#pragma pack()
