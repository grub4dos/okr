/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef _FORMAT_H_
#define _FORMAT_H_
#pragma pack(1)

#define OKR_IMAGE_MAGIC		'9rko'
#define GPT_DATA_SIZE		34

typedef struct OKR_HEADER {

    DWORD	Magic;				//0, 'okr9'
    DWORD	Version;			//4, 
    DWORD	StructSize;			//8, 本结构大小，包括可变长度的分区信息
    DWORD	Time;				//0xC
    BYTE	Memo[240];			//0x10
    BYTE	reserved[8];		//0x100
    UINT64	FileSize;			//0x108, 所有备份文件大小（如果没有切分，就等于本文件大小；否则是所有切分的文件大小之和），字节数
    UINT64	Disksize;			//0x110, 备份的硬盘大小
	BYTE    SourceDiskGUID[16];
    WORD	PartitionIndex;		//0x118, 备份文件所在分区的index，runtime用
    WORD	PartitionCount;		//0x11A, 备份时检测到的有效分区个数，即本结构中可变长度成员变量Partition的个数
    DWORD	GptSize;			//0x11C, GPT分区表备份区域大小, 0-34扇区，IN BYTE
    BYTE	bGpt;				//0x120
    BYTE	bComplete;			//0x121, 备份结束后值1
    BYTE	bCompress;			//0x122, 数据区是否压缩
    BYTE	bFactoryBackup;		//0x123, 是否工厂备份, 必然在服务分区里，如果没有工厂备份，则第一次备份为工厂备份，工厂备份不可被删除
    DWORD	CompressBlockSize;	//0x124, 每100M压缩一次，必须被缓冲区大小整除
    UINT64	OriginalSize;		//0x128 - 0x130, 原始备份数据区字节数，备份后生成精确,压缩前的大小
	DWORD	DataOffset;			//0x130,	备份数据偏移地址，以字节为单位
	DWORD	FileNumber;			//0x134, 备份的镜像文件的个数
	ULONGLONG FileTime;			//0x138, 文件的日期
    BYTE	Reserved[0x40];		//0x140
	BYTE	FileName[0x80];		//0x180
	BYTE	TempBuffer[2048];		//0x180
    WIN32_PARTITIONINFO Partition[1];	//0x200 + 0xb0*64
}OKR_HEADER, *POKR_HEADER;

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)	((DWORD)(ULONG_PTR)&(((type *)0)->field))
#endif

//FIELD_OFFSET(OKR_HEADER, Reserved)
#define OKR_HEADER_MIN_SIZE FIELD_OFFSET(OKR_HEADER, Partition)	
#define OKR_HEADER_MAX_SIZE ((sizeof(WIN32_PARTITIONINFO)*MAX_NUM_PARTITIONS + FIELD_OFFSET(OKR_HEADER, Partition) + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1))

#pragma pack()

#endif


