/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef _PART_H_
#define _PART_H_

#include "biosdisk.h"

#define MAX_NUM_PARTITIONS		64

//extern PARTITION * g_part;
extern DWORD g_dwTotalPartitions;			//所有硬盘的总分区数
extern WIN32_PARTITIONINFO g_Partitions[MAX_NUM_PARTITIONS];//所有硬盘的全部分区信息，如果超过32个，就不支持了
	
typedef INTN FUNCGetData (ULONGLONG sector,ULONGLONG num,VOID *private);

PATTRIBUTE  FindAttribute(PFILE_RECORD_HEADER FileRecord, ATTRIBUTE_TYPE AttrType);
BOOL  FindRun (PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, PULONGLONG lcn, PULONG count);
ULONG RunCount(PBYTE run);
ULONGLONG  RunLCN(PBYTE run);
ULONG  RunLength(PBYTE run);
BOOL  FixupInode(PVOID Inode, DWORD InodeSize);

int  IsUnSupportedPartition(WORD pid);

int  GetPartitionIndex(WORD DiskNumber, DWORD Index);
DWORD  GetPartitionInfo();
void  InitPartition();
 void CleanPartition();
INTN GetData(ULONGLONG sector,ULONGLONG num,VOID *private);

DWORD GetExt3BitmapSector(WIN32_PARTITIONINFO *Partition, DWORD GroupIndex);

int ScanPartitionData(WORD partindex ,FUNCGetData getdatafunc,VOID *private,PBYTE test);

#endif
