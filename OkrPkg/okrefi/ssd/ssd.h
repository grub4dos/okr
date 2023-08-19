/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/

//ssd.h
#ifndef _SSD_H_
#define _SSD_H_

#include <Protocol/AtaPassThru.h>
#include "AtaAtapiPassThru.h"

#define READ      0
#define WRITE      1

//snapshot command 
#define	ATA_CMD_IDENTIFY			0xEC
#define ATA_IDENTIFY				0xEC
#define	ATA_CMD_SNAPSHOT			0x80

//snapshot features
#define ATA_FEA_SAVE_SN				0x01
#define ATA_FEA_GET_SN_COUNT		0x02
#define ATA_FEA_GET_SN_INFO			0x03
#define ATA_FEA_DO_PENDING			0x04
#define ATA_FEA_RESTORE_SN			0x05
#define ATA_FEA_REMOVE_SN			0x06
#define ATA_FEA_SAVE_SN_DESC		0x07
#define ATA_FEA_GET_SN_DESC			0x08
#define ATA_FEA_GET_FREE_SPACE		0x09
#define ATA_FEA_POWER_OFF_NOTIFY	0x0A
#define ATA_FEA_RESERVE_SN_INDEX	0x0B
#define ATA_FEA_GET_PENDING_COUNT	0x0C
#define ATA_FEA_GET_PENDING_INFO	0x0D
#define ATA_FEA_CANCEL_PENDING		0x0E
#define ATA_FEA_CLEAR_RO_RESTORE	0x0F

#define BIT7      0x00000080
#define BIT6      0x00000040
#define BIT5      0x00000020

typedef union  
{
   struct
   	{
		DWORD dwCommandType;
		WORD  wSnapshotindex;
		WORD  wSnapshotParentindex; 
		BYTE  bIsHavePassword;
		//BYTE  bReadonlyRestore;//rsv
		BYTE  bReserved_1[2];
		BYTE  bReservedforFW;
		BYTE  bPassword[8];
		BYTE  bSnapshotBlockCounter;
		BYTE  bReserved_2[3];
		DWORD dwLBARange[16*2];
		unsigned long long timestamp;
		BYTE  bReserved_3[40];
		BYTE  bSnDesc[256];
		BYTE  bReserved_4[54];
		BYTE  bCheckSumIndicator;
		BYTE  bCheckSum;
   	}s1_save;
   struct
   	{
   	   DWORD dwCommandType;
		WORD  wSnapshotindex;
		WORD  wReserved;//parent_index //wSnapshotParentindex; 
		BYTE  bIsHavePassword;
		BYTE  bReadonlyRestore;//rsv
		BYTE  bReserved;
		BYTE  bReservedforFW;
		BYTE  bPassword_1[8];
		BYTE  bSnapshotBlockCounter;
		BYTE  bReserved_2[3];
		DWORD dwLBARange[16*2];
		BYTE  bReserved_3[358];
		BYTE  bCheckSumIndicator;
		BYTE  bCheckSum;
   	}s2_restore;
  
    struct 
   	{
   	   DWORD dwCommandType;
		WORD  wSnapshotindex;
		WORD  wReserved;//parent_index //wSnapshotParentindex; 
		BYTE  bIsHavePassword;
		BYTE  bReserved_1[3];
		BYTE  bPassword[8];
		BYTE  b_Reserved_2[490];
		BYTE  bCheckSumIndicator;
		BYTE  bCheckSum;
   	}s3_delete;

	struct
	{
		DWORD dwCommandType;
		WORD  wSnapshotindex;
		WORD  wResv_2bytes_1;
		BYTE  bIsHavePassword;
		BYTE  wResv_2bytes_2[2];
		BYTE  bReservedbyFw;
		BYTE  bPassword[8];
		BYTE  bSnapshotBlockCounter;
		BYTE  bResv_3bytes[3];
		DWORD dwLBARange[16*2];
		BYTE  bResv[358];
		BYTE  bCheckSumIndicator;
		BYTE  bCheckSum;		
	}s4_ClsRdonly;
}SN_DESC;

typedef struct _SNAPSHOT_SSD_
{
	EFI_ATA_PASS_THRU_PROTOCOL		*PassThru;
	ATA_ATAPI_PASS_THRU_INSTANCE	*Instance;
	WORD			*Buffer;		//分配的512字节大小的缓冲区，对齐16字节
	UINT16			uiPortNo;
	UINT16			uiPortMultiplierPort;
	BOOLEAN			bInstall;
	BOOLEAN			bInit;
	WORD			MaxSnapshot;
	WORD			MaxPendingCmd;
}SNAPSHOT_SSD, *PSNAPSHOT_SSD;

BOOLEAN SSDInit(
	IN EFI_HANDLE        ImageHandle,
	IN EFI_SYSTEM_TABLE  *SystemTable
);

#endif
