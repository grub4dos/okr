/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

//getlist.h
#ifndef _GET_LIST_H_
#define _GET_LIST_H_

#ifndef STRUCT_SIZE_D
#define STRUCT_SIZE_D
typedef struct _SIZE_D
{
	DWORD     Begin;					//begin sector
	DWORD     Number;					//block numbers
} SIZE_D, *PSIZE_D;
typedef struct _SIZE_L
{
	ULONGLONG Begin;					//begin sector
	DWORD     Number;					//block numbers
} SIZE_L, *PSIZE_L;
#endif

typedef struct _UNUSED_FILE_DESC_
{
	BYTE	PartitionIndex;
	BYTE	FileType;		//该描述符记录的文件类型，一般是多个文件放到一个记录里，所以此变量暂时不用
	WORD	ListCount;		
	DWORD	TotalSize;		//size in sectors.
	SIZE_L	*pList;
}UNUSED_FILE_DESC, *PUNUSED_FILE_DESC;

#define MAX_UNUSED_FILES	8

extern SIZE_L	*g_CurrentUnusedList;
extern DWORD	g_dwCurrentUnusedListCount;

ATTRIBUTE *FindAttributeFromAttributeListEx(PATTRIBUTE AttrList, BOOL bAttrHeader, 
	ATTRIBUTE_TYPE AttrType, ULONG Vcn, PVOID Buffer, PWORD AttrOffset);
ATTRIBUTE *FindAttributeFromAttributeList(PATTRIBUTE AttrList, BOOL bAttrHeader, 
	ATTRIBUTE_TYPE AttrType, ULONG Vcn, PVOID Buffer);

DWORD GetUnusedDataList(BYTE PartitionIndex);
DWORD GetAllUnusedDataSize();
void ReleaseUnusedDataList();

#endif
