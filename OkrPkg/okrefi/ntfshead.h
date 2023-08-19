/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#ifndef _NTFS_H_
#define _NTFS_H_

#include "commondef.h"
//#include "NativeApi.h"
#define INODE_MFT				0
#define INODE_MFT_MIRROR		1
#define INODE_LOG_FILE			2
#define INODE_VOLUME			3
#define INODE_ATTRDEF			4
#define INODE_ROOT_DIRECTORY	5
#define INODE_BITMAP			6
#define INVALID_INDEX           0xffffffff

typedef ULONGLONG USN;
typedef CHAR8 CHAR;
typedef BYTE   UCHAR;
typedef CHAR16 WCHAR;

typedef struct FAT_DIRECTORY_ENTRY{
	BYTE filename[11];       //0
	BYTE fileattr;          //11
	BYTE *longname;       //12
	DWORD reserved;         //16
	WORD firstclr_hi;       //20
	WORD filetime;          //22
	WORD filedate;          //24
	WORD firstclr;          //26
	DWORD filesize;         //28
}FAT_DIRECTORY_ENTRY, *PFAT_DIRECTORY_ENTRY;

#pragma pack(1)

typedef struct _SECLIST_{
	ULONGLONG beginsector;
	ULONGLONG sectornumber;
} SECLIST,*PSECLIST,MFTSECLIST,*PMFTSECLIST;

typedef struct _VCNLIST_{
	ULONGLONG beginvcn;
	ULONGLONG vcnnumber;
} VCNLIST,*PVCNLIST;

typedef struct _DATAATTRLIST_{
	DWORD inodenum;
	DWORD begincluster;
} DATAATTRLIST,*PDATAATTRLIST;

typedef struct {
	ULONG Type;
	USHORT UsaOffset;
	USHORT UsaCount;
	USN Usn;
} NTFS_RECORD_HEADER, *PNTFS_RECORD_HEADER;

typedef struct{
	NTFS_RECORD_HEADER Ntfs;
	USHORT SequenceNumber;
	USHORT LinkCount;
	USHORT AttributesOffset;
	USHORT Flags;				// 0x0001 = InUse, 0x0002 =Directory
	ULONG BytesInUse;
	ULONG BytesAllocated;
	ULONGLONG BaseFileRecord;
	USHORT NextAttributeNumber;
} FILE_RECORD_HEADER, *PFILE_RECORD_HEADER;

typedef enum {
	AttributeStandardInformation=0x10,
	AttributeAttributeList=0x20,
	AttributeFileName=0x30,
	AttributeObjectId=0x40,
	AttributeSecurityDescriptor=0x50,
	AttributeVolumeName=0x60,
	AttributeVolumeInformation=0x70,
	AttributeData=0x80,
	AttributeIndexRoot=0x90,
	AttributeIndexAllocation=0xa0,
	AttributeBitmap=0xb0,
	AttributeReparsePoint=0xc0,
	AttributeEAInformation=0xd0,
	AttributeEA=0xe0,
	AttributePropertySet=0xf0,
	AttributeLoggedUtilityStream=0x100
} ATTRIBUTE_TYPE;

typedef struct {
	union{
		ATTRIBUTE_TYPE AttributeType;
		ULONG padding1;
	} ;
	ULONG Length;
	BOOLEAN Nonresident;
	UCHAR NameLength;
	USHORT NameOffset;
	USHORT Flags;				// 0x0001 = Compressed
	USHORT AttributeNumber;
} ATTRIBUTE, *PATTRIBUTE;

typedef struct {
	ATTRIBUTE Attribute;
	ULONG ValueLength;
	USHORT ValueOffset;
	USHORT Flags;				// 0x0001= Indexed
} RESIDENT_ATTRIBUTE, *PRESIDENT_ATTRIBUTE;

typedef struct {
	ATTRIBUTE Attribute;
	ULONGLONG LowVcn;
	ULONGLONG HignVcn;
	USHORT RunArrayOffset;
	UCHAR CompressionUnit;
	UCHAR AlignmentOrReserved[5];
	ULONGLONG AllocatedSize;
	ULONGLONG DataSize;
	ULONGLONG InitializedSize;
	ULONGLONG CompressedSize;		// Only when compressed
} NONRESIDENT_ATTRIBUTE, *PNONRESIDENT_ATTRIBUTE;
/*
typedef struct {
	ULONGLONG CreationTime;
	ULONGLONG ChangeTime;
	ULONGLONG LastWriteTime;
	ULONGLONG LastAccessTime;
	ULONG FileAttributes;
	ULONG AlignmentOrReservedOrUnknown[3];
	ULONG QuotaId;					// NTFS 3.0 only
	ULONG SecurityId;				// NTFS 3.0 only
	ULONGLONG QuotaCharge;			// NTFS 3.0 only
	USN Usn;						// NTFS 3.0 only
} STANDARD_INFORMATION, *PSTANDARD_INFORMATION;
*/
typedef struct {
	union{
		ATTRIBUTE_TYPE AttributeType;
		ULONG padding1;
	} ;
	USHORT Length;
	UCHAR NameLength;
	UCHAR NameOffset;
	ULONGLONG LowVcn;
	ULONGLONG FileReferenceNumber;
	USHORT AttributeNumber;
	USHORT AlignmentOrReserved[3];
} ATTRIBUTE_LIST, *PATTRIBUTE_LIST;

typedef struct {
	ULONGLONG DirectoryFileReferenceNumber;
	ULONGLONG CreationTime;		//Saved when filename last changed
	ULONGLONG ChangeTime;		// ditto
	ULONGLONG LastWriteTime;	//ditto
	ULONGLONG LastAccessTime;	//ditto
	ULONGLONG AllocateSize;		//ditto
	ULONGLONG DataSize;			//ditto
	ULONG FileAttributes;		//ditto
	ULONG AlignmentOrReserved;
	UCHAR NameLength;
	UCHAR NameType;				// 0x01 = Long, 0x02 = Short
	WCHAR Name[1];
} FILENAME_ATTRIBUTE, *PFILENAME_ATTRIBUTE;
/*
typedef struct {
	GUID ObjectId;
	union {
		struct {
			GUID BirthVolumeId;
			GUID BirthObjectId;
			GUID DomainId;
		};
		UCHAR ExtendedInfo[48];
	};
} OBJECTID_ATTRIBUTE, *POBJECTID_ATTRIBUTE;
*/
typedef struct {
	ULONG Unknown[2];
	UCHAR MajorVersion;
	UCHAR MinorVersion;
	USHORT Flags;
} VOLUME_INFORMATION, *PVOLUME_INFORMATION;

typedef struct {
	ULONG EntriesOffset;
	ULONG IndexBlockLength;
	ULONG AllocationSize;
	ULONG Flags;		// 0x00 = Small directory, 0x01 = Large directory
} DIRECTORY_INDEX, *PDIRECTORY_INDEX;

typedef	struct {
	union{
		ATTRIBUTE_TYPE Type;
		ULONG padding1;
	} ;
	ULONG CollationRule;
	ULONG BytesPerIndexBlock;
	ULONG ClusterPerIndexBlock;
	DIRECTORY_INDEX DirectoryIndex;
} INDEX_ROOT, *PINDEX_ROOT;

typedef struct {
	NTFS_RECORD_HEADER Ntfs;
	ULONGLONG IndexBlockVcn;
	DIRECTORY_INDEX DirectoryIndex;
} INDEX_BLOCK_HEADER, *PINDEX_BLOCK_HEADER;

typedef struct {
	ULONGLONG FileReferenceNumber;
	USHORT Length;
	USHORT AttributeLength;
	ULONG Flags;			// 0x01 = Has trailing VCN ,0x02 = Last entry
	FILENAME_ATTRIBUTE Name;
	// ULONGLONG Vcn;		// VCN in IndexAllocation of earlier entries
} DIRECTORY_ENTRY, *PDIRECTORY_ENTRY;

typedef struct {
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	UCHAR ReparseData[1];
} REPARSE_POINT, *PREPARSE_POINT;

typedef struct {
	ULONG EaLength;
	ULONG EaQueryLength;
} EA_INFORMATION, *PEA_INFORMATION;

typedef struct {
	ULONG NextEntryOffset;
	UCHAR Flags;
	UCHAR EaNameLength;
	USHORT EaValueLength;
	CHAR EaName[1];
	// UCHAR EaData[1];
} EA_ATTRIBUTE, *PEA_ATTRIBUTE;

typedef struct {
	WCHAR AttributeName[64];
	ULONG AttributeNumber;
	ULONG Unknown[2];
	ULONG Flags;
	ULONGLONG MinimumSize;
	ULONGLONG MaximumSize;
} ATTRIBUTE_DEFINITION, *PATTRIBUTE_DEFINITION;

typedef struct {
	ULONG ulEmptySectorNumNtfs;
	ULONG ulBitmapSectorNumAboutEmptySector;
	ULONG ulBitmapSectorOffsetAboutEmptySector;
	ULONG ulBitmapBytesAboutEmptySector;
} DATAFOREMPTYSECTORBYBITMAP, *PDATAFOREMPTYSECTORBYBITMAP;

//
// File System Control commands related to defragging
//
// #define	FSCTL_READ_MFT_RECORD			0x90068
// #define FSCTL_GET_VOLUME_BITMAP			0x9006F
// #define FSCTL_GET_RETRIEVAL_POINTERS	0x90073
// #define FSCTL_MOVE_FILE					0x90074

//
// Error codes returned by NtFsControlFile (see NTSTATUS.H)
//
//#define STATUS_SUCCESS			         ((NTSTATUS)0x00000000L)
//#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
//#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
//#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
//#define STATUS_ALREADY_COMMITTED         ((NTSTATUS)0xC0000021L)
//#define STATUS_INVALID_DEVICE_REQUEST    ((NTSTATUS)0xC0000010L)


//--------------------------------------------------------------------
//       F S C T L  S P E C I F I C   T Y P E D E F S  
//--------------------------------------------------------------------


//
// This is the definition for a VCN/LCN (virtual cluster/logical cluster)
// mapping pair that is returned in the buffer passed to 
// FSCTL_GET_RETRIEVAL_POINTERS
//
typedef struct {
	ULONGLONG			Vcn;
	ULONGLONG			Lcn;
} MAPPING_PAIR, *PMAPPING_PAIR;

//
// This is the definition for the buffer that FSCTL_GET_RETRIEVAL_POINTERS
// returns. It consists of a header followed by mapping pairs
//
typedef struct {
	ULONG				NumberOfPairs;
	ULONGLONG			StartVcn;
	MAPPING_PAIR		Pair[1];
} GET_RETRIEVAL_DESCRIPTOR, *PGET_RETRIEVAL_DESCRIPTOR;


//
// This is the definition of the buffer that FSCTL_GET_VOLUME_BITMAP
// returns. It consists of a header followed by the actual bitmap data
//
typedef struct {
	ULONGLONG			StartLcn;
	ULONGLONG			ClustersToEndOfVol;
	BYTE				Map[1];
} BITMAP_DESCRIPTOR, *PBITMAP_DESCRIPTOR; 


//
// This is the definition for the data structure that is passed in to
// FSCTL_MOVE_FILE
//
//typedef struct {
//	HANDLE            FileHandle; 
//	ULONG             Reserved;   
//	LARGE_INTEGER     StartVcn; 
//	LARGE_INTEGER     TargetLcn;
//	ULONG             NumVcns; 
//	ULONG             Reserved1;	
//} MOVEFILE_DESCRIPTOR, *PMOVEFILE_DESCRIPTOR;


//--------------------------------------------------------------------
//     N T F S C O N T R O L F I L E   D E F I N I T I O N S
//--------------------------------------------------------------------

//
// Prototype for NtFsControlFile and data structures
// used in its definition
//
//typedef ULONG NTSTATUS;
//#ifndef _NTDEF_
//typedef __success(return >= 0) LONG NTSTATUS, *PNTSTATUS;
//#endif

//#ifndef STRU_IO_STATUS_BLOCK 
//#define STRU_IO_STATUS_BLOCK
//
// Io Status block (see NTDDK.H)
//
//typedef struct _IO_STATUS_BLOCK {
//    NTSTATUS Status;
//    ULONG Information;
//} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
//#endif

//--------------------------------------------------------------------
//                         D E F I N E S
//--------------------------------------------------------------------


//
// Invalid longlong number
//
#define LLINVALID		((ULONGLONG) -1)

#pragma pack()

#endif
