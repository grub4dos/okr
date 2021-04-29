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
// #include <Library/ShellLib.h>
// #include <Library/TimerLib.h>
// #include <Protocol/UgaDraw.h>

#include "types.h"

#include "ntfs.h"

//#define LOG_ERROR Print
//#define LOG_DEBUG Print 

#define MAX_DISK_NUMBER		16
#define INVALID_DISK_NUMBER	0xffff
#define RW_BLOCK_SECTORS	2048


typedef struct
{
	WORD		ActiveDisk;
	WORD		Count;
	EFI_DISK_PARAMETER	Disk[MAX_DISK_NUMBER];
}DISKIO, *PDISKIO;

extern DISKIO		gDisk;

EFI_STATUS
EFIAPI
BlockIoInit();

VOID BlockIoRelease();
