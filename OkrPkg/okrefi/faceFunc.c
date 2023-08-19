/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#include "faceFunc.h"
#include "okr.h"

int CheckSystemPartition()
{
	int iRet = 0;

	int result = ProbeSystemPartition();
	//if (result == FALSE)
	if (result == 0)
	{
		////关闭提示框
		ShowWaitMsgOnce();
		DrawMsgBoxEx(STRING_HINTNOSYSPART, IDOK, IDCANCEL, TYPE_INFO);
		return -1;
	}
	else if (result == -1)
	{
		////关闭提示框
		ShowWaitMsgOnce();
		DrawMsgBoxEx(STRING_FIND_SYS_ERROR, IDOK, IDCANCEL, TYPE_INFO);
		return -1;
	}
	else if (result == -2)
	{
		////关闭提示框
		if (g_data.bIsAutoFunc)
		{
			//DrawMsgBoxTwoLine(STRING_BITLOCKER_NO_SYSTEM, STRING_CLOSE_BITLOCKER, IDB_AUTO_REBOOT, -1, TYPE_INFO);
			DrawMsgBoxEx(STRING_BITLOCKER_NO_SYSTEM, IDB_AUTO_REBOOT, -1, TYPE_INFO);
		}
		else
		{
			ShowWaitMsgOnce();
			DrawMsgBoxTwoLine(STRING_BITLOCKER_NO_SYSTEM, STRING_CLOSE_BITLOCKER, IDOK, -1, TYPE_INFO);
		}
		//DrawMsgBoxEx(STRING_BITLOCKER_NO_SYSTEM, IDOK, -1, TYPE_INFO);
		return -1;
	}
	else if (result == -3)
	{
		////关闭提示框
		ShowWaitMsgOnce();
		DrawMsgBoxEx(STRING_MULTI_OS, IDOK, -1, TYPE_INFO);
		return -1;
	}

	return iRet;
}

int IsHasLabelTextData(int iPartitionIndex)
{
	int bHasLabelTextData = 0;

	if (g_Partitions[iPartitionIndex].Label != NULL && g_Partitions[iPartitionIndex].Label[0] != 0)
	{
		StringInfo stStringInfo;
		DWORD dwCharLen = 0;
		stStringInfo.enumFontType = FZLTHJW;
		stStringInfo.iFontSize = CalcFontSize(FONT_LIST_ITEM_TEXT_SIZE);

		for (int iFlag = 0; iFlag < StrLen(g_Partitions[iPartitionIndex].Label); iFlag++)
		{
			dwCharLen = GetCharWidth(g_Partitions[iPartitionIndex].Label[iFlag], &stStringInfo);
			//MyLog(LOG_DEBUG, L"Label len %d par %d pos %d char: %c\n", dwCharLen, iPartitionIndex, iFlag, g_Partitions[iPartitionIndex].Label[iFlag]);
			if (dwCharLen == 0)
			{
				break;
			}
		}

		if (0 != dwCharLen)
		{
			bHasLabelTextData = 1;
		}
	}

	return bHasLabelTextData;
}
