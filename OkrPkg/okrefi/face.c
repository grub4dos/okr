/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#include "okr.h"
#include "okrefiStr.h"
#include "faceUIDefine.h"
#include "faceFunc.h"
#include "vga.h"

// DrawMsgBox DrawMsgBoxMultilLine 


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
//EDITBOX *g_editbox = NULL;

extern time_t  g_lastESCTime;
extern time_t  g_lastCountDownTime;
extern WORD g_dCountDown;

BOOL g_isUpdateEmptyRecoveryPointList = FALSE;  
BOOL g_isUpdateEmptyPartList = FALSE;

PBYTE g_ShowOnceWndBuff = NULL;

BOOL g_bIsGetBackupFileInfo = TRUE;

BOOL g_bNeedUpdateBackupFileList = TRUE;
 
WORD  g_nLincenceBoxPage = 1; // current select index  = add by wh
WORD  g_nLincenceboxPicX = 1; // current select page position  = add by wh
WORD  g_nLincenceboxPicY = 1; // current select  page position   = add by wh
//WORD  g_nLincenceboxPreBtnX = 1;   //Lincence PreBtn position    = add by wh
//WORD  g_nLincenceboxPreBtnY = 1;   //Lincence PreBtn position    = add by wh
//WORD  g_nLincenceboxNextBtnX = 1;  //Lincence NextBtn position   = add by wh
//WORD  g_nLincenceboxNextBtnY = 1;  //Lincence NextBtn position   = add by wh
//struct DrvButtonList *g_DriveList = NULL;
struct BUTTON *g_btnDelete = NULL;
struct BUTTON *g_btnRestore = NULL;

//TAB_ENUM g_enuSelectTab = -1;

PART_LIST_INFO g_part_list_info[128];
DISK_LIST_INFO g_disk_list_info[MAX_DISK_NUM];

BUTTON_INFO g_button_info_list[] = 
{
	{IDOK,			IMG_FILE_BUTTON_CONFIRM_PRESS,		IMG_FILE_BUTTON_CONFIRM_FOCUS,		IMAG_FILE_CONFIRM_UNFOCUS,			0, 1, STRING_YES},
	{IDCANCEL,		IMG_FILE_BUTTON_CANCEL_PRESS,		IMG_FILE_BUTTON_CANCEL_FOCUS,		IMG_FILE_BUTTON_CANCEL_UNFOCUS,		0, 1, STRING_NO},
	{IDOKSUB,		IMG_FILE_BUTTON_OK_SUB_PRESS,		IMG_FILE_BUTTON_OK_SUB_FOCUS,		IMG_FILE_BUTTON_OK_SUB_UNFOCUS,		0, 1, STRING_YES},
	{IDCANCELMAIN,	IMG_FILE_BUTTON_CANCEL_MAIN_PRESS,	IMG_FILE_BUTTON_CANCEL_MAIN_FOCUS,	IMG_FILE_BUTTON_CANCEL_MAIN_UNFOCUS,0, 1, STRING_NO},
	{IDB_REBOOT,	IMG_FILE_MSG_REBOOT_PRESS,			IMG_FILE_MSG_REBOOT_FOCUS,			IMG_FILE_MSG_REBOOT_UNFOCUS,		0, 1, STRING_REBOOT_BTN},
	{IDB_SHUTDOWN,	IMG_FILE_MSG_SHUTDOWN_PRESS,		IMG_FILE_MSG_SHUTDOWN_FOCUS,		IMG_FILE_MSG_SHUTDOWN_UNFOCUS,		0, 1, STRING_CLOSE_BTN},
	{IDB_MSG_CLOSE,IMG_FILE_MSG_CLOSE_PRESS,			IMG_FILE_MSG_CLOSE_FOCUS,			IMG_FILE_MSG_CLOSE_UNFOCUS,			0, 0, L""},
	{IDB_MSG_BTN_FORCE_RECOVERY,	IMG_BUTTON_FORCE_RECOVERY_FOCUS, IMG_BUTTON_FORCE_RECOVERY_FOCUS, IMG_BUTTON_FORCE_RECOVERY_UNFOCUS, 0, 1, STRING_FORCE_RECOVERY},
	{IDB_MSG_BTN_INTRODUCE,	0, 0, 0, 0, 0, STRING_ABOUT_INTRODUCE},
	{IDB_MSG_BTN_OPEN_SOURCE,	0, 0, 0, 0, 0, STRING_ABOUT_OPEN_SOURCE},
	{IDB_MSG_BTN_VERSION,	0, 0, 0, 0, 0, STRING_ABOUT_VERSION},
	{IDB_BACKUP,	0, 0, 0, 0, 1, STRING_BTNTEXT_BACKUP},
	{IDB_DELETE,	0, 0, 0, 0, 1, STRING_BTNTEXT_DELETE},
	{IDB_RECOVER,	0, 0, 0, 0, 1, STRING_BTNTEXT_RECOVER},
	{IDB_NEW_HDD_RECOVER,	0, 0, 0, 0, 1, STRING_BTNTEXT_NEXT},
	
	{0,0,0,0,0,0, L""}
};

BUTTON_INFO g_button_info_list_s[] =
{
	{IDOK,			IMG_FILE_BUTTON_CONFIRM_PRESS,		IMG_FILE_BUTTON_CONFIRM_FOCUS,		IMAG_FILE_CONFIRM_UNFOCUS,			0, 1, STRING_YES},
	{IDCANCEL,		IMG_FILE_BUTTON_CANCEL_S_PRESS,		IMG_FILE_BUTTON_CANCEL_S_FOCUS,		IMG_FILE_BUTTON_CANCEL_S_UNFOCUS,		0, 1, STRING_NO},
	{IDOKSUB,		IMG_FILE_BUTTON_OK_SUB_S_PRESS,		IMG_FILE_BUTTON_OK_SUB_S_FOCUS,		IMG_FILE_BUTTON_OK_SUB_S_UNFOCUS,	0, 1, STRING_YES},
	{IDCANCELMAIN,	IMG_FILE_BUTTON_CANCEL_MAIN_S_PRESS,IMG_FILE_BUTTON_CANCEL_MAIN_S_FOCUS,IMG_FILE_BUTTON_CANCEL_MAIN_S_UNFOCUS,0, 1, STRING_NO},
	{IDB_REBOOT,	IMG_FILE_MSG_REBOOT_PRESS,			IMG_FILE_MSG_REBOOT_FOCUS,			IMG_FILE_MSG_REBOOT_UNFOCUS,		0, 1, STRING_REBOOT_BTN},
	{IDB_SHUTDOWN,	IMG_FILE_MSG_SHUTDOWN_S_PRESS,		IMG_FILE_MSG_SHUTDOWN_S_FOCUS,		IMG_FILE_MSG_SHUTDOWN_S_UNFOCUS,		0, 1, STRING_CLOSE_BTN},
	{IDB_MSG_CLOSE,	IMG_FILE_MSG_CLOSE_S_PRESS,			IMG_FILE_MSG_CLOSE_S_FOCUS,			IMG_FILE_MSG_CLOSE_S_UNFOCUS,			0, 0, L""},
	{IDB_MSG_BTN_FORCE_RECOVERY,	IMG_BUTTON_FORCE_RECOVERY_FOCUS, IMG_BUTTON_FORCE_RECOVERY_FOCUS, IMG_BUTTON_FORCE_RECOVERY_UNFOCUS,			0, 1, STRING_FORCE_RECOVERY},
	{IDB_MSG_BTN_INTRODUCE,	0, 0, 0, 0, 0, STRING_ABOUT_INTRODUCE},
	{IDB_MSG_BTN_OPEN_SOURCE,	0, 0, 0, 0, 0, STRING_ABOUT_OPEN_SOURCE},
	{IDB_MSG_BTN_VERSION,	0, 0, 0, 0, 0, STRING_ABOUT_VERSION},
	{IDB_BACKUP,	0, 0, 0, 0, 1, STRING_BTNTEXT_BACKUP},
	{IDB_DELETE,	0, 0, 0, 0, 1, STRING_BTNTEXT_DELETE},
	{IDB_RECOVER,	0, 0, 0, 0, 1, STRING_BTNTEXT_RECOVER},
	{IDB_NEW_HDD_RECOVER,	0, 0, 0, 0, 1, STRING_BTNTEXT_NEXT},
	{IDB_CONTINUE,	0, 0, 0, 0, 1, STRING_BTNTEXT_CONTINUE},
	{IDB_RUN_RESTORE,	0, 0, 0, 0, 1, STRING_BTNTEXT_RUN_RESTORE},
	{IDB_AUTO_CONTINUE,	0, 0, 0, 0, 1, STRING_AUTOBR_BTNTEXT_CONTINUE},
	{IDB_AUTO_REBOOT,	0, 0, 0, 0, 1, STRING_AUTOBR_NO},
	{0,0,0,0,0,0, L""}
};

BUTTON_INFO* FindBtnInfoFromID(DWORD btnid)
{
	INTN index;
	index = 0;
	if (0.5 >= g_data.fMinscale)
	{
		while (g_button_info_list_s[index].btn_id != -1)
		{
			if (g_button_info_list_s[index].btn_id == btnid)
			{
				//MyLog(LOG_DEBUG, L"FindBtnInfoFromID g_button_info_list_s btnid: %d, index %d\n", btnid, index);
				return &g_button_info_list_s[index];
			}
			index++;
		}
	}
	else
	{
		while (g_button_info_list[index].btn_id != -1)
		{
			if (g_button_info_list[index].btn_id == btnid)
			{
				//MyLog(LOG_DEBUG, L"FindBtnInfoFromID btnid: %d, index %d\n", btnid, index);
				return &g_button_info_list[index];
			}
			index++;
		}
	}

	return NULL;
}


IMAGE_FILE_ID g_Img_id_list[] =
{
	////About
	{IMG_FILE_BTN_ABOUT_UNFOCUS, IMG_FILE_BTN_ABOUT_M_UNFOCUS, IMG_FILE_BTN_ABOUT_S_UNFOCUS, IMG_FILE_BTN_ABOUT_S_UNFOCUS},
	{IMG_FILE_BTN_ABOUT_FOCUS, IMG_FILE_BTN_ABOUT_M_FOCUS, IMG_FILE_BTN_ABOUT_S_FOCUS, IMG_FILE_BTN_ABOUT_S_FOCUS},
	{IMG_FILE_BTN_ABOUT_PRESS, IMG_FILE_BTN_ABOUT_M_PRESS, IMG_FILE_BTN_ABOUT_S_PRESS,IMG_FILE_BTN_ABOUT_S_PRESS},

	//Msg type
	{IMG_FILE_ENQUIRE, IMG_FILE_ENQUIRE_M, IMG_FILE_ENQUIRE_S,IMG_FILE_ENQUIRE_S},
	{IMG_FILE_INFO, IMG_FILE_INFO_M, IMG_FILE_INFO_S,IMG_FILE_INFO_S},
	{IMG_FILE_WARNING, IMG_FILE_WARNING_M, IMG_FILE_WARNING_S,IMG_FILE_WARNING_S},

	//EXIT btn
	{IMG_FILE_BTN_EXIT_UNFOCUS, IMG_FILE_BTN_EXIT_M_UNFOCUS, IMG_FILE_BTN_EXIT_S_UNFOCUS, IMG_FILE_BTN_EXIT_S_UNFOCUS},
	{IMG_FILE_BTN_EXIT_FOCUS, IMG_FILE_BTN_EXIT_M_FOCUS, IMG_FILE_BTN_EXIT_S_FOCUS, IMG_FILE_BTN_EXIT_S_FOCUS},
	{IMG_FILE_BTN_EXIT_PRESS, IMG_FILE_BTN_EXIT_M_PRESS, IMG_FILE_BTN_EXIT_S_PRESS, IMG_FILE_BTN_EXIT_S_PRESS},

	//
	{IMG_FILE_SUCCESS, IMG_FILE_SUCCESS_M, IMG_FILE_SUCCESS_S, IMG_FILE_SUCCESS_S},
	{IMG_FILE_FAILED, IMG_FILE_FAILED_M, IMG_FILE_FAILED_S, IMG_FILE_FAILED_S},

	{IMG_FILE_FOOT, IMG_FILE_FOOT_M, IMG_FILE_FOOT_S, IMG_FILE_FOOT_S},

	//Back btn
	{IMG_FILE_GOBACK, IMG_FILE_BUTTON_GO_BACK_M_UNFOCUS, IMG_FILE_BUTTON_GO_BACK_S_UNFOCUS, IMG_FILE_BUTTON_GO_BACK_S_UNFOCUS},
	{IMAG_FILE_GOBACK_FOCUS, IMG_FILE_BUTTON_GO_BACK_M_FOCUS, IMG_FILE_BUTTON_GO_BACK_S_FOCUS, IMG_FILE_BUTTON_GO_BACK_S_FOCUS},
	{IMG_FILE_BUTTON_GO_BACK_PRESS, IMG_FILE_BUTTON_GO_BACK_M_PRESS, IMG_FILE_BUTTON_GO_BACK_S_PRESS, IMG_FILE_BUTTON_GO_BACK_S_PRESS},

	{IMAG_FILE_INITIAL_BK, IMAG_FILE_INITIAL_BK_M, IMAG_FILE_INITIAL_BK_S, IMAG_FILE_INITIAL_BK_S},

	//WHITE BACKGROUND
	{IMG_FILE_WHITE_BG_LEFT_TOP, IMG_FILE_WHITE_BG_LEFT_TOP_M, IMG_FILE_WHITE_BG_LEFT_TOP_S, IMG_FILE_WHITE_BG_LEFT_TOP_S},
	{IMG_FILE_WHITE_BG_LEFT_BOTTOM, IMG_FILE_WHITE_BG_LEFT_BOTTOM_M, IMG_FILE_WHITE_BG_LEFT_BOTTOM_S, IMG_FILE_WHITE_BG_LEFT_BOTTOM_S},
	{IMG_FILE_WHITE_BG_RIGHT_TOP, IMG_FILE_WHITE_BG_RIGHT_TOP_M, IMG_FILE_WHITE_BG_RIGHT_TOP_S, IMG_FILE_WHITE_BG_RIGHT_TOP_S},
	{IMG_FILE_WHITE_BG_RIGHT_BOTTOM, IMG_FILE_WHITE_BG_RIGHT_BOTTOM_M, IMG_FILE_WHITE_BG_RIGHT_BOTTOM_S, IMG_FILE_WHITE_BG_RIGHT_BOTTOM_S},

	//LOGO
	{IMG_FILE_LOGO, IMG_FILE_LOGO_M, IMG_FILE_LOGO_S, IMG_FILE_LOGO_S},

	//APP LOGO
	{IMAG_FILE_APP_LOGO, IMAG_FILE_APP_LOGO_M, IMAG_FILE_APP_LOGO_S, IMAG_FILE_APP_LOGO_S},

	//X btn
	{IMG_FILE_MSG_CLOSE_UNFOCUS, IMG_FILE_MSG_CLOSE_M_UNFOCUS, IMG_FILE_MSG_CLOSE_S_UNFOCUS, IMG_FILE_MSG_CLOSE_S_UNFOCUS},
	{IMG_FILE_MSG_CLOSE_FOCUS, IMG_FILE_MSG_CLOSE_M_FOCUS, IMG_FILE_MSG_CLOSE_S_FOCUS, IMG_FILE_MSG_CLOSE_S_FOCUS},
	{IMG_FILE_MSG_CLOSE_PRESS, IMG_FILE_MSG_CLOSE_M_PRESS, IMG_FILE_MSG_CLOSE_S_PRESS, IMG_FILE_MSG_CLOSE_S_PRESS},

	//PROGRESS
	{IMG_FILE_PROGRESS_HEAD, IMG_FILE_PROGRESS_HEAD_M, IMG_FILE_PROGRESS_HEAD_S, IMG_FILE_PROGRESS_HEAD_S},
	{IMG_FILE_PROGRESS_TAIL, IMG_FILE_PROGRESS_TAIL_M, IMG_FILE_PROGRESS_TAIL_S, IMG_FILE_PROGRESS_TAIL_S},
	{IMG_FILE_PROGRESS_FRAME_HEAD, IMG_FILE_PROGRESS_FRAME_HEAD_M, IMG_FILE_PROGRESS_FRAME_HEAD_S, IMG_FILE_PROGRESS_FRAME_HEAD_S},
	{IMG_FILE_PROGRESS_FRAME_TAIL, IMG_FILE_PROGRESS_FRAME_TAIL_M, IMG_FILE_PROGRESS_FRAME_TAIL_S, IMG_FILE_PROGRESS_FRAME_TAIL_S},

	//TITLE
	{IMG_FILE_BACKGROUND_TITLE, IMG_FILE_BACKGROUND_TITLE_M, IMG_FILE_BACKGROUND_TITLE_S, IMG_FILE_BACKGROUND_TITLE_S },
	{IMAG_FILE_SYS_BK_TITLE, IMAG_FILE_SYS_BK_TITLE_M, IMAG_FILE_SYS_BK_TITLE_S, IMAG_FILE_SYS_BK_TITLE_S},
	{IMAG_FILE_SYS_R_TITLE, IMAG_FILE_SYS_R_TITLE_M, IMAG_FILE_SYS_R_TITLE_S, IMAG_FILE_SYS_R_TITLE_S},

	//ICON
	{IMG_FILE_BACKUP_POINT, IMG_FILE_BACKUP_POINT, IMG_FILE_BACKUP_POINT_S, IMG_FILE_BACKUP_POINT_S },
	{IMG_FILE_BACKUP_POINT_SEL, IMG_FILE_BACKUP_POINT_SEL, IMG_FILE_BACKUP_POINT_SEL_S, IMG_FILE_BACKUP_POINT_SEL_S},

	{IMG_FILE_HARDDISK, IMG_FILE_HARDDISK, IMG_FILE_HARDDISK_S, IMG_FILE_HARDDISK_S},
	{IMG_FILE_HARDDISK_SEL, IMG_FILE_HARDDISK_SEL, IMG_FILE_HARDDISK_SEL_S, IMG_FILE_HARDDISK_SEL_S },

	{IMG_FILE_LOCK, IMG_FILE_LOCK, IMG_FILE_LOCK_S, IMG_FILE_LOCK_S},
	{IMG_FILE_LOCK_SEL, IMG_FILE_LOCK_SEL, IMG_FILE_LOCK_SEL_S, IMG_FILE_LOCK_SEL_S},

	{IMG_FILE_NEW_DISK, IMG_FILE_NEW_DISK, IMG_FILE_NEW_DISK_S, IMG_FILE_NEW_DISK_S },
	{IMG_FILE_NEW_DISK_SEL, IMG_FILE_NEW_DISK_SEL, IMG_FILE_NEW_DISK_SEL_S, IMG_FILE_NEW_DISK_SEL_S},

	{IMG_FILE_ORI_DISK, IMG_FILE_ORI_DISK, IMG_FILE_ORI_DISK_S, IMG_FILE_ORI_DISK_S },
	{IMG_FILE_ORI_DISK_SEL, IMG_FILE_ORI_DISK_SEL, IMG_FILE_ORI_DISK_SEL_S, IMG_FILE_ORI_DISK_SEL_S},

	{IMG_FILE_REMOVEDISK, IMG_FILE_REMOVEDISK, IMG_FILE_REMOVEDISK_S, IMG_FILE_REMOVEDISK_S },
	{IMG_FILE_REMOVEDISK_SEL, IMG_FILE_REMOVEDISK_SEL, IMG_FILE_REMOVEDISK_SEL_S, IMG_FILE_REMOVEDISK_SEL_S},

	///BTN 
	{IMG_FILE_BTNSPL_L, IMG_FILE_BTNSPL_L_M, IMG_FILE_BTNSPL_L_S, IMG_FILE_BTNSPL_L_S },
	{IMG_FILE_BTNSPL_M, IMG_FILE_BTNSPL_M_M, IMG_FILE_BTNSPL_M_S, IMG_FILE_BTNSPL_M_S},
	{IMG_FILE_BTNSPL_R, IMG_FILE_BTNSPL_R_M, IMG_FILE_BTNSPL_R_S, IMG_FILE_BTNSPL_R_S},

	{IMG_FILE_BTNSPL_F_L, IMG_FILE_BTNSPL_F_L_M, IMG_FILE_BTNSPL_F_L_S, IMG_FILE_BTNSPL_F_L_S },
	{IMG_FILE_BTNSPL_F_M, IMG_FILE_BTNSPL_F_M_M, IMG_FILE_BTNSPL_F_M_S, IMG_FILE_BTNSPL_F_M_S},
	{IMG_FILE_BTNSPL_F_R, IMG_FILE_BTNSPL_F_R_M, IMG_FILE_BTNSPL_F_R_S, IMG_FILE_BTNSPL_F_R_S},

	{IMG_FILE_BTNSPL_P_L, IMG_FILE_BTNSPL_P_L_M, IMG_FILE_BTNSPL_P_L_S, IMG_FILE_BTNSPL_P_L_S },
	{IMG_FILE_BTNSPL_P_M, IMG_FILE_BTNSPL_P_M_M, IMG_FILE_BTNSPL_P_M_S, IMG_FILE_BTNSPL_P_M_S},
	{IMG_FILE_BTNSPL_P_R, IMG_FILE_BTNSPL_P_R_M, IMG_FILE_BTNSPL_P_R_S, IMG_FILE_BTNSPL_P_R_S},

	{IMG_FILE_BTNSPL_D_L, IMG_FILE_BTNSPL_D_L_M, IMG_FILE_BTNSPL_D_L_S, IMG_FILE_BTNSPL_D_L_S },
	{IMG_FILE_BTNSPL_D_M, IMG_FILE_BTNSPL_D_M_M, IMG_FILE_BTNSPL_D_M_S, IMG_FILE_BTNSPL_D_M_S},
	{IMG_FILE_BTNSPL_D_R, IMG_FILE_BTNSPL_D_R_M, IMG_FILE_BTNSPL_D_R_S, IMG_FILE_BTNSPL_D_R_S},


	///HOME BTN
	{IMG_FILE_HOMEBTN_LT, IMG_FILE_HOMEBTN_LT, IMG_FILE_HOMEBTN_LT_S, IMG_FILE_HOMEBTN_LT_S },
	{IMG_FILE_HOMEBTN_RT, IMG_FILE_HOMEBTN_RT, IMG_FILE_HOMEBTN_RT_S, IMG_FILE_HOMEBTN_RT_S },
	{IMG_FILE_HOMEBTN_LB, IMG_FILE_HOMEBTN_LB, IMG_FILE_HOMEBTN_LB_S, IMG_FILE_HOMEBTN_LB_S },
	{IMG_FILE_HOMEBTN_RB, IMG_FILE_HOMEBTN_RB, IMG_FILE_HOMEBTN_RB_S, IMG_FILE_HOMEBTN_RB_S },

	{IMG_FILE_HOMEBTN_TM, IMG_FILE_HOMEBTN_TM, IMG_FILE_HOMEBTN_TM_S, IMG_FILE_HOMEBTN_TM_S },
	{IMG_FILE_HOMEBTN_BM, IMG_FILE_HOMEBTN_BM, IMG_FILE_HOMEBTN_BM_S, IMG_FILE_HOMEBTN_BM_S },
	{IMG_FILE_HOMEBTN_LM, IMG_FILE_HOMEBTN_LM, IMG_FILE_HOMEBTN_LM_S, IMG_FILE_HOMEBTN_LM_S },
	{IMG_FILE_HOMEBTN_RM, IMG_FILE_HOMEBTN_RM, IMG_FILE_HOMEBTN_RM_S, IMG_FILE_HOMEBTN_RM_S },

	{ IMG_FILE_HOMEBTN_ICON_BACKUP, IMG_FILE_HOMEBTN_ICON_BACKUP, IMG_FILE_HOMEBTN_ICON_BACKUP_S, IMG_FILE_HOMEBTN_ICON_BACKUP_S },
	{ IMG_FILE_HOMEBTN_ICON_RESTORE, IMG_FILE_HOMEBTN_ICON_RESTORE, IMG_FILE_HOMEBTN_ICON_RESTORE_S, IMG_FILE_HOMEBTN_ICON_RESTORE_S },

	{ IMG_FILE_FIND_BITLOCKER_KEY, IMG_FILE_FIND_BITLOCKER_KEY_M, IMG_FILE_FIND_BITLOCKER_KEY_S, IMG_FILE_FIND_BITLOCKER_KEY_S },

	{0,0,0,0}
};

WORD GetImageFileId(WORD wFileIdL)
{
	INTN index;
	index = 0;

	WORD fileId = wFileIdL;

	while (g_Img_id_list[index].fileIdL != 0)
	{
		if (g_Img_id_list[index].fileIdL == wFileIdL)
		{
			if (g_WindowWidth == 3840 && g_WindowHeight == 2160)//(g_data.fMinscale >= 1)
			{
				fileId = g_Img_id_list[index].fileIdL;
			}
			else if (g_WindowWidth == 2560 && g_WindowHeight == 1440)
			{
				fileId = g_Img_id_list[index].fileIdM;
			}
			else if ((g_WindowWidth == 1920 && g_WindowHeight == 1080) ||
				(g_WindowWidth == 1680 && g_WindowHeight == 1050) ||
				(g_WindowWidth == 1280 && g_WindowHeight == 1024) ||
				(g_WindowWidth == 1400 && g_WindowHeight == 1050) ||
				((0.45 < g_data.fMinscale) && (0.5 >= g_data.fMinscale)))
			{
				fileId = g_Img_id_list[index].fileIdS;
			}
			else if (g_WindowWidth == 1024 && g_WindowHeight == 768)//(0.5 >= g_data.fMinscale)
			{
				fileId = g_Img_id_list[index].fileIdS;
			}
			else
			{
				fileId = g_Img_id_list[index].fileIdL;
			}
		}
		index++;
	}
	return fileId;
}

BACKUP_POINT_ICON g_lock_Img_BackupPoint = { IMG_FILE_LOCK , IMG_FILE_LOCK_SEL };
BACKUP_POINT_ICON g_Img_BackupPoint = { IMG_FILE_BACKUP_POINT , IMG_FILE_BACKUP_POINT_SEL };

int g_checkntfslog = 0;

int g_showAdjustPartText = 0;

int sysdisktype_in_backupfile = 0; // 0 none ; 1 nvme ; 2 sata;

//遍历磁盘，查找Lenovo_Part分区，查找分区中是否包含GSK标志文件。
BOOL IsGSKU()
{
	BOOL bRet = FALSE;

	UINT32 iIndex = 0;

	BOOL bFileSystemInitFailed = FALSE;
	BOOL bHasSupportedPartition = FALSE;
	BOOL bInsufficientSpace = FALSE;
	BYTE Window_Recovery_Par_TypeGuid[16] = { 0xa4, 0xbb, 0x94, 0xde, 0xd1, 0x06, 0x40, 0x4d, 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac };

	WORD diskindex = 0;
	FILE_SYS_INFO  sysinfo;
	ENUM_GSKU_SKIP_TYPE enumGSKUSkipType = NOT_GSKU;

	for (iIndex = 0; iIndex < g_dwTotalPartitions; iIndex++)
	{
		MyLog(LOG_DEBUG, L"Partition index: %d, Label %s\n", iIndex, g_Partitions[iIndex].Label);

		//不支持mbr
		if (StrnCmp(g_Partitions[iIndex].Label, L"LENOVO_PART", 11) == 0 &&
			MyMemCmp(Window_Recovery_Par_TypeGuid, g_Partitions[iIndex].TypeGuid, 16) == 0)
		{
			//检查是否存在GSK标志文件
			bRet = HasGSKUFile(iIndex, &enumGSKUSkipType);
			g_data.enumGSKUSkipType = enumGSKUSkipType;
			break;
		}
	}

	return bRet;
}

BOOL DeleteFactoryBackupfile()
{ 
	 //MyLog(LOG_MESSAGE, L"DeleteFactoryBackupfile,bHasFactoryBackup:%d.",g_data.bHasFactoryBackup);
	 
	if (g_data.bHasFactoryBackup == TRUE)
	{
		int index = 0;
		for (index = 0; index < g_data.ImageFilesNumber; index++)
		{
			if (g_data.images[index].bFactoryBackup)
			{
				FILE_SYS_INFO  sysinfo;
				WORD partindex = (WORD)g_data.images[index].PartitionIndex;
				//BOOL bIsLenovoPart = IsLenovoPart(partindex);

				//需要弹出提示要求确认
				if (DrawMsgBoxEx(STRING_DELETE_FACTORY_CONFIRM, IDOK, IDCANCEL, TYPE_ENQUIRE) == IDOK)
				{
					if (FileSystemInit(&sysinfo, partindex, FALSE) != FALSE)
					{
						CHAR16 name[100] = { 0 };
						DWORD count = g_data.images[index].FileNumber;
						DWORD i;
						BOOL bDeleted = FALSE;
						if (!count)
							count = 1;
						for (i = 0; i < count; i++)
						{
							GetBackupFileFullPath(&sysinfo, partindex, g_data.images[index].FileName, i, name, 100);
							//if (bIsLenovoPart)
							//{
							//	if (i == 0)
							//		SPrint(name, 100, BACKUP_DIR L"\\%s", g_data.images[index].FileName);
							//	else
							//		SPrint(name, 100, BACKUP_DIR L"\\%s.%d", g_data.images[index].FileName, i);
							//}
							//else
							//{
							//	if (i == 0)
							//		SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%s", g_data.images[index].FileName);
							//	else
							//		SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%s.%d", g_data.images[index].FileName, i);
							//}

							bDeleted = FileDelete(&sysinfo, name);
						}
						if (bDeleted)
						{
							//删除备份点以后，就要增加这个文件所在的分区的剩余空间
							g_Partitions[partindex].FreeSectors += g_data.images[index].FileSize / SECTOR_SIZE;
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

//MainFace Initbackupface
void MyWindow_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{

    Window_Dispatchmsg(pwin,dmmessage);
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	if(dmmessage->message == CHAR_F1)
	{
		ShowAboutFace(TRUE);
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

			case 'w':
			case 'W':
			case 'w' - 0x60:
			{
			//	//test
			//	FILE_SYS_INFO  sysinfo;
			//	mymemset(&sysinfo, 0, sizeof(sysinfo));
			//	WORD Destpart = 0;//
			//	for (int i = 0; i < g_dwTotalPartitions; i++)
			//	{
			//		Destpart = i;
			//		if (IsLenovoPart(Destpart) == TRUE)
			//		{
			//			break;
			//		}
			//	}
			//	MyLog(LOG_DEBUG, L"Find dest par %d.\n", Destpart);
			//	FileSystemInit(&sysinfo, Destpart, FALSE);

			//	CHAR16 fullpath[256] = L"\\.okr\\63625B3E";

			//	BOOL bRet = FALSE;
			//	bRet = CalculateFileCheckValue(&sysinfo, fullpath, 10);
			//	if (FALSE == bRet)
			//	{
			//		MyLog(LOG_DEBUG, L"CalculateFileCheckValue Error.\n");
			//	}
			//	else
			//	{
			//		MyLog(LOG_DEBUG, L"CalculateFileCheckValue Success.\n");
			//	}
			//	//getch();
			//	bRet = ValidImageCheckValue(&sysinfo, fullpath, 10);
			//	if (FALSE == bRet)
			//	{
			//		MyLog(LOG_DEBUG, L"ValidImageCheckValue Error.\n");
			//	}
			//	else
			//	{
			//		MyLog(LOG_DEBUG, L"ValidImageCheckValue Success.\n");
			//	}

			//	FileSystemClose(&sysinfo);
			}
				break;
			case 'f':
			case 'F':
			case 'f'-0x60:
				{
					g_data.bForceRestorePartition ^= TRUE;
					MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");

					if (TRUE == g_data.bForceRestorePartition)
					{
						DrawMsgBoxTwoLine(STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_ON_INFO, IDOK, -1, TYPE_INFO);
					}
					else
					{
						DrawMsgBoxEx(STRING_FORCE_RESTORE_MODE_OFF, IDOK, -1, TYPE_INFO);
					}
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
					if(DrawMsgBoxEx(STRING_FACTORY_RESTORE, IDOK, IDCANCEL, TYPE_ENQUIRE) == IDOK) {
						g_data.bFactoryRestore = TRUE;
						g_checkntfslog = 1;
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
					  struct DM_MESSAGE dmessage;
					  dmessage.message = WM_UPDATE_RECOVERY_LIST_AFTER_DELETE;
					  dmessage.thex = plistbox->whichselect;

					  plistbox->controlinfo.sendmessage(plistbox, &dmessage);

					  //Listbox_updateview(plistbox);
					  //Window_exit(pwin, IDB_DELETE);
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
			  PrintAllGraphicMode();
			  MyLog(LOG_DEBUG, L"VgaInit  g_WindowWidth：%d g_WindowHeight %d  modeNumber:%d\r\n", g_WindowWidth, g_WindowHeight, g_VgaModeNumber);
			  MyLog(LOG_DEBUG, L"fMinscale:%lf fFontScale:%lf\r\n", g_data.fMinscale, g_data.fFontScale);
			  break;
		  }

 		case 'z':
		case 'Z':
		case 'z' - 0x60:
		{
			MyLog(LOG_ERROR, L"OKR_DEBUG_TEST_MSGUI  \n");
			DrawMsgBoxTwoLine(STRING_BITLOCKER_NO_SYSTEM, STRING_CLOSE_BITLOCKER, IDB_AUTO_REBOOT, -1, TYPE_INFO);
			DrawMsgBoxTwoLine(STRING_AUTOBR_BITLOCKER_DATA_PAR, STRING_CLOSE_BITLOCKER, IDB_AUTO_REBOOT, IDB_AUTO_CONTINUE, TYPE_INFO);
			DrawMsgBoxExWithQRCode(STRING_AUTOBR_BITLOCKER_FIND_KEY, IDB_CONTINUE, IDCANCEL, TYPE_INFO, IMG_FILE_FIND_BITLOCKER_KEY, 200);
			DrawMsgBoxEx(STRING_AUTOBR_IMG_BITLOCKED, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_AUTOBR_DATAPART_BITLOCKED, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_AUTOBR_DATAPART_NOTENOUGH, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_AUTOBR_DATAPART_NOTEXIST, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_AUTOBR_IMG_NOTFOUND, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_AUTOBR_DISK_NOT_FOUND_ERROR, IDOK, -1, TYPE_INFO);

			DrawMsgBoxTwoLine(STRING_BITLOCKER_DATA_PAR, STRING_CLOSE_BITLOCKER, IDCANCEL, IDB_CONTINUE, TYPE_INFO);
			DrawMsgBoxExWithQRCode(STRING_BITLOCKER_FIND_KEY, IDB_RUN_RESTORE, IDCANCEL, TYPE_INFO, IMG_FILE_FIND_BITLOCKER_KEY, 200);


			DrawMsgBoxTwoLine(STRING_BITLOCKER_SELECT_PAR, STRING_CLOSE_BITLOCKER, IDOK, -1, TYPE_INFO);

			DrawMsgBoxTwoLine(STRING_BITLOCKER_NO_SYSTEM, STRING_CLOSE_BITLOCKER, IDOK, -1, TYPE_INFO);

			DrawMsgBoxTwoLine(STRING_BITLOCKER_NO_DATA_PAR, STRING_CLOSE_BITLOCKER, IDOK, -1, TYPE_INFO);

			DrawMsgBoxTwoLine(STRING_DISK_TOO_SMALL_TO_BACKUP, STRING_NEED_SPACE, IDOK, -1, TYPE_INFO);
			DrawMsgBoxTwoLine(STRING_PART_TOO_SMALL_TO_BACKUP, STRING_NEED_SPACE, IDOK, -1, TYPE_INFO);

			DrawMsgBoxEx(STRING_R_SYS_HINT, IDOK, IDCANCEL, TYPE_WARNING);

			RecoverCompleteUIMsgboxEx(USER_COMMAND_INIT_BACKUP, 1, STRING_CREATE_INIT_BK);

			//DrawMsgBoxMultilLineEx(STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_1, 
			//	STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_2,
			//	STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_3,
			//	STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_4,
			//	IDB_MSG_BTN_FORCE_RECOVERY, IDCANCEL, TYPE_ENQUIRE);

			DrawMsgBoxEx(STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE, IDB_MSG_BTN_FORCE_RECOVERY, IDCANCEL, TYPE_ENQUIRE);

			DrawMsgBoxEx(STRING_RECOVER_NEW_DISK_SUC, IDOK, -1, TYPE_INFO);
			
			DrawMsgBoxEx(STRING_WELCOME, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_FACTORY_RESTORE, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_FACTORY_BACKUP_NOT_FOUND, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_FS_INIT_FAILED_1, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DRIVE_NO_SUPPORT_FS, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_PART_MIGHT_SMALL, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DRIVE_MIGHT_SMALL, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_PARTITION_CHANGED, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_HINTPARTTABLE, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_HINTNOSYSPART, IDOK, IDCANCEL, TYPE_INFO);

			RecoverCompleteUIMsgboxEx(USER_COMMAND_BACKUP, 0, STRING_ERROR_WRITE_FILE);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_ERROR_READ_FILE);
			RecoverCompleteUIMsgbox(USER_COMMAND_BACKUP);

			DrawMsgBoxEx(STRING_BACKUP_USER_BREAK, IDCANCELMAIN, IDOKSUB, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_USER_BREAK, IDCANCELMAIN, IDOKSUB, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_DELETE_CONFIRM, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_DELETE_FACTORY_CONFIRM, IDOK, IDCANCEL, TYPE_ENQUIRE);
			//STRING_VERSION
			DrawMsgBoxEx(STRING_REBOOT, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_NTFS_DELET_ERROR_1, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_FIND_SYS_ERROR, IDOK, IDCANCEL, TYPE_INFO);

			RecoverCompleteUIMsgboxEx(USER_COMMAND_BACKUP, 0, STRING_CREATE_DIR_ERROR);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR1);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR2);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR3);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR4);
			RecoverCompleteUIMsgboxEx(USER_COMMAND_RECOVER, 0, STRING_CREATE_DIR_ERROR5);

			DrawMsgBoxEx(STRING_PART_MODIFY_ERROR, IDOK, IDCANCEL, TYPE_WARNING);
			DrawMsgBoxEx(STRING_RECOVER_NEW_DISK_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_DISK_NOT_SELECTED_SMALL, IDOK, -1, TYPE_INFO);
			//STRING_NEW_DISK_SIZE_SMALL
			DrawMsgBoxEx(STRING_DISK_ORG_SYSDISK_NOT_FOUND, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DISK_UNKNOWN_TYPE, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DISK_ORG_SYSDISK_TOO_SMALL, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DISK_SAME_SELECTED_DISK, IDOK, -1, TYPE_INFO);
			//DrawMsgBoxEx(STRING_C_PART_CHANGE_GOON, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING1, IDOK, IDCANCEL, TYPE_INFO);
			DrawMsgBoxEx(STRING_RECOVER_OLD_DISK_WARNING2, IDOK, IDCANCEL, TYPE_INFO);
			//STRING_OLD_DISK_SYSDISK_TOO_SMALL
			DrawMsgBoxEx(STRING_PARTITION_CHANGED_FORCE_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_DONOT_SELECT_REMOVEABLE_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_NVME_TIPS, IDOK, IDCANCEL, TYPE_ENQUIRE);
			DrawMsgBoxEx(STRING_DISK_NOT_FOUND_ERROR, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DISK_CHECK_TIPS, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DISK_READ_ERROR_TIPS, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_DISK_WRITE_ERROR_TIPS, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_MEDIA_ERROR, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_UUID_ERROR, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_NEW_DISK_SIZE_SMALL, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_OLD_DISK_SYSDISK_TOO_SMALL, IDOK, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_VERSION, IDOK, -1, TYPE_INFO);

			DrawMsgBoxEx(STRING_RAID_BK_NO_RAID_DISK_RECOVERY_ERROR, IDOK, -1, TYPE_WARNING);
			DrawMsgBoxEx(STRING_RAID_DISK_NO_RAID_BK_RECOVERY_ERROR, IDOK, -1, TYPE_WARNING);
			DrawMsgBoxEx(STRING_RAID_BK_INFO_NO_MATCH_RECOVERY_ERROR, IDOK, -1, TYPE_WARNING);
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
		//if( g_DriveList->selected != -1) {
		//	WORD diskindex = g_DriveList->partindex[g_DriveList->selected];
		//	if(g_data.BackupDiskNumber == diskindex) {
		//		if (!(g_editbox->controlinfo.status & CON_STATUS_DISABLE)) {
		//			StrCopy(g_editbox->name, STRING_FACTORY_BACKUP);
		//			g_editbox->current_len = StrLength(STRING_FACTORY_BACKUP);
		//			Control_disable(&g_editbox->controlinfo);
		//		}
		//	} else {
		//		if (g_editbox->controlinfo.status & CON_STATUS_DISABLE) {
		//			g_editbox->name[0] = 0;
		//			g_editbox->current_len = 0;
		//			g_editbox->posofcursor = 0;
		//			Control_enable(&g_editbox->controlinfo);
		//		}
		//	}
		//}
	}

	///Skip ESC on Main Window Window
	//if (dmmessage->message == CHAR_ESC)
	//{
	//	MyLog(LOG_DEBUG, L"========MainWindow: Pressed ESC key.===========pwin: 0x%x", pwin);
	//	Window_exit(pwin, IDCANCEL);
	//}
}

void About_MyWindow_Dispatchmsg(PWINDOW pwin, struct DM_MESSAGE *dmmessage)
{
	About_Window_Dispatchmsg(pwin, dmmessage);

	//if (dmmessage->message == CHAR_ESC)
	//	Window_exit(pwin, IDCANCEL);
}

void Recover_MyWindow_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{
    Recover_Window_Dispatchmsg(pwin,dmmessage);
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	//if(dmmessage->message == CHAR_F1)
	//{
	//	DrawNewCopyrightDlg();
	//}

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
					g_data.bForceRestorePartition ^= TRUE;
					MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");

					if (TRUE == g_data.bForceRestorePartition)
					{
						DrawMsgBoxTwoLine(STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_ON_INFO, IDOK, -1, TYPE_INFO);
					}
					else
					{
						DrawMsgBoxEx(STRING_FORCE_RESTORE_MODE_OFF, IDOK, -1, TYPE_INFO);
					}
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
					if(DrawMsgBoxEx(STRING_FACTORY_RESTORE, IDOK, IDCANCEL, TYPE_ENQUIRE) == IDOK) 
					{
						g_data.bFactoryRestore = TRUE;
						g_checkntfslog = 1;
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
					if (DeleteFactoryBackupfile() == TRUE)
					{
						struct DM_MESSAGE dmessage;
						dmessage.message = WM_UPDATE_RECOVERY_LIST_AFTER_DELETE;
						dmessage.thex = plistbox->whichselect;

						plistbox->controlinfo.sendmessage(plistbox, &dmessage);

						//Listbox_updateview(plistbox);
						//Window_exit(pwin,IDB_DELETE);
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
		//if( g_DriveList->selected != -1) {
		//	WORD diskindex = g_DriveList->partindex[g_DriveList->selected];
		//	if(g_data.BackupDiskNumber == diskindex) {
		//		if (!(g_editbox->controlinfo.status & CON_STATUS_DISABLE)) {
		//			StrCopy(g_editbox->name, STRING_FACTORY_BACKUP);
		//			g_editbox->current_len = StrLength(STRING_FACTORY_BACKUP);
		//			Control_disable(&g_editbox->controlinfo);
		//		}
		//	} else {
		//		if (g_editbox->controlinfo.status & CON_STATUS_DISABLE) {
		//			g_editbox->name[0] = 0;
		//			g_editbox->current_len = 0;
		//			g_editbox->posofcursor = 0;
		//			Control_enable(&g_editbox->controlinfo);
		//		}
		//	}
		//}
	}

    //if (dmmessage->message == CHAR_ESC)
    //    Window_exit(pwin, IDCANCEL);
}
void SelectDiskMyWindow_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{
    //SelectDisk_Window_Dispatchmsg(pwin,dmmessage);
	Window_Dispatchmsg(pwin, dmmessage);

    if( pwin->ifexit == 1)
    {
    	 return;
    }	
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	//if(dmmessage->message == CHAR_F1)
	//{
	//	DrawNewCopyrightDlg();
	//}

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
					g_data.bForceRestorePartition ^= TRUE;
					MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");

					if (TRUE == g_data.bForceRestorePartition)
					{
						DrawMsgBoxTwoLine(STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_ON_INFO, IDOK, -1, TYPE_INFO);
					}
					else
					{
						DrawMsgBoxEx(STRING_FORCE_RESTORE_MODE_OFF, IDOK, -1, TYPE_INFO);
					}
				}
				break;

			case 'a':
			case 'A':
			case 'a'-0x60:
				g_data.bAsyncIO ^= TRUE;
				MyLog(LOG_MESSAGE, L"AsyncIO %s.", g_data.bAsyncIO ? L"on" : L"off");
				break;

			default:
				break;
		}
	}

    //if (dmmessage->message == CHAR_ESC)
    //    Window_exit(pwin, IDCANCEL);
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
		//MyLog(LOG_DEBUG, L"====strbutton_default pcon->pwin: 0x%x, pcon->control_id: 0x%x", pcon->pwin, pcon->control_id);
        Window_exit(pcon->pwin, pcon->control_id);
    }
}

WORD GetLineOfMessage(CHAR16 *buffer, CHAR16 *msg, WORD maxwidth, WORD *width)
{
	WORD i = 0;
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

	if (NULL != width)
	{
		*width = stringwidth;
	}

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
	
	if (NULL != width)
	{
		*width = dwMaxWidth;
	}
	return nlines;
}

//todo
void PrintMessageInRectEx(WORD x, WORD y, WORD strWidth, WORD height, CHAR16 *msg, DWORD color, StringInfo *pstStringInfo)
{
	CHAR16 buf[512] = {0};
	CHAR16  *p = (CHAR16 *)buf;
	WORD i = 0, nlines = 0;
	WORD width = strWidth;

	//width = MSGBOX_WIDTH - 24;
	nlines = ParseString(msg, buf, &width, height);
	//y += (height - nlines * 18) / 3;
	if (nlines == 1 && width < strWidth)
		x += (strWidth - width) / 2;

	for (i = 0; i < nlines; i++)
	{
		DisplayStringEx(x, y, color, p, pstStringInfo, TRUE);	//COLOR_WHITE
		while (*p++ != 0);
		y += 20;  //TOOD font size + space
	}
}

//todo confirm
void okrupdatepercent(int a)
{
	//CHAR16 text[64]={0};
	//SPrint(text, 64, L"正在调整系统分区大小 %d%%", a);
	//FillRect(			g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300, 20, COLOR_CLIENT);
	//DisplayStringInRect(g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300,20, 0X2f557c, text);

	//MyLog(LOG_DEBUG, L"okrupdatepercent: %d %\n", a);
	if (a >= 0)
	{
		ShowWaitMsgFace(STRING_ADJUST_SYSTEM_SIZE);
	}
}
void okrupdatepercent_last(void)
{
	//CHAR16 *text=L"正在做最后设置，请稍候。。。";

	//FillRect(			g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300, 20, COLOR_CLIENT); 
	//DisplayStringInRect(g_x_begin + 220+100, (g_WindowHeight / 3 - 60) / 2 + 180 + 70, 300,20, 0X2f557c, text);
	//MyLog(LOG_DEBUG, L"okrupdatepercent_last...\n");

	if (g_ShowOnceWndBuff != NULL)
	{
		hidemouse();
		PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, g_ShowOnceWndBuff);
		showmouse();
	}

	g_showAdjustPartText = 0;
	ShowWaitMsgFace(STRING_SET_CONFIG);
}

int Listbox_showoneitem_ex(PLISTBOX plistbox, WORD item_index, WORD current_select, DWORD backcolor, DWORD forecolor)
{
	if (NULL != plistbox)
	{
		//backcolor = COLOR_WHITE;
		//forecolor = COLOR_WHITE;
		BYTE buff[512] = { 0 };
		WORD *str = NULL;
		WORD x = 0, y = 0;
		WORD width = 0;

		//MyLog(LOG_DEBUG, L"showoneitem_ex:%d, %d, 0x%08X, 0x%08X\n", item_index, current_select, backcolor, forecolor);
		if (item_index >= plistbox->item_num)
		{
			//MyLog(LOG_DEBUG, L"item_index %d >= plistbox->item_num %d\n", item_index, plistbox->item_num);
			return 0;
		}

		if (NULL != plistbox->pstBuffData && NULL != plistbox->pstBuffData->pBuffData)
		{
			EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffDataPos = plistbox->pstBuffData->pBuffData;

			WORD dwSpan = LIST_SPAN * g_data.fMinscale;

			x = plistbox->controlinfo.thex + dwSpan;
			y = current_select * plistbox->height_peritem + plistbox->controlinfo.they;

			CopyMem(&buff, plistbox->itemlist[item_index].item_buff, 512 - 4);

			if (backcolor != COLOR_WHITE)
			{
				//MyLog(LOG_DEBUG, L"BufferDrawLine: 0x%X,%d,%d,%d,%d,0x%08X\n", plistbox->pstBuffData->pBuffData, x + 1, current_select * plistbox->height_peritem, plistbox->controlinfo.thew - 2, plistbox->height_peritem, backcolor);
				BufferDrawLine(plistbox->pstBuffData->pBuffData, x + 1, 0, plistbox->controlinfo.thew - 2, plistbox->height_peritem, backcolor);
			}

			str = (WORD *)buff;

			StringInfo stStringInfo;
			stStringInfo.enumFontType = FZLTHJW;
			stStringInfo.iFontSize = CalcFontSize(FONT_LIST_ITEM_TEXT_SIZE); 

			width = plistbox->controlinfo.thew - dwSpan;

			WORD dwTextHeight = SEL_REC_POS_ICON_HEIGHT * g_data.fMinscale;
			WORD dwTextTop = y + (SEL_REC_POINT_LIST_ITEM_HEIGHT - SEL_REC_POS_ICON_HEIGHT) * g_data.fMinscale / 2;
			//DisplayStringInWidthEx(x + 4, dwTextTop, forecolor, (CHAR16 *)((PBYTE)str + 4), &stStringInfo, width - 4, 1, TRUE);
			//DisplayStringEx(x + 4, dwTextTop, forecolor, (CHAR16 *)((PBYTE)str + 4), &stStringInfo, TRUE);

			WORD dwOffsetX = 4 + dwSpan;
			WORD dwOffsetY = (SEL_REC_POINT_LIST_ITEM_HEIGHT - SEL_REC_POS_ICON_HEIGHT) * g_data.fMinscale / 2;
			plistbox->pstBuffData->pBuffData += dwOffsetY * plistbox->pstBuffData->buffWidth + dwOffsetX;

			SetTextBKColor(backcolor);
			BufferDisplayStringEx(plistbox->pstBuffData, x + 4, dwTextTop, forecolor, (CHAR16 *)((PBYTE)str + 4), &stStringInfo, TRUE);

			plistbox->pstBuffData->pBuffData = buffDataPos;
		}
	}

	return 0;
}

void aboutlist_dispatchmsg(PCONTROL pcontrol, struct DM_MESSAGE *dmessage)
{
	//MyLog(LOG_DEBUG, L"recoverlist_dispatchmsg\n");
	PLISTBOX pListbox = (PLISTBOX)pcontrol;
	Listbox_DispatchMsg(pcontrol, dmessage);
		
	if (dmessage->message == WM_UPDATE_ABOUT_PAGE_INTRODUCE_LIST)
	{
		float fMinscale = g_data.fMinscale;
		WORD dwLeft = 0;
		WORD dwTop = 0;
		WORD dwWidth = 0;
		WORD dwHeight = 0;
		DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;

		Control_enable(pListbox);
		Control_enable(pListbox->pscroll);

		dwLeft = dwBGLeft + ABOUT_PAGE_TAB_AREA_LEFT * fMinscale;
		dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
		dwWidth = ABOUT_PAGE_TAB_AREA_WIDTH * fMinscale;
		dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_AREA_TOP - 150) * fMinscale;
		}
		DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

		hidemouse();   
		Listbox_delallitem(pListbox);
		showmouse();

		dwLeft = dwBGLeft + ABOUT_PAGE_SUB_TITLE_LEFT * fMinscale;
		dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
		dwWidth = TAB_CONTENT_WIDTH * fMinscale;
		dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_AREA_TOP - 150) * fMinscale;
		}
		ShowIntroduce(pListbox, pListbox->pscroll, dwLeft, dwTop, dwWidth, dwHeight);
		Listbox_updateview(pListbox);
	}
	else if (dmessage->message == WM_UPDATE_ABOUT_PAGE_OPENSOURCE_LIST)
	{
		float fMinscale = g_data.fMinscale;
		WORD dwLeft = 0;
		WORD dwTop = 0;
		WORD dwWidth = 0;
		WORD dwHeight = 0;
		DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;

		Control_enable(pListbox);
		Control_enable(pListbox->pscroll);

		dwLeft = dwBGLeft + ABOUT_PAGE_TAB_AREA_LEFT * fMinscale;
		dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
		dwWidth = ABOUT_PAGE_TAB_AREA_WIDTH * fMinscale;
		dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_AREA_TOP - 150) * fMinscale;
		}
		DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

		hidemouse();
		Listbox_delallitem(pListbox);
		showmouse();

		dwLeft = dwBGLeft + ABOUT_PAGE_SUB_TITLE_LEFT * fMinscale;
		dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
		dwWidth = TAB_CONTENT_WIDTH * fMinscale;
		dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_AREA_TOP - 150) * fMinscale;
		}
		ShowOpenSource(pListbox, pListbox->pscroll, dwLeft, dwTop, dwWidth, dwHeight);
		Listbox_updateview(pListbox);
	}
	else if (dmessage->message == WM_UPDATE_ABOUT_PAGE_VERSION)
	{
		float fMinscale = g_data.fMinscale;
		WORD dwLeft = 0;
		WORD dwTop = 0;
		WORD dwWidth = 0;
		WORD dwHeight = 0;
		DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;

		hidemouse();
		Listbox_delallitem(pListbox);
		showmouse();

		Control_disable(pListbox);
		Control_disable(pListbox->pscroll);

		dwLeft = dwBGLeft + ABOUT_PAGE_TAB_AREA_LEFT * fMinscale;
		dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
		dwWidth = ABOUT_PAGE_TAB_AREA_WIDTH * fMinscale;
		dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_AREA_TOP - 150) * fMinscale;
		}
		DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

		dwLeft = (g_WindowWidth - ABOUT_PAGE_TAB_VER_LOGO_SIZE * fMinscale) / 2;
		dwTop = ABOUT_PAGE_TAB_VER_LOGO_TOP * fMinscale;
		dwWidth = ABOUT_PAGE_TAB_VER_LOGO_SIZE * fMinscale;
		dwHeight = ABOUT_PAGE_TAB_VER_LOGO_SIZE * fMinscale;

		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			dwLeft = (g_WindowWidth - ABOUT_PAGE_TAB_VER_LOGO_SIZE * 0.5) / 2;
			dwTop = (ABOUT_PAGE_TAB_VER_LOGO_TOP - 30)* fMinscale;
			dwWidth = ABOUT_PAGE_TAB_VER_LOGO_SIZE * 0.5;
			dwHeight = ABOUT_PAGE_TAB_VER_LOGO_SIZE * 0.5;
		}
		else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_VER_LOGO_TOP - 150) * fMinscale;
		}

		WORD unfocuspcxAppLogo = GetImageFileId(IMAG_FILE_APP_LOGO);
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxAppLogo, FALSE);

		///////App Name
		StringInfo stStringInfoAPP_NAME;
		stStringInfoAPP_NAME.enumFontType = FZLTZHJW;
		stStringInfoAPP_NAME.iFontSize = CalcFontSize(50); //(int)(50 * g_data.fFontScale + 0.5);

		dwWidth = 0;
		GetStringLen(STRING_APP_NAME, &stStringInfoAPP_NAME, &dwWidth);
		dwLeft = (g_WindowWidth - dwWidth) / 2;

		dwTop = ABOUT_PAGE_TAB_VER_APP_NAME_TOP * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_VER_APP_NAME_TOP - 150) * fMinscale;
		}
		DisplayStringEx(dwLeft, dwTop, COLOR_DEFAULT_BACKGROUND, STRING_APP_NAME, &stStringInfoAPP_NAME, TRUE);

		////Version
		StringInfo stStringInfoCUR_VERSION;
		stStringInfoCUR_VERSION.enumFontType = FZLTHJW;
		stStringInfoCUR_VERSION.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

		dwWidth = 0;
		GetStringLen(STRING_CUR_VERSION, &stStringInfoCUR_VERSION, &dwWidth);
		dwLeft = (g_WindowWidth - dwWidth) / 2;
		dwTop = ABOUT_PAGE_TAB_VER_VERSION_TOP * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_VER_VERSION_TOP - 150) * fMinscale;
		}
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_CUR_VERSION, &stStringInfoCUR_VERSION, TRUE);

		//////Copyright
		StringInfo stStringInfoCOPYRIGHT;
		stStringInfoCOPYRIGHT.enumFontType = FZLTHJW;
		stStringInfoCOPYRIGHT.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

		dwWidth = 0;
		GetStringLen(STRING_COPYRIGHT, &stStringInfoCOPYRIGHT, &dwWidth);
		dwLeft = (g_WindowWidth - dwWidth) / 2;
		dwTop = ABOUT_PAGE_TAB_COPYRIGHT_TOP * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (ABOUT_PAGE_TAB_COPYRIGHT_TOP - 150) * fMinscale;
		}
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_COPYRIGHT, &stStringInfoCOPYRIGHT, TRUE);
	}
}

CHAR16 *g_introduce_text[] =
{
	L"联想一键恢复软件（UEFI OKR）",
	L" ",
	L" ",
	L"一、服务分区（OEM分区）特殊说明",
	L"       如果您在操作系统【磁盘管理】中发现有一个在系统中不可见的≥20GB，标识为恢复分区（卷标：Lenovo_Part）时，请不要删除该分区。该分区为联想服务分区，用于存储联想一键恢复软件及相关数据，如果该服务分区被非联想维修人员所进行的操作删除或损坏，联想一键恢复软件将可能无法使用，联想（北京）有限公司将不对由此而导致的任何损失承担责任。",
	L" ",
	L"二、功能概述",
	L"       联想一键恢复软件为您提供对Windows、Linux系统进行备份和恢复，确保Windows、Linux系统安全、稳定运行。通过该软件对Windows、Linux系统进行备份之后，无论是因为系统分区信息遭到破坏、计算机病毒发作、硬件驱动或应用程序错误等原因，造成Windows、Linux系统无法正常启动和使用时，可以通过该软件将Windows、Linux系统快速恢复到之前备份时的状态。",
	L"       在进行系统备份时，将同步备份系统分区所有数据，包含系统设置，硬件驱动，已安装的应用程序、用户数据等。基于该系统备份文件可快速将系统恢复到工作状态，同时无需进行系统设置，驱动及应用程序安装、配置等操作。",
	L"       为保护您电脑系统及数据安全，在进行系统恢复之前，请将原系统分区中的重要数据备份到其它分区或USB移动存储设备中。",
	L" ",
	L"三、支持操作系统",
	L"       支持Windows 7, 8/8.1, 10, 11 系统备份和恢复",
	L"       支持联想预装中科方德，UOS，KyIin麒麟，Ubuntu等Linux系统备份和恢复",
	L" ",
	L"四、使用介绍",
	L"4.1 调用",
	L"       在开机启动后，出现联想“Lenovo”标识时，请您按下“F2”或“NOVO”键，将会进入“联想一键恢复”程序主界面。",
	L" ",
	L"4.2 出厂备份",
	L"       出厂时已将Windows、Linux系统所在分区的所有数据备份到联想服务分区中，当Windows、Linux系统被破坏后，您可以将系统快速恢复到出厂时的状态。",
	L" ",
	L"4.3 初始备份",
	L"       如果软件检测到当前联想服务分区中没有系统备份，则默认将您创建的第一个系统备份存储到联想服务分区中，并标记为初始备份。",
	L" ",
	L"4.4 系统备份",
	L"       您可以将Windows、Linux系统所在分区所有数据全部备份到您所指定的分区中（包含USB移动存储设备上的分区）。当Windows、Linux操作系统被破坏后，您可以通过“系统恢复”功能将系统恢复到之前创建系统备份时的状态。系统备份文件保存在所选分区根目录的“LenovoOKR_Backup”文件夹中。",
	L"       对于不需要的系统备份文件，可在选择系统备份文件界面进行删除。",
	L"       为了让系统能更好的恢复到您需要的状态，请在系统状态有重大更新（比如升级了Windows、Linux的重要补丁、安装了某些重要软件等）后，及时创建新的系统备份。",
	L"操作步骤：",
	L"       1. 点击主界面“系统备份”按钮进入备份界面。",
	L"       2. 选择可用空间足够的磁盘分区后，点击“开始备份”按钮进行备份。",
	L"注意：",
	L"       1）备份前，一定要确保Windows、Linux系统是正常运行的，否则无法通过这个备份将Windows、Linux系统恢复到正常运行状态。",
	L"       2）备份保存分区的盘符可能与在Windows、Linux系统中看到的不一致，这时要以分区的卷标、容量、可用空间等信息进一步确认。",
	L"       3）如果磁盘分区启用了BitLocker加密，则无法找到系统分区以及其它存储分区。需要先在Windows中关闭BitLocker加密之后才可进行系统备份。",
	L"            关闭BitLocker方法：",
	L"                   BitLocker设备加密：Windows设置 → 更新和安全 → 设备加密 → 关闭",
	L"                   BitLocker驱动器加密：控制面板 → BitLocker驱动器加密 → 检查并关闭磁盘及分区加密",
	L" ",
	L"4.5 系统恢复",
	L"       将Windows、Linux系统恢复到您所选系统备份时的状态。恢复过程中将删除当前系统分区所有数据。建议您备份该分区重要数据之后再进行操作，以避免导致数据丢失，造成不可挽回的损失。",
	L"    ",
	L"操作步骤：",
	L"       1. 点击主界面“系统恢复”按钮进入恢复界面。",
	L"       2. 选择要恢复的系统备份文件后，点击“恢复”按钮。",
	L"注意：",
	L"       1）当Windows、Linux系统无法正常使用时，优先推荐您进行“系统恢复”，不建议直接使用Windows、Linux安装介质重装系统。",
	L"       2）恢复前，请确保已将系统分区的重要数据备份到其他位置或存储介质，否则一旦开始恢复，系统分区的数据将全部被删除！",
	L"       3）如果系统分区起始位置较之前创建的系统备份发生了改变，您将无法将系统恢复到该系统备份时状态。您可尝试选择其他系统备份文件进行系统恢复或按Ctrl + F组合键开启强制恢复模式进行恢复。",
	L"       4）如果您在Windows中对系统分区容量进行了压缩，使用已创建的系统备份文件进行恢复时，如果压缩后的系统分区容量大小满足系统恢复所需要的容量，可以正常恢复。如果不满足，则可按Ctrl + F组合键开启强制恢复模式进行恢复。",
	L"       5）如果磁盘分区启用了BitLocker加密，则无法找到系统分区以及其它存储分区。需要在Windows中关闭BitLocker加密之后才可进行系统恢复。如果在此情况下，系统已崩溃，无法进入系统关闭BitLocker加密时，可基于存储在联想服务分区中的出厂备份或初始备份文件使用强制恢复功能，将系统恢复到出厂状态或者初始备份时状态。",
	L"            关闭BitLocker方法：",
	L"                   BitLocker设备加密：Windows设置 → 更新和安全 → 设备加密 → 关闭",
	L"                   BitLocker驱动器加密：控制面板 → BitLocker驱动器加密 → 检查并关闭磁盘及分区加密",
	L"       6）可将系统备份恢复到另一块新磁盘中。在接入新磁盘后，程序将自动识别并提示，按照界面提示进行操作即可。该恢复采用的是强制恢复机制，恢复过程中将重新初始化新磁盘，并将系统备份的磁盘分区结构及系统分区数据全部复制到新磁盘中。（注：新磁盘容量大小需要等于或大于当前系统分区所在磁盘容量大小）",
	L" ",
	L"4.6 常见问题及解决方案",
	L"       1. 如果在开机、重启过程中按“F2”或“NOVO”键后没有调出OKR软件界面。请多次尝试在开机、重启过程中快速连续按“F2”或“NOVO”键。如果还是没有调出OKR软件，请进入操作系统，在【磁盘管理】中检查联想服务分区是否存在。",
	L"       2. 如果系统异常或已损坏，无法正常进入Windows关闭BitLocker加密时，可在任意界面按“Ctrl + F”组合键，按照提示信息启用强制恢复功能进行系统恢复。强制恢复时，系统及磁盘分区结构将恢复为所选系统备份文件所记录的状态。",
	L"       3. 如果在Windows中，磁盘分区启用了BitLocker加密，对系统分区（C:\\）进行强制恢复之后，Windows系统设置 → 更新和安全 → 设备加密项中的”设备加密”开关按钮有可能会消失。此时除系统分区（C:\\) 外，其它分区仍处于BitLocker设备加密状态。如果需要对系统分区（C:\)继续启用BitLocker加密，可在点击BitLocker设置，对系统分区启用BitLocker驱动加密。（BitLocker设备加密请参看微软官网介绍）",
	L"       4. 将系统恢复到新磁盘后，需要关机断电移除其中一块磁盘再启动计算机，否则将无法正常启动系统。",
	L"       5. 将系统从SATA磁盘恢复到NVME磁盘，从NVME磁盘启动时可能会出现蓝屏现象。如出现该情况，请多次重启计算机，系统将自动进入修复模式，在修复模式中选择进入安全模式之后再重启，系统可正常启动并进入桌面。",
	L"           进入安全模式方法：在出现【自动修复】页面，选择【高级选项】→【疑难解答】→【高级选项】→【启动设置】，点击【重启】，重启后选择【4启用安全模式】。",
	L"       6. 在进行系统恢复时，恢复的目标分区容量大小一定要等于或大于备份时的系统分区容量。当小于备份时的分区容量时，该软件会有相应信息提示。",
	L"       7. 该软件目前仅支持单操作系统备份和恢复，暂不支持双操作系统情况下的系统备份和恢复。",
};

void ShowContent(
	LISTBOX *pListBox,
	SCROLL_BOX *pScrollBox,
	WORD dwLeft,
	WORD dwTop,
	WORD dwWidth,
	WORD dwHeight,
	CHAR16 ** pTextSource,
	WORD tetxSourceSize,
	WORD textType)
{
	float fMinscale = g_data.fMinscale;
	WORD indexList = 0;
	//DWORD dwItemHeight = (DWORD)(50 * fMinscale);
	//if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	//{
	//	dwItemHeight = (DWORD)(50 * 0.4);
	//}

	//Listbox_initEx(pListBox, dwLeft, dwTop, dwWidth, dwHeight, dwItemHeight, 512 * 512);
	////pListBox->controlinfo.sendmessage = ShowIntroduceList_dispatchmsg;
	//pListBox->showoneitem = Listbox_showoneitem_ex;
	//pListBox->controlinfo.sendmessage = aboutlist_dispatchmsg;
	//Listbox_SetbkColor(pListBox, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF);

	WORD i = 0;
	WORD LIMIT_LEN = (TAB_CONTENT_WIDTH - 45) * fMinscale;

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_LIST_ITEM_TEXT_SIZE); //(int)(FONT_LIST_ITEM_TEXT_SIZE * g_data.fFontScale + 0.5);

	for (i = 0; i < tetxSourceSize; i++)
	{
		CHAR16 *pTemp = pTextSource[i];

		WORD len = (WORD)StrLength(pTemp);
		WORD itextIndex = 0;
		while (itextIndex < len)
		{
			CHAR16 bufferTemp[500] = { 0 };
			WORD textLen = 0;
			if (textType == 1)
			{
				//textLen = GetLimitTextIndex_1(&pTemp[itextIndex], bufferTemp, LIMIT_LEN);
				textLen = GetLimitTextIndex_1_ex(&pTemp[itextIndex], bufferTemp, LIMIT_LEN, &stStringInfo);
			}
			else
			{
				//textLen = GetLimitTextIndex_2(&pTemp[itextIndex], bufferTemp, LIMIT_LEN);
				textLen = GetLimitTextIndex_2_ex(&pTemp[itextIndex], bufferTemp, LIMIT_LEN, &stStringInfo);
			}
			if (textLen > 0)
			{
				Listbox_Addoneline(pListBox);
				Listbox_Addoneunit(pListBox, ITEM_TYPE_CHAR_STRING, bufferTemp);			
				itextIndex += textLen;
				indexList++;
			}
			else
			{
				break;
			}
		}
	}

	MyLog(LOG_ERROR, L"ShowContent indexcout: %d\n", indexList);
	Scrollbox_init(pScrollBox, (PCONTROL)pListBox, indexList, pListBox->items_per_win);
	pListBox->pscroll = pScrollBox;
}

void ShowIntroduce(
	LISTBOX *pListBox,
	SCROLL_BOX *pScrollBox,
	WORD dwLeft,
	WORD dwTop,
	WORD dwWidth,
	WORD dwHeight)
{
	ShowContent(
		pListBox, 
		pScrollBox, 
		dwLeft, 
		dwTop, 
		dwWidth, 
		dwHeight, 
		g_introduce_text, 
		sizeof(g_introduce_text) / sizeof(CHAR16 *), 
		1);
}

void ShowOpenSource(
	LISTBOX *pListBox,
	SCROLL_BOX *pScrollBox,
	WORD dwLeft,
	WORD dwTop,
	WORD dwWidth,
	WORD dwHeight)
{
	CHAR16 *g_opensource_text[20];
	g_opensource_text[0] = LicenseStr_1;
	g_opensource_text[1] = LicenseStr_2;
	g_opensource_text[2] = LicenseStr_3;
	g_opensource_text[3] = LicenseStr_4;
	g_opensource_text[4] = LicenseStr_5;
	g_opensource_text[5] = LicenseStr_6;
	g_opensource_text[6] = LicenseStr_7;
	g_opensource_text[7] = LicenseStr_8;
	g_opensource_text[8] = LicenseStr_9;
	g_opensource_text[9] = LicenseStr_10;
	g_opensource_text[10] = LicenseStr_11;
	g_opensource_text[11] = LicenseStr_12;
	g_opensource_text[12] = LicenseStr_13;
	g_opensource_text[13] = LicenseStr_14;
	g_opensource_text[14] = LicenseStr_15;
	g_opensource_text[15] = LicenseStr_16;
	g_opensource_text[16] = LicenseStr_17;
	g_opensource_text[17] = LicenseStr_18;
	g_opensource_text[18] = LicenseStr_19;
	g_opensource_text[19] = LicenseStr_20;
	ShowContent(
		pListBox, 
		pScrollBox, 
		dwLeft, 
		dwTop, 
		dwWidth, 
		dwHeight, 
		g_opensource_text, 
		20,
		2);
}

void ShowAboutFace(BOOL bIsPressF1)
{
	PBYTE buff = NULL;
	if (bIsPressF1 == TRUE)
	{
		hidemouse();
		buff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
		if (NULL != buff)
		{
			GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
		}

		showmouse();
	}

	AboutFace();


	if (bIsPressF1 == TRUE)
	{
		hidemouse();
		PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
		showmouse();
		FreePool(buff);
	}
}


void AboutFace_Tab_Button_show(struct CONTROL * pCtrl)
{
	//MyLog(LOG_DEBUG, L"AboutFace_Tab_Button_show");

	struct BUTTON *pbnctrl = (struct BUTTON *)pCtrl;

	hidemouse();

	StringInfo stStringInfoABOUT;
	stStringInfoABOUT.enumFontType = FZLTHJW;
	stStringInfoABOUT.iFontSize = CalcFontSize(40); //(int)(40 * g_data.fFontScale + 0.5);

	PBUTTON_INFO pButtonInfo = FindBtnInfoFromID(pbnctrl->controlinfo.control_id);
	if (pbnctrl->controlinfo.state == CONTROL_STATE_NORMAL)
	{
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_ABOUT_TAB_BK);
		DisplayStringEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, COLOR_ABOUT_TAB_NORMAL, pButtonInfo->pText, &stStringInfoABOUT, TRUE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_FOCUS) 
	{	
		stStringInfoABOUT.enumFontType = FZLTZHJW;
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_ABOUT_TAB_BK);
		DisplayStringEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, COLOR_ABOUT_TAB_FOCUS, pButtonInfo->pText, &stStringInfoABOUT, TRUE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_SELECT)
	{
		stStringInfoABOUT.enumFontType = FZLTZHJW;
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_ABOUT_TAB_BK);
		DisplayStringEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, COLOR_ABOUT_TAB_SELECT, pButtonInfo->pText, &stStringInfoABOUT, TRUE);
	}
	//else if (pbnctrl->controlinfo.state == CONTROL_STATE_PRESSED) 
	//{
	//	//DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
	//	//	pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, pbnctrl->btn_pressedpcx, FALSE);

	//	stStringInfoABOUT.enumFontType = FZLTZHJW;
	//	DisplayStringEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, COLOR_ABOUT_TAB_FOCUS, pButtonInfo->pText, &stStringInfoABOUT, TRUE);
	//}

	showmouse();
	return;
}

void btn_AboutFaceTab(PCONTROL pcon, struct DM_MESSAGE* dmessage)
{
	struct BUTTON * pbnctrl = (struct BUTTON *)pcon;
	BOOL bRedraw = TRUE;
	WORD wSelectedControlId = -1;
	switch (dmessage->message)
	{
	case WM_LEFTBUTTONPRESSED: //响应鼠标消息

		if (pbnctrl->ifpressed == YES)
			break;
		pbnctrl->controlinfo.state = CONTROL_STATE_PRESSED;
		pbnctrl->ifpressed = YES;
		break;
	case WM_LEFTBUTTONRELEASE:
		if (pbnctrl->ifpressed == NO)
			break;
		pbnctrl->ifpressed = NO;
	case CHAR_CARRIAGE_RETURN:
	case L' ':
		pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
		dmessage->message = WM_BUTTON_CLICK;// 转化成鼠标点击消息
		pcon->pwin->selected = pcon->pwin->current;

		//MyLog(LOG_DEBUG, L"Enter win sel: %d %d\n", pcon->pwin->selected, bIsEnterBtn);
		break;
	case WM_CTL_ACTIVATE:
		//MyLog(LOG_ERROR, L"WM_CTL_ACTIVATE\n");
		//MyLog(LOG_DEBUG, L"control_id: %d current: %d\n", pcon->control_id, pcon->pwin->current);
		pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
		//MyLog(LOG_DEBUG, L"selected control_id: %d selected: %d\n", pcon->pwin->control_list[pcon->pwin->selected]->control_id, pcon->pwin->selected);
		break;
	case WM_CTL_INACTIVATE:
	case WM_CTL_ENABLE:
		//MyLog(LOG_ERROR, L"WM_CTL_INACTIVATE\n");
		//MyLog(LOG_DEBUG, L"control_id: %d precurrent: %d\n", pcon->control_id, pcon->pwin->precurrent);
		wSelectedControlId = pcon->pwin->control_list[pcon->pwin->selected]->control_id;
		//MyLog(LOG_DEBUG, L"selected control_id: %d selected: %d\n", wSelectedControlId, pcon->pwin->selected);

		if (pcon->control_id != wSelectedControlId)
		{
			//MyLog(LOG_DEBUG, L"CONTROL_STATE_NORMAL\n");
			pbnctrl->controlinfo.state = CONTROL_STATE_NORMAL;
		}
		else
		{
			//MyLog(LOG_DEBUG, L"CONTROL_STATE_SELECT\n");
			pbnctrl->controlinfo.state = CONTROL_STATE_SELECT;
		}

		break;
	//case WM_MOUSEMOVEIN: //鼠标移入
	//	pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
	//	break;
	//case WM_MOUSEMOVEOUT: //鼠标移出
	//	pbnctrl->controlinfo.state = CONTROL_STATE_NORMAL;
	//	break;
	default:
		bRedraw = FALSE;
		break;
	}

	if (bRedraw)
		AboutFace_Tab_Button_show(&pbnctrl->controlinfo);

	if (dmessage->message == WM_BUTTON_CLICK) 
	{
		//if (pcon->pwin->selected == pcon->pwin->current)
		//{
		//	MyLog(LOG_ERROR, L"selected == current.\n");
		//	return;
		//}

		PCONTROL pControl = NULL;
		for (int bId = IDB_MSG_BTN_INTRODUCE; bId <= IDB_MSG_BTN_VERSION; bId++)
		{
			//MyLog(LOG_DEBUG, L"11111111111\n");
			pControl = FindControlInWindowById(pcon->pwin, bId);
			if (pControl != NULL &&
				pControl->control_index != pcon->pwin->selected &&
				pControl->state != CONTROL_STATE_NORMAL)
			{
				//MyLog(LOG_DEBUG, L"Set normal status. %d\n", pControl->control_index);
				pControl->state = CONTROL_STATE_NORMAL;
				AboutFace_Tab_Button_show(pControl);
			}
		}
		pControl = NULL;

		pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
		pcon->pwin->selected = pcon->pwin->current;

		struct DM_MESSAGE dmessage;
		PLISTBOX pListBox = FindControlInWindowByType(pcon->pwin, IDC_LISTBOX);

		if (pcon->control_id == IDB_MSG_BTN_INTRODUCE)
		{
			//MyLog(LOG_ERROR, L"Click IDB_MSG_BTN_INTRODUCE.\n");

			//pcon->pwin->current = 0;
			//pcon->pwin->selected = pcon->pwin->current;

			dmessage.message = WM_UPDATE_ABOUT_PAGE_INTRODUCE_LIST;			
			dmessage.thex = 0;

			pListBox->controlinfo.sendmessage(pListBox, &dmessage);

		}
		else if (pcon->control_id == IDB_MSG_BTN_OPEN_SOURCE)
		{
			//MyLog(LOG_ERROR, L"Click IDB_MSG_BTN_OPEN_SOURCE.\n");

			//pcon->pwin->current = 1;
			//pcon->pwin->selected = pcon->pwin->current;

			dmessage.message = WM_UPDATE_ABOUT_PAGE_OPENSOURCE_LIST;
			dmessage.thex = 0;

			pListBox->controlinfo.sendmessage(pListBox, &dmessage);
		}
		else if (pcon->control_id == IDB_MSG_BTN_VERSION)
		{
			//MyLog(LOG_ERROR, L"Click IDB_MSG_BTN_VERSION.\n");

			//pcon->pwin->current = 2;
			//pcon->pwin->selected = pcon->pwin->current;

			dmessage.message = WM_UPDATE_ABOUT_PAGE_VERSION;
			dmessage.thex =0;

			pListBox->controlinfo.sendmessage(pListBox, &dmessage);
		}

		//MyLog(LOG_ERROR, L"dmessage->message == WM_BUTTON_CLICK\n");
		//pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
		//pcon->pwin->selected = pcon->pwin->current;
		//wWinControlId = pcon->pwin->control_list[pcon->pwin->selected]->control_id;
		//MyLog(LOG_ERROR, L"win sel control_id: %d\n", wWinControlId);
		//Window_exit(pcon->pwin, pcon->control_id);
	}

	return;
}

////////////////////////////////////////About Page//////////////////////////////////////////////////////
int AboutFace(/*TAB_ENUM enuSelectTab*/)
{
	int iRet = -1;

	struct BUTTON buttonctrl[4] = { 0 };
	struct WINDOW win;

	LISTBOX listbox;
	SCROLL_BOX scrollBox;

	BYTE oldstate;
	RECT rc;

	float fMinscale = g_data.fMinscale;
	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;
	//DWORD dwBGTop = (g_WindowHeight - WHITE_BG_HEIGHT * fMinscale) / 2;
	
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	MyLog(LOG_ERROR, L"----------AboutFace...\n");
	hidemouse();
	//GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	oldstate = EnableMouse(TRUE);

	//Tab introduce
	dwLeft = dwBGLeft + ABOUT_PAGE_TAB_INTRODUCE_LEFT * fMinscale;
	dwTop = ABOUT_PAGE_TAB_INTRODUCE_TOP * fMinscale;
	dwWidth = ABOUT_PAGE_TAB_INTRODUCE_WIDTH * fMinscale;
	dwHeight = ABOUT_PAGE_TAB_INTRODUCE_HEIGHT * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_TAB_INTRODUCE_TOP - 150) * fMinscale;
	}
	Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight, 0, 0, 0, 0);

	//Tab Open source
	dwLeft = dwBGLeft + ABOUT_PAGE_TAB_OPEN_SOURCE_LEFT * fMinscale;
	dwTop = ABOUT_PAGE_TAB_OPEN_SOURCE_TOP * fMinscale;
	dwWidth = ABOUT_PAGE_TAB_OPEN_SOURCE_WIDTH * fMinscale;
	dwHeight = ABOUT_PAGE_TAB_OPEN_SOURCE_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwWidth = (ABOUT_PAGE_TAB_OPEN_SOURCE_WIDTH + 60) * fMinscale;
		dwHeight = (ABOUT_PAGE_TAB_OPEN_SOURCE_HEIGHT + 30) * fMinscale;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_TAB_OPEN_SOURCE_TOP - 150) * fMinscale;
	}

	Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight, 0, 0, 0, 0);

	//Tab Version
	dwLeft = dwBGLeft + ABOUT_PAGE_TAB_VERSION_INFO_LEFT * fMinscale;
	dwTop = ABOUT_PAGE_TAB_VERSION_INFO_TOP * fMinscale;
	dwWidth = ABOUT_PAGE_TAB_VERSION_INFO_WIDTH * fMinscale;
	dwHeight = ABOUT_PAGE_TAB_VERSION_INFO_HEIGHT * fMinscale;

	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_TAB_VERSION_INFO_TOP - 150) * fMinscale;
	}
	Button_init(&buttonctrl[2], dwLeft, dwTop, dwWidth, dwHeight, 0, 0, 0, 0);

	//Backup button
	dwLeft = BACKUP_BUTTON_LEFT * fMinscale;
	dwTop = BACKUP_BUTTON_TOP * fMinscale;
	dwWidth = BACKUP_BUTTON_SIZE * fMinscale;
	dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = BACKUP_BUTTON_LEFT * 0.5;
		dwTop = BACKUP_BUTTON_TOP * 0.5;
		dwWidth = BACKUP_BUTTON_SIZE * 0.5;
		dwHeight = BACKUP_BUTTON_SIZE * 0.5;
	}

	WORD presspcxGoBack = GetImageFileId(IMG_FILE_BUTTON_GO_BACK_PRESS); //IMG_FILE_BUTTON_GO_BACK_PRESS;
	WORD focuspcxGoBack = GetImageFileId(IMAG_FILE_GOBACK_FOCUS); //IMAG_FILE_GOBACK_FOCUS;
	WORD unfocuspcxGoBack = GetImageFileId(IMG_FILE_GOBACK); //IMG_FILE_GOBACK;

	Button_init(&buttonctrl[3], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxGoBack, focuspcxGoBack, unfocuspcxGoBack, 0);

	Window_init(&win, NULL);
	win.msgfunc = About_MyWindow_Dispatchmsg;
	win.windowID = 501;

	hidemouse();

	ClearScreen();

	DrawWhiteBK();
	DisplayLogo();

	buttonctrl[0].controlinfo.control_id = IDB_MSG_BTN_INTRODUCE;
	buttonctrl[0].controlinfo.sendmessage = btn_AboutFaceTab;
	buttonctrl[0].controlinfo.show = AboutFace_Tab_Button_show;

	buttonctrl[1].controlinfo.control_id = IDB_MSG_BTN_OPEN_SOURCE;
	buttonctrl[1].controlinfo.sendmessage = btn_AboutFaceTab;
	buttonctrl[1].controlinfo.show = AboutFace_Tab_Button_show;

	buttonctrl[2].controlinfo.control_id = IDB_MSG_BTN_VERSION;
	buttonctrl[2].controlinfo.sendmessage = btn_AboutFaceTab;
	buttonctrl[2].controlinfo.show = AboutFace_Tab_Button_show;

	buttonctrl[3].controlinfo.control_id = IDCANCEL;
	buttonctrl[3].controlinfo.sendmessage = btn_default;

	//sub title
	dwLeft = dwBGLeft + ABOUT_PAGE_SUB_TITLE_LEFT * fMinscale;
	dwTop = ABOUT_PAGE_SUB_TITLE_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_SUB_TITLE_TOP - 150) * fMinscale;
	}
	StringInfo stStringInfoABOUT;
	stStringInfoABOUT.enumFontType = FZLTZHJW;
	stStringInfoABOUT.iFontSize = CalcFontSize(50); //(int)(50 * g_data.fFontScale + 0.5);
	DisplayStringEx(dwLeft, dwTop, COLOR_DEFAULT_BACKGROUND, STRING_ABOUT, &stStringInfoABOUT, TRUE);
	
	dwLeft = dwBGLeft + ABOUT_PAGE_SPLIT_LINE_LEFT * fMinscale;
	dwTop = ABOUT_PAGE_SPLIT_LINE_TOP * fMinscale;
	dwWidth = ABOUT_PAGE_SPLIT_LINE_WIDTH * fMinscale;
	dwHeight = 1;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_SPLIT_LINE_TOP - 150) * fMinscale;
	}
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xDCEFFF);

	//Title
	dwLeft = dwBGLeft;
	dwTop = ABOUT_PAGE_TITLE_TOP * fMinscale;
	dwWidth = ABOUT_PAGE_TITLE_WIDTH * fMinscale;
	dwHeight = ABOUT_PAGE_TITLE_HEIGHT * fMinscale;
	WORD unfocuspcxTitle = GetImageFileId(IMG_FILE_BACKGROUND_TITLE); //IMG_FILE_BACKGROUND_TITLE;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft;
		dwTop = (ABOUT_PAGE_TITLE_TOP - 100) * 0.5;
		dwWidth = ABOUT_PAGE_TITLE_WIDTH * 0.5;
		dwHeight = ABOUT_PAGE_TITLE_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_TITLE_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxTitle, FALSE); // title.bmp

	dwLeft = dwBGLeft + ABOUT_PAGE_TAB_AREA_LEFT * fMinscale;
	dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
	dwWidth = ABOUT_PAGE_TAB_AREA_WIDTH * fMinscale;
	dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_TAB_AREA_TOP - 150) * fMinscale;
	}
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);
	
	Window_addctl(&win, (PCONTROL)&buttonctrl[0]);
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]);
	Window_addctl(&win, (PCONTROL)&buttonctrl[2]);
	Window_addctl(&win, (PCONTROL)&buttonctrl[3]);

	//Introduce
	dwLeft = dwBGLeft + ABOUT_PAGE_SUB_TITLE_LEFT * fMinscale;
	dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
	dwWidth = TAB_CONTENT_WIDTH * fMinscale;
	dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
	
	WORD indexList = 0;
	DWORD dwItemHeight = (DWORD)(50 * fMinscale);
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwItemHeight = (DWORD)(50 * 0.4);
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (ABOUT_PAGE_TAB_AREA_TOP - 150) * fMinscale;
	}

	Listbox_initEx(&listbox, dwLeft, dwTop, dwWidth, dwHeight, dwItemHeight, 512 * 512);
	//pListBox->controlinfo.sendmessage = ShowIntroduceList_dispatchmsg;
	listbox.showoneitem = Listbox_showoneitem_ex;
	listbox.showitems = Listbox_showitems_ex;
	listbox.controlinfo.sendmessage = aboutlist_dispatchmsg;
	Listbox_SetbkColor(&listbox, 0xFFFFFF, 0xFFFFFF, COLOR_LIST_SELECTED);

	ShowIntroduce(&listbox, &scrollBox, dwLeft, dwTop, dwWidth, dwHeight);
	Window_addctl(&win, (PCONTROL)&listbox);
	Window_addctl(&win, (PCONTROL)&scrollBox);

	win.current = 0;
	win.selected = win.current;

	////Introduce
	//if (enuSelectTab == TAB_INTRODUCE)
	//{
	//	win.current = 0;
	//	win.selected = win.current;

	//	dwLeft = dwBGLeft + ABOUT_PAGE_SUB_TITLE_LEFT * fMinscale;
	//	dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
	//	dwWidth = TAB_CONTENT_WIDTH * fMinscale;
	//	dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
	//	ShowIntroduce(&listbox, &scrollBox, dwLeft, dwTop, dwWidth, dwHeight);
	//	Window_addctl(&win, (PCONTROL)&listbox);
	//	Window_addctl(&win, (PCONTROL)&scrollBox);
	//	//DisplayString(dwLeft, dwTop, COLOR_TEXT_CONTENT, L"Introduce");
	//}
	//else if (enuSelectTab == TAB_OPEN_SOURCE)
	//{
	//	win.current = 1;
	//	win.selected = win.current;

	//	dwLeft = dwBGLeft + ABOUT_PAGE_SUB_TITLE_LEFT * fMinscale;
	//	dwTop = ABOUT_PAGE_TAB_AREA_TOP * fMinscale;
	//	dwWidth = TAB_CONTENT_WIDTH * fMinscale;
	//	dwHeight = ABOUT_PAGE_TAB_AREA_HEIGHT * fMinscale;
	//	ShowOpenSource(&listbox, &scrollBox, dwLeft, dwTop, dwWidth, dwHeight);
	//	Window_addctl(&win, (PCONTROL)&listbox);
	//	Window_addctl(&win, (PCONTROL)&scrollBox);

	//}
	//else if (enuSelectTab == TAB_VERSION)
	//{
	//	win.current = 2;
	//	win.selected = win.current;

	//	dwLeft = (g_WindowWidth - ABOUT_PAGE_TAB_VER_LOGO_SIZE * fMinscale) / 2;
	//	dwTop = ABOUT_PAGE_TAB_VER_LOGO_TOP * fMinscale;
	//	dwWidth = ABOUT_PAGE_TAB_VER_LOGO_SIZE * fMinscale;
	//	dwHeight = ABOUT_PAGE_TAB_VER_LOGO_SIZE * fMinscale;
	//	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, IMAG_FILE_APP_LOGO, FALSE);


	//	dwLeft = (g_WindowWidth - ABOUT_PAGE_TAB_VER_APP_NAME_WIDTH * fMinscale) / 2;
	//	dwTop = ABOUT_PAGE_TAB_VER_APP_NAME_TOP * fMinscale;
	//	StringInfo stStringInfoAPP_NAME;
	//	stStringInfoAPP_NAME.enumFontType = FZLTZHJW;
	//	stStringInfoAPP_NAME.iFontSize = CalcFontSize(50); //(int)(50 * g_data.fFontScale + 0.5);
	//	DisplayStringEx(dwLeft, dwTop, COLOR_DEFAULT_BACKGROUND, STRING_APP_NAME, &stStringInfoAPP_NAME, TRUE);


	//	dwLeft = (g_WindowWidth - ABOUT_PAGE_TAB_VER_VERSION_WIDTH * fMinscale) / 2;
	//	dwTop = ABOUT_PAGE_TAB_VER_VERSION_TOP * fMinscale;
	//	StringInfo stStringInfoCUR_VERSION;
	//	stStringInfoCUR_VERSION.enumFontType = FZLTHJW;
	//	stStringInfoCUR_VERSION.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);
	//	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_CUR_VERSION, &stStringInfoCUR_VERSION, TRUE);

	//	dwLeft = (g_WindowWidth - ABOUT_PAGE_TAB_VER_COPYRIGHT_WIDTH * fMinscale) / 2;
	//	dwTop = ABOUT_PAGE_TAB_COPYRIGHT_TOP * fMinscale;
	//	StringInfo stStringInfoCOPYRIGHT;
	//	stStringInfoCOPYRIGHT.enumFontType = FZLTHJW;
	//	stStringInfoCOPYRIGHT.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);
	//	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_COPYRIGHT, &stStringInfoCOPYRIGHT, TRUE);
	//}
	
	showmouse();

	iRet = Window_run(&win);
	
	Listbox_destroy(&listbox);

	return iRet;
}


int CalculateLineNumber(CHAR16 *msg, StringInfo * pstStringInfo, DWORD dwWidth)
{
	//MyLog(LOG_DEBUG, L"CalculateLineNumber\n");

	int iLine = 1;

	DWORD dwStrLen = 0;
	unsigned int i = 0;

	if (NULL == msg || NULL == pstStringInfo)
	{
		return -1;
	}

	while (i < 10000)
	{
		//	Print(L"process %x ",wstring[i]);
		if (msg[i] == 0)
			break;

		if (msg[i] == 0x0A)
		{
			iLine++;
			dwStrLen = 0;
			i++;
			continue;
		}

		dwStrLen += GetCharWidth(msg[i], pstStringInfo);

		if (dwStrLen >= dwWidth)
		{
			iLine++;
			dwStrLen = 0;
		}

		i++;
	}
	return iLine;
}

////////////////////////////Msg Box////////////////////////////////////
int DrawMsgBoxEx(CHAR16 *msg, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType)
{
	//MyLog(LOG_DEBUG, L"DrawMsgBoxEx Start.\n");
	float fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}

	g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;

	WORD x = (g_WindowWidth - MSG_BOX_WIDTH * fMinscale) / 2;
	WORD y = (g_WindowHeight - MSG_BOX_HEIGHT * fMinscale) / 2;

	STRINGBUTTON buttonctrl[3] = { 0 };
	struct WINDOW win;
	int ret = -1;
	//WORD x1, y1, rows, width;
	BYTE oldstate, bMouseDisplay;
	BUTTON_INFO* btnInfo = NULL;

	//save img
#if OKR_DEBUG_UI
	MyLog(LOG_MESSAGE, L"======-----=== DrawMsgBoxEx.");
#endif

	PBYTE buff = AllocatePool( g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	if (buff == NULL)
	{
		MyLog(LOG_MESSAGE, L"AllocatePool Error.");
	}
	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	hidemouse();
	if (buff != NULL)
	{
		GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);

	Window_init(&win, NULL);
	//if (btnMainBtnId == IDB_REBOOT &&
	//	btnSubBtnId == IDB_SHUTDOWN)
	{
		//MyLog(LOG_DEBUG, L"====Init Reboot/Shutdown Window.====");
		win.windowID = 100;
		g_lastESCTime = mytime(NULL);
		g_lastCountDownTime = mytime(NULL);
	}

	WORD dwLeft = x;
	WORD dwTop = y;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	//circle rect
	//DrawRect(COLOR_EXIT_MSG_BOX_BK, dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale);
	//Linebox(dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale, COLOR_EXIT_MSG_BOX_BK_LINE);
	DrawMsgBoxWhiteBK(dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale);

	dwLeft = x + MSG_TEXT_LEFT * fMinscale;
	dwTop = y + MSG_TEXT_TOP * fMinscale;
	dwWidth = (MSG_BOX_WIDTH - MSG_TEXT_LEFT * 2) * fMinscale;
	dwHeight = MSG_TEXT_HEIGHT * fMinscale;

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_MESSAGE_BOX_TEXT); 

	int iLine = CalculateLineNumber(msg, &stStringInfo, dwWidth);

	dwHeight = (FONT_MESSAGE_BOX_TEXT + MSG_TEXT_LINE_SPACE) * iLine * fMinscale;

	//show msg1
	dwTop = y + MSG_TEXT_TOP * fMinscale + (MSG_TEXT_AREA_HEIGHT * fMinscale - dwHeight) / 2;
	
	DisplayStringInRectEx(dwLeft, dwTop, dwWidth, dwHeight, COLOR_TEXT_CONTENT, msg, &stStringInfo, 0);
	//DisplayString(dwLeft, dwTop, COLOR_TEXT_CONTENT, msg);
	
	dwWidth = MSG_BTN_WIDTH * fMinscale;
	dwHeight = (MSG_BTN_HEIGHT * fMinscale + 1);

	////for test
	//g_data.bIsAutoFunc = TRUE;

	if (btnSubBtnId != -1)
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + MSG_BTN_TOP * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH - MSG_BTN_WIDTH - MSG_BTN_SPAN) * fMinscale;
		dwTop = y + MSG_BTN_TOP * fMinscale;
		btnInfo = FindBtnInfoFromID(btnSubBtnId);
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[2], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		if (g_data.bIsAutoFunc)
		{
			g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
			buttonctrl[0].button.bIsCountDownBtn = TRUE;
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
			buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		}
		else
		{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		}
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = btnSubBtnId;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);

		buttonctrl[2].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[2].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[2]);
	}
	else
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + (MSG_BOX_HEIGHT - MSG_BTN_HEIGHT - MSG_BTN_BOTTOM) * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		if (g_data.bIsAutoFunc)
		{
			g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
			buttonctrl[0].button.bIsCountDownBtn = TRUE;
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
			buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		}
		else
		{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		}
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);
	}

	//win.current = 0;

	dwLeft = x + MSG_TYPE_ICON_LEFT * fMinscale;
	dwTop = y + MSG_TYPE_ICON_TOP * fMinscale;
	dwWidth = MSG_TYPE_ICON_SIZE * fMinscale;
	dwHeight = MSG_TYPE_ICON_SIZE * fMinscale;
	

	if (enumMsgType == TYPE_INFO)
	{
		WORD presspcxInfo = GetImageFileId(IMG_FILE_INFO); //IMG_FILE_INFO;
		//if (0.5 >= fMinscale)
		//{
		//	presspcxInfo = IMG_FILE_INFO_S;
		//}
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxInfo, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_INFO;
		stStringInfoMSG_INFO.enumFontType = FZLTZHJW;
		stStringInfoMSG_INFO.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 8 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_INFO, &stStringInfoMSG_INFO, TRUE);
	}
	else if (enumMsgType == TYPE_WARNING)
	{
		WORD presspcxWarning = GetImageFileId(IMG_FILE_WARNING); //IMG_FILE_WARNING;
		//if (0.5 >= fMinscale)
		//{
		//	presspcxWarning = IMG_FILE_WARNING_S;
		//}
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxWarning, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_WARNING;
		stStringInfoMSG_WARNING.enumFontType = FZLTZHJW;
		stStringInfoMSG_WARNING.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 6 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_WARNING, &stStringInfoMSG_WARNING, TRUE);
	}
	else if (enumMsgType == TYPE_ENQUIRE)
	{
		WORD presspcxEnquire = GetImageFileId(IMG_FILE_ENQUIRE); //IMG_FILE_ENQUIRE;
		//if (0.5 >= fMinscale)
		//{
		//	presspcxEnquire = IMG_FILE_ENQUIRE_S;
		//}
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxEnquire, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_ENQUIRE;
		stStringInfoMSG_ENQUIRE.enumFontType = FZLTZHJW;
		stStringInfoMSG_ENQUIRE.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 6 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_ENQUIRE, &stStringInfoMSG_ENQUIRE, TRUE);
	}

	dwLeft = x + 2;
	dwTop = y + MSG_SPLIT_LINE_TOP * fMinscale;
	dwWidth = (MSG_BOX_WIDTH - 6) * fMinscale;
	dwHeight = 1;
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xDCEFFF);

	showmouse();
	//MyLog(LOG_DEBUG, L"==DrawMsgBoxEx Window====&win: 0x%x", &win);
	ret = Window_run(&win);

	hidemouse();

	if (buff != NULL)
	{
		//restore img
		PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	EnableMouse(oldstate);

	if (bMouseDisplay)
		showmouse();
	//showmouse();

	if (buff != NULL)
	{
		FreePool(buff);
	}
	return ret;
}

////////////////////////////Msg Box////////////////////////////////////
int DrawMsgBoxExWithQRCode(CHAR16 *msg, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType, WORD wQRCodeFileId, WORD wQRCodeFileSize)
{
	//MyLog(LOG_DEBUG, L"DrawMsgBoxEx Start.\n");
	float fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}

	WORD x = (g_WindowWidth - MSG_BOX_WIDTH * fMinscale) / 2;
	WORD y = (g_WindowHeight - MSG_BOX_HEIGHT * fMinscale) / 2;

	STRINGBUTTON buttonctrl[3] = { 0 };
	struct WINDOW win;
	int ret = -1;
	//WORD x1, y1, rows, width;
	BYTE oldstate, bMouseDisplay;
	BUTTON_INFO* btnInfo = NULL;

	g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;

	//save img
#if OKR_DEBUG_UI
	MyLog(LOG_MESSAGE, L"======-----=== DrawMsgBoxEx.");
#endif

	PBYTE buff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	if (buff == NULL)
	{
		MyLog(LOG_MESSAGE, L"AllocatePool Error.");
	}
	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	hidemouse();
	if (buff != NULL)
	{
		GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);

	Window_init(&win, NULL);
	//if (btnMainBtnId == IDB_REBOOT &&
	//	btnSubBtnId == IDB_SHUTDOWN)
	{
		//MyLog(LOG_DEBUG, L"====Init Reboot/Shutdown Window.====");
		win.windowID = 100;
		g_lastESCTime = mytime(NULL);
		//g_lastCountDownTime = mytime(NULL);
	}

	WORD dwLeft = x;
	WORD dwTop = y;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	//circle rect
	//DrawRect(COLOR_EXIT_MSG_BOX_BK, dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale);
	//Linebox(dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale, COLOR_EXIT_MSG_BOX_BK_LINE);
	DrawMsgBoxWhiteBK(dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale);
	
	////show msg
	dwLeft = x + (MSG_TEXT_LEFT + wQRCodeFileSize + 22) * fMinscale;
	dwTop = y + MSG_TEXT_TOP * fMinscale;
	dwWidth = (MSG_BOX_WIDTH - MSG_TEXT_LEFT * 2 - wQRCodeFileSize - 22) * fMinscale;
	dwHeight = MSG_TEXT_HEIGHT * fMinscale;

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_MESSAGE_BOX_TEXT);

	int iLine = CalculateLineNumber(msg, &stStringInfo, dwWidth);
	dwHeight = (FONT_MESSAGE_BOX_TEXT + MSG_TEXT_LINE_SPACE) * iLine * fMinscale;
	dwTop = y + MSG_TEXT_TOP * fMinscale + (MSG_TEXT_AREA_HEIGHT * fMinscale - dwHeight) / 2;

	DisplayStringInRectEx(dwLeft, dwTop, dwWidth, dwHeight, COLOR_TEXT_CONTENT, msg, &stStringInfo, 0);

	////QR Code
	dwLeft = x + MSG_TEXT_LEFT * fMinscale;
	//dwTop = y + MSG_TEXT_TOP * fMinscale + (MSG_TEXT_AREA_HEIGHT * fMinscale - dwHeight) / 2;
	dwTop = y + (MSG_TYPE_ICON_SIZE + MSG_TYPE_ICON_TOP) * fMinscale + (MSG_SPLIT_LINE_TOP - MSG_TYPE_ICON_TOP - MSG_TYPE_ICON_SIZE - wQRCodeFileSize)  * fMinscale / 2;
	dwWidth = wQRCodeFileSize * fMinscale;
	dwHeight = wQRCodeFileSize * fMinscale;
	if (g_WindowWidth == 2560 && g_WindowHeight == 1440)
	{
		dwWidth = 130;
		dwHeight = 130;
	}

	MyLog(LOG_DEBUG, L"DrawMsgBoxExWithQRCode: L:%d,T:%d,W:%d,H:%d.\n", dwLeft, dwTop, dwWidth, dwHeight);

	WORD qrCodeInfo = GetImageFileId(wQRCodeFileId);
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, qrCodeInfo, FALSE);

	////Button
	dwWidth = MSG_BTN_WIDTH * fMinscale;
	dwHeight = (MSG_BTN_HEIGHT * fMinscale + 1);

	///for test
	//g_data.bIsAutoFunc = TRUE;

	if (btnSubBtnId != -1)
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + MSG_BTN_TOP * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH - MSG_BTN_WIDTH - MSG_BTN_SPAN) * fMinscale;
		dwTop = y + MSG_BTN_TOP * fMinscale;
		btnInfo = FindBtnInfoFromID(btnSubBtnId);
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[2], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		//if (g_data.bIsAutoFunc)
		//{
		//	g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
		//	buttonctrl[0].button.bIsCountDownBtn = TRUE;
		//	buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		//	buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		//}
		//else
		//{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		//}
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = btnSubBtnId;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);

		buttonctrl[2].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[2].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[2]);
	}
	else
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + (MSG_BOX_HEIGHT - MSG_BTN_HEIGHT - MSG_BTN_BOTTOM) * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		if (g_data.bIsAutoFunc)
		{
			g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
			buttonctrl[0].button.bIsCountDownBtn = TRUE;
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
			buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		}
		else
		{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		}
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);
	}

	//win.current = 0;

	dwLeft = x + MSG_TYPE_ICON_LEFT * fMinscale;
	dwTop = y + MSG_TYPE_ICON_TOP * fMinscale;
	dwWidth = MSG_TYPE_ICON_SIZE * fMinscale;
	dwHeight = MSG_TYPE_ICON_SIZE * fMinscale;


	if (enumMsgType == TYPE_INFO)
	{
		WORD presspcxInfo = GetImageFileId(IMG_FILE_INFO);
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxInfo, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_INFO;
		stStringInfoMSG_INFO.enumFontType = FZLTZHJW;
		stStringInfoMSG_INFO.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 8 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_INFO, &stStringInfoMSG_INFO, TRUE);
	}
	else if (enumMsgType == TYPE_WARNING)
	{
		WORD presspcxWarning = GetImageFileId(IMG_FILE_WARNING); //IMG_FILE_WARNING;
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxWarning, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_WARNING;
		stStringInfoMSG_WARNING.enumFontType = FZLTZHJW;
		stStringInfoMSG_WARNING.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 6 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_WARNING, &stStringInfoMSG_WARNING, TRUE);
	}
	else if (enumMsgType == TYPE_ENQUIRE)
	{
		WORD presspcxEnquire = GetImageFileId(IMG_FILE_ENQUIRE); //IMG_FILE_ENQUIRE;
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxEnquire, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_ENQUIRE;
		stStringInfoMSG_ENQUIRE.enumFontType = FZLTZHJW;
		stStringInfoMSG_ENQUIRE.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 6 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_ENQUIRE, &stStringInfoMSG_ENQUIRE, TRUE);
	}

	dwLeft = x + 2;
	dwTop = y + MSG_SPLIT_LINE_TOP * fMinscale;
	dwWidth = (MSG_BOX_WIDTH - 6) * fMinscale;
	dwHeight = 1;
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xDCEFFF);

	showmouse();
	//MyLog(LOG_DEBUG, L"==DrawMsgBoxEx Window====&win: 0x%x", &win);
	ret = Window_run(&win);

	hidemouse();

	if (buff != NULL)
	{
		//restore img
		PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	EnableMouse(oldstate);

	if (bMouseDisplay)
		showmouse();
	//showmouse();

	if (buff != NULL)
	{
		FreePool(buff);
	}
	return ret;
}

////////////////////////////DrawMsgBoxMultilLine////////////////////////////////////
int DrawMsgBoxMultilLineEx(CHAR16 *msg, CHAR16 *msg2, CHAR16 *msg3, CHAR16 *msg4, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType)
{
	float fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}
	g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
	WORD x = (g_WindowWidth - MULTI_MSG_BOX_WIDTH * fMinscale) / 2;
	WORD y = (g_WindowHeight - MULTI_MSG_BOX_HEIGHT * fMinscale) / 2;

	STRINGBUTTON buttonctrl[3] = { 0 };
	struct WINDOW win;
	int ret = -1;
	//WORD x1, y1, rows, width;
	BYTE oldstate, bMouseDisplay;
	BUTTON_INFO* btnInfo = NULL;

	//save img
#if OKR_DEBUG_UI //add by wh
	MyLog(LOG_MESSAGE, L"======-----=== DrawExitMsgBox.");

#endif

	PBYTE buff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	if (buff == NULL)
	{
		MyLog(LOG_MESSAGE, L"AllocatePool Error.");
	}
	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	hidemouse();
	if (buff != NULL)
	{
		GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);

	Window_init(&win, NULL);
	win.windowID = 100;
	g_lastESCTime = mytime(NULL);
	g_lastCountDownTime = mytime(NULL);

	WORD dwLeft = x;
	WORD dwTop = y;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	//circle rect
	//DrawRect(COLOR_EXIT_MSG_BOX_BK, dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale);
	//Linebox(dwLeft, dwTop, MSG_BOX_WIDTH * fMinscale, MSG_BOX_HEIGHT * fMinscale, COLOR_EXIT_MSG_BOX_BK_LINE);
	DrawMsgBoxWhiteBK(dwLeft, dwTop, MULTI_MSG_BOX_WIDTH * fMinscale, MULTI_MSG_BOX_HEIGHT * fMinscale);

	dwLeft = x + MSG_TEXT_LEFT * fMinscale;
	dwTop = y + MULTI_MSG_TEXT_TOP * fMinscale;
	//我们取出来的是16点阵的字体

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_MESSAGE_BOX_TEXT); //(int)(FONT_MESSAGE_BOX_TEXT * g_data.fFontScale + 0.5);


	int iNum = 0;
	if (msg != NULL)
	{
		iNum += 1;
	}
	if (msg2 != NULL)
	{
		iNum += 1;
	}
	if (msg3 != NULL)
	{
		iNum += 1;
	}
	if (msg4 != NULL)
	{
		iNum += 1;
	}

	int iLineHeight = MULTI_MSG_TEXT_AREA_HEIGHT / iNum;
	dwWidth = MULTI_MSG_TEXT_AREA_WIDTH * fMinscale;
	if (msg != NULL)
	{
		//PrintMessageInRectForMsgBox(dwLeft, dwTop,
		//	(MSG_BOX_WIDTH - MSG_TEXT_LEFT * 2) * fMinscale, MULTI_MSG_TEXT_HEIGHT * fMinscale,
		//	msg, COLOR_TEXT_CONTENT, &stStringInfo);
		dwLeft = x + MSG_TEXT_LEFT * fMinscale;
		dwTop = y + MULTI_MSG_TEXT_TOP * fMinscale;
		//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, msg, &stStringInfo, TRUE);
		DisplayStringInRectEx(dwLeft, dwTop, dwWidth, iLineHeight, COLOR_TEXT_CONTENT, msg, &stStringInfo, 0);
	}

	if (msg2 != NULL)
	{
		dwLeft = x + MSG_TEXT_LEFT * fMinscale;
		dwTop = y + (MULTI_MSG_TEXT_TOP + iLineHeight) * fMinscale;
		//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, msg2, &stStringInfo, TRUE);
		DisplayStringInRectEx(dwLeft, dwTop, dwWidth, iLineHeight, COLOR_TEXT_CONTENT, msg2, &stStringInfo, 0);
	}

	if (msg3 != NULL)
	{
		dwLeft = x + MSG_TEXT_LEFT * fMinscale;
		dwTop = y + (MULTI_MSG_TEXT_TOP + iLineHeight * 2) * fMinscale;
		//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, msg3, &stStringInfo, TRUE);
		DisplayStringInRectEx(dwLeft, dwTop, dwWidth, iLineHeight, COLOR_TEXT_CONTENT, msg3, &stStringInfo, 0);
	}

	if (msg4 != NULL)
	{
		StringInfo stStringInfoText;
		stStringInfoText.enumFontType = FZLTHJW;
		stStringInfoText.iFontSize = CalcFontSize(30);

		dwLeft = x + MSG_TEXT_LEFT * fMinscale;
		dwTop = y + (MULTI_MSG_TEXT_TOP + iLineHeight * 3) * fMinscale;
		//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, msg4, &stStringInfo, TRUE);
		DisplayStringInRectEx(dwLeft, dwTop, dwWidth, iLineHeight, COLOR_TEXT_CONTENT, msg4, &stStringInfoText, 0);
	}

	dwWidth = MSG_BTN_WIDTH * fMinscale;
	dwHeight = (MSG_BTN_HEIGHT * fMinscale + 1);

	///for test
	//g_data.bIsAutoFunc = TRUE;

	if (btnSubBtnId != -1)
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + MULTI_MSG_BTN_TOP * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH - MSG_BTN_WIDTH - MSG_BTN_SPAN) * fMinscale;
		dwTop = y + MULTI_MSG_BTN_TOP * fMinscale;
		btnInfo = FindBtnInfoFromID(btnSubBtnId);
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[2], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		if (g_data.bIsAutoFunc)
		{
			g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
			buttonctrl[0].button.bIsCountDownBtn = TRUE;
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
			buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		}
		else
		{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		}
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = btnSubBtnId;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);

		buttonctrl[2].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[2].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[2]);
	}
	else
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + (MSG_BOX_HEIGHT - MSG_BTN_HEIGHT - MSG_BTN_BOTTOM) * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		if (g_data.bIsAutoFunc)
		{
			g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
			buttonctrl[0].button.bIsCountDownBtn = TRUE;
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
			buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		}
		else
		{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		}
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);
	}

	//win.current = 0;

	dwLeft = x + MSG_TYPE_ICON_LEFT * fMinscale;
	dwTop = y + MSG_TYPE_ICON_TOP * fMinscale;
	dwWidth = MSG_TYPE_ICON_SIZE * fMinscale;
	dwHeight = MSG_TYPE_ICON_SIZE * fMinscale;

	if (enumMsgType == TYPE_INFO)
	{
		WORD presspcxInfo = GetImageFileId(IMG_FILE_INFO); //IMG_FILE_INFO;
		//if (0.5 >= fMinscale)
		//{
		//	presspcxInfo = IMG_FILE_INFO_S;
		//}
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxInfo, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_INFO;
		stStringInfoMSG_INFO.enumFontType = FZLTZHJW;
		stStringInfoMSG_INFO.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 8 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_INFO, &stStringInfoMSG_INFO, TRUE);
	}
	else if (enumMsgType == TYPE_WARNING)
	{
		WORD presspcxWarning = GetImageFileId(IMG_FILE_WARNING); //IMG_FILE_WARNING;
		//if (0.5 >= fMinscale)
		//{
		//	presspcxWarning = IMG_FILE_WARNING_S;
		//}
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxWarning, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_WARNING;
		stStringInfoMSG_WARNING.enumFontType = FZLTZHJW;
		stStringInfoMSG_WARNING.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 8 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_WARNING, &stStringInfoMSG_WARNING, TRUE);
	}
	else if (enumMsgType == TYPE_ENQUIRE)
	{
		WORD presspcxEnquire = GetImageFileId(IMG_FILE_ENQUIRE); //IMG_FILE_ENQUIRE;
		//if (0.5 >= fMinscale)
		//{
		//	presspcxEnquire = IMG_FILE_ENQUIRE_S;
		//}
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxEnquire, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_ENQUIRE;
		stStringInfoMSG_ENQUIRE.enumFontType = FZLTZHJW;
		stStringInfoMSG_ENQUIRE.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 8 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_ENQUIRE, &stStringInfoMSG_ENQUIRE, TRUE);
	}

	dwLeft = x + 2;
	dwTop = y + MULTI_MSG_SPLIT_LINE_TOP * fMinscale;
	dwWidth = (MSG_BOX_WIDTH - 6) * fMinscale;
	dwHeight = 1;
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xDCEFFF);

	showmouse();

	ret = Window_run(&win);

	hidemouse();
	if (buff != NULL)
	{
		//restore img
		PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	EnableMouse(oldstate);

	if (bMouseDisplay)
		showmouse();
	//showmouse();

	if (buff != NULL)
	{
		FreePool(buff);
	}
	return ret;
}

////////////////////////////DrawMsgBoxTwoLine////////////////////////////////////
//Show the message box. Different String font size.
//
int DrawMsgBoxTwoLine(CHAR16 *msg, CHAR16 *msg2, int btnMainBtnId, int btnSubBtnId, MESSAGE_BOX_TYPE enumMsgType)
{
	float fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}
	g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
	WORD x = (g_WindowWidth - TWO_LINE_MSG_BOX_WIDTH * fMinscale) / 2;
	WORD y = (g_WindowHeight - TWO_LINE_MSG_BOX_HEIGHT * fMinscale) / 2;

	STRINGBUTTON buttonctrl[3] = { 0 };
	struct WINDOW win;
	int ret = -1;
	//WORD x1, y1, rows, width;
	BYTE oldstate, bMouseDisplay;
	BUTTON_INFO* btnInfo = NULL;

	//save img
#if OKR_DEBUG_UI
	MyLog(LOG_MESSAGE, L"======-----=== DrawMsgBoxTwoLine.");
#endif

	PBYTE buff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	if (buff == NULL)
	{
		MyLog(LOG_MESSAGE, L"AllocatePool Error.");
	}
	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	hidemouse();
	if (buff != NULL)
	{
		GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);

	Window_init(&win, NULL);
	win.windowID = 110;
	g_lastESCTime = mytime(NULL);
	g_lastCountDownTime = mytime(NULL);

	WORD dwLeft = x;
	WORD dwTop = y;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	DrawMsgBoxWhiteBK(dwLeft, dwTop, TWO_LINE_MSG_BOX_WIDTH * fMinscale, TWO_LINE_MSG_BOX_HEIGHT * fMinscale);

	dwLeft = x + MSG_TEXT_LEFT * fMinscale;
	dwTop = y + TWO_LINE_MSG_TEXT_TOP * fMinscale;
	dwWidth = (MSG_BOX_WIDTH - MSG_TEXT_LEFT * 2) * fMinscale;
	//dwHeight = MSG_TEXT_HEIGHT * fMinscale;

	///Msg1 height
	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_MESSAGE_BOX_TEXT);
	int iLineMsg1 = CalculateLineNumber(msg, &stStringInfo, dwWidth);
	DWORD dwMsg1Height = (FONT_MESSAGE_BOX_TEXT + MSG_TEXT_LINE_SPACE) * iLineMsg1 * fMinscale;

	///Msg2 height
	StringInfo stStringInfoText;
	stStringInfoText.enumFontType = FZLTHJW;
	stStringInfoText.iFontSize = CalcFontSize(30);
	int iLineMsg2 = CalculateLineNumber(msg2, &stStringInfoText, dwWidth);
	DWORD dwMsg2Height = (30 + MSG_TEXT_LINE_SPACE) * iLineMsg2 * fMinscale;

	DWORD dwSpace = 30 * fMinscale;

	//show msg1
	dwTop = y + TWO_LINE_MSG_TEXT_TOP * fMinscale + (TWO_LINE_MSG_TEXT_AREA_HEIGHT * fMinscale - dwMsg1Height - dwMsg2Height - dwSpace) / 2;
	DisplayStringInRectEx(dwLeft, dwTop, dwWidth, dwMsg1Height, COLOR_TEXT_CONTENT, msg, &stStringInfo, 0);

	///show msg2
	dwTop = y + TWO_LINE_MSG_TEXT_TOP * fMinscale + (TWO_LINE_MSG_TEXT_AREA_HEIGHT * fMinscale - dwMsg1Height - dwMsg2Height - dwSpace) / 2 + dwMsg1Height + dwSpace;
	DisplayStringInRectEx(dwLeft, dwTop, dwWidth, dwMsg2Height, COLOR_TEXT_CONTENT, msg2, &stStringInfoText, 0);

	dwWidth = MSG_BTN_WIDTH * fMinscale;
	dwHeight = (MSG_BTN_HEIGHT * fMinscale + 1);

	///for test
	//g_data.bIsAutoFunc = TRUE;

	if (btnSubBtnId != -1)
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + (TWO_LINE_MSG_BOX_HEIGHT - MSG_BTN_HEIGHT - MSG_BTN_BOTTOM) * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH - MSG_BTN_WIDTH - MSG_BTN_SPAN) * fMinscale;
		dwTop = y + (TWO_LINE_MSG_BOX_HEIGHT - MSG_BTN_HEIGHT - MSG_BTN_BOTTOM) * fMinscale;
		btnInfo = FindBtnInfoFromID(btnSubBtnId);
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[2], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		if (g_data.bIsAutoFunc)
		{
			g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
			buttonctrl[0].button.bIsCountDownBtn = TRUE;
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
			buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		}
		else
		{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		}
		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = btnSubBtnId;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);

		buttonctrl[2].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[2].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[2]);
	}
	else
	{
		btnInfo = FindBtnInfoFromID(btnMainBtnId);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_BTN_RIGHT - MSG_BTN_WIDTH) * fMinscale;
		dwTop = y + (TWO_LINE_MSG_BOX_HEIGHT - MSG_BTN_HEIGHT - MSG_BTN_BOTTOM) * fMinscale;
		Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		btnInfo = FindBtnInfoFromID(IDB_MSG_CLOSE);
		dwLeft = x + (MSG_BOX_WIDTH - MSG_CLOSE_BTN_RIGHT - MSG_CLOSE_BTN_SIZE) * fMinscale;
		dwTop = y + MSG_CLOSE_BTN_TOP * fMinscale;
		dwWidth = MSG_CLOSE_BTN_SIZE * fMinscale;
		dwHeight = MSG_CLOSE_BTN_SIZE * fMinscale;
		Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
			btnInfo->btn_pressedpcx, btnInfo->btn_focuspcx, btnInfo->btn_unfocuspcx, btnInfo->title);

		buttonctrl[0].button.controlinfo.control_id = btnMainBtnId;
		if (g_data.bIsAutoFunc)
		{
			g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
			buttonctrl[0].button.bIsCountDownBtn = TRUE;
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
			buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
		}
		else
		{
			buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		}

		Window_addctl(&win, (PCONTROL)&buttonctrl[0]);

		buttonctrl[1].button.controlinfo.control_id = IDB_MSG_CLOSE;
		buttonctrl[1].button.controlinfo.sendmessage = btn_default;
		Window_addctl(&win, (PCONTROL)&buttonctrl[1]);
	}

	dwLeft = x + MSG_TYPE_ICON_LEFT * fMinscale;
	dwTop = y + MSG_TYPE_ICON_TOP * fMinscale;
	dwWidth = MSG_TYPE_ICON_SIZE * fMinscale;
	dwHeight = MSG_TYPE_ICON_SIZE * fMinscale;


	if (enumMsgType == TYPE_INFO)
	{
		WORD presspcxInfo = GetImageFileId(IMG_FILE_INFO); //IMG_FILE_INFO;

		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxInfo, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_INFO;
		stStringInfoMSG_INFO.enumFontType = FZLTZHJW;
		stStringInfoMSG_INFO.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 8 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_INFO, &stStringInfoMSG_INFO, TRUE);
	}
	else if (enumMsgType == TYPE_WARNING)
	{
		WORD presspcxWarning = GetImageFileId(IMG_FILE_WARNING); //IMG_FILE_WARNING;

		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxWarning, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_WARNING;
		stStringInfoMSG_WARNING.enumFontType = FZLTZHJW;
		stStringInfoMSG_WARNING.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 6 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_WARNING, &stStringInfoMSG_WARNING, TRUE);
	}
	else if (enumMsgType == TYPE_ENQUIRE)
	{
		WORD presspcxEnquire = GetImageFileId(IMG_FILE_ENQUIRE); //IMG_FILE_ENQUIRE;

		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, presspcxEnquire, FALSE);

		dwLeft = x + MSG_TITLE_TEXT_LEFT * fMinscale;
		StringInfo stStringInfoMSG_ENQUIRE;
		stStringInfoMSG_ENQUIRE.enumFontType = FZLTZHJW;
		stStringInfoMSG_ENQUIRE.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
		dwTop = dwTop + 6 * fMinscale;
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_ENQUIRE, &stStringInfoMSG_ENQUIRE, TRUE);
	}

	dwLeft = x + 2;
	dwTop = y + MSG_SPLIT_LINE_TOP_EX * fMinscale;
	dwWidth = (MSG_BOX_WIDTH - 6) * fMinscale;
	dwHeight = 1;
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xDCEFFF);

	showmouse();

	ret = Window_run(&win);

	hidemouse();

	if (buff != NULL)
	{
		//restore img
		PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);
	}

	EnableMouse(oldstate);

	if (bMouseDisplay)
		showmouse();
	//showmouse();

	if (buff != NULL)
	{
		FreePool(buff);
	}
	return ret;
}

///////////////////////////////////////////////////////////MAIN PAGE DEFINE///////////////////////////////////////////////////////////////////////////////////

//主页
//return value: 
//IDCANCEL: exit
//IDB_BACKUP: backup
//IDB_RECOVER: recover
//IDB_ABOUT: about page
int MainFace()
{
	//MyLog(LOG_DEBUG, L"====MainFace.====");

	struct BUTTON buttonctrl[4] = { 0 };
	struct WINDOW win;
	DWORD ret = 0;
	WORD x = 0, y = 0;
	RECT rc;

	g_data.Cmd = USER_COMMAND_FUNCTION_SELECT;
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));

	float fMinscale = g_data.fMinscale;

	//MyLog(LOG_ERROR, L"fMinscale %f\r\n", fMinscale);
	MyLog(LOG_ERROR, L"x %d y %d width %d height %d\r\n", rc.x, rc.y, rc.width, rc.height);

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	x = (g_WindowWidth - (MAIN_PAGE_BACK_ICON_WIDTH + MAIN_PAGE_RECOVERY_ICON_WIDTH + MAIN_PAGE_SPAN_BACK_RECOVERY) * fMinscale) / 2;
	y = g_WindowHeight - (MAIN_PAGE_BACK_ICON_BOTTOM + MAIN_PAGE_BACK_ICON_WIDTH) * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		y = g_WindowHeight - (MAIN_PAGE_BACK_ICON_BOTTOM - 100 + MAIN_PAGE_BACK_ICON_WIDTH) * fMinscale;
	}

	DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;

	///BACKUP
	dwLeft = x;
	dwTop = y;
	dwWidth = MAIN_PAGE_BACK_ICON_WIDTH * fMinscale;
	dwHeight = MAIN_PAGE_BACK_ICON_HEIGHT * fMinscale;
	WORD presspcxBackup = GetImageFileId(IMG_FILE_BTN_BACKUP_PRESS); //IMG_FILE_BTN_BACKUP_PRESS;
	WORD focuspcxBackup = GetImageFileId(IMG_FILE_BTN_BACKUP_FOCUS); //IMG_FILE_BTN_BACKUP_FOCUS;
	WORD unfocuspcxBackup = GetImageFileId(IMG_FILE_BTN_BACKUP_NORMAL); //IMG_FILE_BTN_BACKUP_NORMAL;
	//if (0.5 >= fMinscale)
	//{
	//	presspcxBackup = IMG_FILE_BTN_BACKUP_PRESS_S;
	//	focuspcxBackup = IMG_FILE_BTN_BACKUP_FOCUS_S;
	//	unfocuspcxBackup = IMG_FILE_BTN_BACKUP_NORMAL_S;
	//}
	Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxBackup, focuspcxBackup, unfocuspcxBackup, 2);

	///Restore
	dwLeft = x + (MAIN_PAGE_RECOVERY_ICON_WIDTH + MAIN_PAGE_SPAN_BACK_RECOVERY) * fMinscale;
	dwTop = y;
	dwWidth = MAIN_PAGE_RECOVERY_ICON_WIDTH * fMinscale;
	dwHeight = MAIN_PAGE_RECOVERY_ICON_HEIGHT * fMinscale;
	WORD presspcxRestore = GetImageFileId(IMG_FILE_BTN_RESTORE_PRESS); //IMG_FILE_BTN_RESTORE_PRESS;
	WORD focuspcxRestore = GetImageFileId(IMG_FILE_BTN_RESTORE_FOCUS); //IMG_FILE_BTN_RESTORE_FOCUS;
	WORD unfocuspcxRestore = GetImageFileId(IMG_FILE_BTN_RESTORE_NORMAL); //IMG_FILE_BTN_RESTORE_NORMAL;

	Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxRestore, focuspcxRestore, unfocuspcxRestore, 2);
	
	///About
	dwLeft = g_WindowWidth - (MAIN_PAGE_ABOUT_BTN_RIGHT + MAIN_PAGE_ABOUT_BTN_WIDTH) * fMinscale;
	dwTop = MAIN_PAGE_ABOUT_BTN_TOP * fMinscale;
	dwWidth = MAIN_PAGE_ABOUT_BTN_WIDTH * fMinscale;
	dwHeight = MAIN_PAGE_ABOUT_BTN_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = g_WindowWidth - (MAIN_PAGE_ABOUT_BTN_RIGHT + MAIN_PAGE_ABOUT_BTN_WIDTH) * 0.5;
		dwTop = MAIN_PAGE_ABOUT_BTN_TOP * 0.5;
		dwWidth = MAIN_PAGE_ABOUT_BTN_WIDTH * 0.5;
		dwHeight = MAIN_PAGE_ABOUT_BTN_HEIGHT * 0.5;
	}

	WORD presspcxAbout = GetImageFileId(IMG_FILE_BTN_ABOUT_PRESS); //IMG_FILE_BTN_ABOUT_PRESS;
	WORD focuspcxAbout = GetImageFileId(IMG_FILE_BTN_ABOUT_FOCUS); //IMG_FILE_BTN_ABOUT_FOCUS;
	WORD unfocuspcxAbout = GetImageFileId(IMG_FILE_BTN_ABOUT_UNFOCUS); //IMG_FILE_BTN_ABOUT_UNFOCUS;

	Button_init(&buttonctrl[2], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxAbout, focuspcxAbout, unfocuspcxAbout, 0);

	///Exit
	dwLeft = g_WindowWidth - (MAIN_PAGE_EXIT_BTN_RIGHT + MAIN_PAGE_EXIT_BTN_WIDTH) * fMinscale;
	dwTop = MAIN_PAGE_EXIT_BTN_TOP * fMinscale;
	dwWidth = MAIN_PAGE_EXIT_BTN_WIDTH * fMinscale;
	dwHeight = MAIN_PAGE_EXIT_BTN_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = g_WindowWidth - (MAIN_PAGE_EXIT_BTN_RIGHT + MAIN_PAGE_EXIT_BTN_WIDTH) * 0.5;
		dwTop = MAIN_PAGE_EXIT_BTN_TOP * 0.5;
		dwWidth = MAIN_PAGE_EXIT_BTN_WIDTH * 0.5;
		dwHeight = MAIN_PAGE_EXIT_BTN_HEIGHT * 0.5;
	}

	WORD presspcxExit = GetImageFileId(IMG_FILE_BTN_EXIT_PRESS); //IMG_FILE_BTN_EXIT_PRESS;
	WORD focuspcxExit = GetImageFileId(IMG_FILE_BTN_EXIT_FOCUS); //IMG_FILE_BTN_EXIT_FOCUS;
	WORD unfocuspcxExit = GetImageFileId(IMG_FILE_BTN_EXIT_UNFOCUS); //IMG_FILE_BTN_EXIT_UNFOCUS;

	Button_init(&buttonctrl[3], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxExit, focuspcxExit, unfocuspcxExit, 0);

	buttonctrl[0].controlinfo.control_id = IDB_BACKUP;// backup btn
	buttonctrl[0].controlinfo.sendmessage = btn_default;
	//buttonctrl[0].clear_background = TRUE;

	buttonctrl[1].controlinfo.control_id = IDB_RECOVER;// RESTORE btn
	buttonctrl[1].controlinfo.sendmessage = btn_default;
	//buttonctrl[1].clear_background = TRUE;

	buttonctrl[2].controlinfo.control_id = IDB_ABOUT; // about button
	buttonctrl[2].controlinfo.sendmessage = btn_default;
	//buttonctrl[2].clear_background = TRUE;

	buttonctrl[3].controlinfo.control_id = IDB_EXIT; // exit button
	buttonctrl[3].controlinfo.sendmessage = btn_default;
	//buttonctrl[3].clear_background = TRUE;

	///String info

	Window_init(&win, NULL);
	win.msgfunc = MyWindow_Dispatchmsg;
	win.windowID = 1;

	//ClearBackground();
	hidemouse();

	//Title
	dwLeft = dwBGLeft;// MAIN_PAGE_TITLE_LEFT * fMinscale;
	dwTop = MAIN_PAGE_TITLE_TOP * fMinscale;
	dwWidth = MAIN_PAGE_TITLE_WIDTH * fMinscale;
	dwHeight = MAIN_PAGE_TITLE_HEIGHT * fMinscale;
	WORD unfocuspcxTitle = GetImageFileId(IMG_FILE_BACKGROUND_TITLE); //IMG_FILE_BACKGROUND_TITLE;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft;// MAIN_PAGE_TITLE_LEFT * fMinscale;
		dwTop = (ABOUT_PAGE_TITLE_TOP - 100) * 0.5;
		dwWidth = MAIN_PAGE_TITLE_WIDTH * 0.5;
		dwHeight = MAIN_PAGE_TITLE_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (MAIN_PAGE_TITLE_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxTitle, FALSE); // title.bmp

	DisplayLogo();

	//info text
	dwLeft = (g_WindowWidth - MAIN_PAGE_INFO_WIDTH * fMinscale) / 2;
	dwTop = g_WindowHeight - (MAIN_PAGE_INFO_HEIGHT + MAIN_PAGE_INFO_BOTTOM) * fMinscale;
	dwWidth = MAIN_PAGE_INFO_WIDTH * fMinscale;
	dwHeight = MAIN_PAGE_INFO_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = (g_WindowWidth - MAIN_PAGE_INFO_WIDTH * 0.5) / 2;
		dwTop = g_WindowHeight - (MAIN_PAGE_INFO_HEIGHT + MAIN_PAGE_INFO_BOTTOM) * fMinscale;
		dwWidth = MAIN_PAGE_INFO_WIDTH * 0.5;
		dwHeight = MAIN_PAGE_INFO_HEIGHT * 0.5;
	}

	//DisplayString(dwLeft, dwTop, COLOR_MAIN_PAGE_INFO, STRING_MAIN_PAGE_INFO);
	WORD unfocuspcxFoot = GetImageFileId(IMG_FILE_FOOT); //IMG_FILE_FOOT;
	//if (0.5 >= fMinscale)
	//{
	//	unfocuspcxFoot = IMG_FILE_FOOT_S;
	//}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxFoot, FALSE);

	showmouse();

	Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//backup按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//restore按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//exit按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[3]); 	//about按钮

	if (g_data.bDebug)
		MyLogString(NULL);

	//return Window_run(&win);

	do
	{
		Window_reset(&win);
		ClearKeyboard();

		ret = Window_run(&win);
		//MyLog(LOG_DEBUG, L"==Window_run ret %d\n", ret);
		if (ret == IDB_EXIT || ret == 0)  //0: IDCANCEL
		{
			//if (DrawMsgBoxEx(STRING_REBOOT, IDOK, IDCANCEL, TYPE_ENQUIRE) == IDOK)
			ret = DrawMsgBoxEx(STRING_REBOOT, IDB_REBOOT, IDB_SHUTDOWN, TYPE_ENQUIRE);
			if (ret == IDB_SHUTDOWN || ret == IDB_REBOOT)
			{
				break;
			}
			else //if (iRet == IDB_MSG_CLOSE)
			{
				continue;
			}
		}
		else
		{
			break;
		}
	} while (TRUE);

	MyLog(LOG_DEBUG, L"MainFace ret %d\n", ret);

	return ret;
}

///////////////////////////////////////////////////////////////////////  CHECK BOX
#ifndef SCROLL_DRIVE_LIST

#define WM_DRVBOX_UNSELECT 0xfe80
//#define DRVTOTALWIDTH 460
#define WM_UPDATE_PART 0xfe81

#define DRVICONWIDTH	SEL_BK_PAR_DISK_ITEM_WIDTH	//48//64
#define DRVICONHEIGHT	SEL_BK_PAR_DISK_ITEM_HEIGHT	//48//64

#define REAL_DRVICONWIDTH  SEL_BK_PAR_DISK_ITEM_WIDTH	//48//64
#define REAL_DRVICONHEIGHT SEL_BK_PAR_DISK_ITEM_HEIGHT	//48//64

void Drvbox_show( PCONTROL  pcontrol )
{
    PDRVBOX pcheck=(PDRVBOX)pcontrol;
    WORD icon;
    WORD delta_x, delta_y;
	BYTE bMouseDisplay;
	DWORD txtcolor = COLOR_BLACK;

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

    bMouseDisplay = hidemouse();

    //FillRect(pcontrol->thex+1,pcontrol->they+2,pcontrol->thew-2,pcontrol->theh-4,CHECK_BG_COLOR);
	
	dwHeight = DRVICONHEIGHT * g_data.fMinscale;

	///ACTIVATE:
	if (pcheck->status==1) 
	{
		if (pcheck->ifremove)
		{
			icon = GetImageFileId(IMG_FILE_REMOVEDISK_SEL); //IMG_FILE_REMOVEDISK_SEL;
		}
		else
		{
			icon = GetImageFileId(IMG_FILE_HARDDISK_SEL); //IMG_FILE_HARDDISK_SEL;
		}

		//FillRect(pcontrol->thex,pcontrol->they,pcontrol->thew,DRVICONHEIGHT,FACE_SELECT_COLOR);
		//FillRect(pcontrol->thex, pcontrol->they, pcontrol->thew, (DWORD)(DRVICONHEIGHT * g_data.fMinscale), COLOR_LIST_SELECTED);
		//txtcolor = COLOR_TEXT_CONTENT;// COLOR_WHITE;
		DisplayImgEx(pcontrol->thex, pcontrol->they, pcontrol->thew, dwHeight, IMAG_FILE_DISK_BG_SELECT, FALSE);

		SetTextBKColor(COLOR_LIST_SELECTED);
	}
	else if (pcheck->status == 2)
	{
		////lost focus
		if (pcheck->ifremove)
		{
			icon = GetImageFileId(IMG_FILE_REMOVEDISK_SEL); //IMG_FILE_REMOVEDISK_SEL;
		}
		else
		{
			icon = GetImageFileId(IMG_FILE_HARDDISK_SEL); //IMG_FILE_HARDDISK_SEL;
		}
		//FillRect(pcontrol->thex, pcontrol->they, pcontrol->thew, (DWORD)(DRVICONHEIGHT * g_data.fMinscale), FACE_SELECT_COLOR);	//FACE_BACKGROUND_COLOR
		 //   txtcolor = COLOR_WHITE;
		DisplayImgEx(pcontrol->thex, pcontrol->they, pcontrol->thew, dwHeight, IMAG_FILE_DISK_BG_SELECT, FALSE);
		SetTextBKColor(COLOR_LIST_SELECTED);
	}
	else
	{
		if (pcheck->ifremove)
		{
			icon = GetImageFileId(IMG_FILE_REMOVEDISK);//IMG_FILE_REMOVEDISK;
		}
		else
		{
			icon = GetImageFileId(IMG_FILE_HARDDISK);//IMG_FILE_HARDDISK;
		}

		//status 0: INACTIVATE
		//FillRect(pcontrol->thex, pcontrol->they, pcontrol->thew, dwHeight, COLOR_CLIENT);	//FACE_BACKGROUND_COLOR
		DisplayImgEx(pcontrol->thex, pcontrol->they, pcontrol->thew, dwHeight, IMAG_FILE_DISK_BG_NORMAL, FALSE);
		//txtcolor = COLOR_SEC_BLUE;
	}
	txtcolor = COLOR_TEXT_CONTENT;// COLOR_WHITE;

	dwLeft = pcontrol->thex + SEL_BK_PAR_DISK_ITEM_ICON_SPAN * g_data.fMinscale;
	dwTop = pcontrol->they + SEL_BK_PAR_DISK_ITEM_ICON_SPAN * g_data.fMinscale;
	dwWidth = SEL_BK_PAR_DISK_ITEM_ICON_WIDTH * g_data.fMinscale;
	dwHeight = SEL_BK_PAR_DISK_ITEM_ICON_HEIGHT * g_data.fMinscale;
    DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, icon, TRUE);

   // DisplayStringInWidth(pcontrol->thex+DRVICONWIDTH+16, pcontrol->they+8, 
    //		DRVICONWIDTH+100, txtcolor, pcheck->volname);
    //DisplayString(pcontrol->thex,pcontrol->they+DRVICONHEIGHT,COLOR_BLACK,pcheck->volname);
	
	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_LIST_ITEM_TEXT_SIZE); //(int)(FONT_LIST_ITEM_TEXT_SIZE * g_data.fFontScale);

	dwLeft = pcontrol->thex + SEL_BK_PAR_DISK_ITEM_TEXT_LEFT * g_data.fMinscale;
	dwTop = pcontrol->they + SEL_BK_PAR_DISK_ITEM_ICON_SPAN * g_data.fMinscale;
	dwWidth = SEL_BK_PAR_DISK_ITEM_TEXT_WIDTH * g_data.fMinscale;
	//DisplayString(dwLeft, dwTop, txtcolor, pcheck->volname);
	//PutstrinWidth(dwLeft, dwTop, pcheck->volname, txtcolor, dwWidth, &stStringInfo);
	DisplayStringInWidthEx(dwLeft, dwTop, txtcolor, pcheck->volname, &stStringInfo, dwWidth, 1, TRUE);

	dwTop = pcontrol->they + (SEL_BK_PAR_DISK_ITEM_ICON_SPAN + SEL_BK_PAR_DISK_ITEM_TEXT_HEIGHT + SEL_BK_PAR_DISK_ITEM_TEXT_SPAN) * g_data.fMinscale;
	DisplayStringEx(dwLeft, dwTop, txtcolor, pcheck->capname, &stStringInfo, TRUE);

	if(bMouseDisplay)
		showmouse();
}

int Drvbox_Change(PDRVBOX pcheck, int status)
{
	//MyLog(LOG_DEBUG, L"Drvbox_Change status=%d\n", status);

	PCONTROL pcontrol = (PCONTROL)pcheck;
	int i;
	struct DrvButtonList * plist = pcheck->plist;


	//MyLog(LOG_DEBUG, L"Drvbox, status 0x%x,pcheck->status:%x\n",status,pcheck->status);

	if (pcheck->status == status)
	{
		// if(pcheck->status==1)
		{
			//    pcheck->status=0;
			//    Drvbox_show((PCONTROL)pcheck);
		}
		return 0;
	}
	if (status == 2)
	{
		int tmp = 0;

		//    struct DM_MESSAGE dmessage;
		tmp = pcheck->status;
		pcheck->status = 2;
		Drvbox_show((PCONTROL)pcheck);
		pcheck->status = tmp;

	}
	else if (pcheck->status == 1)
	{
		//unselect

		pcheck->status = 0;
		Drvbox_show((PCONTROL)pcheck);
	}
	else
	{
		struct DM_MESSAGE dmessage;

		pcheck->status = 1;
		Drvbox_show((PCONTROL)pcheck);

		//unselect other box
		for (i = 0; i < plist->num; i++) {
			dmessage.message = WM_DRVBOX_UNSELECT;
			if (pcheck->indexinlist == i)
				continue;
			plist->icons[i].controlinfo.sendmessage(&plist->icons[i].controlinfo, &dmessage);
		}
		plist->selected = pcheck->indexinlist;
		if (plist->bindctl)
		{
			//MyLog(LOG_DEBUG, L"==========Drvbox_Change WM_UPDATE_PART\n");
			dmessage.message = WM_UPDATE_PART;
			dmessage.thex = plist->partindex[pcheck->indexinlist];
			plist->bindctl->sendmessage(plist->bindctl, &dmessage);
		}
	}

	return 0;
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

	SetMem(pcheck->capname, 50, 0);
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

//////////////////////////////////////////////////
INTN AddDiskInfoLine(PLISTBOX plistbox)
{
	CHAR16 buffer[100];
	WORD index = 0;
	WORD i = 0;
	int currentdisk = -1;
	DWORD size = 0;
	CHAR16 name[100] = { 0 };
	
	//BOOL bIsGSKU = IsGSKU();

	SetMem(g_disk_list_info, MAX_DISK_NUM * sizeof(DISK_LIST_INFO), 0);

	for (i = 0; i < g_disknum; i++)
	{
		currentdisk = i;
		size = DivU64x32(g_disklist[currentdisk].totalsector, 2048);

		//if (diskselected == i) //原始位置
		if (g_disklist[currentdisk].curdisktype == DISK_SYSDISK)
		{
			g_disk_list_info[index].disknum = currentdisk;
			g_disk_list_info[index].TotalSectors = g_disklist[currentdisk].totalsector;
			Listbox_Addoneline(plistbox);

			WORD iData = GetImageFileId(IMG_FILE_ORI_DISK);//IMG_FILE_ORI_DISK;
			Listbox_Addoneunit(plistbox, ITEM_TYPE_IMG, &iData);

			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, STRING_ORI_DISK);
			SetMem(buffer, 200, 0);

			SPrint(buffer, 100, L"%s", g_disklist[currentdisk].diskname);
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
			SetMem(buffer, 200, 0);

			if (size > 8 * 1024)
			{
				SPrint(name, 100, L"%d GB", size / 1024);
			}
			else
			{
				SPrint(name, 100, L"%d MB", size / 1024);
			}
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, name);
			SetMem(buffer, 200, 0);

			index++;
		}
	}

	MyLog(LOG_DEBUG, L"sysdisktype_in_backupfile is %d, SkipType: %d\n", sysdisktype_in_backupfile, g_data.enumGSKUSkipType);
	for (i = 0; i < g_disknum; i++) 
	{
		currentdisk = i;
		//MyLog(LOG_DEBUG, L"currentdisk: %d, i: %d, g_disknum: %d\n", currentdisk, i, g_disknum);

		size = DivU64x32(g_disklist[currentdisk].totalsector, 2048);

		if (g_disklist[currentdisk].curdisktype == NEW_FOUND_DISK) 
		{
			//分区大小小于4GB，可以忽略
			if (size < 4 * 1024)
			{
				MyLog(LOG_DEBUG, L"SKIP Small disk. Current Disk: %d\n", currentdisk);
				continue;
			}

			if (g_disklist[currentdisk].disksn[0] == 0x20 || g_disklist[currentdisk].disksn[0] == 0) //sata
			{
				if (DISABLE_NVME_TO_SATA == g_data.enumGSKUSkipType ||
					DISABLE_DIFFERENT_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 1)  //备份disk类型为NVME盘
					{
						MyLog(LOG_DEBUG, L"==SKIP SATA disk. Current Disk: %d\n", currentdisk);
						//i++;
						continue;
					}
				}

				if (DISABLE_SATA_TO_SATA == g_data.enumGSKUSkipType ||
					DISABLE_SAME_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 2)  //备份disk类型为SATA盘
					{
						MyLog(LOG_DEBUG, L"SKIP SATA disk. Current Disk: %d\n", currentdisk);
						//i++;
						continue;
					}
				}

				g_disk_list_info[index].disknum = currentdisk;
				g_disk_list_info[index].TotalSectors = g_disklist[currentdisk].totalsector;

				Listbox_Addoneline(plistbox);

				WORD iData = GetImageFileId(IMG_FILE_NEW_DISK); 
				Listbox_Addoneunit(plistbox, ITEM_TYPE_IMG, &iData);

				Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, STRING_NEW_DISK);
				SetMem(buffer, 200, 0);

				SPrint(buffer, 100, L"%s", g_disklist[currentdisk].diskname);
				Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
				SetMem(buffer, 200, 0);

				if (size > 8 * 1024)
				{
					SPrint(name, 100, L"%d GB", size / 1024);
				}
				else
				{
					SPrint(name, 100, L"%d MB", size / 1024);
				}
				Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, name);
				SetMem(buffer, 200, 0);
				index++;
			}
			else
			{
				//disk is NVME
				if (DISABLE_SATA_TO_NVME == g_data.enumGSKUSkipType ||
					DISABLE_DIFFERENT_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 2)  //备份disk类型为sata盘
					{
						MyLog(LOG_DEBUG, L"==SKIP NVME disk. Current Disk: %d, i: %d\n", currentdisk, i);
						//i++;
						continue;
					}
				}

				if (DISABLE_NVME_TO_NVME == g_data.enumGSKUSkipType ||
					DISABLE_SAME_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 1)  //备份disk类型为NVME盘
					{
						MyLog(LOG_DEBUG, L"SKIP NVME disk. Current Disk: %d\n", currentdisk);
						//i++;
						continue;
					}
				}

				g_disk_list_info[index].disknum = currentdisk;
				g_disk_list_info[index].TotalSectors = g_disklist[currentdisk].totalsector;
				Listbox_Addoneline(plistbox);

				WORD iData = GetImageFileId(IMG_FILE_NEW_DISK); //IMG_FILE_NEW_DISK;
				Listbox_Addoneunit(plistbox, ITEM_TYPE_IMG, &iData);

				Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, STRING_NEW_DISK);
				SetMem(buffer, 200, 0);

				SPrint(buffer, 100, L"%s", g_disklist[currentdisk].diskname);
				Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
				SetMem(buffer, 200, 0);

				if (size > 8 * 1024)
				{
					SPrint(name, 100, L"%d GB", size / 1024);
				}
				else
				{
					SPrint(name, 100, L"%d MB", size / 1024);
				}
				Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, name);
				SetMem(buffer, 200, 0);
				index++;
			}
		}


		if (index >= 16)
		{
			break;
		}
	}

	////添加一些测试数据
//	for (i = 0; i < 20; i++) {
//		SPrint(buffer, 100, L"Item %d", i);
//		Listbox_Addoneline(plistbox);
//		WORD iData = IMG_FILE_NEW_DISK;
//		Listbox_Addoneunit(plistbox, ITEM_TYPE_IMG, &iData);
//		Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
//		Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
//		Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
//	}
//	return i + index;

	return index;
}


//////////////////////////////////////////////////
INTN AddDiskBtnListLine(PLISTBOX plistbox)
{
	WORD index = 0;
	WORD i = 0;
	int currentdisk = -1;
	DWORD size = 0;

	for (i = 0; i < g_disknum; i++)
	{
		currentdisk = i;
		size = DivU64x32(g_disklist[currentdisk].totalsector, 2048);

		//分区大小小于4GB，可以忽略
		if (size < 4 * 1024)
		{
			continue;
		}

		g_seldisk_list_info[index].disknum = currentdisk;
		g_seldisk_list_info[index].TotalSectors = g_disklist[currentdisk].totalsector;
		g_seldisk_list_info[index].bRemovable = g_disklist[currentdisk].removable;

		Listbox_Addoneline(plistbox);

		Listbox_Addoneunit(plistbox, ITEM_TYPE_DISKBTN, &(g_seldisk_list_info[index].disknum));
		//MyLog(LOG_DEBUG, L"Add disk %d\n", g_seldisk_list_info[index].disknum);

		index++;
		if (index >= 16)
		{
			break;
		}
	}

	////添加一些测试数据
	//for (i = 0; i < 5; i++) {
	//	Listbox_Addoneline(plistbox);

	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_DISKBTN, &(g_seldisk_list_info[index].disknum));
	//}
	//MyLog(LOG_DEBUG, L"AddDiskBtnListLine index %d\n", i + index);
	//return i + index;


	MyLog(LOG_DEBUG, L"AddDiskBtnListLine index %d\n", index);
	return index;
}

//////////////////////////////////////////////////////////////////////////// process box
////获取在list列表中显示的分区信息
//struct DrvButtonList* NewDriveButtons(PWINDOW pwin, WORD x, WORD y,
//		WORD width, WORD height, int diskselected)
//{
//    WORD i;
//    BYTE bRemovable = TRUE;
//    int currentdisk = -1;
//    WORD begin_x, begin_y;
//    DWORD size, remain;
//    struct DrvButtonList *buttonlist = AllocateZeroPool(sizeof(struct DrvButtonList));
//	BYTE bMouseDisplay;
//
//    bMouseDisplay = hidemouse();
//    //DrawSinkableRectangle(x, y, width, height, COLOR_SHADOW_GREY, FACE_BACKGROUND_COLOR);
//	//DrawSinkableRectangle(x, y, width, height, COLOR_SHADOW_GREY, COLOR_LIST_BACK_WHITE);
//	DrawSinkableRectangle(x, y, width, height, COLOR_LIST_BACK_WHITE, COLOR_LIST_BACK_WHITE);
//    //FillRect(x,y, width, 333, FACE_BACKGROUND_COLOR);
//	if(bMouseDisplay)
//		showmouse();
//
//    buttonlist->selected=-1;
//    //x+=10;
//    begin_x=x;
//    begin_y=y;
//
//    i = 0;
//	//y += 16;
//
//	DWORD dwItemHeight = (SEL_BK_PAR_DISK_ITEM_HEIGHT + SEL_BK_PAR_DISK_ITEM_SPAN) * g_data.fMinscale;
//
//    while (i < g_disknum) {
//
//		currentdisk = i;
//		if(bRemovable == g_disklist[currentdisk].removable) 
//		{        	
//        	size = DivU64x32(g_disklist[currentdisk].totalsector, 2048);
//			if (size > 4 * 1024) {
//				//分区大小小于4GB，可以忽略
//				WORD status = 0;
//				CHAR16 name[100] = { 0 };
//
//				if (size > 8 * 1024)
//				{
//					SPrint(name, 100, STRING_CAPACITY_GB, size / 1024);
//					//	SPrint(name, 100, L"%d GB %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", size/1024,g_disklist[currentdisk].disksn[0],\
//					//g_disklist[currentdisk].disksn[1],g_disklist[currentdisk].disksn[2],g_disklist[currentdisk].disksn[3],g_disklist[currentdisk].disksn[4],\
//					//g_disklist[currentdisk].disksn[5],g_disklist[currentdisk].disksn[6],g_disklist[currentdisk].disksn[7],g_disklist[currentdisk].disksn[8],\
//					//g_disklist[currentdisk].disksn[9],g_disklist[currentdisk].disksn[10],g_disklist[currentdisk].disksn[11],g_disklist[currentdisk].disksn[12],\
//					//g_disklist[currentdisk].disksn[13],g_disklist[currentdisk].disksn[14],g_disklist[currentdisk].disksn[15],g_disklist[currentdisk].disksn[16],\
//					//g_disklist[currentdisk].disksn[17],g_disklist[currentdisk].disksn[18],g_disklist[currentdisk].disksn[19]);
//				}
//				else
//				{
//					SPrint(name, 100, STRING_CAPACITY_MB, size);
//				}
//				if (diskselected == i) {
//					status = 1;
//					buttonlist->selected = buttonlist->num;
//				}
//
//				Drvbox_init(&buttonlist->icons[buttonlist->num], 
//					x, y, width - 20 * g_data.fMinscale, dwItemHeight,
//					buttonlist, name, g_disklist[currentdisk].diskname,
//					(WORD)buttonlist->num, (WORD)status, 
//					g_disklist[currentdisk].removable);
//
//				buttonlist->partindex[buttonlist->num]=i;
//				buttonlist->num++;
//											   
//				y += dwItemHeight;
//
//				//use scroll
//				//if(y + dwItemHeight * g_data.fMinscale >= begin_y + height)	//超出窗口高度就不要显示了
//				//	break;
//			}
//	    }
//		i++;
//		if(i == g_disknum && bRemovable) {
//			bRemovable = FALSE;
//			i = 0;
//		}
//    }
//
//    for (i=0;i<buttonlist->num;i++) 
//	{
//        Window_addctl(pwin, (PCONTROL)&buttonlist->icons[i]);
//    }
//    return buttonlist;
//}
//
//void DeleteDriveButtons(struct DrvButtonList *p)
//{
//    if (p) FreePool(p);
//}

///////////////////////////////////////////////////////
#endif

/////////////////////////////////////////////////////
// listbox of drives
void btn_dobackup(PCONTROL pcon,struct DM_MESSAGE* msg)
{
    Button_dispatchmsg(pcon,msg); // 默认处理函数
    if (msg->message==WM_BUTTON_CLICK)
    {
   //     if (g_DriveList->selected != -1) {
			//Window_exit(pcon->pwin, pcon->control_id);
   //     }
		Window_exit(pcon->pwin, pcon->control_id);
    }
}

//返回< 0 表示失败，
//返回-1表示没有足够空间，
//    -2表示文件系统初始化失败，
//    -3表示有分区文件系统初始化失败，并且也有分区没有足够空间
//    -4表示没有可以支持的分区
int DetectAvailablePartition(PLISTBOX plistbox, int *partindex)
{
	//UINT32 i;
	//int candidate = -1;
	//ULONGLONG maxfree = 0;
	//BOOL bFileSystemInitFailed = FALSE;
	//BOOL bHasSupportedPartition = FALSE;
	//BOOL bInsufficientSpace = FALSE;

	//MyLog(LOG_DEBUG, L"g_DriveList->selected %d\n", g_DriveList->selected);

	//if(g_DriveList->selected != -1) 
	if (plistbox != NULL && plistbox->whichselect != -1)
	{
		int index = plistbox->firstitem_inwindow + plistbox->whichselect;
		WORD diskindex = g_seldisk_list_info[index].disknum;	// g_DriveList->partindex[g_DriveList->selected];
		//MyLog(LOG_DEBUG, L"DetectAvailablePartition diskindex %d\n", diskindex);
		*partindex = diskindex;
		
		return 1;
	}	
	
	////if (g_DriveList->selected != -1)
	//if (plistbox->whichselect != -1)
	//{
	//	int index = plistbox->firstitem_inwindow + plistbox->whichselect;
	//	WORD diskindex = g_seldisk_list_info[index].disknum; // g_DriveList->partindex[g_DriveList->selected];
	//	FILE_SYS_INFO  sysinfo;
	//	PBYTE buffer;
	//	buffer = myalloc(1024 * 2048);
	//	if (!buffer)
	//	{
	//		MyLog(LOG_DEBUG, L"DetectAvailablePartition, alloc memory failed.\n");
	//		return FALSE;
	//	}
	//	MyLog(LOG_DEBUG, L"disk %d, backupsize 0x%lx\n", diskindex, g_data.TotalBackupSize);

	//	for (i = 0; i < g_dwTotalPartitions; i++) {

	//		if (diskindex != g_Partitions[i].DiskNumber)
	//			continue;
	//		if (DivU64x32(g_Partitions[i].TotalSectors, 2048 * 1024) < 4)	//分区大小小于8GB，可以忽略
	//			continue;
	//		if ((g_Partitions[i].PartitionType != FAT32) && (g_Partitions[i].PartitionType != FAT32_E) && (g_Partitions[i].PartitionType != NTFS))
	//			continue;
	//		if (g_Partitions[i].bIfBackup)
	//			continue;

	//		bHasSupportedPartition = TRUE;
	//		//检查是否可以初始化
	//		if (FileSystemInit(&sysinfo, i, FALSE) == FALSE) {
	//			bFileSystemInitFailed = TRUE;
	//			continue;
	//		}
	//		//判断是否有工厂备份文件
	//		if (HasFactoryBackupInPartition(&sysinfo, i, buffer))
	//		{
	//			//#if OKR_DEBUG //add by wh
	//			//				if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) == 0 || g_Partitions[i].PartitionId == 0x07)
	//			//#else 
	//			if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) == 0 || g_Partitions[i].PartitionId == 0x12)
	//				//#endif
	//			{
	//				FileSystemClose(&sysinfo);
	//				continue;
	//			}
	//		}

	//		FileSystemClose(&sysinfo);

	//		//
	//		if (maxfree < g_Partitions[i].FreeSectors) 
	//		{
	//			maxfree = g_Partitions[i].FreeSectors;
	//			candidate = i;
	//		}

	//		//约0.7的压缩比， 512 / 70% = 731
	//		if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize, 731)) 
	//		{
	//			//if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,512)) {
	//			bInsufficientSpace = TRUE;
	//			continue;
	//		}

	//		//检查是否服务分区
	//		MyLog(LOG_DEBUG, L"Found partition %d, type %x, %x(%d GB)\n",
	//			i, g_Partitions[i].PartitionType, g_Partitions[i].TotalSectors,
	//			DivU64x32(g_Partitions[i].TotalSectors, 2048 * 1024));
	//	}

	//	myfree(buffer);

	//	if (candidate >= 0) {
	//		*partindex = candidate;

	//		//约0.7的压缩比， 512 / 70% = 731
	//		if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize, 731)) {
	//			//if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize,512)){

	//			return candidate;
	//		}
	//	}
	//}

	//

	//if(!bHasSupportedPartition)
	//	return -4;
	//if(bInsufficientSpace && bFileSystemInitFailed)
	//	return -3;
	//if(bFileSystemInitFailed)
	//	return -2;
	return -1;
}

//检查联想服务分区
//成功时，返回联想服务分区的分区index。
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

	if (1)
	{
		WORD diskindex = 0;
		FILE_SYS_INFO  sysinfo;

		MyLog(LOG_DEBUG, L"disk %d, backupsize 0x%lx\n", diskindex, g_data.TotalBackupSize);

		for (i = 0; i < g_dwTotalPartitions; i++)
		{
			MyLog(LOG_DEBUG, L"i %d, Label %s\n", i, g_Partitions[i].Label);

			if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) != 0)
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
			if (DivU64x32(g_Partitions[i].TotalSectors, 2048 * 1024) < 4)	//分区大小小于8GB，可以忽略
				continue;
			if ((g_Partitions[i].PartitionType != FAT32) && (g_Partitions[i].PartitionType != FAT32_E) && (g_Partitions[i].PartitionType != NTFS))
				continue;
			if (g_Partitions[i].bIfBackup)
				continue;

			bHasSupportedPartition = TRUE;
			//检查是否可以初始化
			if (FileSystemInit(&sysinfo, i, FALSE) == FALSE) {
				bFileSystemInitFailed = TRUE;
				continue;
			}
			FileSystemClose(&sysinfo);

			//
			if (maxfree < g_Partitions[i].FreeSectors) {
				maxfree = g_Partitions[i].FreeSectors;
				candidate = i;
			}

			//约0.7的压缩比， 512 / 70% = 731
			if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize, 731)) {
				//if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,512)) {
				bInsufficientSpace = TRUE;
				continue;
			}

			//检查是否服务分区
			MyLog(LOG_DEBUG, L"Found partition %d, type %x, %x(%d GB)\n",
				i, g_Partitions[i].PartitionType, g_Partitions[i].TotalSectors,
				DivU64x32(g_Partitions[i].TotalSectors, 2048 * 1024));
		}

		if (candidate >= 0) {
			*partindex = candidate;

			//约0.7的压缩比， 512 / 70% = 731
			if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize, 731)) {
				//if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize,512)){
				return candidate;
			}
		}
	}

	if (!bHasSupportedPartition)
		return -4;
	if (bInsufficientSpace && bFileSystemInitFailed)
		return -3;
	if (bFileSystemInitFailed)
		return -2;
	return -1;
}

//返回< 0 表示失败，
//返回-1表示没有足够空间，
//    -2表示文件系统初始化失败，
//    -3表示有分区文件系统初始化失败，并且也有分区没有足够空间
//    -4表示没有可以支持的分区
//    -5表示分区是bitlocker分区
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
			if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
			{
				return -5;
			}

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

			//约0.7的压缩比， 512 / 70% = 731
			if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize, 731)) {
			//if (g_Partitions[i].FreeSectors <= DivU64x32(g_data.TotalBackupSize,512)) {
				
				MyLog(LOG_DEBUG,L"FreeSectors = %llx, type %x, TT: %llx, TTShrink: %llx\n", 
				g_Partitions[i].FreeSectors, g_Partitions[i].PartitionType, DivU64x32(g_data.TotalBackupSize, 512), DivU64x32(g_data.TotalBackupSize, 731));
				
				
				bInsufficientSpace = TRUE;
				goto err;
			}

			//检查是否服务分区
			MyLog(LOG_DEBUG,L"Found partition %d, type %x, %x(%d GB)\n", 
				i, g_Partitions[i].PartitionType, g_Partitions[i].TotalSectors,
				DivU64x32(g_Partitions[i].TotalSectors, 2048*1024));
		}

		if(candidate >= 0) {
			MyLog(LOG_DEBUG, L"candidate >= 0\n");
			
			if (g_Partitions[candidate].FreeSectors >= DivU64x32(g_data.TotalBackupSize, 731))	//约0.7的压缩比， 512 / 70% = 731
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
//////////////////////////////////////////////////////////////////////////////////
//窗口消息函数
void SelectBackupPartFace_Dispatchmsg(PWINDOW pwin, struct DM_MESSAGE *dmmessage)
{
	SelectBackupPart_Window_Dispatchmsg(pwin, dmmessage);

	if (pwin->ifexit == 1)
	{
		return;
	}
	//debug 模式热键 ctrl + d
	//MyLog(LOG_ERROR, L"pressed key 0x%x, unicode 0x%x, scancode 0x%x, gpSimpleEx 0x%x, KeyShiftState 0x%x",
	//	dmmessage->message, gKeyData.Key.UnicodeChar, gKeyData.Key.ScanCode, gpSimpleEx, gKeyData.KeyState.KeyShiftState);
	//if (dmmessage->message == CHAR_F1)
	//{
	//	DrawNewCopyrightDlg();
	//}

	//按下alt+x后退出程序，就会退出到shell，而不是重启
	if (IsAltPressed())
	{
		if (dmmessage->message == 'x' || dmmessage->message == 'X')
			g_data.bExitToShell = TRUE;
	}

	if (IsCtrlPressed())
	{
		switch (dmmessage->message)
		{

		case 'd':
		case 'D':
		case 'd' - 0x60:

			g_data.bDebug ^= TRUE;
			if (g_data.bDebug) {
				g_data.LogLevel = LOG_DEBUG;
				MyLog(LOG_MESSAGE, L"debug mode on.");
			}
			else {
				g_data.LogLevel = LOG_ERROR;
				MyLog(LOG_MESSAGE, L"debug mode off.");

				if (g_data.LogY != LOG_GRAPHIC_Y - 20)
					PutImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
				g_data.bRefeshDebugInfo = TRUE;
			}
			break;

		case 'f':
		case 'F':
		case 'f' - 0x60:
		{
			g_data.bForceRestorePartition ^= TRUE;
			MyLog(LOG_MESSAGE, g_data.bForceRestorePartition ? L"ForceRestore mode on" : L"ForceRestore mode off");

			if (TRUE == g_data.bForceRestorePartition)
			{
				DrawMsgBoxTwoLine(STRING_FORCE_RESTORE_MODE_ON, STRING_FORCE_RESTORE_MODE_ON_INFO, IDOK, -1, TYPE_INFO);
			}
			else
			{
				DrawMsgBoxEx(STRING_FORCE_RESTORE_MODE_OFF, IDOK, -1, TYPE_INFO);
			}
		}
		break;

		case 'a':
		case 'A':
		case 'a' - 0x60:
			g_data.bAsyncIO ^= TRUE;
			MyLog(LOG_MESSAGE, L"AsyncIO %s.", g_data.bAsyncIO ? L"on" : L"off");
			break;
		default:
			break;
		}
	}

	//if (dmmessage->message == CHAR_ESC)
	//	Window_exit(pwin, IDCANCEL);
}

int Listbox_showdiskitem(PLISTBOX plistbox, WORD item_index, WORD current_select, DWORD backcolor, DWORD forecolor)
{
	//MyLog(LOG_DEBUG, L"Listbox_showdiskitem index %d sel %d", item_index, current_select);

	//backcolor = COLOR_WHITE;
	//forecolor = COLOR_WHITE;
	BYTE buff[512] = { 0 };

	WORD x = 0, y = 0;
	//WORD width = 0;
	WORD icon;

	//MyLog(LOG_DEBUG, L"item_index(%d), plistbox->item_num(%d)", item_index, plistbox->item_num);
	if (item_index >= plistbox->item_num)
	{
		//MyLog(LOG_DEBUG, L"item_index(%d) > plistbox->item_num(%d)", item_index, plistbox->item_num);
		return 0;
	}

	DWORD txtcolor = COLOR_BLACK;

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	WORD dwSpan = SEL_BK_PAR_DISK_ITEM_SPAN * g_data.fMinscale;

	//MyLog(LOG_DEBUG, L"list num: %d item pre win: %d\n", plistbox->item_num, plistbox->items_per_win);

	x = plistbox->controlinfo.thex - SEL_BK_PAR_DISK_LIST_SCROL_SPACE * g_data.fMinscale;
	y = current_select * plistbox->height_peritem + plistbox->controlinfo.they;

	CopyMem(&buff, plistbox->itemlist[item_index].item_buff, 512 - 4);

	BYTE bMouseDisplay = hidemouse();

	FillRect(x + 1, y + 1, 
		plistbox->controlinfo.thew - 2 - SEL_BK_PAR_DISK_LIST_SCROL_SPACE * g_data.fMinscale, 
		plistbox->height_peritem - dwSpan, COLOR_WHITE);

	WORD* str = (WORD *)buff;
	WORD *ImageID = (WORD *)((PBYTE)str + 4);

	WORD dwDiskNum = *ImageID;

	//MyLog(LOG_DEBUG, L"Get disk: %d\n", dwDiskNum);

	if (dwDiskNum > g_disknum)
	{
		//MyLog(LOG_DEBUG, L"dwDiskNum(%d) > g_disknum(%d)", dwDiskNum, g_disknum);
		return 0;
	}

	//MyLog(LOG_DEBUG, L"backcolor: %x %x\n", backcolor, plistbox->selectItemBkColor);

	dwHeight = SEL_BK_PAR_DISK_ITEM_HEIGHT * g_data.fMinscale;
	if (backcolor == plistbox->selectItemBkColor)
	//if (current_select == plistbox->whichselect)
	{
		if (g_disklist[dwDiskNum].removable)
		{
			icon = GetImageFileId(IMG_FILE_REMOVEDISK_SEL); //IMG_FILE_REMOVEDISK_SEL;
		}
		else
		{
			icon = GetImageFileId(IMG_FILE_HARDDISK_SEL); //IMG_FILE_HARDDISK_SEL;
		}
		DisplayImgEx(x, y, plistbox->controlinfo.thew, dwHeight, IMAG_FILE_DISK_BG_SELECT, FALSE);
		SetTextBKColor(COLOR_LIST_SELECTED);
	}
	else
	{
		if (g_disklist[dwDiskNum].removable)
		{
			icon = GetImageFileId(IMG_FILE_REMOVEDISK);//IMG_FILE_REMOVEDISK;
		}
		else
		{
			icon = GetImageFileId(IMG_FILE_HARDDISK);//IMG_FILE_HARDDISK;
		}

		DisplayImgEx(x, y, plistbox->controlinfo.thew, dwHeight, IMAG_FILE_DISK_BG_NORMAL, FALSE);
	}

	txtcolor = COLOR_TEXT_CONTENT;// COLOR_WHITE;

	dwLeft = x + SEL_BK_PAR_DISK_ITEM_ICON_SPAN * g_data.fMinscale;
	dwTop = y + SEL_BK_PAR_DISK_ITEM_ICON_SPAN * g_data.fMinscale;
	dwWidth = SEL_BK_PAR_DISK_ITEM_ICON_WIDTH * g_data.fMinscale;
	dwHeight = SEL_BK_PAR_DISK_ITEM_ICON_HEIGHT * g_data.fMinscale;
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, icon, TRUE);

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_LIST_ITEM_TEXT_SIZE); //(int)(FONT_LIST_ITEM_TEXT_SIZE * g_data.fFontScale);

	///////////////disk name////////////////////
	CHAR16  volname[50] = { 0 };
	
	if (!g_disklist[dwDiskNum].diskname || g_disklist[dwDiskNum].diskname[0] == 0)
	{
		if (g_disklist[dwDiskNum].removable)
			StrnCpy(volname, STRING_REMOVABLE_DISK, 15);
		else
			StrnCpy(volname, STRING_LOCAL_DISK, 11);
	}
	else
	{
		StrnCpy(volname, g_disklist[dwDiskNum].diskname, sizeof(g_disklist[dwDiskNum].diskname) / sizeof(CHAR16));
	}

	dwLeft = x + SEL_BK_PAR_DISK_ITEM_TEXT_LEFT * g_data.fMinscale;
	dwTop = y + SEL_BK_PAR_DISK_ITEM_ICON_SPAN * g_data.fMinscale;
	dwWidth = SEL_BK_PAR_DISK_ITEM_TEXT_WIDTH * g_data.fMinscale;
	DisplayStringInWidthEx(dwLeft, dwTop, txtcolor, volname, &stStringInfo, dwWidth, 1, TRUE);

	///////////////disk size////////////////////
	DWORD size = DivU64x32(g_disklist[dwDiskNum].totalsector, 2048);
	CHAR16 name[100] = { 0 };

	if (size > 8 * 1024)
	{
		SPrint(name, 100, STRING_CAPACITY_GB, size / 1024);
	}
	else
	{
		SPrint(name, 100, STRING_CAPACITY_MB, size);
	}
	dwTop = y + stStringInfo.iFontSize + (SEL_BK_PAR_DISK_ITEM_ICON_SPAN + SEL_BK_PAR_DISK_ITEM_TEXT_SPAN) * g_data.fMinscale;
	DisplayStringEx(dwLeft, dwTop, txtcolor, name, &stStringInfo, TRUE);

	if (bMouseDisplay)
		showmouse();

	return 0;
}

/////////////////////////////////////////Select Backup Part Face//////////////////////////////////////////////////////////

//return 0 if successfully
//desc: backup name,
//partindex: select par index. -1 not selected
int SelectBackupPartFace(CHAR16 *desc, int *partindex)
{
	*partindex = -1;

	struct BUTTON buttonctrl[3];
	struct WINDOW win;
	DWORD ret = -1;
	EDITBOX editbox;

	UINT64 data = 0;
	CHAR16 space[80] = { 0 };
	//CHAR16 capspace[80] = { 0 };
	CHAR16 descDefault[100] = {0};
	WORD i, x, y;
	RECT rc;
	int result = 0;
	int img_err_count = 0;
	LISTBOX listbox;
	LISTBOX *pListbox;
	SCROLL_BOX ParlistScrollbox;

	TITLEBOX titlebox;
	
	LISTBOX diskListbox;
	SCROLL_BOX diskListScrollbox;

	MyLog(LOG_ERROR, L"-------------SelectBackupPartFace...\n");


	g_data.Cmd = USER_COMMAND_BACKUP_SELECT;

	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	
	result = CheckSystemPartition();
	if (result != 0)
	{
		return result;
	}

	if (g_bIsGetBackupFileInfo)
	{
		//第一次进入备份页面时，调用一次，之后不调用此项

		GetAllBackupFiles(&img_err_count);
		CheckAllBackupFiles();

		g_bIsGetBackupFileInfo = FALSE;
	}

	if (g_data.bHasFactoryBackup < 0) 
	{
		g_data.bHasFactoryBackup = HasFactoryBackup();
	}

	//清除数据
	if (g_ShowOnceWndBuff != NULL)
	{
		FreePool(g_ShowOnceWndBuff);
		g_ShowOnceWndBuff = NULL;
	}

re_init:
	showmouse();

	//SPrint(space, 80, L"%s", STRING_BACKUP_SIZE);

	//show message box
	g_data.TotalBackupSize = GetBackupTotaldata(); 
	data = DivU64x32(g_data.TotalBackupSize, 1024 * 1024);

	if (data > 1024)
	{
		//SPrint(space, 80, STRING_BACKUP_SIZE L" %d.%d GB", data / 1024, (data % 1024) / 10);
		SPrint(space, 80, STRING_BACKUP_SIZE L" %d GB", data / 1462);   //约0.7的压缩比， 1024 / 0.7 = 1462
	}
	else
	{
		SPrint(space, 80, STRING_BACKUP_SIZE L" %d MB", data / 1462);    //约0.7的压缩比， 1024 / 0.7 = 1462
	}

	/////////////////////////UI////////////////////////////////
	float fMinscale = g_data.fMinscale;
	WORD dwLeft = rc.x;
	WORD dwTop = rc.y;
	WORD dwWidth = 0;
	WORD dwHeight = 0;
	int index;
	int disknum = 0;

	DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;
	//DWORD dwBGTop = (g_WindowHeight - WHITE_BG_HEIGHT * fMinscale) / 2;

	dwLeft = dwBGLeft + (WHITE_BG_WIDTH - SEL_BK_PAR_BK_BUTTON_WIDTH - SEL_BK_PAR_BK_BUTTON_RIGHT) * fMinscale;
	dwTop = SEL_BK_PAR_BK_BUTTON_TOP * fMinscale;
	dwWidth = SEL_BK_PAR_BK_BUTTON_WIDTH * fMinscale;
	dwHeight = SEL_BK_PAR_BK_BUTTON_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwHeight = SEL_BK_PAR_BK_BUTTON_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_BK_BUTTON_TOP - 150) * fMinscale;
	}
	Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
		IMG_FILE_BTN_BACKUP1_PRESS, IMG_FILE_BTN_BACKUP1_FOCUS, IMG_FILE_BTN_BACKUP1_NORMAL, 1);


	dwLeft = BACKUP_BUTTON_LEFT * fMinscale;
	dwTop = BACKUP_BUTTON_TOP * fMinscale;
	dwWidth = BACKUP_BUTTON_SIZE * fMinscale;
	dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = BACKUP_BUTTON_LEFT * 0.5;
		dwTop = BACKUP_BUTTON_TOP * 0.5;
		dwWidth = BACKUP_BUTTON_SIZE * 0.5;
		dwHeight = BACKUP_BUTTON_SIZE * 0.5;
	}

	WORD presspcxGoBack = GetImageFileId(IMG_FILE_BUTTON_GO_BACK_PRESS); //IMG_FILE_BUTTON_GO_BACK_PRESS;
	WORD focuspcxGoBack = GetImageFileId(IMAG_FILE_GOBACK_FOCUS); //IMAG_FILE_GOBACK_FOCUS;
	WORD unfocuspcxGoBack = GetImageFileId(IMG_FILE_GOBACK); //IMG_FILE_GOBACK;
	//if (0.5 >= fMinscale)
	//{
	//	presspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_PRESS;
	//	focuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_FOCUS;
	//	unfocuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_UNFOCUS;
	//}
	Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxGoBack, focuspcxGoBack, unfocuspcxGoBack, 0);

	time_t filetime = mytime(NULL);
	struct mytm tm;
	struct mytm *ptm;
	ptm = mygmtime(&filetime, &tm);
	if (ptm)
	{
		if (1 == g_disklist[g_data.BackupDiskNumber].isRAIDDisk)
		{
			SPrint(descDefault, 100, STRING_DEFAULT_BK_NAME_RAID, (UINTN)(1900 + ptm->tm_year), (UINTN)ptm->tm_mon, (UINTN)ptm->tm_mday, (UINTN)ptm->tm_hour, (UINTN)ptm->tm_min);
		}
		else
		{
			SPrint(descDefault, 100, STRING_DEFAULT_BK_NAME, (UINTN)(1900 + ptm->tm_year), (UINTN)ptm->tm_mon, (UINTN)ptm->tm_mday, (UINTN)ptm->tm_hour, (UINTN)ptm->tm_min);
		}
		//MyLog(LOG_DEBUG, L"desc: %s, descDefault:%s\n", desc, descDefault);
		if (StrnCmp(desc, descDefault, 6) == 0)
		{
			SetMem(desc, sizeof(CHAR16) * 100, 0);
		}
		//MyLog(LOG_DEBUG, L"desc: %s, descDefault:%s\n", desc, descDefault);
	}

	dwLeft = dwBGLeft + SEL_BK_PAR_NAME_EDITBOX_LEFT * fMinscale;
	dwTop = SEL_BK_PAR_NAME_EDITBOX_TOP * fMinscale;
	dwWidth = SEL_BK_PAR_NAME_EDITBOX_WIDTH * fMinscale;
	dwHeight = SEL_BK_PAR_NAME_EDITBOX_HEIGHT * fMinscale;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft + (SEL_BK_PAR_NAME_EDITBOX_LEFT + 30) * fMinscale;
		dwWidth = (SEL_BK_PAR_NAME_EDITBOX_WIDTH - 30) * fMinscale;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_NAME_EDITBOX_TOP - 150) * fMinscale;
	}

	Editbox_init(&editbox, dwLeft, dwTop, dwWidth, dwHeight, desc, descDefault, EDIT_MODE_NORMAL, 30);

	//g_editbox = &editbox;

	buttonctrl[0].controlinfo.control_id = IDB_BACKUP;
	buttonctrl[0].controlinfo.sendmessage = btn_dobackup;
	//buttonctrl[0].clear_background = TRUE;

	buttonctrl[1].controlinfo.control_id = IDCANCEL;
	buttonctrl[1].controlinfo.sendmessage = btn_default;

	plistbox = &listbox;
	dwLeft = dwBGLeft + SEL_BK_PAR_PART_LIST_LEFT * fMinscale;
	dwTop = SEL_BK_PAR_PART_LIST_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_PART_LIST_TOP - 150) * fMinscale;
	}
	dwWidth = SEL_BK_PAR_PART_LIST_WIDTH * fMinscale;
	dwHeight = SEL_BK_PAR_PART_LIST_HEIGHT * fMinscale;
	//Listbox_init(&listbox, dwLeft, dwTop, dwWidth, dwHeight, 10, 0x8000);
	Listbox_initEx(&listbox, dwLeft, dwTop, dwWidth, dwHeight, (DWORD)(SEL_BK_PAR_PART_ITEM_HEIGHT * fMinscale), 0x8000);
	listbox.controlinfo.sendmessage = backupPartitionList_dispatchmsg;


	dwLeft = dwBGLeft + SEL_BK_PAR_DISK_LIST_LEFT * fMinscale;
	dwTop = SEL_BK_PAR_DISK_LIST_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_DISK_LIST_TOP - 150) * fMinscale;
	}
	dwWidth = SEL_BK_PAR_DISK_LIST_WIDTH * fMinscale;
	dwHeight = SEL_BK_PAR_DISK_LIST_HEIGHT * fMinscale;
	Listbox_initEx(&diskListbox, dwLeft, dwTop, dwWidth, dwHeight, (DWORD)((SEL_BK_PAR_DISK_ITEM_HEIGHT + SEL_BK_PAR_DISK_ITEM_SPAN) * fMinscale), 0x8000);
	diskListbox.showoneitem = Listbox_showdiskitem;
	Listbox_SetbkColor(&diskListbox, 0xFFFFFF, 0xFFFFFF, COLOR_LIST_SELECTED);
	diskListbox.pBindCtrlListbox = &listbox;
	//diskListbox.controlinfo.sendmessage = backupPartitionList_dispatchmsg;

	//g_pDiskListbox = &diskListbox;

	Window_init(&win, NULL);

	win.msgfunc = SelectBackupPartFace_Dispatchmsg;
	win.windowID = 200;

	hidemouse();

	/////////Draw/////////
	ClearScreen();

	DrawWhiteBK();

	//dwLeft = dwBGLeft + SEL_BK_PAR_DISK_LIST_LEFT * fMinscale;
	//dwTop = SEL_BK_PAR_DISK_LIST_TOP * fMinscale;
	//dwWidth = SEL_BK_PAR_DISK_LIST_WIDTH * fMinscale;
	//dwHeight = SEL_BK_PAR_DISK_LIST_HEIGHT * fMinscale;

	//g_DriveList = NewDriveButtons(&win, dwLeft, dwTop, dwWidth, dwHeight, g_data.BackupDiskNumber);

	////////////////////////////////////////init disk list////////////////////////////////////////
	int iDiskNum = AddDiskBtnListLine(&diskListbox);
	Scrollbox_init(&diskListScrollbox, (PCONTROL)&diskListbox, iDiskNum, diskListbox.items_per_win);
	diskListbox.pscroll = &diskListScrollbox;

	////////////////////////////////////////init part list////////////////////////////////////////
	ret = AddPartListLine(&listbox, disknum);
	Scrollbox_init(&ParlistScrollbox, (PCONTROL)&listbox, ret, plistbox->items_per_win);
	listbox.pscroll = &ParlistScrollbox;
	//g_DriveList->bindctl = &listbox;

	dwWidth = SEL_BK_PAR_PART_LIST_HEAD_WIDTH * fMinscale;

	ONECOLUMN columns[4];

	columns[0].column_name = STRING_LIST_ITEM_BK_VOLUME;
	columns[0].orig_width = dwWidth / 3;

	columns[1].column_name = STRING_LIST_ITEM_BK_TYPE;
	columns[1].orig_width = dwWidth * 2 / 9;

	columns[2].column_name = STRING_LIST_ITEM_BK_TOTAL_SIZE;
	columns[2].orig_width = dwWidth * 2 / 9;

	columns[3].column_name = STRING_LIST_ITEM_BK_REMAIN_SIZE;
	columns[3].orig_width = dwWidth * 2 / 9;
	Titlebox_init(&titlebox, &listbox, dwWidth, columns, 4);

	if (ret == 0)
	{
		buttonctrl[0].controlinfo.status |= CON_STATUS_DISABLE;
		g_isUpdateEmptyPartList = TRUE;
	}

	Window_addctl(&win, (PCONTROL)&listbox);
	Window_addctl(&win, (PCONTROL)&ParlistScrollbox);
	Window_addctl(&win, (PCONTROL)&diskListbox);
	Window_addctl(&win, (PCONTROL)&diskListScrollbox);
	Window_addctl(&win, (PCONTROL)&editbox); 	//EDIT按钮 交换tab顺序 
	Window_addctl(&win, (PCONTROL)&buttonctrl[0]);
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]);
	Window_addctl(&win, (PCONTROL)&titlebox);

	//input sub title
	dwLeft = dwBGLeft + SEL_BK_PAR_SEL_POS_TEXT_LEFT * fMinscale;
	dwTop = SEL_BK_PAR_SEL_POS_TEXT_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_SEL_POS_TEXT_TOP - 150) * fMinscale;
	}
	StringInfo stStringInfoBACKUPHINT;
	stStringInfoBACKUPHINT.enumFontType = FZLTZHJW;
	stStringInfoBACKUPHINT.iFontSize = CalcFontSize(50); //50 * g_data.fFontScale;
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_BACKUPHINT, &stStringInfoBACKUPHINT, TRUE);

	//input name text
	dwLeft = dwBGLeft + SEL_BK_PAR_NAME_TEXT_LEFT * fMinscale;
	dwTop = SEL_BK_PAR_NAME_TEXT_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_NAME_TEXT_TOP - 150) * fMinscale;
	}
	StringInfo stStringInfoINPUTNAME;
	stStringInfoINPUTNAME.enumFontType = FZLTHJW;
	stStringInfoINPUTNAME.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_INPUTNAME, &stStringInfoINPUTNAME, TRUE);

	//size text
	dwLeft = dwBGLeft + SEL_BK_PAR_BK_SIZE_TEXT_LEFT * fMinscale;
	dwTop = SEL_BK_PAR_BK_SIZE_TEXT_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_BK_SIZE_TEXT_TOP - 150) * fMinscale;
	}
	StringInfo stStringInfosizetext;
	stStringInfosizetext.enumFontType = FZLTHJW;
	stStringInfosizetext.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, space, &stStringInfosizetext, TRUE);
	
	//info 1
	dwLeft = dwBGLeft + SEL_BK_PAR_INFO_TEXT_LEFT * fMinscale;
	dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale;
	StringInfo stStringInfoREMOVE_DES;
	stStringInfoREMOVE_DES.enumFontType = FZLTHJW;
	stStringInfoREMOVE_DES.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM - 60) * fMinscale;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150 + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale;
	}

	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_REMOVE_DES, &stStringInfoREMOVE_DES, TRUE);

	dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale + stStringInfoREMOVE_DES.iFontSize / 4;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM - 60) * fMinscale + stStringInfoREMOVE_DES.iFontSize / 4;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150 + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale + stStringInfoREMOVE_DES.iFontSize / 4;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_STAR, &stStringInfoREMOVE_DES, TRUE);

	//info 2
	///Get pre string width
	dwWidth = 0;
	GetStringLen(STRING_REMOVE_DES, &stStringInfoREMOVE_DES, &dwWidth);
	dwLeft = dwBGLeft + dwWidth + (SEL_BK_PAR_INFO_PAR_TEXT_LEFT + SEL_BK_PAR_INFO_TEXT_SPAN) * fMinscale;
	dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale;
	StringInfo stStringInfoPARTITION_TYPE_INFO;
	stStringInfoPARTITION_TYPE_INFO.enumFontType = FZLTHJW;
	stStringInfoPARTITION_TYPE_INFO.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft + SEL_BK_PAR_INFO_TEXT_LEFT * fMinscale;
		dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM - 60 + 10) * fMinscale + stStringInfoPARTITION_TYPE_INFO.iFontSize;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150 + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale;
	}

	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_PARTITION_TYPE_INFO, &stStringInfoPARTITION_TYPE_INFO, TRUE);

	dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale + stStringInfoPARTITION_TYPE_INFO.iFontSize / 4;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft + SEL_BK_PAR_INFO_TEXT_LEFT * fMinscale;
		dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM - 60 + 10) * fMinscale +
			stStringInfoPARTITION_TYPE_INFO.iFontSize + stStringInfoPARTITION_TYPE_INFO.iFontSize / 4;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150 + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale + stStringInfoPARTITION_TYPE_INFO.iFontSize / 4;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_STAR, &stStringInfoPARTITION_TYPE_INFO, TRUE);

	//info 3
	///Get pre string width
	dwWidth = 0;
	GetStringLen(STRING_PARTITION_TYPE_INFO, &stStringInfoPARTITION_TYPE_INFO, &dwWidth);
	dwLeft = dwLeft + dwWidth + SEL_BK_PAR_INFO_TEXT_SPAN * fMinscale;
	dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale;
	StringInfo stStringInfoNoStoreOsPar;
	stStringInfoNoStoreOsPar.enumFontType = FZLTHJW;
	stStringInfoNoStoreOsPar.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft + SEL_BK_PAR_INFO_TEXT_LEFT * fMinscale;
		dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM - 60 + 10 * 2) * fMinscale + 
			stStringInfoPARTITION_TYPE_INFO.iFontSize * 2;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150 + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale;
	}

	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_NO_STORE_OS_PAR, &stStringInfoNoStoreOsPar, TRUE);

	dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale + stStringInfoNoStoreOsPar.iFontSize / 4;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft + SEL_BK_PAR_INFO_TEXT_LEFT * fMinscale;
		dwTop = (WHITE_BG_TOP + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM - 60 + 10 * 2) * fMinscale +
			stStringInfoPARTITION_TYPE_INFO.iFontSize * 2 + stStringInfoPARTITION_TYPE_INFO.iFontSize / 4;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150 + WHITE_BG_HEIGHT - SEL_BK_PAR_INFO_PAR_TEXT_BOTTOM) * fMinscale + stStringInfoNoStoreOsPar.iFontSize / 4;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_STAR, &stStringInfoNoStoreOsPar, TRUE);

	////BTN
	dwLeft = dwBGLeft;
	dwTop = SEL_BK_PAR_TITLE_TOP * fMinscale;
	dwWidth = SEL_BK_PAR_TITLE_WIDTH * fMinscale;
	dwHeight = SEL_BK_PAR_TITLE_HEIGHT * fMinscale;
	WORD unfocuspcxSysBk = GetImageFileId(IMAG_FILE_SYS_BK_TITLE); //IMAG_FILE_SYS_BK_TITLE;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft;
		dwTop = (SEL_BK_PAR_TITLE_TOP - 100) * 0.5;
		dwWidth = SEL_BK_PAR_TITLE_WIDTH * 0.5;
		dwHeight = SEL_BK_PAR_TITLE_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_TITLE_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxSysBk, FALSE);

	dwLeft = dwBGLeft + SEL_BK_PAR_PART_LIST_LEFT * fMinscale;
	dwTop = SEL_BK_PAR_PART_LIST_TOP * fMinscale;
	dwWidth = SEL_BK_PAR_PART_LIST_HEAD_WIDTH * fMinscale;
	dwHeight = SEL_BK_PAR_PART_LIST_HEIGHT * fMinscale;

	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_BK_PAR_PART_LIST_TOP - 150) * fMinscale;
	}
	FillRect(dwLeft, dwTop, dwWidth, dwHeight, COLOR_LIST_BK);
	Linebox(dwLeft, dwTop, dwWidth, dwHeight, COLOR_LIST_SPLIT_LINE);

	DisplayLogo();

	showmouse();

	if (g_data.bDebug)
		MyLogString(NULL);

	do
	{
		Window_reset(&win);

		ret = Window_run(&win);
		///when select backup btn
		if (ret == IDB_BACKUP)
		{
			MyLog(LOG_DEBUG, L"ret == IDB_BACKUP\n");

			//选择磁盘的处理
			int iDiskIndex = -1;
			int destpart = DetectAvailablePartition(&diskListbox, &iDiskIndex);
			MyLog(LOG_DEBUG, L"DetectAvailablePartition destpart %d, iDiskIndex %d\n", destpart, iDiskIndex);

			if (iDiskIndex != -1)
			{
				BOOL bIsAllDataParBitlocker = CheckIfAllDataParBitlocker(iDiskIndex);
				if (bIsAllDataParBitlocker)
				{
					DrawMsgBoxTwoLine(STRING_BITLOCKER_NO_DATA_PAR, STRING_CLOSE_BITLOCKER, IDOK, -1, TYPE_INFO);
					//DrawMsgBoxEx(STRING_BITLOCKER_NO_DATA_PAR, IDOK, -1, TYPE_INFO);
					//Listbox_destroy(&listbox);
					showmouse();
					//goto re_init;
					continue;
				}
			}

			//if (listbox.item_num == 0)
			//{
			//	DrawMsgBoxEx(STRING_NO_DISK_AVAIBLE_HINT, IDOK, -1, TYPE_WARNING);
			//	//Listbox_destroy(&listbox);
			//	showmouse();

			//	//goto re_init;
			//	continue;
			//}

			if (destpart < 0)
			{
				if (destpart == -4)
				{
					DrawMsgBoxEx(STRING_DRIVE_NO_SUPPORT_FS, IDOK, -1, TYPE_INFO);
				}
				else if (destpart == -2)
				{
					//	DrawMsgBoxEx(STRING_FS_INIT_FAILED, IDOK, IDCANCEL, TYPE_INFO);
					//DrawMsgBoxMultilLine(STRING_FS_INIT_FAILED_1, STRING_FS_INIT_FAILED_2, STRING_FS_INIT_FAILED_3, NULL, STRING_YES, NULL);
					DrawMsgBoxEx(STRING_FS_INIT_FAILED_1, IDOK, -1, TYPE_INFO);
				}
				else if (destpart == -1)
				{
					CHAR16 capspacesize[100] = { 0 };
					//SPrint(capspacesize, 100, STRING_NEED_SPACE, data / 1024, (data % 1024) / 10);
					SPrint(capspacesize, 100, STRING_NEED_SPACE, data / 1462);   //约0.7的压缩比， 1024 / 0.7 = 1462
					//DrawMsgBoxEx(capspacesize, IDOK, -1, TYPE_INFO);
					DrawMsgBoxTwoLine(STRING_DISK_TOO_SMALL_TO_BACKUP, capspacesize, IDOK, -1, TYPE_INFO);
				}
				else
				{
					DrawMsgBoxEx(STRING_DRIVE_MIGHT_SMALL, IDOK, -1, TYPE_INFO);
				}

				//ret = IDCANCEL;
				showmouse();

				//Listbox_destroy(&listbox);

				//DeleteDriveButtons(g_DriveList);

				//goto re_init;
				continue;
			}


			//选择分区的判断处理
			index = listbox.firstitem_inwindow + listbox.whichselect;
			*partindex = g_part_list_info[index].partindex;

			int iRetVal = DetectAvailableAssignPartition(*partindex);
			MyLog(LOG_DEBUG, L"DetectAvailablePartition iRetVal %d, *partindex %d\n", iRetVal, *partindex);

			if (iRetVal < 0)
			{
				if (iRetVal == -4)
				{
					CHAR16 capspacesize[100] = { 0 };
					//SPrint(capspacesize, 100, STRING_NEED_SPACE, data / 1024, (data % 1024) / 10);
					SPrint(capspacesize, 100, STRING_NEED_SPACE, data / 1462 );   //约0.7的压缩比， 1024 / 0.7 = 1462
					//DrawMsgBoxEx(capspacesize, IDOK, -1, TYPE_INFO);
					DrawMsgBoxTwoLine(STRING_PART_TOO_SMALL_TO_BACKUP, capspacesize, IDOK, -1, TYPE_INFO);
				}
				else if (iRetVal == -2)
				{
					DrawMsgBoxEx(STRING_FS_INIT_FAILED_1, IDOK, -1, TYPE_INFO);
				}
				else if (iRetVal == -1)
				{
					CHAR16 capspacesize[100] = { 0 };
					//SPrint(capspacesize, 100, STRING_NEED_SPACE, data / 1024, (data % 1024) / 10);
					SPrint(capspacesize, 100, STRING_NEED_SPACE, data / 1462);  //约0.7的压缩比， 1024 / 0.7 = 1462
					//DrawMsgBoxEx(capspacesize, IDOK, -1, TYPE_INFO);
					DrawMsgBoxTwoLine(STRING_PART_TOO_SMALL_TO_BACKUP, capspacesize, IDOK, -1, TYPE_INFO);
				}
				else if (iRetVal == -5)
				{
					DrawMsgBoxTwoLine(STRING_BITLOCKER_SELECT_PAR, STRING_CLOSE_BITLOCKER, IDOK, -1, TYPE_INFO);
				}
				else
				{
					DrawMsgBoxEx(STRING_PART_MIGHT_SMALL, IDOK, -1, TYPE_INFO);
				}
				showmouse();

				//Listbox_destroy(&listbox);

				//DeleteDriveButtons(g_DriveList);

				//goto re_init;
				continue;
			}
			break;
		}
		else
		{
			break;
		}

		MyLog(LOG_DEBUG, L"SelectBackupPartFace ret %d\n", ret);
	} while (TRUE);

	int iStrLen = (WORD)StrLength(editbox.name);
	if (iStrLen <= 0)
	{
		CopyMem(editbox.name, editbox.defaultName, 100);
	}
	
	Listbox_destroy(&listbox);
	Listbox_destroy(&diskListbox);

	//DeleteDriveButtons(g_DriveList);

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

//TODO:
//return 0 if successfully
//desc: backup name,
//partindex: -1 not selected
int InitialBackupFace(int *partindex)
{
    //struct BUTTON buttonctrl[3];
    //struct WINDOW win;
    DWORD ret;
    //EDITBOX editbox;
    //UINT64 data;

    //WORD i, x, y;
    //RECT rc;
    int result = 0;
    
    //CopyMem(&rc, &g_data.rWnd, sizeof(RECT));	

 //   result = ProbeSystemPartition() ;
 //   //if ( result == FALSE)
	//if (result == 0)
 //   {
 //       DrawMsgBoxEx(STRING_HINTNOSYSPART, IDOK, IDCANCEL, TYPE_INFO);
 //       return 3;
 //   }
 //   else if ( result == -1)
 //   {
 //       DrawMsgBox(STRING_FIND_SYS_ERROR,STRING_YES,STRING_NO);
 //       return 3;
 //   }
 //   else if ( result == -2)
 //   {
 //       DrawMsgBoxEx(STRING_FS_INIT_FAILED, IDOK, IDCANCEL, TYPE_INFO);     //系统分区Bitlocker加密提示
 //       return -1;
 //   } 
	//调用InitialBackupFace，没有对返回值3做特殊处理，可以返回-1.
	result = CheckSystemPartition();
	if (result != 0)
	{
		return -1;
	}

	if (g_data.bHasFactoryBackup < 0)
	{
		g_data.bHasFactoryBackup = HasFactoryBackup();
	}
  
   if(g_data.bHasFactoryBackup == TRUE)
   {
  	  return -2;
   }
 
   if (HasSystemPart() == 0 && IsLinuxSystem == FALSE)
   {
	   ////关闭提示框
	   ShowWaitMsgOnce();
	   DrawMsgBoxEx(STRING_HINTNOSYSPART, IDOK, IDCANCEL, TYPE_INFO);

	   return 3;
   }
  
   ///////////////////////////初始备份点做成UI///////////////////////////////////////
   g_data.Cmd = USER_COMMAND_DRIVER_INSTALL;

   g_data.TotalBackupSize = GetBackupTotaldata();

   int destpart = DetectAvailableOEMPartition(partindex);
   if (destpart < 0)
   {
	   //todo: destpart < 0 return???
	   //if (destpart == -4)
		  // //DrawMsgBoxEx(STRING_DRIVE_NO_SUPPORT_FS, IDOK, -1, TYPE_INFO);
		  // ;//The msgbox won't be shown, skip it and the user backup selection page will be shown. 
	   //else if (destpart == -2)
	   //{
		  // DrawMsgBoxEx(STRING_FS_INIT_FAILED_1, IDOK, -1, TYPE_INFO);
	   //}
	   ////else if (destpart == -1)
		  //// DrawMsgBoxEx(STRING_OEM_PART_TOO_SMALL_TO_BACKUP, IDOK, -1, TYPE_INFO);
	   //else
		  // DrawMsgBoxEx(STRING_DRIVE_MIGHT_SMALL, IDOK, -1, TYPE_INFO);

	   //空间不足
	   if (destpart == -1)
	   {
		   ////不显示初始备份提示框，继续进入备份页面。
		   return 4;
	   }
   }
   ////关闭提示框
   ShowWaitMsgOnce();
   ret = RecoverCompleteUIMsgboxEx(USER_COMMAND_INIT_BACKUP, 1, STRING_CREATE_INIT_BK);

   if (ret == IDOK)
   {
	   ret = IDB_IDB_INIT_BACKUP;
   }

//	y = (g_WindowHeight / 3 - 60) /2;
//	if (g_WindowHeight <= 800)
//	{
//		y -= 80;
//	}
//
//	Button_init(&buttonctrl[0], rc.x+rc.width-500, y + 560, 185,60,
//		IMG_FILE_BUTTON_CONFIRM_PRESS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 0);
//	  Button_init(&buttonctrl[1], g_x_begin - 80, y - 5, 64,64,
//		  IMG_FILE_BUTTON_GO_BACK_PRESS, IMAG_FILE_GOBACK_FOCUS, IMG_FILE_GOBACK, 0);
//
//    buttonctrl[0].controlinfo.control_id = IDB_IDB_INIT_BACKUP;//
//    buttonctrl[0].controlinfo.sendmessage = btn_dobackup;
//	  buttonctrl[0].clear_background = TRUE;
//
//    buttonctrl[1].controlinfo.control_id = IDOK;
//    buttonctrl[1].controlinfo.sendmessage = btn_default;
////    buttonctrl[2].controlinfo.control_id = IDCANCEL;
// //   buttonctrl[2].controlinfo.sendmessage = btn_default;
//    
//    Window_init(&win,NULL);
//    win.msgfunc=MyWindow_Dispatchmsg;
//
//    ClearScreen();
//
//
//    Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//确定按钮
//    Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//取消按钮
// //   Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//取消按钮
//
//  //  Window_addctl(&win, (PCONTROL)&editbox); 	//确定按钮
//
//    hidemouse();
//   
//    //DisplayImg(g_x_begin+90, y+10, IMG_FILE_GOBACK,FALSE);
//    
//    DisplayImg(g_x_begin+260+30, y, IMAG_FILE_INITIAL_BK,FALSE);	//
//
//    DisplayImg(rc.x+50, y + 330, IMAG_FILE_INITIAL_BK_TXT,FALSE);	//
//
//
//    showmouse();
//
//	if(g_data.bDebug)
//		MyLogString(NULL);
//
//    ret = Window_run(&win);
//
//  	MyLog(LOG_ERROR,L"InitialBackupFace ret:%x.\n", ret);

   if (ret == IDB_IDB_INIT_BACKUP)
   {
	   //选择的是磁盘，而不是分区，要自动计算哪个分区最合适
	   int destpart = 0;

	   destpart = DetectAvailableOEMPartition(partindex);
	   if (destpart < 0)
	   {
		   if (destpart == -4)
		   {   
			   //DrawMsgBoxEx(STRING_DRIVE_NO_SUPPORT_FS, IDOK, -1, TYPE_INFO);
			   ;//The msgbox won't be shown, skip it and the user backup selection page will be shown. 
		   }
		   else if (destpart == -2)
		   {
			   DrawMsgBoxEx(STRING_FS_INIT_FAILED_1, IDOK, -1, TYPE_INFO);
		   }
		   else if (destpart == -1)
		   {  
			   ;
			   // Check oem part space before popup backup dialog. 
			   // DrawMsgBoxEx(STRING_OEM_PART_TOO_SMALL_TO_BACKUP, IDOK, -1, TYPE_INFO);
		   }
		   else
			   DrawMsgBoxEx(STRING_DRIVE_MIGHT_SMALL, IDOK, -1, TYPE_INFO);
		   ret = IDCANCEL;

		   //if(destpart == -1)
		   if (destpart < 0)
		   {
			   g_data.Cmd = USER_COMMAND_BACKUP_SELECT;
			   return -2;
		   }
	   }
   }
   g_data.Cmd = USER_COMMAND_BACKUP_SELECT;

   //MyLog(LOG_ERROR, L"InitialBackupFace ret:%x.\n", ret);
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
			WORD iData = GetImageFileId(IMG_FILE_BACKUP_POINT); //IMG_FILE_BACKUP_POINT;
			//if (0.5 >= g_data.fMinscale)
			//{
			//	iData = IMG_FILE_BACKUP_POINT_S;
			//}
			Listbox_Addoneunit(plistbox, ITEM_TYPE_IMG, &iData);

			///备份点
			//SPrint(buffer, 100, L"%s " STRING_FACTORY_BACKUP, g_data.images[i].Memo);
			if (g_data.images[i].bFactoryBackup)
			{
				if (StrnCmp(g_data.images[i].Memo, L"Initial Autobackup", 18) == 0)
				{
					if (TRUE == IsRAIDBackupFile(i))
					{
						CopyMem(buffer, STRING_AUTO_FACTORY_BACKUP_RAID, sizeof(STRING_AUTO_FACTORY_BACKUP_RAID));
					}
					else
					{
						CopyMem(buffer, STRING_AUTO_FACTORY_BACKUP, sizeof(STRING_AUTO_FACTORY_BACKUP));
					}
				}
				else if (StrnCmp(g_data.images[i].Memo, L"Initial backup.", 15) == 0)
				{
					if (TRUE == IsRAIDBackupFile(i))
					{
						CopyMem(buffer, STRING_FACTORY_BACKUP_RAID, sizeof(STRING_FACTORY_BACKUP_RAID));
					}
					else
					{
						CopyMem(buffer, STRING_FACTORY_BACKUP, sizeof(STRING_FACTORY_BACKUP));
					}
				}
				else
				{
					if (TRUE == IsRAIDBackupFile(i))
					{
						CopyMem(buffer, STRING_INIT_BACKUP_RAID, sizeof(STRING_INIT_BACKUP_RAID));
					}
					else
					{
						CopyMem(buffer, STRING_INIT_BACKUP, sizeof(STRING_INIT_BACKUP));
					}
				}
			}
			else
			{
				if (StrnCmp(g_data.images[i].Memo, L"Autobackup", 10) == 0)
				{
					//Autobackup.
					CopyMem(buffer, STRING_AUTO_BACKUP, sizeof(STRING_AUTO_BACKUP));
				}
				else
				{
					CopyMem(buffer, g_data.images[i].Memo, 100);
				}
			}
			Listbox_Addoneunit(plistbox,ITEM_TYPE_CHAR_STRING,buffer);
			SetMem(buffer, 200, 0);

			///备份时间
			SPrint(buffer, 100, L"%02d/%02d/%02d %02d:%02d:%02d", (UINTN)(1900 + ptm->tm_year), (UINTN)ptm->tm_mon, (UINTN)ptm->tm_mday, (UINTN)ptm->tm_hour, (UINTN)ptm->tm_min, (UINTN)ptm->tm_sec);
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
			SetMem(buffer, 200, 0);

			int iFileSize = ((g_data.images[i].FileSize / 1024 / 1024) % 1024) / 10;

			while (0 == iFileSize % 10)
			{
				iFileSize = iFileSize / 10;
				if (0 == iFileSize)
				{
					break;
				}
			}

			///备份镜像大小
			SPrint(buffer, 100, L"%d.%d GB", g_data.images[i].FileSize / 1024 / 1024 / 1024, iFileSize);

			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
			SetMem(buffer, 200, 0);
		}
    }

	////添加一些测试数据
	//for (i = 0; i < 6; i++) {
	//	SPrint(buffer, 100, L"Item %d", i);
	//	Listbox_Addoneline(plistbox);
	//	WORD iData1 = IMG_FILE_BACKUP_POINT;
	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_IMG, &iData1);
	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
	//}
	//return i + g_data.ImageFilesNumber;

	//MyLog(LOG_DEBUG, L"AddBackupLine iRet %d.\n", g_data.ImageFilesNumber);
    return g_data.ImageFilesNumber;
}

void recoverlist_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage)
{
	//MyLog(LOG_DEBUG, L"recoverlist_dispatchmsg\n");
    PLISTBOX pListbox=(PLISTBOX)pcontrol;
    Listbox_DispatchMsg(pcontrol, dmessage);

    if (dmessage->message== WM_UPDATE_RECOVERY_LIST_AFTER_DELETE)
	{
		//MyLog(LOG_DEBUG, L"WM_UPDATE_RECOVERY_LIST_AFTER_DELETE\n");
		int img_err_count = 0;
		int img_check_num = 0;

		GetAllBackupFiles(&img_err_count);
		img_check_num = CheckAllBackupFiles();

		hidemouse();

		DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * g_data.fMinscale) / 2;
		WORD dwLeft = dwBGLeft + SEL_REC_POINT_LIST_LEFT * g_data.fMinscale;
		WORD dwTop = SEL_REC_POINT_LIST_TOP * g_data.fMinscale;
		WORD dwWidth = (SEL_REC_POINT_LIST_WIDTH + SEL_REC_POINT_LIST_SCROLL_WIDTH) * g_data.fMinscale;
		WORD dwHeight = SEL_REC_POINT_LIST_HEIGHT * g_data.fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (SEL_REC_POINT_LIST_TOP - 150) * g_data.fMinscale;
		}
		FillRect(dwLeft + 1, dwTop + 1, dwWidth - 2, dwHeight - 2, COLOR_LIST_BK);

		//MyLog(LOG_DEBUG, L"img_check_num %d %d to restore.\n", img_check_num,img_err_count);    
        Listbox_delallitem(pListbox);
		showmouse();

		int iRet = AddBackupLine(pListbox);
		Scrollbox_init(pListbox->pscroll, pListbox, iRet, plistbox->items_per_win);

		PWINDOW pwin = pListbox->controlinfo.pwin;
		PCONTROL pctlBackupBtn = NULL;
		for (int i = 0; i < pwin->control_num; i++)
		{
			if (pwin->control_list[i]->control_id == IDB_DELETE)
			{
				pctlBackupBtn = pwin->control_list[i];

				if (iRet == 0)
				{
					if (!(pctlBackupBtn->status & CON_STATUS_DISABLE))
					{
						Control_disable(pctlBackupBtn);
					}
				}
				else
				{
					if (pctlBackupBtn->status & CON_STATUS_DISABLE)
					{
						Control_enable(pctlBackupBtn);
					}
				}

				break;
			}
		}

		if (iRet > 0)
		{
			Listbox_updateview(pListbox);
		}
		else
		{
			g_isUpdateEmptyRecoveryPointList = TRUE;
		}
		
		pctlBackupBtn = NULL;
    }

	if (g_isUpdateEmptyRecoveryPointList && pListbox->item_num == 0)
	{
		//MyLog(LOG_DEBUG, L"WM_UPDATE_RECOVERY_LIST_AFTER_DELETE\n");
		DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * g_data.fMinscale) / 2;
		DWORD dwBGTop = SEL_REC_POINT_LIST_TOP * g_data.fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwBGTop = (SEL_REC_POINT_LIST_TOP - 150) * g_data.fMinscale;
		}
		WORD dwLeft = dwBGLeft + SEL_REC_POINT_LIST_LEFT * g_data.fMinscale;
		WORD dwTop = dwBGTop;
		WORD dwWidth = (SEL_REC_POINT_LIST_WIDTH + SEL_REC_POINT_LIST_SCROLL_WIDTH) * g_data.fMinscale;
		WORD dwHeight = SEL_REC_POINT_LIST_HEIGHT * g_data.fMinscale;

		hidemouse();
		FillRect(dwLeft + 1, dwTop + 1, dwWidth - 2, dwHeight - 2, COLOR_LIST_GRAY);

		StringInfo stStringInfo;
		stStringInfo.enumFontType = FZLTHJW;
		stStringInfo.iFontSize = CalcFontSize(30); //30 * g_data.fFontScale;
		DWORD dwStrWidth = 0;
		GetStringLen(STRING_NO_BACKUP_FILE, &stStringInfo, &dwStrWidth);

		dwLeft = dwBGLeft + ((SEL_REC_POINT_LIST_WIDTH + SEL_REC_POINT_LIST_SCROLL_WIDTH) * g_data.fMinscale - dwStrWidth) / 2;
		dwTop = dwBGTop + (SEL_REC_POINT_LIST_HEIGHT * g_data.fMinscale - stStringInfo.iFontSize) / 2;
		
		BOOL bIfExistBitlocker = IfExistBitlocker();
		if (bIfExistBitlocker && (0 == g_data.ImageFilesNumber))
		{
			dwTop = dwBGTop + (SEL_REC_POINT_LIST_HEIGHT * g_data.fMinscale - stStringInfo.iFontSize * 3 + 10 * 2 * g_data.fMinscale) / 2;
			SetTextBKColor(COLOR_LIST_GRAY);
			DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_NO_BACKUP_FILE, &stStringInfo, TRUE);

			if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
			{
				dwTop = dwTop + stStringInfo.iFontSize + 10 * g_data.fMinscale;
				dwStrWidth = 0;
				GetStringLen(STRING_BITLOCKER_EXIST_BACKUPFILE_1_1, &stStringInfo, &dwStrWidth);
				dwLeft = dwBGLeft + (dwWidth - dwStrWidth) / 2;
				SetTextBKColor(COLOR_LIST_GRAY);
				DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_BITLOCKER_EXIST_BACKUPFILE_1_1, &stStringInfo, TRUE);

				dwTop = dwTop + stStringInfo.iFontSize + 10 * g_data.fMinscale;
				dwStrWidth = 0;
				GetStringLen(STRING_BITLOCKER_EXIST_BACKUPFILE_1_2, &stStringInfo, &dwStrWidth);
				dwLeft = dwBGLeft + (dwWidth - dwStrWidth) / 2;
				SetTextBKColor(COLOR_LIST_GRAY);
				DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_BITLOCKER_EXIST_BACKUPFILE_1_2, &stStringInfo, TRUE);

				dwTop = dwTop + stStringInfo.iFontSize + 10 * g_data.fMinscale;
				dwStrWidth = 0;
				GetStringLen(STRING_BITLOCKER_EXIST_BACKUPFILE_2, &stStringInfo, &dwStrWidth);
				dwLeft = dwBGLeft + (dwWidth - dwStrWidth) / 2;
				SetTextBKColor(COLOR_LIST_GRAY);
				DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_BITLOCKER_EXIST_BACKUPFILE_2, &stStringInfo, TRUE);

				///////clean the area
				dwLeft = dwBGLeft + SEL_REC_POINT_INFO_TEXT_LEFT * g_data.fMinscale;
				dwTop = (SEL_REC_POINT_INFO_TEXT_TOP + 2) * g_data.fMinscale;
				dwWidth = SEL_REC_POINT_INFO_TEXT_WIDTH * g_data.fMinscale;
				dwHeight = (SEL_REC_POINT_REC_BUTTON_TOP - SEL_REC_POINT_INFO_TEXT_TOP - 4) * g_data.fMinscale;

				DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

				dwLeft = dwBGLeft + SEL_REC_POINT_INFO_TEXT_LEFT * g_data.fMinscale;
				dwTop = (SEL_REC_POINT_INFO_TEXT_TOP + 2) * g_data.fMinscale;
				dwWidth = (SEL_REC_POINT_DEL_BUTTON_LEFT - dwLeft - 5) * g_data.fMinscale;
				dwHeight = (SEL_REC_POINT_DEL_BUTTON_HEIGHT + SEL_REC_POINT_DEL_BUTTON_TOP - SEL_REC_POINT_INFO_TEXT_TOP - 4) * g_data.fMinscale;

				DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);
			}
			else
			{
				dwTop = dwTop + stStringInfo.iFontSize + 10 * g_data.fMinscale;

				dwStrWidth = 0;
				GetStringLen(STRING_BITLOCKER_EXIST_BACKUPFILE_1, &stStringInfo, &dwStrWidth);
				dwLeft = dwBGLeft + (dwWidth - dwStrWidth) / 2;

				SetTextBKColor(COLOR_LIST_GRAY);
				DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_BITLOCKER_EXIST_BACKUPFILE_1, &stStringInfo, TRUE);

				dwTop = dwTop + stStringInfo.iFontSize + 10 * g_data.fMinscale;

				dwStrWidth = 0;
				GetStringLen(STRING_BITLOCKER_EXIST_BACKUPFILE_2, &stStringInfo, &dwStrWidth);
				dwLeft = dwBGLeft + (dwWidth - dwStrWidth) / 2;

				SetTextBKColor(COLOR_LIST_GRAY);
				DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_BITLOCKER_EXIST_BACKUPFILE_2, &stStringInfo, TRUE);

				///////clean the area
				dwLeft = dwBGLeft + SEL_REC_POINT_INFO_TEXT_LEFT * g_data.fMinscale;
				dwTop = (SEL_REC_POINT_INFO_TEXT_TOP + 2) * g_data.fMinscale;
				dwWidth = SEL_REC_POINT_INFO_TEXT_WIDTH * g_data.fMinscale;
				dwHeight = (SEL_REC_POINT_REC_BUTTON_TOP - SEL_REC_POINT_INFO_TEXT_TOP - 4) * g_data.fMinscale;

				DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);
			}
		}
		else
		{
			dwTop = dwBGTop + (SEL_REC_POINT_LIST_HEIGHT * g_data.fMinscale - stStringInfo.iFontSize) / 2;
			SetTextBKColor(COLOR_LIST_GRAY);
			DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_NO_BACKUP_FILE, &stStringInfo, TRUE);
		}

		showmouse();
		g_isUpdateEmptyRecoveryPointList = FALSE;
	}
}

void backupPartitionList_dispatchmsg(PCONTROL pcontrol, struct DM_MESSAGE *dmessage)
{
	PLISTBOX pListbox = (PLISTBOX)pcontrol;
	Listbox_DispatchMsg(pcontrol, dmessage);
	if (dmessage->message == WM_UPDATE_PART || dmessage->message == WM_UPDATE_PART_LIST)
	{
		//MyLog(LOG_DEBUG, L"WM_UPDATE_PART disk num %d ", dmessage->thex);
		hidemouse();

		DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * g_data.fMinscale) / 2;
		DWORD dwLeft = dwBGLeft + SEL_BK_PAR_PART_LIST_LEFT * g_data.fMinscale;
		DWORD dwTop = SEL_BK_PAR_PART_LIST_TOP * g_data.fMinscale;
		DWORD dwWidth = (SEL_BK_PAR_PART_LIST_WIDTH + SEL_BK_PAR_SCROLL_WIDTH) * g_data.fMinscale;
		DWORD dwHeight = SEL_BK_PAR_PART_LIST_HEIGHT * g_data.fMinscale;

		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (SEL_BK_PAR_PART_LIST_TOP - 150) * g_data.fMinscale;
		}
		FillRect(dwLeft + 1, dwTop + 1, dwWidth - 2, dwHeight - 2, COLOR_LIST_BK);

		Listbox_delallitem(pListbox);
		int iRet = AddPartListLine(pListbox, dmessage->thex);
		Scrollbox_init(pListbox->pscroll, pListbox, iRet, plistbox->items_per_win);
		showmouse();

		PWINDOW pwin = pListbox->controlinfo.pwin;
		PCONTROL pctlBackupBtn = NULL;
		for (int i = 0; i < pwin->control_num; i++)
		{
			if (pwin->control_list[i]->control_id == IDB_BACKUP)
			{
				pctlBackupBtn = pwin->control_list[i];

				if (iRet == 0)
				{
					if (!(pctlBackupBtn->status & CON_STATUS_DISABLE))
					{
						Control_disable(pctlBackupBtn);
					}
				}
				else
				{
					if (pctlBackupBtn->status & CON_STATUS_DISABLE)
					{
						Control_enable(pctlBackupBtn);
					}
				}

				break;
			}
		}

		if (iRet > 0)
		{
			Listbox_updateview(pListbox);			
		}
		else
		{
			g_isUpdateEmptyPartList = TRUE;
		}

		pctlBackupBtn = NULL;
	}

	if (g_isUpdateEmptyPartList && pListbox->item_num == 0)
	{
		hidemouse();
		DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * g_data.fMinscale) / 2;
		DWORD dwLeft = dwBGLeft + SEL_BK_PAR_PART_LIST_LEFT * g_data.fMinscale;
		DWORD dwTop = SEL_BK_PAR_PART_LIST_TOP * g_data.fMinscale;
		DWORD dwWidth = (SEL_BK_PAR_PART_LIST_WIDTH + SEL_BK_PAR_SCROLL_WIDTH) * g_data.fMinscale;
		DWORD dwHeight = SEL_BK_PAR_PART_LIST_HEIGHT * g_data.fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (SEL_BK_PAR_PART_LIST_TOP - 150) * g_data.fMinscale;
		}
		FillRect(dwLeft + 1, dwTop + 1, dwWidth - 2, dwHeight - 2, COLOR_LIST_GRAY);

		StringInfo stStringInfoINPUTNAME;
		stStringInfoINPUTNAME.enumFontType = FZLTHJW;
		stStringInfoINPUTNAME.iFontSize = CalcFontSize(30); //30 * g_data.fFontScale;
		DWORD dwStrWidth = 0;
		GetStringLen(STRING_NO_DISK_AVAIBLE_HINT, &stStringInfoINPUTNAME, &dwStrWidth);

		dwLeft = dwLeft + ((SEL_BK_PAR_PART_LIST_WIDTH + SEL_BK_PAR_SCROLL_WIDTH) * g_data.fMinscale - dwStrWidth) / 2;
		dwTop = dwTop + (SEL_BK_PAR_PART_LIST_HEIGHT * g_data.fMinscale - stStringInfoINPUTNAME.iFontSize) / 2;

		SetTextBKColor(COLOR_LIST_GRAY);
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_NO_DISK_AVAIBLE_HINT, &stStringInfoINPUTNAME, TRUE);

		showmouse();
		g_isUpdateEmptyPartList = FALSE;
	}

}

void btn_dorecover(PCONTROL pcon, struct DM_MESSAGE* msg)
{
	Button_dispatchmsg(pcon, msg); // 默认处理函数

//MyLog(LOG_ERROR, L"selected %d\n", plistbox->firstitem_inwindow + plistbox->whichselect);

	if (msg->message == WM_BUTTON_CLICK) {

		if (pcon->control_id == IDB_DELETE) {
			int index = plistbox->firstitem_inwindow + plistbox->whichselect;
			if (!g_data.images[index].bFactoryBackup) {
				//delete image file.
				FILE_SYS_INFO  sysinfo;
				WORD partindex = (WORD)g_data.images[index].PartitionIndex;

				//需要弹出提示要求确认
				if (DrawMsgBoxEx(STRING_DELETE_CONFIRM, IDOK, IDCANCEL, TYPE_ENQUIRE) == IDOK) {
					if (FileSystemInit(&sysinfo, partindex, FALSE) != FALSE) {
						CHAR16 name[100];
						DWORD count = g_data.images[index].FileNumber;
						DWORD i;
						BOOL bDeleted = FALSE;
						if (!count)
							count = 1;
						for (i = 0; i < count; i++)
						{
							GetBackupFileFullPath(&sysinfo, partindex, g_data.images[index].FileName, i, name, 100);
							//if (i == 0)
							//{
							//	SPrint(name, 100, BACKUP_DIR L"\\%s", g_data.images[index].FileName);
							//	SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%s", g_data.images[index].FileName);
							//}
							//else
							//{
							//	SPrint(name, 100, BACKUP_DIR L"\\%s.%d", g_data.images[index].FileName, i);
							//	SPrint(name, 100, BACKUP_DIR_CUSTOMER L"\\%s.%d", g_data.images[index].FileName, i);
							//}
							bDeleted = FileDelete(&sysinfo, name);
						}
						if (bDeleted) {
							//删除备份点以后，就要增加这个文件所在的分区的剩余空间
							g_Partitions[partindex].FreeSectors += g_data.images[index].FileSize / SECTOR_SIZE;
						}

						FileSystemClose(&sysinfo);

						if (bDeleted == FALSE)
						{
							//   DrawMsgBox(STRING_NTFS_DELET_ERROR, STRING_YES, NULL);
							//DrawMsgBoxMultilLine(STRING_NTFS_DELET_ERROR_1,STRING_NTFS_DELET_ERROR_2,STRING_NTFS_DELET_ERROR_3,NULL,STRING_YES, NULL);
							DrawMsgBoxEx(STRING_NTFS_DELET_ERROR_1, IDOK, -1, TYPE_INFO);
						}
						else
						{
							struct DM_MESSAGE dmessage;
							dmessage.message = WM_UPDATE_RECOVERY_LIST_AFTER_DELETE;
							dmessage.thex = plistbox->whichselect;

							plistbox->controlinfo.sendmessage(plistbox, &dmessage);
							//Listbox_updateview(plistbox);
						}

						//Window_exit(pcon->pwin, pcon->control_id);
					}
					else
					{
						//DrawMsgBox(STRING_NTFS_DELET_ERROR, STRING_YES, NULL);
						//DrawMsgBoxMultilLine(STRING_NTFS_DELET_ERROR_1, STRING_NTFS_DELET_ERROR_2, STRING_NTFS_DELET_ERROR_3, NULL, STRING_YES, NULL);
						DrawMsgBoxEx(STRING_NTFS_DELET_ERROR_1, IDOK, -1, TYPE_INFO);
					}

				}
			}
		}
		else if (plistbox->item_num) {
			Window_exit(pcon->pwin, pcon->control_id);
		}
	}
}

//PART_LIST_INFO g_part_list_info[128];

INTN AddPartListLine(PLISTBOX plistbox, WORD diskindex)
{
	CHAR16 buffer[100];
	WORD i = 0, index = 0;
	int partnum = 0;

	SetMem(g_part_list_info, sizeof(g_part_list_info), 0);

	for (i = 0; i < g_dwTotalPartitions; i++)
	{
		if (diskindex != g_Partitions[i].DiskNumber)
			continue;

		partnum++;    //记录该磁盘分区个数

		//分区大小小于4GB，可以忽略
		if (DivU64x32(g_Partitions[i].TotalSectors, 2048 * 1024) < 4) 
		{			
			continue;
		}

		//#if OKR_DEBUG //add by wh
		//			if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) == 0 || g_Partitions[i].PartitionId == 0x07)
		//#else 
		if (StrnCmp(g_Partitions[i].Label, L"LENOVO_PART", 11) == 0 || g_Partitions[i].PartitionId == 0x12)
			//#endif	
		{
			continue;
		}

		if (g_Partitions[i].PartitionType == FAT16_E || g_Partitions[i].PartitionType == FAT16)
		{
			continue;
		}

		if ((g_Partitions[i].PartitionType != FAT32) && (g_Partitions[i].PartitionType != FAT32_E) && (g_Partitions[i].PartitionType != NTFS) &&
			(g_Partitions[i].PartitionType != SHD_BITLOCKER) && (g_Partitions[i].PartitionType != SHD_BITLOCKERTOGO))
			continue;

		//MyLog(LOG_DEBUG, L"par %d disk %d bIfBackup =%d\n", i, g_Partitions[i].DiskNumber, g_Partitions[i].bIfBackup);
		if (g_Partitions[i].bIfBackup)
			continue;

		g_part_list_info[index].flag = 1;
		g_part_list_info[index].partindex = i;
		g_part_list_info[index].disknum = g_Partitions[i].DiskNumber;
		g_part_list_info[index].PartitionType = g_Partitions[i].PartitionType;
		g_part_list_info[index].TotalSectors = g_Partitions[i].TotalSectors;
		g_part_list_info[index].RemainSectors = g_Partitions[i].FreeSectors;
		SPrint(g_part_list_info[index].partname, 16, L"分区%d", index);
		Listbox_Addoneline(plistbox);

		if (g_Partitions[i].PartitionType == SHD_BITLOCKER || g_Partitions[i].PartitionType == SHD_BITLOCKERTOGO)
		{
			SPrint(buffer, 100, STRING_LIST_ITEM_BK_VOLUME_VALUE_B, index);
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);

			SetMem(buffer, 200, 0);
			SPrint(buffer, 100, L"-");
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);

			//分区总容量
			SetMem(buffer, 200, 0);
			//SPrint(buffer, 100, L"%d G", DivU64x32(g_part_list_info[index].TotalSectors, 2048 * 1024));
			SPrint(buffer, 100, L"-");
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);

			//剩余空间
			SetMem(buffer, 200, 0);
			SPrint(buffer, 100, L"-");
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
		}
		else
		{
			//卷标
			if (0 == IsHasLabelTextData(i))
			{
				SPrint(buffer, 100, STRING_LIST_ITEM_BK_VOLUME_VALUE, index);
			}
			else
			{
				SPrint(buffer, 100, STRING_LIST_ITEM_BK_VOLUME_VALUE, index);
				CopyMem(buffer, g_Partitions[i].Label, 100);
			}

			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);

			//类型
			SetMem(buffer, 200, 0);
			if ((g_Partitions[i].PartitionType == FAT32) || (g_Partitions[i].PartitionType == FAT32_E))
			{
				SPrint(buffer, 100, L"FAT32");
			}
			else if ((g_Partitions[i].PartitionType == FAT16) || (g_Partitions[i].PartitionType == FAT16_E))
			{
				SPrint(buffer, 100, L"FAT16");
			}
			else if (g_Partitions[i].PartitionType == NTFS)
			{
				SPrint(buffer, 100, L"NTFS");
			}
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);

			//总容量
			SetMem(buffer, 200, 0);
			SPrint(buffer, 100, L"%d GB", DivU64x32(g_part_list_info[index].TotalSectors, 2048 * 1024));
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);

			//剩余空间
			SetMem(buffer, 200, 0);

			if (DivU64x32(g_part_list_info[index].RemainSectors, 2048) > 1 * 1024)
			{
				SPrint(buffer, 100, L"%d GB", DivU64x32(g_part_list_info[index].RemainSectors, 2048 * 1024));
			}
			else
			{
				SPrint(buffer, 100, L"%d MB", DivU64x32(g_part_list_info[index].RemainSectors, 2048));
			}

			//SPrint(buffer, 100, L"%d GB", DivU64x32(g_part_list_info[index].RemainSectors, 2048 * 1024));
			Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
		}

		index++;

		if (index >= 128)
		{
			break;
		}

	}

    //if(partnum == 1 && BitLockerNum > 0){
 
  	 //    return -1;
    // }
	
	////添加一些测试数据
	//for(i=0;i<6;i++) {
	//	SPrint(buffer,100,L"Item %d", i);
	//	Listbox_Addoneline(plistbox);
	//	Listbox_Addoneunit(plistbox,ITEM_TYPE_CHAR_STRING,buffer);
	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
	//	Listbox_Addoneunit(plistbox, ITEM_TYPE_CHAR_STRING, buffer);
	//}
	//return i+index;

   return index;
}


int Listbox_showoneitemForRecoveryPointFace(PLISTBOX plistbox, WORD item_index, WORD current_select, DWORD backcolor, DWORD forecolor)
{
	BYTE buff[200] = { 0 };
	WORD *str = NULL;
	WORD j = 0;
	WORD x = 0, y = 0;
	WORD width = 0;
	if (item_index >= plistbox->item_num)
	{
		return 0;
	}

	WORD dwSpan = LIST_SPAN * g_data.fMinscale;
	if (plistbox->item_num > plistbox->items_per_win)
	{		
		dwSpan = 0;
	}
	//MyLog(LOG_DEBUG, L"list num: %d item pre win: %d\n", plistbox->item_num, plistbox->items_per_win);

	x = plistbox->controlinfo.thex;// +dwSpan;
	y = current_select * plistbox->height_peritem + plistbox->controlinfo.they;

	//Linebox(x,y,plistbox->controlinfo.thew,plistbox->height_peritem,COLOR_BLACK); //DEBUG
	CopyMem(&buff, plistbox->itemlist[item_index].item_buff, 200);

	//	if (backcolor!=COLOR_WHITE)
	FillRect(x + 1, y + 1, plistbox->controlinfo.thew - 2 + dwSpan, plistbox->height_peritem, backcolor);
	DrawLine(x + LIST_SPAN * g_data.fMinscale, y + plistbox->height_peritem, plistbox->controlinfo.thew - LIST_SPAN * g_data.fMinscale, 1, COLOR_LIST_SPLIT_LINE);

	str = (WORD *)buff;

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_LIST_ITEM_TEXT_SIZE); //(int)(FONT_LIST_ITEM_TEXT_SIZE * g_data.fFontScale);

	for (j = 0; j < plistbox->itemlist[item_index].columns; j++)
	{
		//show item 's cols one by one
		if ((plistbox->ptitle == NULL))
		{
			width = plistbox->controlinfo.thew + dwSpan;
		}
		else
		{
			width = plistbox->ptitle->column_list[j].curr_width;
			x = plistbox->ptitle->column_list[j].begin_x;
		}

		WORD dwTextHeight = SEL_REC_POS_ICON_HEIGHT * g_data.fMinscale;
		WORD dwTextTop = y + (SEL_REC_POINT_LIST_ITEM_HEIGHT * g_data.fMinscale - stStringInfo.iFontSize) / 2;

		if (str[0] & ITEM_TYPE_NOCOLUMN)
		{
			//特殊行，不受title控制，颜色特殊
			width = plistbox->controlinfo.thew + dwSpan;
			forecolor = COLOR_LISTBOX_SPECIAL;
		}
		if (str[0] & ITEM_TYPE_CHAR_STRING)
		{
			SetTextBKColor(backcolor);
			//PutstrinWidth(x + 4, dwTextTop, (CHAR16 *)((PBYTE)str + 4), forecolor, width - 4, &stStringInfo);
			DisplayStringInWidthEx(x + 4, dwTextTop, forecolor, (CHAR16 *)((PBYTE)str + 4), &stStringInfo, width - 4, 1, TRUE);
			//  PutstrinWidth(x+4,y+8,(CHAR16 *)((PBYTE)str+4),COLOR_WHITE, width-4);

			if (StrLen(STRING_FACTORY_BACKUP) == StrLen((CHAR16 *)((PBYTE)str + 4)))
			{
				CHAR16 buffer[20] = { 0 };
				SPrint(buffer, 20, L"%s", (CHAR16 *)((PBYTE)str + 4));
				if (StrnCmp(buffer, STRING_FACTORY_BACKUP, StrLen(STRING_FACTORY_BACKUP)) == 0 ||
					StrnCmp(buffer, STRING_INIT_BACKUP, StrLen(STRING_FACTORY_BACKUP)) == 0)
				{
					DWORD dwImageID = GetImageFileId(g_lock_Img_BackupPoint.fileBackupPointId);
					if (backcolor == plistbox->selectItemBkColor)
					{
						dwImageID = GetImageFileId(g_lock_Img_BackupPoint.fileBackupPointSelId);
					}

					DWORD dwWordLength = 0;
					GetStringLen((CHAR16 *)((PBYTE)str + 4), &stStringInfo, &dwWordLength);
					DisplayImgEx(x + 4 + dwWordLength + SEL_REC_POINT_LOCK_ICON_TEXT_LEFT * g_data.fMinscale, dwTextTop,
						(WORD)(SEL_REC_POINT_LOCK_ICON_WIDTH * g_data.fMinscale), (WORD)(SEL_REC_POINT_LOCK_ICON_HEIGHT * g_data.fMinscale), dwImageID, FALSE);
				}
			}
		}
		else if (str[0] & ITEM_TYPE_ISCONTROL)
		{
			struct CONTROL *p = (struct CONTROL *)((PBYTE)str + 4);
			p->show(p);
		}
		else if (str[0] & ITEM_TYPE_IMG)
		{
			WORD *ImageID = (WORD *)((PBYTE)str + 4);
			WORD dwImageID = *ImageID;

			//if (current_select == plistbox->whichselect)
			if (backcolor == plistbox->selectItemBkColor)
			{
				dwImageID = GetImageFileId(g_Img_BackupPoint.fileBackupPointSelId);
			}

			//MyLog(LOG_DEBUG, L"ITEM_TYPE_IMG %d\n", dwImageID);

			DisplayImgEx(x + 4, dwTextTop, (WORD)(SEL_REC_POINT_ICON_WIDTH * g_data.fMinscale), (WORD)(SEL_REC_POINT_ICON_HEIGHT * g_data.fMinscale), dwImageID, FALSE);
		}

		if (str[0] & ITEM_TYPE_NOCOLUMN)
		{
			break;
		}
		x += GetWidthofString((CHAR16 *)((PBYTE)str + 4)) + 3; //当没有标题框时
		str = (WORD *)(str[1] + 4 + (PBYTE)str);

	}
	return 0;

}


///////////////////////////////////Select Recovery Point Face/////////////////////////////////////////////////////////////
int SelectRecoveryPointFace(int *partindex/*0*/, DWORD *backupfileid/*-1*/)
{
	struct BUTTON buttonctrl[3] = { 0 };
	struct BUTTON externbuttonctrl[2] = { 0 };

	struct WINDOW win;
	DWORD ret;
	LISTBOX listbox;
	SCROLL_BOX scrollbox;
	TITLEBOX titlebox;
	RECT rc;
	WORD x, y;
	int index;
	int img_err_count = -1, img_check_num = -1;

	CHAR16 space[256] = { 0 };

	BOOL bIfExistBitlocker = FALSE;

	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	g_data.Cmd = USER_COMMAND_RECOVER_SELECT;

	//清除数据
	if (g_ShowOnceWndBuff != NULL)
	{
		FreePool(g_ShowOnceWndBuff);
		g_ShowOnceWndBuff = NULL;
	}

	//ProbeSystemPartition();
	int result = ProbeSystemPartition();
	if (result == -3)
	{
		////关闭提示框
		ShowWaitMsgOnce();
		DrawMsgBoxEx(STRING_MULTI_OS_RECOVER, IDOK, -1, TYPE_INFO);
		return IDCANCEL;
	}

	if (g_data.bHasFactoryBackup < 0)
	{
		g_data.bHasFactoryBackup = HasFactoryBackup();
	}

re_init:
	img_err_count = 0;
	img_check_num = 0;

	mymemset(space, 0, sizeof(space));

	//if (TRUE == g_bNeedUpdateBackupFileList)
	{
		//获取当前系统中检测到的所有备份的镜像文件，存储在全局变量g_headerlist里
		GetAllBackupFiles(&img_err_count);

		img_check_num = CheckAllBackupFiles();

		g_bNeedUpdateBackupFileList = FALSE;
	}

	//MyLog(LOG_DEBUG, L"img_check_num %d %d to restore.\n", img_check_num,img_err_count);    

	if (g_data.bFactoryRestore == TRUE) 
	{
		//工厂恢复模式
		ret = IDCANCEL;
		for (index = 0; index < g_data.ImageFilesNumber; index++) 
		{
			if (g_data.images[index].bFactoryBackup) 
			{
				*partindex = (int)g_data.images[index].PartitionIndex;
				*backupfileid = index;	//g_data.images[index].Time;
				ret = IDB_RECOVER;
				break;
			}
		}
		if (ret == IDCANCEL) 
		{
			MyLog(LOG_DEBUG, L"Factory backup not found.\n");
			//没有找到初始备份点，清除工厂恢复模式
			DrawMsgBoxEx(STRING_FACTORY_BACKUP_NOT_FOUND, IDOK, -1, TYPE_INFO);
			g_data.bFactoryRestore = FALSE;
		}
		goto completed;
	}

	//////////////////////////////////////UI///////////////////////////////////////////////////
	float fMinscale = g_data.fMinscale;
	WORD dwLeft = rc.x;
	WORD dwTop = rc.y;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	int disknum = 0;

	DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;

	plistbox = &listbox;

	dwLeft = dwBGLeft + SEL_REC_POINT_LIST_LEFT * fMinscale;
	dwTop = SEL_REC_POINT_LIST_TOP * fMinscale;
	dwWidth = SEL_REC_POINT_LIST_WIDTH * fMinscale;
	dwHeight = SEL_REC_POINT_LIST_HEIGHT * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POINT_LIST_TOP - 150) * fMinscale;
	}
	//Listbox_init(&listbox, dwLeft, dwTop, dwWidth, dwHeight, 10, 0x8000);
	Listbox_initEx(&listbox, dwLeft, dwTop, dwWidth, dwHeight, (DWORD)(SEL_REC_POINT_LIST_ITEM_HEIGHT * fMinscale), 0x8000);
	listbox.controlinfo.sendmessage = recoverlist_dispatchmsg;
	listbox.showoneitem = Listbox_showoneitemForRecoveryPointFace;

	ret = AddBackupLine(&listbox);
	Scrollbox_init(&scrollbox, (PCONTROL)&listbox, (WORD)ret, plistbox->items_per_win);
	listbox.pscroll = &scrollbox;  //绑定滚动条

	dwWidth = (SEL_REC_POINT_LIST_HEAD_WIDTH - SEL_REC_POINT_ICON_WIDTH - SEL_REC_POINT_ICON_TEXT_LEFT) * fMinscale;
	ONECOLUMN columns[4];

	columns[0].column_name = L"";  //point icon
	columns[0].orig_width = (SEL_REC_POINT_ICON_WIDTH + SEL_REC_POINT_ICON_TEXT_LEFT) * fMinscale;// dwWidth / 3;

	columns[1].column_name = STRING_LIST_ITEM_BK_POINT;
	columns[1].orig_width = dwWidth / 3;

	columns[2].column_name = STRING_LIST_ITEM_BK_TIME;
	columns[2].orig_width = dwWidth / 3;

	columns[3].column_name = STRING_LIST_ITEM_BK_IMG_SIZE;
	columns[3].orig_width = dwWidth / 3;

	dwWidth = SEL_REC_POINT_LIST_HEAD_WIDTH * fMinscale;
	Titlebox_init(&titlebox, &listbox, dwWidth, columns, 4);

	dwLeft = dwBGLeft + (WHITE_BG_WIDTH - SEL_REC_POINT_REC_BUTTON_WIDTH - SEL_REC_POINT_REC_BUTTON_RIGHT) * fMinscale;
	dwTop = SEL_REC_POINT_REC_BUTTON_TOP * fMinscale;
	dwWidth = SEL_REC_POINT_REC_BUTTON_WIDTH * fMinscale;
	dwHeight = SEL_REC_POINT_REC_BUTTON_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwHeight = SEL_REC_POINT_REC_BUTTON_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POINT_REC_BUTTON_TOP - 150) * fMinscale;
	}
	Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
		IMG_FILE_BTN_RESTORE1_PRESS, IMG_FILE_BTN_RESTORE1_FOCUS, IMG_FILE_BTN_RESTORE1_NORMAL, 1);

	dwLeft = dwBGLeft + SEL_REC_POINT_DEL_BUTTON_LEFT * fMinscale;
	dwTop = SEL_REC_POINT_DEL_BUTTON_TOP * fMinscale;
	dwWidth = SEL_REC_POINT_DEL_BUTTON_WIDTH * fMinscale;
	dwHeight = SEL_REC_POINT_DEL_BUTTON_HEIGHT * fMinscale;

	WORD presspcxDelete = GetImageFileId(IMG_FILE_BTN_DELETE_PRESS); //IMG_FILE_BTN_DELETE_PRESS;
	WORD focuspcxDelete = GetImageFileId(IMG_FILE_BTN_DELETE_FOCUS); //IMG_FILE_BTN_DELETE_FOCUS;
	WORD unfocuspcxDelete = GetImageFileId(IMG_FILE_BTN_DELETE_NORMAL); //IMG_FILE_BTN_DELETE_NORMAL;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwHeight = SEL_REC_POINT_DEL_BUTTON_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POINT_DEL_BUTTON_TOP - 150) * fMinscale;
	}
	Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxDelete, focuspcxDelete, unfocuspcxDelete, 1);

	dwLeft = BACKUP_BUTTON_LEFT * fMinscale;
	dwTop = BACKUP_BUTTON_TOP * fMinscale;
	dwWidth = BACKUP_BUTTON_SIZE * fMinscale;
	dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = BACKUP_BUTTON_LEFT * 0.5;
		dwTop = BACKUP_BUTTON_TOP * 0.5;
		dwWidth = BACKUP_BUTTON_SIZE * 0.5;
		dwHeight = BACKUP_BUTTON_SIZE * 0.5;
	}

	WORD presspcxGoBack = GetImageFileId(IMG_FILE_BUTTON_GO_BACK_PRESS); //IMG_FILE_BUTTON_GO_BACK_PRESS;
	WORD focuspcxGoBack = GetImageFileId(IMAG_FILE_GOBACK_FOCUS); //IMAG_FILE_GOBACK_FOCUS;
	WORD unfocuspcxGoBack = GetImageFileId(IMG_FILE_GOBACK); //IMG_FILE_GOBACK;

	Button_init(&buttonctrl[2], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxGoBack, focuspcxGoBack, unfocuspcxGoBack, 0);


	buttonctrl[0].controlinfo.control_id = IDB_RECOVER;
	buttonctrl[0].controlinfo.sendmessage = btn_dorecover;

	buttonctrl[0].btn_disablepcx = IMAG_FILE_BTN_RESTORE_DISABLE;
	//buttonctrl[0].clear_background = TRUE;

	buttonctrl[1].controlinfo.control_id = IDB_DELETE;
	buttonctrl[1].controlinfo.sendmessage = btn_dorecover;

	if (g_data.images[0].bFactoryBackup || ret == 0)
	{
		buttonctrl[1].controlinfo.status |= CON_STATUS_DISABLE;
	}

	WORD disablepcxDel = GetImageFileId(IMG_FILE_BTN_DELETE_DISABLE); //IMG_FILE_BTN_DELETE_DISABLE;

	buttonctrl[1].btn_disablepcx = disablepcxDel;
	//buttonctrl[1].clear_background = TRUE;

	buttonctrl[2].controlinfo.control_id = IDCANCEL;
	buttonctrl[2].controlinfo.sendmessage = btn_default;
	g_btnDelete = &buttonctrl[1];
	g_btnRestore = &buttonctrl[0];

	Window_init(&win, NULL);
	win.msgfunc = Recover_MyWindow_Dispatchmsg;
	win.windowID = 300;

	hidemouse();

	ClearScreen();
	DrawWhiteBK();

	//注意不要第一个添加cancel按钮，否则cancel会作为默认按钮
	Window_addctl(&win, (PCONTROL)&listbox);

	Window_addctl(&win, (PCONTROL)&buttonctrl[0]); 	//恢复按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]); 	//delete按钮
	Window_addctl(&win, (PCONTROL)&buttonctrl[2]); 	//goback按钮
	Window_addctl(&win, (PCONTROL)&titlebox);
    Window_addctl(&win, (PCONTROL)&scrollbox);

	dwLeft = dwBGLeft;
	dwTop = BK_PAGE_TITLE_TOP * fMinscale;
	dwWidth = BK_PAGE_TITLE_WIDTH * fMinscale;
	dwHeight = BK_PAGE_TITLE_HEIGHT * fMinscale;
	WORD unfocuspcxSysR = GetImageFileId(IMAG_FILE_SYS_R_TITLE); //IMAG_FILE_SYS_R_TITLE;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft;
		dwTop = (BK_PAGE_TITLE_TOP - 100) * 0.5;
		dwWidth = BK_PAGE_TITLE_WIDTH * 0.5;
		dwHeight = BK_PAGE_TITLE_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_TITLE_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxSysR, FALSE);

	dwLeft = dwBGLeft + SEL_REC_POINT_SEL_POS_TEXT_LEFT * fMinscale;
	dwTop = SEL_REC_POINT_SEL_POS_TEXT_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POINT_SEL_POS_TEXT_TOP - 150) * fMinscale;
	}
	StringInfo stStringInfoRECOVER_LIST_TITILE;
	stStringInfoRECOVER_LIST_TITILE.enumFontType = FZLTZHJW;
	stStringInfoRECOVER_LIST_TITILE.iFontSize = CalcFontSize(50); //50 * g_data.fFontScale;
	DisplayStringEx(dwLeft, dwTop, COLOR_DEFAULT_BACKGROUND, STRING_RECOVER_LIST_TITILE, &stStringInfoRECOVER_LIST_TITILE, TRUE);

	DisplayLogo();

	dwLeft = dwBGLeft + SEL_REC_POINT_LIST_LEFT * fMinscale;
	dwTop = SEL_REC_POINT_LIST_TOP * fMinscale;
	dwWidth = SEL_REC_POINT_LIST_HEAD_WIDTH * fMinscale;
	dwHeight = SEL_REC_POINT_LIST_HEIGHT * fMinscale;

	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POINT_LIST_TOP - 150) * fMinscale;
	}
	FillRect(dwLeft, dwTop, dwWidth, dwHeight, COLOR_LIST_BK);
	Linebox(dwLeft, dwTop, dwWidth, dwHeight, COLOR_LIST_SPLIT_LINE);

	if ((img_err_count + img_check_num) > 0)
	{
		StringInfo stStringInfoBK_NUM_DESC2;
		stStringInfoBK_NUM_DESC2.enumFontType = FZLTHJW;
		stStringInfoBK_NUM_DESC2.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

		dwLeft = dwBGLeft + SEL_REC_POINT_INFO_TEXT_LEFT * fMinscale;
		//dwTop = SEL_REC_POINT_INFO_TEXT_TOP * fMinscale;
		dwTop = (SEL_REC_POINT_INFO_TEXT_TOP + (69 - 26) / 2) * fMinscale;
		if (1440 == g_WindowWidth && 900 == g_WindowHeight)
		{
			dwTop = (SEL_REC_POINT_INFO_TEXT_TOP - 150 + (69 - 26) / 2) * fMinscale;
		}
		//SPrint(space, 256, L"%s %d %s", STRING_BK_NUM_DESC1, (img_err_count + img_check_num), STRING_BK_NUM_DESC2, DivU64x32(g_part_list_info[index].TotalSectors, 2048 * 1024));
		SPrint(space, 256, L"%s %d %s", STRING_BK_NUM_DESC1, (img_err_count + img_check_num), STRING_BK_NUM_DESC2);

		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, space, &stStringInfoBK_NUM_DESC2, TRUE);
	}

	if (ret != 0)
	{
		bIfExistBitlocker = IfExistBitlocker();
		if (bIfExistBitlocker)
		{
			StringInfo stStringInfo;
			stStringInfo.enumFontType = FZLTHJW;
			stStringInfo.iFontSize = CalcFontSize(26); //(int)(30 * g_data.fFontScale + 0.5);

			dwLeft = dwBGLeft + SEL_REC_POINT_INFO_TEXT_LEFT * fMinscale;
			dwTop = (SEL_REC_POINT_INFO_TEXT_TOP + (69 - 26) / 2) * fMinscale;
			dwWidth = SEL_REC_POINT_INFO_TEXT_WIDTH * fMinscale;
			dwHeight = SEL_REC_POINT_INFO_TEXT_HEIGHT * fMinscale;

			if (1440 == g_WindowWidth && 900 == g_WindowHeight)
			{
				dwTop = (SEL_REC_POINT_INFO_TEXT_TOP - 150 + (69 - 26) / 2) * fMinscale;
			}

			//if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
			//{
			//	dwTop = (SEL_REC_POINT_INFO_TEXT_TOP + (69 - 26) / 2) * fMinscale;
			//	dwHeight = SEL_REC_POINT_INFO_TEXT_HEIGHT * fMinscale + 20;
			//}

			DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

			if (g_data.ImageFilesNumber > 0)
			{
				DisplayStringInRectEx(dwLeft, dwTop, dwWidth, dwHeight, COLOR_TEXT_INFO, STRING_BITLOCKER_EXIST_BACKUPFILE, &stStringInfo, 0);
				//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_BITLOCKER_EXIST_BACKUPFILE, &stStringInfo, FALSE);
			}
			//else
			//{
			//	DisplayStringInRectEx(dwLeft, dwTop, dwWidth, dwHeight, COLOR_TEXT_INFO, STRING_BITLOCKER_NO_BACKUPFILE, &stStringInfo, 0);
			//	//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_INFO, STRING_BITLOCKER_NO_BACKUPFILE, &stStringInfo, FALSE);
			//}
		}
	}

	if (ret == 0)
	{
		MyLog(LOG_DEBUG, L"List is empty.\n");

		buttonctrl[0].controlinfo.status |= CON_STATUS_DISABLE;
		g_isUpdateEmptyRecoveryPointList = TRUE;
	}

	showmouse();

	if (g_data.bDebug)
		MyLogString(NULL);

	do
	{
		Window_reset(&win);

		ret = Window_run(&win);

		if (ret == IDB_RECOVER && listbox.item_num)
		{
			index = listbox.firstitem_inwindow + listbox.whichselect;
			*partindex = (int)g_data.images[index].PartitionIndex;
			*backupfileid = index;	//g_data.images[index].Time;

			break;

			//if (DrawMsgBoxEx(STRING_R_SYS_HINT, IDOK, IDCANCEL, TYPE_WARNING) == IDOK)
			//{
			//	break;
			//}
			//else
			//{
			//	continue;
			//}
		}
		else
		{
			break;
		}

		MyLog(LOG_DEBUG, L"SelectRecoveryPointFace ret %d\n", ret);
	} while (TRUE);

	Listbox_destroy(&listbox);

	//if (ret == IDB_DELETE)
	//{
	//	goto re_init;
	//}

completed:

	if (ret == IDB_RECOVER)
	{
		ret = IDB_RECOVER;

		MyLog(LOG_DEBUG, L"Selected %d(%s) to restore.\n", index, g_data.images[index].FileName);
	}
	//getch();
	//ret = IDCANCEL;
	//Listbox_destroy(&listbox);

	return ret;
}


/////////////////////////////////////////////////////////////////////////////
// this function is the recover end ,and show the end UI ,only reboot button on it 
int RecoverCompleteUIMsgbox(int func)
{
	float fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}

	g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	///for test
	//g_data.bIsAutoFunc = TRUE;


	STRINGBUTTON buttonctrl[2] = { 0 };
	struct WINDOW win;
	int ret = -1;
	//WORD x1, y1, rows, width;
	BYTE oldstate, bMouseDisplay;
	BUTTON_INFO* btnInfo = NULL;

	WORD dwBGLeft = (g_WindowWidth - COMPLETE_MSG_WIDTH * fMinscale) / 2;
	WORD dwBGTop = (g_WindowHeight - COMPLETE_MSG_HEIGHT * fMinscale) / 2;

	//save img
#if OKR_DEBUG_UI //add by wh
	MyLog(LOG_MESSAGE, L"======-----=== RecoverCompleteUIMsgbox.");

#endif

	//PBYTE buff = AllocatePool(4 * (COMPLETE_MSG_WIDTH + 5)*(COMPLETE_MSG_HEIGHT + 5));
	PBYTE buff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	//GetImgEx(dwBGLeft, dwBGTop, COMPLETE_MSG_WIDTH, COMPLETE_MSG_HEIGHT, buff);
	GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);

	dwLeft = dwBGLeft + COMPLETE_MSG_CONFIRM_BTN_LEFT * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_CONFIRM_BTN_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_CONFIRM_BTN_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_CONFIRM_BTN_HEIGHT * fMinscale;
	Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight, //OK  button 
		IMG_FILE_BUTTON_CONFIRM_PRESS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 1);

	dwLeft = dwBGLeft + (COMPLETE_MSG_WIDTH - COMPLETE_MSG_CLOSE_BTN_RIGHT - COMPLETE_MSG_CLOSE_BTN_WIDTH) * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_CLOSE_BTN_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_CLOSE_BTN_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_CLOSE_BTN_HEIGHT * fMinscale;

	WORD presspcxClose = GetImageFileId(IMG_FILE_MSG_CLOSE_PRESS); //IMG_FILE_MSG_CLOSE_PRESS;
	WORD focuspcxClose = GetImageFileId(IMG_FILE_MSG_CLOSE_FOCUS); //IMG_FILE_MSG_CLOSE_FOCUS;
	WORD unfocuspcxClose = GetImageFileId(IMG_FILE_MSG_CLOSE_UNFOCUS); //IMG_FILE_MSG_CLOSE_UNFOCUS;
	//if (0.5 >= fMinscale)
	//{
	//	presspcxClose = IMG_FILE_MSG_CLOSE_S_PRESS;
	//	focuspcxClose = IMG_FILE_MSG_CLOSE_S_FOCUS;
	//	unfocuspcxClose = IMG_FILE_MSG_CLOSE_S_UNFOCUS;
	//}
	Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,  // the cancel  button  
		presspcxClose, focuspcxClose, unfocuspcxClose, 0);


	buttonctrl[0].button.controlinfo.control_id = IDOK;
	if (g_data.bIsAutoFunc)
	{
		g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
		buttonctrl[0].button.bIsCountDownBtn = TRUE;
		buttonctrl[0].button.controlinfo.sendmessage = btn_default;
		buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
	}
	else
	{
		buttonctrl[0].button.controlinfo.sendmessage = btn_default;
	}
	buttonctrl[1].button.controlinfo.control_id = IDCANCEL;
	buttonctrl[1].button.controlinfo.sendmessage = btn_default;

	Window_init(&win, NULL);
	win.windowID = 120;
	g_lastCountDownTime = mytime(NULL);
	hidemouse();

	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);

	dwLeft = dwBGLeft;
	dwTop = dwBGTop;
	dwWidth = COMPLETE_MSG_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_HEIGHT * fMinscale;
	//DrawRect(COLOR_EXIT_MSG_BOX_BK, dwLeft, dwTop, dwWidth, dwHeight);

	DrawMsgBoxWhiteBK(dwLeft, dwTop, dwWidth, dwHeight);

	dwLeft = dwBGLeft + COMPLETE_MSG_INFO_ICON_LEFT * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_INFO_ICON_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_INFO_ICON_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_INFO_ICON_HEIGHT * fMinscale;
	WORD successImage = GetImageFileId(IMG_FILE_SUCCESS); //IMG_FILE_SUCCESS;
	//if (0.5 >= fMinscale)
	//{
	//	successImage = IMG_FILE_SUCCESS_S;
	//}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, successImage, FALSE);

	dwLeft = dwBGLeft + COMPLETE_MSG_TEXT_LEFT * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_TEXT_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_TEXT_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_TEXT_HEIGHT * fMinscale;
	StringInfo stStringInfoCOMPLETE;
	stStringInfoCOMPLETE.enumFontType = FZLTZHJW;
	stStringInfoCOMPLETE.iFontSize = CalcFontSize(60); //60 * g_data.fFontScale;
	if (func == USER_COMMAND_RECOVER)
	{
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_R_COMPLETE, &stStringInfoCOMPLETE, TRUE);
	}
	else
	{
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_BK_COMPLETE, &stStringInfoCOMPLETE, TRUE);
	}

	dwLeft = dwBGLeft + 2;
	dwTop = dwBGTop + COMPLETE_MSG_SPLIT_LINE_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_SPLIT_LINE_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_TEXT_HEIGHT;
	DrawLine(dwLeft, dwTop, dwWidth, COMPLETE_MSG_SPLIT_LINE_HEIGHT, 0xDCEFFF);

	showmouse();

	Window_addctl(&win, (PCONTROL)&buttonctrl[0]);
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]);

	ret = Window_run(&win);

	hidemouse();
	//restore img
	//PutImgEx(dwBGLeft, dwBGTop, COMPLETE_MSG_WIDTH, COMPLETE_MSG_HEIGHT, buff);
	PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);

	EnableMouse(oldstate);

	//if (bMouseDisplay)
	//	showmouse();
	showmouse();

	FreePool(buff);
	return ret;

}

// RecoverCompleteUIMsgboxEx with message
//func: Backup or recovery
//iResult: 1: success or 0: faile
//msg: message info
int RecoverCompleteUIMsgboxEx(int func, int iResult, CHAR16 *msg)
{
	float fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}

	g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	///for test
	//g_data.bIsAutoFunc = TRUE;

	STRINGBUTTON buttonctrl[2] = { 0 };
	struct WINDOW win;
	int ret = -1;
	//WORD x1, y1, rows, width;
	BYTE oldstate, bMouseDisplay;
	BUTTON_INFO* btnInfo = NULL;

	WORD dwBGLeft = (g_WindowWidth - COMPLETE_MSG_EX_WIDTH * fMinscale) / 2;
	WORD dwBGTop = (g_WindowHeight - COMPLETE_MSG_EX_HEIGHT * fMinscale) / 2;

	//save img
#if OKR_DEBUG_UI //add by wh
	MyLog(LOG_MESSAGE, L"======-----=== RecoverCompleteUIMsgbox.");

#endif

	PBYTE buff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

	ClearKeyboard();

	bMouseDisplay = hidemouse();
	oldstate = EnableMouse(TRUE);

	GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);

	dwLeft = dwBGLeft + COMPLETE_MSG_EX_CONFIRM_BTN_LEFT * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_EX_CONFIRM_BTN_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_EX_CONFIRM_BTN_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_EX_CONFIRM_BTN_HEIGHT * fMinscale;
	Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight, //OK  button 
		IMG_FILE_BUTTON_CONFIRM_PRESS, IMG_FILE_BUTTON_CONFIRM_FOCUS, IMAG_FILE_CONFIRM_UNFOCUS, 1);

	dwLeft = dwBGLeft + (COMPLETE_MSG_EX_WIDTH - COMPLETE_MSG_EX_CLOSE_BTN_RIGHT - COMPLETE_MSG_EX_CLOSE_BTN_WIDTH) * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_EX_CLOSE_BTN_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_EX_CLOSE_BTN_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_EX_CLOSE_BTN_HEIGHT * fMinscale;
	WORD presspcxClose = GetImageFileId(IMG_FILE_MSG_CLOSE_PRESS); //IMG_FILE_MSG_CLOSE_PRESS;
	WORD focuspcxClose = GetImageFileId(IMG_FILE_MSG_CLOSE_FOCUS); //IMG_FILE_MSG_CLOSE_FOCUS;
	WORD unfocuspcxClose = GetImageFileId(IMG_FILE_MSG_CLOSE_UNFOCUS); //IMG_FILE_MSG_CLOSE_UNFOCUS;
	//if (0.5 >= fMinscale)
	//{
	//	presspcxClose = IMG_FILE_MSG_CLOSE_S_PRESS;
	//	focuspcxClose = IMG_FILE_MSG_CLOSE_S_FOCUS;
	//	unfocuspcxClose = IMG_FILE_MSG_CLOSE_S_UNFOCUS;
	//}
	Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,  // the cancel  button  
		presspcxClose, focuspcxClose, unfocuspcxClose, 0);
	
	buttonctrl[0].button.controlinfo.control_id = IDOK;
	if (g_data.bIsAutoFunc)
	{
		g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
		buttonctrl[0].button.bIsCountDownBtn = TRUE;
		buttonctrl[0].button.controlinfo.show = ButtonSplicingCountdown_showEx;
	}
	else
	{
		buttonctrl[0].button.controlinfo.sendmessage = btn_default;
	}

	buttonctrl[1].button.controlinfo.control_id = IDCANCEL;
	buttonctrl[1].button.controlinfo.sendmessage = btn_default;

	Window_init(&win, NULL);
	win.windowID = 120;
	g_lastCountDownTime = mytime(NULL);

	hidemouse();

	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);

	dwLeft = dwBGLeft;
	dwTop = dwBGTop;
	dwWidth = COMPLETE_MSG_EX_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_EX_HEIGHT * fMinscale;
	//DrawRect(COLOR_EXIT_MSG_BOX_BK, dwLeft, dwTop, dwWidth, dwHeight);

	DrawMsgBoxWhiteBK(dwLeft, dwTop, dwWidth, dwHeight);

	dwLeft = dwBGLeft + COMPLETE_MSG_EX_INFO_ICON_LEFT * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_EX_INFO_ICON_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_EX_INFO_ICON_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_EX_INFO_ICON_HEIGHT * fMinscale;
	if (func == USER_COMMAND_RECOVER || func == USER_COMMAND_BACKUP)
	{
		if (iResult == 1)
		{
			WORD successImage = GetImageFileId(IMG_FILE_SUCCESS); //IMG_FILE_SUCCESS;
			//if (0.5 >= fMinscale)
			//{
			//	successImage = IMG_FILE_SUCCESS_S;
			//}
			DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, successImage, FALSE);
		}
		else
		{
			WORD failedImage = GetImageFileId(IMG_FILE_FAILED); //IMG_FILE_FAILED;
			//if (0.5 >= fMinscale)
			//{
			//	failedImage = IMG_FILE_FAILED_S;
			//}
			DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, failedImage, FALSE);
		}
	}
	else if (func == USER_COMMAND_INIT_BACKUP)
	{
		WORD unfocuspcxInitial = GetImageFileId(IMAG_FILE_INITIAL_BK); //IMAG_FILE_INITIAL_BK;
		//if (0.5 >= fMinscale)
		//{
		//	unfocuspcxInitial = IMAG_FILE_INITIAL_BK_S;
		//}
		DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxInitial, FALSE);
	}

	dwLeft = dwBGLeft + COMPLETE_MSG_EX_TEXT_LEFT * fMinscale;
	dwTop = dwBGTop + COMPLETE_MSG_EX_TEXT_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_EX_TEXT_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_EX_TEXT_HEIGHT * fMinscale;
	StringInfo stStringInfoCOMPLETE;
	stStringInfoCOMPLETE.enumFontType = FZLTZHJW;
	stStringInfoCOMPLETE.iFontSize = CalcFontSize(60); //60 * g_data.fFontScale;
	if (func == USER_COMMAND_RECOVER)
	{
		if (iResult == 1)
		{
			DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_R_COMPLETE, &stStringInfoCOMPLETE, TRUE);
		}
		else
		{
			DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_RESTORE_FAIL, &stStringInfoCOMPLETE, TRUE);
		}
	}
	else if (func == USER_COMMAND_INIT_BACKUP) /////////
	{
		DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_TITLE_INIT_BACKUP, &stStringInfoCOMPLETE, TRUE);
	}
	else //if (func == USER_COMMAND_BACKUP)
	{
		if (iResult == 1)
		{
			DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_BK_COMPLETE, &stStringInfoCOMPLETE, TRUE);
		}
		else
		{
			DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_BACKUP_FAIL, &stStringInfoCOMPLETE, TRUE);
		}
	}

	StringInfo stStringInfoMsg;
	stStringInfoMsg.enumFontType = FZLTHJW;
	stStringInfoMsg.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	DWORD dwTextLen = 0;
	GetStringLen(msg, &stStringInfoMsg, &dwTextLen);

	MyLog(LOG_DEBUG, L"GetStringLen %d", dwTextLen);

	//dwLeft = dwBGLeft + (COMPLETE_MSG_EX_WIDTH * fMinscale - dwTextLen) / 2;
	if (dwTextLen >= (DWORD)(COMPLETE_MSG_EX_WIDTH * fMinscale))
	{
		dwLeft = dwBGLeft + COMPLETE_MSG_EX_MSG_TEXT_LEFT * fMinscale;
	}
	else
	{
		dwLeft = dwBGLeft + (COMPLETE_MSG_EX_WIDTH * fMinscale - dwTextLen) / 2;
	}

	dwTop = dwBGTop + COMPLETE_MSG_EX_MSG_TEXT_TOP * fMinscale;
	dwWidth = (COMPLETE_MSG_EX_WIDTH - COMPLETE_MSG_EX_MSG_TEXT_LEFT * 2) * fMinscale;
	dwHeight = COMPLETE_MSG_EX_MSG_TEXT_HEIGHT * fMinscale;
	//PrintMessageInRectEx(dwLeft, dwTop, dwWidth, dwHeight, msg, COLOR_TEXT_CONTENT, &stStringInfoMsg);
	DisplayStringInRectEx(dwLeft, dwTop, dwWidth, dwHeight, COLOR_TEXT_CONTENT, msg, &stStringInfoMsg, 0);


	dwLeft = dwBGLeft + 2;
	dwTop = dwBGTop + COMPLETE_MSG_EX_SPLIT_LINE_TOP * fMinscale;
	dwWidth = COMPLETE_MSG_EX_SPLIT_LINE_WIDTH * fMinscale;
	dwHeight = COMPLETE_MSG_EX_TEXT_HEIGHT;
	DrawLine(dwLeft, dwTop, dwWidth, COMPLETE_MSG_EX_SPLIT_LINE_HEIGHT, 0xDCEFFF);

	showmouse();

	Window_addctl(&win, (PCONTROL)&buttonctrl[0]);
	Window_addctl(&win, (PCONTROL)&buttonctrl[1]);

	ret = Window_run(&win);

	hidemouse();
	//restore img
	PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, buff);

	EnableMouse(oldstate);

	//if (bMouseDisplay)
	//	showmouse();
	showmouse();

	FreePool(buff);
	return ret;

}

// Same: return 1, else return -1.
int CompareHddSN(CHAR8* sn1, CHAR8* sn2)
{
	UINT16 i = 0 ;
	 
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

	GetBackupFileFullPath(&sysinfo, partindex, filename, 0, fullpath, 100);

	//if (IsLenovoPart(partindex) == TRUE)
	//{
	//	SPrint(fullpath, 128, BACKUP_DIR L"\\%s", filename);
	//}
	//else
	//{
	//	SPrint(fullpath, 128, BACKUP_DIR_CUSTOMER L"\\%s", filename);
	//}

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

//int sysdisktype_in_backupfile = 0 ; // 0 none ; 1 nvme ; 2 sata;
int FindNewHdd(WORD recoverfilepartindex,DWORD backupfileID)
{
	int found = -1;
	CHAR8 *TempBuffer = NULL ;
	CHAR8 diskcount = 0 ;
	CHAR8 *temphddsn[MAX_SUPPORED_HDD_NO] = { 0 };
	CHAR8 *tmphddsn;
	CHAR8  tmpDisksn[20] = {0};

	OKR_HEADER *Header = NULL ;
	UINT16 i = 0 , j =0 ;
	int newhdd = 0 ;
	BOOL bIsRAIDBackupFile = FALSE;
	
	Header =&(g_data.images[backupfileID]);
	
	g_data.CurImageSyspartitionsize = 0;
	
	if(Header->Version != OKR_VERSION)
		return -1;
	
	TempBuffer = (CHAR8 *)Header->Memo + 100 ;
	
	if (TempBuffer[0] != 'D' || TempBuffer[1] != 'S' || TempBuffer[2] != 'N')
	{
		return -1;
	}	
	
	diskcount = TempBuffer[3];
	
	g_data.CurImageSyspartitionsize =*(UINT64*)&TempBuffer[4];

	if (g_data.CurImageSyspartitionsize == 0)
	{
		MyLog(LOG_DEBUG, L"CurImageSyspartitionsize is 0. return -1.\n");
		return -1;
	}
	
	TempBuffer= TempBuffer + 12;
	
	for(j=0; j< MAX_SUPPORED_HDD_NO; j++)
	{
		temphddsn[j] = 0;
	}
	
	for(j=0; j< diskcount && j< MAX_SUPPORED_HDD_NO; j++)   //解决内存泄漏问题
	{
		temphddsn[j] = TempBuffer;
		TempBuffer = TempBuffer + 20;
	} 

	//MyLog(LOG_ERROR, L"backup file store sn: %a\n", *temphddsn);
	tmphddsn = temphddsn[0]; 
	if(tmphddsn[0] == 0x20 || tmphddsn[0] == 0)
		sysdisktype_in_backupfile = 2 ; // sata
	else
		sysdisktype_in_backupfile = 1 ; //nvme
	
	if (tmphddsn[0] == 'R' &&
		tmphddsn[1] == 'A' &&
		tmphddsn[2] == 'I' &&
		tmphddsn[3] == 'D')
	{
		bIsRAIDBackupFile = TRUE;
		g_data.enumGSKUSkipType = DISABLE_NEW_HDD_FUNC;
		MyLog(LOG_DEBUG, L"BackupFile is RAID disk. Change GSKU Skip type to DISABLE_NEW_HDD_FUNC.\n");
	}

	//for(j=0; j< MAX_DISK_NUM; j++)
	for (j = 0; j < g_disknum; j++)
	{
		found = -1;
		//if(g_disklist[j].removable == TRUE)
		//	continue ;
		MyLog(LOG_ERROR, L"FindNewHdd-Sysdisk[%d].sn=%02x%02x\n",j,g_disklist[j].disksn[0],g_disklist[j].disksn[19]);   
		//if(checkIfSnZero(g_disklist[j].disksn) == 1)
		//	continue ;
		if( g_disklist[j].totalsector == 0)
		{
#ifdef OKR_DEBUG
			MyLog(LOG_ERROR, L"disk %d, zero size ,continue;\n", j);
#endif
			continue;
		}
		
		if(checkIfSnZero(g_disklist[j].disksn) == 1)
		{
			MyLog(LOG_ERROR, L"disk %d sn is zero, continue;\n", j);	
			continue;
		}

		SetMem(tmpDisksn, 20, 0);
		CopyMem(tmpDisksn, g_disklist[j].disksn, 20);
		
		MyLog(LOG_DEBUG, L"bIsRAIDBackupFile: %d. isRAIDDisk: %d. \n", bIsRAIDBackupFile, g_disklist[j].isRAIDDisk);
		MyLog(LOG_DEBUG, L"diskcount %d.\n", diskcount);
		for(i = 0; i< diskcount ; i++)
		{
			///System disk
			if (i == 0)
			{				
				if (bIsRAIDBackupFile)
				{
					if (1 == g_disklist[j].isRAIDDisk)
					{
						//RAID mode
						SetMem(tmpDisksn, 20, 0);
						CopyMem(tmpDisksn, g_disklist[j].raidDisksn, 20);

						g_disklist[j].curdisktype = RAID_DISK;
						found = 1;
					}
					else
					{
						//Error, can not recovery
					}
				}
				else
				{
					if (1 == g_disklist[j].isRAIDDisk)
					{
						//Error, can not recovery
						SetMem(tmpDisksn, 20, 0);
						CopyMem(tmpDisksn, g_disklist[j].raidDisksn, 20);

						g_disklist[j].curdisktype = RAID_DISK;
						found = 1;
					}
					else
					{
						//Normal
					}
				}
			}

			MyLog(LOG_DEBUG, L"CompareHddSN disksn: %a, temphddsn: %a, i:%d\n", tmpDisksn, temphddsn[i], i);
			if(CompareHddSN(tmpDisksn, temphddsn[i]) == 1)
			{
				if (i == 0)
				{
					g_disklist[j].curdisktype = DISK_SYSDISK;
					MyLog(LOG_DEBUG, L"disk %d is system disk.\n", j);
				}
				else
				{
					g_disklist[j].curdisktype = DISK_DATA;
					MyLog(LOG_DEBUG, L"disk %d is data disk.\n", j);
				}

				found = 1;
				
				MyLog(LOG_DEBUG, L"Match-imagedisk[%d].sn=%02x%02x\n",i,g_disklist[j].disksn[0],g_disklist[j].disksn[19]);
			}
			else
			{
				MyLog(LOG_DEBUG, L"CompareHddSN is not same.\n");
			}
		}
		MyLog(LOG_DEBUG, L"found %d.\n", found);
		if(found != 1 )
		{
			MyLog(LOG_DEBUG, L"FindNewHdd-NewDisk");
			g_disklist[j].curdisktype = NEW_FOUND_DISK;
			newhdd ++;
		}			
	}
	
	if(newhdd > 0)
	{
		MyLog(LOG_DEBUG, L"FindNewHdd-NewDisk num: %d", newhdd);
		return 1;
	}		
	else
	{
		MyLog(LOG_DEBUG, L"Does not find new disk.");
		return -1;
	}
}

int GetRecoverableDiskNumber()
{
	int iRecoverableDisk = 0;
	DWORD dwSize = 0;

	for (WORD iFlag = 0; iFlag < g_disknum; iFlag++)
	{
		if (g_disklist[iFlag].curdisktype == DISK_SYSDISK)
		{
			iRecoverableDisk++;
			MyLog(LOG_DEBUG, L"Found DISK_SYSDISK.\n");
			continue;
		}

		if (g_disklist[iFlag].curdisktype == NEW_FOUND_DISK)
		{
			//分区大小小于4GB，可以忽略
			dwSize = DivU64x32(g_disklist[iFlag].totalsector, 2048);
			if (dwSize < 4 * 1024)
			{
				MyLog(LOG_DEBUG, L"SKIP small partition.\n");
				continue;
			}

			if (DISABLE_NEW_HDD_FUNC == g_data.enumGSKUSkipType)
			{
				MyLog(LOG_DEBUG, L"SKIP New Disk.\n");
				continue;
			}

			MyLog(LOG_DEBUG, L"sysdisktype_in_backupfile is %d, SkipType: %d\n", sysdisktype_in_backupfile, g_data.enumGSKUSkipType);

			if (g_disklist[iFlag].disksn[0] == 0x20 || g_disklist[iFlag].disksn[0] == 0) //sata
			{
				if (DISABLE_NVME_TO_SATA == g_data.enumGSKUSkipType ||
					DISABLE_DIFFERENT_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 1)  //备份disk类型为NVME盘
					{
						MyLog(LOG_DEBUG, L"SKIP SATA Disk.\n");
						continue;
					}
				}

				if (DISABLE_SATA_TO_SATA == g_data.enumGSKUSkipType ||
					DISABLE_SAME_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 2)  //备份disk类型为SATA盘
					{
						MyLog(LOG_DEBUG, L"SKIP SATA Disk.\n");
						continue;
					}
				}
			}
			else
			{
				if (DISABLE_SATA_TO_NVME == g_data.enumGSKUSkipType ||
					DISABLE_DIFFERENT_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 2)  //备份disk类型为sata盘
					{
						MyLog(LOG_DEBUG, L"SKIP NVME Disk.\n");
						continue;
					}
				}

				if (DISABLE_NVME_TO_NVME == g_data.enumGSKUSkipType ||
					DISABLE_SAME_TYPE == g_data.enumGSKUSkipType)
				{
					if (sysdisktype_in_backupfile == 1)  //备份disk类型为NVME盘
					{
						MyLog(LOG_DEBUG, L"SKIP NVME Disk.\n");
						continue;
					}
				}
			}

			iRecoverableDisk++;
		}

		if (iRecoverableDisk >= 16)
		{
			break;
		}
	}

	MyLog(LOG_DEBUG, L"==GetRecoverableDiskNumber %d TotalDiskNum: %d\n", iRecoverableDisk, g_disknum);
	return iRecoverableDisk;
}

#define EXTRA_SIZE 0x2800000  // 20G 扇区
int DetectNewHddFace(WORD recoverfilepartindex,DWORD backupfileID) //BackupSetting
{
    struct BUTTON buttonctrl[3];
    struct WINDOW win;

	LISTBOX listbox;
	SCROLL_BOX scrollbox;
	TITLEBOX titlebox;
	LISTBOX *pListbox = NULL;

    DWORD ret = IDCANCEL;
    //EDITBOX editbox;
	int selecteddiskindex =-1;
    UINT64 data;
    CHAR16 space[80] = { 0 };
    CHAR16 capspace[80] = { 0 };
	WORD i = 0;
    RECT rc;
    int result = 0;
    int img_err_count = 0;
	
    //struct BUTTON externbuttonctrl[2] = { 0 };
	
	//BOOL bIsGSKU = FALSE;
	int iIndex = 0;
	BOOL bSkip = FALSE;

	MyLog(LOG_ERROR, L"-----wh--------BackupSetting...\n");
	
	result = FindNewHdd(recoverfilepartindex,backupfileID) ;

	if (DISABLE_NEW_HDD_FUNC == g_data.enumGSKUSkipType)
	{
		MyLog(LOG_ERROR, L"GSKU flag is DISABLE_NEW_HDD_FUNC.\n");
		return IDB_RECOVER;
	}

	if (result != 1)
	{
		return IDB_RECOVER;
	}
	else
	{
		MyLog(LOG_DEBUG, L"sysdisktype_in_backupfile is %d, SkipType: %d\n", sysdisktype_in_backupfile, g_data.enumGSKUSkipType);

		if (DISABLE_SATA_TO_NVME == g_data.enumGSKUSkipType ||
			DISABLE_DIFFERENT_TYPE == g_data.enumGSKUSkipType)
		{
			if (sysdisktype_in_backupfile == 2)  //备份disk类型为sata盘
			{
				bSkip = FALSE;
				
				//目标硬盘均为New disk & NVME
				while (iIndex < g_disknum)
				{
					if (g_disklist[iIndex].curdisktype == NEW_FOUND_DISK)
					{
						MyLog(LOG_DEBUG, L"Disk: %d sn: %c\n", iIndex, g_disklist[iIndex].disksn[0]);
						if (g_disklist[iIndex].disksn[0] == 0 || g_disklist[iIndex].disksn[0] == 0x20) //恢复disk类型为到sata盘
						{
							bSkip = FALSE;
							MyLog(LOG_DEBUG, L"Found new sata disk. Disk index: %d\n", iIndex);
							break;
						}
						else
						{
							MyLog(LOG_DEBUG, L"Disk %d is NVME disk.\n", iIndex);
							bSkip = TRUE;
						}
					}
					iIndex++;
				}

				MyLog(LOG_DEBUG, L"bSkip: %d\n", bSkip);
				if (bSkip)
				{
					MyLog(LOG_DEBUG, L"Skip new disk recovery.(DISABLE_SATA_TO_NVME | DISABLE_DIFFERENT_TYPE)\n");
					return IDB_RECOVER;
				}
				else
				{
					MyLog(LOG_DEBUG, L"Exist new sata disk.\n");
				}
			}
		}

		if (DISABLE_NVME_TO_SATA == g_data.enumGSKUSkipType ||
			DISABLE_DIFFERENT_TYPE == g_data.enumGSKUSkipType)
		{
			if (sysdisktype_in_backupfile == 1)  //备份disk类型为nvme盘
			{
				bSkip = FALSE;

				//目标硬盘均为New disk & NVME
				while (iIndex < g_disknum)
				{
					if (g_disklist[iIndex].curdisktype == NEW_FOUND_DISK)
					{
						if (g_disklist[iIndex].disksn[0] == 0 || g_disklist[iIndex].disksn[0] == 0x20) //恢复disk类型为到sata盘
						{
							bSkip = TRUE;
						}
						else
						{
							//恢复disk类型为到nvme盘
							bSkip = FALSE;
							MyLog(LOG_DEBUG, L"Found new nvme disk. Disk index: %d\n", iIndex);
							break;
						}
					}
					iIndex++;
				}

				MyLog(LOG_DEBUG, L"bSkip: %d\n", bSkip);

				if (bSkip)
				{
					MyLog(LOG_DEBUG, L"Skip new disk recovery.(DISABLE_NVME_TO_SATA | DISABLE_DIFFERENT_TYPE)\n");
					return IDB_RECOVER;
				}
				else
				{
					MyLog(LOG_DEBUG, L"Exist new nvme disk.\n");
				}
			}
		}

		if (DISABLE_NVME_TO_NVME == g_data.enumGSKUSkipType ||
			DISABLE_SAME_TYPE == g_data.enumGSKUSkipType)
		{
			if (sysdisktype_in_backupfile == 1)  //备份disk类型为nvme盘
			{
				bSkip = FALSE;

				//目标硬盘均为New disk & NVME
				while (iIndex < g_disknum)
				{
					if (g_disklist[iIndex].curdisktype == NEW_FOUND_DISK)
					{
						MyLog(LOG_DEBUG, L"Disk: %d sn: %c\n", iIndex, g_disklist[iIndex].disksn[0]);
						if (g_disklist[iIndex].disksn[0] == 0 || g_disklist[iIndex].disksn[0] == 0x20) //恢复disk类型为到sata盘
						{
							bSkip = FALSE;
							MyLog(LOG_DEBUG, L"Found new sata disk. Disk index: %d\n", iIndex);
							break;
						}
						else
						{
							MyLog(LOG_DEBUG, L"Disk %d is NVME disk.\n", iIndex);
							bSkip = TRUE;
						}
					}
					iIndex++;
				}

				MyLog(LOG_DEBUG, L"bSkip: %d\n", bSkip);
				if (bSkip)
				{
					MyLog(LOG_DEBUG, L"Skip new disk recovery.(DISABLE_NVME_TO_NVME | DISABLE_SAME_TYPE)\n");
					return IDB_RECOVER;
				}
				else
				{
					MyLog(LOG_DEBUG, L"Exist new sata disk.\n");
				}
			}
		}

		if (DISABLE_SATA_TO_SATA == g_data.enumGSKUSkipType ||
			DISABLE_SAME_TYPE == g_data.enumGSKUSkipType)
		{
			if (sysdisktype_in_backupfile == 2)  //备份disk类型为sata盘
			{
				bSkip = FALSE;

				while (iIndex < g_disknum)
				{
					if (g_disklist[iIndex].curdisktype == NEW_FOUND_DISK)
					{
						MyLog(LOG_DEBUG, L"Disk: %d sn: %c\n", iIndex, g_disklist[iIndex].disksn[0]);
						if (g_disklist[iIndex].disksn[0] == 0 || g_disklist[iIndex].disksn[0] == 0x20) //恢复disk类型为到sata盘
						{
							bSkip = TRUE;
						}
						else
						{
							MyLog(LOG_DEBUG, L"Disk %d is NVME disk.\n", iIndex);
							bSkip = FALSE;
							MyLog(LOG_DEBUG, L"Found new nvme disk. Disk index: %d\n", iIndex);
							break;
						}
					}
					iIndex++;
				}

				MyLog(LOG_DEBUG, L"bSkip: %d\n", bSkip);
				if (bSkip)
				{
					MyLog(LOG_DEBUG, L"Skip new disk recovery.(DISABLE_SATA_TO_SATA | DISABLE_SAME_TYPE)\n");
					return IDB_RECOVER;
				}
				else
				{
					MyLog(LOG_DEBUG, L"Exist new nvme disk.\n");
				}
			}
		}

		//if (DrawMsgBoxEx(STRING_FIND_NEW_DISK_CONFIRM, IDOK, IDCANCEL, TYPE_ENQUIRE) == IDCANCEL)
		//{
		//	return IDB_RECOVER;
		//}
	}

re_init:

	g_data.Cmd = USER_COMMAND_RECOVER_NEWHDD_SELECT;
    
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));

	/////////////////////////////////Select disk for Recovery System///////////////////////////////////////////////

	float fMinscale = g_data.fMinscale;
	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;

 //   //show message box
	//g_data.TotalBackupSize = GetBackupTotaldata(); //20 * 1024 * 1024;// GetBackupTotaldata();
	//data = DivU64x32(g_data.TotalBackupSize,1024*1024);
	//if (data > 1024)
	//{
	//	SPrint(capspace, 80, L"%d.%d GB", data / 1024, (data % 1024) / 10);
	//}
	//else
	//{
	//	SPrint(capspace, 80, L"%d MB", data);
	//}

	dwLeft = dwBGLeft + (WHITE_BG_WIDTH - SEL_REC_POS_NEXT_BUTTON_WIDTH - SEL_REC_POS_NEXT_BUTTON_RIGHT) * fMinscale;
	dwTop = SEL_REC_POS_NEXT_BUTTON_TOP * fMinscale;
	dwWidth = SEL_REC_POS_NEXT_BUTTON_WIDTH * fMinscale;
	dwHeight = SEL_REC_POS_NEXT_BUTTON_HEIGHT * fMinscale;

	WORD presspcxNext = GetImageFileId(IMAG_FILE_NEXT_PRESS); //IMAG_FILE_NEXT_PRESS;
	WORD focuspcxNext = GetImageFileId(IMAG_FILE_NEXT_FOCUS); //IMAG_FILE_NEXT_FOCUS;
	WORD unfocuspcxNext = GetImageFileId(IMAG_FILE_NEXT); //IMAG_FILE_NEXT;
	//if (0.5 >= fMinscale)
	//{
	//	presspcxNext = IMAG_FILE_NEXT_PRESS_S;
	//	focuspcxNext = IMAG_FILE_NEXT_S_FOCUS;
	//	unfocuspcxNext = IMAG_FILE_NEXT_S;
	//}
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwHeight = SEL_REC_POS_NEXT_BUTTON_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POS_NEXT_BUTTON_TOP - 150) * fMinscale;
	}
	Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxNext, focuspcxNext, unfocuspcxNext, 1);

	dwLeft = BACKUP_BUTTON_LEFT * fMinscale;
	dwTop = BACKUP_BUTTON_TOP * fMinscale;
	dwWidth = BACKUP_BUTTON_SIZE * fMinscale;
	dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = BACKUP_BUTTON_LEFT * 0.5;
		dwTop = BACKUP_BUTTON_TOP * 0.5;
		dwWidth = BACKUP_BUTTON_SIZE * 0.5;
		dwHeight = BACKUP_BUTTON_SIZE * 0.5;
	}

	WORD presspcxGoBack = GetImageFileId(IMG_FILE_BUTTON_GO_BACK_PRESS); //IMG_FILE_BUTTON_GO_BACK_PRESS;
	WORD focuspcxGoBack = GetImageFileId(IMAG_FILE_GOBACK_FOCUS); //IMAG_FILE_GOBACK_FOCUS;
	WORD unfocuspcxGoBack = GetImageFileId(IMG_FILE_GOBACK); //IMG_FILE_GOBACK;
	//if (0.5 >= fMinscale)
	//{
	//	presspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_PRESS;
	//	focuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_FOCUS;
	//	unfocuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_UNFOCUS;
	//}
	Button_init(&buttonctrl[1], dwLeft, dwTop, dwWidth, dwHeight,
		presspcxGoBack, focuspcxGoBack, unfocuspcxGoBack, 0);

      
    buttonctrl[0].controlinfo.control_id = IDB_NEW_HDD_RECOVER;
    buttonctrl[0].controlinfo.sendmessage = btn_dobackup;
	//buttonctrl[0].clear_background = TRUE;

    buttonctrl[1].controlinfo.control_id = IDCANCEL;
    buttonctrl[1].controlinfo.sendmessage = btn_default;
  
	pListbox = &listbox;

	dwLeft = dwBGLeft + SEL_REC_POS_LIST_LEFT * fMinscale;
	dwTop = SEL_REC_POS_LIST_TOP * fMinscale;
	dwWidth = SEL_REC_POS_LIST_WIDTH * fMinscale;
	dwHeight = SEL_REC_POS_LIST_HEIGHT * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POS_LIST_TOP - 150) * fMinscale;
	}
	//Listbox_init(&listbox, dwLeft, dwTop, dwWidth, dwHeight, 10, 0x8000);
	Listbox_initEx(&listbox, dwLeft, dwTop, dwWidth, dwHeight, (DWORD)(SEL_REC_POS_LIST_ITEM_HEIGHT * fMinscale), 0x8000);
	//listbox.controlinfo.sendmessage = recoverlist_dispatchmsg;

	ret = AddDiskInfoLine(&listbox);
	Scrollbox_init(&scrollbox, (PCONTROL)&listbox, (WORD)ret, plistbox->items_per_win);
	listbox.pscroll = &scrollbox;

	dwWidth = (SEL_REC_POS_LIST_HEAD_WIDTH - SEL_REC_POS_ICON_WIDTH - SEL_REC_POS_ICON_TEXT_LEFT) * fMinscale;
	ONECOLUMN columns[4];

	columns[0].column_name = L"";  //point icon
	columns[0].orig_width = (SEL_REC_POS_ICON_WIDTH + SEL_REC_POS_ICON_TEXT_LEFT) * fMinscale;// dwWidth / 3;

	columns[1].column_name = STRING_LIST_ITEM_DISK;
	columns[1].orig_width = dwWidth / 3;

	columns[2].column_name = STRING_LIST_ITEM_TYPE;
	columns[2].orig_width = dwWidth / 3;

	columns[3].column_name = STRING_LIST_ITEM_CAP;
	columns[3].orig_width = dwWidth / 3;

	dwWidth = SEL_REC_POS_LIST_HEAD_WIDTH * fMinscale;
	Titlebox_init(&titlebox, &listbox, dwWidth, columns, 4);

    Window_init(&win,NULL);
	win.windowID = 400;
    
    win.msgfunc=SelectDiskMyWindow_Dispatchmsg;

	hidemouse();

    //ClearScreen();
	DrawWhiteBK();

	//dwLeft = dwBGLeft + SEL_REC_POS_LIST_LEFT * fMinscale;
	//dwTop = SEL_REC_POS_LIST_TOP * fMinscale;
	//dwWidth = SEL_REC_POS_LIST_WIDTH * fMinscale;
	//dwHeight = SEL_REC_POS_LIST_HEIGHT * fMinscale;
	//g_DriveList = Recover_NewDriveButtons(&win, dwLeft, dwTop, dwWidth, dwHeight, g_data.BackupDiskNumber);

	Window_addctl(&win, (PCONTROL)&listbox);

    Window_addctl(&win, (PCONTROL)&buttonctrl[0]);
    Window_addctl(&win, (PCONTROL)&buttonctrl[1]);

	Window_addctl(&win, (PCONTROL)&scrollbox);
	Window_addctl(&win, (PCONTROL)&titlebox);

    hidemouse();
 
	dwLeft = dwBGLeft + SEL_REC_POS_SEL_POS_TEXT_LEFT * fMinscale;
	dwTop = SEL_REC_POS_SEL_POS_TEXT_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POS_SEL_POS_TEXT_TOP - 150) * fMinscale;
	}
	StringInfo stStringInfoFORCE_RESTORE_HDD;
	stStringInfoFORCE_RESTORE_HDD.enumFontType = FZLTZHJW;
	stStringInfoFORCE_RESTORE_HDD.iFontSize = CalcFontSize(50); //50 * g_data.fFontScale;
	DisplayStringEx(dwLeft, dwTop, COLOR_DEFAULT_BACKGROUND, STRING_FORCE_RESTORE_HDD, &stStringInfoFORCE_RESTORE_HDD, TRUE);
	
	dwLeft = dwBGLeft;
	dwTop = BK_PAGE_TITLE_TOP * fMinscale;
	dwWidth = BK_PAGE_TITLE_WIDTH * fMinscale;
	dwHeight = BK_PAGE_TITLE_HEIGHT * fMinscale;
	WORD unfocuspcxSysR = GetImageFileId(IMAG_FILE_SYS_R_TITLE); //IMAG_FILE_SYS_R_TITLE;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft;
		dwTop = (BK_PAGE_TITLE_TOP - 100) * 0.5;
		dwWidth = BK_PAGE_TITLE_WIDTH * 0.5;
		dwHeight = BK_PAGE_TITLE_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_TITLE_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxSysR, FALSE);

	dwLeft = dwBGLeft + SEL_REC_POS_LIST_LEFT * fMinscale;
	dwTop = SEL_REC_POS_LIST_TOP * fMinscale;
	dwWidth = SEL_REC_POS_LIST_HEAD_WIDTH * fMinscale;
	dwHeight = SEL_REC_POS_LIST_HEIGHT * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (SEL_REC_POS_LIST_TOP - 150) * fMinscale;
	}
	FillRect(dwLeft, dwTop, dwWidth, dwHeight, COLOR_LIST_BK);
	Linebox(dwLeft, dwTop, dwWidth, dwHeight, COLOR_LIST_SPLIT_LINE);

	DisplayLogo();

    showmouse();

	if(g_data.bDebug)
		MyLogString(NULL);

	do
	{
		Window_reset(&win);

		ret = Window_run(&win);

		g_data.PartCBootBegin = 0;
		g_data.PartCTotalSector = 0;
	
		if (ret == IDB_NEW_HDD_RECOVER && listbox.item_num)
		{
			//选择的是磁盘，而不是分区，要自动计算哪个分区最合适
			int mydestdisk;
			int checkstatus;
			int index;

			g_data.needlimit = 0;

			index = listbox.firstitem_inwindow + listbox.whichselect;
			MyLog(LOG_DEBUG, L"select index %d", index);

			mydestdisk = g_disk_list_info[index].disknum;

			MyLog(LOG_DEBUG, L"select recovery disk %d", mydestdisk);

			//if (g_DriveList->selected != -1)
			//	mydestdisk = g_DriveList->partindex[g_DriveList->selected];


			if (mydestdisk < 0 || mydestdisk >= MAX_DISK_NUM)
			{
				DrawMsgBoxEx(STRING_DISK_NOT_SELECTED_SMALL, IDOK, -1, TYPE_INFO);
				//DeleteDriveButtons(g_DriveList);
				//Listbox_destroy(&listbox);
				//goto re_init;
				continue;
			}

			if (mydestdisk == g_Partitions[recoverfilepartindex].DiskNumber)
			{
				MyLog(LOG_DEBUG, L"select recovery disk is the same as backupfile stored.");
				if (g_disklist[mydestdisk].curdisktype == DISK_SYSDISK)
				{
					//force recovery
					if (g_data.bForceRestorePartition == TRUE)
					{
						//backupfileID
						if (g_data.images[backupfileID].bFactoryBackup == FALSE)
						{
							DrawMsgBoxEx(STRING_DISK_SAME_SELECTED_DISK, IDOK, -1, TYPE_INFO);
							//DeleteDriveButtons(g_DriveList);
							//Listbox_destroy(&listbox);
							//goto re_init;
							continue;
						}
					}
				}
				else if (g_disklist[mydestdisk].curdisktype == NEW_FOUND_DISK)
				{				
					DrawMsgBoxEx(STRING_DISK_SAME_SELECTED_DISK, IDOK, -1, TYPE_INFO);
					//DeleteDriveButtons(g_DriveList);
					//Listbox_destroy(&listbox);
					//goto re_init;
					continue;
				}
			}

			checkstatus = g_disklist[mydestdisk].curdisktype;
			if (checkstatus == DISK_DATA)
			{
				////Data disk does not add to the new disk list, this case can not reach.
				//DrawMsgBoxEx(STRING_NOT_SUPPORT_DATA_DISK_TYPE, IDOK, -1, TYPE_INFO);
				//DeleteDriveButtons(g_DriveList);
				//Listbox_destroy(&listbox);
				//goto re_init;
				continue;
			}
			else if (checkstatus == DISK_SYSDISK)
			{
				int image_partc_index = 0;
				int gpart_index = 0;
				int retvalue = 0;

				retvalue = CheckSysDiskPartationCSize(recoverfilepartindex, backupfileID, mydestdisk, &gpart_index, &image_partc_index);

				if (retvalue == PARTC_NOT_FOUND)
				{
					DrawMsgBoxEx(STRING_DISK_ORG_SYSDISK_NOT_FOUND, IDOK, -1, TYPE_INFO);
					//DeleteDriveButtons(g_DriveList);
					//Listbox_destroy(&listbox);
					//goto re_init;
					continue;
				}

				if (retvalue == PARTC_SMALL_THAN_IMAGE)
				{
					UINT64	 NewAlignNum1 = MultU64x32(g_data.CurImageSyspartitionsize, BLOCK_SIZE);
					MyLog(LOG_DEBUG, L"sys disk CurImageSyspin sec:0x%llx\n", NewAlignNum1);
					if (g_Partitions[gpart_index].TotalSectors < (NewAlignNum1 + EXTRA_SIZE))
					{
						DrawMsgBoxEx(STRING_DISK_ORG_SYSDISK_TOO_SMALL, IDOK, -1, TYPE_INFO);
						//DeleteDriveButtons(g_DriveList);
						//Listbox_destroy(&listbox);
						//goto re_init;
						continue;
					}
					g_data.needlimit = 1;
				}

				index = listbox.firstitem_inwindow + listbox.whichselect;
				g_data.selecteddisk = g_disk_list_info[index].disknum;
				g_data.destdisktype = checkstatus;
				g_data.comefromnewhddface = 0;

				break;
			}
			else if (checkstatus == NEW_FOUND_DISK)
			{
				CHAR16 stringbuff[256] = { 0 };
				UINT64	 NewAlignNum = MultU64x32(g_data.CurImageSyspartitionsize, BLOCK_SIZE);
				UINT64	 size_c = MultU64x32(NewAlignNum + 1, 512);
				UINT64	 size_newhdd = MultU64x32(g_disklist[mydestdisk].totalsector, 512);

				OKR_HEADER * head = NULL;

				head = ReadImageFileHeader_Max(recoverfilepartindex, backupfileID);

				MyLog(LOG_DEBUG, L"new hdd CurImageSyspartitionsize sec :0x%llx\n", NewAlignNum);
				size_c = DivU64x32(size_c, 1024);
				size_c = DivU64x32(size_c, 1024);
				size_c = DivU64x32(size_c, 1024);

				size_newhdd = DivU64x32(size_newhdd, 1024);
				size_newhdd = DivU64x32(size_newhdd, 1024);
				size_newhdd = DivU64x32(size_newhdd, 1024);

				UINT64 NeedSize = 0, MyNeedSize = 0;

				DWORD j;
				for (j = 0; j < head->PartitionCount; j++) 
				{
					if (!head->Partition[j].bIfBackup) 
					{
						continue;
					}
					NeedSize += head->Partition[j].TotalSectors;
				}
				MyNeedSize = DivU64x32(NeedSize, 2 * 1024 * 1024);
				CheckNewDiskPartationCSize(recoverfilepartindex, backupfileID);

				if (TRUE == g_disklist[mydestdisk].removable)
				{
					if (DrawMsgBoxEx(STRING_DONOT_SELECT_REMOVEABLE_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK) //可移动设备提示
					{
						//DeleteDriveButtons(g_DriveList);
						//Listbox_destroy(&listbox);
						//goto re_init;
						continue;
					}
				}

				if (NeedSize > g_disklist[mydestdisk].totalsector)
				{
					///新硬盘空间无法满足ESP，Windows系统分区以及Winre分区空间大小之和
					SPrint(stringbuff, 256, STRING_VERSION, size_newhdd, MyNeedSize);
					DrawMsgBoxEx(stringbuff, IDOK, -1, TYPE_INFO);
					//DeleteDriveButtons(g_DriveList);
					//Listbox_destroy(&listbox);
					//goto re_init;
					continue;
				}

				if ((NewAlignNum + EXTRA_SIZE) > g_disklist[mydestdisk].totalsector)
				{
					mydestdisk = -1;

					SPrint(stringbuff, 256, STRING_NEW_DISK_SIZE_SMALL, size_c);
					DrawMsgBoxEx(stringbuff, IDOK, -1, TYPE_INFO);
					//DeleteDriveButtons(g_DriveList);
					//Listbox_destroy(&listbox);
					//goto re_init;
					continue;
				}

				//if (DrawMsgBoxEx(STRING_RECOVER_NEW_DISK_WARNING, IDOK, IDCANCEL, TYPE_ENQUIRE) != IDOK) //免责提示
				//{
				//	//DeleteDriveButtons(g_DriveList);
				//	//goto re_init;
				//	continue;
				//}


				//if (g_DriveList->selected != -1)
				//	g_data.selecteddisk = g_DriveList->partindex[g_DriveList->selected];

				index = listbox.firstitem_inwindow + listbox.whichselect;
				g_data.selecteddisk = g_disk_list_info[index].disknum;
				g_data.destdisktype = checkstatus;
				g_data.comefromnewhddface = 1;

				break;
			}
			else
			{
				DrawMsgBoxEx(STRING_DISK_UNKNOWN_TYPE, IDOK, -1, TYPE_INFO);
				//DeleteDriveButtons(g_DriveList);
				//Listbox_destroy(&listbox);
				//goto re_init;
				continue;
			}
		}
		else
		{
			break;
		}

		MyLog(LOG_DEBUG, L"SelectBackupPartFace ret %d\n", ret);
	} while (TRUE);
	   

#if OKR_DEBUG
		//	MyLog(LOG_ERROR,L"auto-selected partition %d.\n", selectedpartindex);
			//MyLog(LOG_ERROR,L"press any key to continue...\n");
			//getch();
#endif	

	Listbox_destroy(&listbox);

	//DeleteDriveButtons(g_DriveList);
 // getch();
//  ret = IDCANCEL;
	return ret;
}

////////////////////////Recovery face/////////////////////////////////////////
int RecoveryFace(WORD recoverfilepartindex, DWORD backupfileID)
{
	struct BUTTON buttonctrl[1];
	struct WINDOW win;

	PROCESSBOX progress;
	int nRet;
	BYTE oldstate;
	RECT rc;
	//WORD x, y;


	MyLog(LOG_ERROR, L"----RecoveryFace...\n");
	g_data.selecteddisk = -1;
	g_data.comefromnewhddface = 0;
	g_data.enumGSKUSkipType = NOT_GSKU;
	g_data.bIsGSKU = FALSE;
	g_data.bIsGSKU = IsGSKU();

	nRet = DetectNewHddFace(recoverfilepartindex, backupfileID);
	if (IDCANCEL == nRet)
	{
		g_data.selecteddisk = -1;
		g_data.comefromnewhddface = 0;
		return -1;
	}

	if (IDB_RECOVER == nRet)
	{
		g_data.selecteddisk = -1;
		g_data.comefromnewhddface = 0;
	}

	MyLog(LOG_DEBUG, L"CurImageSyspartitionsize %lld\n", g_data.CurImageSyspartitionsize);
				
	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));

	oldstate = EnableMouse(FALSE);
	hidemouse();
	float fMinscale = g_data.fMinscale;

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;
	//DWORD dwBGTop = (g_WindowHeight - WHITE_BG_HEIGHT * fMinscale) / 2;

	//Backup button
	dwLeft = BACKUP_BUTTON_LEFT * fMinscale;
	dwTop = BACKUP_BUTTON_TOP * fMinscale;
	dwWidth = BACKUP_BUTTON_SIZE * fMinscale;
	dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = BACKUP_BUTTON_LEFT * 0.5;
		dwTop = BACKUP_BUTTON_TOP * 0.5;
		dwWidth = BACKUP_BUTTON_SIZE * 0.5;
		dwHeight = BACKUP_BUTTON_SIZE * 0.5;
	}

	hidemouse();

	DrawLine(dwLeft - 1, dwTop - 1, dwWidth + 2, dwHeight + 2, COLOR_DEFAULT_BACKGROUND);

	//WORD presspcxGoBack = GetImageFileId(IMG_FILE_BUTTON_GO_BACK_PRESS); //IMG_FILE_BUTTON_GO_BACK_PRESS;
	//WORD focuspcxGoBack = GetImageFileId(IMAG_FILE_GOBACK_FOCUS); //IMAG_FILE_GOBACK_FOCUS;
	//WORD unfocuspcxGoBack = GetImageFileId(IMG_FILE_GOBACK); //IMG_FILE_GOBACK;
	////if (0.5 >= fMinscale)
	////{
	////	presspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_PRESS;
	////	focuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_FOCUS;
	////	unfocuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_UNFOCUS;
	////}
	//Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
	//	presspcxGoBack, focuspcxGoBack, unfocuspcxGoBack, 0);
	//Button_show((PCONTROL)&buttonctrl[0]);

	//buttonctrl[0].controlinfo.control_id = IDCANCEL;
	//buttonctrl[0].controlinfo.sendmessage = btn_default;

	//Window_init(&win, NULL);

	//ClearScreen();

	DrawWhiteBK();
	
	StringInfo stStringInfoRECOVERY_SYSTEM;
	stStringInfoRECOVERY_SYSTEM.enumFontType = FZLTZHJW;
	stStringInfoRECOVERY_SYSTEM.iFontSize = CalcFontSize(90); //90 * g_data.fFontScale;

	dwWidth = 0;
	GetStringLen(STRING_RECOVERY_SYSTEM, &stStringInfoRECOVERY_SYSTEM, &dwWidth);
	dwLeft = (g_WindowWidth - dwWidth) / 2;
	dwTop = REC_PAGE_SUB_TITLE_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_SUB_TITLE_TOP - 150) * fMinscale;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_RECOVERY_SYSTEM, &stStringInfoRECOVERY_SYSTEM, TRUE);

	StringInfo stStringInfoREMAIN_TIME;
	stStringInfoREMAIN_TIME.enumFontType = FZLTHJW;
	stStringInfoREMAIN_TIME.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;

	dwWidth = 0;
	GetStringLen(STRING_PREPARE_RECOVERY, &stStringInfoREMAIN_TIME, &dwWidth);

	dwLeft = (g_WindowWidth - dwWidth) / 2;
	dwTop = REC_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_REAMIN_TIME_TEXT_TOP - 150) * fMinscale;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_PREPARE_RECOVERY, &stStringInfoREMAIN_TIME, TRUE);

	//dwLeft = (g_WindowWidth - (REC_PAGE_REAMIN_TIME_TEXT_WIDTH + REC_PAGE_REAMIN_TIME_VALUE_WIDTH + REC_PAGE_REAMIN_TIME_TEXT_VALUE_SPAN) * fMinscale) / 2;
	//dwTop = REC_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
	////dwWidth = REC_PAGE_REAMIN_TIME_TEXT_WIDTH * fMinscale;
	////dwHeight = REC_PAGE_REAMIN_TIME_TEXT_HEIGHT * fMinscale;
	//StringInfo stStringInfoREMAIN_TIME;
	//stStringInfoREMAIN_TIME.enumFontType = FZLTHJW;
	//stStringInfoREMAIN_TIME.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
	//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_REMAIN_TIME, &stStringInfoREMAIN_TIME, TRUE);

	//dwLeft = (g_WindowWidth - (REC_PAGE_REAMIN_TIME_TEXT_WIDTH + REC_PAGE_REAMIN_TIME_VALUE_WIDTH + REC_PAGE_REAMIN_TIME_TEXT_VALUE_SPAN) * fMinscale) / 2 + (REC_PAGE_REAMIN_TIME_TEXT_WIDTH + REC_PAGE_REAMIN_TIME_TEXT_VALUE_SPAN) * fMinscale;
	//dwTop = REC_PAGE_REAMIN_TIME_VALUE_TOP * fMinscale;
	//dwWidth = REC_PAGE_REAMIN_TIME_VALUE_WIDTH * fMinscale + 100;
	//dwHeight = REC_PAGE_REAMIN_TIME_VALUE_HEIGHT * fMinscale + 10;
	//StringInfo stStringInfoREMAIN_TIME_VALUE;
	//stStringInfoREMAIN_TIME_VALUE.enumFontType = FZLTHJW;
	//stStringInfoREMAIN_TIME_VALUE.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
	//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_REMAIN_TIME_VALUE, &stStringInfoREMAIN_TIME_VALUE, TRUE);
		 
	StringInfo stStringInfoRECOVERY_INFO;
	stStringInfoRECOVERY_INFO.enumFontType = FZLTHJW;
	stStringInfoRECOVERY_INFO.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	dwWidth = 0;
	GetStringLen(STRING_RECOVERY_INFO, &stStringInfoRECOVERY_INFO, &dwWidth);
	dwLeft = (g_WindowWidth - dwWidth) / 2;
	dwTop = REC_PAGE_INFO_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_INFO_TOP - 150) * fMinscale;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_RECOVERY_INFO, &stStringInfoRECOVERY_INFO, TRUE);

	////Do not display the info. [20210114]
	//dwLeft = (g_WindowWidth - REC_PAGE_INFO_TEXT_WIDTH * fMinscale) / 2;
	//dwTop = REC_PAGE_INFO_TEXT_TOP * fMinscale;
	//StringInfo stStringInfoRECOVERY_CANCEL;
	//stStringInfoRECOVERY_CANCEL.enumFontType = FZLTHJW;
	//stStringInfoRECOVERY_CANCEL.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);
	//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_CANCEL, &stStringInfoRECOVERY_CANCEL, TRUE);

	dwLeft = (g_WindowWidth - REC_PAGE_PROGRESS_WIDTH * fMinscale) / 2;
	dwTop = REC_PAGE_PROGRESS_TOP * fMinscale;
	dwWidth = REC_PAGE_PROGRESS_WIDTH * fMinscale;
	dwHeight = PROGRESS_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwHeight = PROGRESS_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_PROGRESS_TOP - 150) * fMinscale;
	}

	Processbox_init(&progress, dwLeft, dwTop, dwWidth, dwHeight);
	Processbox_show((PCONTROL)&progress);

	dwLeft = dwBGLeft;
	dwTop = REC_PAGE_TITLE_TOP * fMinscale;
	dwWidth = REC_PAGE_TITLE_WIDTH * fMinscale;
	dwHeight = REC_PAGE_TITLE_HEIGHT * fMinscale;

	WORD dwWidthBK = (MAIN_PAGE_TITLE_WIDTH + 10) * fMinscale;

	WORD unfocuspcxSysR = GetImageFileId(IMAG_FILE_SYS_R_TITLE); //IMAG_FILE_SYS_R_TITLE;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft;
		dwTop = (REC_PAGE_TITLE_TOP - 100) * 0.5;
		dwWidth = REC_PAGE_TITLE_WIDTH * 0.5;
		dwHeight = REC_PAGE_TITLE_HEIGHT * 0.5;
		dwWidthBK = (MAIN_PAGE_TITLE_WIDTH + 10) * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (REC_PAGE_TITLE_TOP - 150) * fMinscale;
	}

	DrawLine(dwLeft, dwTop, dwWidthBK, dwHeight, COLOR_DEFAULT_BACKGROUND);

	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxSysR, FALSE);

	DisplayLogo();

	hidemouse();

	MyLog(LOG_DEBUG, L"CurImageSyspartitionsize %lld\n", g_data.CurImageSyspartitionsize);

	if (g_data.CurImageSyspartitionsize == 0)
	{
		nRet = Recover_Linux(recoverfilepartindex, backupfileID, (PCONTROL)&progress);
	}
	else
	{
		MyLog(LOG_DEBUG, L"recoverfilepartindex:%d, backupfileID:%d\n", recoverfilepartindex, backupfileID);
		//getch();
		nRet = Recover(recoverfilepartindex, backupfileID, (PCONTROL)&progress);

		//if (g_data.bIsGSKU && (0 == GetRecoverableDiskNumber()))
		//{
		//	MyLog(LOG_DEBUG, L"Destination disk not found.\n");
		//	DrawMsgBoxEx(STRING_DISK_NOT_FOUND_ERROR, IDOK, -1, TYPE_INFO);
		//	nRet = -1;
		//}
		//else
		//{
		//	nRet = Recover(recoverfilepartindex, backupfileID, (PCONTROL)&progress);
		//}
	}

#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing disk...\n");
#endif
	GetDiskInfo();
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing partition...\n");
#endif
	GetPartitionInfo();

	g_bNeedUpdateBackupFileList = TRUE;

	EnableMouse(oldstate);
	showmouse();

	//ret = Window_run(&win);

	MyLog(LOG_DEBUG, L"DoRecovery nRet:%d.\n", nRet);

	if (nRet < 0)
	{
		return nRet;
	}
	else
	{
		if (g_data.destdisktype == NEW_FOUND_DISK && g_data.comefromnewhddface == 1 && g_data.dwErrorCode == OKR_ERROR_SUCCESS) //kang 
		{
			//Show STRING_RECOVER_NEW_DISK_SUC Message box
		}
		else
		{
			RecoverCompleteUIMsgbox(USER_COMMAND_RECOVER);
		}

	}

	return 0;
}


//////////////////////////Backup Face///////////////////////////////////////////
int BackupFace(BOOL if_init_backup, WORD Destpart, CHAR16 *desc)
{
	struct BUTTON buttonctrl[1];
	//struct WINDOW win;

	PROCESSBOX progress;
	int nRet = -1;
	BYTE oldstate;
	RECT rc;
	//WORD x, y;

	CopyMem(&rc, &g_data.rWnd, sizeof(RECT));
	MyLog(LOG_DEBUG, L"----------BackupFace...\n");
	//MyLog(LOG_DEBUG, L"desc: %s.\n", desc);

	oldstate = EnableMouse(FALSE);
	hidemouse();

	float fMinscale = g_data.fMinscale;

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	DWORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;
	//DWORD dwBGTop = (g_WindowHeight - WHITE_BG_HEIGHT * fMinscale) / 2;

	//Backup button
	dwLeft = BACKUP_BUTTON_LEFT * fMinscale;
	dwTop = BACKUP_BUTTON_TOP * fMinscale;
	dwWidth = BACKUP_BUTTON_SIZE * fMinscale;
	dwHeight = BACKUP_BUTTON_SIZE * fMinscale;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = BACKUP_BUTTON_LEFT * 0.5;
		dwTop = BACKUP_BUTTON_TOP * 0.5;
		dwWidth = BACKUP_BUTTON_SIZE * 0.5;
		dwHeight = BACKUP_BUTTON_SIZE * 0.5;
	}

	hidemouse();

	DrawLine(dwLeft - 1, dwTop - 1, dwWidth + 2, dwHeight + 2, COLOR_DEFAULT_BACKGROUND);

	//WORD presspcxGoBack = GetImageFileId(IMG_FILE_BUTTON_GO_BACK_PRESS); //IMG_FILE_BUTTON_GO_BACK_PRESS;
	//WORD focuspcxGoBack = GetImageFileId(IMAG_FILE_GOBACK_FOCUS); //IMAG_FILE_GOBACK_FOCUS;
	//WORD unfocuspcxGoBack = GetImageFileId(IMG_FILE_GOBACK); //IMG_FILE_GOBACK;
	////if (0.5 >= fMinscale)
	////{
	////	presspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_PRESS;
	////	focuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_FOCUS;
	////	unfocuspcxGoBack = IMG_FILE_BUTTON_GO_BACK_S_UNFOCUS;
	////}
	//Button_init(&buttonctrl[0], dwLeft, dwTop, dwWidth, dwHeight,
	//	presspcxGoBack, focuspcxGoBack, unfocuspcxGoBack, 0);
	//Button_show((PCONTROL)&buttonctrl[0]);

	//buttonctrl[0].controlinfo.control_id = IDCANCEL;
	//buttonctrl[0].controlinfo.sendmessage = btn_default;

	//Window_init(&win, NULL);

	//ClearScreen();
	   
	DrawWhiteBK();
	   
	StringInfo stStringInfoBACKUP_SYSTEM;
	stStringInfoBACKUP_SYSTEM.enumFontType = FZLTZHJW;
	stStringInfoBACKUP_SYSTEM.iFontSize = CalcFontSize(90); //90 * g_data.fFontScale;

	dwWidth = 0;
	GetStringLen(STRING_BACKUP_SYSTEM, &stStringInfoBACKUP_SYSTEM, &dwWidth);
	dwLeft = (g_WindowWidth - dwWidth) / 2;
	dwTop = BK_PAGE_SUB_TITLE_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_SUB_TITLE_TOP - 150) * fMinscale;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_TITLE, STRING_BACKUP_SYSTEM, &stStringInfoBACKUP_SYSTEM, TRUE);

	StringInfo stStringInfoREMAIN_TIME;
	stStringInfoREMAIN_TIME.enumFontType = FZLTHJW;
	stStringInfoREMAIN_TIME.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;

	dwWidth = 0;
	GetStringLen(STRING_PREPARE_BACKUP, &stStringInfoREMAIN_TIME, &dwWidth);
	dwLeft = (g_WindowWidth - dwWidth) / 2; 
	dwTop = BK_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_REAMIN_TIME_TEXT_TOP - 150) * fMinscale;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_PREPARE_BACKUP, &stStringInfoREMAIN_TIME, TRUE);

	//dwLeft = (g_WindowWidth - (BK_PAGE_REAMIN_TIME_TEXT_WIDTH + BK_PAGE_REAMIN_TIME_VALUE_WIDTH + BK_PAGE_REAMIN_TIME_TEXT_VALUE_SPAN) * fMinscale) / 2;
	//dwTop = BK_PAGE_REAMIN_TIME_TEXT_TOP * fMinscale;
	////dwWidth = BK_PAGE_REAMIN_TIME_TEXT_WIDTH * fMinscale;
	////dwHeight = BK_PAGE_REAMIN_TIME_TEXT_HEIGHT * fMinscale;
	//StringInfo stStringInfoREMAIN_TIME;
	//stStringInfoREMAIN_TIME.enumFontType = FZLTHJW;
	//stStringInfoREMAIN_TIME.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
	//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_REMAIN_TIME, &stStringInfoREMAIN_TIME, TRUE);

	//dwLeft = (g_WindowWidth - (BK_PAGE_REAMIN_TIME_TEXT_WIDTH + BK_PAGE_REAMIN_TIME_VALUE_WIDTH + BK_PAGE_REAMIN_TIME_TEXT_VALUE_SPAN) * fMinscale) / 2 + (BK_PAGE_REAMIN_TIME_TEXT_WIDTH + BK_PAGE_REAMIN_TIME_TEXT_VALUE_SPAN) * fMinscale;
	//dwTop = BK_PAGE_REAMIN_TIME_VALUE_TOP * fMinscale;
	//dwWidth = BK_PAGE_REAMIN_TIME_VALUE_WIDTH * fMinscale + 100;
	//dwHeight = BK_PAGE_REAMIN_TIME_VALUE_HEIGHT * fMinscale + 10;
	//StringInfo stStringInfoREMAIN_TIME_VALUE;
	//stStringInfoREMAIN_TIME_VALUE.enumFontType = FZLTHJW;
	//stStringInfoREMAIN_TIME_VALUE.iFontSize = CalcFontSize(40); //40 * g_data.fFontScale;
	//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_REMAIN_TIME_VALUE, &stStringInfoREMAIN_TIME_VALUE, TRUE);
	
	StringInfo stStringInfoBACKUP_INFO;
	stStringInfoBACKUP_INFO.enumFontType = FZLTHJW;
	stStringInfoBACKUP_INFO.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	dwWidth = 0;
	GetStringLen(STRING_BACKUP_INFO, &stStringInfoBACKUP_INFO, &dwWidth);
	dwLeft = (g_WindowWidth - dwWidth) / 2;
	dwTop = BK_PAGE_INFO_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_INFO_TOP - 150) * fMinscale;
	}
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_BACKUP_INFO, &stStringInfoBACKUP_INFO, TRUE);

	////Do not display the info. [20210114]
	//dwLeft = (g_WindowWidth - BK_PAGE_INFO_TEXT_WIDTH * fMinscale) / 2;
	//dwTop = BK_PAGE_INFO_TEXT_TOP * fMinscale;
	//StringInfo stStringInfoCANCEL;
	//stStringInfoCANCEL.enumFontType = FZLTHJW;
	//stStringInfoCANCEL.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);
	//DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_CANCEL, &stStringInfoCANCEL, TRUE);

	dwLeft = (g_WindowWidth - BK_PAGE_PROGRESS_WIDTH * fMinscale) / 2;
	dwTop = BK_PAGE_PROGRESS_TOP * fMinscale;
	dwWidth = BK_PAGE_PROGRESS_WIDTH * fMinscale;
	dwHeight = PROGRESS_HEIGHT * fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwHeight = PROGRESS_HEIGHT * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_PROGRESS_TOP - 150) * fMinscale;
	}
	Processbox_init(&progress, dwLeft, dwTop, dwWidth, dwHeight);
	Processbox_show((PCONTROL)&progress);

	dwLeft = dwBGLeft;
	dwTop = BK_PAGE_TITLE_TOP * fMinscale;
	dwWidth = BK_PAGE_TITLE_WIDTH * fMinscale;
	dwHeight = BK_PAGE_TITLE_HEIGHT * fMinscale;

	WORD dwWidthBK = (MAIN_PAGE_TITLE_WIDTH + 10) * fMinscale;

	WORD unfocuspcxSysBk = GetImageFileId(IMAG_FILE_SYS_BK_TITLE); //IMAG_FILE_SYS_BK_TITLE;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = dwBGLeft;
		dwTop = (BK_PAGE_TITLE_TOP - 100) * 0.5;
		dwWidth = BK_PAGE_TITLE_WIDTH * 0.5;
		dwHeight = BK_PAGE_TITLE_HEIGHT * 0.5;
		dwWidthBK = (MAIN_PAGE_TITLE_WIDTH + 10) * 0.5;
	}
	else if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (BK_PAGE_TITLE_TOP - 150) * fMinscale;
	}

	DrawLine(dwLeft, dwTop, dwWidthBK, dwHeight, COLOR_DEFAULT_BACKGROUND);

	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxSysBk, FALSE);

	DisplayLogo();

	//showmouse();

	nRet = Backup(if_init_backup, Destpart, desc, (PCONTROL)&progress);

#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing disk...\n");
#endif
	GetDiskInfo();
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing partition...\n");
#endif
	GetPartitionInfo();

	EnableMouse(oldstate);

	showmouse();

	MyLog(LOG_DEBUG, L"DoBackup nRet:%d.\n", nRet);

	if (nRet < 0) 
	{
		return nRet;
	}
	else
	{
		RecoverCompleteUIMsgbox(USER_COMMAND_BACKUP);
	}

	return 0;
}


void ShowWaitMsgOnce(void)
{
	//MyLog(LOG_DEBUG, L"ShowWaitMsgOnce================ \n ");

	if (g_checkntfslog == 1)
	{
		if (g_ShowOnceWndBuff != NULL)
		{
			hidemouse();
			PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, g_ShowOnceWndBuff);
			FreePool(g_ShowOnceWndBuff);
			g_ShowOnceWndBuff = NULL;
			showmouse();
		}

		return;
	}
	if (g_ShowOnceWndBuff == NULL)
	{
		g_ShowOnceWndBuff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	}

	float fMinscale = g_data.fMinscale;

	WORD dwWidth = WAIT_MSG_WIDTH * fMinscale;
	WORD dwHeight = WAIT_MSG_HEIGHT * fMinscale;
	WORD dwLeft = (g_WindowWidth - dwWidth) / 2;
	WORD dwTop = (g_WindowHeight - dwHeight) / 2;

	hidemouse();

	GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, g_ShowOnceWndBuff);

	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);
	//DrawRect(0XFFFFFF, dwLeft, dwTop, dwWidth, dwHeight);
	DrawMsgBoxWhiteBK(dwLeft, dwTop, dwWidth, dwHeight);

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_MESSAGE_BOX_TEXT); //(int)(FONT_MESSAGE_BOX_TEXT * g_data.fFontScale + 0.5);

	DWORD dwStrLen = 0;
	GetStringLen(STRING_WAIT_NTFS_MSG, &stStringInfo, &dwStrLen);

	//MyLog(LOG_DEBUG, L"String Len: %d.\n", dwStrLen);

	WORD dwMsgWidth =  WAIT_MSG_TEXT_WIDTH * fMinscale;
	WORD dwMsgHeight = WAIT_MSG_TEXT_HEIGHT * fMinscale;
	dwLeft += (dwWidth - dwStrLen) / 2;
	dwTop += (dwHeight - stStringInfo.iFontSize) / 2;

	//PrintMessageInRectEx(dwLeft, dwTop, dwMsgWidth, dwMsgHeight, STRING_WAIT_NTFS_MSG, COLOR_TEXT_CONTENT, &stStringInfo);
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, STRING_WAIT_NTFS_MSG, &stStringInfo, TRUE);

	g_checkntfslog = 1;
	showmouse();
}

void ShowWaitMsgFace(CHAR16 *strMessage)
{
	if (g_showAdjustPartText == 1)
	{
		return;
	}

	if (g_ShowOnceWndBuff == NULL)
	{
		g_ShowOnceWndBuff = AllocatePool(g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	}

	SetMem(g_ShowOnceWndBuff, g_WindowWidth * g_WindowHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0);
	GetImgEx(0, 0, g_WindowWidth, g_WindowHeight, g_ShowOnceWndBuff);

	float fMinscale = g_data.fMinscale;

	WORD dwWidth = WAIT_MSG_WIDTH * fMinscale;
	WORD dwHeight = WAIT_MSG_HEIGHT * fMinscale;
	WORD dwLeft = (g_WindowWidth - dwWidth) / 2;
	WORD dwTop = (g_WindowHeight - dwHeight) / 2;

	hidemouse();
	DrawLineAlpha(0, 0, g_WindowWidth, g_WindowHeight, 8);
	DrawMsgBoxWhiteBK(dwLeft, dwTop, dwWidth, dwHeight);

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_MESSAGE_BOX_TEXT); //(int)(FONT_MESSAGE_BOX_TEXT * g_data.fFontScale + 0.5);

	DWORD dwStrLen = 0;
	GetStringLen(strMessage, &stStringInfo, &dwStrLen);

	//MyLog(LOG_DEBUG, L"String Len: %d.\n", dwStrLen);

	WORD dwMsgWidth = WAIT_MSG_TEXT_WIDTH * fMinscale;
	WORD dwMsgHeight = WAIT_MSG_TEXT_HEIGHT * fMinscale;

	dwLeft += (dwWidth - dwStrLen) / 2;
	dwTop += (dwHeight - stStringInfo.iFontSize) / 2;

	//PrintMessageInRectEx(dwLeft, dwTop, dwMsgWidth, dwMsgHeight, strMessage, COLOR_TEXT_CONTENT, &stStringInfo);
	DisplayStringEx(dwLeft, dwTop, COLOR_TEXT_CONTENT, strMessage, &stStringInfo, TRUE);

	g_showAdjustPartText = 1;
	showmouse();
}

void DonotShowWaitMsgFace()
{
	if (g_ShowOnceWndBuff != NULL)
	{
		hidemouse();
		PutImgEx(0, 0, g_WindowWidth, g_WindowHeight, g_ShowOnceWndBuff);
		FreePool(g_ShowOnceWndBuff);
		g_ShowOnceWndBuff = NULL;
		showmouse();
	}
}

int Face()
{
	//MyLog(LOG_DEBUG, L"====Face Start");
	int iReturnValue = -1;  //return value, reboot or shutdown

    int func=0; //0,backup,
    DWORD ret;
	CHAR16 backup_desc[100] = {0};
    int backupfile_partindex=-1;
    int disknum=-1;
    int recoverfile_partindex=-1;
    DWORD backupfileid=0;
    BOOLEAN bComplete = FALSE;
    int result = 0; 
    
	while (!bComplete)
	{
		g_data.bFactoryRestore = FALSE;	//复位bFactoryRestore标志

		//画背景图
		ClearScreen();

		//ret = StartFace(); //第一个提示框
		ret = MainFace();

#if OKR_DEBUG
		MyLog(LOG_DEBUG, L"User select command(%d) to continue.\n", ret);
#endif

		if (ret == IDB_BACKUP)
		{
			func = USER_COMMAND_BACKUP;
			g_checkntfslog = 0;
		}
		else if (ret == IDB_RECOVER)
		{
			func = USER_COMMAND_RECOVER;
			g_checkntfslog = 0;
		}
		else if (ret == IDB_RECOVER_FACTORY)
		{
			func = USER_COMMAND_RECOVER;
			g_checkntfslog = 0;
		}
		//else if (ret == IDB_DRIVER_INSTALL)
		//{
		//	func = USER_COMMAND_DRIVER_INSTALL;
		//	// MyTestReadWriteFileOrDir(1);
		//	// DriverInstallFace();
		//	// DriverInstall();

		//	// MyTestReadWriteFileOrDir(3);
		//	// getch();
		//	// break;
		//}
		else if (ret == IDB_SHUTDOWN || ret == IDB_REBOOT)
		{
			iReturnValue = ret;
			break;
		}
		else if (ret == IDB_ABOUT)
		{
			ShowAboutFace(FALSE);
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
			if (func == USER_COMMAND_BACKUP)
			{
				int destpart = -1;
				int partindex = -1;

				//g_checkntfslog = 0;
				ShowWaitMsgOnce();

				///判断是否有Lenovo_part分区
				//如果存在，判断分区是否包含初始备份点				
				destpart = DetectAvailableOEMPartition(&partindex);
				MyLog(LOG_DEBUG, L"DetectAvailableOEMPartition ret: %d.\n", destpart);
				if (destpart < 0)
				{
					//如果不存在，直接进入备份页面
					MyLog(LOG_DEBUG, L"Does not exist Lenovo Part, enter backup face.\n");
					goto nextbkface;
				}

				//首先判断是否有初始备份点
				result = InitialBackupFace(&backupfile_partindex);
				MyLog(LOG_DEBUG, L"InitialBackupFace return : %d.\n", result);
				if (result == -2)
				{
					//存在初始备份点
					MyLog(LOG_DEBUG, L"Exist init backup.\n");
				}
				else if (result == IDB_IDB_INIT_BACKUP)
				{
					//不存在初始备份点，显示初始备份页面，创建初始备份点
					MyLog(LOG_DEBUG, L"Does not exist init backup.\n");

					//	CreateTestTmpFile(backupfile_partindex);
					result = BackupFace(TRUE, (WORD)backupfile_partindex, L"Initbackup");
					MyLog(LOG_DEBUG, L"BackupFace(init) return : %d.\n", result);
					if (result < 0)
					{						
						break;
					}
					else
					{
						//RecoverCompleteUIMsgbox(USER_COMMAND_BACKUP);
						break;
					}
				}
				else if (result == 4)
				{
					MyLog(LOG_DEBUG, L"OEM partition is too small to backup. Continue to backup.\n");
				}
				else
				{
					break;
				}
			nextbkface:                
				//备份镜像的存储分区，备份镜像的说明信息
				//if (BackupSetting(backup_desc, &disknum) != IDB_BACKUP)
				//	break;
				//result = SelectPartFace(disknum, &backupfile_partindex);
				result = SelectBackupPartFace(backup_desc, &backupfile_partindex);
				//MyLog(LOG_DEBUG, L"SelectBackupPartFace return : %d.\n", result);
				if (result != IDB_BACKUP)
				{
					MyLog(LOG_DEBUG, L"SelectBackupPartFace result != IDB_BACKUP\n");
					break;
				}

				result = BackupFace(FALSE, (WORD)backupfile_partindex, backup_desc);
				MyLog(LOG_DEBUG, L"BackupFace return : %d.\n", result);
				if (result < 0)
				{
					continue;
				}
				else if (result == 0)
				{
					break;
				}				
			}
			else if (func == USER_COMMAND_RECOVER) {
				//g_checkntfslog = 0;
				ShowWaitMsgOnce();
				//选择镜像文件
				//if (RecoverSettingFace(&recoverfile_partindex, &backupfileid) != IDB_RECOVER)
				if (SelectRecoveryPointFace(&recoverfile_partindex, &backupfileid) != IDB_RECOVER)					
				{
					MyLog(LOG_DEBUG, L"SelectRecoveryPointFace return != IDB_RECOVER.\n");
					break;
				}					

				result = RecoveryFace((WORD)recoverfile_partindex, backupfileid);
				MyLog(LOG_DEBUG, L"RecoveryFace return : %d.\n", result);
				if (result < 0)
				{
					if (g_data.bFactoryRestore)
						break;
					continue;
				}
				else if (result == 0)
				{
					break;
				}
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

		//if (bComplete && func != USER_COMMAND_DRIVER_INSTALL)
		//{
		//	//RecoverCompleteUI(func);
		//	RecoverCompleteUIMsgbox(func);
		//}
	}

	ReleaseUnusedDataList();
	return iReturnValue;
}

void DisplayLogo()
{
	float fMinscale = g_data.fMinscale;
	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	//logo
	dwLeft = g_WindowWidth - (WORD)(LOGO_WIDTH * fMinscale + 0.5);
	dwTop = g_WindowHeight - (WORD)((LOGO_BOTTOM + LOGO_HEIGHT) * fMinscale + 0.5);
	dwWidth = (WORD)(LOGO_WIDTH * fMinscale + 0.5);
	dwHeight = (WORD)(LOGO_HEIGHT * fMinscale + 0.5);
	WORD unfocuspcxLogo = GetImageFileId(IMG_FILE_LOGO); //IMG_FILE_LOGO;

	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		dwLeft = g_WindowWidth - LOGO_WIDTH * 0.5;
		dwTop = g_WindowHeight - (LOGO_BOTTOM + LOGO_HEIGHT) * 0.5;
		dwWidth = LOGO_WIDTH * 0.5;
		dwHeight = LOGO_HEIGHT * 0.5;
	}
	//MyLog(LOG_MESSAGE, L"x: %d y: %d Width: %d Height: %d", dwLeft, dwTop, dwWidth, dwHeight);
	//MyLog(LOG_MESSAGE, L"fileid: %d", unfocuspcxLogo);
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxLogo, FALSE); // logo
}

void DrawWhiteBK()
{
	float fMinscale = g_data.fMinscale;

	WORD dwBGLeft = (g_WindowWidth - WHITE_BG_WIDTH * fMinscale) / 2;

	WORD dwLeft = 0;
	WORD dwTop = 0;
	WORD dwWidth = 0;
	WORD dwHeight = 0;

	dwLeft = dwBGLeft;
	dwTop = WHITE_BG_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150) * fMinscale;
	}

	dwWidth = WHITE_BG_WIDTH * fMinscale;
	dwHeight = WHITE_BG_HEIGHT * fMinscale;

	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);
	//DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, IMG_FILE_PAGE_BG, FALSE); // logo

	dwWidth = (int)(WHITE_BG_CORNER_SIZE * fMinscale + 0.5); //WHITE_BG_CORNER_SIZE * fMinscale;
	dwHeight = (int)(WHITE_BG_CORNER_SIZE * fMinscale + 0.5); //WHITE_BG_CORNER_SIZE * fMinscale;

	dwLeft = dwBGLeft;
	dwTop = WHITE_BG_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150) * fMinscale;
	}
	WORD unfocuspcxLT = GetImageFileId(IMG_FILE_WHITE_BG_LEFT_TOP); //IMG_FILE_WHITE_BG_LEFT_TOP;
	WORD unfocuspcxLB = GetImageFileId(IMG_FILE_WHITE_BG_LEFT_BOTTOM); //IMG_FILE_WHITE_BG_LEFT_BOTTOM;
	WORD unfocuspcxRT = GetImageFileId(IMG_FILE_WHITE_BG_RIGHT_TOP); //IMG_FILE_WHITE_BG_RIGHT_TOP;
	WORD unfocuspcxRB = GetImageFileId(IMG_FILE_WHITE_BG_RIGHT_BOTTOM); //IMG_FILE_WHITE_BG_RIGHT_BOTTOM;
	//if (0.5 >= fMinscale)
	//{
	//	unfocuspcxLT = IMG_FILE_WHITE_BG_LEFT_TOP_S;
	//	unfocuspcxLB = IMG_FILE_WHITE_BG_LEFT_BOTTOM_S;
	//	unfocuspcxRT = IMG_FILE_WHITE_BG_RIGHT_TOP_S;
	//	unfocuspcxRB = IMG_FILE_WHITE_BG_RIGHT_BOTTOM_S;
	//}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxLT, FALSE);

	dwLeft = dwBGLeft;
	dwTop = (WHITE_BG_HEIGHT - WHITE_BG_CORNER_SIZE + WHITE_BG_TOP) * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_HEIGHT - WHITE_BG_CORNER_SIZE + WHITE_BG_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxLB, FALSE);

	dwLeft = dwBGLeft + WHITE_BG_WIDTH * fMinscale - dwWidth;
	dwTop = WHITE_BG_TOP * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxRT, FALSE);

	dwLeft = dwBGLeft + WHITE_BG_WIDTH * fMinscale - dwWidth;
	dwTop = (WHITE_BG_HEIGHT - WHITE_BG_CORNER_SIZE + WHITE_BG_TOP) * fMinscale;
	if (1440 == g_WindowWidth && 900 == g_WindowHeight)
	{
		dwTop = (WHITE_BG_HEIGHT - WHITE_BG_CORNER_SIZE + WHITE_BG_TOP - 150) * fMinscale;
	}
	DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, unfocuspcxRB, FALSE);
}

void DrawMsgBoxWhiteBK(WORD dwBoxLeft, WORD dwBoxTop, WORD dwBoxWidth, WORD dwBoxHeight)
{
	float fMinscale = g_data.fMinscale;

	WORD dwLeft = dwBoxLeft;
	WORD dwTop = dwBoxTop;
	WORD dwWidth = dwBoxWidth;
	WORD dwHeight = dwBoxHeight;

	dwLeft = dwBoxLeft + (WHITE_BG_CORNER_SIZE - 10) * fMinscale;
	dwTop = dwBoxTop;
	dwWidth = dwBoxWidth - (WHITE_BG_CORNER_SIZE - 10) * 2 * fMinscale;
	dwHeight = dwBoxHeight;
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

	dwLeft = dwBoxLeft;
	dwTop = dwBoxTop + (WHITE_BG_CORNER_SIZE - 10) * fMinscale;
	dwWidth = dwBoxWidth;
	dwHeight = dwBoxHeight - (WHITE_BG_CORNER_SIZE - 10) * 2 * fMinscale;
	DrawLine(dwLeft, dwTop, dwWidth, dwHeight, 0xFFFFFF);

	//DisplayImgEx(dwLeft, dwTop, dwWidth, dwHeight, IMG_FILE_PAGE_BG, FALSE); // logo

	dwWidth = WHITE_BG_CORNER_SIZE * fMinscale;
	dwHeight = WHITE_BG_CORNER_SIZE * fMinscale;

	dwLeft = dwBoxLeft;
	dwTop = dwBoxTop;
	DisplayImgFilterColorEx(dwLeft, dwTop, dwWidth, dwHeight, IMG_FILE_WHITE_BG_LEFT_TOP, FALSE);

	dwLeft = dwBoxLeft;
	dwTop = dwBoxTop + dwBoxHeight - WHITE_BG_CORNER_SIZE * fMinscale;
	DisplayImgFilterColorEx(dwLeft, dwTop, dwWidth, dwHeight, IMG_FILE_WHITE_BG_LEFT_BOTTOM, FALSE);

	dwLeft = dwBoxLeft + dwBoxWidth - WHITE_BG_CORNER_SIZE * fMinscale;
	dwTop = dwBoxTop;
	DisplayImgFilterColorEx(dwLeft, dwTop, dwWidth, dwHeight, IMG_FILE_WHITE_BG_RIGHT_TOP, FALSE);

	dwLeft = dwBoxLeft + dwBoxWidth - WHITE_BG_CORNER_SIZE * fMinscale;
	dwTop = dwBoxTop + dwBoxHeight - WHITE_BG_CORNER_SIZE * fMinscale;
	DisplayImgFilterColorEx(dwLeft, dwTop, dwWidth, dwHeight, IMG_FILE_WHITE_BG_RIGHT_BOTTOM, FALSE);
}
