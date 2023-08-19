#ifndef _OKR_AUTOFACEFUNC_H_
#define _OKR_AUTOFACEFUNC_H_

#include "commondef.h"

#define LENOVO_BACKUP_RESTORE_NAME	L"OKRFunKey"
#define LENOVO_DM_VARIABLE \
{ \
	 0x27A0192B, 0xF890, 0x43E7, { 0xAD, 0xBE, 0xE9, 0x37, 0x2A, 0x64, 0x5E, 0x6 }\
}

typedef struct _LENOVO_BACKUP_RESTORE_DATA {
	BYTE    VariableData;
} LENOVO_BACKUP_RESTORE_DATA;

BOOL GetAutoFuncFlagFromBIOS(BYTE* value);
BOOL GetAutoFuncFlagFromConfigFile(BYTE* value);

void ClearAutoFuncFlagInBIOS();
void ClearAutoFuncFlagInConfigFile();
BOOL AutoBRHasFactoryBackup(int* iLenovoPart);
BOOL AutoBRDetectAvailableDataPartition(int* iAvailablePartIndex, BOOL* bHasBitLocker, BOOL* available);
BOOL IsPartHasEnoughSpace(int iPartIndex);
void SaveAutoBackupFileLocation(int iBackupPartIndex);
void DeletePreAutoBackupFile();
BOOL DetectRestoreImge(WORD* recoverfilepartindex, DWORD* backupfileID);
BOOL GetAutoBackupLocationFromConfigFile(CHAR8* disksn, ULONGLONG* BootBegin, DWORD* Time);
BOOL GetFactoryImageFromLenPart(WORD* recoverfilepartindex, DWORD* backupfileID);
#endif