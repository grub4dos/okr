/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef _FACE_H_
#define _FACE_H_
// DrawMsgBox DrawMsgBoxMultilLine 
VOID test_window();
VOID Face(); // main windows show 

#define FACE_BACKGROUND_COLOR 0XF5F3F4
#define FACE_SELECT_COLOR  0xB3B6B7




typedef struct _PART_LIST_INFO{

	UINT8	 flag;
	
	UINT8	 partindex;
  UINT8  disknum;
  UINT8  PartitionType;
  UINT64 TotalSectors;
  CHAR16 partname[16];
  
}PART_LIST_INFO,*PPART_LIST_INFO;

int DrawMsgBox(CHAR16 *msg,CHAR16 *yes,CHAR16 *no);
int DrawMsgBox_Color(CHAR16 *msg,CHAR16 *yes,CHAR16 *no,UINT32 Color);

int DrawMsgBoxMultilLine(CHAR16 *msg1,CHAR16 *msg2,CHAR16 *msg3,CHAR16 *msg4,CHAR16 *yes,CHAR16 *no);

//void DeleteDriveButtons(struct DrvButtonList *p);
//struct DrvButtonList* NewDriveButtons(PWINDOW pwin, WORD x, WORD y,		WORD width, WORD height, int diskselected);
#endif

