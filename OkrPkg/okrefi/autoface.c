#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "okr.h"
#include "autofacefunc.h"
#include "autoface.h"
#include "commondef.h"
#include "func.h"



UINT32 g_databitmapbufsizeautoface;
INTN  g_lz4_sizeautoface;
PBYTE g_lz4_bufferautoface = NULL;

DWORD g_auto_show_total = 0;	//MB
DWORD g_auto_shown_unit = 0;	//MB
DWORD g_auto_shown_count = 0;	//KB
DWORD g_auto_show_interval = 0;
DWORD g_auto_show_percent = 0;
PCONTROL g_auto_showctrl = NULL;

DWORD pre_auto_show_percent = 0;

DWORD g_auto_update_backup = 1;

int AutoBackupFace()
{
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing disk...\n");
#endif
	GetDiskInfo();
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing partition...\n");
#endif
	GetPartitionInfo();

	int iReturnValue = IDB_REBOOT;  //return value, reboot or shutdown
	g_data.bFactoryRestore = FALSE;	//复位bFactoryRestore标志
	
	//Test Block
	/*
	g_data.bDebug = TRUE;
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
	*/
	//设置整个背景为蓝色
	ClearScreen();
	//绘制出系统备份的界面
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

	//开始执行自动备份的逻辑
	//判断系统分区能否进行备份，可能的情况：无系统分区、多系统分区、系统分区存在BitLocker
	if (0 != CheckSystemPartition())
	{
		return iReturnValue;
	}
	g_data.TotalBackupSize = GetBackupTotaldata();
	
	//showmouse();

	BOOL if_init_backup = FALSE;	//是否为出厂备份的标识
	int iDestpartIndex = 0;	//备份到的分区索引
	CHAR16 desc[100] = L"Initbackup";	//备份文件的描述
	BOOL bEnterBackupFunc = FALSE; //是否进入备份动作
	BOOL bNeedJudgeDataPartition = FALSE;                            
	//判断是否存在出厂备份，通过参数带回服务分区的索引
	//有文件系统初始化失败的情况，通过Log输出错误情况
	int iLenovoPartIndex = 0;
	if (!AutoBRHasFactoryBackup(&iLenovoPartIndex))
	{
		//不存在出厂备份时，保存镜像到联想服务分区
		//能否存储下备份文件
		//约0.7的压缩比， 512 / 70% = 731
		//
		MyLog(LOG_DEBUG, L"[AUTOBR]No factory img, backup to len part.\n");
		if (IsPartHasEnoughSpace(iLenovoPartIndex))
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace has enough space in Lenovo Part.\n");
			//设置存储目标为联想服务分区
			iDestpartIndex = iLenovoPartIndex;
			//将备份标记，设为自动出厂备份
			CHAR16 descinit[] = L"Initial Autobackup";
			CopyMem(desc, descinit, sizeof(descinit));
			if_init_backup = TRUE;
			bEnterBackupFunc = TRUE;
		}
		else
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace not enough space in Lenovo Part.\n");
			//服务分区下没有足够空间存储镜像，继续判断是否存在数据分区
			bNeedJudgeDataPartition = TRUE;
		}
	}
	else
	{
		//存在出厂备份时，继续判断是否存在数据分区
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace has factory img in Lenovo Part.\n");
		bNeedJudgeDataPartition = TRUE;
	}
	//getch();
	//需要判断是否存在数据分区的情况
	if (bNeedJudgeDataPartition)
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace Enter NeedJudgeAvailableDataPartition.\n");
		BOOL bBackupAvailable = FALSE;
		int iAvailablePartIndex = 0;
		BOOL bPartHasBitlocker = FALSE;
		//获取有效的数据分区
		BOOL bHasDataPart = AutoBRDetectAvailableDataPartition(&iAvailablePartIndex, &bPartHasBitlocker, &bBackupAvailable);
		//getch();
		if (bHasDataPart)
		{
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace bHasDataPart.\n");
			//能否存储下备份文件
			if (bBackupAvailable)
			{
				//能存储下
				//设置存储目标为这个分区
				iDestpartIndex = iAvailablePartIndex;
				//标记为自动备份
				CHAR16 descauto[] = L"Autobackup";
				CopyMem(desc, descauto, sizeof(descauto));
				if_init_backup = FALSE;
				bEnterBackupFunc = TRUE;
				MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace Can EnterBackupFunc use data part iDestpartIndex：%d, desc:%s.\n", iDestpartIndex, desc);
			}
			else
			{
				//存储不下
				//数据分区是否存在BitLocker
				if (bPartHasBitlocker)
				{
					//提示用户
					//DivU64x32(g_data.TotalBackupSize, 731);
					CHAR16 aTip[165] = { 0 };
					UINT64 data = 0;
					data = DivU64x32(g_data.TotalBackupSize, 1024 * 1024);
					SPrint(aTip, 165, STRING_AUTOBR_DATAPART_BITLOCKED, data / 1462);   //约0.7的压缩比， 1024 / 0.7 = 1462
					DrawMsgBoxEx(aTip, IDOK, -1, TYPE_INFO);
					//logflush(FALSE);
					return iReturnValue;
				}
				else
				{
					//提示用户
					MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace show messagebox, not enough data part for backup.\n");
					DrawMsgBoxEx(STRING_AUTOBR_DATAPART_NOTENOUGH, IDOK, -1, TYPE_INFO);
					//logflush(FALSE);
					return iReturnValue;
				}
			}
			//getch();
		}
		else
		{
			//提示用户
			//没有数据分区，提醒用户插入移动存储设备进行备份
			MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace show messagebox, no data part, please insert mobile data device.\n");
			DrawMsgBoxEx(STRING_AUTOBR_DATAPART_NOTEXIST, IDOK, -1, TYPE_INFO);
			//logflush(FALSE);
			return iReturnValue;
		}
	}
	MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace bEnterBackupFunc:%d\n", bEnterBackupFunc);
	if (bEnterBackupFunc)
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]iDestpartIndex:%d\n", iDestpartIndex);
		MyLog(LOG_DEBUG, L"[AUTOBR]desc:%s\n", desc);
		MyLog(LOG_DEBUG, L"[AUTOBR]Call BACKUP if_init_backup:%d, Destpart:%d, desc:%s\n", if_init_backup, iDestpartIndex, desc);
		//getch();
		nRet = Backup(if_init_backup, iDestpartIndex, desc, (PCONTROL)&progress);
	}
	
	MyLog(LOG_DEBUG, L"DoBackup nRet:%d.\n", nRet);

	if (nRet == 0)
	{
		//linwtodo：存储备份文件的保存到路径信息
		//GetDiskInfo();
		//GetPartitionInfo();
		//getch();
			//从配置文件获取之前备份文件的路径
	//linwtodo：删除前一个自动备份的镜像
		DeletePreAutoBackupFile();
		SaveAutoBackupFileLocation(iDestpartIndex);
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackup Succed!!!\n");
		RecoverCompleteUIMsgbox(USER_COMMAND_BACKUP);
	}
	else
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackup Failed!!!\n");
	}

	MyLog(LOG_DEBUG, L"[AUTOBR]AutoBackupFace iReturnValue:%d.\n", iReturnValue);
	//logflush(FALSE);
	//getch();
	//usleep(3000);
	return iReturnValue;
}

int AutoRestoreFace()
{
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing disk...\n");
#endif
	GetDiskInfo();
#if OKR_DEBUG
	MyLog(LOG_DEBUG, L"Analyzing partition...\n");
#endif
	GetPartitionInfo();

	int iReturnValue = IDB_REBOOT;  //return value, reboot or shutdown
	g_data.bFactoryRestore = FALSE;	//复位bFactoryRestore标志

	//Test Block
	/*
	g_data.bDebug = TRUE;
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
	*/

	MyLog(LOG_DEBUG, L"[AUTOBR]Enter AutoRestoreFace.\n");
	//设置整个背景为蓝色
	ClearScreen();
	//绘制出系统恢复的界面
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
	g_data.enumGSKUSkipType = DISABLE_NEW_HDD_FUNC;
	g_data.bIsGSKU = FALSE;
	//g_data.bIsGSKU = IsGSKU();
	

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
	//getch();
	WORD recoverfilepartindex = 1;
	DWORD backupfileID = 1;
	int result = ProbeSystemPartition();
	if (result == -3)
	{
		////关闭提示框
		//ShowWaitMsgOnce();
		DrawMsgBoxEx(STRING_MULTI_OS_RECOVER, IDOK, -1, TYPE_INFO);
		return iReturnValue;
	}

	if (TRUE == g_data.bForceRestorePartition)
	{
		if (DrawMsgBoxEx(STRING_AUTOBR_FORCE_RECOVERY, IDOK, -1, TYPE_INFO) != IDOK)
		{
			MyLog(LOG_DEBUG, L"Cancel force recovery\n");
			return iReturnValue;
		}
	}
	
	if (DetectRestoreImge(&recoverfilepartindex, &backupfileID))
	{
		MyLog(LOG_DEBUG, L"recoverfilepartindex:%d, backupfileID:%d\n", recoverfilepartindex, backupfileID);
		MyLog(LOG_DEBUG, L"DetectRestoreImge %d(%s) to restore.\n", backupfileID, g_data.images[backupfileID].FileName);
		FindNewHdd(recoverfilepartindex, backupfileID);	//通过此函数将g_data.CurImageSyspartitionsize初始化	
		g_data.comefromnewhddface = 0;	//自动恢复过程，不支持恢复到新硬盘，将此标记设置为0
		nRet = Recover(recoverfilepartindex, backupfileID, (PCONTROL)&progress);
	}
	//getch();
	//EnableMouse(oldstate);
	//showmouse();

	MyLog(LOG_DEBUG, L"DoRecovery nRet:%d.\n", nRet);

	if (nRet == 0)
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoRecover Succed!!!\n");
		RecoverCompleteUIMsgbox(USER_COMMAND_RECOVER);
	}
	else
	{
		MyLog(LOG_DEBUG, L"[AUTOBR]AutoRecover Failed!!!\n");
	}
	
	MyLog(LOG_DEBUG, L"[AUTOBR]AutoRestoreFace iReturnValue:%d.\n", iReturnValue);
	//logflush(FALSE);
	return iReturnValue;
}

int ForceRestoreFace()
{
	MyLog(LOG_DEBUG, L"[AUTOBR]Enter ForceRestoreFace.\n");
	g_data.bForceRestorePartition = TRUE;
	MyLog(LOG_DEBUG, L"[AUTOBR]Set g_data.bForceRestorePartition = TRUE call AutoRestoreFace.\n");
	return AutoRestoreFace();
}