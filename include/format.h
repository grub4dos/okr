/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/

#ifndef _FORMAT_H_
#define _FORMAT_H_
#pragma pack(1)

#define OKR_VERSION 0x09000811
#define OKR_IMAGE_MAGIC		'9rko'
#define GPT_DATA_SIZE		128
#define GPT_OLD_DATA_SIZE	34
#define GRUB_CODE_MAX_SIZE	56

typedef struct OKR_HEADER {

    DWORD	Magic;				//0, 'okr9'
    DWORD	Version;			//4, 
    DWORD	StructSize;			//8, 本结构大小，包括可变长度的分区信息
    DWORD	Time;				//0xC
    BYTE	Memo[240];			//0x10	 出厂备份，存储"Initial backup.",初始备份，存储"Initbackup".
	BYTE	bOkrPE;				//0x100, 是否PE下的备份
    BYTE	reserved[3];		//0x101
	DWORD	TimeZone;			//0x104, 时区，多少秒
    UINT64	FileSize;			//0x108, 所有备份文件大小（如果没有切分，就等于本文件大小；否则是所有切分的文件大小之和），字节数
    UINT64	Disksize;			//0x110, 备份的硬盘大小
	BYTE    SourceDiskGUID[16];
	BYTE	WinPartNtfsGuid[16];		//0x180 12 个扇区
	BYTE	Reserved_2[1024];		//0x180 12 个扇区
    WORD	PartitionIndex;		//0x538, 备份文件所在分区的index，runtime用
    WORD	PartitionCount;		//0x53A, 备份时检测到的有效分区个数，即本结构中可变长度成员变量Partition的个数
    DWORD	GptSize;			//0x53C, GPT分区表备份区域大小, 0-34扇区，IN BYTE
    BYTE	bGpt;				//0x540
    BYTE	bComplete;			//0x541, 备份结束后值1
    BYTE	bCompress;			//0x542, 数据区是否压缩
    BYTE	bFactoryBackup;		//0x543, 是否工厂备份, 必然在服务分区里，如果没有工厂备份，则第一次备份为工厂备份，工厂备份不可被删除。
    DWORD	CompressBlockSize;	//0x544, 每100M压缩一次，必须被缓冲区大小整除
    UINT64	OriginalSize;		//0x548 - 0x550, 原始备份数据区字节数，备份后生成精确,压缩前的大小
	DWORD	DataOffset;			//0x550,	备份数据偏移地址，以字节为单位
	DWORD	FileNumber;			//0x554, 备份的镜像文件的个数
	ULONGLONG FileTime;			//0x558, 文件的日期
    BYTE	Reserved[0x40];		//0x560
	BYTE	FileName[0x80];		//0x5A0
	BYTE	TempBuffer[6144];		//0x620 12 个扇区
    WIN32_PARTITIONINFO Partition[1];	//0x1E20  0x200 + 0xb0*64
}OKR_HEADER, *POKR_HEADER;




typedef struct OKR_HEADER_810 {     //old

    DWORD	Magic;				//0, 'okr9'
    DWORD	Version;			//4, 
    DWORD	StructSize;			//8, 本结构大小，包括可变长度的分区信息
    DWORD	Time;				//0xC
    BYTE	Memo[240];			//0x10
	BYTE	bOkrPE;				//0x100, 是否PE下的备份
    BYTE	reserved[3];		//0x101
	DWORD	TimeZone;			//0x104, 时区，多少秒
    UINT64	FileSize;			//0x108, 所有备份文件大小（如果没有切分，就等于本文件大小；否则是所有切分的文件大小之和），字节数
    UINT64	Disksize;			//0x110, 备份的硬盘大小
//kang--	BYTE    SourceDiskGUID[16];
//kang--	BYTE	WinPartNtfsGuid[16];		//0x180 12 个扇区
//kang--	BYTE	Reserved_2[1024];		//0x180 12 个扇区
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
//kang--	BYTE	TempBuffer[6144];		//0x180 12 个扇区
    WIN32_PARTITIONINFO Partition[1];	//0x200 + 0xb0*64
}OKR_HEADER_810, *POKR_HEADER_810;	




#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)	((DWORD)(ULONG_PTR)&(((type *)0)->field))
#endif

//FIELD_OFFSET(OKR_HEADER, Reserved)
#define OKR_HEADER_MIN_SIZE FIELD_OFFSET(OKR_HEADER, Partition)	
#define OKR_HEADER_MAX_SIZE ((sizeof(WIN32_PARTITIONINFO)*MAX_NUM_PARTITIONS + FIELD_OFFSET(OKR_HEADER, Partition) + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1))

#define OKR_OLD_HEADER_MIN_SIZE FIELD_OFFSET(OKR_HEADER_810, Partition)	
#define OKR_OLD_HEADER_MAX_SIZE ((sizeof(WIN32_PARTITIONINFO)*MAX_NUM_PARTITIONS + FIELD_OFFSET(OKR_HEADER_810, Partition) + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1))


#pragma pack()

#endif


