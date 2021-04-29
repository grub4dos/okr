
#include "disk.h"

DISKIO		gDisk = {0};

EFI_STATUS
EFIAPI
BlockIoInit()
{
	EFI_STATUS				status;
	UINTN					handleCount, num, index;
	EFI_HANDLE 				*handleBuffer;
	EFI_BLOCK_IO_PROTOCOL	*blockIo;

	status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &handleCount, &handleBuffer);
	if (EFI_ERROR (status)) 
	{
		//LOG_ERROR(L"BlockIoInit, LocateHandleBuffer failed with error 0x%x\n", status);
		return status;
	}
	//Print(L"LocateHandleBuffer, handle count %d\n", handleCount);

	ZeroMem(&gDisk, sizeof(gDisk));
	gDisk.ActiveDisk = INVALID_DISK_NUMBER;
	gDisk.Count = 0;

	num = 0;
	for(index=0; index<handleCount; index++) 
	{
		status = gBS->OpenProtocol (
			handleBuffer[index],
			&gEfiBlockIoProtocolGuid,
			(VOID **)&blockIo,
			gImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
			);
		if (EFI_ERROR (status))
		{
			//Print(L"OpenProtocol(%x) failed with error 0x%x\n", handleBuffer[index], status);
			continue;
		} 

		if(
			blockIo->Media->LogicalPartition
			&& blockIo->Media->MediaPresent
			&& blockIo->Media->LastBlock > 10*1024*2048 //为了方便调试，NTFS分区只有一个，大于48G
			)
		{
			gDisk.Disk[num].MediaId = blockIo->Media->MediaId;
			gDisk.Disk[num].BytesPerSector = blockIo->Media->BlockSize;
			gDisk.Disk[num].BytesPerPhysicalSector = blockIo->Media->BlockSize * blockIo->Media->LogicalBlocksPerPhysicalBlock;
			gDisk.Disk[num].TotalSectors = blockIo->Media->LastBlock;
			gDisk.Disk[num].DiskHandle = handleBuffer[index];
			gDisk.Disk[num].ReadBlocks = blockIo->ReadBlocks;
			gDisk.Disk[num].WriteBlocks = blockIo->WriteBlocks;
			gDisk.Disk[num].FlushBlocks = blockIo->FlushBlocks;
			gDisk.Disk[num].BlockIo = blockIo;
			gDisk.Count++;
			num++;

			//LOG_DEBUG(L"%d: Handle 0x%x, BlockIo %p, Revision 0x%lx, Read %p, Write %p\n", 
			//	index, 
			//	handleBuffer[index],
			//	blockIo,
			//	blockIo->Revision,
			//	blockIo->ReadBlocks,
			//	blockIo->WriteBlocks);
			//LOG_DEBUG(L"blockio %p, handle %p, MediaId 0x%x, Removable %d, Present %d, localpart %d\n", 
			//	gDisk.Disk[num].BlockIo, handleBuffer[index], 
			//	blockIo->Media->MediaId, 
			//	blockIo->Media->RemovableMedia,
			//	blockIo->Media->MediaPresent,
			//	blockIo->Media->LogicalPartition
			//	);
			//LOG_DEBUG(L"BlockSize %d, IoAlign %d, LastBlock 0x%lx, phy-sec 0x%x, size %d MB\n", 
			//	blockIo->Media->BlockSize,
			//	blockIo->Media->IoAlign,
			//	blockIo->Media->LastBlock,
			//	blockIo->Media->LogicalBlocksPerPhysicalBlock,
			//	(DWORD)DivU64x32(blockIo->Media->LastBlock, 2048));
		}
		else
		{
			//PrintBinary((PBYTE)blockIo->Media, 64);
			gBS->CloseProtocol (
				handleBuffer[index],
				&gEfiBlockIoProtocolGuid,
				gImageHandle,
				NULL
				);
		}

	}
	gBS->FreePool(handleBuffer);

	return status;
}

VOID BlockIoRelease()
{
	DWORD i;
	for(i=0;i<MAX_DISK_NUMBER;i++)
	{
		if(gDisk.Disk[i].BlockIo)
		{
			gBS->CloseProtocol(
				gDisk.Disk[i].DiskHandle,
				&gEfiBlockIoProtocolGuid,
				gImageHandle,
				NULL
				);
		}
		gDisk.Disk[i].BlockIo = NULL;
	}
	gDisk.ActiveDisk = INVALID_DISK_NUMBER;
}
