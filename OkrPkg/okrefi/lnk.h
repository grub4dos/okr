/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#ifndef _OKR_LNK_H_

#define _OKR_LNK_H_


//#define PUBLIC_DESKTOP_PATH L"\\Users\\Public\\Desktop\\联想软件与驱动安装.lnk"

//#define FILE_NAME L"联想软件与驱动安装.exe"

#define PUBLIC_DESKTOP_PATH L"\\Users\\Public\\Desktop\\Lenovo_DAI_Setup.lnk"

#define FILE_NAME L"Lenovo_DAI_Setup.exe"

#define PUBLIC_DESKTOP_PATH_ZH L"\\Users\\Public\\Desktop\\联想软件与驱动安装.lnk"
#define FILE_NAME_ZH L"联想软件与驱动安装.exe"
#define LNK_FILE_NAME_ZH L"联想软件与驱动安装.lnk"


#define FILE_WORK_PATH L"C:\\"

#define OKR_TOOL_PATH L"\\okrtool\\"


#define CONN_PATH_STR(str) str##FILE_WORK_PATH

#define CONN_STR(str) str##FILE_NAME

#define ZH_CONN_STR(str) str##FILE_NAME_ZH

#define ROOT_FILE_NAME CONN_STR(L"\\")


#define FILE_REATIVE_PATH CONN_STR(L".\\") 

#define REV_FILE_WORK_PATH CONN_PATH_STR(L"/") 

#define ZH_FILE_REATIVE_PATH ZH_CONN_STR(L".\\") 


#define OKR_TOOL_FULL_PATH CONN_STR(L"\\okrtool\\")


#pragma pack(1)

typedef struct _DESCRINFO
{
	 unsigned short size;
	 CHAR16  path[64];//“.\best_773.mid”
}DESCRINFO,*PDESCRINFO;

typedef struct _FILETIME 
{  
   DWORD dwLowDateTime;  
   DWORD dwHighDateTime;  
} FILETIME, *PFILETIME;  


typedef struct _LNKHEAD
{
	DWORD		dwID;
	DWORD		dwGUID[4];
	DWORD		dwFlags;
	DWORD		dwFileAttributes;
	FILETIME	dwCreationTime;
	FILETIME	dwModificationTime;
	FILETIME	dwLastaccessTime;
	DWORD		dwFileLen;
	DWORD		dwIconNum;
	DWORD		dwWinStyle;
	DWORD		dwHotkey;
	DWORD		dwReserved1;
	DWORD		dwReserved2;

}LNKHEAD, *PLNKHEAD;


typedef struct _SHITEMID_ITEM
{
	//我的电脑->盘符->目录1->目录2…->文件名
  WORD ItemIdlistlen;//0x7B

  WORD item_len;//fixed 0x14
  
  BYTE   guid[18];//我的电脑 guid
  //{0x1f,0x50,0xe0,0x4f,0xd0,0x20,0xea,0x3a,0x69,0x10,0xa2,0xd8,0x08,0x2b,0x30,0x30,0x9d}

  WORD str_item_path_len;//0x19 ,//盘符 字符串路径长度
  char   str_item_path[23];//"/C:\"
  
  WORD   item_id_len;//0x4c
  WORD   unknown_flag;//0x32

  DWORD   filelen;//
  BYTE   unknown2[4];//快捷方式图标的index,0x000000
  WORD  fileattr;//0x20
  char  filename[60];//*.exe

  WORD  reserve_offset;//- 0x10

  WORD lastitemlen; //0
        
} SHITEMID_ITEM, * LPSHITEMID_ITEM;



typedef struct _FILELOCATIONINFO
{
	DWORD		dwSize;
	DWORD		dwFirstOffset;
	DWORD		dwFlags;
	DWORD		dwOffsetOfVolume;
	DWORD		dwOffsetOfBasePath;
	DWORD		dwOffsetOfNetworkVolume;
	DWORD		dwOffsetOfRemainingPath;
}FILELOCATIONINFO, *PFILELOCATIONINFO;

typedef struct _LOCALVOLUMETAB
{
	DWORD		dwSize;
	DWORD		dwTypeOfVolume;
	DWORD		dwVolumeSerialNumber;
	DWORD		dwOffsetOfVolumeName;
	char		strVolumeName[7];
}LOCALVOLUMETAB, *PLOCALVOLUMETAB;

typedef struct _ENDINGSTUFF
{
	BYTE buf[4];
}ENDINGSTUFF,*PENDINGSTUFF;


typedef struct _NETWORKVOLUMETAB
{
	DWORD		dwSize;
	DWORD		dwUnknown1;
	DWORD		dwOffsetOfNetShareName;
	DWORD		dwUnknown2;
	DWORD		dwUnknown3;
	char		strNetShareName[0];
	
}NETWORKVOLUMETAB, *PNETWORKVOLUMETAB;



#pragma pack()


BOOL CreateWindowsLnkFile(int partindex,CHAR16 *path,CHAR16 *pFileName);
BOOL CreateWindowsLnkFileByBufFileLnk(int partindex,CHAR16 *path,CHAR16 *pFileName);

#endif

