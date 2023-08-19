/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#ifndef _FACE_H_
#define _FACE_H_

#include "faceUIDefine.h"

// DrawMsgBox DrawMsgBoxMultilLine 
//VOID test_window();
int Face(); // main windows show 

#define FACE_BACKGROUND_COLOR 0XF5F3F4
#define FACE_SELECT_COLOR  0xB3B6B7

#define WM_UPDATE_RECOVERY_LIST_AFTER_DELETE	0xfe85
#define WM_UPDATE_ABOUT_PAGE_INTRODUCE_LIST		0xfe86
#define WM_UPDATE_ABOUT_PAGE_OPENSOURCE_LIST	0xfe87
#define WM_UPDATE_ABOUT_PAGE_VERSION			0xfe88

typedef enum _MESSAGE_BOX_TYPE_ENUM
{
	TYPE_INFO = 0,		//提示框，提示
	TYPE_WARNING,		//提示框，警告
	TYPE_ENQUIRE		//提示框，询问
} MESSAGE_BOX_TYPE;

typedef struct _BUTTON_INFO
{
	WORD btn_id;
	WORD btn_pressedpcx;
	WORD btn_focuspcx;
	WORD btn_unfocuspcx;
	WORD btn_disablepcx;
	WORD title;
	CHAR16* pText;
}BUTTON_INFO,*PBUTTON_INFO;

typedef struct _PART_LIST_INFO
{
	UINT8	 flag;

	UINT8	 partindex;
	UINT8  disknum;
	UINT8  PartitionType;
	UINT64 TotalSectors;
	CHAR16 partname[16];
	UINT64 RemainSectors;
  
}PART_LIST_INFO,*PPART_LIST_INFO;

typedef struct _DISK_LIST_INFO
{
	UINT8	disknum;
	UINT64  TotalSectors;

}DISK_LIST_INFO, *PDISK_LIST_INFO;

typedef struct _SEL_DISK_LIST_INFO
{
	BOOL	bRemovable;
	WORD	disknum;
	UINT64  TotalSectors;

}SEL_DISK_LIST_INFO, *PSEL_DISK_LIST_INFO;

SEL_DISK_LIST_INFO g_seldisk_list_info[MAX_DISK_NUM];

typedef enum _TAB_ENUM
{
	TAB_INTRODUCE = 0,
	TAB_OPEN_SOURCE,
	TAB_VERSION
} TAB_ENUM;


typedef struct
{
	WORD	fileIdL;	//4k			2160/2160  
	WORD	fileIdM;	//2560*1440		1440/2160  
	WORD	fileIdS;	//2k			1080/2160, (0.45, 0.5]
	WORD	fileIdSS;	//1024*768		 768/2160  
} IMAGE_FILE_ID, *PIMAGE_FILE_ID;

//Select backup point icon
typedef struct
{
	WORD	fileBackupPointId;
	WORD	fileBackupPointSelId;
} BACKUP_POINT_ICON, *PBACKUP_POINT_ICON;

//int DrawMsgBox(CHAR16 *msg,CHAR16 *yes,CHAR16 *no);
//int DrawMsgBox_Color(CHAR16 *msg,CHAR16 *yes,CHAR16 *no,UINT32 Color);

int CalculateLineNumber(CHAR16 *msg, StringInfo * pstStringInfo, DWORD dwWidth);
int DrawMsgBoxEx(CHAR16 *msg, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType);
int DrawMsgBoxExWithQRCode(CHAR16 *msg, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType, WORD wQRCodeFileId, WORD wQRCodeFileSize);
int DrawMsgBoxMultilLineEx(CHAR16 *msg, CHAR16 *msg2, CHAR16 *msg3, CHAR16 *msg4, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType);
int DrawMsgBoxTwoLine(CHAR16 *msg, CHAR16 *msg2, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType);
INTN AddPartListLine(PLISTBOX plistbox, WORD diskindex);
void recoverlist_dispatchmsg(PCONTROL pcontrol, struct DM_MESSAGE *dmessage);
void backupPartitionList_dispatchmsg(PCONTROL pcontrol, struct DM_MESSAGE *dmessage);
//void DeleteDriveButtons(struct DrvButtonList *p);
//struct DrvButtonList* NewDriveButtons(PWINDOW pwin, WORD x, WORD y,		WORD width, WORD height, int diskselected);

int SelectRecoveryPointFace(int *partindex/*0*/, DWORD *backupfileid/*-1*/);

int RecoverCompleteUIMsgbox(int func);
int RecoverCompleteUIMsgboxEx(int func, int iResult, CHAR16 *msg);

void DisplayLogo();
void DrawWhiteBK();
void DrawMsgBoxWhiteBK(WORD dwBoxLeft, WORD dwBoxTop, WORD dwBoxWidth, WORD dwBoxHeight);

void About_MyWindow_Dispatchmsg(PWINDOW pwin, struct DM_MESSAGE *dmmessage);
void aboutlist_dispatchmsg(PCONTROL pcontrol, struct DM_MESSAGE *dmessage);
void ShowAboutFace(BOOL bIsPressF1);
void ShowWaitMsgOnce(void);
void ShowWaitMsgFace(CHAR16 *strMessage);

void DonotShowWaitMsgFace();

BUTTON_INFO* FindBtnInfoFromID(DWORD btnid);

WORD GetImageFileId(WORD wFileIdL);

void ShowIntroduce(
	LISTBOX *pListBox,
	SCROLL_BOX *pScrollBox,
	WORD dwLeft,
	WORD dwTop,
	WORD dwWidth,
	WORD dwHeight);

void ShowOpenSource(
	LISTBOX *pListBox,
	SCROLL_BOX *pScrollBox,
	WORD dwLeft,
	WORD dwTop,
	WORD dwWidth,
	WORD dwHeight);

void ShowContent(
	LISTBOX *pListBox,
	SCROLL_BOX *pScrollBox,
	WORD dwLeft,
	WORD dwTop,
	WORD dwWidth,
	WORD dwHeight,
	CHAR16 ** pTextSource,
	WORD tetxSourceSize,
	WORD textType);

#endif

