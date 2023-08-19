/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/

//ssd.c
#include "../okr.h"
#include "ssd.h"

#ifdef _SNAPSHOT_SSD_MODULE_

SNAPSHOT_SSD g_ssd;

/////////////////////////////////////////
void Swap(char *id, UINT32 length)
{
	char ch;
	UINT32 l = 0;
	while (l < length)
	{
		ch = id[l];
		id[l] = id[l+1];
		id[l+1] = ch;
		l += 2;
	}
}

/////////////////////////////////////
#include "AtaAtapiPassThru.c"
#include "AhciMode.c"
#include "IdeMode.c"

BOOLEAN CheckIfDiskMedia(EFI_ATA_PASS_THRU_PROTOCOL * pAtaInterface, 
			UINT16 uiPortNo, UINT16 uiPortMultiplierPort)
{
	BOOLEAN bFound = FALSE;
	EFI_DEVICE_PATH_PROTOCOL * pDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)NULL;
	EFI_ATA_PASS_THRU_BUILD_DEVICE_PATH  pBuildDevicePath = pAtaInterface->BuildDevicePath;

	if (pBuildDevicePath(pAtaInterface, uiPortNo, uiPortMultiplierPort, &pDevicePath))
	{
		Print (L"CheckIfDiskMedia::Error: Device path not found for the device port = %d , PMP = %d...\n", uiPortNo, uiPortMultiplierPort);
		goto end;
	}
	Print(L"CheckIfDiskMedia:: Found the Device Path for portNumber = %d, UlPortNumber = %d \n", uiPortNo, uiPortMultiplierPort);
	Print(L"CheckIfDiskMedia:: Device Type  = %d, Device Subtype  = %d\n", pDevicePath->Type, pDevicePath->SubType);
	// && pDevicePath->SubType == MSG_SATA_DP
	if (pDevicePath->Type == MESSAGING_DEVICE_PATH)
	{
		Print(L"CheckIfDiskMedia:: Got the device with device path MESSAGING_DEVICE_PATH & MSG_SATA_DP values for PORT : %d & Multiport = %d ..."
				,uiPortNo, uiPortMultiplierPort);
		bFound = TRUE;
	}
	else
	{
		Print(L"CheckIfDiskMedia::Error: DEvice path not MESSAGING_DEVICE_PATH(%d) & Subtype not MSG_SATA_DP(%d) for PORT : %d & Multiport = %d ... \n",
				MESSAGING_DEVICE_PATH, MSG_SATA_DP,
				uiPortNo, uiPortMultiplierPort
			 );
	}
end:
	if (pDevicePath)
		gBS ->FreePool(pDevicePath);
	return bFound;
}

BOOLEAN GetNextDevicePath (EFI_ATA_PASS_THRU_PROTOCOL * pAtaInterface,
		UINT16 * pPortNumber,
		UINT16 * pPortMultiplierPort)
{
	EFI_ATA_PASS_THRU_GET_NEXT_PORT   pGetNextPort;
	EFI_ATA_PASS_THRU_GET_NEXT_DEVICE  pGetNextDevice;
	BOOLEAN bFound   = FALSE;
	BOOLEAN bPortValid = TRUE;

	pGetNextPort =  pAtaInterface->GetNextPort;
	pGetNextDevice =  pAtaInterface->GetNextDevice;

	if(*pPortNumber == 0xffff)
		bPortValid = FALSE;
		
	while (!bFound)
	{
		if (!bPortValid)
		{
			if (pGetNextPort(pAtaInterface, pPortNumber))
			{
				Print (L"GetNextDevicePath:: Error: Port number not found\n");
				break;
			}
        	
			bPortValid = TRUE;
			Print((CHAR16 *)L"Found port number = %d \n", *pPortNumber);
		}

		while (!bFound)
		{
			if (pGetNextDevice(pAtaInterface, *pPortNumber, pPortMultiplierPort))
			{
				Print (L"GetNextDevicePath:: Error : Port Multiplier Port number not found\n");
				break;
			}
			Print(L"GetNextDevicePath:: Found the portNumber = %d, UlPortNumber = %d \n", *pPortNumber, *pPortMultiplierPort);
			if (CheckIfDiskMedia(pAtaInterface, *pPortNumber, *pPortMultiplierPort))
			{
				Print(L"GetNextDevicePath:: Disk Media found ,  port  %d and PortMutipler %d \n", *pPortNumber, *pPortMultiplierPort);
				bFound = TRUE;
			}
		}

		if(!bFound)
		{
			*pPortMultiplierPort = 0xFFFF;
			bPortValid = FALSE;
		}
	}
	
	return bFound;
}

BOOLEAN SSDIdentify(EFI_ATA_PASS_THRU_PROTOCOL *pInterface, UINT16 Port, UINT16 PortMultiplerPort)
{
	EFI_ATA_PASS_THRU_COMMAND_PACKET	*packet;
	EFI_ATA_PASS_THRU_COMMAND_PACKET cmdPacket;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	EFI_IDENTIFY_DATA *ideinfo = (EFI_IDENTIFY_DATA *)g_ssd.Buffer;
	EFI_STATUS status;
	BOOLEAN bRet = FALSE;

	//packet = myalloc(sizeof(EFI_ATA_PASS_THRU_COMMAND_PACKET));
	//if (!packet)
	//	goto cleanup;
	packet = &cmdPacket;
	ZeroMem(packet, sizeof(EFI_ATA_PASS_THRU_COMMAND_PACKET));

	////////////////////////////////////////////////////////////////////
	packet->Acb = &cmdBlock;
	memset(packet->Acb, 0, sizeof(EFI_ATA_COMMAND_BLOCK));

	packet->Asb  = &statusBlock;
	packet->Acb->AtaDeviceHead = (UINT8) (BIT7 | BIT6 | BIT5 | (PortMultiplerPort << 4));
	packet->Acb->AtaCommand    = ATA_IDENTIFY;

	packet->InDataBuffer   = g_ssd.Buffer;
	packet->InTransferLength  = sizeof(EFI_IDENTIFY_DATA);
	packet->OutDataBuffer  = g_ssd.Buffer;
	packet->OutTransferLength = sizeof(EFI_IDENTIFY_DATA);
	packet->Timeout    = ATA_ATAPI_TIMEOUT;
	packet->Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet->Protocol = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_IN;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, Port, PortMultiplerPort, packet, NULL);
	if (!EFI_ERROR (status))
	{
		WORD *IdentifyBuf = g_ssd.Buffer;
		if(1 == (IdentifyBuf[129]&0x01) )
		{
			g_ssd.MaxPendingCmd = ((IdentifyBuf[129]>>1)&0x0F);
			g_ssd.MaxSnapshot = IdentifyBuf[130];
			bRet = TRUE;
		}
		
		//Swap((char*)&ideinfo, sizeof(EFI_IDENTIFY_DATA));
		//
        //if(stristr(ideinfo.AtaData.ModelName, "snapshot") == NULL)
        //	goto cleanup;

		//PrintBinary(pBuffer, 0x80);
		//bRet = TRUE;
	}
	else
	{
		Print(L"ATAIdentify failed with error 0x%x\n", status);
	}
	
//cleanup:

	return bRet;
}

//
DWORD SSDGetSnapshotCount()
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	EFI_STATUS status;
	DWORD count = 0;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_GET_SN_COUNT;
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_ATA_NON_DATA;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
		count = packet.Asb->AtaSectorCount;
	}
	else
	{
		Print(L"SSDGetSnapshotCount failed with error 0x%x\n", status);
	}
	
	return count;	
}

EFI_STATUS SSDGetSnapshotInfo(DWORD Index)
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	SN_DESC* sndesc;
	EFI_STATUS status;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_GET_SN_INFO;
	packet.Acb->AtaSectorCount = (UINT8)Index;
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));

	sndesc = (SN_DESC *)g_ssd.Buffer;
	packet.InDataBuffer   = sndesc;
	packet.InTransferLength  = 512;
	packet.OutDataBuffer  = sndesc;
	packet.OutTransferLength = 512;
	packet.Timeout    = ATA_ATAPI_TIMEOUT;

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_IN;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
		//printf("Command Type:   %08x\n",buffer->s1_save.dwCommandType);
		//printf("Snapshot Index: %d\n",buffer->s1_save.wSnapshotindex);
		//printf("Block Counter:  %d\n",buffer->s1_save.bSnapshotBlockCounter);
		//printf("Timestamp:      %08x\n",buffer->s1_save.timestamp);
		//printf("Description:    %s\n",buffer->s1_save.bSnDesc);
	}
	else
	{
		Print(L"SSDGetSnapshotCount failed with error 0x%x\n", status);
	}
	
	return status;	
}

EFI_STATUS SSDSaveSnapshot(SN_DESC* sndesc)
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	EFI_STATUS status;
	DWORD count = 0;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_SAVE_SN;
	packet.Acb->AtaSectorCount = 1;	//(is_pending == TRUE)?0:1;
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));

	packet.InDataBuffer   = sndesc;
	packet.InTransferLength  = 512;
	packet.OutDataBuffer  = sndesc;
	packet.OutTransferLength = 512;
	packet.Timeout    = ATA_ATAPI_TIMEOUT;

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_OUT;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
		//printf("Command Type:   %08x\n",buffer->s1_save.dwCommandType);
		//printf("Snapshot Index: %d\n",buffer->s1_save.wSnapshotindex);
		//printf("Block Counter:  %d\n",buffer->s1_save.bSnapshotBlockCounter);
		//printf("Timestamp:      %08x\n",buffer->s1_save.timestamp);
		//printf("Description:    %s\n",buffer->s1_save.bSnDesc);
	}
	else
	{
		Print(L"SSDSaveSnapshot failed with error 0x%x\n", status);
	}
	
	return status;
}

EFI_STATUS SSDRemoveSnapshot(DWORD Index, SN_DESC* sndesc)
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	EFI_STATUS status;
	DWORD count = 0;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_REMOVE_SN;
	packet.Acb->AtaSectorCount = (UINT8)Index;
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));

	packet.InDataBuffer   = sndesc;
	packet.InTransferLength  = 512;
	packet.OutDataBuffer  = sndesc;
	packet.OutTransferLength = 512;
	packet.Timeout    = ATA_ATAPI_TIMEOUT;

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_OUT;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
		//printf("Command Type:   %08x\n",buffer->s1_save.dwCommandType);
		//printf("Snapshot Index: %d\n",buffer->s1_save.wSnapshotindex);
		//printf("Block Counter:  %d\n",buffer->s1_save.bSnapshotBlockCounter);
		//printf("Timestamp:      %08x\n",buffer->s1_save.timestamp);
		//printf("Description:    %s\n",buffer->s1_save.bSnDesc);
	}
	else
	{
		Print(L"SSDRemoveSnapshot failed with error 0x%x\n", status);
	}
	
	return status;
}

EFI_STATUS SSDRestoreSnapshot(DWORD Index, SN_DESC* sndesc)
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	EFI_STATUS status;
	DWORD count = 0;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_RESTORE_SN;
	packet.Acb->AtaSectorCount = (UINT8)Index;
	packet.Acb->AtaSectorNumber = 1;	//(is_pending == TRUE)?0:1;
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));
	packet.InDataBuffer   = sndesc;
	packet.InTransferLength  = 512;
	packet.OutDataBuffer  = sndesc;
	packet.OutTransferLength = 512;
	packet.Timeout    = ATA_ATAPI_TIMEOUT;

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_OUT;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
		//printf("Command Type:   %08x\n",buffer->s1_save.dwCommandType);
		//printf("Snapshot Index: %d\n",buffer->s1_save.wSnapshotindex);
		//printf("Block Counter:  %d\n",buffer->s1_save.bSnapshotBlockCounter);
		//printf("Timestamp:      %08x\n",buffer->s1_save.timestamp);
		//printf("Description:    %s\n",buffer->s1_save.bSnDesc);
	}
	else
	{
		Print(L"SSDRestoreSnapshot failed with error 0x%x\n", status);
	}
	
	return status;	
}

DWORD SSDGetPendingCommandNumber()
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	EFI_STATUS status;
	DWORD count = 0;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_GET_PENDING_COUNT;
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_ATA_NON_DATA;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
		count = packet.Asb->AtaSectorCount;
	}
	else
	{
		Print(L"SSDGetPendingCommandNumber failed with error 0x%x\n", status);
	}
	
	return count;		
}

EFI_STATUS SSDGetPendingCommand(DWORD Index, BYTE * Buffer, DWORD Length)
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	SN_DESC* sndesc;
	EFI_STATUS status;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_GET_PENDING_INFO;
	packet.Acb->AtaSectorCount = (UINT8)(Index & 7);
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));

	packet.InDataBuffer   = Buffer;
	packet.InTransferLength  = Length;
	packet.OutDataBuffer  = Buffer;
	packet.OutTransferLength = Length;
	packet.Timeout    = ATA_ATAPI_TIMEOUT;

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_IN;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
		//printf("Command Type:   %08x\n",buffer->s1_save.dwCommandType);
		//printf("Snapshot Index: %d\n",buffer->s1_save.wSnapshotindex);
		//printf("Block Counter:  %d\n",buffer->s1_save.bSnapshotBlockCounter);
		//printf("Timestamp:      %08x\n",buffer->s1_save.timestamp);
		//printf("Description:    %s\n",buffer->s1_save.bSnDesc);
	}
	else
	{
		Print(L"SSDGetPendingCommand failed with error 0x%x\n", status);
	}
	
	return status;		
}

EFI_STATUS SSDDoPendingCommand()
{
	EFI_ATA_PASS_THRU_PROTOCOL *pInterface = g_ssd.PassThru;
	EFI_ATA_PASS_THRU_COMMAND_PACKET packet;
	EFI_ATA_COMMAND_BLOCK	cmdBlock;
	EFI_ATA_STATUS_BLOCK	statusBlock;
	EFI_STATUS status;

	memset(&packet, 0, sizeof(packet));
	memset(&cmdBlock, 0, sizeof(cmdBlock));
	memset(&statusBlock, 0, sizeof(statusBlock));
	
	packet.Acb = &cmdBlock;
	packet.Asb = &statusBlock;

	packet.Acb->AtaCommand    = ATA_CMD_SNAPSHOT;
	packet.Acb->AtaFeatures   = ATA_FEA_DO_PENDING;
	packet.Acb->AtaDeviceHead = (UINT8) (0x40 | (g_ssd.uiPortMultiplierPort << 4));

	packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
	packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_ATA_NON_DATA;

	//
	// Perform the Identify command.
	//
	status = pInterface->PassThru(pInterface, g_ssd.uiPortNo, g_ssd.uiPortMultiplierPort, &packet, NULL);
	if (!EFI_ERROR (status))
	{
	}
	else
	{
		Print(L"SSDDoPendingCommand failed with error 0x%x\n", status);
	}
	
	return status;		
}

// MAIN Functions
BOOLEAN SSDInit(
	IN EFI_HANDLE        ImageHandle,
	IN EFI_SYSTEM_TABLE  *SystemTable
)
{
	ATA_ATAPI_PASS_THRU_INSTANCE *Instance = NULL;
	EFI_STATUS  Status   = (EFI_STATUS)0;
	UINTN     	HandleCount  = 0;
	EFI_HANDLE  *Handles  = (EFI_HANDLE *)NULL;
	UINT16 		uiPortNo        = 0xFFFF;
	UINT16 		uiPortMultiplierPort     = 0xFFFF;
	UINTN     	i;

	memset(&g_ssd, 0, sizeof(g_ssd));
	g_ssd.Buffer = myalloc(512);

	Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiAtaPassThruProtocolGuid,
			NULL, &HandleCount, &Handles);
	if (EFI_ERROR (Status))
	{
		Status = InitializeAtaAtapiPassThru(ImageHandle, SystemTable, &Instance);
		Print(L"InitializeAtaAtapiPassThru returns %d \n", Status);
		if (!EFI_ERROR (Status))
			HandleCount = 1;
	}
	
	if (!EFI_ERROR (Status))
	{
		Print(L"HandleCount is: %d\n", HandleCount);
		Print(L"status %d\n", Status);
		
		for (i = 0; i < HandleCount; i++)
		{
			EFI_ATA_PASS_THRU_PROTOCOL * pAtaInterface;
			
			if(Instance)
			{
				pAtaInterface = &Instance->AtaPassThru;
			}
			else
			{
				Status = gBS->OpenProtocol(Handles[i], &gEfiAtaPassThruProtocolGuid,
							&pAtaInterface, gImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
				if (EFI_ERROR (Status))
				{
					Print(L"GetPassThroughProtocol failed with error : %d\n", Status);
					continue;
				}
				Print(L"GetPassThroughProtocol: 0x%x\n ", pAtaInterface);
			}
			
			for(;;)
			{
				if (GetNextDevicePath(pAtaInterface, &uiPortNo, &uiPortMultiplierPort))
				{
					if (SSDIdentify(pAtaInterface, uiPortNo, uiPortMultiplierPort))
					{
						Print (L"SSDIdentify succesful for port = %d, MPORT = %d  \n",
								uiPortNo, uiPortMultiplierPort);
								
						g_ssd.PassThru = pAtaInterface;
						g_ssd.Instance = Instance;
						g_ssd.uiPortNo = uiPortNo;
						g_ssd.uiPortMultiplierPort = uiPortMultiplierPort;
						g_ssd.bInstall = TRUE;
						break;
					}
					else
					{
						Print (L"SSDIdentify failed for port = %d, MPORT = %d \n",
								uiPortNo, uiPortMultiplierPort);
					}
				}
				else
				{
					Print (L"SSDInit: Could not found the disk Media device...\n");
					break;
				}
			}
			
			if(g_ssd.bInstall)
				break;
		}
		
		if (Handles)
		{
			gBS->FreePool (Handles);
		}
	}
	
	g_ssd.bInit = TRUE;
	
	return g_ssd.bInstall;
}

#endif
