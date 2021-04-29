/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef _PART_H_
#define _PART_H_

#include "biosdisk.h"

#define MAX_NUM_PARTITIONS		64

//extern PARTITION * g_part;
extern DWORD g_dwTotalPartitions;			//����Ӳ�̵��ܷ�����
extern WIN32_PARTITIONINFO g_Partitions[MAX_NUM_PARTITIONS];//����Ӳ�̵�ȫ��������Ϣ���������32�����Ͳ�֧����
	
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
