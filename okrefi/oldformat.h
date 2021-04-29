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
    DWORD	StructSize;			//8, ���ṹ��С�������ɱ䳤�ȵķ�����Ϣ
    DWORD	Time;				//0xC
    BYTE	Memo[240];			//0x10
    BYTE	reserved[8];		//0x100
    UINT64	FileSize;			//0x108, ���б����ļ���С�����û���з֣��͵��ڱ��ļ���С�������������зֵ��ļ���С֮�ͣ����ֽ���
    UINT64	Disksize;			//0x110, ���ݵ�Ӳ�̴�С
	BYTE    SourceDiskGUID[16];
    WORD	PartitionIndex;		//0x118, �����ļ����ڷ�����index��runtime��
    WORD	PartitionCount;		//0x11A, ����ʱ��⵽����Ч���������������ṹ�пɱ䳤�ȳ�Ա����Partition�ĸ���
    DWORD	GptSize;			//0x11C, GPT�������������С, 0-34������IN BYTE
    BYTE	bGpt;				//0x120
    BYTE	bComplete;			//0x121, ���ݽ�����ֵ1
    BYTE	bCompress;			//0x122, �������Ƿ�ѹ��
    BYTE	bFactoryBackup;		//0x123, �Ƿ񹤳�����, ��Ȼ�ڷ����������û�й������ݣ����һ�α���Ϊ�������ݣ��������ݲ��ɱ�ɾ��
    DWORD	CompressBlockSize;	//0x124, ÿ100Mѹ��һ�Σ����뱻��������С����
    UINT64	OriginalSize;		//0x128 - 0x130, ԭʼ�����������ֽ��������ݺ����ɾ�ȷ,ѹ��ǰ�Ĵ�С
	DWORD	DataOffset;			//0x130,	��������ƫ�Ƶ�ַ�����ֽ�Ϊ��λ
	DWORD	FileNumber;			//0x134, ���ݵľ����ļ��ĸ���
	ULONGLONG FileTime;			//0x138, �ļ�������
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


