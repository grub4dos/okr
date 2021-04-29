
#include "okr.h"
#include "okrefiStr.h"
// DrawMsgBox DrawMsgBoxMultilLine 
//#define SCROLL_DRIVE_LIST  1    

#ifndef SCROLL_DRIVE_LIST
/*
struct DrvButtonList;
typedef struct DRVBOX
{
	struct CONTROL controlinfo; 	//控件的位置信息
	CHAR16  volname[50];			//存放
	CHAR16  capname[50];
	BOOL  ifremove;
	//   struct RADIOBOX* next;
	WORD status;
	struct DrvButtonList * plist;
	WORD indexinlist;
} DRVBOX, *PDRVBOX;
struct DrvButtonList {
	DRVBOX icons[MAX_NUM_PARTITIONS];
	INTN num;//64 max
	WORD partindex[MAX_NUM_PARTITIONS];
	INTN selected;//-1 no selected
	PCONTROL bindctl;//listbox

};
*/
#endif

#define MSGBOX_WIDTH 600
#define MSGBOX_HEIGHT 256

#define LENCENCE_MSGBOX_WIDTH 600
#define LENCENCE_MSGBOX_HEIGHT 690
#define BOTTOM_Y  192

#define COPYRIGHT_WIDTH 533
#define COPYRIGHT_HEIGTH 616
WORD  g_x_begin = 0;	//窗口的x坐标
WORD  g_y_begin = 0;	//窗口的y坐标
extern DWORD	g_VgaModeNumber;
PLISTBOX plistbox;
EDITBOX *g_editbox = NULL;
 
WORD  g_nLincenceBoxPage = 1; // current select index  = add by wh
WORD  g_nLincenceboxPicX = 1; // current select page position  = add by wh
WORD  g_nLincenceboxPicY = 1; // current select  page position   = add by wh
//WORD  g_nLincenceboxPreBtnX = 1;   //Lincence PreBtn position    = add by wh
//WORD  g_nLincenceboxPreBtnY = 1;   //Lincence PreBtn position    = add by wh
//WORD  g_nLincenceboxNextBtnX = 1;  //Lincence NextBtn position   = add by wh
//WORD  g_nLincenceboxNextBtnY = 1;  //Lincence NextBtn position   = add by wh
struct DrvButtonList *g_DriveList = NULL;
struct BUTTON *g_btnDelete = NULL;
struct BUTTON *g_btnRestore = NULL;

int checkntfslog = 0;
BOOL DeleteFactoryBackupfile()
{ 
	 //MyLog(LOG_MESSAGE, L"DeleteFactoryBackupfile,bHasFactoryBackup:%d.",g_data.bHasFactoryBackup);
	 
	 if(g_data.bHasFactoryBackup == TRUE)
	 {
      int index = 0;
		  for(index = 0; index < g_data.ImageFilesNumber; index++ )
	    {
			    if(g_data.images[index].bFactoryBackup)
		      {
				     FILE_SYS_INFO  sysinfo;
				     WORD partindex = (WORD)g_data.images[index].PartitionIndex;

				    //需要弹出提示要求确认
				    if(DrawMsgBox(STRING_DELETE_FACTORY_CONFIRM, STRING_YES, STRING_NO) == IDOK) 
				    {
				     	if (FileSystemInit(&sysinfo, partindex, FALSE) != FALSE)
				      {
					     	 CHAR16 name[100] = { 0 };
					    	 DWORD count = g_data.images[index].FileNumber;
						     DWORD i;
						     BOOL bDeleted = FALSE;
						     if(!count)
						    	 count = 1;
						     for(i=0;i<count;i++)
						     {
						      	if(i == 0)
						       		SPrint(name, 100, BACKUP_DIR L"\\%s", g_data.images[index].FileName);
							     else
								      SPrint(name, 100, BACKUP_DIR L"\\%s.%d", g_data.images[index].FileName, i);
							     bDeleted = FileDelete(&sysinfo, name);
						     }
						     if(bDeleted)
						     {
							      //删除备份点以后，就要增加这个文件所在的分区的剩余空间
							      g_Partitions[partindex].FreeSectors += g_data.images[index].FileSize/SECTOR_SIZE;
						     }

					       FileSystemClose(&sysinfo);
					
					       g_data.images[index].bFactoryBackup = FALSE;
					
				      }
				      g_data.bFactoryRestore = FALSE;
				      g_data.bHasFactoryBackup = FALSE;
				 
				      return TRUE;
			      }
		      }

	    }

			
	 }
	 return FALSE;
}

void MyWindow_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{

    Window_Dispatchmsg(pwin,dmmessage);
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	if(dmmessage->message == CHAR_F1)
	{

	
 
		 DrawNewCopyrightDlg();
	}

	//MyLog(LOG_MESSAGE, L"debug mode off.g_data.Cmd %d ", g_data.Cmd );

	//按下alt+x后退出程序，就会退出到shell，而不是重启
	if(IsAltPressed())
	{
		if(dmmessage->message == 'x' || dmmessage->message == 'X')
			g_data.bExitToShell = TRUE;
	}

	if(IsCtrlPressed()) {

		switch(dmmessage->message) {

			case 'd':
			case 'D':
			case 'd'-0x60:

				g_data.bDebug ^= TRUE;
				if(g_data.bDebug) {
					g_data.LogLevel = LOG_DEBUG;
					MyLog(LOG_MESSAGE, L"debug mode on.");
				} else {
					g_data.LogLevel = LOG_ERROR;
					MyLog(LOG_MESSAGE, L"debug mode off.");

					if(g_data.LogY != LOG_GRAPHIC_Y - 20)
						PutImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
					g_data.bRefeshDebugInfo = TRUE;
				}
				break;

			case 'f':
			case 'F':
			case 'f'-0x60:
				{
					WCHAR *msg[2] = {STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_OFF};
					g_data.bForceRestorePartition ^= TRUE;
					MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");
					DrawMsgBox(g_data.bForceRestorePartition ? msg[0] : msg[1], STRING_YES, NULL);
				}
				break;

			case 'a':
			case 'A':
			case 'a'-0x60:
				g_data.bAsyncIO ^= TRUE;
				MyLog(LOG_MESSAGE, L"AsyncIO %s.", g_data.bAsyncIO ? L"on" : L"off");
				break;
			case 't':
			case 'T':
			case 't'-0x60:
				if(g_data.Cmd == USER_COMMAND_FUNCTION_SELECT)
				{
					MyLog(LOG_MESSAGE, L"Factory restoring is activated.");
					//需要弹出提示要求确认
					if(DrawMsgBox(STRING_FACTORY_RESTORE, STRING_YES, STRING_NO) == IDOK) {
						g_data.bFactoryRestore = TRUE;
						Window_exit(pwin, IDB_RECOVER_FACTORY);
						return;
					}
				}
				break;
		  case 'e':
		  case 'E':
		  case 'e'-0x60:  
		  {
			  if (g_data.Cmd == USER_COMMAND_RECOVER_SELECT)
			  {
				  if (DeleteFactoryBackupfile() == TRUE)
				  {
					  Window_exit(pwin, IDB_DELETE);
					  return;
				  }
			  }
			  break;
		  }
#if OKR_DEBUG_TEST_MSGUI //add by wh
		  case 'x':
		  case 'X':
		  case 'x' - 0x60:
		  {

			  MyLog(LOG_DEBUG, L"VgaInit  g_WindowWidth：%d g_WindowHeight %d  modeNumber:%d\r\n", g_WindowWidth, g_WindowHeight, g_VgaModeNumber);

			  break;
		  }

 		case 'z':
		case 'Z':
		case 'z' - 0x60:
		{
 

			MyLog(LOG_ERROR, L"OKR_DEBUG_TEST_MSGUI  \n");
			DrawMsgBox(STRING_DRIVE_NO_SUPPORT_FS, STRING_YES, NULL);
			DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1, STRING_FS_INIT_FAILED_2, STRING_FS_INIT_FAILED_3, NULL, STRING_YES, NULL);
			DrawMsgBox(STRING_OEM_PART_TOO_SMALL_TO_BACKUP, STRING_YES, NULL);
			DrawMsgBox(STRING_DRIVE_MIGHT_SMALL, STRING_YES, NULL);
			DrawMsgBox(STRING_DELETE_CONFIRM, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_FACTORY_RESTORE, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_HINTNOSYSPART, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_FIND_SYS_ERROR, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_HINTNOSYSPART, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_DELETE_CONFIRM, STRING_YES, STRING_NO);
			DrawMsgBoxMultilLine(STRING_NTFS_DELET_ERROR_1, STRING_NTFS_DELET_ERROR_2, STRING_NTFS_DELET_ERROR_3, NULL, STRING_YES, NULL);
			DrawMsgBoxMultilLine(STRING_NTFS_DELET_ERROR_1, STRING_NTFS_DELET_ERROR_2, STRING_NTFS_DELET_ERROR_3, NULL, STRING_YES, NULL);
			DrawMsgBox(STRING_NO_DISK_AVAIBLE_HINT, STRING_YES, NULL);
			DrawMsgBox(STRING_PART_TOO_SMALL_TO_BACKUP, STRING_YES, NULL);
			DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1, STRING_FS_INIT_FAILED_2, STRING_FS_INIT_FAILED_3, NULL, STRING_YES, NULL);
			DrawMsgBox(STRING_PART_TOO_SMALL_TO_BACKUP, STRING_YES, NULL);
			DrawMsgBox(STRING_PART_MIGHT_SMALL, STRING_YES, NULL);
			DrawMsgBox(STRING_FACTORY_BACKUP_NOT_FOUND, STRING_YES, NULL);
			DrawMsgBox(STRING_R_SYS_HINT, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_REBOOT, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_BACKUP_USER_BREAK, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_USER_BREAK, STRING_YES, STRING_NO);
			DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1, STRING_FS_INIT_FAILED_2, STRING_FS_INIT_FAILED_3, NULL, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_CREATE_DIR_ERROR, STRING_YES, NULL);
			DrawMsgBox(STRING_ERROR_WRITE_FILE, STRING_YES, NULL);
			DrawMsgBox(STRING_PART_MODIFY_ERROR, STRING_YES, STRING_NO);
			DrawMsgBox(STRING_ERROR_READ_FILE, STRING_YES, NULL);
			DrawMsgBox(STRING_PARTITION_CHANGED, STRING_YES, NULL);
			DrawMsgBox(STRING_HINTPARTTABLE, STRING_YES, STRING_NO);
 
		}
#endif

			default:
				break;
		}

	}

	if (g_data.Cmd == USER_COMMAND_RECOVER_SELECT) {
		if ( plistbox->item_num ) {
			//enable/disable delete button.
			int index = plistbox->firstitem_inwindow + plistbox->whichselect;
			if(g_data.images[index].bFactoryBackup){
				if (!(g_btnDelete->controlinfo.status & CON_STATUS_DISABLE))
					Control_disable(&g_btnDelete->controlinfo);
			} else {
				if (g_btnDelete->controlinfo.status & CON_STATUS_DISABLE)
					Control_enable(&g_btnDelete->controlinfo);
			}
		}
	} else if( g_data.Cmd == USER_COMMAND_BACKUP_SELECT && g_data.bHasFactoryBackup == FALSE ) {	
		if( g_DriveList->selected != -1) {
			WORD diskindex = g_DriveList->partindex[g_DriveList->selected];
			if(g_data.BackupDiskNumber == diskindex) {
				if (!(g_editbox->controlinfo.status & CON_STATUS_DISABLE)) {
					StrCopy(g_editbox->name, STRING_FACTORY_BACKUP);
					g_editbox->current_len = StrLength(STRING_FACTORY_BACKUP);
					Control_disable(&g_editbox->controlinfo);
				}
			} else {
				if (g_editbox->controlinfo.status & CON_STATUS_DISABLE) {
					g_editbox->name[0] = 0;
					g_editbox->current_len = 0;
					g_editbox->posofcursor = 0;
					Control_enable(&g_editbox->controlinfo);
				}
			}
		}
	}

    if (dmmessage->message == CHAR_ESC)
        Window_exit(pwin, IDCANCEL);
}

void Recover_MyWindow_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{
    Recover_Window_Dispatchmsg(pwin,dmmessage);
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	if(dmmessage->message == CHAR_F1)
	{
		DrawNewCopyrightDlg();
	}

	//按下alt+x后退出程序，就会退出到shell，而不是重启
	if(IsAltPressed())
	{
		if(dmmessage->message == 'x' || dmmessage->message == 'X')
			g_data.bExitToShell = TRUE;
	}

	if(IsCtrlPressed()) {

		switch(dmmessage->message) {

			case 'd':
			case 'D':
			case 'd'-0x60:

				g_data.bDebug ^= TRUE;
				if(g_data.bDebug) {
					g_data.LogLevel = LOG_DEBUG;
					MyLog(LOG_MESSAGE, L"debug mode on.");
				} else {
					g_data.LogLevel = LOG_ERROR;
					MyLog(LOG_MESSAGE, L"debug mode off.");

					if(g_data.LogY != LOG_GRAPHIC_Y - 20)
						PutImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
					g_data.bRefeshDebugInfo = TRUE;
				}
				break;

			case 'f':
			case 'F':
			case 'f'-0x60:
				{
					WCHAR *msg[2] = {STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_OFF};
					g_data.bForceRestorePartition ^= TRUE;
					MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");
					DrawMsgBox(g_data.bForceRestorePartition ? msg[0] : msg[1], STRING_YES, NULL);
				}
				break;

			case 'a':
			case 'A':
			case 'a'-0x60:
				g_data.bAsyncIO ^= TRUE;
				MyLog(LOG_MESSAGE, L"AsyncIO %s.", g_data.bAsyncIO ? L"on" : L"off");
				break;
			case 't':
			case 'T':
			case 't'-0x60:
				if(g_data.Cmd == USER_COMMAND_FUNCTION_SELECT)
				{
					MyLog(LOG_MESSAGE, L"Factory restoring is activated.");
					//需要弹出提示要求确认
					if(DrawMsgBox(STRING_FACTORY_RESTORE, STRING_YES, STRING_NO) == IDOK) {
						g_data.bFactoryRestore = TRUE;
						Window_exit(pwin, IDB_RECOVER_FACTORY);
						return;
					}
				}
				break;
		  case 'e':
		  case 'E':
		  case 'e'-0x60:  
			  {
		  		if(g_data.Cmd == USER_COMMAND_RECOVER_SELECT)
		  		{		 	
		  			if( DeleteFactoryBackupfile() == TRUE)
		  			  {
		  		  		Window_exit(pwin,IDB_DELETE);
							  return;
							}	   
		  		}
		  		break;
				}// end of  'e'
#if OKR_DEBUG //add by wh
 			case 'z':
		case 'Z':
		case 'z' - 0x60:
		{
			if (USER_COMMAND_RECOVER_SELECT == pwin->ret_data)
			{

			}
			Window_exit(pwin, IDB_RECOVER_FACTORY);
    	}
#endif
				default:
					break;
			}


	}

	if (g_data.Cmd == USER_COMMAND_RECOVER_SELECT) {
  
  //	MyLog(LOG_MESSAGE, L"RECOVER_SELECT first:%d,sel:%d,num %d",plistbox->firstitem_inwindow,plistbox->whichselect,plistbox->item_num );

		if ( plistbox->item_num ) {
			//enable/disable delete button.
			int index = plistbox->firstitem_inwindow + plistbox->whichselect;
   
   	//	MyLog(LOG_MESSAGE, L"RECOVER_SELECT index:%d,bFactoryBackup:%d",index,g_data.images[index].bFactoryBackup);

			if(g_data.images[index].bFactoryBackup){
				if (!(g_btnDelete->controlinfo.status & CON_STATUS_DISABLE))
					Control_disable(&g_btnDelete->controlinfo);
			} else {
				if (g_btnDelete->controlinfo.status & CON_STATUS_DISABLE)
					Control_enable(&g_btnDelete->controlinfo);
			}
		}
		if(plistbox->item_num <= 0)
		{
			//if (!(g_btnRestore->controlinfo.status & CON_STATUS_DISABLE))
			{
						Control_disable(&g_btnRestore->controlinfo);
			} 
		/*
			else 
			{
				if (g_btnRestore->controlinfo.status & CON_STATUS_DISABLE)
					Control_enable(&g_btnRestore->controlinfo);
			
			}
		*/
		}			
		
	} else if( g_data.Cmd == USER_COMMAND_BACKUP_SELECT && g_data.bHasFactoryBackup == FALSE ) {	
		if( g_DriveList->selected != -1) {
			WORD diskindex = g_DriveList->partindex[g_DriveList->selected];
			if(g_data.BackupDiskNumber == diskindex) {
				if (!(g_editbox->controlinfo.status & CON_STATUS_DISABLE)) {
					StrCopy(g_editbox->name, STRING_FACTORY_BACKUP);
					g_editbox->current_len = StrLength(STRING_FACTORY_BACKUP);
					Control_disable(&g_editbox->controlinfo);
				}
			} else {
				if (g_editbox->controlinfo.status & CON_STATUS_DISABLE) {
					g_editbox->name[0] = 0;
					g_editbox->current_len = 0;
					g_editbox->posofcursor = 0;
					Control_enable(&g_editbox->controlinfo);
				}
			}
		}
	}

    if (dmmessage->message == CHAR_ESC)
        Window_exit(pwin, IDCANCEL);
}
void SelectDiskMyWindow_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{
    SelectDisk_Window_Dispatchmsg(pwin,dmmessage);
    
    if( pwin->ifexit == 1)
    {
    	 return;
    }	
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	if(dmmessage->message == CHAR_F1)
	{
		DrawNewCopyrightDlg();
	}

	//按下alt+x后退出程序，就会退出到shell，而不是重启
	if(IsAltPressed())
	{
		if(dmmessage->message == 'x' || dmmessage->message == 'X')
			g_data.bExitToShell = TRUE;
	}

	if(IsCtrlPressed()) {

		switch(dmmessage->message) {

			case 'd':
			case 'D':
			case 'd'-0x60:

				g_data.bDebug ^= TRUE;
				if(g_data.bDebug) {
					g_data.LogLevel = LOG_DEBUG;
					MyLog(LOG_MESSAGE, L"debug mode on.");
				} else {
					g_data.LogLevel = LOG_ERROR;
					MyLog(LOG_MESSAGE, L"debug mode off.");

					if(g_data.LogY != LOG_GRAPHIC_Y - 20)
						PutImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
					g_data.bRefeshDebugInfo = TRUE;
				}
				break;

			case 'f':
			case 'F':
			case 'f'-0x60:
				{
					WCHAR *msg[2] = {STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_OFF};
					g_data.bForceRestorePartition ^= TRUE;
					MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");
					DrawMsgBox(g_data.bForceRestorePartition ? msg[0] : msg[1], STRING_YES, NULL);
				}
				break;

			case 'a':
			case 'A':
			case 'a'-0x60:
				g_data.bAsyncIO ^= TRUE;
				MyLog(LOG_MESSAGE, L"AsyncIO %s.", g_data.bAsyncIO ? L"on" : L"off");
				break;
		/*
			case 't':
			case 'T':
			case 't'-0x60:
				if(g_data.Cmd == USER_COMMAND_FUNCTION_SELECT)
				{
					MyLog(LOG_MESSAGE, L"Factory restoring is activated.");
					//需要弹出提示要求确认
					if(DrawMsgBox(STRING_FACTORY_RESTORE, STRING_YES, STRING_NO) == IDOK) {
						g_data.bFactoryRestore = TRUE;
						Window_exit(pwin, IDB_RECOVER_FACTORY);
						return;
					}
				}
				break;
		  case 'e':
		  case 'E':
		  case 'e'-0x60:  
		  {
		  	if(g_data.Cmd == USER_COMMAND_RECOVER_SELECT)
		  	{		 	
		  	    if( DeleteFactoryBackupfile() == TRUE)
		  		  {
		  		  	Window_exit(pwin,IDB_DELETE);
						  return;
						}	   
		  	}
		  	break;
			}
		*/	
			default:
				break;
		}
	}



    if (dmmessage->message == CHAR_ESC)
        Window_exit(pwin, IDCANCEL);
}

void SelectPartWindow_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{
    Window_Dispatchmsg(pwin,dmmessage);
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	if(dmmessage->message == CHAR_F1)
	{
		DrawNewCopyrightDlg();
	}

	//按下alt+x后退出程序，就会退出到shell，而不是重启
	if(IsAltPressed())
	{
		if(dmmessage->message == 'x' || dmmessage->message == 'X')
			g_data.bExitToShell = TRUE;
	}

	if(IsCtrlPressed()) {

		switch(dmmessage->message) {

			case 'd':
			case 'D':
			case 'd'-0x60:

				g_data.bDebug ^= TRUE;
				if(g_data.bDebug) {
					g_data.LogLevel = LOG_DEBUG;
					MyLog(LOG_MESSAGE, L"debug mode on.");
				} else {
					g_data.LogLevel = LOG_ERROR;
					MyLog(LOG_MESSAGE, L"debug mode off.");

					if(g_data.LogY != LOG_GRAPHIC_Y - 20)
						PutImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
					g_data.bRefeshDebugInfo = TRUE;
				}
				break;

			case 'f':
			case 'F':
			case 'f'-0x60:
				{
					WCHAR *msg[2] = {STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_OFF};
					g_data.bForceRestorePartition ^= TRUE;
					MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");
					DrawMsgBox(g_data.bForceRestorePartition ? msg[0] : msg[1], STRING_YES, NULL);
				}
				break;

			case 'a':
			case 'A':
			case 'a'-0x60:
				g_data.bAsyncIO ^= TRUE;
				MyLog(LOG_MESSAGE, L"AsyncIO %s.", g_data.bAsyncIO ? L"on" : L"off");
				break;
			case 't':
			case 'T':
			case 't'-0x60:
				if(g_data.Cmd == USER_COMMAND_FUNCTION_SELECT)
				{
					MyLog(LOG_MESSAGE, L"Factory restoring is activated.");
					//需要弹出提示要求确认
					if(DrawMsgBox(STRING_FACTORY_RESTORE, STRING_YES, STRING_NO) == IDOK) {
						g_data.bFactoryRestore = TRUE;
						Window_exit(pwin, IDB_RECOVER_FACTORY);
						return;
					}
				}
				break;
		  case 'e':
		  case 'E':
		  case 'e'-0x60:  
		  {
		  	break;
			}
#if OKR_DEBUG  //add by wh
 			case 'z':
		case 'Z':
		case 'z' - 0x60:
		{
			if (USER_COMMAND_RECOVER_SELECT == pwin->ret_data)
			{

			}
			Window_exit(pwin, IDB_RECOVER_FACTORY);
		}
#endif
			default:
				break;
		}

	}


    if (dmmessage->message == CHAR_ESC)
        Window_exit(pwin,0);
}
//////////////////////////////////////////////////////////////////
typedef struct {
    BUTTON button;
    CHAR16 *str;
    DWORD active_color;
    DWORD inactive_color;
} STRINGBUTTON,* PSTRINGBUTTON;
STRINGBUTTON *g_pLincenceboxPreBtn = NULL;// Lincencebox btn   = add by wh
STRINGBUTTON *g_pLincenceboxNexBtn = NULL;// Lincencebox btn   = add by wh
void _stringbutton_show(PCONTROL pcon,BOOL active)
{
    WORD thex=(pcon->thew-GetWidthofString(((PSTRINGBUTTON)pcon)->str))/2;
    WORD they=(pcon->theh-PIXEL_PER_CHAR)/2;
    hidemouse();
    if (active) {
        DisplayString(pcon->thex+thex,pcon->they+they,((PSTRINGBUTTON)pcon)->active_color,((PSTRINGBUTTON)pcon)->str);
    } else {
        DisplayString(pcon->thex+thex,pcon->they+they,((PSTRINGBUTTON)pcon)->inactive_color,((PSTRINGBUTTON)pcon)->str);
    }
    showmouse();
}

void string_button_msg(PCONTROL pcon,struct DM_MESSAGE* msg)
{
    WORD thex=(pcon->thew-GetWidthofString(((PSTRINGBUTTON)pcon)->str))/2;
    WORD they=(pcon->theh-PIXEL_PER_CHAR)/2;

    Button_dispatchmsg(pcon,msg); // 默认处理函数

    if (msg->message==WM_CTL_ACTIVATE)
        _stringbutton_show(pcon,TRUE);

    if (msg->message==WM_CTL_INACTIVATE)
        _stringbutton_show(pcon,FALSE);

}
void StringButton_show(struct CONTROL * button)
{
    _stringbutton_show(button,FALSE);
}

void InitStringButton(PSTRINGBUTTON button,WORD x, WORD y ,WORD width,WORD height,CHAR16 *str)
{
    Button_init((PBUTTON)button,x,y,width,height,0,0,0,0);
    button->str=str;
    button->active_color=0xf13326;
    button->inactive_color=COLOR_BLACK;
    button->button.controlinfo.sendmessage=string_button_msg;
    button->button.controlinfo.show=StringButton_show;
}

void strbutton_default(PCONTROL pcon,struct DM_MESSAGE* msg)
{
    string_button_msg(pcon,msg); // 默认处理函数

    if (msg->message==WM_BUTTON_CLICK)
    {
        Window_exit(pcon->pwin, pcon->control_id);
    }
}

WORD GetLineOfMessage(CHAR16 *buffer, CHAR16 *msg, WORD maxwidth, WORD *width)
{
	WORD i;
	CHAR16 ch;
	CHAR16 *p = (CHAR16 *)buffer;
	WORD stringwidth = 0;
	
	for(i=0;;i++)
	{
		ch = msg[i];
		*(CHAR16*)p = ch;
		
		if(ch == 0)
			break;
		if(ch >= 0x100)
			stringwidth += 16;
		else
			stringwidth += 8;
		if(stringwidth > maxwidth)
			break;
		p++;
	}
	
	for(;i>0;i--)
	{
		ch = msg[i];
		if(ch <= 0x20 || ch == '-' || ch > 0x100)
			break;
		p--;
	}
	if(*p == ',' || *p == '.' || *p == ';' || *p == ':' || *p == '?' || *p == '!') {
		p++;
		i++;
	}
	
	*p = 0;
	*width = stringwidth;
	return i;
}

WORD ParseString(CHAR16 *szMsg, CHAR16 *MessageBuffer, WORD *width, WORD height)
{
	WORD len = 0, nlines = 0;
	WORD maxwidth = 0, maxlines, dwMaxWidth = 0;
	CHAR16 *pMsg = szMsg;
	CHAR16 *pMsgBuf = (WORD *)MessageBuffer;
	
	maxlines = height / 18;
	for (;;)
	{
		len = GetLineOfMessage(pMsgBuf, pMsg, *width, &maxwidth);
		if (dwMaxWidth < maxwidth)
			dwMaxWidth = maxwidth;

		nlines++;
		pMsgBuf += StrLength(pMsgBuf) + 1;
		pMsg += len;
		if (*pMsg == 0)
			break;
		if (nlines >= maxlines)
			break;
	}

	*pMsgBuf = 0;	//double 0 means eof.
	*width = dwMaxWidth;
	return nlines;
}

void PrintMessageInRect(WORD x, WORD y, WORD width, WORD height, CHAR16 *msg, DWORD color)
{
	CHAR16 buf[512];
	CHAR16  *p = (CHAR16 *)buf;
	WORD i, nlines;
	
	width = MSGBOX_WIDTH - 24;
	nlines = ParseString(msg, buf, &width, height);
	y += (height - nlines*18)/3;
	if(nlines == 1 && width < MSGBOX_WIDTH - 24)
		x += (MSGBOX_WIDTH - 24 - width)/2;
	
	for(i=0;i<nlines;i++)
	{
		DisplayString(x, y, color, p);	//COLOR_WHITE
		while(*p++ != 0);
		y += 18;
	}
}

void okrupdatepercent(int a)
{
	CHAR16 text[64]={0};
	SPrint(text, 64, L"正在调整系统分区大小 %d%%", a);
	FillRect(			g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300, 20, COLOR_CLIENT);
	DisplayStringInRect(g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300,20, 0X2f557c, text);
}
void okrupdatepercent_last(void)
{
	CHAR16 *text=L"正在做最后设置，请稍候。。。";

	FillRect(			g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300, 20, COLOR_CLIENT); 
	DisplayStringInRect(g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300,20, 0X2f557c, text);
}

int DrawCurCopyrightPage(WORD Index)
{
	DWORD FontColor = 0X2f557c;
	CHAR16 Buffer[256]={0};
	WORD x = (g_WindowWidth - LENCENCE_MSGBOX_WIDTH) / 2;
	WORD y = (g_WindowHeight - LENCENCE_MSGBOX_HEIGHT) / 2;
	
	x= x + 50;
	y= y + LENCENCE_MSGBOX_HEIGHT-45 +10 ;
	UnicodeSPrint (Buffer, 256, L"Page %d/%d", Index,NEW_LINCENCE_MAXPAGE);
	
	  
	FillRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, 0XFEFEFE);
	FillRect(x, y, 100,20, 0XFEFEFE);
	DisplayStringInRect(x, y, 100,20, FontColor, Buffer);
	switch(Index)
	{
		case 1:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_1);
			break;
		case 2:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_2);
			break;
		case 3:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_3);
			break;
		case 4:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_4);
			break;
		case 5:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_5);
			break;
		case 6:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_6);
			break;
		case 7:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_7);
			break;
		case 8:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_8);
			break;
		case 9:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_9);
			break;
		case 10:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_10);
			break;
		case 11:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_11);
			break;
		case 12:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_12);
			break;
		case 13:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_13);
			break;
		case 14:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_14);
			break;
		case 15:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_15);
			break;
		case 16:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_16);
			break;
			
		case 17:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_17);
			break;
			/*
		case 18:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_18);
			break;
		case 19:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_19);
			break;
		case 20:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_20);
			break;
		case 21:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_21);
			break;
		case 22:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_22);
			break;
		case 23:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_23);
			break;
		case 24:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_24);
			break;
		case 25:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_25);
			break;
		case 26:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_26);
			break;
		case 27:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_27);
			break;
		case 28:
			DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_28);
			break;
			*/
	//	case 29:
	//		DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_29);
	//		break;
	//	case 30:
		//	DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, FontColor, LicenseStr_30);
		//	break;
		default:
			break;
	}
	//KP_SaveScreen (gImageHandle,gBS,g_vga,L"BG.BMP");
	return 0;
}
//显示版权信息
int DrawNewCopyrightDlg() //  need show many info add by wh 2018-02-01
{
	WORD x = (g_WindowWidth - LENCENCE_MSGBOX_WIDTH) / 2;
	WORD y = (g_WindowHeight - LENCENCE_MSGBOX_HEIGHT) / 2;
	STRINGBUTTON buttonctrl[3] = { 0 };
	struct WINDOW win;
	int ret;
	WORD x1, y1, rows, width;
	BYTE oldstate;
 
	x1 = x;
	y1 = y;

	PBYTE buff = AllocatePool(4 * (LENCENCE_MSGBOX_WIDTH + 5)*(LENCENCE_MSGBOX_HEIGHT + 5));

	ClearKeyboard();

	hidemouse();
	oldstate = EnableMouse(TRUE);

	GetImgEx(x, y, MSGBOX_WIDTH, LENCENCE_MSGBOX_HEIGHT, buff);

	Window_init(&win, NULL); 

	DrawRect(0XFEFEFE, x, y, LENCENCE_MSGBOX_WIDTH, LENCENCE_MSGBOX_HEIGHT); //draw the BKG of the DLG 
	Linebox(x, y, LENCENCE_MSGBOX_WIDTH, LENCENCE_MSGBOX_HEIGHT, 0X010101); //0X010101 draw the the edge add by wh 
	 
  
	if (g_WindowHeight <= 800)
	{	
		Button_init(&buttonctrl[0],   x + LENCENCE_MSGBOX_WIDTH - 40, y  + 8, 35, 35, //>>  
			IMG_LINCENCE_CLOSE_01, IMG_LINCENCE_CLOSE_01, IMG_LINCENCE_CLOSE_02, 0);
	
		Button_init((PBUTTON)&buttonctrl[1], x + LENCENCE_MSGBOX_WIDTH / 2 + 10, y + LENCENCE_MSGBOX_HEIGHT-45   , 116, 38,
			IMG_LINCENCE_BACK_01, IMG_LINCENCE_BACK_01, IMG_LINCENCE_BACK_02, 0);
		Button_init((PBUTTON)&buttonctrl[2], x + LENCENCE_MSGBOX_WIDTH / 2 + 10 + 116+20, y + LENCENCE_MSGBOX_HEIGHT  -45   , 116, 38,
			IMG_LINCENCE_NEXT_01, IMG_LINCENCE_NEXT_01, IMG_LINCENCE_NEXT_02, 0);

       g_nLincenceboxPicY = y + 18; // current select  page position   = add by wh	
	   //g_nLincenceboxPreBtnX = x + LENCENCE_MSGBOX_WIDTH / 2 + 10;   //Lincence PreBtn position    = add by wh
	   //g_nLincenceboxPreBtnY = 32 + LENCENCE_MSGBOX_HEIGHT - 45;   //Lincence PreBtn position    = add by wh
	   //g_nLincenceboxNextBtnX = x + LENCENCE_MSGBOX_WIDTH / 2 + 10 + 116 + 20;  //Lincence NextBtn position   = add by wh
	   //g_nLincenceboxNextBtnY = 32 + LENCENCE_MSGBOX_HEIGHT - 45;  //Lincence NextBtn position   = add by wh

	} 
	else
	{
		Button_init(&buttonctrl[0], x + LENCENCE_MSGBOX_WIDTH - 40, y + 8, 35, 35, //>>  
			IMG_LINCENCE_CLOSE_01, IMG_LINCENCE_CLOSE_01, IMG_LINCENCE_CLOSE_02, 0);

		Button_init((PBUTTON)&buttonctrl[1], x + LENCENCE_MSGBOX_WIDTH / 2 + 10, y + LENCENCE_MSGBOX_HEIGHT - 45, 116, 38,
			IMG_LINCENCE_BACK_01, IMG_LINCENCE_BACK_01, IMG_LINCENCE_BACK_02, 0);
		Button_init((PBUTTON)&buttonctrl[2], x + LENCENCE_MSGBOX_WIDTH / 2 + 10 + 116 + 20, y + LENCENCE_MSGBOX_HEIGHT - 45, 116, 38,
			IMG_LINCENCE_NEXT_01, IMG_LINCENCE_NEXT_01, IMG_LINCENCE_NEXT_02, 0);
        
		g_nLincenceboxPicY = y + 24; // current select  page position   = add by wh	

		 //g_nLincenceboxPreBtnX = x + LENCENCE_MSGBOX_WIDTH / 2 + 10;   //Lincence PreBtn position    = add by wh
		 //g_nLincenceboxPreBtnY = 104 + LENCENCE_MSGBOX_HEIGHT - 45;   //Lincence PreBtn position    = add by wh
		 //g_nLincenceboxNextBtnX = x + LENCENCE_MSGBOX_WIDTH / 2 + 10 + 116 + 20;  //Lincence NextBtn position   = add by wh
		 //g_nLincenceboxNextBtnY = 104 + LENCENCE_MSGBOX_HEIGHT - 45;  //Lincence NextBtn position   = add by wh
	}

	 g_nLincenceboxPicX = x + 30; // current select page position  = add by wh
	
	 g_nLincenceBoxPage = 1;
	 //DisplayImg(g_nLincenceboxPicX, g_nLincenceboxPicY, IMG_NEW_LINCENCE_PAGE_1, FALSE); // 当前版本  version_normal.bmp
	// DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, COLOR_BLACK, LicenseStr_1);
	DrawCurCopyrightPage(1);
 	 //DisplayImg( g_nLincenceboxPreBtnX , g_nLincenceboxPreBtnY , IMG_LINCENCE_BACK_03, FALSE); //  
	    
	showmouse();


	buttonctrl[0].button.controlinfo.control_id = IDOK;
	buttonctrl[0].button.controlinfo.sendmessage = strbutton_default;
	buttonctrl[0].str=L" ";
 
	buttonctrl[1].button.controlinfo.control_id = IDB_PAGE_PRE;
  	buttonctrl[1].button.controlinfo.sendmessage = New_CopyRight_contro_pre_dispatchmsg;
	buttonctrl[1].button.controlinfo.status |= CON_STATUS_DISABLE;
	buttonctrl[1].button.btn_disablepcx = IMG_LINCENCE_BACK_03;


	buttonctrl[2].button.controlinfo.control_id = IDB_PAGE_NEXT;
    buttonctrl[2].button.controlinfo.sendmessage = New_CopyRight_contro_Next_dispatchmsg;
	buttonctrl[2].button.btn_disablepcx = IMG_LINCENCE_NEXT_03;

	Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//翻页按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//翻页按钮											//Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//确定按钮

	 g_pLincenceboxPreBtn = &buttonctrl[1];// Lincencebox btn   = add by wh
	 g_pLincenceboxNexBtn = &buttonctrl[2];// Lincencebox btn   = add by wh

	ret = Window_run(&win);

	hidemouse();


	//restore img
	PutImgEx(x, y, LENCENCE_MSGBOX_WIDTH, LENCENCE_MSGBOX_HEIGHT, buff);

	EnableMouse(oldstate);
	showmouse();
    FreePool(buff);
	return ret;
}
//显示版权信息
int DrawCopyrightDlg() 
{
	WORD x=(g_WindowWidth-MSGBOX_WIDTH)/2;
	WORD y=(g_WindowHeight-MSGBOX_HEIGHT)/2;
	STRINGBUTTON buttonctrl[2] = { 0 };
	struct WINDOW win;
	int ret;
	WORD x1, y1, rows, width;
	BYTE oldstate;
	//save img

	PBYTE buff=AllocatePool(4*(MSGBOX_WIDTH+5)*(MSGBOX_HEIGHT+5));

	ClearKeyboard();

	hidemouse();
	oldstate = EnableMouse(TRUE);

	GetImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	Window_init(&win,NULL);

	//DisplayImg(x, y, IMG_FILE_MESSAGEBOX, FALSE);
	DrawLinearRect(0X959595,0x4D4D4D,x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT);

	DrawLine(x+8,y+BOTTOM_Y,MSGBOX_WIDTH-16,2,0xFF7F00);
	//DrawLine(x+5,y+BOTTOM_Y,MSGBOX_WIDTH-10,1,0x484848);
	//DrawLine(x+5,y+BOTTOM_Y+1,MSGBOX_WIDTH-10,1,0x797979);
	//Linebox(x-1,y-1,MSGBOX_WIDTH+2,MSGBOX_HEIGHT+2,0XF0F0F0);
	//Linebox(x-2,y-2,MSGBOX_WIDTH+4,MSGBOX_HEIGHT+4,0xcdcdcd);

	//我们取出来的是16点阵的字体
	//PrintMessageInRect(x+16, y+10, MSGBOX_WIDTH-24, BOTTOM_Y, msg);
//	DisplayString(x+96, y + 60, COLOR_BLACK, STRING_VERSION);
//	DisplayString(x+96, y + 90, COLOR_BLACK, STRING_COPYRIGHT);

	DisplayString(x+96-30, y + 60-40, COLOR_BLACK, STRING_OPEN_DESC_1);
	DisplayString(x+96-30, y + 90-40, COLOR_BLACK, STRING_OPEN_DESC_2);
	DisplayString(x+96-30, y + 90+20-40, COLOR_BLACK, STRING_OPEN_DESC_3);
	DisplayString(x+96-30, y + 90+20+30-40, COLOR_BLACK, STRING_OPEN_DESC_4);
	DisplayString(x+96-30, y + 90+20+30+30-40, COLOR_BLACK, STRING_OPEN_DESC_5);
	DisplayString(x+96-30, y + 90+20+30+30+20-40, COLOR_BLACK, STRING_OPEN_DESC_6);

	//InitStringButton(&buttonctrl[0],x,y+BOTTOM_Y,MSGBOX_WIDTH/2,MSGBOX_HEIGHT-BOTTOM_Y,no );
	InitStringButton(&buttonctrl[0],x, y+BOTTOM_Y, MSGBOX_WIDTH, MSGBOX_HEIGHT-BOTTOM_Y, STRING_YES);
	showmouse();

	//buttonctrl[0].button.controlinfo.control_id = IDCANCEL;
	//buttonctrl[0].button.controlinfo.sendmessage = strbutton_default;
	buttonctrl[0].button.controlinfo.control_id = IDOK;
	buttonctrl[0].button.controlinfo.sendmessage = strbutton_default;

	Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
	//Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//确定按钮

	ret = Window_run(&win);

	hidemouse();
	//restore img
	PutImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	EnableMouse(oldstate);
	showmouse();

	FreePool(buff);
	return ret;
}

int DrawMsgBox_Color(CHAR16 *msg,CHAR16 *yes,CHAR16 *bTwoBtn,UINT32 Color)
{
    WORD x=(g_WindowWidth-MSGBOX_WIDTH)/2;
    WORD y=(g_WindowHeight-MSGBOX_HEIGHT)/2;
    STRINGBUTTON buttonctrl[2] = { 0 };
    struct WINDOW win;
    int ret;
    WORD x1, y1, rows, width;
    BYTE oldstate, bMouseDisplay;
    //save img
#if OKR_DEBUG_UI //add by wh
	MyLog(LOG_MESSAGE, L"======-----=== DrawMsgBox.%s",msg );

#endif

    PBYTE buff=AllocatePool(4*(MSGBOX_WIDTH+5)*(MSGBOX_HEIGHT+5));

	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	GetImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	Window_init(&win,NULL);

 
	//DrawLinearRect(0X959595,0x4D4D4D,x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT); // back color
	DrawRect(0XFEFEFE, x, y, MSGBOX_WIDTH, MSGBOX_HEIGHT); //draw the BKG of the DLG 
	Linebox(x, y, MSGBOX_WIDTH, MSGBOX_HEIGHT, COLOR_MESSAGEBOX_BK_LINE_DRAK_BLUE); // draw the the edge add by wh 
 
	//DrawLine(x + 8, y + BOTTOM_Y, MSGBOX_WIDTH - 16, 2, 0X010101); //~~ 下方横线 
 
	//if(bTwoBtn) { // 
	////	DrawLine(g_WindowWidth/2,y+BOTTOM_Y +2,1,MSGBOX_HEIGHT-BOTTOM_Y-8 ,0xFF7F00); //~~ 下方竖线 
	//	//DrawLine(g_WindowWidth/2+1,y+BOTTOM_Y+2,1,MSGBOX_HEIGHT-BOTTOM_Y-8 ,0x797979);
	//}
 

	//我们取出来的是16点阵的字体
	PrintMessageInRect(x+16, y+10, MSGBOX_WIDTH-24, BOTTOM_Y, msg, Color);
	//DisplayString(x+10,y+BOTTOM_Y/2 - 10,COLOR_WHITE,msg);
	//DisplayString(x1,y1,COLOR_WHITE,msg);

	if(bTwoBtn) {
		Button_init(&buttonctrl[0], x + 58, y + BOTTOM_Y-10, 185, 60,  // the cancel  button  
		   IMG_FILE_BUTTON_CANCEL_FOCUS, IMG_FILE_BUTTON_CANCEL_FOCUS, IMG_FILE_BUTTON_CANCEL_UNFOCUS, 0);
		Button_init(&buttonctrl[1], x + MSGBOX_WIDTH / 2+50, y + BOTTOM_Y-10, 185, 60, //OK  button 
			IMG_FILE_BUTTON_CONFIRM_FOCUS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);
		 
		//InitStringButton(&buttonctrl[0],x+8,y+BOTTOM_Y,MSGBOX_WIDTH/2-8,MSGBOX_HEIGHT-BOTTOM_Y, bTwoBtn);
		//InitStringButton(& buttonctrl[1],x+MSGBOX_WIDTH/2,y+BOTTOM_Y,MSGBOX_WIDTH/2-8,MSGBOX_HEIGHT-BOTTOM_Y,yes);
		buttonctrl[0].button.controlinfo.control_id = IDCANCEL;
		buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
	} else {
		//InitStringButton(&buttonctrl[1],x+8, y+BOTTOM_Y, MSGBOX_WIDTH-16, MSGBOX_HEIGHT-BOTTOM_Y, yes);
		Button_init(&buttonctrl[1], x + MSGBOX_WIDTH / 2-90, y + BOTTOM_Y - 8, 185, 60, //>>  
			IMG_FILE_BUTTON_CONFIRM_FOCUS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);
	}

    buttonctrl[1].button.controlinfo.control_id = IDOK;
    buttonctrl[1].button.controlinfo.sendmessage = btn_default;
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//确定按钮

	showmouse();

	ret = Window_run(&win);

	hidemouse();
	//restore img
	PutImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	EnableMouse(oldstate);

	if(bMouseDisplay)
		showmouse();
    
    FreePool(buff);
    return ret;
}

int DrawMsgBox(CHAR16 *msg,CHAR16 *yes,CHAR16 *bTwoBtn)
{
    WORD x=(g_WindowWidth-MSGBOX_WIDTH)/2;
    WORD y=(g_WindowHeight-MSGBOX_HEIGHT)/2;
    STRINGBUTTON buttonctrl[2] = { 0 };
    struct WINDOW win;
    int ret;
    WORD x1, y1, rows, width;
    BYTE oldstate, bMouseDisplay;
    //save img
#if OKR_DEBUG_UI //add by wh
	MyLog(LOG_MESSAGE, L"======-----=== DrawMsgBox.%s",msg );

#endif

    PBYTE buff=AllocatePool(4*(MSGBOX_WIDTH+5)*(MSGBOX_HEIGHT+5));

	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	GetImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	Window_init(&win,NULL);

 
	//DrawLinearRect(0X959595,0x4D4D4D,x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT); // back color
	DrawRect(0XFEFEFE, x, y, MSGBOX_WIDTH, MSGBOX_HEIGHT); //draw the BKG of the DLG 
	Linebox(x, y, MSGBOX_WIDTH, MSGBOX_HEIGHT, COLOR_MESSAGEBOX_BK_LINE_DRAK_BLUE); // draw the the edge add by wh 
 
	//DrawLine(x + 8, y + BOTTOM_Y, MSGBOX_WIDTH - 16, 2, 0X010101); //~~ 下方横线 
 
	//if(bTwoBtn) { // 
	////	DrawLine(g_WindowWidth/2,y+BOTTOM_Y +2,1,MSGBOX_HEIGHT-BOTTOM_Y-8 ,0xFF7F00); //~~ 下方竖线 
	//	//DrawLine(g_WindowWidth/2+1,y+BOTTOM_Y+2,1,MSGBOX_HEIGHT-BOTTOM_Y-8 ,0x797979);
	//}
 

	//我们取出来的是16点阵的字体
	PrintMessageInRect(x+16, y+10, MSGBOX_WIDTH-24, BOTTOM_Y, msg, COLOR_SEC_BLUE);
	//DisplayString(x+10,y+BOTTOM_Y/2 - 10,COLOR_WHITE,msg);
	//DisplayString(x1,y1,COLOR_WHITE,msg);

	if(bTwoBtn) {
		Button_init(&buttonctrl[0], x + 58, y + BOTTOM_Y-10, 185, 60,  // the cancel  button  
		   IMG_FILE_BUTTON_CANCEL_FOCUS, IMG_FILE_BUTTON_CANCEL_FOCUS, IMG_FILE_BUTTON_CANCEL_UNFOCUS, 0);
		Button_init(&buttonctrl[1], x + MSGBOX_WIDTH / 2+50, y + BOTTOM_Y-10, 185, 60, //OK  button 
			IMG_FILE_BUTTON_CONFIRM_FOCUS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);
		 
		//InitStringButton(&buttonctrl[0],x+8,y+BOTTOM_Y,MSGBOX_WIDTH/2-8,MSGBOX_HEIGHT-BOTTOM_Y, bTwoBtn);
		//InitStringButton(& buttonctrl[1],x+MSGBOX_WIDTH/2,y+BOTTOM_Y,MSGBOX_WIDTH/2-8,MSGBOX_HEIGHT-BOTTOM_Y,yes);
		buttonctrl[0].button.controlinfo.control_id = IDCANCEL;
		buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
	} else {
		//InitStringButton(&buttonctrl[1],x+8, y+BOTTOM_Y, MSGBOX_WIDTH-16, MSGBOX_HEIGHT-BOTTOM_Y, yes);
		Button_init(&buttonctrl[1], x + MSGBOX_WIDTH / 2-90, y + BOTTOM_Y - 8, 185, 60, //>>  
			IMG_FILE_BUTTON_CONFIRM_FOCUS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);
	}

    buttonctrl[1].button.controlinfo.control_id = IDOK;
    buttonctrl[1].button.controlinfo.sendmessage = btn_default;
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//确定按钮

	showmouse();

	ret = Window_run(&win);

	hidemouse();
	//restore img
	PutImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	EnableMouse(oldstate);

	if(bMouseDisplay)
		showmouse();
    
    FreePool(buff);
    return ret;
}


int DrawMsgBoxMultilLine(CHAR16 *msg1,CHAR16 *msg2,CHAR16 *msg3,CHAR16 *msg4,CHAR16 *yes,CHAR16 *no)
{
    WORD x=(g_WindowWidth-MSGBOX_WIDTH)/2;
    WORD y=(g_WindowHeight-MSGBOX_HEIGHT)/2;
    STRINGBUTTON buttonctrl[2] = { 0 };
    struct WINDOW win;
    int ret;
    WORD x1, y1, rows, width;
    BYTE oldstate, bMouseDisplay;
    //save img
#if OKR_DEBUG_UI //add by wh
	MyLog(LOG_MESSAGE, L"======-----=== DrawMsgBoxMultilLine.");

#endif
    PBYTE buff=AllocatePool(4*(MSGBOX_WIDTH+5)*(MSGBOX_HEIGHT+5));

	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	GetImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	Window_init(&win,NULL);


	DrawRect(0XFEFEFE, x, y, MSGBOX_WIDTH, MSGBOX_HEIGHT); //draw the BKG of the DLG 
	Linebox(x, y, MSGBOX_WIDTH, MSGBOX_HEIGHT, 0X010101); // draw the the edge add by wh 

//	DrawLinearRect(0XFF0000, 0x0000FF, x, y, MSGBOX_WIDTH, MSGBOX_HEIGHT);
//	DrawLine(x+8,y+BOTTOM_Y,MSGBOX_WIDTH-16,2,0xFFFFFF);
	if(no) {
		//DrawLine(g_WindowWidth/2,y+BOTTOM_Y +2,1,MSGBOX_HEIGHT-BOTTOM_Y-8 ,0xFF7F00);
		//DrawLine(g_WindowWidth/2+1,y+BOTTOM_Y+2,1,MSGBOX_HEIGHT-BOTTOM_Y-8 ,0x797979);
	}
	//Linebox(x-1,y-1,MSGBOX_WIDTH+2,MSGBOX_HEIGHT+2,0XF0F0F0);
	//Linebox(x-2,y-2,MSGBOX_WIDTH+4,MSGBOX_HEIGHT+4,0xcdcdcd);

	//我们取出来的是16点阵的字体
  if( msg1 != NULL  )
	{
	//	PrintMessageInRect(x+16, y+10, MSGBOX_WIDTH-24, BOTTOM_Y, msg1, COLOR_BLACK);
  	DisplayString(x+16, y+10+50, COLOR_SEC_BLUE, msg1);

  }
  if( msg2 != NULL  )
	{
//		PrintMessageInRect(x+16, y+10+30, MSGBOX_WIDTH-24, BOTTOM_Y, msg2, COLOR_BLACK);
   	DisplayString(x+16, y+10+30+50, COLOR_SEC_BLUE, msg2);

  }
  if( msg3 != NULL  )
	{
//		PrintMessageInRect(x+16, y+10+60, MSGBOX_WIDTH-24, BOTTOM_Y, msg3, COLOR_BLACK);
    	DisplayString(x+16, y+10+30+30+50, COLOR_SEC_BLUE, msg3);

  } 
  if( msg4 != NULL  )
	{
//		PrintMessageInRect(x+16, y+10+90, MSGBOX_WIDTH-24, BOTTOM_Y, msg4, COLOR_BLACK);
   	DisplayString(x+16, y+10+30+30+30+50, COLOR_SEC_BLUE, msg4);

  }
	//DisplayString(x+10,y+BOTTOM_Y/2 - 10,COLOR_WHITE,msg);
	//DisplayString(x1,y1,COLOR_WHITE,msg);

	if(no) {
		Button_init(&buttonctrl[0], x + 58, y + BOTTOM_Y - 10, 185, 60,  // the cancel  button  
			IMG_FILE_BUTTON_CANCEL_FOCUS, IMG_FILE_BUTTON_CANCEL_FOCUS, IMG_FILE_BUTTON_CANCEL_UNFOCUS, 0);
		Button_init(&buttonctrl[1], x + MSGBOX_WIDTH / 2 + 50 , y + BOTTOM_Y - 10, 185, 60, //OK  button 
			IMG_FILE_BUTTON_CONFIRM_FOCUS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);
			
		//InitStringButton(&buttonctrl[0],x+8,y+BOTTOM_Y,MSGBOX_WIDTH/2-8,MSGBOX_HEIGHT-BOTTOM_Y,no );
		//InitStringButton(&buttonctrl[1],x+MSGBOX_WIDTH/2,y+BOTTOM_Y,MSGBOX_WIDTH/2-8,MSGBOX_HEIGHT-BOTTOM_Y,yes);
		buttonctrl[0].button.controlinfo.control_id = IDCANCEL;
		buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
	} else {
		//InitStringButton(&buttonctrl[1],x+8, y+BOTTOM_Y, MSGBOX_WIDTH-16, MSGBOX_HEIGHT-BOTTOM_Y, yes);
		Button_init(&buttonctrl[1], x + MSGBOX_WIDTH / 2 - 90, y + BOTTOM_Y - 8, 185, 60, //>>  
			IMG_FILE_BUTTON_CONFIRM_FOCUS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);

	}

    buttonctrl[1].button.controlinfo.control_id = IDOK;
    buttonctrl[1].button.controlinfo.sendmessage = btn_default;
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//确定按钮

	showmouse();

	ret = Window_run(&win);

	hidemouse();
	//restore img
	PutImgEx(x,y,MSGBOX_WIDTH,MSGBOX_HEIGHT,buff);

	EnableMouse(oldstate);

	if(bMouseDisplay)
		showmouse();
    
    FreePool(buff);
    return ret;
}


//return IDCANCEL exit ,backup = IDB_BACKUP; return  recover = IDB_RECOVER ;
int StartFace()
{
    struct BUTTON buttonctrl[4] = { 0 };
    struct WINDOW win;
    DWORD ret;
    WORD x, y;
    RECT rc;

	g_data.Cmd = USER_COMMAND_FUNCTION_SELECT;
    CopyMem(&rc, &g_data.rWnd, sizeof(RECT));

	x = (g_WindowWidth - 560) / 2;
	y = (g_WindowHeight / 3 - 60) / 2;

	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	Button_init(&buttonctrl[0], rc.x + 124, y + 220, 196, 252,
    	IMG_FILE_BTN_BACKUP_FOCUS, IMG_FILE_BTN_BACKUP_FOCUS, IMG_FILE_BTN_BACKUP_NORMAL, 0 );
    Button_init(&buttonctrl[1], rc.x + 492, y + 220, 196, 252,
		IMG_FILE_BTN_RESTORE_FOCUS, IMG_FILE_BTN_RESTORE_FOCUS, IMG_FILE_BTN_RESTORE_NORMAL, 0 );
    //Button_init(&buttonctrl[0], rc.x + 74-50,rc.y+268-120,168,200, // BACKUP btn
    //	IMG_FILE_BTN_BACKUP_FOCUS, IMG_FILE_BTN_BACKUP_FOCUS, IMG_FILE_BTN_BACKUP_NORMAL, 0 );
    //Button_init(&buttonctrl[1], rc.x + 144+228-50,rc.y+268-120,168,200,// RESTORE btn
    //	IMG_FILE_BTN_RESTORE_FOCUS, IMG_FILE_BTN_RESTORE_FOCUS, IMG_FILE_BTN_RESTORE_NORMAL, 0 );
    //Button_init(&buttonctrl[2],rc.x + 442+228-50,rc.y+268-122,160,200,// install driver btn
	//  IMG_FILE_BUTTON_TEST_FOCUS,IMG_FILE_BUTTON_TEST_FOCUS, IMG_FILE_BUTTON_TEST, 0);

    Button_init(&buttonctrl[2], rc.x + 144 + 228 - 70, y + 510,150,48, // exit button
    	IMAG_FILE_EXIT_FOCUS, IMAG_FILE_EXIT_FOCUS, IMAG_FILE_EXIT_NORMAL, 0);
    	
	buttonctrl[0].controlinfo.control_id = IDB_BACKUP;// RESTORE btn
    buttonctrl[0].controlinfo.sendmessage = btn_default;
	buttonctrl[0].clear_background = TRUE;
    buttonctrl[1].controlinfo.control_id = IDB_RECOVER;// RESTORE btn
    buttonctrl[1].controlinfo.sendmessage = btn_default;
	buttonctrl[1].clear_background = TRUE;
	//buttonctrl[2].controlinfo.control_id = IDB_DRIVER_INSTALL; // install driver btn
	//buttonctrl[2].controlinfo.sendmessage = btn_default;
	//buttonctrl[2].clear_background = FALSE;
    //buttonctrl[3].controlinfo.control_id = IDB_EXIT; // exit button
    //buttonctrl[3].controlinfo.sendmessage=btn_default;
	//buttonctrl[3].clear_background = TRUE;
    buttonctrl[2].controlinfo.control_id = IDB_EXIT; // exit button
    buttonctrl[2].controlinfo.sendmessage=btn_default;
	buttonctrl[2].clear_background = TRUE;
	
    Window_init(&win,NULL);
    win.msgfunc = MyWindow_Dispatchmsg;
    
    //ClearBackground();
    hidemouse();
    
    DisplayImg(x, y-10,IMG_FILE_BACKGROUND_TITLE,FALSE); // lenovo 备份与恢复系统 title.bmp

    DisplayImg(x+160, y+100,IMAG_FILE_VERSION_NORMAL,FALSE); // 当前版本  version_normal.bmp

	DisplayString(x+140+150, y + 110, COLOR_SEC_BLUE, STRING_CUR_VERSION); // 当前版本号 ，从资源里取出的 //<> by wh change the color 

    DisplayImg((g_WindowWidth - 500)/2-110, y + 610,IMG_FILE_FOOT,FALSE);  // 按enter键确认... foot.bmp

	if (g_WindowHeight <= 800)
	{
		DisplayImg((g_WindowWidth - 500) / 2 + 80, g_WindowHeight - 84, IMAG_FILE_COYPY_RIGHT_MINI, 0); //  copyright-mini.bmp
	}
	else
	{
		DisplayImg((g_WindowWidth - 500) / 2 + 30, g_WindowHeight - 68, IMAG_FILE_COYPY_RIGHT, 0); //  copyright.bmp
	}

	showmouse();

    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//backup按钮
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//restore按钮
  //  Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//driverInstall按钮
  //  Window_addctl(&win, (PCONTROL)&buttonctrl[3]); 	//exit按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//exit按钮

	if(g_data.bDebug)
		MyLogString(NULL);

    return Window_run(&win);
}

///////////////////////////////////////////////////////////////////////  CHECK BOX
#ifndef SCROLL_DRIVE_LIST

#define WM_DRVBOX_UNSELECT 0xfe80
#define DRVTOTALWIDTH 460
#define WM_UPDATE_PART 0xfe81

#define DRVICONWIDTH  48//64
#define DRVICONHEIGHT 48//64

#define REAL_DRVICONWIDTH  48//64
#define REAL_DRVICONHEIGHT 48//64

void Drvbox_show( PCONTROL  pcontrol )
{
    PDRVBOX pcheck=(PDRVBOX)pcontrol;
    WORD icon;
    WORD delta_x, delta_y;
	BYTE bMouseDisplay;
	DWORD txtcolor = COLOR_BLACK;

    bMouseDisplay = hidemouse();

    //if (pcheck->ifremove)
    //    icon=IMG_FILE_REMOVEDISK;
    //else
    icon=IMG_FILE_HARDDISK;

    //FillRect(pcontrol->thex+1,pcontrol->they+2,pcontrol->thew-2,pcontrol->theh-4,CHECK_BG_COLOR);

	if (pcheck->status==1) {
   //    FillRect(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,FACE_SELECT_COLOR);
       FillRect(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,LISTBOX_SELLINE_COLOR);
  		    txtcolor = COLOR_WHITE;

	}
  else if(pcheck->status==2) 
  {
        FillRect(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,FACE_SELECT_COLOR);	//FACE_BACKGROUND_COLOR
		 //   txtcolor = COLOR_WHITE;
	}
	else
	{
       FillRect(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,COLOR_CLIENT);	//FACE_BACKGROUND_COLOR
		    txtcolor = COLOR_SEC_BLUE;
		 
	}	
	

    delta_x = (DRVICONWIDTH - REAL_DRVICONWIDTH)/2;
    delta_y = (DRVICONHEIGHT - REAL_DRVICONHEIGHT)/2;
    DisplayImg(pcontrol->thex+delta_x, pcontrol->they+delta_y, icon, TRUE);

   // DisplayStringInWidth(pcontrol->thex+DRVICONWIDTH+16, pcontrol->they+8, 
    //		DRVICONWIDTH+100, txtcolor, pcheck->volname);
    //DisplayString(pcontrol->thex,pcontrol->they+DRVICONHEIGHT,COLOR_BLACK,pcheck->volname);
    
    DisplayString(pcontrol->thex+DRVICONWIDTH+16, pcontrol->they+8,txtcolor,pcheck->volname);

    DisplayString(pcontrol->thex+DRVICONWIDTH+16,pcontrol->they+24,
    		txtcolor,pcheck->capname);

	if(bMouseDisplay)
		showmouse();
}

int Drvbox_Change(PDRVBOX pcheck,int status)
{
    PCONTROL pcontrol=(PCONTROL)pcheck;
    int i;
    struct DrvButtonList * plist=pcheck->plist;
 

		//MyLog(LOG_DEBUG, L"Drvbox, status 0x%x,pcheck->status:%x\n",status,pcheck->status);

    if (pcheck->status==status)
    {
    	 // if(pcheck->status==1)
    	  {
       //    pcheck->status=0;
       //    Drvbox_show((PCONTROL)pcheck);
    	  }	
    	  return 0;
    }
    if(status==2)
    {
    	 int tmp = 0;
    	 
  //    struct DM_MESSAGE dmessage;
        tmp = pcheck->status;
        pcheck->status=2; 
      	Drvbox_show((PCONTROL)pcheck);
        pcheck->status=tmp;

    }	
    else if (pcheck->status==1) {//unselect

        pcheck->status=0;
        Drvbox_show((PCONTROL)pcheck);
    } else {
        struct DM_MESSAGE dmessage;

        pcheck->status=1;
        Drvbox_show((PCONTROL)pcheck);

        //unselect other box
        for (i=0;i<plist->num;i++) {
            dmessage.message=WM_DRVBOX_UNSELECT;
            if (pcheck->indexinlist==i)
                continue;
            plist->icons[i].controlinfo.sendmessage(&plist->icons[i].controlinfo,&dmessage);
        }
        plist->selected=pcheck->indexinlist;
        if (plist->bindctl) {
            dmessage.message=WM_UPDATE_PART;
            dmessage.thex=plist->partindex[pcheck->indexinlist];
            plist->bindctl->sendmessage(plist->bindctl,&dmessage);
        }
    }

    return 0;
}

void New_CopyRight_contro_pre_dispatchmsg(struct CONTROL * pcontrol, struct DM_MESSAGE *dmmessage)
{ 
	int select = 0; 
	struct DM_MESSAGE mymsg;
    BOOL bRedraw = FALSE;
 
	Button_dispatchmsg(pcontrol, dmmessage); // 默认处理函数
  
	switch (dmmessage->message)
	{
	//case IDB_PAGE_PRE:
	//	MyLog(LOG_MESSAGE, L"pre IDB_PAGE_PRE \n");
	//case CHAR_LEFT:
	//	g_nLincenceBoxPage--;
	//	if (g_nLincenceBoxPage <= 0)
	//	{
	//		g_nLincenceBoxPage = 1;
	//	}
	//	MyLog(LOG_MESSAGE, L"pre CHAR_LEFT \n");
	//	break;
	//case CHAR_RIGHT:
	//	g_nLincenceBoxPage++;
	//	if (g_nLincenceBoxPage > NEW_LINCENCE_MAXPAGE)
	//	{
	//		g_nLincenceBoxPage = NEW_LINCENCE_MAXPAGE;
	//	}
	//	MyLog(LOG_MESSAGE, L"pre CHAR_LEFT \n");
	//	break;
	case WM_BUTTON_CLICK: 
		g_nLincenceBoxPage--;
		if (g_nLincenceBoxPage <= 1)
		{
			g_nLincenceBoxPage = 1;
			Control_disable(g_pLincenceboxPreBtn);
			Control_enable(g_pLincenceboxNexBtn);
			//DisplayImg(g_nLincenceboxPreBtnX, g_nLincenceboxPreBtnY, IMG_LINCENCE_BACK_03, FALSE); //  
		}
		else if (g_nLincenceBoxPage == NEW_LINCENCE_MAXPAGE)
		{
			Control_enable(g_pLincenceboxPreBtn);
			Control_disable(g_pLincenceboxNexBtn);
			//DisplayImg(g_nLincenceboxNextBtnX, g_nLincenceboxNextBtnY, IMG_LINCENCE_BACK_03, FALSE); //  
		}
		else
		{
			if (g_pLincenceboxPreBtn->button.controlinfo.status& CON_STATUS_DISABLE)
				Control_enable(g_pLincenceboxPreBtn);
			if (g_pLincenceboxNexBtn->button.controlinfo.status & CON_STATUS_DISABLE)
				Control_enable(g_pLincenceboxNexBtn);
		}
		bRedraw = TRUE;
		MyLog(LOG_MESSAGE, L" New_CopyRight WM_LEFTBTN %d \n", g_nLincenceBoxPage);
		break; 

	default:   
 
		break;
	}

	if (bRedraw)
	{ 
	    //DisplayImg(g_nLincenceboxPicX, g_nLincenceboxPicY, IMG_NEW_LINCENCE_PAGE_1-1+ g_nLincenceBoxPage, FALSE); // LINCENCE PAGE index
			DrawCurCopyrightPage(g_nLincenceBoxPage); 
		if (g_nLincenceBoxPage == NEW_LINCENCE_MAXPAGE)
		{
		   		MyLog(LOG_MESSAGE, L" New_CopyRight %d \n" , g_nLincenceBoxPage);  //   Button_show(&pcontrol->controlinfo);
		}

	} 
}

void New_CopyRight_contro_Next_dispatchmsg(struct CONTROL * pcontrol, struct DM_MESSAGE *dmmessage)
{ 
	int select = 0; 
	struct DM_MESSAGE mymsg; 
	BOOL bRedraw = FALSE;
 
	Button_dispatchmsg(pcontrol, dmmessage); // 默认处理函数 

	switch (dmmessage->message)
	{ 
 //   case IDB_PAGE_NEXT:
	//	 MyLog(LOG_MESSAGE, L" IDB_PAGE_NEXT \n");
	//case CHAR_LEFT:
	//	g_nLincenceBoxPage--;
	//	if (g_nLincenceBoxPage <= 0)
	//	{
	//		g_nLincenceBoxPage = 1;
	//	}
	//	MyLog(LOG_MESSAGE, L"Next CHAR_LEFT \n");
	//	break;
	//case CHAR_RIGHT:
	//	g_nLincenceBoxPage++;
	//	if (g_nLincenceBoxPage > NEW_LINCENCE_MAXPAGE)
	//	{
	//		g_nLincenceBoxPage = NEW_LINCENCE_MAXPAGE;
	//	}
	//	MyLog(LOG_MESSAGE, L"Next CHAR_LEFT \n");
	//	break;
	case WM_BUTTON_CLICK:
		g_nLincenceBoxPage++;
		if (g_nLincenceBoxPage >= NEW_LINCENCE_MAXPAGE)
		{
			g_nLincenceBoxPage = NEW_LINCENCE_MAXPAGE;

			 if (pcontrol->status & CON_STATUS_DISABLE)
				  Control_enable( g_pLincenceboxPreBtn);
			 Control_disable( g_pLincenceboxNexBtn);
 	//DisplayImg(g_nLincenceboxNextBtnX, g_nLincenceboxNextBtnY, IMG_LINCENCE_BACK_03, FALSE); //  
			
		}
		else if (g_nLincenceBoxPage == 1)
		{ 
			Control_disable(g_pLincenceboxPreBtn); 
			Control_enable(g_pLincenceboxNexBtn);
	    }
		else
		{
			if (g_pLincenceboxPreBtn->button.controlinfo.status& CON_STATUS_DISABLE)
 				Control_enable(g_pLincenceboxPreBtn);
			if (g_pLincenceboxNexBtn->button.controlinfo.status & CON_STATUS_DISABLE)
				Control_enable(g_pLincenceboxNexBtn); 
		}
		bRedraw = TRUE;
		//MyLog(LOG_MESSAGE, L" Next_dispa PRESSED CHAR_LEFT %d \n", g_nLincenceBoxPage);
		break;
 

	default:  //其余消息均发给绑定的控件
 
		break;
	}

	if (bRedraw)
	{
		//DisplayImg(g_nLincenceboxPicX, g_nLincenceboxPicY, IMG_NEW_LINCENCE_PAGE_1 - 1 + g_nLincenceBoxPage, FALSE);  // LINCENCE PAGE index
		//DisplayString(g_nLincenceboxPicX, g_nLincenceboxPicY, COLOR_BLACK, LicenseStr_01); //kang
		//DisplayStringInRect(g_nLincenceboxPicX, g_nLincenceboxPicY, COPYRIGHT_WIDTH,COPYRIGHT_HEIGTH, COLOR_BLACK, LicenseStr_1);
		DrawCurCopyrightPage(g_nLincenceBoxPage);
		MyLog(LOG_MESSAGE, L" New_CopyRight 6666 \n");
 
	}

}

void sel_disk_extern_control_dispatchmsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmmessage)
{
    WORD y;
    int select = 0;
    
    struct DM_MESSAGE mymsg;
    EXTERN_BUTTON *pextern_button=(EXTERN_BUTTON *)pcontrol;
	  BOOL bRedraw = TRUE;
    struct DrvButtonList * plist = (struct DrvButtonList * )pextern_button->pbindctl;

  	Button_dispatchmsg(pcontrol,dmmessage); // 默认处理函数


    switch (dmmessage->message) {

 //   case CHAR_LEFT:
 //   case CHAR_DOWN:
 //   case CHAR_RIGHT:
    	  
 //        break; 
//		case WM_DOUBLECLICKED:
    case WM_LEFTBUTTONPRESSED:
      
		    pextern_button->controlinfo.state = CONTROL_STATE_PRESSED;

        break;
    case WM_LEFTBUTTONRELEASE:
    		
    		pextern_button->controlinfo.state = CONTROL_STATE_NORMAL;

        switch ( pextern_button->controlinfo.control_id) {
        case IDB_PAGE_UP:
            pextern_button->controlinfo.status = 1;
               
            
          //  select = plist->selected-1; 
            
         //   MyLog(LOG_MESSAGE, L"select %d.",select);

          //  if( select >= 8)
            {
            //	mymsg.message=WM_CTL_INACTIVATE;
           // 	Control_sendmsg((struct CONTROL * )(&plist->icons[select-1]),&mymsg);
        //    	mymsg.message=CHAR_UP;//WM_CTL_ACTIVATE;
       //     	Control_sendmsg((struct CONTROL * )(&plist->icons[select]),&mymsg);
            }	
            
          //  mymsg.message=WM_CTL_ACTIVATE;
          //  Control_sendmsg(pextern_button->pbindctl,&mymsg);
       //     dmmessage->message = 0;

            break;
        case IDB_PAGE_DOWN:
          //  pextern_button->controlinfo.status = 1;
     				
          //  mymsg.message=CHAR_DOWN;
          //  Control_sendmsg(pextern_button->pbindctl,&mymsg);
      //      dmmessage->message = 0;
            
            break;
        }

        break;
    default:  //其余消息均发给绑定的控件
        bRedraw = FALSE;
        Control_sendmsg((PCONTROL)pextern_button->pbindctl,dmmessage);
        break;
    }
    
    if(bRedraw)
		{
			  Button_show(&pextern_button->controlinfo);
    }
    
  //  MyLog(LOG_MESSAGE, L" extern control over\n");

}

void Drvbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage)
{
  	int status;
  	PDRVBOX pcheck = (PDRVBOX)pcontrol;
    struct DrvButtonList * plist=pcheck->plist;

  	BYTE bMouseDisplay = hidemouse();

    switch (dmessage->message )
    {
    
    case CHAR_UP:
     	  if( pcheck->indexinlist== 0)
    	  {
    	  	
    	  	//Drvbox_Change(pcheck,1);
          dmessage->message = 0;

          //Control_disable(pcontrol);
        }
       // if( pcheck->indexinlist== plist->num)
        {
        	Control_enable(pcontrol);
        }
   	//		MyLog(LOG_DEBUG, L"Drvbox, message up 0x%x,indexinlist:%d\n",dmessage->message,pcheck->indexinlist);

    	  break;
    	  
    case CHAR_TAB:
   	//		MyLog(LOG_DEBUG, L"Drvbox, message CHAR_TAB 0x%x,indexinlist:%d\n",dmessage->message,pcheck->indexinlist);
      //  Control_disable(pcontrol);

        break;
    case CHAR_DOWN:
   	//		MyLog(LOG_DEBUG, L"Drvbox, message down 0x%x,indexinlist:%d\n",dmessage->message,pcheck->indexinlist);
        
      //  Control_disable(pcontrol);
    	//  Drvbox_Change(pcheck,1);
    	  
    	  if( pcheck->indexinlist== plist->num-1)
    	  {
    	  	//Drvbox_Change(pcheck,1);
          //
          Drvbox_Change(pcheck,1);
          dmessage->message = 0;
        }
        else
        {
        	
        	 Drvbox_Change(pcheck,0);
    
        }
        
     //   dmessage->message = 0;
    
        
	      break;
    case CHAR_LEFT:
    case CHAR_RIGHT:	
        Control_disable(pcontrol);
      //  Drvbox_Change(pcheck,1);
        break;
    
    case WM_CTL_ACTIVATE:

   	//		MyLog(LOG_DEBUG, L"Drvbox, message ACTIVATE 0x%x,indexinlist:%d\n",dmessage->message,pcheck->indexinlist);

    	  Drvbox_Change(pcheck,1);

  	//		MyLog(LOG_DEBUG, L"Drvbox, message ACTIVATE OVER\n");
    //    FillRect(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,LISTBOX_SELLINE_COLOR);
     // DotLinebox(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,LISTBOX_SELLINE_COLOR);
        break;
    case WM_CTL_INACTIVATE:
       // DotLinebox(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,RADIO_BG_COLOR);
   		//	MyLog(LOG_DEBUG, L"Drvbox, message INACTIVATE 0x%x,indexinlist:%d\n",dmessage->message,pcheck->indexinlist);
    	  Drvbox_Change(pcheck,0);

     //   Drvbox_Change(pcheck,0);
        break;
    case WM_LEFTBUTTONPRESSED:
    case L' ':
    case CHAR_RETURN:
        Drvbox_Change(pcheck,1);
        //dmessage->message=WM_CONTEXT_CHANGE;
        break;
    case WM_DRVBOX_UNSELECT:
    	//		MyLog(LOG_DEBUG, L"Drvbox, message UNSELECT 0x%x,indexinlist:%d\n",dmessage->message,pcheck->indexinlist);

        Drvbox_Change(pcheck,0);
    case WM_CTL_DISABLE:
        //Checkbox_Disable(pcontrol);
        break;
    case WM_CTL_ENABLE:
        //		pcheck->status=!(pcheck->status);
        //		Checkbox_Change(pcheck);
        //		Checkbox_Enable(pcontrol);
        Drvbox_show((PCONTROL)pcheck);
        break;
    case WM_BTN_SELCHANGE:
    	  Drvbox_Change(pcheck,2);

    	  break;
    }

	if(bMouseDisplay)
		showmouse();
}

int Drvbox_init(PDRVBOX pcheck,WORD x,WORD y,WORD width,WORD height,
		struct DrvButtonList *plist,CHAR16 *name,CHAR16 *name1,
		WORD indexinlist, WORD status,BOOL remove)
{
    pcheck->controlinfo.thex=x;
    pcheck->controlinfo.they=y;
    pcheck->controlinfo.thew=width;//todo
    pcheck->controlinfo.theh=height;
    pcheck->controlinfo.status=0;
    pcheck->controlinfo.type=IDC_CHECKBOX;
    pcheck->controlinfo.thez=0;
    pcheck->controlinfo.control_id = IDB_BUTTON_SHOW;


    pcheck->controlinfo.sendmessage=Drvbox_dispatchmsg;
    pcheck->controlinfo.show=Drvbox_show;
    pcheck->status=status;
    StrnCpy(pcheck->capname,name,sizeof(pcheck->capname)/sizeof(CHAR16));
    if (!name1 || name1[0] == 0)
    {
        if (remove)
            StrnCpy(pcheck->volname,L"Removable Drive",sizeof(pcheck->volname)/sizeof(CHAR16));
        else
            StrnCpy(pcheck->volname,L"Local Drive",sizeof(pcheck->volname)/sizeof(CHAR16));
    }
    else
    {
        StrnCpy(pcheck->volname,name1,sizeof(pcheck->volname)/sizeof(CHAR16));
    }
        
    pcheck->plist=plist;
    pcheck->indexinlist=indexinlist;
    pcheck->ifremove=remove;
    return 0;
}

//还原界面的硬盘选择，拿掉可移动设备
struct DrvButtonList* Recover_NewDriveButtons(PWINDOW pwin, WORD x, WORD y,
		WORD width, WORD height, int diskselected)
{
    WORD i;
    BYTE bRemovable = TRUE;
    int currentdisk = -1;
    WORD begin_x, begin_y;
    DWORD size, remain;
    struct DrvButtonList *buttonlist = AllocateZeroPool(sizeof(struct DrvButtonList));
	BYTE bMouseDisplay;
	
	CHAR16 *diskstring =NULL;
	CHAR16 *diskstring_data = L"数据硬盘";
	CHAR16 *diskstring_sys = L"系统硬盘";
	CHAR16 *diskstring_newhdd = L"新安装硬盘";

    bMouseDisplay = hidemouse();
    //DrawSinkableRectangle(x, y, width, height, COLOR_SHADOW_GREY, FACE_BACKGROUND_COLOR);
	DrawSinkableRectangle(x, y, width, height, COLOR_SHADOW_GREY, COLOR_LIST_BACK_WHITE);
    //FillRect(x,y, width, 333, FACE_BACKGROUND_COLOR);
	if(bMouseDisplay)
		showmouse();

    buttonlist->selected=-1;
    x+=10;
    begin_x=x;
    begin_y=y;

    i = 0;
	y += 16;
    while (i < g_disknum) {

		currentdisk = i;
		if( g_disklist[currentdisk].curdisktype == NEW_FOUND_DISK) {
        	
        	size = DivU64x32(g_disklist[currentdisk].totalsector, 2048);
			if (size > 4 * 1024) {
				//分区大小小于4GB，可以忽略
				WORD status = 0;
				CHAR16 name[100] = { 0 };
				
				
			//	if(g_disklist[currentdisk].curdisktype == DISK_DATA )
			//		diskstring = diskstring_data;
			//	else if(g_disklist[currentdisk].curdisktype == DISK_SYSDISK)
			//		diskstring = diskstring_sys;
			//	else if(g_disklist[currentdisk].curdisktype == NEW_FOUND_DISK)
					diskstring = diskstring_newhdd;
					
					
					
				if(g_disklist[currentdisk].disksn[0] == 0x20 || g_disklist[currentdisk].disksn[0] == 0) //sata
				{					
					if (size > 8*1024)
						SPrint(name, 100, L"%d GB %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c  %s", size/1024,g_disklist[currentdisk].disksn[1],\
					g_disklist[currentdisk].disksn[0],g_disklist[currentdisk].disksn[3],g_disklist[currentdisk].disksn[2],g_disklist[currentdisk].disksn[5],\
					g_disklist[currentdisk].disksn[4],g_disklist[currentdisk].disksn[7],g_disklist[currentdisk].disksn[6],g_disklist[currentdisk].disksn[9],\
					g_disklist[currentdisk].disksn[8],g_disklist[currentdisk].disksn[11],g_disklist[currentdisk].disksn[10],g_disklist[currentdisk].disksn[13],\
					g_disklist[currentdisk].disksn[12],g_disklist[currentdisk].disksn[15],g_disklist[currentdisk].disksn[14],g_disklist[currentdisk].disksn[17],\
					g_disklist[currentdisk].disksn[16],g_disklist[currentdisk].disksn[19],g_disklist[currentdisk].disksn[18],diskstring);
					else
						SPrint(name, 100, L"%d MB", size);
				}
				else
					
				{
					if (size > 8*1024)
						SPrint(name, 100, L"%d GB %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c  %s", size/1024,g_disklist[currentdisk].disksn[0],\
					g_disklist[currentdisk].disksn[1],g_disklist[currentdisk].disksn[2],g_disklist[currentdisk].disksn[3],g_disklist[currentdisk].disksn[4],\
					g_disklist[currentdisk].disksn[5],g_disklist[currentdisk].disksn[6],g_disklist[currentdisk].disksn[7],g_disklist[currentdisk].disksn[8],\
					g_disklist[currentdisk].disksn[9],g_disklist[currentdisk].disksn[10],g_disklist[currentdisk].disksn[11],g_disklist[currentdisk].disksn[12],\
					g_disklist[currentdisk].disksn[13],g_disklist[currentdisk].disksn[14],g_disklist[currentdisk].disksn[15],g_disklist[currentdisk].disksn[16],\
					g_disklist[currentdisk].disksn[17],g_disklist[currentdisk].disksn[18],g_disklist[currentdisk].disksn[19],diskstring);
					else
						SPrint(name, 100, L"%d MB", size);
					
				}
				
				
				
				if (diskselected == i) {
					status = 1;
					buttonlist->selected = buttonlist->num;
				}

				Drvbox_init(&buttonlist->icons[buttonlist->num], 
					x, y, width - 20, DRVICONHEIGHT+8,
					buttonlist, name, g_disklist[currentdisk].diskname,
					(WORD)buttonlist->num, (WORD)status, 
					g_disklist[currentdisk].removable);

				buttonlist->partindex[buttonlist->num]=i;
				buttonlist->num++;

				y += DRVICONHEIGHT+8;
				if(y + DRVICONHEIGHT+8 >= begin_y + height)	//超出窗口高度就不要显示了
					break;
			}
	    }
		i++;
		 
    }

    for (i=0;i<buttonlist->num;i++) {

        Window_addctl(pwin, (PCONTROL)&buttonlist->icons[i]);
    }
    return buttonlist;
}

////////////////////////////////////////////////////////////////////////// process box
//获取在list列表中显示的分区信息
struct DrvButtonList* NewDriveButtons(PWINDOW pwin, WORD x, WORD y,
		WORD width, WORD height, int diskselected)
{
    WORD i;
    BYTE bRemovable = TRUE;
    int currentdisk = -1;
    WORD begin_x, begin_y;
    DWORD size, remain;
    struct DrvButtonList *buttonlist = AllocateZeroPool(sizeof(struct DrvButtonList));
	BYTE bMouseDisplay;

    bMouseDisplay = hidemouse();
    //DrawSinkableRectangle(x, y, width, height, COLOR_SHADOW_GREY, FACE_BACKGROUND_COLOR);
	DrawSinkableRectangle(x, y, width, height, COLOR_SHADOW_GREY, COLOR_LIST_BACK_WHITE);
    //FillRect(x,y, width, 333, FACE_BACKGROUND_COLOR);
	if(bMouseDisplay)
		showmouse();

    buttonlist->selected=-1;
    x+=10;
    begin_x=x;
    begin_y=y;

    i = 0;
	y += 16;
    while (i < g_disknum) {

		currentdisk = i;
		if(bRemovable == g_disklist[currentdisk].removable) {
        	
        	size = DivU64x32(g_disklist[currentdisk].totalsector, 2048);
			if (size > 4 * 1024) {
				//分区大小小于4GB，可以忽略
				WORD status = 0;
				CHAR16 name[100] = { 0 };

				if (size > 8*1024)
					SPrint(name, 100, L"%d GB %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", size/1024,g_disklist[currentdisk].disksn[0],\
				g_disklist[currentdisk].disksn[1],g_disklist[currentdisk].disksn[2],g_disklist[currentdisk].disksn[3],g_disklist[currentdisk].disksn[4],\
				g_disklist[currentdisk].disksn[5],g_disklist[currentdisk].disksn[6],g_disklist[currentdisk].disksn[7],g_disklist[currentdisk].disksn[8],\
				g_disklist[currentdisk].disksn[9],g_disklist[currentdisk].disksn[10],g_disklist[currentdisk].disksn[11],g_disklist[currentdisk].disksn[12],\
				g_disklist[currentdisk].disksn[13],g_disklist[currentdisk].disksn[14],g_disklist[currentdisk].disksn[15],g_disklist[currentdisk].disksn[16],\
				g_disklist[currentdisk].disksn[17],g_disklist[currentdisk].disksn[18],g_disklist[currentdisk].disksn[19]);
				else
					SPrint(name, 100, L"%d MB", size);
				if (diskselected == i) {
					status = 1;
					buttonlist->selected = buttonlist->num;
				}

				Drvbox_init(&buttonlist->icons[buttonlist->num], 
					x, y, width - 20, DRVICONHEIGHT+8,
					buttonlist, name, g_disklist[currentdisk].diskname,
					(WORD)buttonlist->num, (WORD)status, 
					g_disklist[currentdisk].removable);

				buttonlist->partindex[buttonlist->num]=i;
				buttonlist->num++;

				y += DRVICONHEIGHT+8;
				if(y + DRVICONHEIGHT+8 >= begin_y + height)	//超出窗口高度就不要显示了
					break;
			}
	    }
		i++;
		if(i == g_disknum && bRemovable) {
			bRemovable = FALSE;
			i = 0;
		}
    }

    for (i=0;i<buttonlist->num;i++) {

        Window_addctl(pwin, (PCONTROL)&buttonlist->icons[i]);
    }
    return buttonlist;
}

void DeleteDriveButtons(struct DrvButtonList *p)
{
    if (p) FreePool(p);
}

///////////////////////////////////////////////////////
#endif

/////////////////////////////////////////////////////
// listbox of drives
void btn_dobackup(PCONTROL pcon,struct DM_MESSAGE* msg)
{
    Button_dispatchmsg(pcon,msg); // 默认处理函数
    if (msg->message==WM_BUTTON_CLICK)
    {
        if (g_DriveList->selected != -1) {
			Window_exit(pcon->pwin, pcon->control_id);
        }
    }
}

//返回< 0 表示失败，
//返回-1表示没有足够空间，
//    -2表示文件系统初始化失败，
//    -3表示有分区文件系统初始化失败，并且也有分区没有足够空间
//    -4表示没有可以支持的分区
int DetectAvailablePartition(int *partindex)
{
	UINT32 i;
	int candidate = -1;
	ULONGLONG maxfree = 0;
	BOOL bFileSystemInitFailed = FALSE;
	BOOL bHasSupportedPartition = FALSE;
	BOOL bInsufficientSpace = FALSE;

	if(g_DriveList->selected != -1) 
	{
		WORD diskindex = g_DriveList->partindex[g_DriveList->selected];
		
		*partindex = diskindex;
		
		return 1;
	}	
	
	if(g_DriveList->selected != -1) 
	{
		WORD diskindex = g_DriveList->partindex[g_DriveList->selected];
		FILE_SYS_INFO  sysinfo;
	  PBYTE buffer;
	  buffer = myalloc(1024*2048);
	  if(!buffer)
	  {
		   MyLog(LOG_DEBUG, L"DetectAvailablePartition, alloc memory failed.\n");
		   return FALSE;
	  }
		MyLog(LOG_DEBUG, L"disk %d, backupsize 0x%lx\n", diskindex, g_data.TotalBackupSize);

		for (i=0;i<g_dwTotalPartitions;i++) {

			if (diskindex != g_Partitions[i].DiskNumber)
				continue;
			if (DivU64x32(g_Partitions[i].TotalSectors, 2048*1024) < 4)	//分区大小小于8GB，可以忽略
				continue;
			if ((g_Partitions[i].PartitionType!=FAT32)&&(g_Partitions[i].PartitionType!=FAT32_E)&&(g_Partitions[i].PartitionType!=NTFS))
				continue;
			if (g_Partitions[i].bIfBackup)
				continue;

			bHasSupportedPartition = TRUE;
			//检查是否可以初始化
			if (FileSystemInit(&sysinfo,i,FALSE)==FALSE) {
				bFileSystemInitFailed = TRUE;
				continue;
			}
			//判断是否有工厂备份文件
			if(HasFactoryBackupInPartition(&sysinfo, i, buffer))
			{
//#if OKR_DEBUG //add by wh
//				if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) == 0 || g_Partitions[i].PartitionId == 0x07)
//#else 
				if(StrnCmp(g_Partitions[i].Label,L"LENOVO_PART", 11)==0 || g_Partitions[i].PartitionId == 0x12)
//#endif
				{
					FileSystemClose(&sysinfo);
					continue;     	
				}	
			}

			FileSystemClose(&sysinfo);

			//
			if(maxfree < g_Partitions[i].FreeSectors) {
				maxfree = g_Partitions[i].FreeSectors;
				candidate = i;
			}

			//约0.7的压缩比， 512 / 70% = 728
			if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,728)) {
      //if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,512)) {
				bInsufficientSpace = TRUE;
				continue;
			}

			//检查是否服务分区
			MyLog(LOG_DEBUG,L"Found partition %d, type %x, %x(%d GB)\n", 
				i, g_Partitions[i].PartitionType, g_Partitions[i].TotalSectors,
				DivU64x32(g_Partitions[i].TotalSectors, 2048*1024));
		}

    myfree(buffer);
    
		if(candidate >= 0) {
			*partindex = candidate;
				
			//约0.7的压缩比， 512 / 70% = 728
      if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize,728)){
      //if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize,512)){
			
					return candidate;
			}
		}
	} 

	

	if(!bHasSupportedPartition)
		return -4;
	if(bInsufficientSpace && bFileSystemInitFailed)
		return -3;
	if(bFileSystemInitFailed)
		return -2;
	return -1;
}

//返回< 0 表示失败，
//返回-1表示没有足够空间，
//    -2表示文件系统初始化失败，
//    -3表示有分区文件系统初始化失败，并且也有分区没有足够空间
//    -4表示没有可以支持的分区
int DetectAvailableOEMPartition(int *partindex)
{
	UINT32 i = 0;
	int candidate = -1;
	ULONGLONG maxfree = 0;
	BOOL bFileSystemInitFailed = FALSE;
	BOOL bHasSupportedPartition = FALSE;
	BOOL bInsufficientSpace = FALSE;

  if(1) 
  {
		WORD diskindex = 0;
		FILE_SYS_INFO  sysinfo;

		MyLog(LOG_DEBUG, L"disk %d, backupsize 0x%lx\n", diskindex, g_data.TotalBackupSize);

		for (i=0;i<g_dwTotalPartitions;i++) 
		{
  		MyLog(LOG_DEBUG, L"i %d, Label %s\n", i,g_Partitions[i].Label);

			if(StrnCmp(g_Partitions[i].Label,L"LENOVO_PART", 11)!=0)
      {
//#if OKR_DEBUG //add by wh		
//      	if(g_Partitions[i].PartitionId != 0x07)
//#else 
		if (g_Partitions[i].PartitionId != 0x12)
//#endif
		{
      		continue;
      	}	      	
      }	
			if (DivU64x32(g_Partitions[i].TotalSectors, 2048*1024) < 4)	//分区大小小于8GB，可以忽略
				continue;
			if ((g_Partitions[i].PartitionType!=FAT32)&&(g_Partitions[i].PartitionType!=FAT32_E)&&(g_Partitions[i].PartitionType!=NTFS))
				continue;
			if (g_Partitions[i].bIfBackup)
				continue;

			bHasSupportedPartition = TRUE;
			//检查是否可以初始化
			if (FileSystemInit(&sysinfo,i,FALSE)==FALSE) {
				bFileSystemInitFailed = TRUE;
				continue;
			}
			FileSystemClose(&sysinfo);

			//
			if(maxfree < g_Partitions[i].FreeSectors) {
				maxfree = g_Partitions[i].FreeSectors;
				candidate = i;
			}

			//约0.7的压缩比， 512 / 70% = 728
			if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,728)) {
      //if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,512)) {
				bInsufficientSpace = TRUE;
				continue;
			}

			//检查是否服务分区
			MyLog(LOG_DEBUG,L"Found partition %d, type %x, %x(%d GB)\n", 
				i, g_Partitions[i].PartitionType, g_Partitions[i].TotalSectors,
				DivU64x32(g_Partitions[i].TotalSectors, 2048*1024));
		}

		if(candidate >= 0) {
			*partindex = candidate;

			//约0.7的压缩比， 512 / 70% = 728
      if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize,728)){
      //if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize,512)){
				return candidate;
       }
		}
	} 

	if(!bHasSupportedPartition)
		return -4;
	if(bInsufficientSpace && bFileSystemInitFailed)
		return -3;
	if(bFileSystemInitFailed)
		return -2;
	return -1;
}

//返回< 0 表示失败，
//返回-1表示没有足够空间，
//    -2表示文件系统初始化失败，
//    -3表示有分区文件系统初始化失败，并且也有分区没有足够空间
//    -4表示没有可以支持的分区
int DetectAvailableAssignPartition(int partindex)
{
	DWORD i = partindex;
	int candidate = -1;
	ULONGLONG maxfree = 0;
	BOOL bFileSystemInitFailed = FALSE;
	BOOL bHasSupportedPartition = FALSE;
	BOOL bInsufficientSpace = FALSE;


		MyLog(LOG_DEBUG, L"DetectAvailableAssignPartition\n");

		if (1) 
		{
		  WORD diskindex = 0;
		  FILE_SYS_INFO  sysinfo;
			
			bHasSupportedPartition = TRUE;
			//检查是否可以初始化
			if (FileSystemInit(&sysinfo,i,FALSE)==FALSE) {
				bFileSystemInitFailed = TRUE;
				MyLog(LOG_DEBUG,L"bFileSystemInitFailed\n"); 
				goto err;
			}
			FileSystemClose(&sysinfo);

			//
			if(maxfree < g_Partitions[i].FreeSectors) {
				maxfree = g_Partitions[i].FreeSectors;
				candidate = i;
				MyLog(LOG_DEBUG,L"maxfree < g_Partitions[i].FreeSectors\n"); 
			}

			//约0.7的压缩比， 512 / 70% = 728
			//if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,728)) {
			if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,512)) {
				
				MyLog(LOG_DEBUG,L"FreeSectors = %llx, type %x, %llx\n", 
				g_Partitions[i].FreeSectors, g_Partitions[i].PartitionType, DivU64x32(g_data.TotalBackupSize,512));
				
				
				bInsufficientSpace = TRUE;
				goto err;
			}

			//检查是否服务分区
			MyLog(LOG_DEBUG,L"Found partition %d, type %x, %x(%d GB)\n", 
				i, g_Partitions[i].PartitionType, g_Partitions[i].TotalSectors,
				DivU64x32(g_Partitions[i].TotalSectors, 2048*1024));
		}

		if(candidate >= 0) {
				MyLog(LOG_DEBUG,L"candidate >= 0\n"); 
			if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize,512))
				return candidate;
		}
	
	
err:
	
	if(!bHasSupportedPartition)
		return -4;
	if(bInsufficientSpace && bFileSystemInitFailed)
		return -3;
	if(bFileSystemInitFailed)
		return -2;
	return -1;
}

//return 0 if successfully
//desc: backup name,
//partindex: -1not selected
int BackupSetting(CHAR16 *desc, int *partindex)
{
    struct BUTTON buttonctrl[3];
    struct WINDOW win;
    DWORD ret;
    EDITBOX editbox;

    UINT64 data;
    CHAR16 space[80] = { 0 };
    CHAR16 capspace[80] = { 0 };
    WORD i, x, y;
    RECT rc;
    int result = 0;
    int img_err_count = 0;
    
    struct BUTTON externbuttonctrl[2] = { 0 };

	MyLog(LOG_ERROR, L"-----wh--------BackupSetting...\n");   


	g_data.Cmd = USER_COMMAND_BACKUP_SELECT;
    
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
///* 
    result = ProbeSystemPartition() ;
    if ( result == FALSE)
    {
        DrawMsgBox(STRING_HINTNOSYSPART,STRING_YES,STRING_NO);
        return -1;
    }
    else if ( result == -1)
    {
        DrawMsgBox(STRING_FIND_SYS_ERROR,STRING_YES,STRING_NO);
        return -1;
    }
 

    GetAllBackupFiles(&img_err_count);

    CheckAllBackupFiles();
       
	if(g_data.bHasFactoryBackup < 0) {
		g_data.bHasFactoryBackup = HasFactoryBackup();
	}

//*/
	SPrint(space,80,L"%s", STRING_BACKUP_SIZE);

    //show message box
	g_data.TotalBackupSize = GetBackupTotaldata(); //20 * 1024 * 1024;// GetBackupTotaldata();
	data = DivU64x32(g_data.TotalBackupSize,1024*1024);
    if (data > 1024)
        SPrint(capspace,80,L"%d.%d GB", data/1024, (data%1024)/10);
    else
        SPrint(capspace,80,L"%d MB", data);
	y = (g_WindowHeight / 3 - 60) /2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	Button_init(&buttonctrl[0], rc.x+rc.width-500, y + 686, 185,60,
    	IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT, 0);//ysy 1230 560>686
	Button_init(&buttonctrl[1], g_x_begin-80, y - 5, 64,64,
		IMAG_FILE_GOBACK_FOCUS, IMAG_FILE_GOBACK_FOCUS, IMG_FILE_GOBACK, 0);

 //Button_init(&buttonctrl[0], rc.x+rc.width-500, y + 560, 185,60,
    //	IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT, 0);//ysy 1226
  
  
  
  
  
    Button_init((PBUTTON)&externbuttonctrl[0],rc.x+120+rc.width-135,y+439, 28, 28,IMAG_FILE_BTN_UP_FOCUS,IMAG_FILE_BTN_UP,IMAG_FILE_BTN_UP, 0);//ysy 1230 313>565
    Button_init((PBUTTON)&externbuttonctrl[1],rc.x+120+rc.width-135,y+474, 28, 28,IMAG_FILE_BTN_DOWN_FOCUS,IMAG_FILE_BTN_DOWN,IMAG_FILE_BTN_DOWN, 0);//ysy 1230 348>600
    
//Button_init((PBUTTON)&externbuttonctrl[0],rc.x+120+rc.width-135,y+313, 28, 28,IMAG_FILE_BTN_UP_FOCUS,IMAG_FILE_BTN_UP,IMAG_FILE_BTN_UP, 0);
  //  Button_init((PBUTTON)&externbuttonctrl[1],rc.x+120+rc.width-135,y+348, 28, 28,IMAG_FILE_BTN_DOWN_FOCUS,IMAG_FILE_BTN_DOWN,IMAG_FILE_BTN_DOWN, 0);


	Editbox_init(&editbox, rc.x + 160, y + 621, 601, desc, EDIT_MODE_NORMAL, 30);//ysy 1230 495>621
 // 	Editbox_init(&editbox, rc.x + 160, y + 495, 601, desc, EDIT_MODE_NORMAL, 30);
	g_editbox = &editbox;
        


    buttonctrl[0].controlinfo.control_id = IDB_BACKUP;
    buttonctrl[0].controlinfo.sendmessage = btn_dobackup;
	buttonctrl[0].clear_background = TRUE;
    buttonctrl[1].controlinfo.control_id = IDCANCEL;
    buttonctrl[1].controlinfo.sendmessage = btn_default;
    //buttonctrl[2].controlinfo.control_id = IDCANCEL;
    //buttonctrl[2].controlinfo.sendmessage = btn_default;
  
  
    externbuttonctrl[0].controlinfo.control_id = IDB_PAGE_UP;
	  externbuttonctrl[0].controlinfo.sendmessage = sel_disk_extern_control_dispatchmsg;	
	
	  externbuttonctrl[1].controlinfo.control_id = IDB_PAGE_DOWN;
	  externbuttonctrl[1].controlinfo.sendmessage = sel_disk_extern_control_dispatchmsg;

  
    Window_init(&win,NULL);
    
    
  //  win.msgfunc=MyWindow_Dispatchmsg;
    win.msgfunc=SelectDiskMyWindow_Dispatchmsg;


    ClearScreen();

 // 	Editbox_init(&editbox, rc.x + 160, y + 495, 601, desc, EDIT_MODE_NORMAL, 30);
  
    g_DriveList = NewDriveButtons(&win, g_x_begin + 40, y + 125, 736, 378, g_data.BackupDiskNumber);//ysy 1230 252>378
    //g_DriveList = NewDriveButtons(&win, g_x_begin + 40, y + 125, 736, 252, g_data.BackupDiskNumber);

    Window_addctl(&win, (PCONTROL)&editbox); 	//EDIT按钮 交换tab顺序 
    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
    
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//取消按钮
   
   // g_DriveList = NewDriveButtons(&win, g_x_begin+336+30, g_y_begin+90+25, 450, 320+48, g_data.BackupDiskNumber);

    externbuttonctrl[0].pbindctl = g_DriveList;
    externbuttonctrl[1].pbindctl = g_DriveList;
	 
	  Window_addctl(&win, (PCONTROL)&externbuttonctrl[0]); 	//翻页按钮
	  Window_addctl(&win, (PCONTROL)&externbuttonctrl[1]); 	//翻页按钮


   // win.current = g_DriveList->num;

    hidemouse();

    //DrawLine(rc.x+20,rc.y+70,rc.width-20,1,0xf13326);
    //DrawLine(rc.x+20,rc.y+71,rc.width-20,1,0xF6F7F9);
    //DrawLine(rc.x + rc.width - 40, rc.y + 15, 1, 25, 0xD0D4D3);
 //   DisplayString(rc.x+50,rc.y+150,COLOR_TEXT,STRING_BACKUPHINT);

    DisplayString(g_x_begin + 40, y + 90, COLOR_SEC_BLUE,STRING_BACKUPHINT);
 //   g_DriveList = NewDriveButtons(&win, g_x_begin + 40, y + 125, 736, 252, g_data.BackupDiskNumber);
 
    DisplayString(g_x_begin + 40, y + 576, COLOR_SEC_BLUE,space);//ysy 1230 450>576
    //  DisplayString(g_x_begin + 40, y + 450, COLOR_SEC_BLUE,space);
  
  // DisplayString(g_x_begin + 40 + 180, y + 450, COLOR_SEC_BLUE,capspace);
    DisplayString(g_x_begin + 40 + 180, y + 576, COLOR_SEC_BLUE,capspace);//ysy 1230 450>576
    
	
	 //  DisplayString(g_x_begin + 40, y + 500, COLOR_SEC_BLUE,STRING_INPUTNAME);
    DisplayString(g_x_begin + 40, y + 626, COLOR_SEC_BLUE,STRING_INPUTNAME);//ysy 1230 500>626
 
 
 
 
 //DisplayString(g_x_begin + 40, y + 387, COLOR_RED,STRING_REMOVE_DES);
    DisplayString(g_x_begin + 40, y + 513, COLOR_RED,STRING_REMOVE_DES);//ysy 1230  387>513
 
  
  
    //DisplayImg(g_x_begin+90, y+10, IMG_FILE_GOBACK,FALSE);
    DisplayImg(g_x_begin+260+30, y, IMAG_FILE_SYS_BK_TITLE,FALSE);	//IMG_FILE_BACKUP_TITLE
	//x = (g_WindowWidth - 560) / 2;
	//DisplayImg(x, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE

	//DisplayImg(g_x_begin + 130, y + 90, IMAG_FILE_TIP_BACKUP_SELECT_STORE_DEVICE, FALSE);
	//DisplayImg(g_x_begin + 130, y + 400, IMAG_FILE_TIP_BACKUP_HOT_PNP, FALSE);
	//DisplayImg(g_x_begin + 130, y + 440, IMAG_FILE_TIP_BACKUP_DATA_SIZE, FALSE);
	//DisplayImg(g_x_begin + 130, y + 480, IMAG_FILE_TIP_BACKUP_MEMO, FALSE);

    showmouse();

	if(g_data.bDebug)
		MyLogString(NULL);

    ret = Window_run(&win);

    if (ret == IDB_BACKUP) 
    {
    	//选择的是磁盘，而不是分区，要自动计算哪个分区最合适

		int destpart;

		destpart = DetectAvailablePartition(partindex);
   	   	if(destpart < 0) {
			if(destpart == -4)
				DrawMsgBox(STRING_DRIVE_NO_SUPPORT_FS, STRING_YES, NULL);
			else if(destpart == -2)
			//	DrawMsgBox(STRING_FS_INIT_FAILED, STRING_YES, NULL);
				DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1,STRING_FS_INIT_FAILED_2,STRING_FS_INIT_FAILED_3,NULL,STRING_YES, NULL);
			else if(destpart == -1)
				DrawMsgBox(STRING_DISK_TOO_SMALL_TO_BACKUP, STRING_YES, NULL);
			else
				DrawMsgBox(STRING_DRIVE_MIGHT_SMALL, STRING_YES, NULL);

			ret = IDCANCEL;

		} 

#if OKR_DEBUG
	MyLog(LOG_ERROR,L"auto-selected partition %d.\n", *partindex);
	//MyLog(LOG_ERROR,L"press any key to continue...\n");
	//getch();
#endif
    }
    DeleteDriveButtons(g_DriveList);
 // getch();
//  ret = IDCANCEL;
	return ret;
}

 

int HasSystemPart(void)
{
	int ret = 0;
	DWORD index = 0;
  for(index=0;index<g_dwTotalPartitions;index++)
  {
     if(g_Partitions[index].bIfBackup == TRUE&&g_Partitions[index].HasWindows == TRUE)
     {
     	  return 1;
     }	 	
  }
  
  return ret;
}
//return 0 if successfully
//desc: backup name,
//partindex: -1not selected
int InitialBackupFace(int *partindex)
{
    struct BUTTON buttonctrl[3];
    struct WINDOW win;
    DWORD ret;
    EDITBOX editbox;
    UINT64 data;

    WORD i, x, y;
    RECT rc;
    int result = 0;
    
    CopyMem(&rc, &g_data.rWnd, sizeof(RECT));

#if OKR_DEBUG_UI //add by wh

	MyLog(LOG_ERROR, L"InitialBackupFace ret:%x.\n", ret);
 
#endif


    result = ProbeSystemPartition() ;
    if ( result == FALSE)
    {
        DrawMsgBox(STRING_HINTNOSYSPART,STRING_YES,STRING_NO);
        return 3;
    }
    else if ( result == -1)
    {
        DrawMsgBox(STRING_FIND_SYS_ERROR,STRING_YES,STRING_NO);
        return 3;
    }    


	 if(g_data.bHasFactoryBackup < 0)
   {
		   g_data.bHasFactoryBackup = HasFactoryBackup();
 
		   
	 }
  
   if(g_data.bHasFactoryBackup == TRUE)
   {
  	  return -2;
   }
 
   if( HasSystemPart()== 0 && IsLinuxSystem == FALSE)
   {
       DrawMsgBox(STRING_HINTNOSYSPART,STRING_YES,STRING_NO);

    	 return 3;
   }
  
   g_data.Cmd = USER_COMMAND_DRIVER_INSTALL;

   g_data.TotalBackupSize = GetBackupTotaldata();
//*/
	y = (g_WindowHeight / 3 - 60) /2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	Button_init(&buttonctrl[0], rc.x+rc.width-500, y + 560, 185,60,
    	IMG_FILE_BUTTON_CONFIRM_FOCUS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);
	  Button_init(&buttonctrl[1], g_x_begin - 80, y - 5, 64,64,
		IMAG_FILE_GOBACK_FOCUS, IMAG_FILE_GOBACK_FOCUS, IMG_FILE_GOBACK, 0);

    buttonctrl[0].controlinfo.control_id = IDB_IDB_INIT_BACKUP;//
    buttonctrl[0].controlinfo.sendmessage = btn_dobackup;
	  buttonctrl[0].clear_background = TRUE;
    buttonctrl[1].controlinfo.control_id = IDOK;
    buttonctrl[1].controlinfo.sendmessage = btn_default;
//    buttonctrl[2].controlinfo.control_id = IDCANCEL;
 //   buttonctrl[2].controlinfo.sendmessage = btn_default;
    
    Window_init(&win,NULL);
    win.msgfunc=MyWindow_Dispatchmsg;

    ClearScreen();


    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//取消按钮
 //   Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//取消按钮

  //  Window_addctl(&win, (PCONTROL)&editbox); 	//确定按钮

    hidemouse();
   
    //DisplayImg(g_x_begin+90, y+10, IMG_FILE_GOBACK,FALSE);
    
    DisplayImg(g_x_begin+260+30, y, IMAG_FILE_INITIAL_BK,FALSE);	//

    DisplayImg(rc.x+50, y + 330, IMAG_FILE_INITIAL_BK_TXT,FALSE);	//


    showmouse();

	if(g_data.bDebug)
		MyLogString(NULL);

    ret = Window_run(&win);

  	MyLog(LOG_ERROR,L"InitialBackupFace ret:%x.\n", ret);

    if (ret == IDB_IDB_INIT_BACKUP) 
    { 
	 
		//选择的是磁盘，而不是分区，要自动计算哪个分区最合适
		int destpart = 0;
    
		destpart = DetectAvailableOEMPartition(partindex);
   	   	if(destpart < 0) {
			if (destpart == -4)
				//DrawMsgBox(STRING_DRIVE_NO_SUPPORT_FS, STRING_YES, NULL);
				;//The msgbox won't be shown, skip it and the user backup selection page will be shown. 
			else if(destpart == -2)
				//DrawMsgBox(STRING_FS_INIT_FAILED, STRING_YES, NULL);
 	 			DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1,STRING_FS_INIT_FAILED_2,STRING_FS_INIT_FAILED_3,NULL,STRING_YES, NULL);
			else if(destpart == -1)
				DrawMsgBox(STRING_OEM_PART_TOO_SMALL_TO_BACKUP, STRING_YES, NULL);
			else
				DrawMsgBox(STRING_DRIVE_MIGHT_SMALL, STRING_YES, NULL);
    		ret = IDCANCEL;
     
			//if(destpart == -1)
			if(destpart < 0)
			{	
				g_data.Cmd = USER_COMMAND_BACKUP_SELECT;
				return -2;
			}
		} 
	}
	 g_data.Cmd = USER_COMMAND_BACKUP_SELECT;
   

	 return ret;
}




INTN AddBackupLine(PLISTBOX plistbox)
{
    CHAR16 buffer[100];
    WORD i;

    for (i=0;i<g_data.ImageFilesNumber;i++) {
		struct mytm tm;
        struct mytm *ptm;
		time_t filetime = g_data.images[i].FileTime;
		if(filetime == 0)
			filetime = g_data.images[i].Time;
        Listbox_Addoneline(plistbox);

        ptm = mygmtime(&filetime, &tm);
		if(ptm) {
			SPrint(buffer,100,L"%02d/%02d/%02d %02d:%02d:%02d",(UINTN)(1900+ptm->tm_year),(UINTN)ptm->tm_mon,(UINTN)ptm->tm_mday,(UINTN)ptm->tm_hour,(UINTN)ptm->tm_min,(UINTN)ptm->tm_sec);

			Listbox_Addoneunit(plistbox,ITEM_TYPE_CHAR_STRING,buffer);
			SetMem(buffer,200,0);
			//SPrint(buffer, 100, L"%s " STRING_FACTORY_BACKUP, g_data.images[i].Memo);
			if(g_data.images[i].bFactoryBackup)
				CopyMem(buffer,STRING_FACTORY_BACKUP,sizeof(STRING_FACTORY_BACKUP));
			else
				CopyMem(buffer,g_data.images[i].Memo,100);
			Listbox_Addoneunit(plistbox,ITEM_TYPE_CHAR_STRING,buffer);
		}
    }

	////添加一些测试数据
	//for(;i<20;i++) {
	//	SPrint(buffer,100,L"Item %d", i);
	//	Listbox_Addoneline(plistbox);
	//	Listbox_Addoneunit(plistbox,ITEM_TYPE_CHAR_STRING,buffer);
	//}
	//return i;

    return g_data.ImageFilesNumber;
}

void recoverlist_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage)
{
    //PLISTBOX plist=(PLISTBOX)pcontrol;
    Listbox_DispatchMsg(pcontrol, dmessage);
    //if (dmessage->message==WM_UPDATE_PART) {
    //	
    //    Listbox_delallitem(plist);
    //    AddBackupLine(plist,dmessage->thex);
    //    Listbox_updateview(plist);
    //}
}



void btn_dorecover(PCONTROL pcon,struct DM_MESSAGE* msg)
{
    Button_dispatchmsg(pcon,msg); // 默认处理函数

//MyLog(LOG_ERROR, L"selected %d\n", plistbox->firstitem_inwindow + plistbox->whichselect);

    if (msg->message == WM_BUTTON_CLICK) {

		if (pcon->control_id == IDB_DELETE) {
			int index = plistbox->firstitem_inwindow + plistbox->whichselect;
			if(!g_data.images[index].bFactoryBackup){
				//delete image file.
				FILE_SYS_INFO  sysinfo;
				WORD partindex = (WORD)g_data.images[index].PartitionIndex;

				//需要弹出提示要求确认
				if(DrawMsgBox(STRING_DELETE_CONFIRM, STRING_YES, STRING_NO) == IDOK) {
					if (FileSystemInit(&sysinfo, partindex, FALSE) != FALSE) {
						CHAR16 name[100];
						DWORD count = g_data.images[index].FileNumber;
						DWORD i;
						BOOL bDeleted = FALSE;
						if(!count)
							count = 1;
						for(i=0;i<count;i++)
						{
							if(i == 0)
								SPrint(name, 100, BACKUP_DIR L"\\%s", g_data.images[index].FileName);
							else
								SPrint(name, 100, BACKUP_DIR L"\\%s.%d", g_data.images[index].FileName, i);
							bDeleted = FileDelete(&sysinfo, name);
						}
						if(bDeleted) {
							//删除备份点以后，就要增加这个文件所在的分区的剩余空间
							g_Partitions[partindex].FreeSectors += g_data.images[index].FileSize/SECTOR_SIZE;
						}

						FileSystemClose(&sysinfo);
            
            if(bDeleted==FALSE)
             //   DrawMsgBox(STRING_NTFS_DELET_ERROR, STRING_YES, NULL);
        				DrawMsgBoxMultilLine(STRING_NTFS_DELET_ERROR_1,STRING_NTFS_DELET_ERROR_2,STRING_NTFS_DELET_ERROR_3,NULL,STRING_YES, NULL);
    
						Window_exit(pcon->pwin, pcon->control_id);
					}
					else
					{
						//DrawMsgBox(STRING_NTFS_DELET_ERROR, STRING_YES, NULL);
					  DrawMsgBoxMultilLine(STRING_NTFS_DELET_ERROR_1,STRING_NTFS_DELET_ERROR_2,STRING_NTFS_DELET_ERROR_3,NULL,STRING_YES, NULL);
					}	
					
				}
			}
		} else if (plistbox->item_num) {
            Window_exit(pcon->pwin, pcon->control_id);
		}
    }
}

PART_LIST_INFO g_part_list_info[128];

INTN AddPartListLine(PLISTBOX plistbox,WORD diskindex)
{
    CHAR16 buffer[100];
    WORD i=0,index = 0;
    
    SetMem(g_part_list_info,sizeof(g_part_list_info),0);
    
  	for (i=0;i<g_dwTotalPartitions;i++) 
  	{

			if (diskindex != g_Partitions[i].DiskNumber)
				continue;
			if (DivU64x32(g_Partitions[i].TotalSectors, 2048*1024) < 4)	//分区大小小于8GB，可以忽略
				continue;
//#if OKR_DEBUG //add by wh
//			if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) == 0 || g_Partitions[i].PartitionId == 0x07)
//#else 
			if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) == 0 || g_Partitions[i].PartitionId == 0x12)
//#endif	
      {
     	   continue;
      }	
      
      if( g_Partitions[i].PartitionType == FAT16_E || g_Partitions[i].PartitionType == FAT16)
      {
      	continue;
      } 
      	       
			if ((g_Partitions[i].PartitionType!=FAT32)&&(g_Partitions[i].PartitionType!=FAT32_E)&&(g_Partitions[i].PartitionType!=NTFS))
				continue;
			if (g_Partitions[i].bIfBackup)
				continue;
			
			g_part_list_info[index].flag = 1;
			g_part_list_info[index].partindex = i;
			g_part_list_info[index].disknum = g_Partitions[i].DiskNumber;
			g_part_list_info[index].PartitionType = g_Partitions[i].PartitionType;
			g_part_list_info[index].TotalSectors = g_Partitions[i].TotalSectors;
		
		  SPrint(g_part_list_info[index].partname,16,L"分区%d",index);	
		 	
			if ((g_Partitions[i].PartitionType==FAT32)||(g_Partitions[i].PartitionType==FAT32_E))
			{
			   SPrint(buffer,100,L"%s    文件系统：FAT32    分区容量：%d G",g_part_list_info[index].partname,DivU64x32(g_part_list_info[index].TotalSectors, 2048*1024));				
			}
			else if ((g_Partitions[i].PartitionType==FAT16)||(g_Partitions[i].PartitionType==FAT16_E))
			{
			   SPrint(buffer,100,L"%s    文件系统：FAT16    分区容量：%d G",g_part_list_info[index].partname,DivU64x32(g_part_list_info[index].TotalSectors, 2048*1024));			
			}
			else if (g_Partitions[i].PartitionType==NTFS)
			{
			   SPrint(buffer,100,L"%s    文件系统：NTFS     分区容量：%d G",g_part_list_info[index].partname,DivU64x32(g_part_list_info[index].TotalSectors, 2048*1024));							
			}

      Listbox_Addoneline(plistbox);
      
     // Listbox_Addoneunit(plistbox,ITEM_TYPE_ISCONTROL,pbutton);

			Listbox_Addoneunit(plistbox,ITEM_TYPE_CHAR_STRING,buffer);
			
			index++;
			
			if( index >= 128 )
			{
				  break;
			}

	  }
	
	////添加一些测试数据
//	for(i=0;i<20;i++) {
//		SPrint(buffer,100,L"Item %d", i);
//		Listbox_Addoneline(plistbox);
//		Listbox_Addoneunit(plistbox,ITEM_TYPE_CHAR_STRING,buffer);
//	}
//	return i+index;

   return index;
}

int SelectPartFace(WORD disknum,int *partindex)
{
	  int ret = 0;
    struct BUTTON buttonctrl[2];

    struct BUTTON externbuttonctrl[2];

    struct WINDOW win;

    LISTBOX listbox;

    RECT rc;
    WORD x, y;
	  int index;
	  
re_init:
	    
    CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	
	x = (g_WindowWidth - 560) / 2;
	y = (g_WindowHeight / 3 - 60) / 2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

    plistbox = &listbox;
    Listbox_init(&listbox, g_x_begin + 40, y + 125,736,400,10,0x8000);
    listbox.controlinfo.sendmessage = recoverlist_dispatchmsg;
///*  
    ret = AddPartListLine(&listbox,disknum);
    if(ret==0)
    {
    	
    	 DrawMsgBox(STRING_NO_DISK_AVAIBLE_HINT, STRING_YES, NULL);

    	 Listbox_destroy( &listbox);
    	 return IDCANCEL;
    }  
//*/

    Button_init(&buttonctrl[0], rc.x + rc.width - 500, y + 560,185,60,
    	IMG_FILE_BTN_BACKUP1_FOCUS, IMG_FILE_BTN_BACKUP1_FOCUS, IMG_FILE_BTN_BACKUP1_NORMAL, 0);
    Button_init(&buttonctrl[1], g_x_begin - 80, y - 5, 64, 64,
    	IMAG_FILE_GOBACK_FOCUS, IMAG_FILE_GOBACK_FOCUS, IMG_FILE_GOBACK, 0);

    Button_init((PBUTTON)&externbuttonctrl[0],rc.x+40+rc.width-55, y + 460, 28, 28,IMAG_FILE_BTN_UP_FOCUS,IMAG_FILE_BTN_UP,IMAG_FILE_BTN_UP, 0);
    Button_init((PBUTTON)&externbuttonctrl[1],rc.x+40+rc.width-55, y + 495, 28, 28,IMAG_FILE_BTN_DOWN_FOCUS,IMAG_FILE_BTN_DOWN,IMAG_FILE_BTN_DOWN, 0);
 
 //   Button_init((PBUTTON)&externbuttonctrl[2],rc.x+40,rc.y,60,48,IMAG_FILE_BTN_DOWN_FOCUS,IMAG_FILE_BTN_DOWN,IMAG_FILE_BTN_DOWN, 0);
    
    externbuttonctrl[0].pbindctl = &listbox;
    externbuttonctrl[1].pbindctl = &listbox;
     
    	
    buttonctrl[0].controlinfo.control_id = IDB_SELECT_PART;
    buttonctrl[0].controlinfo.sendmessage=btn_dorecover;
	  buttonctrl[0].clear_background = TRUE;
  
  	buttonctrl[1].controlinfo.control_id = IDCANCEL;
    buttonctrl[1].controlinfo.sendmessage = btn_default;

	  externbuttonctrl[0].controlinfo.control_id = IDB_PAGE_UP;
	  externbuttonctrl[0].controlinfo.sendmessage = extern_control_dispatchmsg;	


	
	externbuttonctrl[1].controlinfo.control_id = IDB_PAGE_DOWN;
	externbuttonctrl[1].controlinfo.sendmessage = extern_control_dispatchmsg;

//  	externbuttonctrl[2].controlinfo.control_id = IDCANCEL;

	//  externbuttonctrl[2].controlinfo.sendmessage = btn_default;	

	

    Window_init(&win,NULL);
    win.msgfunc = SelectPartWindow_Dispatchmsg;

    ClearScreen();

	//注意不要第一个添加cancel按钮，否则cancel会作为默认按钮
    Window_addctl(&win, (PCONTROL)&listbox);

    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//备份按钮
  	Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//ESC按钮

	 Window_addctl(&win, (PCONTROL)&externbuttonctrl[0]); 	//翻页按钮
	 Window_addctl(&win, (PCONTROL)&externbuttonctrl[1]); 	//翻页按钮

    hidemouse();

    //DisplayImg(g_x_begin+80, y+8, IMG_FILE_GOBACK,FALSE);
    //DisplayImg(g_x_begin+160, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE

	DisplayImg(g_x_begin+260+30, y, IMAG_FILE_SYS_BK_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE

	DisplayString(g_x_begin + 40, y + 90, COLOR_SEC_BLUE, STRING_SELEC_PART_TITLE);

    showmouse();

	if(g_data.bDebug)
		MyLogString(NULL);

   // win.control_num =  win.control_num-2;


    ret = Window_run(&win);

    if (ret == IDB_SELECT_PART && listbox.item_num)
    {
    	int ret = 0;
    	
    	index = listbox.firstitem_inwindow + listbox.whichselect;
    	
    	*partindex = g_part_list_info[index].partindex;
    	
    	ret = DetectAvailableAssignPartition(*partindex);
   	  if(ret < 0) 
   		{
			MyLog(LOG_DEBUG, L"DetectAvailableAssignPartition ret =%d\n",ret);
		   	 if(ret == -4)
				    DrawMsgBox(STRING_PART_TOO_SMALL_TO_BACKUP, STRING_YES, NULL);
			   else if(ret == -2)
				 {
				 	   DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1,STRING_FS_INIT_FAILED_2,STRING_FS_INIT_FAILED_3,NULL,STRING_YES, NULL);

				 	//   DrawMsgBox(STRING_FS_INIT_FAILED, STRING_YES, NULL);
			   }
			   else if(ret == -1)
				    DrawMsgBox(STRING_PART_TOO_SMALL_TO_BACKUP, STRING_YES, NULL);
			   else
				    DrawMsgBox(STRING_PART_MIGHT_SMALL, STRING_YES, NULL);
    		 ret = IDCANCEL;
    		 if(ret == IDCANCEL)
    		 {
    		 	   Listbox_destroy( &listbox);
    		 	   goto re_init;
    		 }
	  	} 

    }
    
    Listbox_destroy( &listbox);

	
	if(ret == IDB_SELECT_PART)
	{
		MyLog(LOG_DEBUG, L"Selected part %d to backup.\n", *partindex);
	}
//getch();
//ret = IDCANCEL;	  
	  return ret;
}

int RecoverSetting(int *partindex/*0*/,DWORD *backupfileid/*-1*/)
{
    struct BUTTON buttonctrl[3];
    struct WINDOW win;
    DWORD ret;
    LISTBOX listbox;
    SCROLL_BOX scrollbox;
    RECT rc;
    WORD x, y;
	int index;
    
    CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	g_data.Cmd = USER_COMMAND_RECOVER_SELECT;

   ProbeSystemPartition();

	 if(g_data.bHasFactoryBackup < 0) 
	 {
	  	g_data.bHasFactoryBackup = HasFactoryBackup();
 
	 }
	 
re_init:

    //获取当前系统中检测到的所有备份的镜像文件，存储在全局变量g_headerlist里
 //   GetAllBackupFiles();

	if(g_data.bFactoryRestore == TRUE) {
		//工厂恢复模式
		ret = IDCANCEL;
		for(index = 0; index < g_data.ImageFilesNumber; index++ ){
			if(g_data.images[index].bFactoryBackup) {
				*partindex = (int)g_data.images[index].PartitionIndex;
				*backupfileid = index;	//g_data.images[index].Time;
				ret = IDB_RECOVER;
				break;
			}
		}
		if(ret == IDCANCEL) {

			MyLog(LOG_DEBUG, L"Factory backup not found.\n");
			//没有找到初始备份点，清除工厂恢复模式
			DrawMsgBox(STRING_FACTORY_BACKUP_NOT_FOUND, STRING_YES, NULL);
			g_data.bFactoryRestore = FALSE;
		}
		goto completed;
	}

    plistbox = &listbox;
    Listbox_init(&listbox,rc.x+100,rc.y+100,rc.width-120,rc.height-160,10,0x8000);
    listbox.controlinfo.sendmessage = recoverlist_dispatchmsg;
    
	ret = AddBackupLine(&listbox);
    Scrollbox_init(&scrollbox,(PCONTROL)&listbox,(WORD)ret,10);
    
    listbox.pscroll = &scrollbox;  //绑定滚动条

	y = (g_WindowHeight / 3 - 60) /2;
    Button_init(&buttonctrl[0], rc.x+rc.width-210,rc.y+rc.height,200,64,
    	IMG_FILE_BTN_RESTORE1_FOCUS, IMG_FILE_BTN_RESTORE1_FOCUS, IMG_FILE_BTN_RESTORE1_NORMAL, 0);
	Button_init(&buttonctrl[1], rc.x+rc.width-410,rc.y+rc.height,200,64,
		IMG_FILE_BTN_DELETE_FOCUS, IMG_FILE_BTN_DELETE_FOCUS, IMG_FILE_BTN_DELETE_NORMAL, 0);
    Button_init(&buttonctrl[2], g_x_begin+80, y+8-8, 64, 64,
    	IMG_FILE_GOBACK, IMG_FILE_GOBACK, IMG_FILE_GOBACK, 0);
    //Button_init(&buttonctrl[2], rc.x + rc.width - 32, rc.y+15, 25, 23,
    //	IMG_FILE_CANCELA ,IMG_FILE_CANCELA,IMG_FILE_CANCELA, NULL );
    	
//  	Titlebox_init(&titlebox, &listbox,300+14, columns,3); //标题框初始化
    buttonctrl[0].controlinfo.control_id = IDB_RECOVER;
    buttonctrl[0].controlinfo.sendmessage=btn_dorecover;

	
	buttonctrl[0].clear_background = TRUE;
	buttonctrl[1].controlinfo.control_id = IDB_DELETE;
	buttonctrl[1].controlinfo.sendmessage=btn_dorecover;
	buttonctrl[1].controlinfo.status |= CON_STATUS_DISABLE;
	buttonctrl[1].btn_disablepcx = IMG_FILE_BTN_DELETE_DISABLE;
	buttonctrl[1].clear_background = TRUE;
	buttonctrl[2].controlinfo.control_id = IDCANCEL;
	buttonctrl[2].controlinfo.sendmessage = btn_default;
	g_btnDelete = &buttonctrl[1];


    Window_init(&win,NULL);
    win.msgfunc = Recover_MyWindow_Dispatchmsg;

    ClearScreen();

	//注意不要第一个添加cancel按钮，否则cancel会作为默认按钮
    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//恢复按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//delete按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//goback按钮
    Window_addctl(&win, (PCONTROL)&listbox);
    Window_addctl(&win, (PCONTROL)&scrollbox);

    hidemouse();

    y = (g_WindowHeight / 3 - 60) /2;
    //DisplayImg(g_x_begin+80, y+8, IMG_FILE_GOBACK,FALSE);
    //DisplayImg(g_x_begin+160, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE
	x = (g_WindowWidth - 560) / 2;
	DisplayImg(g_x_begin+160+40, y, IMAG_FILE_SYS_R_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE

    showmouse();

	if(g_data.bDebug)
		MyLogString(NULL);

    ret = Window_run(&win);

    if (ret == IDB_RECOVER && listbox.item_num) {
    	index = listbox.firstitem_inwindow + listbox.whichselect;
    	*partindex = (int)g_data.images[index].PartitionIndex;
        *backupfileid = index;	//g_data.images[index].Time;
    }
    
    Listbox_destroy( &listbox);

	if(ret == IDB_DELETE) {
		goto re_init;
	}

completed:

	if(ret == IDB_RECOVER){
		MyLog(LOG_DEBUG, L"Selected %d(%s) to restore.\n", index, g_data.images[index].FileName);
	}
//getch();
//ret = IDCANCEL;
	return ret;
}

int RecoverSettingFace(int *partindex/*0*/,DWORD *backupfileid/*-1*/)
{
    struct BUTTON buttonctrl[3] = { 0 };
    struct BUTTON externbuttonctrl[2] = { 0 };

    struct WINDOW win;
    DWORD ret;
    LISTBOX listbox;
 //   SCROLL_BOX scrollbox;
    RECT rc;
    WORD x, y;
	  int index;
    int img_err_count = -1,img_check_num = -1;
    
    CHAR16 space[256] = { 0 };
    
    CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	g_data.Cmd = USER_COMMAND_RECOVER_SELECT;

   ProbeSystemPartition();

	 if(g_data.bHasFactoryBackup < 0) 
	 {
	  	g_data.bHasFactoryBackup = HasFactoryBackup();
 
	 }
	 
re_init:
	
	 img_err_count = 0;
	 img_check_num = 0;
	 
	  mymemset(space,0,sizeof(space));
	  
    //获取当前系统中检测到的所有备份的镜像文件，存储在全局变量g_headerlist里
    GetAllBackupFiles(&img_err_count);

    img_check_num =  CheckAllBackupFiles();
    
 		//MyLog(LOG_DEBUG, L"img_check_num %d %d to restore.\n", img_check_num,img_err_count);
    
	if(g_data.bFactoryRestore == TRUE) {
		//工厂恢复模式
		ret = IDCANCEL;
		for(index = 0; index < g_data.ImageFilesNumber; index++ ){
			if(g_data.images[index].bFactoryBackup) {
				*partindex = (int)g_data.images[index].PartitionIndex;
				*backupfileid = index;	//g_data.images[index].Time;
				ret = IDB_RECOVER;
				break;
			}
		}
		if(ret == IDCANCEL) {

			MyLog(LOG_DEBUG, L"Factory backup not found.\n");
			//没有找到初始备份点，清除工厂恢复模式
			DrawMsgBox(STRING_FACTORY_BACKUP_NOT_FOUND, STRING_YES, NULL);
			g_data.bFactoryRestore = FALSE;
		}
		goto completed;
	}

	y = (g_WindowHeight / 3 - 60) / 2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	plistbox = &listbox;
	Listbox_init(&listbox, g_x_begin + 40, y + 125, 736, 380, 10, 0x8000);
    listbox.controlinfo.sendmessage = recoverlist_dispatchmsg;
    
	  ret = AddBackupLine(&listbox);
  //  Scrollbox_init(&scrollbox,(PCONTROL)&listbox,(WORD)ret,10);
    
  //  listbox.pscroll = &scrollbox;  //绑定滚动条

    Button_init(&buttonctrl[0], rc.x+rc.width-360, y + 560, 185, 60,
    	IMG_FILE_BTN_RESTORE1_FOCUS, IMG_FILE_BTN_RESTORE1_FOCUS, IMG_FILE_BTN_RESTORE1_NORMAL, 0);
	Button_init(&buttonctrl[1], rc.x+rc.width-620, y + 560, 185, 60,
		IMG_FILE_BTN_DELETE_FOCUS, IMG_FILE_BTN_DELETE_FOCUS, IMG_FILE_BTN_DELETE_NORMAL, 0);
    Button_init(&buttonctrl[2], g_x_begin - 80, y - 5, 64, 64,
    	IMAG_FILE_GOBACK_FOCUS, IMAG_FILE_GOBACK_FOCUS, IMG_FILE_GOBACK, 0);
 
    Button_init((PBUTTON)&externbuttonctrl[0], rc.x + 40 + rc.width - 55, y + 440, 28, 28,IMAG_FILE_BTN_UP_FOCUS,IMAG_FILE_BTN_UP,IMAG_FILE_BTN_UP, 0);
    Button_init((PBUTTON)&externbuttonctrl[1], rc.x + 40 + rc.width - 55, y + 475, 28, 28,IMAG_FILE_BTN_DOWN_FOCUS,IMAG_FILE_BTN_DOWN,IMAG_FILE_BTN_DOWN, 0);
    
    externbuttonctrl[0].pbindctl = &listbox;
    externbuttonctrl[1].pbindctl = &listbox;
    
    
    //Button_init(&buttonctrl[2], rc.x + rc.width - 32, rc.y+15, 25, 23,
    //	IMG_FILE_CANCELA ,IMG_FILE_CANCELA,IMG_FILE_CANCELA, NULL );
    	
//  	Titlebox_init(&titlebox, &listbox,300+14, columns,3); //标题框初始化
    buttonctrl[0].controlinfo.control_id = IDB_RECOVER;
    buttonctrl[0].controlinfo.sendmessage=btn_dorecover;
	  if(ret==0)
	  {
	  	buttonctrl[0].controlinfo.status |= CON_STATUS_DISABLE;
	  }
	  buttonctrl[0].btn_disablepcx = IMAG_FILE_BTN_RESTORE_DISABLE;
	  
	buttonctrl[0].clear_background = TRUE;
	buttonctrl[1].controlinfo.control_id = IDB_DELETE;
	buttonctrl[1].controlinfo.sendmessage=btn_dorecover;
	
	if(g_data.images[0].bFactoryBackup||ret==0)
	{
			buttonctrl[1].controlinfo.status |= CON_STATUS_DISABLE;
	}	
	
	buttonctrl[1].btn_disablepcx = IMG_FILE_BTN_DELETE_DISABLE;
	buttonctrl[1].clear_background = TRUE;
	buttonctrl[2].controlinfo.control_id = IDCANCEL;
	buttonctrl[2].controlinfo.sendmessage = btn_default;
	g_btnDelete = &buttonctrl[1];
  g_btnRestore = &buttonctrl[0];

	 externbuttonctrl[0].controlinfo.control_id = IDB_PAGE_UP;
	 externbuttonctrl[0].controlinfo.sendmessage = extern_control_dispatchmsg;	
	
	 externbuttonctrl[1].controlinfo.control_id = IDB_PAGE_DOWN;
	 externbuttonctrl[1].controlinfo.sendmessage = extern_control_dispatchmsg;
	
    Window_init(&win,NULL);
    win.msgfunc = Recover_MyWindow_Dispatchmsg;

    ClearScreen();

	//注意不要第一个添加cancel按钮，否则cancel会作为默认按钮
    Window_addctl(&win, (PCONTROL)&listbox);

    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//恢复按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//delete按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//goback按钮
 //   Window_addctl(&win, (PCONTROL)&scrollbox);

	Window_addctl(&win, (PCONTROL)&externbuttonctrl[0]); 	//翻页按钮
	Window_addctl(&win, (PCONTROL)&externbuttonctrl[1]); 	//翻页按钮
	
    hidemouse();

    //DisplayImg(g_x_begin+80, y+8, IMG_FILE_GOBACK,FALSE);
    //DisplayImg(g_x_begin+160, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE
	DisplayImg(g_x_begin+260+30, y, IMAG_FILE_SYS_R_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE


	DisplayString(g_x_begin + 40, y + 90, COLOR_SEC_BLUE, STRING_RECOVER_LIST_TITILE);
	//DisplayImg(g_x_begin + 40, y + 100, IMAG_FILE_SYS_R_TIP_SELECT_BACKUP_POINT, FALSE);	//IMG_FILE_RESTORE_TITLE

	if ((img_err_count+img_check_num) > 0)
	{
		SPrint(space,256,L"%s %d %s",STRING_BK_NUM_DESC1,(img_err_count+img_check_num),STRING_BK_NUM_DESC2,DivU64x32(g_part_list_info[index].TotalSectors, 2048*1024));							  
		DisplayString(g_x_begin + 295, y + 513, COLOR_SEC_BLUE, space);
		//DisplayImg(g_x_begin + 310, y + 460, IMAG_FILE_SYS_R_TIP_CHECK_BACKUP_POINT, FALSE);	//IMG_FILE_RESTORE_TITLE
		//SPrint(space, 256, L"%d", (img_err_count + img_check_num));
		//DisplayString(rc.x + rc.width - 448, rc.y + rc.height - 69, COLOR_BLACK, space);
	}

   	// DisplayImg(30,30,IMAG_FILE_GOBACK_FOCUS,FALSE);
   	// DisplayImg(40,40,IMG_FILE_GOBACK,FALSE);
	 

    showmouse();

	if(g_data.bDebug)
		MyLogString(NULL);

    ret = Window_run(&win);

    if (ret == IDB_RECOVER && listbox.item_num) {
    	index = listbox.firstitem_inwindow + listbox.whichselect;
    	*partindex = (int)g_data.images[index].PartitionIndex;
        *backupfileid = index;	//g_data.images[index].Time;
    }
    
    Listbox_destroy( &listbox);

	if(ret == IDB_DELETE) {
		goto re_init;
	}
	
  if (ret == IDB_RECOVER && listbox.item_num)	
	{
		  if( DrawMsgBox(STRING_R_SYS_HINT,STRING_YES,STRING_NO) == IDOK )
		  {
		  }
		  else
		  {
		  	 goto re_init;
		  }	
	}

completed:

	if(ret == IDB_RECOVER)
	{
		 ret = IDB_RECOVER;
		 
		MyLog(LOG_DEBUG, L"Selected %d(%s) to restore.\n", index, g_data.images[index].FileName);
	}
//getch();
//ret = IDCANCEL;
	return ret;
}




// this function is the recover end ,and show the end UI ,only reboot button on it 
void RecoverCompleteUI(int func)
{
    struct WINDOW win;
    struct BUTTON buttonctrl;
    DWORD ret;
    RECT rc;
    WORD x,y;
    
	MyLog(LOG_DEBUG, L"--==== --RecoverComplete  ui \n");


	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));

	x = (g_WindowWidth - 560) / 2;
	y = (g_WindowHeight / 3 - 60) / 2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	Button_init(&buttonctrl, rc.x + (rc.width - 230) / 2 + 20 + 5, y + 560, 185, 60,
		IMAG_FILE_BK_COMPLETE_FOCUS, IMAG_FILE_BK_COMPLETE_FOCUS, IMAG_FILE_BK_COMPLETE_FOCUS, 0/*IMG_FILE_COMPLETE*/);

	buttonctrl.controlinfo.control_id = IDB_COMPLETE;
	buttonctrl.controlinfo.sendmessage = btn_default;
	Window_init(&win, NULL);

    ClearScreen();

    hidemouse();
    
    //DrawLine(rc.x+20,rc.y+70,rc.width-20,1,0xf13326);
    //DrawLine(rc.x+20,rc.y+71,rc.width-20,1,0xF6F7F9);
//	DisplayImg(x, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE
	//if (func == USER_COMMAND_RECOVER)
 //   	DisplayImg(rc.x+(rc.width-220)/2, y, IMG_FILE_RESTORE_COMPLETE,FALSE);
 //   else
 //   	DisplayImg(rc.x+(rc.width-220)/2, y, IMG_FILE_BACKUP_COMPLETE,FALSE);

//    DisplayString(rc.x+300, rc.y+210,COLOR_TEXT,STRING_COMPLETE);

	if (func == USER_COMMAND_RECOVER)
	{
		DisplayImg(rc.x + (rc.width - 230) / 2, y, IMAG_FILE_SYS_R_TITLE, FALSE);	//IMG_FILE_RESTORE_TITLE

																					//      DisplayString(rc.x+(rc.width-230)/2, rc.y+210,COLOR_TEXT,STRING_R_COMPLETE);
		DisplayImg(rc.x + (rc.width - 230) / 2 - 210, y + 330, IMAG_FILE_R_OK_DESC, FALSE);
	}
	else
	{
		DisplayImg(rc.x + (rc.width - 230) / 2, y, IMAG_FILE_SYS_BK_TITLE, FALSE);	//IMG_FILE_RESTORE_TITLE

																					// DisplayString(rc.x+(rc.width-230)/2, rc.y+210,COLOR_TEXT,STRING_BK_COMPLETE);
		DisplayImg(rc.x + (rc.width - 230) / 2 - 210, y + 330, IMAG_FILE_BK_OK_DESC, FALSE);

	}
    showmouse();


    Window_addctl(&win, (PCONTROL)&buttonctrl);
	 

    ret = Window_run(&win);
    
    if(ret == IDB_COMPLETE) {
    	gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
}


int CompareHddSN(CHAR8* sn1, CHAR8* sn2)
{
	UINT16 i =0 ;
	 
	for( i=0;i< 20; i++)
	{
		if(sn1[i] != sn2[i])
			return -1;
	}
	
	return 1;
}

int checkIfSnZero(CHAR8* sn)
{
	UINT16 i =0 ;
	 
	for( i=0;i< 20; i++)
	{
		if(sn[i] != 0)
			return -1;
	}
	
	return 1;
}
 


OKR_HEADER * ReadImageFileHeader_Max(int partindex ,DWORD fileindex)
{
    OKR_HEADER *header = NULL;
    CHAR16 fullpath[128] = { 0 };
	CHAR16 *filename =NULL;
    UINT64 filesize = 0;
    int ret=-1;
    BOOL bRet = 0, bPartitionChanged = 0, bShowError = TRUE;
    BOOL bPartModify = FALSE;

    DWORD i = 0;
    struct CONTINUE_RWDISK_INFO conti_info;
    PBUFFER_INFO buffinfo;
    struct BUFFERARRAY buffarray;
     
    FILE_HANDLE_INFO filehandle;
    FILE_SYS_INFO  sysinfo;
    VOID *bitbuffer = NULL;
    UINT64 after_alignlen = 0;
    UINT64 curlen = 0;
    
    g_data.Cmd = USER_COMMAND_RECOVER;
	g_data.bCompleted = FALSE;
    g_data.ImageDiskNumber = g_Partitions[partindex].DiskNumber;
	filename = (CHAR16 *)g_data.images[fileindex].FileName;

   

	header = myalloc(OKR_HEADER_MAX_SIZE);
	if (!header) {
		SET_ERROR(__LINE__, OKR_ERROR_ALLOC_MEM);
		goto Done;
	}

	if (FileSystemInit(&sysinfo,partindex,FALSE) == FALSE) {
		if (FileSystemInit(&sysinfo,partindex,TRUE) == FALSE) {
			SET_ERROR(__LINE__, OKR_ERROR_FILE_SYSTEM);
			goto  Done;
		}
	} else {
	 
	}

  	MyLog(LOG_DEBUG,L"Recover index:%d,filenum %d\n",fileindex,g_data.images[fileindex].FileNumber);

    SPrint(fullpath, 128, BACKUP_DIR L"\\%s", filename);
    //针对多个fat32备份文件拷贝NTFS分区
    if( g_data.images[fileindex].FileNumber > 1 && sysinfo.ifntfs )
    {
    	  if (!SegFileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE)) 
    	  {
		        SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
            goto  Done;
        }    	
    }
    else
    {
    	  if (!FileOpen(&sysinfo, &filehandle, fullpath, EFI_FILE_MODE_READ, 0, TRUE)) 
    	  {
		        SET_ERROR(__LINE__, OKR_ERROR_OPEN_FILE);
            goto  Done;
        }
    }

   // curlen += filesize;
    filesize = OKR_HEADER_MAX_SIZE;
    bRet = FileRead(&filehandle, (UINTN*)&filesize ,(PBYTE)header);
    if (!bRet || filesize != OKR_HEADER_MAX_SIZE) {
		SET_ERROR(__LINE__, OKR_ERROR_READ_FILE);
        goto  Done;
    }

    if (header->Magic != OKR_IMAGE_MAGIC || header->bComplete != TRUE) {
		SET_ERROR(__LINE__, OKR_ERROR_INVALID_IMAGE_FILE);
        goto  Done;
    }
    Done :
	FileClose(&filehandle);
	FileSystemClose(&sysinfo);
	return header;
}

#define PARTC_SAME 0
#define PARTC_BIGGER_THAN_IMAGE 1
#define PARTC_SMALL_THAN_IMAGE 2
#define PARTC_NOT_FOUND 3


int  CheckSysDiskPartationCSize(int imagedisk,int imageindex,int destdisknum,int *g_partc_index , int *head_partc_index)
{
	DWORD i, j;
	ULONGLONG datasize = 0;
	int backupdisk = 0;
	int destdisk = -1;
	int candidate = -1;
	int partC_index = -1;
	
	OKR_HEADER * head =NULL;
	
	head = ReadImageFileHeader_Max(imagedisk,imageindex);
	
	if(head == NULL)
	{
		if(head)
		myfree(head);
		return PARTC_NOT_FOUND;
	}
	
	for (i=0;i<head->PartitionCount;i++) {
		if (head->Partition[i].HasWindows && head->Partition[i].bIfBackup) {
			//backupdisk = head->Partition[i].DiskNumber;
			partC_index = i; //第一个有windows的就是系统盘，因为只备份系统分区及之前的小于1G的分区
			break;
		}
	}
	if(partC_index == -1)
	{
		if(head)
		myfree(head);
		return PARTC_NOT_FOUND;
	}
	
	*head_partc_index = partC_index;
	MyLog(LOG_DEBUG, L"head %x.\n", *(DWORD*)head);
	g_data.PartCBootBegin = 0;
	g_data.PartCTotalSector = 0;
	//计算分区总大小
    for (i=0;i< 1;i++) {
    	MyLog(LOG_DEBUG, L"backupdisk =%d par[%d].Boot=0x%llx\n", destdisknum,partC_index,head->Partition[partC_index].BootBegin);
		//遍历所有分区，检测到和备份时的分区信息完全一致时，可以根据它是否备份分区来判断它为backupdisk或datadisk。
       	for (j=0;j<g_dwTotalPartitions;j++) {
			MyLog(LOG_DEBUG, L"[%d].DiskNumber=%d .BootBegin=0x%llx\n",j,g_Partitions[j].DiskNumber,g_Partitions[j].BootBegin);	
       	    if (destdisknum == g_Partitions[j].DiskNumber && g_Partitions[j].BootBegin == head->Partition[partC_index].BootBegin)
			{
				 *g_partc_index = j;
				  g_data.PartCBootBegin = g_Partitions[j].BootBegin; 
				  g_data.PartCTotalSector = g_Partitions[j].TotalSectors; 
				 if ( g_Partitions[j].TotalSectors == head->Partition[partC_index].TotalSectors)
				 {
					  MyLog(LOG_DEBUG, L"equ disk sec =0x%llx .img  sec =0x%llx\n",g_Partitions[j].TotalSectors,head->Partition[partC_index].TotalSectors);	
					 if(head)
						myfree(head);
					 return PARTC_SAME;
				 }
				 else if( g_Partitions[j].TotalSectors > head->Partition[partC_index].TotalSectors)
				 {
					  MyLog(LOG_DEBUG, L"bigger disk sec =0x%llx .img  sec =0x%llx\n",g_Partitions[j].TotalSectors,head->Partition[partC_index].TotalSectors);	
					 if(head)
						myfree(head);
				     return PARTC_BIGGER_THAN_IMAGE;
				 }
				 else if( g_Partitions[j].TotalSectors < head->Partition[partC_index].TotalSectors)
				 {
					 MyLog(LOG_DEBUG, L"c sec=0x%llx .img totalsec =0x%llx\n",g_Partitions[j].TotalSectors,head->Partition[partC_index].TotalSectors);	
					 if(head)
						myfree(head);
				     return PARTC_SMALL_THAN_IMAGE;
				 }
				 

		
			}
 
        }
 
    }
   
    if(head)
		myfree(head);
	 MyLog(LOG_DEBUG, L"Partitions c not found\n");
    return PARTC_NOT_FOUND;
}


int  CheckNewDiskPartationCSize(int imagedisk,int imageindex)
{
	DWORD i, j;
	ULONGLONG datasize = 0;
	int backupdisk = 0;
	int destdisk = -1;
	int candidate = -1;
	int partC_index = -1;
	
	OKR_HEADER * head =NULL;
	
	head = ReadImageFileHeader_Max(imagedisk,imageindex);
	
	if(head == NULL)
	{
		if(head)
		myfree(head);
		return PARTC_NOT_FOUND;
	}
	
	for (i=0;i<head->PartitionCount;i++) {
		if (head->Partition[i].HasWindows && head->Partition[i].bIfBackup) {
			//backupdisk = head->Partition[i].DiskNumber;
			partC_index = i; //第一个有windows的就是系统盘，因为只备份系统分区及之前的小于1G的分区
			break;
		}
	}
	if(partC_index == -1)
	{
		if(head)
		myfree(head);
		return PARTC_NOT_FOUND;
	}
	
 
	MyLog(LOG_DEBUG, L"head %x.\n", *(DWORD*)head);

 	g_data.PartCBootBegin = head->Partition[partC_index].BootBegin; 
	g_data.PartCTotalSector = head->Partition[partC_index].TotalSectors; 
   
    if(head)
		myfree(head);
	 MyLog(LOG_DEBUG, L"New HDD Partitions c   found\n");
    return PARTC_NOT_FOUND;
}

int sysdisktype_in_backupfile = 0 ; // 0 none ; 1 nvme ; 2 sata;
int FindNewHdd(WORD recoverfilepartindex,DWORD backupfileID)
{
	int found = -1;
	CHAR8 *TempBuffer = NULL ;
	CHAR8 diskcount = 0 ;
	CHAR8 *temphddsn[MAX_SUPPORED_HDD_NO]  ;
	CHAR8 *tmphddsn;
	OKR_HEADER *Header = NULL ;
	UINT16 i = 0 , j =0 ;
	int newhdd = 0 ;
	
	Header =&(g_data.images[backupfileID]);
	
	g_data.CurImageSyspartitionsize = 0;
	
	if(Header->Version != OKR_VERSION)
		return -1;
	
	TempBuffer = (CHAR8 *)Header->Memo + 100 ;
	
	if(TempBuffer[0] != 'D' || TempBuffer[1] != 'S' || TempBuffer[2] != 'N')
		return -1;
 
	
	
	
	diskcount = TempBuffer[3];
	
	g_data.CurImageSyspartitionsize =*(UINT64*)&TempBuffer[4];
	MyLog(LOG_DEBUG,L"CurImageSyspartitionsize:0x%llx\n",g_data.CurImageSyspartitionsize);
	
	if(g_data.CurImageSyspartitionsize == 0)
		return -1;
	
	TempBuffer= TempBuffer + 12;
	
	for(j=0; j< MAX_SUPPORED_HDD_NO; j++)
	{
		temphddsn[j] = 0;
	}
	
	for(j=0; j< diskcount; j++)
	{
		temphddsn[j] = TempBuffer;
		TempBuffer = TempBuffer + 20;
	} 
	tmphddsn = temphddsn[0]; 
	if(tmphddsn[0] == 0x20 || tmphddsn[0] == 0)
		sysdisktype_in_backupfile = 2 ; // sata
	else
		sysdisktype_in_backupfile = 1 ; //nvme
	
	for(j=0; j< MAX_DISK_NUM; j++)
	{
		found = -1;
		//if(g_disklist[j].removable == TRUE)
		//	continue ;
		MyLog(LOG_ERROR, L"FindNewHdd-Sysdisk[%d].sn=%02x%02x\n",j,g_disklist[j].disksn[0],g_disklist[j].disksn[19]);   
		//if(checkIfSnZero(g_disklist[j].disksn) == 1)
		//	continue ;
		if( g_disklist[j].totalsector == 0)
		{
			MyLog(LOG_ERROR, L"zero size ,continue;\n");
				continue;
		}
		
		if(checkIfSnZero(g_disklist[j].disksn) == 1)
		{
			MyLog(LOG_ERROR, L"disksn zero  ,continue;\n");
				continue;
		}
		
		
		for(i = 0; i< diskcount ; i++)
		{
			
			
			
			if(CompareHddSN(g_disklist[j].disksn , temphddsn[i]) == 1)
			{
				if(i == 0)
					g_disklist[j].curdisktype = DISK_SYSDISK;
				else
					g_disklist[j].curdisktype = DISK_DATA;
				found = 1;
				
				MyLog(LOG_ERROR, L"Match-imagedisk[%d].sn=%02x%02x\n",i,g_disklist[j].disksn[0],g_disklist[j].disksn[19]);   
			}
		}
		
		if(found != 1 )
		{
			MyLog(LOG_ERROR, L"FindNewHdd-NewDisk");   
			g_disklist[j].curdisktype = NEW_FOUND_DISK;
			newhdd ++;
		}
			
	}
	
	
	
	if(newhdd > 0)
		return 1;
	else
		return -1;
	
 
	
}

#define EXTRA_SIZE 0x2800000  // 20G 扇区

int DetectNewHddFace(WORD recoverfilepartindex,DWORD backupfileID) //BackupSetting
{
    struct BUTTON buttonctrl[3];
    struct WINDOW win;
    DWORD ret = IDCANCEL;
    //EDITBOX editbox;
	int selecteddiskindex =-1;
    UINT64 data;
    CHAR16 space[80] = { 0 };
    CHAR16 capspace[80] = { 0 };
    WORD i, x, y;
    RECT rc;
    int result = 0;
    int img_err_count = 0;
    
	 
	
    struct BUTTON externbuttonctrl[2] = { 0 };

	MyLog(LOG_ERROR, L"-----wh--------BackupSetting...\n");   


	
	result = FindNewHdd(recoverfilepartindex,backupfileID) ;
    if ( result != 1)
        return IDB_RECOVER;
	else
		if(DrawMsgBox(STRING_FIND_NEW_DISK_CONFIRM, STRING_YES, STRING_NO_1) == IDNO) 
			return IDB_RECOVER;	

re_init:

	g_data.Cmd = USER_COMMAND_RECOVER_NEWHDD_SELECT;
    
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
///* 
    
	

 
 

  //kang--  GetAllBackupFiles(&img_err_count);

  //kang--  CheckAllBackupFiles();
       
	//kang--if(g_data.bHasFactoryBackup < 0) {
	//kang--	g_data.bHasFactoryBackup = HasFactoryBackup();
	//kang--}

//*/
	//kang--SPrint(space,80,L"%s", STRING_BACKUP_SIZE);

    //show message box
	g_data.TotalBackupSize = GetBackupTotaldata(); //20 * 1024 * 1024;// GetBackupTotaldata();
	data = DivU64x32(g_data.TotalBackupSize,1024*1024);
    if (data > 1024)
        SPrint(capspace,80,L"%d.%d GB", data/1024, (data%1024)/10);
    else
        SPrint(capspace,80,L"%d MB", data);
	y = (g_WindowHeight / 3 - 60) /2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	//Button_init(&buttonctrl[0], rc.x+rc.width-500, y + 560, 185,60,
    //	IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT, 0);
    	Button_init(&buttonctrl[0], rc.x+rc.width-500, y + 623, 185,60,
    	IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT_FOCUS, IMAG_FILE_NEXT, 0);
	Button_init(&buttonctrl[1], g_x_begin-80, y - 5, 64,64,
		IMAG_FILE_GOBACK_FOCUS, IMAG_FILE_GOBACK_FOCUS, IMG_FILE_GOBACK, 0);


 Button_init((PBUTTON)&externbuttonctrl[0],rc.x+120+rc.width-135,y+376, 28, 28,IMAG_FILE_BTN_UP_FOCUS,IMAG_FILE_BTN_UP,IMAG_FILE_BTN_UP, 0);
   Button_init((PBUTTON)&externbuttonctrl[1],rc.x+120+rc.width-135,y+411, 28, 28,IMAG_FILE_BTN_DOWN_FOCUS,IMAG_FILE_BTN_DOWN,IMAG_FILE_BTN_DOWN, 0);
 //   Button_init((PBUTTON)&externbuttonctrl[0],rc.x+120+rc.width-135,y+313, 28, 28,IMAG_FILE_BTN_UP_FOCUS,IMAG_FILE_BTN_UP,IMAG_FILE_BTN_UP, 0);
  //  Button_init((PBUTTON)&externbuttonctrl[1],rc.x+120+rc.width-135,y+348, 28, 28,IMAG_FILE_BTN_DOWN_FOCUS,IMAG_FILE_BTN_DOWN,IMAG_FILE_BTN_DOWN, 0);
    
	//Editbox_init(&editbox, rc.x + 160, y + 495, 601, desc, EDIT_MODE_NORMAL, 30);
	//g_editbox = &editbox;
        


    buttonctrl[0].controlinfo.control_id = IDB_BACKUP;
    buttonctrl[0].controlinfo.sendmessage = btn_dobackup;
	buttonctrl[0].clear_background = TRUE;
    buttonctrl[1].controlinfo.control_id = IDCANCEL;
    buttonctrl[1].controlinfo.sendmessage = btn_default;
    //buttonctrl[2].controlinfo.control_id = IDCANCEL;
    //buttonctrl[2].controlinfo.sendmessage = btn_default;
  
  
    externbuttonctrl[0].controlinfo.control_id = IDB_PAGE_UP;
	  externbuttonctrl[0].controlinfo.sendmessage = sel_disk_extern_control_dispatchmsg;	
	
	  externbuttonctrl[1].controlinfo.control_id = IDB_PAGE_DOWN;
	  externbuttonctrl[1].controlinfo.sendmessage = sel_disk_extern_control_dispatchmsg;

  
    Window_init(&win,NULL);
    
    
  //  win.msgfunc=MyWindow_Dispatchmsg;
    win.msgfunc=SelectDiskMyWindow_Dispatchmsg;


    ClearScreen();

  //  g_DriveList = Recover_NewDriveButtons(&win, g_x_begin + 40, y + 125, 736, 252, g_data.BackupDiskNumber);
  g_DriveList = Recover_NewDriveButtons(&win, g_x_begin + 40, y + 125, 736, 315, g_data.BackupDiskNumber);

   //kang-- Window_addctl(&win, (PCONTROL)&editbox); 	//EDIT按钮 交换tab顺序 
    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
    
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//取消按钮
   
   // g_DriveList = NewDriveButtons(&win, g_x_begin+336+30, g_y_begin+90+25, 450, 320+48, g_data.BackupDiskNumber);

    externbuttonctrl[0].pbindctl = g_DriveList;
    externbuttonctrl[1].pbindctl = g_DriveList;
	 
	  Window_addctl(&win, (PCONTROL)&externbuttonctrl[0]); 	//翻页按钮
	  Window_addctl(&win, (PCONTROL)&externbuttonctrl[1]); 	//翻页按钮


   // win.current = g_DriveList->num;

    hidemouse();

 

    DisplayString(g_x_begin + 40, y + 90, COLOR_SEC_BLUE,STRING_FORCE_RESTORE_HDD);
   // DisplayString(g_x_begin + 40, y + 450, COLOR_SEC_BLUE,space);
  //  DisplayString(g_x_begin + 40 + 180, y + 450, COLOR_SEC_BLUE,capspace);
    //kang-- DisplayString(g_x_begin + 40, y + 500, COLOR_SEC_BLUE,STRING_INPUTNAME);
 
    //kang-- DisplayString(g_x_begin + 40, y + 387, COLOR_RED,STRING_REMOVE_DES);
 
  
  
    //DisplayImg(g_x_begin+90, y+10, IMG_FILE_GOBACK,FALSE);
    DisplayImg(g_x_begin+260+30, y, IMAG_FILE_SYS_R_TITLE,FALSE);	//IMG_FILE_BACKUP_TITLE
	//x = (g_WindowWidth - 560) / 2;
	//DisplayImg(x, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE

	//DisplayImg(g_x_begin + 130, y + 90, IMAG_FILE_TIP_BACKUP_SELECT_STORE_DEVICE, FALSE);
	//DisplayImg(g_x_begin + 130, y + 400, IMAG_FILE_TIP_BACKUP_HOT_PNP, FALSE);
	//DisplayImg(g_x_begin + 130, y + 440, IMAG_FILE_TIP_BACKUP_DATA_SIZE, FALSE);
	//DisplayImg(g_x_begin + 130, y + 480, IMAG_FILE_TIP_BACKUP_MEMO, FALSE);

    showmouse();

	if(g_data.bDebug)
		MyLogString(NULL);

    ret = Window_run(&win);

	g_data.PartCBootBegin = 0;
	g_data.PartCTotalSector = 0;
	
    if (ret == IDB_BACKUP) 
    {
    	//选择的是磁盘，而不是分区，要自动计算哪个分区最合适

		int mydestdisk;
		int checkstatus;

		g_data.needlimit = 0;
		
		if(g_DriveList->selected != -1) 
			mydestdisk = g_DriveList->partindex[g_DriveList->selected];
 
		
		if(mydestdisk < 0 || mydestdisk >= MAX_DISK_NUM)
		{
			DrawMsgBox(STRING_DISK_NOT_SELECTED_SMALL, STRING_YES, NULL);
			DeleteDriveButtons(g_DriveList);
			goto re_init ;
		}
		

		if(mydestdisk == g_Partitions[recoverfilepartindex].DiskNumber  )
		{
			DrawMsgBox(STRING_DISK_SAME_SELECTED_DISK, STRING_YES, NULL);
			DeleteDriveButtons(g_DriveList);
			goto re_init ;
		}
		
			
		checkstatus = g_disklist[mydestdisk].curdisktype;
   	   
		if(checkstatus == DISK_DATA)
		{
			
			DrawMsgBox(STRING_NOT_SUPPORT_DATA_DISK_TYPE, STRING_YES, NULL);
			DeleteDriveButtons(g_DriveList);
			goto re_init ;
			
		}
		else if(checkstatus == DISK_SYSDISK)
		{
			int image_partc_index =0;
			int gpart_index =0;
			int retvalue = 0;
			
			retvalue = CheckSysDiskPartationCSize(recoverfilepartindex,backupfileID,mydestdisk,&gpart_index,&image_partc_index);
			
			if(retvalue == PARTC_NOT_FOUND) 
			{	
			
			DrawMsgBox(STRING_DISK_ORG_SYSDISK_NOT_FOUND, STRING_YES, NULL);
			DeleteDriveButtons(g_DriveList);
			goto re_init ;
			}
			
			if(retvalue == PARTC_SMALL_THAN_IMAGE) 
			{	
				UINT64	 NewAlignNum1 = MultU64x32(g_data.CurImageSyspartitionsize, BLOCK_SIZE );
				MyLog(LOG_DEBUG,L"sys disk CurImageSyspin sec:0x%llx\n",NewAlignNum1);
				if(g_Partitions[gpart_index].TotalSectors < (NewAlignNum1+EXTRA_SIZE))
				{
					DrawMsgBox(STRING_DISK_ORG_SYSDISK_TOO_SMALL, STRING_YES, NULL);
					DeleteDriveButtons(g_DriveList);
					goto re_init ;
				}
				g_data.needlimit = 1;
			}
			
		 
		}
		else if(checkstatus == NEW_FOUND_DISK)
		{
			CHAR16 stringbuff[256] = {0};
			UINT64	 NewAlignNum = MultU64x32(g_data.CurImageSyspartitionsize, BLOCK_SIZE );
			UINT64	 size_c = MultU64x32(NewAlignNum+1, 512 );
			UINT64	 size_newhdd = MultU64x32(g_disklist[mydestdisk].totalsector, 512 );

	    OKR_HEADER * head =NULL;
	
	    head = ReadImageFileHeader_Max(recoverfilepartindex,backupfileID);
      
			MyLog(LOG_DEBUG,L"new hdd CurImageSyspartitionsize sec :0x%llx\n",NewAlignNum);
			size_c  = DivU64x32(size_c , 1024);
			size_c  = DivU64x32(size_c , 1024);
			size_c  = DivU64x32(size_c , 1024);
			
			size_newhdd  = DivU64x32(size_newhdd , 1024);
			size_newhdd  = DivU64x32(size_newhdd , 1024);
			size_newhdd  = DivU64x32(size_newhdd , 1024);

      UINT64 NeedSize = 0, MyNeedSize = 0;
      
      DWORD j;
    


    for (j=0;j<head->PartitionCount;j++){
        if (!head->Partition[j].bIfBackup){
            continue;
        }
        NeedSize += head->Partition[j].TotalSectors;
        
    }
        MyNeedSize = DivU64x32(NeedSize, 2*1024*1024);
     



    
			CheckNewDiskPartationCSize(recoverfilepartindex,backupfileID);
			
			
			
			
			
			if(TRUE == g_disklist[mydestdisk].removable) 
			{
				if(DrawMsgBox(STRING_DONOT_SELECT_REMOVEABLE_WARNING, STRING_YES, STRING_NO) == IDCANCEL) //可移动设备提示
				{
					DeleteDriveButtons(g_DriveList);
			 
					goto re_init ;
				}
				
				
			}


      if( NeedSize > g_disklist[mydestdisk].totalsector){

        
      	SPrint(stringbuff, 256, STRING_VERSION, size_newhdd,MyNeedSize,MyNeedSize);
      DrawMsgBox(stringbuff, STRING_YES, NULL);
      DeleteDriveButtons(g_DriveList);
      goto re_init ;
		
      }      
      

      
      if( ( NewAlignNum+ EXTRA_SIZE ) > g_disklist[mydestdisk].totalsector)
      {
				
				mydestdisk = -1;
 
				SPrint(stringbuff, 256, STRING_NEW_DISK_SIZE_SMALL, size_c,size_newhdd,size_c);
				DrawMsgBox(stringbuff, STRING_YES, NULL);
				DeleteDriveButtons(g_DriveList);
				goto re_init ;
			}
			
			
			
		
		}
		else
		{
			DrawMsgBox(STRING_DISK_UNKNOWN_TYPE, STRING_YES, NULL);
			DeleteDriveButtons(g_DriveList);
			goto re_init ;
		
		}
			
			 
			
		 
		
		if(DrawMsgBox(STRING_RECOVER_NEW_DISK_WARNING, STRING_YES, STRING_NO) == IDCANCEL) //免责提示
		{
			DeleteDriveButtons(g_DriveList);
			 
			goto re_init ;
		}
	
		
		if(g_DriveList->selected != -1) 
			g_data.selecteddisk  = g_DriveList->partindex[g_DriveList->selected];
		
		g_data.destdisktype  = checkstatus; 
		g_data.comefromnewhddface = 1;
		

#if OKR_DEBUG
//	MyLog(LOG_ERROR,L"auto-selected partition %d.\n", selectedpartindex);
	//MyLog(LOG_ERROR,L"press any key to continue...\n");
	//getch();
#endif
    }
    DeleteDriveButtons(g_DriveList);
 // getch();
//  ret = IDCANCEL;
	return ret;
}


int DoRecover(WORD recoverfilepartindex,DWORD backupfileID)
{
    PROCESSBOX progress;
    int nRet;
    BYTE oldstate;
    RECT rc;
    WORD x,y;
 
	
	MyLog(LOG_ERROR, L"---wh---temp-------DoRecover...\n");
 	g_data.selecteddisk = -1;
	g_data.comefromnewhddface =0;
	nRet = DetectNewHddFace(recoverfilepartindex, backupfileID);  
    if(IDCANCEL == nRet)
	{
		g_data.selecteddisk = -1;
		g_data.comefromnewhddface = 0;
		return -1;
	}
	
	if(IDB_RECOVER == nRet)
	{
		g_data.selecteddisk = -1;
		g_data.comefromnewhddface = 0;
		
	}
	
				
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
    
    hidemouse();
    oldstate = EnableMouse(FALSE);
    
    ClearScreen();

	x = (g_WindowWidth - 560) / 2;
	y = (g_WindowHeight / 3 - 60) / 2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	//DrawLine(rc.x+20,rc.y+70,rc.width-20,1,0xf13326);
    //DrawLine(rc.x+20,rc.y+71,rc.width-20,1,0xF6F7F9);

	DisplayImg(g_x_begin + 260 + 30, y, IMAG_FILE_SYS_R_TITLE,FALSE);

	DisplayString(rc.x + 270, y + 350, COLOR_SEC_BLUE, STRING_USED_TIME);
	DisplayString(rc.x + 270, y + 380, COLOR_SEC_BLUE, STRING_REMAIN_TIME);

    //DisplayImg(rc.x+20,rc.y+20,IMG_FILE_TITLERE,FALSE);
    //DisplayString(rc.x+300, rc.y+300,COLOR_BLACK,STRING_CANCEL);
	//DisplayImg(x, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE
	DisplayImg(g_x_begin + 220, y + 180, IMG_FILE_RESTORING, FALSE);

    DisplayImg((g_WindowWidth - 128) / 2 - 35 - 225, y + 520,IMG_FILE_FOOT_RESTORE,FALSE);

    Processbox_init(&progress,rc.x+100, y + 430,rc.width-200);
    Processbox_show((PCONTROL)&progress);
    
	
	if(g_data.CurImageSyspartitionsize == 0)
    nRet = Recover_Linux(recoverfilepartindex,backupfileID,(PCONTROL)&progress);
    else
	nRet = Recover(recoverfilepartindex,backupfileID,(PCONTROL)&progress);
	//nRet = 0;
	//delayEx(9000000);
	//delayEx(9000000);

    EnableMouse(oldstate);

 
    showmouse();
    
    if(nRet < 0) {
        return nRet;
    }
    
    return 0;
}

int DoBackup(BOOL if_init_backup,WORD Destpart,CHAR16 *desc)
{
    PROCESSBOX progress;
    int nRet;
    BYTE oldstate;
    RECT rc;
    WORD x, y;
    
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	MyLog(LOG_ERROR, L"---wh---temp-------DoBackup...\n");
    hidemouse();
    oldstate = EnableMouse(FALSE);
    
    ClearScreen();

	x = (g_WindowWidth - 560) / 2;
	y = (g_WindowHeight / 3 - 60) /2;
	if (g_WindowHeight <= 800)
	{
		y -= 80;
	}

	//DisplayImg(x, y, IMG_FILE_BACKGROUND_TITLE,FALSE);	//IMG_FILE_RESTORE_TITLE
	DisplayImg(g_x_begin + 260 + 30, y, IMAG_FILE_SYS_BK_TITLE, FALSE);	//IMG_FILE_BACKUP_TITLE
	DisplayImg(g_x_begin + 220, y + 180, IMG_FILE_BACKUPING,FALSE);
	DisplayImg((g_WindowWidth - 128)/2-35-170, y + 560,IMG_FILE_FOOT_BACKUP,FALSE);

	DisplayString(rc.x+270, y + 350, COLOR_SEC_BLUE,STRING_USED_TIME);
	DisplayString(rc.x+270, y + 380, COLOR_SEC_BLUE,STRING_REMAIN_TIME);

    Processbox_init(&progress,rc.x+100, y + 430,rc.width-200);
    Processbox_show((PCONTROL)&progress);
    
    nRet = Backup(if_init_backup,Destpart,desc,(PCONTROL)&progress);
    //  nRet = testBackupProcess(Destpart,desc,(PCONTROL)&progress);
    
    EnableMouse(oldstate);

   // ClearScreen();

  	showmouse();
	
	MyLog(LOG_DEBUG, L"DoBackup nRet:%d.\n", nRet);
    
    if(nRet < 0) {
        return nRet;
    }

    return 0;
}

void ShowWaitMsgOnce(void)
{
	if(checkntfslog == 1)
		return ;
	DrawRect(0XFEFEFE, g_WindowWidth/2 -300, g_WindowHeight/2 - 60 , 600, 120); //draw the BKG of the DLG ysy备份前分析分区空间 框
	PrintMessageInRect(g_WindowWidth/2 -300 +30,g_WindowHeight/2 - 60 +60-15, 200, 30, STRING_WAIT_NTFS_MSG, COLOR_SEC_BLUE);//yuanban
	checkntfslog = 1;
}

void Face()
{
    int func=0; //0,backup,
    DWORD ret;
	CHAR16 backup_desc[100] = {0};
    int backupfile_partindex=-1;
    int disknum=-1;
    int recoverfile_partindex=-1;
    DWORD backupfileid=0;
    BOOLEAN bComplete = FALSE;
    int result = 0; 
//Complete(COMMAND_BACKUP);


    
    while (!bComplete)
    {
		g_data.bFactoryRestore = FALSE;	//复位bFactoryRestore标志

	    //画背景图
    	ClearScreen();

        ret = StartFace(); //第一个提示框
		MyLog(LOG_DEBUG, L"User select command(%d) to continue.\n", ret);
#if OKR_DEBUG //add by wh

		MyLog(LOG_MESSAGE, L" add by wh func=%d .", func);

#endif

        if (ret == IDB_BACKUP)
            func = USER_COMMAND_BACKUP;
        else if (ret == IDB_RECOVER)
            func = USER_COMMAND_RECOVER;
		    else if (ret == IDB_RECOVER_FACTORY)
			      func = USER_COMMAND_RECOVER;
		    else if(ret==IDB_DRIVER_INSTALL)
		    {
		    	
		    	  func = USER_COMMAND_DRIVER_INSTALL;
		    	// MyTestReadWriteFileOrDir(1);
         //  DriverInstallFace();
           
		    //	 DriverInstall();

		    //	 MyTestReadWriteFileOrDir(3);
    	    //	getch();
			      // break;

		    }
		    else if(ret==IDB_EXIT || ret==0)
		    {
		    	 	if(DrawMsgBox(STRING_REBOOT,STRING_YES, STRING_NO) == IDOK)
		    	 	{
		    	 		break;
		    	 	}
		    	 	continue; 
		    }
		    else
		    {
		    	  break;
		    }


        //在StartFace里选择要执行的功能后，下面进一步选择参数：
		backup_desc[0] = 0;
        while (!bComplete)
        {
            
#if OKR_DEBUG_UI //add by wh

	

#endif


			
			
            if (func == USER_COMMAND_BACKUP)
			{
				int destpart = -1;
				int partindex = -1;
				
				ShowWaitMsgOnce();
				 
					
				destpart = DetectAvailableOEMPartition(&partindex);
				if (destpart < 0)
				{
					goto nextbkface;
				}

            	 //首先判断是否有初始备份点
            	  result = InitialBackupFace(&backupfile_partindex);  	 
            	  if(result == -2)
            	  {            	  	
            	  }
            	  else if(result == IDB_IDB_INIT_BACKUP)
            	  {
            	  	
            	  //	CreateTestTmpFile(backupfile_partindex);
            	  	
            	  	if (DoBackup(TRUE,(WORD)backupfile_partindex,L"Initbackup") < 0)
					        {
					        //	 DeleteTestTmpFile(backupfile_partindex);
					        	 continue;
					        }  
					        
					     //   getch(); 
					        
					      //  DeleteTestTmpFile(backupfile_partindex);
					        
				          bComplete = TRUE;
            	  	
            	  	break;
            	  }
                else 
            	  {
            	  	break;
            	  }       	  		
nextbkface:                //备份镜像的存储分区，备份镜像的说明信息
                if (BackupSetting(backup_desc, &disknum) != IDB_BACKUP)
                    break;
                 result = SelectPartFace(disknum,&backupfile_partindex);
                 if(result==IDCANCEL)
                 {
					 goto nextbkface;
                 }
                 else if(result!=IDB_SELECT_PART)
                 {
                 	  break;
                 }
				if (DoBackup(FALSE,(WORD)backupfile_partindex,backup_desc) < 0)
				{
						continue;
				}		
				bComplete = TRUE;
            } else if (func == USER_COMMAND_RECOVER) {
				
				ShowWaitMsgOnce();
                //选择镜像文件
                if (RecoverSettingFace(&recoverfile_partindex,&backupfileid) != IDB_RECOVER)
                    break;
				if (DoRecover((WORD)recoverfile_partindex,backupfileid) < 0) {
					if(g_data.bFactoryRestore)
						break;
					continue;
				}
				bComplete = TRUE;
            }
            else if (func == USER_COMMAND_DRIVER_INSTALL)
            {
//           	   	UINT32 file_partindex = 0;
/////*
//            	  if( DriverInstallFace() != IDB_DRIVER_INSTALL)
//            	  	break;
//            	
//            	  //判断驱动文件是否存在
//            	//  if(IsExistFileInAllPartAssignPath(OKR_TOOL_PATH,FILE_NAME,&file_partindex) == FALSE)
//            	  {
//            	 // 	DriverInstallFailFace();
//            	 //  	break;
//            	  }
//            	  result = installExeToSystemDir();
//#if OKR_DEBUG_UI //add by wh
//
//
//#endif
//            	  if(result==ERR_INSTALL_ALREADY_HAVE_DRIVER)
//            	  {
//            	  	 DrawMsgBox(STRING_DRV_EXIST_HINT,STRING_YES, NULL);
//            	  	 break;
//            	  }
//            	  else if(result==ERR_INSTALL_MOUNT_SYSFS_RW)
//            	  {
//            	  	 //DrawMsgBox(STRING_MOUNT_SYS_ERROR,STRING_YES, NULL);
//     	  	 		DrawMsgBoxMultilLine(STRING_MOUNT_SYS_ERROR_1,STRING_MOUNT_SYS_ERROR_2,STRING_MOUNT_SYS_ERROR_3,NULL,STRING_YES, NULL);
//
//            	  	 break;
//            	  }
//                if( result < 0 )
//            	  {
//            	  	DriverInstallFailFace();
//            	   	break;
//            	  } 
//            	          	   
//           	   // MyTestReadWriteFileOrDir(2);
//          	  //   getch();
//           	  //  MyTestReadWriteFileOrDir(3);
//           	    
//           	  //  getch();
////*/
//            	  bComplete = TRUE;	

            }
        }
  //      if(func == USER_COMMAND_DRIVER_INSTALL && bComplete)
  //      {
  //      	DriverInstallComplete(func);
		//}
		if (bComplete && func != USER_COMMAND_DRIVER_INSTALL)
		{
			RecoverCompleteUI(func);
		}
        	
    }

	ReleaseUnusedDataList();


return;
}
