#include "autofacefunc.h"
#include "faceUIDefine.h"
#include "okr.h"
BYTE AUTOBR_Windows_Data_TypeGuid[16] = { 0xa2,0xa0,0xd0,0xeb,0xe5,0xb9,0x33,0x44,0x87,0xc0,0x68,0xb6,0xb7,0x26,0x99,0xc7 };

BOOL GetAutoFuncFlagFromBIOS(BYTE* value)
{
	BOOL bRet = TRUE;
	*value = 0;
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter GetAutoFuncFlagFromBIOS.\n");
	UINT8 VariableSize = 0;
	LENOVO_BACKUP_RESTORE_DATA  stLenovoBackupRestoreData;
	EFI_STATUS Status = 0;
	EFI_GUID  gBkRsVariableGuid = LENOVO_DM_VARIABLE;
	VariableSize = sizeof(LENOVO_BACKUP_RESTORE_DATA);
	if (NULL != gRT)
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]Call GetVariable.\n");
		Status = gRT->GetVariable(
			LENOVO_BACKUP_RESTORE_NAME,
			&gBkRsVariableGuid,
			NULL,
			&VariableSize,
			&stLenovoBackupRestoreData
		);
		MyLog(LOG_DEBUG, L"[AUTOBR]GetVariable Status:%x.\n", Status);
		if (EFI_ERROR(Status))
		{
			bRet = FALSE;
			MyLog(LOG_DEBUG, L"[AUTOBR]GetVariable ERROR! Status:%x\n", Status);
		}
		else
		{
			*value = stLenovoBackupRestoreData.VariableData;
		}
	}
	MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoFuncFlagFromBIOS value:%x.\n", *value);
	MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoFuncFlagFromBIOS bRet:%d.\n", bRet);
	if (bRet)
	{
		//如果获取成功，将标识清除
		ClearAutoFuncFlagInBIOS();
	}
	return bRet;
}

//从配置文件获取自动备份恢复的状态值
//位置及名称：LENOVO_PART\\okr\\OKRFunKey
BOOL GetAutoFuncFlagFromConfigFile(BYTE* value)
{
	BOOL bRet = FALSE;
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter GetAutoFuncFlagFromConfigFile.\n");
	UINT32 iIndex = 0;

	BOOL bHasSupportedPartition = FALSE;
	BOOL bInsufficientSpace = FALSE;
	BYTE Window_Recovery_Par_TypeGuid[16] = { 0xa4, 0xbb, 0x94, 0xde, 0xd1, 0x06, 0x40, 0x4d, 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac };

	WORD diskindex = 0;
	FILE_SYS_INFO  sysinfo;
	
	for (iIndex = 0; iIndex < g_dwTotalPartitions; iIndex++)
	{
		MyLog(LOG_DEBUG, L"Partition index: %d, Label %s\n", iIndex, g_Partitions[iIndex].Label);

		//不支持mbr
		if (StrnCmp(g_Partitions[iIndex].Label, L"LENOVO_PART", 11) == 0 &&
			MyMemCmp(Window_Recovery_Par_TypeGuid, g_Partitions[iIndex].TypeGuid, 16) == 0)
		{
			//判断值
			FILE_HANDLE_INFO filehandle;
			FILE_SYS_INFO  sysinfo;
			if (FileSystemInit(&sysinfo, iIndex, TRUE) == TRUE)
			{
				if (!FileOpen(&sysinfo, &filehandle, L"\\okr\\OKRFunKey", EFI_FILE_MODE_READ, 0, FALSE))
				{
					MyLog(LOG_DEBUG, L"OKRFunKeyFile FileOpen FAILED.\n");
				}
				else
				{
					MyLog(LOG_DEBUG, L"found OKRFunKey flag file\n");
					UINTN uiSize = 1024;
					CHAR8 buffer[1024] = { 0 };
					EFI_STATUS Status = (filehandle.filehandle)->Read(filehandle.filehandle, &uiSize, (VOID*)buffer);
					if (!EFI_ERROR(Status))
					{
						MyLog(LOG_DEBUG, L"uiSize: %d\n", uiSize);
						MyLog(LOG_DEBUG, L"buffer: %s\n", (CHAR16*)buffer);
						CHAR16 flagPreFix[6] = L"flag=";
						CHAR16* pFlagContent = NULL;
						CHAR16 flagValue[100] = { 0 };
						pFlagContent = StrStr(buffer, flagPreFix);
						if (NULL != pFlagContent)
						{
							StrnCpy(flagValue, pFlagContent + StrLen(flagPreFix), 1);
							MyLog(LOG_DEBUG, L"flagValue: %s\n", flagValue);
							*value = StrDecimalToUintn(flagValue);
							bRet = TRUE;
							MyLog(LOG_DEBUG, L"value: %d\n", *value);
						}
						
						//getch();
						
						/*
						if (uiSize < 1024 && uiSize > 0)
						{
							UINTN uiFlag = uiSize;
							while (uiFlag >= 1)
							{
								if (buffer[uiFlag - 1] >= L'0' && buffer[uiFlag - 1] <= L'9')
								{
									//MyLog(LOG_DEBUG, L"uiFlag - 1: %d buffer[uiFlag - 1] : %d\n", uiFlag - 1, buffer[uiFlag - 1]);

									Type[0] = buffer[uiFlag - 1];

									*value = StrDecimalToUintn(Type);
									MyLog(LOG_DEBUG, L"GetAutoFuncFlagFromConfigFile value in file: %d\n", *value);
									break;
								}

								uiFlag--;
							}
						}
						*/

					}
					else
					{
						MyLog(LOG_DEBUG, L"OKRFunKeyFile FileRead FAILED\n");
					}
					FileClose(&filehandle);
				}

				
				FileSystemClose(&sysinfo);
			}
			else
			{
				MyLog(LOG_DEBUG, L"OKRFunKeyFile FileSystemInit FAILED\n");
			}
			MyLog(LOG_DEBUG, L"OKRFunKeyFile value: %d\n", *value);
			break;
		}
	}

	MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoFuncFlagFromConfigFile value:%x.\n", *value);
	MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoFuncFlagFromConfigFile bRet:%d.\n", bRet);
	return bRet;
}

void ClearAutoFuncFlagInBIOS()
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter ClearAutoFuncFlagInBIOS.\n");
	UINT8  VariableSize = 0;
	LENOVO_BACKUP_RESTORE_DATA  stLenovoBackupRestoreData;
	EFI_STATUS Status = 0;
	VariableSize = sizeof(LENOVO_BACKUP_RESTORE_DATA);
	stLenovoBackupRestoreData.VariableData = 0;
	EFI_GUID  gBkRsVariableGuid = LENOVO_DM_VARIABLE;

	if (NULL != gRT)
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]Call SetVariable.\n");
		Status = gRT->SetVariable(
			LENOVO_BACKUP_RESTORE_NAME,
			&gBkRsVariableGuid,
			EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
			VariableSize,
			&stLenovoBackupRestoreData
		);
		MyLog(LOG_DEBUG, L"[AUTOBR]SetVariable Status:%x.\n", Status);
		if (EFI_ERROR(Status))
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]SetVariable ERROR! Status:%x\n", Status);
		}
		else
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]SetVariable SUC!\n");
		}
	}
}

void ClearAutoFuncFlagInConfigFile()
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter ClearAutoFuncFlagInConfigFile.\n");
	BOOL bRet = FALSE;
	UINT32 iIndex = 0;

	BOOL bHasSupportedPartition = FALSE;
	BOOL bInsufficientSpace = FALSE;
	BYTE Window_Recovery_Par_TypeGuid[16] = { 0xa4, 0xbb, 0x94, 0xde, 0xd1, 0x06, 0x40, 0x4d, 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac };

	WORD diskindex = 0;
	FILE_SYS_INFO  sysinfo;

	for (iIndex = 0; iIndex < g_dwTotalPartitions; iIndex++)
	{
		MyLog(LOG_DEBUG, L"Partition index: %d, Label %s\n", iIndex, g_Partitions[iIndex].Label);

		//不支持mbr
		if (StrnCmp(g_Partitions[iIndex].Label, L"LENOVO_PART", 11) == 0 &&
			MyMemCmp(Window_Recovery_Par_TypeGuid, g_Partitions[iIndex].TypeGuid, 16) == 0)
		{
			//判断值
			FILE_HANDLE_INFO filehandle;
			FILE_SYS_INFO  sysinfo;
			if (FileSystemInit(&sysinfo, iIndex, TRUE) == TRUE)
			{
				if (!FileOpen(&sysinfo, &filehandle, L"\\okr\\OKRFunKey", EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0, FALSE))
				{
					MyLog(LOG_DEBUG, L"OKRFunKeyFile FileOpen FAILED.\n");
				}
				else
				{
					MyLog(LOG_DEBUG, L"found OKRFunKey flag file\n");
					CHAR16 valueSet[16] = { 0 };
					int iValue = 0;
					SPrint(valueSet, 16, L"flag=%d\r\n", iValue);
					FileSetPos(&filehandle, 0);
					if (FileWrite(&filehandle, StrLen(valueSet) * 2, valueSet))
					{
						MyLog(LOG_DEBUG, L"FileWrite valueSet: %s Suc.\n", valueSet);
					}
					else
					{
						MyLog(LOG_DEBUG, L"FileWrite value: %s Failed!\n", valueSet);
					}
					FileClose(&filehandle);
				}

				FileSystemClose(&sysinfo);
			}
			else
			{
				MyLog(LOG_DEBUG, L"OKRFunKeyFile FileSystemInit FAILED\n");
			}
			//MyLog(LOG_DEBUG, L"OKRFunKeyFile value: %d\n", *value);
			break;
		}
	}

	//MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoFuncFlagFromConfigFile value:%x.\n", *value);
	//MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoFuncFlagFromConfigFile bRet:%d.\n", bRet);
	//return bRet;

}


//判断是否出厂备份，通过返回值返回
//TRUE: 存在
//FALSE: 不存在
//参数iLenovoPart返回服务分区的索引
BOOL AutoBRHasFactoryBackup(int* iLenovoPart)
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter AutoBRHasFactoryBackup.\n");
	int iLenovoPartIndex = 0;
	BYTE bFound = FALSE;
	int i;
	FILE_SYS_INFO  sysinfo;
	PBYTE buffer;

	buffer = myalloc(1024 * 2048);
	if (!buffer) {
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup, alloc memory failed.\n");
		return FALSE;
	}

	MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup,g_dwTotalPartitions:%d.\n",g_dwTotalPartitions);
	for (i = 0; i < g_dwTotalPartitions; i++) {
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup,BackupDiskNumber:%d,DiskNumber:%d,i:%d,bIfBackup:%d,PartitionType:0x%x,Label:%s\n", g_data.BackupDiskNumber, g_Partitions[i].DiskNumber, i, g_Partitions[i].bIfBackup, g_Partitions[i].PartitionType, g_Partitions[i].Label);
		if (g_Partitions[i].bIfBackup)
		{
			continue;
		}

		if ((g_Partitions[i].PartitionType != FAT32)
			&& (g_Partitions[i].PartitionType != FAT32_E)
			&& (g_Partitions[i].PartitionType != NTFS))
			continue;

		if (DivU64x32(g_Partitions[i].TotalSectors, 2048 * 1024) < 4)	//分区大小小于4GB，可以忽略
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup size less than 4G\n");
			continue;
		}

		//Factory backup store in LENOVO_PART part.
		if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) != 0)
		{
			continue;
		}
		*iLenovoPart = i;
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup call FileSystemInit in utils\n");
		if (FileSystemInit(&sysinfo, i, TRUE) != FALSE) {
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup call HasFactoryBackupInPartition in func\n");
			if (HasFactoryBackupInPartition(&sysinfo, i, buffer))
			{
				bFound = TRUE;
				
				MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup HasFactoryBackupInPartition Found, iLenovoPart:%s\n", *iLenovoPart);
			}
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup PartitionIndex(%d) FoundRes:%d.\n", i, bFound);

			FileSystemClose(&sysinfo);
		}
		else {
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup, call FileSystemInit(%d) failed.\n", i);
		}
		break;
	}

	myfree(buffer);

	MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRHasFactoryBackup ret:%d, iLenovoPart:%d.\n", bFound, *iLenovoPart);
	return bFound;
}

//return:TRUE有数据分区，FALSE没有数据分区
//available:TRUE有足够空间存储备份文件，FALSE没有足够空间存储备份文件
//iAvailablePartIndex:可用空间最大的数据分区
BOOL AutoBRDetectAvailableDataPartition(int* iMaxFreePartIndex, BOOL* bHasBitLocker, BOOL* available)
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter AutoBRDetectAvailableDataPartition.\n");
	BOOL bRet = FALSE;
	int i;
	ULONGLONG ulFreeSectors = 0;
	BOOL bHasFreeDataPart = FALSE;
	MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRDetectAvailableDataPartition g_dwTotalPartitions：%d.\n", g_dwTotalPartitions);
	MyLog(LOG_DEBUG, L"g_dwTotalPartitions：%d.\n", g_dwTotalPartitions);
	for (i = 0; i < g_dwTotalPartitions; i++)
	{
		//排除移动存储设备。
		MyLog(LOG_DEBUG, L"uiDevicePathProtocolType=%d, uiDevicePathProtocolSubType=%d.\n", g_disklist[g_Partitions[i].DiskNumber].uiDevicePathProtocolType, g_disklist[g_Partitions[i].DiskNumber].uiDevicePathProtocolSubType);
		if ((g_disklist[g_Partitions[i].DiskNumber].uiDevicePathProtocolType == 3) && (g_disklist[g_Partitions[i].DiskNumber].uiDevicePathProtocolSubType == 5))
		{
			continue;
		}
		
		MyLog(LOG_DEBUG, L"index:%d, Label：%s.\n", i, g_Partitions[i].Label);
		if (MyMemCmp(AUTOBR_Windows_Data_TypeGuid, g_Partitions[i].TypeGuid, 16) == 0 && g_Partitions[i].HasWindows != TRUE)
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]index:%d, Lable:%s is Data partition.\n", i, g_Partitions[i].Label);
			MyLog(LOG_DEBUG, L"[AUTOBR]Current ulFreeSectors：%lld.\n", ulFreeSectors);
			bRet = TRUE;	//存在数据分区
			if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]Partition i:%d has BitLocker.\n", i);
				*bHasBitLocker = TRUE;
			}
			else if (g_Partitions[i].FreeSectors > ulFreeSectors)
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]Current max free data partition index:%d.\n", i);
				bHasFreeDataPart = TRUE;
				ulFreeSectors = g_Partitions[i].FreeSectors;
				*iMaxFreePartIndex = i;
			}

		}
	}

	if (bHasFreeDataPart)
	{
			MyLog(LOG_DEBUG, L"[AUTOBR]Free Sectors:%d.\n", g_Partitions[*iMaxFreePartIndex].FreeSectors);
			MyLog(LOG_DEBUG, L"[AUTOBR]Backup need Sectores:%d.\n", DivU64x32(g_data.TotalBackupSize, 731));
			*available = g_Partitions[*iMaxFreePartIndex].FreeSectors >= DivU64x32(g_data.TotalBackupSize, 731);
	}
	MyLog(LOG_DEBUG, L"[AUTOBR]AutoBRDetectAvailableDataPartition Finish, bHasDataPart:%d, MaxFreePartIndex:%d, DataPartHasBitLocker:%d, SizeCanBackup:%d.\n", bRet, *iMaxFreePartIndex, *bHasBitLocker, *available);
	return bRet;
}

BOOL IsPartHasEnoughSpace(int iPartIndex)
{
	MyLog(LOG_DEBUG, L"[AUTOBR]IsPartHasEnoughSpace:%lld. \n", g_Partitions[iPartIndex].FreeSectors);
	MyLog(LOG_DEBUG, L"[AUTOBR]backupsector:%lld\n", DivU64x32(g_data.TotalBackupSize, 731));
	MyLog(LOG_DEBUG, L"[AUTOBR]g_data.TotalBackupSize:%lld\n",  g_data.TotalBackupSize);
	//getch();
	return g_Partitions[iPartIndex].FreeSectors >= DivU64x32(g_data.TotalBackupSize, 731);
}

void SaveAutoBackupFileLocation(int iBackupPartIndex)
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter SaveAutoBackupFileLocation.\n");
	//从iBackupPartIndex获取用于定位存储位置的信息
	MyLog(LOG_DEBUG, L"Search backup img info by part index：%d.\n", iBackupPartIndex);
	CHAR8 disksn[20] = { 0 };
	//BYTE* UniGuid = NULL;
	ULONGLONG BootBegin = 0;;
	DWORD Time = g_data.BackupImgTime;
	MyLog(LOG_DEBUG, L"iBackupPartIndex=%d, g_dwTotalPartitions=%d.\n", iBackupPartIndex, g_dwTotalPartitions);
	//getch();
	if (iBackupPartIndex < g_dwTotalPartitions)
	{
		BYTE iDiskIndex = g_Partitions[iBackupPartIndex].DiskNumber;
		MyLog(LOG_DEBUG, L"iDiskIndex=%d, g_disknum=%d.\n", iDiskIndex, g_disknum);

		if (iDiskIndex < g_disknum)
		{
			MyLog(LOG_DEBUG, L"disksn found =%a.\n", g_disklist[iDiskIndex].disksn);
			//disksn = g_disklist[iDiskIndex].disksn;
			StrnCpy(disksn, g_disklist[iDiskIndex].disksn, StrLen(g_disklist[iDiskIndex].disksn));
		}
		//UniGuid = g_Partitions[iBackupPartIndex].UniGuid;
		BootBegin = g_Partitions[iBackupPartIndex].BootBegin;
	}
	MyLog(LOG_DEBUG, L"disksn=%a.\n", disksn);
	//MyLog(LOG_DEBUG, L"UniGuid=%x.\n", UniGuid);
	MyLog(LOG_DEBUG, L"BootBegin=%lld.\n", BootBegin);
	MyLog(LOG_DEBUG, L"Time=%lld.\n", Time);
	//getch();

	//写入此次备份文件存储的路径
	//用不用先把之前配置文件内容清空？
	UINT32 iIndex = 0;
	BOOL bHasSupportedPartition = FALSE;
	BOOL bInsufficientSpace = FALSE;
	BYTE Window_Recovery_Par_TypeGuid[16] = { 0xa4, 0xbb, 0x94, 0xde, 0xd1, 0x06, 0x40, 0x4d, 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac };

	WORD diskindex = 0;
	FILE_SYS_INFO  sysinfo;
	MyLog(LOG_DEBUG, L"[AUTOBR]Find Lenovo Part.\n");
	for (iIndex = 0; iIndex < g_dwTotalPartitions; iIndex++)
	{
		MyLog(LOG_DEBUG, L"Partition index: %d, Label %s\n", iIndex, g_Partitions[iIndex].Label);

		//不支持mbr
		if (StrnCmp(g_Partitions[iIndex].Label, L"LENOVO_PART", 11) == 0 &&
			MyMemCmp(Window_Recovery_Par_TypeGuid, g_Partitions[iIndex].TypeGuid, 16) == 0)
		{
			//判断值
			FILE_HANDLE_INFO filehandle;
			FILE_SYS_INFO  sysinfo;
			if (FileSystemInit(&sysinfo, iIndex, TRUE) == TRUE)
			{
				if (!FileOpen(&sysinfo, &filehandle, L"\\okr\\AutoBackupInfo", EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0, FALSE))
				{
					MyLog(LOG_DEBUG, L"AutoBackupInfo FileOpen FAILED.\n");
				}
				else
				{
					MyLog(LOG_DEBUG, L"found AutoBackupInfo file\n");
					//logflush(FALSE);
					//写文件
					//sn是char直接写，length就是长度
					//L""是CHAR16，length是长度*2
					//
					CHAR16 pSN[100] = { 0 };
					CHAR16 pBootBegin[100] = { 0 };
					CHAR16 pTime[100] = { 0 };
					SPrint(pSN, 100, L"SN=%a\r\n", disksn);
					SPrint(pBootBegin, 100, L"BootBegin=%lld\r\n", BootBegin);
					SPrint(pTime, 100, L"Time=%lld\r\n", Time);
					int iPos = 0;
					MyLog(LOG_DEBUG, L"11111111111111\n");
					//logflush(FALSE);
					FileSetPos(&filehandle, iPos);
					int iLength = StrLen(pBootBegin) * 2;
					FileWrite(&filehandle, iLength, pBootBegin);
					MyLog(LOG_DEBUG, L"22222222222222222\n");
					//logflush(FALSE);
					iPos += iLength;
					FileSetPos(&filehandle, iPos);
					MyLog(LOG_DEBUG, L"3333333333333333\n");
					//logflush(FALSE);
					iLength = StrLen(pTime) * 2;
					FileWrite(&filehandle, iLength, pTime);
					MyLog(LOG_DEBUG, L"44444444444444444\n");
					//logflush(FALSE);
					iPos += iLength;
					FileSetPos(&filehandle, iPos);
					MyLog(LOG_DEBUG, L"55555555555555555\n");
					//logflush(FALSE);
					iLength = StrLen(pSN) * 2;
					FileWrite(&filehandle, iLength, pSN);
					MyLog(LOG_DEBUG, L"6666666666666666\n");
					//logflush(FALSE);
					/*
					UINT32 loglength = 0;
					UINT32 logbufsize = 1024 * 64;
					char* logbuf = myalloc(logbufsize);
					mymemset(logbuf, 'X', logbufsize);

					CHAR16 buffer[1024] = { 0 };
					//CHAR16 partname[100] = { 0 };
					CHAR8 pdisksn[32] = { 0 };
					//CHAR16 pUniGuid[32] = { 0 };
					
					CHAR16 pPreFix[8] = L"disksn=";// { 0 };
					//SPrint(pPreFix, 8, );

					CHAR16 partname[32] = { 0 };
					CHAR16 diskSN16[32];
					Ascii2Unicode(diskSN16, disksn);
					//disksn
					int a = 0;
					SPrint(partname, 32, L"disksn=%a\r\n", diskSN16);
					CHAR16 *Format = partname;

					DWORD iDiskSnL = (DWORD)StrLength(disksn);
					DWORD iTotalL = 7 + iDiskSnL;
					CHAR16* pSNBuffer = myalloc(iTotalL + 1);
					MyLog(LOG_DEBUG, L"StrLength:%d, iTotalL:%d\n", iDiskSnL, iTotalL);
					SPrint(pSNBuffer, iTotalL, L"disksn=%a", disksn);
					//SPrint(pUniGuid, 32, L"UniGuid=%x\r\n", UniGuid);

					


					//SPrint(partname, 100, L"disksn=%s\r\nUniGuid=%s\r\nBootBegin=%lld\r\nTime=%lld\r\n", disksn, UniGuid, BootBegin, Time);
					//CHAR16 *Format = partname;

					VA_LIST Marker;
					VA_START(Marker, Format);
					UINT32 len1 = UnicodeVSPrint(buffer, 1024, Format, Marker);
					VA_END(Marker);
					MyLog(LOG_DEBUG, L"len1:%d, buffer:%s\n", len1, buffer);
					DWORD len;
					len = (DWORD)StrLength(buffer);
					MyLog(LOG_DEBUG, L"len:%d, buffer:%s\n", len, buffer);
					if (len + 0 + 2 < logbufsize) {
						Unicode2Ascii(logbuf, buffer);
						loglength += len;
						MyLog(LOG_DEBUG, L"loglength:%d, logbuf:%s\n", loglength, logbuf);
						if (buffer[len - 1] != '\n') {
							logbuf[len++] = '\r';
							logbuf[len++] = '\n';
							MyLog(LOG_DEBUG, L"logbuf:%s\n", logbuf);
						}
					}
					//DWORD lenBuf;
					//lenBuf = (DWORD)StrLength(partname);
					////*value = 0;
					//DWORD len = ((lenBuf + 511) & ~511);
					FileSetPos(&filehandle, 0);
					//MyLog(LOG_DEBUG, L"loglength:%d, logbuf:%s\n", loglength, logbuf);
					FileWrite(&filehandle, loglength, logbuf);
					UINT32 newPos = loglength;
					FileSetPos(&filehandle, newPos);
					/*
					VA_START(Marker, pUniGuid);
					len1 = UnicodeVSPrint(buffer, 1024, pUniGuid, Marker);
					VA_END(Marker);
					MyLog(LOG_DEBUG, L"len1:%d, buffer:%s\n", len1, buffer);
					len = (DWORD)StrLength(buffer);
					MyLog(LOG_DEBUG, L"len:%d, buffer:%s\n", len, buffer);
					loglength = 0;
					if (newPos + len + 0 + 2 < logbufsize) {
						Unicode2Ascii(logbuf, buffer);
						loglength += len;
						MyLog(LOG_DEBUG, L"loglength:%d, logbuf:%s\n", loglength, logbuf);
						if (buffer[len - 1] != '\n') {
							logbuf[len++] = '\r';
							logbuf[len++] = '\n';
							MyLog(LOG_DEBUG, L"logbuf:%s\n", logbuf);
						}
					}
					FileWrite(&filehandle, loglength, logbuf);
					newPos += loglength;
					FileSetPos(&filehandle, newPos);
					*/
					/*
					VA_START(Marker, pBootBegin);
					len1 = UnicodeVSPrint(buffer, 1024, pBootBegin, Marker);
					VA_END(Marker);
					MyLog(LOG_DEBUG, L"len1:%d, buffer:%s\n", len1, buffer);
					len = (DWORD)StrLength(buffer);
					MyLog(LOG_DEBUG, L"len:%d, buffer:%s\n", len, buffer);
					loglength = 0;
					if (newPos + len + 0 + 2 < logbufsize) {
						Unicode2Ascii(logbuf, buffer);
						loglength += len;
						MyLog(LOG_DEBUG, L"loglength:%d, logbuf:%s\n", loglength, logbuf);
						if (buffer[len - 1] != '\n') {
							logbuf[len++] = '\r';
							logbuf[len++] = '\n';
							MyLog(LOG_DEBUG, L"logbuf:%s\n", logbuf);
						}
					}
					FileWrite(&filehandle, loglength, logbuf);
					newPos += loglength;
					FileSetPos(&filehandle, newPos);

					VA_START(Marker, pTime);
					len1 = UnicodeVSPrint(buffer, 1024, pTime, Marker);
					VA_END(Marker);
					MyLog(LOG_DEBUG, L"len1:%d, buffer:%s\n", len1, buffer);
					len = (DWORD)StrLength(buffer);
					MyLog(LOG_DEBUG, L"len:%d, buffer:%s\n", len, buffer);
					loglength = 0;
					if (newPos + len + 0 + 2 < logbufsize) {
						Unicode2Ascii(logbuf, buffer);
						loglength += len;
						MyLog(LOG_DEBUG, L"loglength:%d, logbuf:%s\n", loglength, logbuf);
						if (buffer[len - 1] != '\n') {
							logbuf[len++] = '\r';
							logbuf[len++] = '\n';
							MyLog(LOG_DEBUG, L"logbuf:%s\n", logbuf);
						}
					}
					FileWrite(&filehandle, loglength, logbuf);
					//EFI_STATUS Status = (filehandle.filehandle)->Read(filehandle.filehandle, &uiSize, (VOID*)buffer);
					//linw:这么写成0对不对？
					MyLog(LOG_DEBUG, L"Write Complete.\n");
					/*
					UINT64 BufferSize = StrLen(partname) * 2;
					MyLog(LOG_DEBUG, L"BufferSize:%d  partname:%s.\n", BufferSize, partname);
					VOID *bitbuffer = &vKey;
					SetMem(bitbuffer, sizeof(BYTE), 0);
					//after_alignlen = CalcAliginSize(1);
					UINT64 position = 0;
					EFI_STATUS Status = (filehandle.filehandle)->GetPosition(filehandle.filehandle, &position);
					MyLog(LOG_DEBUG, L"GetPosition Status:%d  position:%d.\n", Status, position);
					Status = (filehandle.filehandle)->SetPosition(filehandle.filehandle, 0);
					MyLog(LOG_DEBUG, L"SetPosition Status:%d.\n", Status);
					Status = (filehandle.filehandle)->Write(filehandle.filehandle, BufferSize, partname);
					MyLog(LOG_DEBUG, L"Write Status:%d.\n", Status);
					Status = (filehandle.filehandle)->Flush(filehandle.filehandle);
					MyLog(LOG_DEBUG, L"Flush Status:%d.\n", Status);
					if (!EFI_ERROR(Status))
					{
						MyLog(LOG_DEBUG, L"Status: %d\n", Status);
						//MyLog(LOG_DEBUG, L"buffer: %s\n", (CHAR16*)buffer);
						//if (uiSize < 1024 && uiSize > 0)
						//{
						//	UINTN uiFlag = uiSize;
						//	while (uiFlag >= 1)
						//	{
						//		if (buffer[uiFlag - 1] >= L'0' && buffer[uiFlag - 1] <= L'9')
						//		{
						//			//MyLog(LOG_DEBUG, L"uiFlag - 1: %d buffer[uiFlag - 1] : %d\n", uiFlag - 1, buffer[uiFlag - 1]);

						//			Type[0] = buffer[uiFlag - 1];

						//			*value = StrDecimalToUintn(Type);
						//			MyLog(LOG_DEBUG, L"GetAutoFuncFlagFromConfigFile value in file: %d\n", *value);
						//			break;
						//		}

						//		uiFlag--;
						//	}
						//}

					}
					else
					{
						MyLog(LOG_DEBUG, L"OKRFunKeyFile FileWrite FAILED\n");
					}
					*/
				}

				FileClose(&filehandle);
				FileSystemClose(&sysinfo);
			}
			else
			{
				MyLog(LOG_DEBUG, L"OKRFunKeyFile FileSystemInit FAILED\n");
			}
			//MyLog(LOG_DEBUG, L"OKRFunKeyFile value: %d\n", *value);
			break;
		}
	}
	//logflush(FALSE);
}

void DeletePreAutoBackupFile()
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter DeleteAutoBackupFile.\n");
	BYTE Window_Recovery_Par_TypeGuid[16] = { 0xa4, 0xbb, 0x94, 0xde, 0xd1, 0x06, 0x40, 0x4d, 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac };
	//找到之前镜像的索引
	int img_count = 0;
	GetAllBackupFiles(&img_count);	//g_data.ImageFilesNumber是总数
	CheckAllBackupFiles(); //linw:error的已经删除了，已经是最终的；
	//getch();
	//从配置文件获取自动备份文件的信息，sn，guid，
	CHAR8 disksn[20] = { 0 };
	//BYTE UniGuid[16] = { 0 };
	ULONGLONG BootBegin = 0;
	DWORD Time = 0;

	if (GetAutoBackupLocationFromConfigFile(disksn, &BootBegin, &Time))
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupConfigFile Found.\n");
		int iIndexImg = 0;
		//linw:是有效镜像的总数,按时间排好了
		for (iIndexImg = 0; iIndexImg < g_data.ImageFilesNumber; iIndexImg++)
		{
			if (g_data.images[iIndexImg].Time == Time)
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]iIndexImg:%d.\n", iIndexImg);
				//找到了上次自动备份的镜像，磁盘和分区都没有变化
				WORD partindex = (WORD)g_data.images[iIndexImg].PartitionIndex;
				MyLog(LOG_DEBUG, L"[AUTOBR]partindex:%d.\n", partindex);
				if ((MyMemCmp(disksn, g_disklist[g_Partitions[partindex].DiskNumber].disksn, 20) == 0)
					&& (g_Partitions[partindex].BootBegin == BootBegin))
				{
					MyLog(LOG_DEBUG, L"[AUTOBR]partindex:%d.\n", partindex);
					//如果上次备份的镜像存储在Lenovo Part，证明上次备份成了出厂备份，则不删除
					if (StrnCmp(g_Partitions[partindex].Label, L"LENOVO_PART", 11) == 0 &&
						MyMemCmp(Window_Recovery_Par_TypeGuid, g_Partitions[partindex].TypeGuid, 16) == 0)
					{
						MyLog(LOG_DEBUG, L"[AUTOBR]PreAutoBackup is Factory Backup. Don't delete the img.\n");
						break;
					}
					FILE_SYS_INFO  sysinfo;
					if (FileSystemInit(&sysinfo, partindex, FALSE) != FALSE)
					{
						MyLog(LOG_DEBUG, L"[AUTOBR]FileSystemInit Suc.\n");
						CHAR16 name[100] = { 0 };
						DWORD count = g_data.images[iIndexImg].FileNumber;
						MyLog(LOG_DEBUG, L"[AUTOBR]images.FileNumber:%d.\n", count);
						DWORD i;
						BOOL bDeleted = FALSE;
						if (!count)
							count = 1;
						for (i = 0; i < count; i++)
						{
							GetBackupFileFullPath(&sysinfo, partindex, g_data.images[iIndexImg].FileName, i, name, 100);
							bDeleted = FileDelete(&sysinfo, name);
						}
						if (bDeleted)
						{
							MyLog(LOG_DEBUG, L"[AUTOBR]Delete Suc.\n");
							//删除备份点以后，就要增加这个文件所在的分区的剩余空间
							g_Partitions[partindex].FreeSectors += g_data.images[iIndexImg].FileSize / SECTOR_SIZE;
						}

						FileSystemClose(&sysinfo);
					}
					break;
				}
				else
				{
					MyLog(LOG_DEBUG, L"[AUTOBR]Img position Changed!!!.\n");
				}
			}
			else
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]No Match PreBackup Img Found!!!.\n");
			}
		}
	}
	else
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]No PreBackup Img config file Found!!!.\n");
	}
}

BOOL DetectRestoreImge(WORD* recoverfilepartindex, DWORD* backupfileID)
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter DetectRestoreImge.\n");
	//getch();
	BOOL bCallRecoverFunc = FALSE;
	BOOL bNeedGetLatestAutoBackupFileFromLenovoPart = FALSE;
	BOOL bConfigFileFound = FALSE;
	BOOL bUseFactoryImg = FALSE;
	int img_count = 0;
	GetAllBackupFiles(&img_count);	//g_data.ImageFilesNumber是总数
	CheckAllBackupFiles(); //linw:error的已经删除了，已经是最终的；
	//从配置文件获取自动备份文件的信息，sn，guid，
	CHAR8 disksn[20] = { 0 };

	//BYTE UniGuid[16] = { 0 };
	ULONGLONG BootBegin = 0;
	DWORD Time = 0;
	BOOL bLastBkImgPartExistBitLock = FALSE;

	if (GetAutoBackupLocationFromConfigFile(disksn, &BootBegin, &Time))
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoBackupLocationFromConfigFile.\n");
		MyLog(LOG_DEBUG, L"[AUTOBR]disksn:%a.\n", disksn);
		MyLog(LOG_DEBUG, L"[AUTOBR]BootBegin:%d.\n", BootBegin);
		MyLog(LOG_DEBUG, L"[AUTOBR]Time:%d.\n", Time);
		//getch();
		//存在配置文件
		bConfigFileFound = TRUE;
		//找到上次备份的镜像保存到的分区index
		int iDiskIndex = 0;
		int iPartIndex = 0;
		BOOL bDiskFound = FALSE;
		BOOL bPartitionFound = FALSE;
		for (iDiskIndex = 0; iDiskIndex < g_disknum; iDiskIndex++)
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]iDiskIndex:%d, SN:%a.\n", iDiskIndex, g_disklist[iDiskIndex].disksn);
			if ((disksn != NULL) && (MyMemCmp(disksn, g_disklist[iDiskIndex].disksn, 20) == 0))
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]bDiskFound iDiskIndex:%d.\n", iDiskIndex);
				bDiskFound = TRUE;
				break;
			}
		}
		//getch();
		if (bDiskFound)
		{
			//DiskNumber是disk的index
			for (iPartIndex = 0; iPartIndex < g_dwTotalPartitions; iPartIndex++)
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]iPartIndex:%d, DiskNumber:%d.\n", iPartIndex, g_Partitions[iPartIndex].DiskNumber);
				MyLog(LOG_DEBUG, L"[AUTOBR]BootBegin:%d.\n", g_Partitions[iPartIndex].BootBegin);
				if ((g_Partitions[iPartIndex].DiskNumber == iDiskIndex)
					&& (g_Partitions[iPartIndex].BootBegin == BootBegin))
				{
					MyLog(LOG_DEBUG, L"[AUTOBR]bPartitionFound iPartIndex:%d.\n", iPartIndex);
					bPartitionFound = TRUE;
					break;
				}
			}
		}
		//指定的分区是否有BitLocker加密
		if (bPartitionFound)
		{
			if (iPartIndex < g_dwTotalPartitions)
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]iPartIndex:%d, PartitionType:%x.\n", iPartIndex, g_Partitions[iPartIndex].PartitionType);
				if (g_Partitions[iPartIndex].PartitionType == SHD_BITLOCKER || g_Partitions[iPartIndex].PartitionType == SHD_BITLOCKERTOGO)
				{
					MyLog(LOG_DEBUG, L"[AUTOBR]Recover file locatin in config file has BitLocker.\n");
					//存在bitlocker，从联想服务分区获取镜像
					bNeedGetLatestAutoBackupFileFromLenovoPart = TRUE;
					bLastBkImgPartExistBitLock = TRUE;
				}
				else
				{
					MyLog(LOG_DEBUG, L"[AUTOBR]Recover file not BitLocked.\n");
					//不存在bitlocker，确定在当前环境下的Part index和id
					int iIndexImg = 0;
					//linw:是有效镜像的总数,按时间排好了
					for (iIndexImg = 0; iIndexImg < g_data.ImageFilesNumber; iIndexImg++)
					{
						//匹配同磁盘同分区下的同时间img
						if ((g_data.images[iIndexImg].Time == Time) &&
							(g_data.images[iIndexImg].PartitionIndex == iPartIndex) &&
							(g_Partitions[iPartIndex].DiskNumber == iDiskIndex))
						{
							MyLog(LOG_DEBUG, L"[AUTOBR]Img Match.\n");
							//然后进入恢复流程
							bCallRecoverFunc = TRUE;
							*recoverfilepartindex = g_data.images[iIndexImg].PartitionIndex;
							*backupfileID = iIndexImg;
							break;
						}
					}
					if (!bCallRecoverFunc)
					{
						//按配置文件信息，没有找到之前的备份镜像
						//从出厂备份恢复？
						bNeedGetLatestAutoBackupFileFromLenovoPart = TRUE;
					}

				}
			}
			else
			{
				//根据配置文件的记录信息，没有找到对应的镜像文件
				MyLog(LOG_DEBUG, L"[AUTOBR]Recover file not found-Partition index too large.\n");
				bNeedGetLatestAutoBackupFileFromLenovoPart = TRUE;
			}

		}
		else
		{
			//根据配置文件的记录信息，没有找到对应的镜像文件
			MyLog(LOG_DEBUG, L"[AUTOBR]Recover file not found-Partition not Found.\n");
			bNeedGetLatestAutoBackupFileFromLenovoPart = TRUE;
		}
	}
	else
	{
		//不存在配置文件，从联想服务分区获取镜像
		MyLog(LOG_DEBUG, L"[AUTOBR]GetAutoBackupLocationFromConfigFile Failed!\n");
		//getch();
		bNeedGetLatestAutoBackupFileFromLenovoPart = TRUE;
	}

	if (bNeedGetLatestAutoBackupFileFromLenovoPart)
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]NeedGetLatestAutoBackupFileFromDataPart\n");
		//getch();
		if (GetFactoryImageFromLenPart(recoverfilepartindex, backupfileID))
		{
			//有出厂备份，用出厂备份去恢复
			MyLog(LOG_DEBUG, L"[AUTOBR]GetLatestAutoBackupFileFromDataPart Suc.\n");
			MyLog(LOG_DEBUG, L"[AUTOBR]Use Factory BackupFile to Recover.\n");
			//getch();
			//
			bUseFactoryImg = TRUE;
			bCallRecoverFunc = TRUE;
		}
		else
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]GetLatestAutoBackupFileFromDataPart Failed.\n");
			//没有出厂备份
			//上次自动备份的镜像存储的分区是否存在BitLocker
			if (bConfigFileFound && bLastBkImgPartExistBitLock)
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]Last AutoBackup's Partition Exist BitLocker.\n");
				MyLog(LOG_DEBUG, L"[AUTOBR]Promp exist BitLocker.\n");
				//存在
				//提示用户：存储镜像的分区存在Bitlocker，在Windows中关闭BitLocker加密后再试。
				DrawMsgBoxEx(STRING_AUTOBR_IMG_BITLOCKED, IDOK, -1, TYPE_INFO);
			}
			else
			{
				MyLog(LOG_DEBUG, L"[AUTOBR]Last AutoBackup's Partition Do Not Exist BitLocker.\n");
				MyLog(LOG_DEBUG, L"[AUTOBR]Promp no Backup File Found.\n");
				//不存在：提示
				//提示用户: 未找到系统备份镜像文件，请检查确认后再试
				DrawMsgBoxEx(STRING_AUTOBR_IMG_NOTFOUND, IDOK, -1, TYPE_INFO);
			}
		}
	}
	MyLog(LOG_DEBUG, L"[AUTOBR]bImgFound:%d, bExistConfig:%d, bPartbitlocked:%d, bUseFactoryImg:%d,.\n", bCallRecoverFunc, bConfigFileFound, bLastBkImgPartExistBitLock, bUseFactoryImg);
	//logflush(FALSE);
	return bCallRecoverFunc;
}

//bRet: TURE:存在配置文件
//FALSE：不存在配置文件
//配置文件里记载的镜像文件
BOOL GetAutoBackupLocationFromConfigFile(CHAR8* disksn, ULONGLONG* BootBegin, DWORD* Time)
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter GetAutoBackupLocationFromConfigFile.\n");
	BOOL bRet = FALSE;
	BYTE Window_Recovery_Par_TypeGuid[16] = { 0xa4, 0xbb, 0x94, 0xde, 0xd1, 0x06, 0x40, 0x4d, 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac };
	//getch();
	int iIndex = 0;
	FILE_SYS_INFO  sysinfo;
	MyLog(LOG_DEBUG, L"[AUTOBR]Find Lenovo Part.\n");
	for (iIndex = 0; iIndex < g_dwTotalPartitions; iIndex++)
	{
		MyLog(LOG_DEBUG, L"Partition index: %d, Label %s\n", iIndex, g_Partitions[iIndex].Label);

		//不支持mbr
		if (StrnCmp(g_Partitions[iIndex].Label, L"LENOVO_PART", 11) == 0 &&
			MyMemCmp(Window_Recovery_Par_TypeGuid, g_Partitions[iIndex].TypeGuid, 16) == 0)
		{
			//getch();
			//判断值
			FILE_HANDLE_INFO filehandle;
			FILE_SYS_INFO  sysinfo;
			if (FileSystemInit(&sysinfo, iIndex, TRUE) == TRUE)
			{
				BOOL bFileExitRet = IsFileExistEx(&sysinfo, L"\\okr\\AutoBackupInfo");
				if (bFileExitRet)
				{
					MyLog(LOG_DEBUG, L"AutoBackupInfo exist\n");
					if (!FileOpen(&sysinfo, &filehandle, L"\\okr\\AutoBackupInfo", EFI_FILE_MODE_READ, 0, FALSE))
					{
						//getch();
						MyLog(LOG_DEBUG, L"AutoBackupInfo FileOpen FAILED.\n");
					}
					else
					{
						MyLog(LOG_DEBUG, L"found AutoBackupInfo file in GetAutoBackupLocationFromConfigFile\n");
						//getch();
						//读文件
						
						UINTN uiSize = 1024;
						CHAR8 buffer[1024] = { 0 };
						EFI_STATUS Status = (filehandle.filehandle)->Read(filehandle.filehandle, &uiSize, (VOID*)buffer);
						if (!EFI_ERROR(Status))
						{
							MyLog(LOG_DEBUG, L"uiSize: %d\n", uiSize);
							MyLog(LOG_DEBUG, L"buffer: %s\n", (CHAR16*)buffer);
							if (0 != uiSize)
							{
								CHAR16 snPreFix[4] = L"SN=";
								CHAR16 bootBeginPreFix[11] = L"BootBegin=";
								CHAR16 timePreFix[6] = L"Time=";
								CHAR16* pbootBeginContent = NULL;
								CHAR16* ptimeContent = NULL;
								CHAR16* psnContent = NULL;
								CHAR16 snValue[100] = { 0 };
								CHAR16 bootBeginValue[100] = { 0 };
								CHAR16 timeValue[100] = { 0 };
								pbootBeginContent = StrStr(buffer, bootBeginPreFix);
								ptimeContent = StrStr(buffer, timePreFix);
								psnContent = StrStr(buffer, snPreFix);
								if ((NULL != pbootBeginContent) && (NULL != ptimeContent) && (NULL != psnContent))
								{
									StrnCpy(bootBeginValue, pbootBeginContent + StrLen(bootBeginPreFix), StrLen(buffer) - StrLen(bootBeginPreFix) - StrLen(ptimeContent));
									StrnCpy(timeValue, ptimeContent + StrLen(timePreFix), StrLen(ptimeContent) - StrLen(timePreFix) - StrLen(psnContent));
									StrnCpy(snValue, psnContent + StrLen(snPreFix), StrLen(psnContent) - StrLen(snPreFix));
									Unicode2Ascii(disksn, snValue);
									//StrnCpy(disksn, snValue, StrLen(snValue));
									*BootBegin = StrDecimalToUintn(bootBeginValue);
									*Time = StrDecimalToUintn(timeValue);
									MyLog(LOG_DEBUG, L"pbootBeginContent:%s\n", pbootBeginContent);
									MyLog(LOG_DEBUG, L"ptimeContent:%s\n", ptimeContent);
									MyLog(LOG_DEBUG, L"psnContent:%s\n", psnContent);

									MyLog(LOG_DEBUG, L"StrLen(buffer):%d, StrLen(bootBeginPreFix):%d, StrLen(ptimeContent):%d\n", StrLen(buffer), StrLen(bootBeginPreFix), StrLen(ptimeContent));
									MyLog(LOG_DEBUG, L"StrLen(bootBeginPreFix):%d\n", StrLen(bootBeginPreFix));
									MyLog(LOG_DEBUG, L"bootBeginValue:%s\n", bootBeginValue);
									MyLog(LOG_DEBUG, L"timeValue:%s\n", timeValue);
									MyLog(LOG_DEBUG, L"snValue:%s\n", snValue);
									MyLog(LOG_DEBUG, L"BootBegin:%d\n", *BootBegin);
									MyLog(LOG_DEBUG, L"Time:%d\n", *Time);
									MyLog(LOG_DEBUG, L"disksn:%s\n", disksn);
									bRet = TRUE;
								}
							}
							//getch();
						}
						else
						{
							MyLog(LOG_DEBUG, L"AutoBackupInfo FileRead FAILED\n");
						}
						FileClose(&filehandle);
					}
				}
				else
				{
					MyLog(LOG_DEBUG, L"AutoBackupInfo not exist!!!!\n");
				}
			
				FileSystemClose(&sysinfo);
			}
			else
			{
				MyLog(LOG_DEBUG, L"AutoBackupInfo FileSystemInit FAILED\n");
			}
			//MyLog(LOG_DEBUG, L"OKRFunKeyFile value: %d\n", *value);
			break;
		}
	}

	return bRet;
}

BOOL GetFactoryImageFromLenPart(WORD* recoverfilepartindex, DWORD* backupfileID)
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter GetFactoryImageFromLenPart.g_data.ImageFilesNumber:%d\n", g_data.ImageFilesNumber);

	BOOL bRet = FALSE;
	int iIndexImg = 0;
	//linw:是有效镜像的总数,按时间排好了
	for (iIndexImg = 0; iIndexImg < g_data.ImageFilesNumber; iIndexImg++)
	{
		if (g_data.images[iIndexImg].bFactoryBackup)
		{
			bRet = TRUE;
			*recoverfilepartindex = g_data.images[iIndexImg].PartitionIndex;
			*backupfileID = iIndexImg;
			break;
		}
	}
	MyLog(LOG_DEBUG, L"[AUTOBR]GetFactoryImageFromLenPart bRet：%d, recoverfilepartindex:%d, backupfileID:%d.\n", bRet, *recoverfilepartindex, *backupfileID);

	return bRet;
}
