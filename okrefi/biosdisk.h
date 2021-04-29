/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/



#ifndef _DISK_H
#define _DISK_H
#define UEFI_MODULE 
#define CACHE_DOWN						0		//PRE-READ 
#define CACHE_UP						1

//定义一次磁盘IO最大可以访问的扇区数，UEFI下是1MB，legacy bios下是64KB
#define MAX_SECTORS_PER_IO				2048
#define SECTOR_SIZE						512
#define MAX_EXCEPTION_FUNCTIONS			4

//define partition type
#ifndef PARTITION_ENTRY_UNUSED
#define PARTITION_ENTRY_UNUSED			0x00	// Entry unused
#define PARTITION_FAT_12				0x01	// 12-bit FAT entries
#define PARTITION_XENIX_1				0x02	// Xenix
#define PARTITION_XENIX_2				0x03	// Xenix
#define PARTITION_FAT_16				0x04	// 16-bit FAT entries
#define PARTITION_EXTENDED				0x05	// Extended partition entry
#define PARTITION_HUGE					0x06	// Huge partition MS-DOS V4
#define PARTITION_IFS					0x07	// IFS Partition
#define PARTITION_FAT32					0x0B	// FAT32
#define PARTITION_FAT32_XINT13			0x0C	// FAT32 using extended int13 services
#define PARTITION_XINT13				0x0E	// Win95 partition using extended int13 services
#define PARTITION_XINT13_EXTENDED		0x0F	// Same as type 5 but uses extended int13 services
#define PARTITION_PREP					0x41	// PowerPC Reference Platform (PReP) Boot Partition
#define PARTITION_LDM					0x42	// Logical Disk Manager partition
#define PARTITION_UNIX					0x63	// Unix
#define VALID_NTFT						0xC0	// NTFT uses high order bits
#define PARTITION_UNUSED  0x00

// The high bit of the partition type code indicates that a partition is part of an NTFT mirror or striped array.
#define PARTITION_NTFT					0x80	// NTFT partition
#endif

#define	FAT12							1
#define FAT16							0x06
#define FAT16_E							0x0E
#define BIG_FAT16_PARTITION				FAT16_E
#define NTFS							0x07
#define FAT32							0x0B
#define FAT32_E							0x0C
#define BIG_FAT32_PARTITION				FAT32_E
#define EXTEND_PARTITION				0x05
#define BIG_EXTEND_PARTITION			0x0F
#define PARTITION_DOS_12				0x01	// 12-BIT FAT ENTRIES
#define PARTITION_XENIX_ROOT			0x02	// XENIX
#define PARTITION_XENIX_USR				0x03	// XENIX
#define PLAN9_PARTITION					0x039	// PLAN 9 PARTITION ID
#define DM6_AUX1PARTITION				0x051	// NO DDO:  USE XLATED GEOM
#define DM6_AUX3PARTITION				0x053	// NO DDO:  USE XLATED GEOM
#define DM6_PARTITION					0x054	// HAS DDO: USE XLATED GEOM & OFFSET
#define EZD_PARTITION					0x055	// EZ-DRIVE
#define PARTITION_NOVELL_NETWARE		0x064
#define MINIX_PARTITION					0x081	// MINIX PARTITION ID
#define PARTITION_LINUX_SWAP			0x082
#define PARTITION_LINUX					0x083
#define LINUX_EXTENDED_PARTITION		0x085
#define PARTITION_NTFS_VOL_SET			0x086
#define PARTITION_BSD_386				0x0A5
#define PARTITION_OPENBSD				0x0A6
#define NETBSD_PARTITION				0x0A9	// NETBSD PARTITION ID
#define BSDI_PARTITION					0x0B7	// BSDI PARTITION ID
#define	GPT_PROTECTIVE_PARTITION		0xEE
#define	GPT_SYSTEM_PARTITION			0xEF
#define LINUX_RAID_PARTITION			0x0FD	// AUTODETECT RAID PARTITION
#define UNKNOWN_PARTITION				0xff

#define LENOVO_PART						0x12

#define FAT16_EOF						0xFFFF
#define FAT32_EOF						0x0FFFFFFF

//定义我们自己的分区类型，我们将NTFS、FAT16、FAT32转换成我们自己定义的分区类型，这样可以避免驱动删除后不能保护的问题
#define	SHD_PARTITION_ID				0x0D2
//#define SHD_FAT16						0x0D2
//#define SHD_FAT32						0x0D3
//#define SHD_NTFS						0x0D4

#define DISK_ID_SIZE					64		// SIZE IN BYTES

//Magic numbers in CPU format
#define MAGIC_VMDB						'BDMV'	// VMDB
#define MAGIC_VBLK						'KLBV'	// VBLK
#define MAGIC_PRIVHEAD1					'VIRP'	// PRIVHEAD
#define MAGIC_PRIVHEAD2					'DAEH'	//
#define MAGIC_TOCBLOCK1					'BCOT'	// TOCBLOCK
#define MAGIC_TOCBLOCK2					'KCOL'	//

// The defined vblk types
#define VBLK_COMP						0x032	// COMPONENT
#define VBLK_PART						0x033	// PARTITION
#define VBLK_DSK1						0x034	// DISK
#define VBLK_DSK2						0x044	// DISK
#define VBLK_DGR1						0x035	// DISK GROUP
#define VBLK_DGR2						0x045	// DISK GROUP
#define VBLK_VOLU						0x051	// VOLUME

// Other constants
#define LDM_BLOCKSIZE					1024	// SIZE OF BLOCK IN BYTES
#define LDM_DB_SIZE						2048	// SIZE IN SECTORS (= 1MIB)
#define LDM_FIRST_PART_OFFSET			4		// ADD THIS TO FIRST_PART_MINOR to get to the first data partition device minor

#define OFF_PRIVHEAD1					3		// OFFSET OF THE FIRST PRIVHEAD relative to the start of the device in units of sectors

// Offsets to structures within the LDM Database in units of LDM_BLOCKSIZE
#define OFF_PRIVHEAD2					928		// BACKUP PRIVATE HEADERS
#define OFF_PRIVHEAD3					1023

#define OFF_TOCBLOCK1					0		// TABLES OF CONTENTS
#define OFF_TOCBLOCK2					1
#define OFF_TOCBLOCK3					1022
#define OFF_TOCBLOCK4					1023

#define OFF_VMDB						8		// LIST OF PARTITIONS
#define OFF_VBLK						9

#define	SHIELD_MOUNT_DEVICE_PREFIX		"ShieldM_"	//数据同步设备名的前缀(+ mount的盘符)

#pragma pack(1)

#ifndef STRUCT_SIZE_D
#define STRUCT_SIZE_D

typedef struct  _SIZE_D{
	DWORD     Begin;		//begin sector
	DWORD     Number;		//block numbers
}SIZE_D, *PSIZE_D;

typedef struct  _SIZE_L{
	ULONGLONG Begin;		//begin sector
	DWORD     Number;		//block numbers
}SIZE_L, *PSIZE_L;

#endif

typedef struct MY_REGISTERS
{
	WORD	reg_ds;
	WORD	reg_es;
	union{
		struct{
			BYTE reg_al;
			BYTE reg_ah;
		};
		WORD	reg_ax;
	} ;
	union{
		struct{
			BYTE reg_bl;
			BYTE reg_bh;
		};
		WORD	reg_bx;
	} ;
	union{
		struct{
			BYTE reg_cl;
			BYTE reg_ch;
		};
		WORD	reg_cx;
	} ;
	union{
		struct{
			BYTE reg_dl;
			BYTE reg_dh;
		};
		WORD	reg_dx;
	};
	WORD	reg_si;
	WORD	reg_di;
	DWORD	reg_eflags;
} MY_REGISTERS;

typedef struct EDD_PARAMETERS
{
	WORD DiskIoPort;			//0, physical I/O port base address
	WORD DiskControlPort;		//2, disk-drive control port address
	BYTE DriveFlags;			//4, drive flags (see #00261)
	BYTE ProprietaryInfo;		//5, proprietary information
								//		bits 7-4 reserved (0)
								//		bits 3-0: Phoenix proprietary (used by BIOS)
	BYTE DriveIRQ;				//6, IRQ for drive (bits 3-0; bits 7-4 reserved and must be 0)
	BYTE SectorCount;			//7, sector count for multi-sector transfers
	BYTE DMAControl;			//8, DMA control
								//;bits 7-4: DMA type (0-2) as per ATA-2 specification
								//;bits 3-0: DMA channel
	BYTE ProgramIOControl;		//9, programmed I/O control
								//	;bits 7-4: reserved (0)
								//	;bits 3-0: PIO type (1-4) as per ATA-2 specification
	WORD DriveOptions;			//0AH, drive options (see #00262)
	WORD Reserved;				//0CH, 2 BYTEs	reserved (0)
	BYTE ExtensionRevisionLevel;//0Eh	BYTE	extension revision level (high nybble=major, low nybble=minor)
								//		;		(currently 10h for v1.0 and 11h for v1.1-3.0)
	BYTE CheckSum;				//0Fh checksum of bytes 00h-0Eh 8-bit sum of all bytes 00h-0Fh should equal 00h
} EDD_PARAMETERS, *PEDD_PARAMETERS;

typedef struct _DISK_PARA_PACKET	//扩展INT13-48功能(获取磁盘参数)使用的参数包
{
	WORD	PacketSize;			//0
	WORD	ExtFlags;			//0
	DWORD	Cylinders;			//0
	DWORD	Heads;				//0
	DWORD	SectorsPerHead;		//0
	DWORD	TotalSectors;		//0
	DWORD	TotalSectorsHigh;	//0
	WORD	SectorSize;			//0
	DWORD	EDDPara;
} DISK_PARA_PACKET, *PDISK_PARA_PACKET;

typedef struct _DISK_IORQ		//扩展INT13读写函数使用的参数包
{
	BYTE	PacketSize;		//0, size of packet in bytes
	BYTE	reserved;		//1, reserved, must be 0
	WORD	Number;			//2, number of blocks to transfer
	DWORD	Buffer;			//4, address of transfer buffer
	DWORD	Sector;			//8, starting absolute block number
	DWORD	SectorHigh;		//12,the high dword of startblock
} DISKIORQ, *PDISKIORQ;

#ifndef STRUCT_DISK_PARAMETER
#define STRUCT_DISK_PARAMETER
typedef struct _DISK_PARAMETER	//保护系统所有程序使用的磁盘参数结构, 20个字节
{	
	BYTE		DiskNumber;				//磁盘号
	BYTE		DeviceNumber;
	WORD		SectorsPerHead;         //2
	WORD		Heads;                  //4
	WORD		SectorSize;				//6
	DWORD		SectorsPerCyln;			//8
	DWORD		Cylinders;				//12
	ULONGLONG	TotalSectors;           //16
} DISK_PARAMETER, *PDISK_PARAMETER;
#endif

typedef struct _PARTITION_ENTRY	//分区表项结构
{
	BYTE	id;					//0
	BYTE	beginhead;			//0
	WORD	begin_cylnAndsector;	//0
	BYTE	pid;				//0
	BYTE	endHead;			//0
	WORD	endCylnAndSector;	//0
	DWORD	frondsectortotal;	//0
	DWORD	totalsector;		//0
} PARTITION_ENTRY, *PPARTITION_ENTRY;

typedef struct WIN32_PARTITIONINFO
{
	ULONGLONG BootBegin;            //0, 相对于本硬盘开始处的扇区编号
	ULONGLONG ProtectionBegin;		//8, 相对于保护系统虚拟硬盘的开始处的扇区编号
	ULONGLONG TotalSectors;         //16
	union{
		struct{
			DWORD   FirstFatSector;	//24, FAT表的开始位置，相对于分区开始处
			union{
				DWORD	RootDirClr;		//28
				DWORD	RootSectors;	//	;FAT16-根目录所占扇区数；FAT32/NTFS-没有意义
			};
			DWORD	SectorsPerFAT;	//32	;FAT表扇区数，NTFS下表示BITMAP的扇区数
		};
		struct{
			ULONGLONG MftBeginSector;	//24, MFT的开始扇区, 相对于分区开始处
			WORD SectorsPerFileRecord;	//32
			WORD SectorsPerIndxBlock;	//34
		};
		struct{
			DWORD blocks_per_group;		//每个group有多少个block（簇）
			DWORD groups_count;			//一共有多少个group
			DWORD group_desc_sector;	//group描述符所在扇区，相对于分区开始处
		};
	};
	BYTE	DriveLetter;		//36
	BYTE	PartitionId;		//37
	BYTE	FatCopies;			//38	;FAT表的个数
	BYTE	SectorsPerClr;		//39
	BYTE	PartitionType;		//40
	BYTE	bBootPartition;		//41
	BYTE	bIfBackup;			//42
	BYTE	DiskNumber;			//43	;0 is the first disk, 1 is the second...
	DWORD	PartitionTableBegin;//44
	//前面48个字节和结构PARTITION_INFO保持一致
	//////////////////////////////////////////////////////////////////////////
    ULONGLONG FreeSectors;		//48
    ULONGLONG BackupDataSize; 	//56 in bytes
    BYTE TypeGuid[16];			//64
    BYTE UniGuid[16];			//80
    CHAR16 Label[33];			//96 +33*2
	BYTE bGpt;					//162
	BYTE BitmapValid;			//163, 0/1/0xff, 0 - invalid, 1 - valid, 0xff - error, data bitmap是否构造完毕, runtime用
	BYTE HasWindows;			//164, 是否有windows目录
	BYTE PartitionIndex;		//165，在分区表中的索引号
    BYTE padding[2];			//166
    EFI_HANDLE Handle;  		//168, device handle can use protocol simple_filesystem
    
} WIN32_PARTITIONINFO, *PWIN32_PARTITIONINFO;

typedef struct _FAT16_BOOT_SECTOR	//FAT16格式的BPB参数块
{
	BYTE	Jumpto[3];		//0  DB 0EBH, 03CH, 090H
	BYTE	OemID[8];		//3  BYTE "MSDOS5.0"
	WORD	SecInByte;		//11  dw 0200H         ; 扇区字节数
	BYTE	SecPerClr;		//13  db 00H           ; 每簇扇区数
	WORD	ResSec;			//14  dw 0001H         ; 保留扇区数
	BYTE	NumOfFat;		//16  db 02H           ; FAT数
	WORD	RootEntry;		//17  dw 512           ; 根目录数
	WORD	TotalSec;		//19  dw 0000H         ; 总扇区数           ; unused
	BYTE	FormatID;		//21  db 0F8H          ; 磁盘格式代号
	WORD	SecPerFat;		//22  dw 0000H         ; 每FAT扇区数
	WORD	SecPerTrk;		//24  dw 003fH         ; 每磁道扇区数
	WORD	Sides;			//26  dw 0000H         ; 磁盘面数
	DWORD	Hsector;		//28  dd 003fH         ; 隐藏扇区数
	DWORD	BigTotalSec;	//32  dd 00000000H   ; Big total number of sectors
	BYTE	Phydrvnum;		//36  db 80H       ; physical drive number
	BYTE	Dirty;			//37
	BYTE	Signature;		//38  db  029H        ; Extended Boot Record Signature
	DWORD	SerialNumber;	//39  dd  015470ff4h  ; Volume Serial Number
	BYTE	DISKLabel[11];	//43  BYTE  "DISK1_VOL1 "
	BYTE	FileSystem[8];	//54  BYTE  "FAT16   "
} FAT16_BOOT_SECTOR, *PFAT16_BOOT_SECTOR, FAT16_BPB, *PFAT16_BPB;

typedef struct _FAT32_BOOT_SECTOR	//FAT32格式的BPB参数块
{
	BYTE	Jumpto[3];		//0,  DB 0EBH, 03CH, 090H
	BYTE	OemID[8];		//3,  BYTE "MSDOS5.0"
	WORD	SecInByte;		//11,  dw 0200H         ; 扇区字节数
	BYTE	SecPerClr;		//13  db 00H           ; 每簇扇区数
	WORD	ResSec;			//14  dw 0001H         ; 保留扇区数
	BYTE	NumOfFat;		//16  db 02H           ; FAT数
	DWORD	Reserved1;		//17	dd 00H
	BYTE	FormatID;		//21  db 0F8H          ; 磁盘格式代号
	WORD	SecPerFat;		//22  dw 0000H         ; 每FAT扇区数
	WORD	SecPerTrk;		//24  dw 003fH         ; 每磁道扇区数
	WORD	Sides;			//26  dw 0000H         ; 磁盘面数
	DWORD	Hsector;		//28  dd 003fH         ; 隐藏扇区数
	DWORD	BigTotalSec;	//32  dd 00000000H     ; Big total number of sectors
	DWORD	BigSecPerFat;	//36  dd 00000000H
	WORD	flags;			//40
	WORD	fs_version;		//42		/* file system version number */
	DWORD	RootCluster;	//44		/* cluster number of the first cluster of root */
	WORD	info_sec;		//48		/* file system information sector */
	WORD	bootbackup_sec;	//50		/* backup boot sector sector */
	BYTE	Free[12];		//52
	BYTE	Phydrvnum;		//64  db 80H       ; physical drive number
	BYTE	Dirty;			//65
	BYTE	Signature;		//66 db  029H        ; Extended Boot Record Signature
	DWORD	SerialNumber;	//67  dd  015470ff4h  ; Volume Serial Number
	BYTE	DISKLabel[11];	//71     BYTE  "DISK1_VOL1 "
	BYTE	FileSystem[8];	//82  BYTE  "FAT16   "
} FAT32_BOOT_SECTOR, *PFAT32_BOOT_SECTOR, FAT32_BPB, *PFAT32_BPB;

typedef struct _NTFS_BOOT_SECTOR	//NTFS格式的参数块
{
	BYTE	Jump[3];				//0
	BYTE	OemID[8];				//3
	WORD	BytesPerSector;			//11
	BYTE	SecPerClr;				//13
	WORD	BootSectors;			//14
	BYTE	Mbz1;					//16
	WORD	Mbz2;					//17
	WORD	Reserved1;				//19
	BYTE	MediaType;				//21
	WORD	Mbz3;					//22
	WORD	SecPerTrk;				//24
	WORD	Sides;					//26
	DWORD	PartitionOffset;		//28
	DWORD	Reserved2[2];			//32
    ULONGLONG TotalSectors;			//40
	ULONGLONG MftStartLcn;			//48
	ULONGLONG Mft2StartLcn;			//56
	DWORD	ClustersPerFileRecord;	//64
	DWORD	ClustersPerIndexBlock;	//68
	DWORD	SerialNumber;			//72
	DWORD	SerialNumber1;			//76
	BYTE	Code[0x1AE];			//80
	WORD	BootSignature;
} NTFS_BOOT_SECTOR, *PNTFS_BOOT_SECTOR, NTFS_BPB, *PNTFS_BPB;

typedef struct _EXT3_SUPER_BLOCK
{
	DWORD		s_inodes_count;			//00	inodes 计数
	DWORD		s_blocks_count;			//		blocks 计数
	DWORD		s_r_blocks_count;		//		保留的 blocks 计数
	DWORD		s_free_blocks_count;	//		空闲的 blocks 计数
	DWORD		s_free_inodes_count;	//10	空闲的 inodes 计数
	DWORD		s_first_data_block;		//		第一个数据 block
	DWORD		s_log_block_size;		//		block 的大小
	DWORD		s_log_frag_size;		//		可以忽略
	DWORD		s_blocks_per_group;		//20	每 block group 的 block 数量
	DWORD		s_frags_per_group;		//		可以忽略
	DWORD		s_inodes_per_group;		//		每 block group 的 inode 数量
	DWORD		s_mtime;				//		Mount time
	DWORD		s_wtime;				//30	Write time
	WORD		s_mnt_count;			//		Mount count
	WORD		s_max_mnt_count;		//		Maximal mount count
	WORD		s_magic;				//0x38	Magic 签名
	WORD		s_state;				//		File system state
	WORD		s_errors;				//		Behaviour when detecting errors
	WORD		s_minor_rev_level;		//		minor revision level
	DWORD		s_lastcheck;			//40	time of last check
	DWORD		s_checkinterval;		//		max. time between checks
	DWORD		s_creator_os;			//		可以忽略
	DWORD		s_rev_level;			//		Revision level
	BYTE		unknown[0xAE];			//50
	WORD		s_desc_size;			//0xfe  Size of group descriptors, in bytes, if the 64bit
	
}EXT3_SUPER_BLOCK, *PEXT3_SUPER_BLOCK;

#define EXT4_BG_INODE_UNINIT	1
#define EXT4_BG_BLOCK_UNINIT	2
#define EXT4_BG_INODE_ZEROED	4
//Block group flags. Any of:
//0x1 inode table and bitmap are not initialized
//(EXT4_BG_INODE_UNINIT).
//0x2 block bitmap is not initialized
//(EXT4_BG_BLOCK_UNINIT).
//0x4 inode table is zeroed
//(EXT4_BG_INODE_ZEROED).
typedef struct _EXT3_GROUP_DESC
{
	DWORD		bg_block_bitmap;		//block 指针指向 block bitmap
	DWORD		bg_inode_bitmap;		//block 指针指向 inode bitmap
	DWORD		bg_inode_table;			//block 指针指向 inodes table
	WORD		bg_free_blocks_count;	//空闲的 blocks 计数
	WORD		bg_free_inodes_count;	//空闲的 inodes 计数
	WORD		bg_used_dirs_count;		//目录计数
	WORD		bg_flags;				//flags, 指明该group desc是否初始化等
	DWORD		bg_reserved[3];			//可以忽略
}EXT3_GROUP_DESC, *PEXT3_GROUP_DESC;

typedef struct _EXT4_GROUP_DESC
{  
	DWORD		bg_block_bitmap_lo;     /*0, Blocks bitmap block块位图所在的第一个块的块ID */  
	DWORD		bg_inode_bitmap_lo;     /*4, Inodes bitmap block inode位图所在的第一个块的块ID */  
	DWORD		bg_inode_table_lo;      /*8, Inodes table block inode表所在的第一个块的块ID */  
	WORD		bg_free_blocks_count_lo;/*C, Free blocks count块组中未使用的块数*/  
	WORD		bg_free_inodes_count_lo;/*E, Free inodes count块组中未使用的inode数*/  
	WORD		bg_used_dirs_count_lo;  /*10,Directories count块组分配的目录的inode数*/  
	WORD		bg_flags;               /*12, EXT4_BG_flags (INODE_UNINIT,etc) */  
	DWORD		bg_reserved[2];         /*14, Likely block/inode bitmap checksum*/  
	WORD		bg_itable_unused_lo;    /*1C, Unused inodes count */  
	WORD		bg_checksum;            /*1E crc16(sb_uuid+group+desc) */  
	//these fields only exist if the 64bit feature is enabled and s_desc_size > 32
	DWORD		bg_block_bitmap_hi;     /*20, Blocks bitmap block MSB */  
	DWORD		bg_inode_bitmap_hi;     /*24, Inodes bitmap block MSB */  
	DWORD		bg_inode_table_hi;      /*28, Inodes table block MSB */  
	WORD		bg_free_blocks_count_hi;/*2C, Free blocks count MSB */  
	WORD		bg_free_inodes_count_hi;/*2E, Free inodes count MSB */  
	WORD		bg_used_dirs_count_hi;  /*30, Directories count MSB */  
	WORD		bg_itable_unused_hi;    /*32, Unused inodes count MSB */  
	DWORD		bg_reserved2[3];  		//34,
}EXT4_GROUP_DESC, *PEXT4_GROUP_DESC;

//////////////////////////////////////////////////////////////////////////
#ifndef UEFI_MODULE

//gpt分区，第一个扇区是legacy MBR，第二个扇区是GPT header，就是结构EFI_PARTITION_TABLE_HEADER；
//从第三个扇区开始，是结构EFI_PARTITION_ENTRY数组，至少有32个扇区，也就是32*4 = 128个该结构大小
typedef struct 
{
	ULONGLONG  Signature;	//0
	DWORD  Revision;		//8
	DWORD  HeaderSize;		//0x0C
	DWORD  CRC32;			//0x10, headersize指定大小的数据的crc值，计算该值前需要将crc32设置为0
	DWORD  Reserved;		//0x14
} EFI_TABLE_HEADER;

typedef ULONGLONG EFI_LBA;
typedef GUID EFI_GUID;

///
/// GPT Partition Table Header.
///
typedef struct 
{
	EFI_TABLE_HEADER  Header;					//0
	EFI_LBA           MyLBA;					//0x18
	EFI_LBA           AlternateLBA;				//0x20
	EFI_LBA           FirstUsableLBA;			//0x28
	EFI_LBA           LastUsableLBA;			//0x30
	EFI_GUID          DiskGUID;					//0x38
	EFI_LBA           PartitionEntryLBA;		//0x48
	DWORD             NumberOfPartitionEntries;	//0x50
	DWORD             SizeOfPartitionEntry;		//0x54
	DWORD             PartitionEntryArrayCRC32;	//0x58
} EFI_PARTITION_TABLE_HEADER;

///
/// GPT Partition Entry, 128字节，一个扇区可以存储4个结构
///
typedef struct {
	EFI_GUID  PartitionTypeGUID;		//0
	EFI_GUID  UniquePartitionGUID;		//16
	EFI_LBA   StartingLBA;				//32
	EFI_LBA   EndingLBA;				//40
	ULONGLONG Attributes;				//48
	WCHAR     PartitionName[36];		//56 + 72 = 128
} EFI_PARTITION_ENTRY;
#endif

/////////////////////////////////////////////////////////////////////////
//structure of dynamic disk
typedef struct _PRIV_HEAD
{
	WORD      ver_major;				//
	WORD      ver_minor;				//
	DWORD     logical_disk_start;		//
	DWORD     logical_disk_size;		//
	DWORD     config_start;				//
	DWORD     config_size;				//
	BYTE      disk_id[DISK_ID_SIZE];	//
} PRIVHEAD;

typedef struct _VMDB
{
	WORD      ver_major;				//
	WORD      ver_minor;				//
	DWORD     vblk_size;				//
	DWORD     vblk_offset;				//
	DWORD     last_vblk_seq;			//
} VMDB;

#ifdef _WINDOWS
#ifndef THUNK_DLL
typedef struct tagLDM_PRIVATE_HEADER
{
	WORD			ver_major;
	WORD			ver_minor;
	LARGE_INTEGER	partition_area_start;
	LARGE_INTEGER	partition_area_size;
	LARGE_INTEGER	ldm_database_start;
	LARGE_INTEGER	ldm_database_size;
	BYTE			disk_id[64];
} LDM_PRIVATE_HEADER, *PLDM_PRIVATE_HEADER;

typedef struct tagLDM_TOCBLOCK
{
	BYTE			bitmap1_name[16];
	LARGE_INTEGER	bitmap1_start;
	LARGE_INTEGER	bitmap1_size;
//	LARGE_INTEGER	bitmap1_flags;
	BYTE			bitmap2_name[16];
	LARGE_INTEGER	bitmap2_start;
	LARGE_INTEGER	bitmap2_size;
//	LARGE_INTEGER	bitmap2_flags;
} LDM_TOCBLOCK, *PLDM_TOCBLOCK;

typedef struct tagLDM_VMDB
{
	WORD			ver_major;
	WORD			ver_minor;
	DWORD			vblk_size;
	DWORD			vblk_offset;
	DWORD			last_vblk_seq;
} LDM_VMDB, *PLDM_VMDB;

typedef struct tagLDM_VBLK
{
	BYTE			name[64];
	BYTE			vblk_type;
	LARGE_INTEGER	obj_id;
	LARGE_INTEGER	disk_id;
	LARGE_INTEGER	start_sector;
	LARGE_INTEGER	num_sectors;
} LDM_VBLK, *PLDM_VBLK;
#endif
#endif

#pragma pack()

#endif
