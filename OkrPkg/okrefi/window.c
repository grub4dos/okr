/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

// mouse contrl and the window 
#include "okr.h"
#include "faceUIDefine.h"
 
INT32 g_mouse_x=100;  //last position
INT32 g_mouse_y=100;
INT32 g_mouse_z = 0;
BYTE g_mouse_show = FALSE;		//当前是否显示了鼠标
BYTE g_mouse_enable = TRUE;		//当前鼠标的状态，是否允许使用
BOOLEAN g_mouse_oldlbutton = FALSE;
BOOLEAN g_mouse_oldrbutton = FALSE;
VOID *g_mouse_buf = NULL;

EFI_KEY_DATA gKeyData = {0};	//上一次调用getch获得的键值
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *gpSimpleEx = NULL;

struct CURSOR_INFO Cursor_normal_4k ={ IMG_FILE_MOUSEA_4K,24,42,0,0 };

struct CURSOR_INFO Cursor_normal = { IMG_FILE_MOUSEA,12,21,0,0 };
struct CURSOR_INFO Cursor_text={IMG_FILE_CURSORTEXT,9,19,4,9};
struct CURSOR_INFO Cursor_move={IMG_FILE_CURSORMOVE,19,9,9,4}; 
struct CURSOR_INFO Cursor_hand={IMG_FILE_CURSORHAND,17,22,5,0};

struct CURSOR_INFO *g_current_cursor=&Cursor_normal;

//WORD g_pixelPerChar = 8;
BOOL g_isListViewUpdating = FALSE;

WORD g_ProgressPos = 0;

time_t  g_lastESCTime = 0;

time_t  g_lastCountDownTime = 0;
int g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;

BOOL MouseInit()
{
    WORD ret=0;
    EFI_STATUS status;
    EFI_SIMPLE_POINTER_PROTOCOL *mouse=NULL;
    EFI_SIMPLE_POINTER_STATE mousestate;

	g_mouse_x = g_WindowWidth / 2;
	g_mouse_y = g_WindowHeight / 2;
	g_mouse_show = FALSE;
	
    status=gBS->LocateProtocol(&gEfiSimplePointerProtocolGuid,NULL,&mouse);
    if (!EFI_ERROR(status))
    {
        INT32 mouseresX,mouseresY;
        
        mouseresX=(INT32)mouse->Mode->ResolutionX;
        mouseresY=(INT32)mouse->Mode->ResolutionY;
            
        SetMem(&mousestate,sizeof(EFI_SIMPLE_POINTER_STATE),0);
        
        status=mouse->GetState(mouse,&mousestate);
        
        if (!EFI_ERROR(status)) 
        {
#if OKR_DEBUG
			MyLog(LOG_DEBUG, L"mouseres(%d,%d), RelativeMovement(%d,%d), Button(%d,%d)\n",
				mouseresX, mouseresY,
				mousestate.RelativeMovementX,
				mousestate.RelativeMovementY,
				mousestate.LeftButton,
				mousestate.RightButton);
#endif
        }
		//else
		//{
		//	MyLog(LOG_ERROR, L"GetState failed with error 0x%x, x %d, y %d\n",
		//		status, mouseresX, mouseresY);
		//}
	}
	else
	{
		MyLog(LOG_ERROR, L"LocateProtocol(SimplePointerProtocol) failed with error 0x%x\n",
			status );
	}
	
//#if OKR_DEBUG
//	getch();
//#endif
    return TRUE;
}

void MouseRelease()
{
	if(g_mouse_buf) {
		FreePool(g_mouse_buf);
	}
	g_mouse_buf = NULL;
}

BYTE EnableMouse(BYTE bEnable)
{
	BYTE oldstate = g_mouse_enable;
	g_mouse_enable = bEnable;
	return oldstate;
}

void adjustmousepos(WORD *x,WORD *y)
{
	if (g_data.fMinscale == 1)
	{
		g_current_cursor = &Cursor_normal_4k;
	}
	else
	{
		g_current_cursor = &Cursor_normal;
	}

    if ((*x) < g_current_cursor->focus_x )
        (*x) = 0;
    else
        (*x) -= g_current_cursor->focus_x;

    if ((*y) < g_current_cursor->focus_y )
        (*y) = 0;
    else
        (*y) -= g_current_cursor->focus_y;
    return;
}

//恢复鼠标经过的地方
BYTE hidemouse()
{
	BYTE bMouseDisplayed = g_mouse_show;
	
	if(g_mouse_show)
	{
    	WORD x = (WORD)g_mouse_x;
    	WORD y = (WORD)g_mouse_y;
    	
    	adjustmousepos(&x,&y);
    	
    	if (g_mouse_buf!=NULL) 
		{
			if (g_data.fMinscale == 1)
			{
				g_current_cursor = &Cursor_normal_4k;
			}
			else
			{
				g_current_cursor = &Cursor_normal;
			}
    	    PutImgEx(x,y,g_current_cursor->width,g_current_cursor->height,g_mouse_buf);
    	}
    }
    	
   	//恢复了后鼠标就处于没有显示的状态
   	g_mouse_show = FALSE;
   	
   	return bMouseDisplayed;
}

//保护mouse图片所在的区域
void Savemouse()
{
    WORD x = (WORD)g_mouse_x;
    WORD y = (WORD)g_mouse_y;

    adjustmousepos(&x,&y);

    if (g_mouse_buf==NULL) {
        g_mouse_buf=AllocatePool(4*100*100);
    }
	if (g_data.fMinscale == 1)
	{
		g_current_cursor = &Cursor_normal_4k;
	}
	else
	{
		g_current_cursor = &Cursor_normal;
	}
    GetImgEx(x,y,g_current_cursor->width,g_current_cursor->height,g_mouse_buf);
}

//显示mouse, 返回显示鼠标前的状态
BYTE showmouse()
{
	if(!g_mouse_show && g_mouse_enable)
	{
    	WORD x=(WORD)g_mouse_x;
    	WORD y=(WORD)g_mouse_y;
    	
    	Savemouse();
    	
    	adjustmousepos(&x,&y);
    	
		if (g_data.fMinscale == 1)
		{
			g_current_cursor = &Cursor_normal_4k;
		}
		else
		{
			g_current_cursor = &Cursor_normal;
		}
    	DisplayImg(x,y,g_current_cursor->pcx,TRUE);
    	
    	g_mouse_show = TRUE;
    	return FALSE;
    }
    else
    	return g_mouse_show;
}

//
BOOL KeyboardInit()
{
	//EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *simpleEx;
	EFI_STATUS          status;

	if(!gpSimpleEx)
	{
		status = gBS->OpenProtocol(
			gST->ConsoleInHandle,
			&gEfiSimpleTextInputExProtocolGuid,
			(VOID**)&gpSimpleEx,
			g_data.ImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR(status)) 
			gpSimpleEx = NULL;
		//gbKbhit = FALSE;
	}

	if(gpSimpleEx)
		return TRUE;
	else
		return FALSE;
}

VOID ClearKeyboard()
{
	//EFI_STATUS status;
	//gST->ConIn->Reset(gST->ConIn, FALSE);
	while(gST->ConIn->ReadKeyStroke(gST->ConIn, &gKeyData.Key) == EFI_SUCCESS);
	//MyLog(LOG_DEBUG, L"====ClearKeyboard ScanCode=%04x, UnicodeChar=%04x", gKeyData.Key.ScanCode, gKeyData.Key.UnicodeChar);
}

BOOL IsAltPressed()
{
	if(gKeyData.KeyState.KeyShiftState & EFI_SHIFT_STATE_VALID)
	{
		if(gKeyData.KeyState.KeyShiftState & (EFI_RIGHT_ALT_PRESSED | EFI_LEFT_ALT_PRESSED))
			return TRUE;
	}
	return FALSE;
}

BOOL IsCtrlPressed()
{
	if(gKeyData.KeyState.KeyShiftState & EFI_SHIFT_STATE_VALID)
	{
		if(gKeyData.KeyState.KeyShiftState & (EFI_RIGHT_CONTROL_PRESSED | EFI_LEFT_CONTROL_PRESSED))
			return TRUE;
	}
	return FALSE;
}

//当键盘有按键的时候或产生鼠标中断后退出改函数 bison
//输入无
//输出当键盘有按键的时候返回键盘扫描码或鼠标状态字
//timeout: 超时返回WM_TIMEOUT消息，以秒为单位。=0时不检查超时。
void detectmessage(struct DM_MESSAGE *pdmmessage,EFI_SIMPLE_POINTER_PROTOCOL *mouse) //直接将鼠标handle传入，改动最小，影响最小
{
    WORD ret=0;
    EFI_STATUS status;
	UINTN i=0;
    //EFI_SIMPLE_POINTER_PROTOCOL *mouse=NULL;
    EFI_SIMPLE_POINTER_STATE mousestate;

    pdmmessage->message=WM_NOMESSAGE;

   // status=gBS->LocateProtocol(&gEfiSimplePointerProtocolGuid,NULL,&mouse);
    //if (!EFI_ERROR(status)) {

    //} else {
    //    //	Print(L"Can't find mouse");
    //}
    
    for (i=0;i<2;i++) { //将原来的无限循环改为有限循环，单鼠标可能没问题，但是多鼠标状态下防止某一鼠标一直返回ERROR状态而陷入死循环，导致其他鼠标没有响应的情况
        SetMem(&mousestate,sizeof(EFI_SIMPLE_POINTER_STATE),0);

		if(gpSimpleEx)
			status = gpSimpleEx->ReadKeyStrokeEx(gpSimpleEx, &gKeyData);
		else
		{
			if (NULL != gST && NULL != gST->ConIn)
			{
				status = gST->ConIn->ReadKeyStroke(gST->ConIn, &gKeyData.Key);
			}
		}

        if (!EFI_ERROR(status))
		{
            if (gKeyData.Key.UnicodeChar == 0)
			{
				if (gKeyData.Key.ScanCode < 0x80)
				{
					pdmmessage->message = gKeyData.Key.ScanCode + 0x0080;
				}
            } 
			else
			{
				pdmmessage->message = gKeyData.Key.UnicodeChar;
			}
            break;
        }
		if (!mouse || !g_mouse_enable)
		{
			continue;
		}
            
        status = mouse->GetState(mouse,&mousestate);
        
        if (!EFI_ERROR(status)) {
            INT32 mouseresX,mouseresY;
			INT32 mouseresZ;
            
            mouseresX=(INT32)mouse->Mode->ResolutionX;
            mouseresY=(INT32)mouse->Mode->ResolutionY;
			mouseresZ= (INT32)mouse->Mode->ResolutionZ;
			if(!mouseresX || !mouseresY || !mouseresZ)
				continue;
            
			//MyLog(LOG_DEBUG, L"mouseres(%d,%d,%d), RelativeMovement(%d,%d,%d), Button(%d,%d)\n",
			//	mouseresX, mouseresY, mouseresZ,
			//	mousestate.RelativeMovementX,
			//	mousestate.RelativeMovementY,
			//	mousestate.RelativeMovementZ,
			//	mousestate.LeftButton,
			//	mousestate.RightButton);
				
            if(mousestate.RelativeMovementX || mousestate.RelativeMovementY || mousestate.RelativeMovementZ)
			{
            	pdmmessage->thex = g_mouse_x + mousestate.RelativeMovementX * 8 / mouseresX;
            	pdmmessage->they = g_mouse_y + mousestate.RelativeMovementY * 8 / mouseresY;
				pdmmessage->thez = g_mouse_z + mousestate.RelativeMovementZ * 8 / mouseresZ;

            	if (pdmmessage->thex < 0) pdmmessage->thex = 0;
            	if (pdmmessage->they < 0) pdmmessage->they = 0;
            		
				if(pdmmessage->thex > g_WindowWidth - 16)
					pdmmessage->thex = g_WindowWidth - 16;
				if(pdmmessage->they > g_WindowHeight - 24)
					pdmmessage->they = g_WindowHeight - 24;
            }
			else {
				pdmmessage->thex = g_mouse_x;
				pdmmessage->they = g_mouse_y;
				pdmmessage->thez = g_mouse_z;
			}

			//MyLog(LOG_DEBUG, L"thez: %d g_mouse_z: %d\n", pdmmessage->thez, g_mouse_z);
            if (pdmmessage->thex != g_mouse_x || pdmmessage->they != g_mouse_y) {
            	
				BYTE bMouseDisplay = hidemouse();

                pdmmessage->message = WM_MOUSEMOVE;
                if (g_mouse_oldlbutton && mousestate.LeftButton)
                    pdmmessage->message = WM_MOUSEDRAG;

	            g_mouse_x = pdmmessage->thex;
    	        g_mouse_y = pdmmessage->they;
    	        
				if(bMouseDisplay)
        			showmouse();


        	    
            } 
			else if (pdmmessage->thez != g_mouse_z)
			{
				if (pdmmessage->thez > g_mouse_z)
				{
					pdmmessage->message = WM_MOUSE_MID_SCROLL_DOWN;
					g_mouse_z = pdmmessage->thez;
				}
				else
				{
					pdmmessage->message = WM_MOUSE_MID_SCROLL_UP;
					g_mouse_z = pdmmessage->thez;
				}
			}
			//else {
   //             if (!g_mouse_oldlbutton && mousestate.LeftButton)
   //                 pdmmessage->message = WM_LEFTBUTTONPRESSED;
   //             if (g_mouse_oldlbutton && !mousestate.LeftButton)
   //                 pdmmessage->message = WM_LEFTBUTTONRELEASE;

   //             if (!g_mouse_oldrbutton && mousestate.RightButton)
   //                 pdmmessage->message = WM_RIGHTBUTTONPRESSED;
   //             if (g_mouse_oldrbutton && !mousestate.RightButton)
   //                 pdmmessage->message = WM_RIGHTBUTTONRELEASE;

	  //          g_mouse_oldlbutton = mousestate.LeftButton;
   // 	        g_mouse_oldrbutton = mousestate.RightButton;
   //         }

			if (!g_mouse_oldlbutton && mousestate.LeftButton)
			{
				pdmmessage->message = WM_LEFTBUTTONPRESSED;
				g_mouse_oldlbutton = mousestate.LeftButton;
			}	
			if (g_mouse_oldlbutton && !mousestate.LeftButton)
			{
				pdmmessage->message = WM_LEFTBUTTONRELEASE;
				g_mouse_oldlbutton = mousestate.LeftButton;
			}
				

			if (!g_mouse_oldrbutton && mousestate.RightButton)
			{
				pdmmessage->message = WM_RIGHTBUTTONPRESSED;
				g_mouse_oldrbutton = mousestate.RightButton;
			}		
			if (g_mouse_oldrbutton && !mousestate.RightButton)
			{
				pdmmessage->message = WM_RIGHTBUTTONRELEASE;
				g_mouse_oldrbutton = mousestate.RightButton;
			}	

            break;
        }
	}

    return ;
}

// 切换光标，输入新的图形，返回老图形,并显示出来
PCURSOR_INFO change_cursor(PCURSOR_INFO cur)
{
	if (g_data.fMinscale == 1)
	{
		g_current_cursor = &Cursor_normal_4k;
	}
	else
	{
		g_current_cursor = &Cursor_normal;
	}

    PCURSOR_INFO old_cur = g_current_cursor;

	BYTE bMouseDisplay = hidemouse();
    
    g_current_cursor = cur;

	if(bMouseDisplay)
		showmouse();

    return old_cur;
}

void Control_sendmsg(PCONTROL pctrl,struct DM_MESSAGE *dmmessage)
{
    if ((pctrl->status&CON_STATUS_DISABLE)==0)
        if (pctrl->sendmessage!=NULL)
            pctrl->sendmessage(pctrl,dmmessage);

}
int Control_enable(PCONTROL pctl)
{
    struct DM_MESSAGE dmmessage;
    pctl->status&=~CON_STATUS_DISABLE;
    dmmessage.message=WM_CTL_ENABLE;
    Control_sendmsg(pctl,&dmmessage);
    return 0;
}
int Control_disable(PCONTROL pctl)
{
    struct DM_MESSAGE dmmessage;
    dmmessage.message=WM_CTL_DISABLE;
    Control_sendmsg(pctl,&dmmessage);
    pctl->status|=CON_STATUS_DISABLE;
    return 0;
}
//int Control_ShowUI(PCONTROL pctl)
//{
//	struct DM_MESSAGE dmmessage;
//	pctl->status &= ~CON_STATUS_HIDE;
//	dmmessage.message = WM_CTL_ENABLE;
//	Control_sendmsg(pctl, &dmmessage);
//	return 0;
//}
//int Control_HideUI(PCONTROL pctl)
//{
//	struct DM_MESSAGE dmmessage;
//	dmmessage.message = WM_CTL_DISABLE;
//	Control_sendmsg(pctl, &dmmessage);
//	pctl->status |= CON_STATUS_HIDE;
//	return 0;
//}

//只显示控件
void Window_showctls(PWINDOW pwin)
{
    WORD i;
    struct DM_MESSAGE msg;

    for (i=0 ;i<pwin->control_num ;i++ )
    {
        if (pwin->control_list[i]->status & CON_STATUS_HIDE)
            continue;
        (pwin->control_list[i])->show((PCONTROL)pwin->control_list[i]);
        if (pwin->control_list[i]->status & CON_STATUS_DISABLE ) {//显示disable的控件
            msg.message = WM_CTL_DISABLE;
            pwin->control_list[i]->sendmessage(pwin->control_list[i],&msg);
        }
    }
////////////////////////////////////////////////////////////////
    msg.message=WM_CTL_ACTIVATE;
	if (pwin->current < pwin->control_num)
	{
		Control_sendmsg(pwin->control_list[pwin->current], &msg);
	}
	else
	{
		MyLog(LOG_DEBUG, L"Window_showctls pwin->current 0x%x >= pwin->control_num: 0x%x\n", pwin->current, pwin->control_num);
	}

//  (pwin->control_list[pwin->current])->sendmessage(pwin->control_list[pwin->current],&msg); //激活当前控件
    return ;
}

 
//初始化window结构，可指定show函数指针，如果=NULL,使用默认显示函数
//用户如果需要自定义消息处理，可更改msgfunc函数指针
int Window_init(PWINDOW pwin,void (* show)(struct WINDOW *) )
{
//	pwin->control_list=con;
    pwin->control_num=0;
    pwin->current=pwin->precurrent=0;
    pwin->current_cursor=0xffff; //当前鼠标在背景上
    pwin->ifexit=0;
    pwin->selected = 0;
	pwin->windowID = 0;
	pwin->ret_data = -1;
    if (show==NULL)
        pwin->show=Window_showctls;
    else
        pwin->show=show;
    pwin->msgfunc=Window_Dispatchmsg;
    return 0;
}

void Window_show(PWINDOW pwin)
{
    //DisplayImg(0,0,IMG_FILE_DIALOG_BK,FALSE);
    Window_showctls(pwin);
    return ;
}

//控件调用此函数，让整个界面退出，记录返回值ret
int Window_exit(PWINDOW pwin,DWORD ret)
{
    pwin->ifexit=1;
    pwin->ret_data=ret;
    return 0;

}

int Window_reset(PWINDOW pwin)
{
	//MyLog(LOG_DEBUG, L"==Window_reset pwin: 0x%x, pwin->current: 0x%x, pwin->precurrent: 0x%x\n", pwin, pwin->current, pwin->precurrent);
	pwin->current = pwin->precurrent = 0;
	pwin->ifexit = 0;
	pwin->ret_data = -1;

	for (int i = 0; i < pwin->control_num; i++)
	{
		pwin->control_list[i]->state = CONTROL_STATE_NORMAL;
	}

	return 0;

}

//添加一个控件，返回控件索引号
int Window_addctl(PWINDOW pwin,PCONTROL ctl)
{
    WORD num=pwin->control_num;
    if (num>=MAX_CONTRL_INWIN)
        return -1;
    pwin->control_list[num]=ctl;
    pwin->control_num++;
    ctl->pwin=pwin;
    ctl->control_index=num;
    return num;
}

//设置控件焦点
int Window_setfocus(PWINDOW pwin, PCONTROL ctl)
{
	struct DM_MESSAGE msg;

	for (int i = 0; i < pwin->control_num; i++)
	{
		if (pwin->control_list[i] == ctl && 
			pwin->current != i)
		{
			msg.message = WM_CTL_INACTIVATE;
			if (pwin->current < pwin->control_num)
			{
				Control_sendmsg(pwin->control_list[pwin->current], &msg);
			}

			pwin->precurrent = pwin->current;

			msg.message = WM_CTL_ACTIVATE;
			Control_sendmsg(pwin->control_list[i], &msg);
			pwin->current = i;
			break;
		}
	}

	return 0;
}

PCONTROL FindControlInWindowByType(PWINDOW pwin, BYTE type)
{
	PCONTROL ctl = NULL;

	if (pwin != NULL)
	{
		for (int i = 0; i < pwin->control_num; i++)
		{
			if (pwin->control_list[i]->type == type)
			{
				ctl = pwin->control_list[i];
				break;
			}
		}
	}

	return ctl;
}

PCONTROL FindCountDownBtnInWindow(PWINDOW pwin)
{
	PCONTROL ctl = NULL;

	if (pwin != NULL)
	{
		for (int i = 0; i < pwin->control_num; i++)
		{
			if (pwin->control_list[i]->type == IDC_BUTTON)
			{
				struct BUTTON *pbnctrl = (struct BUTTON *)pwin->control_list[i];
				if (TRUE == pbnctrl->bIsCountDownBtn)
				{
					ctl = pwin->control_list[i];
					break;
				}
			}
		}
	}

	return ctl;
}

PCONTROL FindControlInWindowById(PWINDOW pwin, WORD control_id)
{
	PCONTROL ctl = NULL;

	if (pwin != NULL)
	{
		for (int i = 0; i < pwin->control_num; i++)
		{
			if (pwin->control_list[i]->control_id == control_id)
			{
				ctl = pwin->control_list[i];
				break;
			}
		}
	}

	return ctl;
}

#define offsetof(TYPE, MEMBER) ((UINTN) &((TYPE *)0)->MEMBER)


//处理用户按上下左右键后，控件焦点的转移
//key: 关键变量； dir:方向,向右，向下为1；其余为-1
//key1: 辅助变量，以key1值相近优先选择，当key1值相同时，key值优先
//返回焦点控件
WORD _window_control_move(PWINDOW pwin,WORD key,WORD key1,INT16 dir)
{
    WORD min_key=0xffff;
    WORD min_key1=0xffff;
    WORD i,find_ctl;
    INT16 key_dist,key1_dist;
    WORD current=pwin->current;
    PCONTROL *ctl=pwin->control_list;
    find_ctl=current;

    for (i=0;i< pwin->control_num;i++)
    {
        if ((ctl[i]->status&CON_STATUS_TABNOTSTOP)||(ctl[i]->status&CON_STATUS_DISABLE))
            continue;
        key_dist=*(INT16 *)((PBYTE)ctl[i]+key)-*(INT16 *)((PBYTE)ctl[current]+key);
        key_dist*=dir;

        if (key_dist<=0) //不符合方向
            continue;
        key1_dist=*(INT16 *)((PBYTE )ctl[i]+key1)-*(INT16 *)((PBYTE )ctl[current]+key1);
        if (key1_dist<0)
            key1_dist*=-1; //得到Key1方向上的相对距离

        if ((WORD)key1_dist>min_key1)
            continue;
        if (((WORD)key1_dist==min_key1)&&((WORD)key_dist>=min_key))
            continue;

        find_ctl=i;
        min_key1=key1_dist;
        min_key=key_dist;

    }

    return find_ctl;

}
// 窗口所有控件的消息处理，包括控件切换，定位鼠标位置，发送消息给子控件等等
// 调用前初始化pwin,并显示一次
void  Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{
	//MyLog(LOG_DEBUG, L"Window_Dispatchmsg...\n");

    WORD precurrent,current;
    WORD controlnum=pwin->control_num;
    PCONTROL *ctl=pwin->control_list;
    struct DM_MESSAGE msg;
    WORD current_cursor;

	if ((pwin->windowID == 100 || pwin->windowID == 110 || pwin->windowID == 120) && g_lastCountDownTime != 0)
	{
		time_t  curTime = mytime(NULL);
		if (curTime - g_lastCountDownTime >= 1 && g_dCountDown >= -1)
		{
			g_lastCountDownTime = curTime;
			struct DM_MESSAGE mymsg;
			mymsg.thex = dmmessage->thex;
			mymsg.they = dmmessage->they;
			mymsg.message = WM_TIME_COUNT_DOWN;

			PCONTROL pctrl = FindCountDownBtnInWindow(pwin);
			if (pctrl)
			{
				Control_sendmsg(pctrl, &mymsg);
				if (mymsg.message == WM_BUTTON_CLICK)
				{
					g_lastCountDownTime = 0;
					g_dCountDown = AUTO_MESSAGE_COUNT_DOWN_TIME;
					Window_exit(pctrl->pwin, pctrl->control_id);
					mymsg.message = 0;
					goto exit_dispatch;
				}
			}
		}
	}

    precurrent=pwin->precurrent;
    current=pwin->current;
//	debug(0,500,current);
    //判断鼠标位置,当鼠标位置发生改变时，发送鼠标消息
    if (ifmousemsg(dmmessage)) {
        current_cursor=detectwhere(ctl,controlnum,dmmessage);
		//if (current_cursor != 0xFFFF && current_cursor != pwin->current_cursor) {
		if (current_cursor != pwin->current_cursor) {
            struct DM_MESSAGE mymsg;
            mymsg.thex=dmmessage->thex;
            mymsg.they=dmmessage->they;
            if (pwin->current_cursor<pwin->control_num) { //不是背景
                mymsg.message=WM_MOUSEMOVEOUT;
                //->sendmessage(ctl[pwin->current_cursor],&mymsg);
                Control_sendmsg(ctl[pwin->current_cursor],&mymsg);
            }
            if (current_cursor<pwin->control_num) { //不是背景
                mymsg.message=WM_MOUSEMOVEIN;
                //ctl[current_cursor] ->sendmessage(ctl[current_cursor],&mymsg);
                Control_sendmsg(ctl[current_cursor],&mymsg);

            }
            pwin->current_cursor=current_cursor;
        }
    }
    //		getch();
    switch (dmmessage->message) {
    case CHAR_TAB:

        //MyLog(LOG_DEBUG, L"win-01, pre %d,cur:%d,mes:%x,id %x\n",precurrent,current,dmmessage->message,ctl[current]->control_id);

        while (1) {

            current++;         //切换到下一个控件

            if (current>=controlnum)
                current=0;
            //MyLog(LOG_DEBUG, L"win-02, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[current]->status,ctl[current]->control_id);
            if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
            {
            	 continue;
            }
            
            if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP ) && !( ctl[current]->status&CON_STATUS_DISABLE))//此控件支持激活状态
                break;
        }
        dmmessage->message=0; //消息已经响应，控件不需要处理tab消息
        break;
    case CHAR_ESC:	//取消
	{
		//MyLog(LOG_DEBUG, L"===Window_Dispatchmsg: ESC key.winID: %d", pwin->windowID);

		Window_exit(pwin, IDCANCEL);

		//BOOL bIsSkip = FALSE;
		//for (int i = 0; i < pwin->control_num; i++)
		//{
		//	///Skip ESC on Reboot/Shutdown Window
		//	if (pwin->control_list[i]->control_id == IDB_REBOOT ||
		//		pwin->control_list[i]->control_id == IDB_SHUTDOWN)
		//	{
		//		MyLog(LOG_DEBUG, L"Is Reboot/Shutdown Window.====");
		//		bIsSkip = TRUE;
		//		break;
		//	}

		//	///Skip ESC on Main Window Window
		//	if (pwin->control_list[i]->control_id == IDB_EXIT)
		//	{
		//		MyLog(LOG_DEBUG, L"Is Main Window.====");
		//		bIsSkip = TRUE;
		//		break;
		//	}
		//}

		//if (!bIsSkip)
		//{
		//	Window_exit(pwin, IDCANCEL);
		//}
		//else
		//{
		//	MyLog(LOG_DEBUG, L"Skip ESC button====");
		//}	
    	break;
	}
    case WM_LEFTBUTTONPRESSED: //切换控见

        if ((current_cursor<controlnum)&&(ctl[current_cursor]->status&CON_STATUS_DISABLE) )
            break;
        current=pwin->current_cursor;
        break;
    }

switch_control:

   //  if( current < controlnum &&  precurrent < controlnum)
   //     MyLog(LOG_DEBUG, L"win-03, pre %d,cur:%d,status %x,id:%x mes %x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id,dmmessage->message);

	  if( current < controlnum && (dmmessage->message == CHAR_DOWN || dmmessage->message == CHAR_UP  ||dmmessage->message == CHAR_LEFT || dmmessage->message == CHAR_RIGHT) )
    {
    	  if(current== precurrent && (/*ctl[current]->control_id == IDB_BACKUP|| */ctl[current]->control_id == IDB_SELECT_PART/*||ctl[current]->control_id == IDB_RECOVER_BTN|| ctl[current]->control_id == IDB_DELETE*/)) 
    	  {
    	  	 dmmessage->message = 0;
    	  	 goto exit_dispatch;
    	  }
    	  if(current == precurrent && ctl[current]->control_id == IDB_BACKUP && dmmessage->message == CHAR_UP )
    	  {
     	  	 dmmessage->message = 0;
    	  	 goto exit_dispatch;
   	  	
    	  }
    }	 
	  if( current < controlnum && dmmessage->message < WM_LEFTBUTTONPRESSED && dmmessage->message > WM_RIGHTBUTTONRELEASE)
    {
	      if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP )
        {
             while (1) {
                current++;         //切换到下一个控件
               if (current>=controlnum)
                   current=0;
	             if(ctl[current]->control_id  != IDB_PAGE_DOWN && ctl[current]->control_id  != IDB_PAGE_UP )
                   break;
             }  	
       }
    }
    if (precurrent!=current) {   //切换控件焦点
        //有些控件没有焦点状态(CON_STATUS_NOACTIVE),则不需要调用

        if  ((precurrent<controlnum)  ) {
            msg.message=WM_CTL_INACTIVATE;
            //ctl[precurrent]->sendmessage(ctl[precurrent],&msg);
            Control_sendmsg(ctl[precurrent],&msg);
        }
        if ((current<controlnum)  ) {
            msg.message=WM_CTL_ACTIVATE;
            //ctl[current]->sendmessage(ctl[current],&msg);
            Control_sendmsg(ctl[current],&msg);
        }
        precurrent=current;
    }

	//if (
	//	ctl[pwin->precurrent]->type == IDC_LISTBOX)
	//{
	//	Control_sendmsg(ctl[pwin->current], dmmessage);
	//}

    // 对鼠标移动区域所属的控件发送鼠标消息,即鼠标消息只有当鼠标在控件上时才会发
    if ((pwin->current_cursor!=current)&&(ifmousemsg(dmmessage)) ) {

        if ( (pwin->current_cursor!=0xffff) ) {
            //	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
			Control_sendmsg(ctl[pwin->current_cursor], dmmessage);

        }
        dmmessage->message=0;
    }

    if (current>controlnum) //当前没有焦点
        goto exit_dispatch;
    if (dmmessage->message==0)
        goto exit_dispatch;

    // 对没有禁用的控见发消息
    Control_sendmsg(ctl[current],dmmessage);

    //处理上下左右键消息,切换控见
    //	debug(500,0,dmmessage->message);
    //min=0xffff;
    //find_ctl=current;
    pwin->current=current;

    switch (dmmessage->message) {
    case CHAR_UP:
    {
        current=_window_control_move(pwin,offsetof(CONTROL,they),offsetof(CONTROL,thex),-1);

        dmmessage->message=0;
        goto switch_control;
    }
    case CHAR_DOWN:
    {
        current=_window_control_move(pwin,offsetof(CONTROL,they),offsetof(CONTROL,thex),1);

        dmmessage->message=0;
        goto switch_control;
    }
    break;
    case CHAR_LEFT:
    {
    	  if( current<controlnum && current == precurrent && ctl[current]->control_id == IDB_EXIT )
    	  {   	  
    	  	current = current - 1;
    	  	
    	  }	
    	  else    	  	
           current=_window_control_move(pwin,offsetof(CONTROL,thex),offsetof(CONTROL,they),-1);

     //   MyLog(LOG_DEBUG, L"win-06,LEFT  pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);

        dmmessage->message=0;
        goto switch_control;
    }

    break;
    case CHAR_RIGHT:
    {
        current=_window_control_move(pwin,offsetof(CONTROL,thex),offsetof(CONTROL,they),1);
        dmmessage->message=0;
        goto switch_control;
    }
    break;
    }
exit_dispatch:

    pwin->current=current;
    pwin->precurrent=precurrent;
    return ;

}

// 窗口所有控件的消息处理，包括控件切换，定位鼠标位置，发送消息给子控件等等
// 调用前初始化pwin,并显示一次
void  About_Window_Dispatchmsg(PWINDOW pwin, struct DM_MESSAGE *dmmessage)
{
	WORD precurrent, current;
	WORD controlnum = pwin->control_num;
	PCONTROL *ctl = pwin->control_list;
	struct DM_MESSAGE msg;
	WORD current_cursor;

	precurrent = pwin->precurrent;
	current = pwin->current;
	//判断鼠标位置,当鼠标位置发生改变时，发送鼠标消息
	if (ifmousemsg(dmmessage)) {
		current_cursor = detectwhere(ctl, controlnum, dmmessage);
		//if (current_cursor != 0xFFFF && current_cursor != pwin->current_cursor) {
		if (current_cursor != pwin->current_cursor) {
			struct DM_MESSAGE mymsg;
			mymsg.thex = dmmessage->thex;
			mymsg.they = dmmessage->they;
			if (pwin->current_cursor < pwin->control_num) { //不是背景
				mymsg.message = WM_MOUSEMOVEOUT;
				//->sendmessage(ctl[pwin->current_cursor],&mymsg);
				Control_sendmsg(ctl[pwin->current_cursor], &mymsg);
			}
			if (current_cursor < pwin->control_num) { //不是背景
				mymsg.message = WM_MOUSEMOVEIN;
				//ctl[current_cursor] ->sendmessage(ctl[current_cursor],&mymsg);
				Control_sendmsg(ctl[current_cursor], &mymsg);

			}
			pwin->current_cursor = current_cursor;
		}
	}
	//		getch();
	switch (dmmessage->message) {
	case CHAR_TAB:
		while (1) {

			current++;         //切换到下一个控件

			if (current >= controlnum)
				current = 0;
			//MyLog(LOG_DEBUG, L"win-02, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[current]->status,ctl[current]->control_id);
			if (ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
			{
				continue;
			}

			if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP) && !(ctl[current]->status&CON_STATUS_DISABLE))//此控件支持激活状态
				break;
		}
		dmmessage->message = 0; //消息已经响应，控件不需要处理tab消息
		break;
	case CHAR_ESC:	//取消
		//MyLog(LOG_DEBUG, L"========AboutWindowDispatchmsg: Pressed ESC key.===========pwin: 0x%x", pwin);
		Window_exit(pwin, IDCANCEL);
		break;
	case WM_LEFTBUTTONPRESSED: //切换控见
		if ((current_cursor < controlnum) && (ctl[current_cursor]->status&CON_STATUS_DISABLE))
			break;
		current = pwin->current_cursor;

		//if (current >= 0 && current <= 2)
		//{
		//	pwin->selected = current;
		//}

		break;
	}
    


switch_control:
	if (current < controlnum && (dmmessage->message == CHAR_DOWN || dmmessage->message == CHAR_UP || dmmessage->message == CHAR_LEFT || dmmessage->message == CHAR_RIGHT))
	{
		if (current == precurrent && ctl[current]->control_id == IDB_BACKUP && dmmessage->message == CHAR_UP)
		{
			dmmessage->message = 0;
			goto exit_dispatch;

		}
	}
	if (current < controlnum && dmmessage->message < WM_LEFTBUTTONPRESSED && dmmessage->message > WM_RIGHTBUTTONRELEASE)
	{
		if (ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
		{
			while (1) {
				current++;         //切换到下一个控件
				if (current >= controlnum)
					current = 0;
				if (ctl[current]->control_id != IDB_PAGE_DOWN && ctl[current]->control_id != IDB_PAGE_UP)
					break;
			}
		}
	}
	if (precurrent != current) {   //切换控件焦点
		//有些控件没有焦点状态(CON_STATUS_NOACTIVE),则不需要调用
		if ((current < controlnum)) {
			msg.message = WM_CTL_ACTIVATE;
			//ctl[current]->sendmessage(ctl[current],&msg);
			Control_sendmsg(ctl[current], &msg);
		}
		if ((precurrent < controlnum)) {
			msg.message = WM_CTL_INACTIVATE;
			//ctl[precurrent]->sendmessage(ctl[precurrent],&msg);
			Control_sendmsg(ctl[precurrent], &msg);
		}
		precurrent = current;
	}

	// 对鼠标移动区域所属的控件发送鼠标消息,即鼠标消息只有当鼠标在控件上时才会发
	if ((pwin->current_cursor != current) && (ifmousemsg(dmmessage))) {

		if ((pwin->current_cursor != 0xffff)) {
			//	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
			Control_sendmsg(ctl[pwin->current_cursor], dmmessage);

		}
		dmmessage->message = 0;
	}

	if (current > controlnum) //当前没有焦点
		goto exit_dispatch;
	if (dmmessage->message == 0)
		goto exit_dispatch;

	// 对没有禁用的控见发消息
	Control_sendmsg(ctl[current], dmmessage);

	//处理上下左右键消息,切换控见
	//	debug(500,0,dmmessage->message);
	//min=0xffff;
	//find_ctl=current;
	pwin->current = current;

	switch (dmmessage->message) {
	case CHAR_UP:
	{
		current = _window_control_move(pwin, offsetof(CONTROL, they), offsetof(CONTROL, thex), -1);

		dmmessage->message = 0;
		goto switch_control;
	}
	case CHAR_DOWN:
	{
		current = _window_control_move(pwin, offsetof(CONTROL, they), offsetof(CONTROL, thex), 1);

		dmmessage->message = 0;
		goto switch_control;
	}
	break;
	case CHAR_LEFT:
	{
		if (current < controlnum && current == precurrent && ctl[current]->control_id == IDB_EXIT)
		{
			current = current - 1;

		}
		else
			current = _window_control_move(pwin, offsetof(CONTROL, thex), offsetof(CONTROL, they), -1);

		//   MyLog(LOG_DEBUG, L"win-06,LEFT  pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);

		dmmessage->message = 0;
		goto switch_control;
	}

	break;
	case CHAR_RIGHT:
	{
		current = _window_control_move(pwin, offsetof(CONTROL, thex), offsetof(CONTROL, they), 1);
		dmmessage->message = 0;
		goto switch_control;
	}
	break;
	}
exit_dispatch:

	pwin->current = current;
	pwin->precurrent = precurrent;
	return;

}

void  Recover_Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{

    WORD precurrent,current;
    WORD controlnum=pwin->control_num;
    PCONTROL *ctl=pwin->control_list;
    struct DM_MESSAGE msg;
    WORD current_cursor;

    precurrent=pwin->precurrent;
    current=pwin->current;
//	debug(0,500,current);
    //判断鼠标位置,当鼠标位置发生改变时，发送鼠标消息

   // if( current < controlnum &&  precurrent < controlnum)
   //  MyLog(LOG_DEBUG, L"win-01, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);


    if (ifmousemsg(dmmessage)) {
        current_cursor=detectwhere(ctl,controlnum,dmmessage);
		//if (current_cursor != 0xFFFF && current_cursor != pwin->current_cursor) {
		if (current_cursor != pwin->current_cursor) {
            struct DM_MESSAGE mymsg;
            mymsg.thex=dmmessage->thex;
            mymsg.they=dmmessage->they;
            if (pwin->current_cursor<pwin->control_num) { //不是背景
                mymsg.message=WM_MOUSEMOVEOUT;
                //->sendmessage(ctl[pwin->current_cursor],&mymsg);
                Control_sendmsg(ctl[pwin->current_cursor],&mymsg);
            }
            if (current_cursor<pwin->control_num) { //不是背景
                mymsg.message=WM_MOUSEMOVEIN;
                //ctl[current_cursor] ->sendmessage(ctl[current_cursor],&mymsg);
                Control_sendmsg(ctl[current_cursor],&mymsg);

            }
            pwin->current_cursor=current_cursor;
        }
    }
    //		getch();
    switch (dmmessage->message) {
    case CHAR_TAB:

    //    MyLog(LOG_DEBUG, L"win-01, pre %d,cur:%d,mes:%x,id %x\n",precurrent,current,dmmessage->message,ctl[current]->control_id);

        while (1) {

            current++;         //切换到下一个控件

            if (current>=controlnum)
                current=0;
       //     MyLog(LOG_DEBUG, L"win-02, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[current]->status,ctl[current]->control_id);
            if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
            {
            	 continue;
            }
            
            if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP ) && !( ctl[current]->status&CON_STATUS_DISABLE))//此控件支持激活状态
                break;
        }
        dmmessage->message=0; //消息已经响应，控件不需要处理tab消息
        break;
    case CHAR_ESC:	//取消
		//MyLog(LOG_DEBUG, L"====RecoveryWindowDispatchmsg: Pressed ESC key.===========pwin: 0x%x", pwin);
    	Window_exit(pwin, IDCANCEL);
    	break;
    case WM_LEFTBUTTONPRESSED: //切换控见

        if ((current_cursor<controlnum)&&(ctl[current_cursor]->status&CON_STATUS_DISABLE) )
            break;
        current=pwin->current_cursor;
        break;
    }
    
   //if( current < controlnum &&  precurrent < controlnum)
   //  MyLog(LOG_DEBUG, L"win-02, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);


switch_control:

  //   if( current < controlnum &&  precurrent < controlnum)
   //     MyLog(LOG_DEBUG, L"win-03, pre %d,cur:%d,status %x,id:%x mes %x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id,dmmessage->message);

	  if( current < controlnum && (dmmessage->message == CHAR_DOWN || dmmessage->message == CHAR_UP  ||dmmessage->message == CHAR_LEFT || dmmessage->message == CHAR_RIGHT) )
    {
    	//   MyLog(LOG_DEBUG, L"win-03-01, pre %d,cur:%d,status %x,id:%x mes %x\n",precurrent,current,ctl[current]->status,ctl[current]->control_id,dmmessage->message );
    	  if( (ctl[current]->control_id == IDB_RECOVER && dmmessage->message == CHAR_RIGHT)||(ctl[current]->control_id == IDB_DELETE && dmmessage->message == CHAR_LEFT) ) 
    	  {
    	  	
    	//  	 MyLog(LOG_DEBUG, L"win-07, pre %d,cur:%d,status %x,id:%x mes %x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id,dmmessage->message);
    	  	 dmmessage->message = 0;
    	  	 goto exit_dispatch;
    	  }
    }	 
	  if( current < controlnum && dmmessage->message < WM_LEFTBUTTONPRESSED && dmmessage->message > WM_RIGHTBUTTONRELEASE)
    {
	      if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP )
        {
             while (1) {
                current++;         //切换到下一个控件
               if (current>=controlnum)
                   current=0;
	             if(ctl[current]->control_id  != IDB_PAGE_DOWN && ctl[current]->control_id  != IDB_PAGE_UP )
                   break;
             }  	
       }
    }
    if (precurrent!=current) {   //切换控件焦点
        //有些控件没有焦点状态(CON_STATUS_NOACTIVE),则不需要调用

        if  ((precurrent<controlnum)  ) {
            msg.message=WM_CTL_INACTIVATE;
            //ctl[precurrent]->sendmessage(ctl[precurrent],&msg);
           
       //     MyLog(LOG_DEBUG, L"win-04, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);

            Control_sendmsg(ctl[precurrent],&msg);
        }
        if ((current<controlnum)  ) {
            msg.message=WM_CTL_ACTIVATE;
            //ctl[current]->sendmessage(ctl[current],&msg);
          //    MyLog(LOG_DEBUG, L"win-05, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[current]->status,ctl[current]->control_id);

            Control_sendmsg(ctl[current],&msg);
        }
        precurrent=current;
    }

	//if ((ctl[pwin->current]->type == IDC_LISTBOX ||
	//	ctl[pwin->current]->type == IDC_SCROLLBOX) &&
	//	(dmmessage->message == WM_MOUSE_MID_SCROLL_UP ||
	//		dmmessage->message == WM_MOUSE_MID_SCROLL_DOWN))
	//{
	//	Control_sendmsg(ctl[pwin->current], dmmessage);
	//}

    // 对鼠标移动区域所属的控件发送鼠标消息,即鼠标消息只有当鼠标在控件上时才会发
    if ((pwin->current_cursor!=current)&&(ifmousemsg(dmmessage)) ) {

        if ( (pwin->current_cursor!=0xffff) ) {
            //	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
			Control_sendmsg(ctl[pwin->current_cursor], dmmessage);

        }
        dmmessage->message=0;
    }

    if (current>controlnum) //当前没有焦点
        goto exit_dispatch;
    if (dmmessage->message==0)
        goto exit_dispatch;

    // 对没有禁用的控见发消息
    Control_sendmsg(ctl[current],dmmessage);

    //处理上下左右键消息,切换控见
    //	debug(500,0,dmmessage->message);
    //min=0xffff;
    //find_ctl=current;
    pwin->current=current;

    switch (dmmessage->message) {
    case CHAR_UP:
    {
      	int tmp = 0;
      	
        tmp=_window_control_move(pwin,offsetof(CONTROL,they),offsetof(CONTROL,thex),-1);
    	  
    	 // MyLog(LOG_DEBUG, L"win-10,up  pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);
    	  if( current<controlnum && current == precurrent && ( ctl[current]->control_id == IDB_RECOVER || ctl[current]->control_id == IDB_DELETE ))
    	  {   	  
    	  	// if( ctl[tmp]->control_id != IDB_DELETE  && ctl[current]->control_id == IDB_RECOVER )
    	  	 {
    	  	 	 tmp = current;
    	  	 }
    	  }	
    	  current = tmp;
    	   
        dmmessage->message=0;
        goto switch_control;
    }
    case CHAR_DOWN:
    {
        current=_window_control_move(pwin,offsetof(CONTROL,they),offsetof(CONTROL,thex),1);

        dmmessage->message=0;
        goto switch_control;
    }
    break;
    case CHAR_LEFT:
    {
    	  int tmp = 0;
    	//  MyLog(LOG_DEBUG, L"win-07,LEFT  pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);

    	    	  	
        tmp=_window_control_move(pwin,offsetof(CONTROL,thex),offsetof(CONTROL,they),-1);

    	  if( current<controlnum && current == precurrent && ctl[current]->control_id == IDB_RECOVER )
    	  {   	  
    	  	 if( ctl[tmp]->control_id != IDB_DELETE )
    	  	 {
    	  	 	 tmp = current;
    	  	 }
    	  }	
    	  
    	  current = tmp;
    	  
      //  MyLog(LOG_DEBUG, L"win-08,LEFT  pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);

        dmmessage->message=0;
        goto switch_control;
    }

    break;
    case CHAR_RIGHT:
    {
   	 // 	 MyLog(LOG_DEBUG, L"win-09, pre %d,cur:%d,status %x,id:%x mes %x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id,dmmessage->message);
    	  if(current<controlnum && (ctl[current]->control_id == IDB_RECOVER)) 
    	  {
      	   precurrent = current;
    	  	 dmmessage->message=0;
           goto exit_dispatch;
    	  } 	
        current=_window_control_move(pwin,offsetof(CONTROL,thex),offsetof(CONTROL,they),1);
        dmmessage->message=0;
        goto switch_control;
    }
    break;
    }
exit_dispatch:

    pwin->current=current;
    pwin->precurrent=precurrent;
    return ;

}

// 窗口所有控件的消息处理，包括控件切换，定位鼠标位置，发送消息给子控件等等
// 调用前初始化pwin,并显示一次
//void  SelectDisk_Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
//{
//	//MyLog(LOG_DEBUG, L"SelectDisk_Window_Dispatchmsg\n");
//
//    WORD precurrent,current;
//    WORD controlnum=pwin->control_num;
//    PCONTROL *ctl=pwin->control_list;
//    struct DM_MESSAGE msg;
//    WORD current_cursor;
//    WORD prev_control_type;
//    static int prev_tab_current =-1,tab_current = -1;
//
//    precurrent=pwin->precurrent;
//    current=pwin->current;
//
//	//MyLog(LOG_DEBUG, L"Drvbox-00, precurrent %d,current:%d,%x\n",precurrent,current,dmmessage->message);
//
////	debug(0,500,current);
//    //判断鼠标位置,当鼠标位置发生改变时，发送鼠标消息
//    if (ifmousemsg(dmmessage)) {
//        current_cursor=detectwhere(ctl,controlnum,dmmessage);
//        if (current_cursor!=pwin->current_cursor) {
//            struct DM_MESSAGE mymsg;
//            mymsg.thex=dmmessage->thex;
//            mymsg.they=dmmessage->they;
//            if (pwin->current_cursor<pwin->control_num) { //不是背景
//                mymsg.message=WM_MOUSEMOVEOUT;
//                //->sendmessage(ctl[pwin->current_cursor],&mymsg);
//                Control_sendmsg(ctl[pwin->current_cursor],&mymsg);
//            }
//            if (current_cursor<pwin->control_num) { //不是背景
//                mymsg.message=WM_MOUSEMOVEIN;
//                //ctl[current_cursor] ->sendmessage(ctl[current_cursor],&mymsg);
//                Control_sendmsg(ctl[current_cursor],&mymsg);
//
//            }
//            pwin->current_cursor=current_cursor;
//        }
//    }
//  
//  //MyLog(LOG_DEBUG, L"Drvbox-01, precurrent %d,current:%d,%x\n",precurrent,current,dmmessage->message);
//        
//	switch (dmmessage->message)
//	{
//	case CHAR_TAB:
//		//MyLog(LOG_DEBUG, L"Drvbox-0, precurrent %d,current:%d\n",precurrent,current);
//		while (1)
//		{
//			current++;         //切换到下一个控件            
//			if (current >= controlnum)
//				current = 0;
//			if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP) && !(ctl[current]->status&CON_STATUS_DISABLE))//此控件支持激活状态
//				break;
//		}
//		//dmmessage->message=0; //消息已经响应，控件不需要处理tab消息
////        MyLog(LOG_DEBUG, L"Drvbox-2 prev_tab_current:%d,current:%d\n",prev_tab_current,current);
//		break;
//	case CHAR_RETURN:
//		Window_exit(pwin, IDB_BACKUP);
//		break;
//	case CHAR_ESC:	//取消
//		Window_exit(pwin, IDCANCEL);
//		break;
//	case WM_LEFTBUTTONPRESSED: //切换控见
//
// //       MyLog(LOG_DEBUG, L"Drvbox-02, current_cursor %x\n",current_cursor);
//
//		if ((current_cursor < controlnum) && (ctl[current_cursor]->status & CON_STATUS_DISABLE))
//			break;
//		current = pwin->current_cursor;
//		break;
//	}
//    
//	//MyLog(LOG_DEBUG, L"Drvbox-02, precurrent %d,current:%d\n",precurrent,current);
//	if (current < controlnum  && precurrent < controlnum)
//	{
//		if (ctl[current]->control_id == IDB_BACKUP && dmmessage->message == WM_LEFTBUTTONPRESSED)
//		{
//			//MyLog(LOG_DEBUG, L"Drvbox-5, precurrent %d,current:%d,%d\n",precurrent,current,pwin->current_cursor);
//
//			msg.message = CHAR_RETURN;
//			Control_sendmsg(ctl[current], &msg);
//			dmmessage->message = CHAR_RETURN;
//			goto exit_dispatch;
//		}
//	}
//	if (current < controlnum  && precurrent < controlnum)
//	{
//		if (ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
//		{
//			if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
//			{
//				Control_sendmsg(ctl[current], dmmessage);
//
//				if (ctl[current]->control_id == IDB_PAGE_UP)
//				{
//					if (precurrent == 0)
//					{
//						current = 0;
//					}
//					else
//					{
//						current = precurrent - 1;
//					}
//					//	 MyLog(LOG_DEBUG, L"Drvbox-03, up precurrent %d,current:%d\n",precurrent,current);        
//
//				   //	 precurrent = current;
//					dmmessage->message = WM_LEFTBUTTONPRESSED;
//
//					Control_sendmsg(ctl[current], dmmessage);
//					dmmessage->message = 0;
//					precurrent = current;
//					pwin->selected = current;
//
//					goto exit_dispatch;
//				}
//				else  if (ctl[current]->control_id == IDB_PAGE_DOWN)
//				{
//					int show_button_num = 0, i = 0;
//
//					for (i = 0; i < controlnum; i++)
//					{
//						if (ctl[i]->control_id == IDB_BUTTON_SHOW)
//						{
//							show_button_num++;
//						}
//					}
//
//					current = precurrent + 1;
//
//					//	 MyLog(LOG_DEBUG, L"Drvbox-03-0, down precurrent %d,current:%d,%d\n",precurrent,current,show_button_num);        
//
//					if (current >= show_button_num)
//					{
//						current = current - 1;
//					}
//
//					precurrent = current;
//					//		 MyLog(LOG_DEBUG, L"Drvbox-03-1, down precurrent %d,current:%d\n",precurrent,current);        
//
//					dmmessage->message = WM_LEFTBUTTONPRESSED;
//					Control_sendmsg(ctl[current], dmmessage);
//					dmmessage->message = 0;
//
//					pwin->selected = current;
//
//					goto exit_dispatch;
//				}
//
//			}
//
//		}
//	}
//	else if (current < controlnum && precurrent == 0xffff)
//	{
//		if (ctl[current]->control_id == IDB_PAGE_UP)
//		{
//			current = pwin->selected;
//
//			if (current == 0)
//			{
//			}
//			else
//			{
//				current = current - 1;
//			}
//			//	 MyLog(LOG_DEBUG, L"Drvbox-03, up precurrent %d,current:%d\n",precurrent,current);        
//
//		   //	 precurrent = current;
//			dmmessage->message = WM_LEFTBUTTONPRESSED;
//			Control_sendmsg(ctl[current], dmmessage);
//			dmmessage->message = 0;
//			pwin->selected = precurrent = current;
//
//			goto exit_dispatch;
//		}
//		else  if (ctl[current]->control_id == IDB_PAGE_DOWN)
//		{
//			int show_button_num = 0, i = 0;
//
//			for (i = 0; i < controlnum; i++)
//			{
//				if (ctl[i]->control_id == IDB_BUTTON_SHOW)
//				{
//					show_button_num++;
//				}
//			}
//			current = pwin->selected;
//
//			current = current + 1;
//
//			//	 MyLog(LOG_DEBUG, L"Drvbox-03-0, down precurrent %d,current:%d,%d\n",precurrent,current,show_button_num);        
//
//			if (current >= show_button_num)
//			{
//				current = current - 1;
//			}
//
//			pwin->selected = precurrent = current;
//			//	 MyLog(LOG_DEBUG, L"Drvbox-03-1, down precurrent %d,current:%d\n",precurrent,current);        
//
//			dmmessage->message = WM_LEFTBUTTONPRESSED;
//			Control_sendmsg(ctl[current], dmmessage);
//			dmmessage->message = 0;
//
//			goto exit_dispatch;
//		}
//	}
//	else if (precurrent < controlnum && current == 0xffff)
//	{
//
//	}
//
//switch_control:
//	// MyLog(LOG_DEBUG, L"Drvbox-1, precurrent %d,current:%d,mes %x\n",precurrent,current,dmmessage->message);
//	if (dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN)
//	{
//		if (current < controlnum  && precurrent == current)
//		{
//			if (ctl[current]->control_id == IDB_EDIT_BTN)
//			{
//				dmmessage->message = 0;
//
//				goto exit_dispatch;
//			}
//		}
//	}
//	if (dmmessage->message == CHAR_LEFT || dmmessage->message == CHAR_RIGHT)
//	{
//		if (current < controlnum  && precurrent < controlnum  && ctl[current]->control_id != IDB_EDIT_BTN)
//		{
//			dmmessage->message = 0;
//
//			pwin->selected = current;
//
//			goto exit_dispatch;
//		}
//	}
//	if (dmmessage->message == CHAR_TAB && current < controlnum  && precurrent < controlnum)
//	{
//		if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW || ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
//		{
//			while (1) {
//				//切换到下一个控件
//				if (current >= controlnum)
//					current = 0;
//				if (ctl[current]->control_id != IDB_BUTTON_SHOW)
//					break;
//				current++;
//			}
//
//		}
//	}
//	if (dmmessage->message == CHAR_RIGHT && current < controlnum  && precurrent < controlnum)
//	{
//		if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW || ctl[current]->control_id == IDB_BUTTON_SHOW)
//		{
//			while (1) {
//				//切换到下一个控件
//				current++;
//				if (current >= controlnum)
//					current = 0;
//				if (ctl[current]->control_id != IDB_BUTTON_SHOW && ctl[current]->control_id != IDB_PAGE_DOWN && ctl[current]->control_id != IDB_PAGE_UP)
//					break;
//			}
//
//		}
//	}
//
//	if (current < controlnum  && precurrent < controlnum)
//	{
//		//MyLog(LOG_DEBUG, L"Drvbox-2, pre id %x,cur id:%x\n",ctl[precurrent]->control_id,ctl[current]->control_id);
//
//		if (ctl[precurrent]->control_id == IDB_BACKUP && ctl[current]->control_id == IDB_BUTTON_SHOW && dmmessage->message != WM_LEFTBUTTONPRESSED)
//		{
//			current = precurrent;
//			dmmessage->message = 0;
//		}
//	}
//
//	//MyLog(LOG_DEBUG, L"Drvbox-2, precurrent %d,current:%d\n",precurrent,current);
//
//	if (current < controlnum  && precurrent < controlnum)
//	{
//		if (ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
//		{
//			while (1)
//			{
//
//				if (current >= controlnum)
//					current = 0;
//				if (ctl[current]->control_id != IDB_PAGE_DOWN && ctl[current]->control_id != IDB_PAGE_UP)
//					break;
//				current++;         //切换到下一个控件
//			}
//		}
//	}
//	//   if(current < controlnum  && precurrent < controlnum  )
//	 //     	MyLog(LOG_DEBUG, L"Drvbox-3, pre %d,cur:%d,%d,mes:%x,%x\n",precurrent,current,pwin->current_cursor,dmmessage->message,ctl[current]->control_id);
// /*
//	 if(dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN)
//	 {
//
//		 if (precurrent<controlnum  )
//		 {
//			   dmmessage->message=0;
//			 msg.message = WM_CTL_INACTIVATE;
//
//			 Control_sendmsg(ctl[precurrent],&msg);
//		 }
//		   dmmessage->message=0;
//		 msg.message = WM_CTL_ACTIVATE;
//
//		 if (current<controlnum  )
//		 {
//				Control_sendmsg(ctl[current],&msg);
//
//		 }
//		 goto exit_dispatch;
//
//	 }
// */
//	if (precurrent != current) {   //切换控件焦点
//		//有些控件没有焦点状态(CON_STATUS_NOACTIVE),则不需要调用
//
//		if ((precurrent < controlnum)) {
//			msg.message = WM_CTL_INACTIVATE;
//			//ctl[precurrent]->sendmessage(ctl[precurrent],&msg);
//			if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
//			{
//				if (dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN)
//				{
//					dmmessage->message = 0;
//					msg.message = WM_CTL_INACTIVATE;
//					Control_sendmsg(ctl[precurrent], &msg);
//				}
//				else  if (current < controlnum)
//				{
//					if (ctl[current]->control_id == IDB_BUTTON_SHOW)
//					{
//
//					}
//					else
//					{
//						if (current < controlnum && ctl[current]->control_id == IDCANCEL)
//						{
//						}
//						else
//						{
//							dmmessage->message = 0;
//						}
//						msg.message = WM_BTN_SELCHANGE;
//						Control_sendmsg(ctl[precurrent], &msg);
//					}
//				}
//				else
//				{
//					if (current < controlnum && ctl[current]->control_id == IDCANCEL)
//					{
//					}
//					else
//					{
//						dmmessage->message = 0;
//					}
//					msg.message = WM_BTN_SELCHANGE;
//					Control_sendmsg(ctl[precurrent], &msg);
//				}
//
//
//			}
//			else
//				Control_sendmsg(ctl[precurrent], &msg);
//
//
//		}
//		if ((current < controlnum)) {
//			msg.message = WM_CTL_ACTIVATE;
//			//ctl[current]->sendmessage(ctl[current],&msg);	     	       
//			Control_sendmsg(ctl[current], &msg);
//
//			if (precurrent < controlnum)
//			{
//				if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
//				{
//					pwin->selected = current;
//				}
//			}
//			if (ctl[current]->control_id == IDB_BUTTON_SHOW)
//			{
//				pwin->selected = current;
//
//				if (1)
//				{
//					dmmessage->message = 0;
//					msg.message = WM_CTL_INACTIVATE;
//					Control_sendmsg(ctl[current], &msg);
//					msg.message = WM_CTL_ACTIVATE;
//					Control_sendmsg(ctl[current], &msg);
//				}
//			}
//
//			//  if( precurrent < controlnum  )
//			//     Control_enable(ctl[precurrent]);
//			 /*
//			  if(ctl[current]->control_id == IDB_BUTTON_SHOW)
//			  {
//				  dmmessage->message=0;
//				  //Control_disable(ctl[precurrent]);
//				  msg.message = ' ';
//				  Control_sendmsg(ctl[precurrent],&msg);
//
//			  }
//			  else
//				  Control_sendmsg(ctl[current],&msg);
//			*/
//		}
//		precurrent = current;
//	}
//	// 对鼠标移动区域所属的控件发送鼠标消息,即鼠标消息只有当鼠标在控件上时才会发
//	if ((pwin->current_cursor != current) && (ifmousemsg(dmmessage))) {
//
//		if ((pwin->current_cursor != 0xffff)) {
//			//	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
//		  //  	MyLog(LOG_DEBUG, L"Drvbox-8, precurrent %d,current:%d,%d\n",precurrent,current,pwin->current_cursor);
//
//			Control_sendmsg(ctl[pwin->current_cursor], dmmessage);
//
//
//
//			//if (ctl[pwin->current_cursor]->control_id == IDB_BUTTON_SHOW)
//			//{
//			//	pwin->selected = current;
//
//			//}
//		}
//		dmmessage->message = 0;
//	}
//
//	if (current > controlnum) //当前没有焦点
//		goto exit_dispatch;
//	if (dmmessage->message == 0)
//		goto exit_dispatch;
//
//	//   if(current < controlnum  && precurrent < controlnum  )
//	  //    	MyLog(LOG_DEBUG, L"Drvbox-9, pre %d,cur:%d,%d,mes:%x,%x\n",precurrent,current,pwin->current_cursor,dmmessage->message,ctl[current]->control_id);
//	 // 对没有禁用的控见发消息
//	Control_sendmsg(ctl[current], dmmessage);
//
//	//处理上下左右键消息,切换控见
//	//	debug(500,0,dmmessage->message);
//	//min=0xffff;
//	//find_ctl=current;
//	pwin->current = current;
//
//	switch (dmmessage->message) {
//	case CHAR_UP:
//	{
//		current = _window_control_move(pwin, offsetof(CONTROL, they), offsetof(CONTROL, thex), -1);
//
//		dmmessage->message = 0;
//		goto switch_control;
//	}
//	case CHAR_DOWN:
//	{
//		current = _window_control_move(pwin, offsetof(CONTROL, they), offsetof(CONTROL, thex), 1);
//
//		dmmessage->message = 0;
//		goto switch_control;
//	}
//	break;
//	case CHAR_LEFT:
//	{
//		current = _window_control_move(pwin, offsetof(CONTROL, thex), offsetof(CONTROL, they), -1);
//		dmmessage->message = 0;
//		goto switch_control;
//	}
//
//	break;
//	case CHAR_RIGHT:
//	{
//		current = _window_control_move(pwin, offsetof(CONTROL, thex), offsetof(CONTROL, they), 1);
//		dmmessage->message = 0;
//		goto switch_control;
//	}
//	break;
//	}
//
//exit_dispatch:
//	pwin->current = current;
//	pwin->precurrent = precurrent;
//	return;
//
//}

// 窗口所有控件的消息处理，包括控件切换，定位鼠标位置，发送消息给子控件等等
// 调用前初始化pwin,并显示一次
void  SelectBackupPart_Window_Dispatchmsg(PWINDOW pwin, struct DM_MESSAGE *dmmessage)
{
	WORD precurrent, current;
	WORD controlnum = pwin->control_num;
	PCONTROL *ctl = pwin->control_list;
	struct DM_MESSAGE msg;
	WORD current_cursor;
	WORD prev_control_type;
	static int prev_tab_current = -1, tab_current = -1;

	precurrent = pwin->precurrent;
	current = pwin->current;

//	debug(0,500,current);
	//判断鼠标位置,当鼠标位置发生改变时，发送鼠标消息
	if (ifmousemsg(dmmessage)) {

		//MyLog(LOG_DEBUG, L"precurrent %d,current:%d,msg: %x\n", precurrent, current, dmmessage->message);

		current_cursor = detectwhere(ctl, controlnum, dmmessage);
		//if (current_cursor != 0xFFFF && current_cursor != pwin->current_cursor) {
		if (current_cursor != pwin->current_cursor) {
			struct DM_MESSAGE mymsg;
			mymsg.thex = dmmessage->thex;
			mymsg.they = dmmessage->they;
			if (pwin->current_cursor < pwin->control_num) { //不是背景
				mymsg.message = WM_MOUSEMOVEOUT;
				//->sendmessage(ctl[pwin->current_cursor],&mymsg);
				Control_sendmsg(ctl[pwin->current_cursor], &mymsg);
			}
			if (current_cursor < pwin->control_num) { //不是背景
				mymsg.message = WM_MOUSEMOVEIN;
				//ctl[current_cursor] ->sendmessage(ctl[current_cursor],&mymsg);
				Control_sendmsg(ctl[current_cursor], &mymsg);

			}
			pwin->current_cursor = current_cursor;
		}
	}

	//MyLog(LOG_DEBUG, L"precurrent %d,current:%d,%x\n",precurrent,current,dmmessage->message);

	switch (dmmessage->message)
	{
	case CHAR_TAB:
		//MyLog(LOG_DEBUG, L"Drvbox-0, precurrent %d,current:%d\n",precurrent,current);
		while (1)
		{
			current++;         //切换到下一个控件            
			if (current >= controlnum)
				current = 0;
			if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP) && !(ctl[current]->status&CON_STATUS_DISABLE))//此控件支持激活状态
				break;
		}
		//dmmessage->message=0; //消息已经响应，控件不需要处理tab消息
//        MyLog(LOG_DEBUG, L"Drvbox-2 prev_tab_current:%d,current:%d\n",prev_tab_current,current);
		break;
	case CHAR_RETURN:
		//MyLog(LOG_DEBUG, L"====SelectBackupPartWindowDispatchmsg: Pressed RETURN key.===========pwin: 0x%x", pwin);
		Window_exit(pwin, IDB_BACKUP);
		break;
	case CHAR_ESC:	//取消
		//MyLog(LOG_DEBUG, L"====SelectBackupPartWindowDispatchmsg: Pressed ESC key.===========pwin: 0x%x", pwin);
		Window_exit(pwin, IDCANCEL);
		break;
	case WM_LEFTBUTTONPRESSED: //切换控见

 //       MyLog(LOG_DEBUG, L"Drvbox-02, current_cursor %x\n",current_cursor);

		if ((current_cursor < controlnum) && (ctl[current_cursor]->status & CON_STATUS_DISABLE))
			break;
		current = pwin->current_cursor;
		break;
	}
    
	//MyLog(LOG_DEBUG, L"Drvbox-02, precurrent %d,current:%d\n",precurrent,current);
	if (current < controlnum  && precurrent < controlnum)
	{
		if (ctl[current]->control_id == IDB_BACKUP && dmmessage->message == WM_LEFTBUTTONPRESSED)
		{
			//MyLog(LOG_DEBUG, L"Drvbox-5, precurrent %d,current:%d,%d\n",precurrent,current,pwin->current_cursor);

			msg.message = CHAR_RETURN;
			Control_sendmsg(ctl[current], &msg);
			dmmessage->message = CHAR_RETURN;
			goto exit_dispatch;
		}
	}
	if (current < controlnum  && precurrent < controlnum)
	{
		if (ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
		{
			if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
			{
				Control_sendmsg(ctl[current], dmmessage);

				if (ctl[current]->control_id == IDB_PAGE_UP)
				{
					if (precurrent == 0)
					{
						current = 0;
					}
					else
					{
						current = precurrent - 1;
					}
					//	 MyLog(LOG_DEBUG, L"Drvbox-03, up precurrent %d,current:%d\n",precurrent,current);        

				   //	 precurrent = current;
					dmmessage->message = WM_LEFTBUTTONPRESSED;

					Control_sendmsg(ctl[current], dmmessage);
					dmmessage->message = 0;
					precurrent = current;
					pwin->selected = current;

					goto exit_dispatch;
				}
				else  if (ctl[current]->control_id == IDB_PAGE_DOWN)
				{
					int show_button_num = 0, i = 0;

					for (i = 0; i < controlnum; i++)
					{
						if (ctl[i]->control_id == IDB_BUTTON_SHOW)
						{
							show_button_num++;
						}
					}

					current = precurrent + 1;

					//	 MyLog(LOG_DEBUG, L"Drvbox-03-0, down precurrent %d,current:%d,%d\n",precurrent,current,show_button_num);        

					if (current >= show_button_num)
					{
						current = current - 1;
					}

					precurrent = current;
					//		 MyLog(LOG_DEBUG, L"Drvbox-03-1, down precurrent %d,current:%d\n",precurrent,current);        

					dmmessage->message = WM_LEFTBUTTONPRESSED;
					Control_sendmsg(ctl[current], dmmessage);
					dmmessage->message = 0;

					pwin->selected = current;

					goto exit_dispatch;
				}

			}

		}
	}
	else if (current < controlnum && precurrent == 0xffff)
	{
		if (ctl[current]->control_id == IDB_PAGE_UP)
		{
			current = pwin->selected;

			if (current == 0)
			{
			}
			else
			{
				current = current - 1;
			}
			//	 MyLog(LOG_DEBUG, L"Drvbox-03, up precurrent %d,current:%d\n",precurrent,current);        

		   //	 precurrent = current;
			dmmessage->message = WM_LEFTBUTTONPRESSED;
			Control_sendmsg(ctl[current], dmmessage);
			dmmessage->message = 0;
			pwin->selected = precurrent = current;

			goto exit_dispatch;
		}
		else  if (ctl[current]->control_id == IDB_PAGE_DOWN)
		{
			int show_button_num = 0, i = 0;

			for (i = 0; i < controlnum; i++)
			{
				if (ctl[i]->control_id == IDB_BUTTON_SHOW)
				{
					show_button_num++;
				}
			}
			current = pwin->selected;

			current = current + 1;

			//	 MyLog(LOG_DEBUG, L"Drvbox-03-0, down precurrent %d,current:%d,%d\n",precurrent,current,show_button_num);        

			if (current >= show_button_num)
			{
				current = current - 1;
			}

			pwin->selected = precurrent = current;
			//	 MyLog(LOG_DEBUG, L"Drvbox-03-1, down precurrent %d,current:%d\n",precurrent,current);        

			dmmessage->message = WM_LEFTBUTTONPRESSED;
			Control_sendmsg(ctl[current], dmmessage);
			dmmessage->message = 0;

			goto exit_dispatch;
		}
	}
	else if (precurrent < controlnum && current == 0xffff)
	{

	}

switch_control:
	// MyLog(LOG_DEBUG, L"Drvbox-1, precurrent %d,current:%d,mes %x\n",precurrent,current,dmmessage->message);
	if (dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN)
	{
		if (current < controlnum  && precurrent == current)
		{
			//MyLog(LOG_ERROR, L"111111111111111222222222\n");
			if (ctl[current]->control_id == IDB_EDIT_BTN)
			{
				dmmessage->message = 0;

				goto exit_dispatch;
			}
		}
	}

	if (dmmessage->message == CHAR_LEFT || dmmessage->message == CHAR_RIGHT)
	{
		if (current < controlnum  && precurrent < controlnum  && ctl[current]->control_id != IDB_EDIT_BTN)
		{
			//MyLog(LOG_ERROR, L"asdfasfd\n");
			dmmessage->message = 0;

			pwin->selected = current;

			goto exit_dispatch;
		}
	}

	if (dmmessage->message == CHAR_TAB && current < controlnum  && precurrent < controlnum)
	{
		if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW || ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
		{
			//MyLog(LOG_ERROR, L"eeeeeeeeee\n");
			while (1) {
				//切换到下一个控件
				if (current >= controlnum)
					current = 0;
				if (ctl[current]->control_id != IDB_BUTTON_SHOW)
					break;
				current++;
			}

		}
	}

	if (dmmessage->message == CHAR_RIGHT && current < controlnum  && precurrent < controlnum)
	{
		if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW || ctl[current]->control_id == IDB_BUTTON_SHOW)
		{
			//MyLog(LOG_ERROR, L"yyyyyyyy\n");
			while (1) {
				//切换到下一个控件
				current++;
				if (current >= controlnum)
					current = 0;
				if (ctl[current]->control_id != IDB_BUTTON_SHOW && ctl[current]->control_id != IDB_PAGE_DOWN && ctl[current]->control_id != IDB_PAGE_UP)
					break;
			}

		}
	}

	if (current < controlnum  && precurrent < controlnum)
	{
		//MyLog(LOG_DEBUG, L"Drvbox-2, pre id %x,cur id:%x\n",ctl[precurrent]->control_id,ctl[current]->control_id);

		if (ctl[precurrent]->control_id == IDB_BACKUP && ctl[current]->control_id == IDB_BUTTON_SHOW && dmmessage->message != WM_LEFTBUTTONPRESSED)
		{
			//MyLog(LOG_ERROR, L"tttttttttt\n");
			current = precurrent;
			dmmessage->message = 0;
		}
	}

	if (current < controlnum  && precurrent < controlnum)
	{
		if (ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
		{
			//MyLog(LOG_ERROR, L"rrrrrrrrrrrr\n");
			while (1)
			{

				if (current >= controlnum)
					current = 0;
				if (ctl[current]->control_id != IDB_PAGE_DOWN && ctl[current]->control_id != IDB_PAGE_UP)
					break;
				current++;         //切换到下一个控件
			}
		}
	}

	if (precurrent != current) {   //切换控件焦点
		//有些控件没有焦点状态(CON_STATUS_NOACTIVE),则不需要调用
		if ((precurrent < controlnum))
		{
			msg.message = WM_CTL_INACTIVATE;
			//ctl[precurrent]->sendmessage(ctl[precurrent],&msg);
			if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
			{
				if (dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN)
				{
					dmmessage->message = 0;
					msg.message = WM_CTL_INACTIVATE;
					Control_sendmsg(ctl[precurrent], &msg);
				}
				else  if (current < controlnum)
				{
					if (ctl[current]->control_id == IDB_BUTTON_SHOW)
					{

					}
					else
					{
						if (current < controlnum && ctl[current]->control_id == IDCANCEL)
						{
						}
						else
						{
							dmmessage->message = 0;
						}
						msg.message = WM_BTN_SELCHANGE;
						Control_sendmsg(ctl[precurrent], &msg);
					}
				}
				else
				{
					if (current < controlnum && ctl[current]->control_id == IDCANCEL)
					{
					}
					else
					{
						dmmessage->message = 0;
					}
					msg.message = WM_BTN_SELCHANGE;
					Control_sendmsg(ctl[precurrent], &msg);
				}


			}
			else
				Control_sendmsg(ctl[precurrent], &msg);


		}
		if ((current < controlnum)) 
		{
			msg.message = WM_CTL_ACTIVATE;
			//ctl[current]->sendmessage(ctl[current],&msg);	     	       
			Control_sendmsg(ctl[current], &msg);

			if (precurrent < controlnum)
			{
				if (ctl[precurrent]->control_id == IDB_BUTTON_SHOW)
				{
					pwin->selected = current;
				}
			}
			if (ctl[current]->control_id == IDB_BUTTON_SHOW)
			{
				pwin->selected = current;

				if (1)
				{
					dmmessage->message = 0;
					msg.message = WM_CTL_INACTIVATE;
					Control_sendmsg(ctl[current], &msg);
					msg.message = WM_CTL_ACTIVATE;
					Control_sendmsg(ctl[current], &msg);
				}
			}
		}
		precurrent = current;
	}

	//if ((ctl[pwin->current]->type == IDC_LISTBOX ||
	//	ctl[pwin->current]->type == IDC_SCROLLBOX) &&
	//	(dmmessage->message == WM_MOUSE_MID_SCROLL_UP ||
	//	dmmessage->message == WM_MOUSE_MID_SCROLL_DOWN))
	//{
	//	Control_sendmsg(ctl[pwin->current], dmmessage);
	//}
	
	// 对鼠标移动区域所属的控件发送鼠标消息,即鼠标消息只有当鼠标在控件上时才会发
	if ((pwin->current_cursor != current) && (ifmousemsg(dmmessage))) {

		if ((pwin->current_cursor != 0xffff)) {
			//	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
		  //  	MyLog(LOG_DEBUG, L"Drvbox-8, precurrent %d,current:%d,%d\n",precurrent,current,pwin->current_cursor);

			Control_sendmsg(ctl[pwin->current_cursor], dmmessage);

			if (ctl[pwin->current_cursor]->control_id == IDB_BUTTON_SHOW)
			{
				pwin->selected = current;

			}
		}
		dmmessage->message = 0;
	}

	if (current > controlnum) //当前没有焦点
		goto exit_dispatch;
	if (dmmessage->message == 0)
		goto exit_dispatch;

	//   if(current < controlnum  && precurrent < controlnum  )
	  //    	MyLog(LOG_DEBUG, L"Drvbox-9, pre %d,cur:%d,%d,mes:%x,%x\n",precurrent,current,pwin->current_cursor,dmmessage->message,ctl[current]->control_id);
	 // 对没有禁用的控见发消息
	Control_sendmsg(ctl[current], dmmessage);

	//处理上下左右键消息,切换控见
	//	debug(500,0,dmmessage->message);
	//min=0xffff;
	//find_ctl=current;
	pwin->current = current;

	switch (dmmessage->message) {
	case CHAR_UP:
	{
		current = _window_control_move(pwin, offsetof(CONTROL, they), offsetof(CONTROL, thex), -1);

		dmmessage->message = 0;
		goto switch_control;
	}
	case CHAR_DOWN:
	{
		current = _window_control_move(pwin, offsetof(CONTROL, they), offsetof(CONTROL, thex), 1);

		dmmessage->message = 0;
		goto switch_control;
	}
	break;
	case CHAR_LEFT:
	{
		current = _window_control_move(pwin, offsetof(CONTROL, thex), offsetof(CONTROL, they), -1);
		dmmessage->message = 0;
		goto switch_control;
	}

	break;
	case CHAR_RIGHT:
	{
		current = _window_control_move(pwin, offsetof(CONTROL, thex), offsetof(CONTROL, they), 1);
		dmmessage->message = 0;
		goto switch_control;
	}
	break;
	}

exit_dispatch:
	pwin->current = current;
	pwin->precurrent = precurrent;
	//MyLog(LOG_DEBUG, L"exit_dispatch, pre: %d,cur: %d\n", precurrent, current);
	return;

}

DWORD Window_run(PWINDOW pwin)
{
	//MyLog(LOG_DEBUG, L"enter Window_run====pwin: 0x%x", pwin);

	UINT32 Index;
	EFI_HANDLE                            *HandleArray = NULL;
	UINTN                                 HandleArrayCount = 0;
	EFI_SIMPLE_POINTER_PROTOCOL *TempPoint = NULL;
	EFI_STATUS Status = EFI_SUCCESS;
	Index = 0;


	struct DM_MESSAGE dmmessage;

	if (g_data.fMinscale == 1)
	{
		change_cursor(&Cursor_normal_4k);
	}
	else
	{
		change_cursor(&Cursor_normal);
	}

	pwin->show(pwin);

	g_mouse_oldlbutton = FALSE;	//有些鼠标会有问题
	g_mouse_oldrbutton = FALSE;

	while (1)
	{
		dmmessage.message = WM_NOMESSAGE;

		//枚举所有鼠标handle，代码改动最小
		Status = gBS->LocateHandleBuffer(
			ByProtocol,
			&gEfiSimplePointerProtocolGuid,
			NULL,
			&HandleArrayCount,
			&HandleArray
		);
		if (!EFI_ERROR(Status))
		{
			for (Index = 0; Index < HandleArrayCount; Index++)
			{
				Status = gBS->HandleProtocol(
					HandleArray[Index],
					&gEfiSimplePointerProtocolGuid,
					(VOID **)&TempPoint
				);
				if (!EFI_ERROR(Status))
				{

					detectmessage(&dmmessage, TempPoint);//直接传入鼠标handle，代码改动最小
					//MyLog(LOG_DEBUG, L"message %x, l %d, r %d, x %x, y %x\n",
					//	dmmessage.message, g_mouse_oldlbutton, g_mouse_oldrbutton, g_mouse_x, g_mouse_y);
					if (dmmessage.message == CHAR_ESC ||
						dmmessage.message == CHAR_RETURN ||
						dmmessage.message == ' ')
					{
						MyLog(LOG_DEBUG, L"====detectmessage ReadKey: ESC, ENTER or Space==");
						if ((pwin->windowID == 100 || pwin->windowID == 110) && g_lastESCTime != 0)
						{
							time_t  curTime = mytime(NULL);
							//MyLog(LOG_DEBUG, L"====curTime== %lld", curTime);
							//MyLog(LOG_DEBUG, L"====g_lastESCTime== %lld", g_lastESCTime);

							if (curTime - g_lastESCTime <= 1)
							{
								MyLog(LOG_DEBUG, L"====skip esc between 1 second==");
								continue;
							}
							else
							{
								g_lastESCTime = 0;
							}
						}
					}

					//if (dmmessage.message == CHAR_RETURN)
					//{
					//	MyLog(LOG_DEBUG, L"====detectmessage ReadKey: ENTER==");
					//}

					//if (dmmessage.message == ' ')
					//{
					//	MyLog(LOG_DEBUG, L"====detectmessage ReadKey: SPACE ==");
					//}
					pwin->msgfunc(pwin, &dmmessage);

				}
			}
			if (HandleArray)
			{
				FreePool(HandleArray);
				HandleArray = NULL;
			}
		}
		else //没有鼠标的情况，防止键盘不能操作
		{
			detectmessage(&dmmessage, NULL);//直接传入鼠标handle，代码改动最小
				  //MyLog(LOG_DEBUG, L"message %x, l %d, r %d, x %x, y %x\n",
				  //	dmmessage.message, g_mouse_oldlbutton, g_mouse_oldrbutton, g_mouse_x, g_mouse_y);
			if (dmmessage.message == CHAR_ESC ||
				dmmessage.message == CHAR_RETURN ||
				dmmessage.message == ' ')
			{
				MyLog(LOG_DEBUG, L"====detectmessage ReadKey: ESC, ENTER or Space==");
				if ((pwin->windowID == 100 || pwin->windowID == 110) && g_lastESCTime != 0)
				{
					time_t  curTime = mytime(NULL);
					//MyLog(LOG_DEBUG, L"====curTime== %lld", curTime);
					//MyLog(LOG_DEBUG, L"====g_lastESCTime== %lld", g_lastESCTime);

					if (curTime - g_lastESCTime <= 1)
					{
						MyLog(LOG_DEBUG, L"====skip esc between 1 second==");
						continue;
					}
					else
					{
						g_lastESCTime = 0;
					}
				}
			}

			//if (dmmessage.message == CHAR_RETURN)
			//{
			//	MyLog(LOG_DEBUG, L"====detectmessage ReadKey: ENTER==");
			//}

			//if (dmmessage.message == ' ')
			//{
			//	MyLog(LOG_DEBUG, L"====detectmessage ReadKey: SPACE ==");
			//}
			pwin->msgfunc(pwin, &dmmessage);
		}

		if (pwin->ifexit != 0)
			break;

		usleep(200);
	}

	SetMem(&gKeyData, sizeof(EFI_KEY_DATA), 0);
	dmmessage.message = WM_NOMESSAGE;

	MyLog(LOG_DEBUG, L"==Level Window_run====pwin: 0x%x", pwin);
	return pwin->ret_data;
}

//判断鼠标点击处处于哪一个响应,如果有覆盖，thez高者优先，
//在无需响应的区域返回0xffff
WORD detectwhere(struct CONTROL * p[],WORD totalnum,struct DM_MESSAGE *msg)
{
    WORD i,ret=0xffff;
    WORD x=(WORD)msg->thex;
    WORD y=(WORD)msg->they;
    WORD upper=0;
    for (i=0;i<totalnum;i++)
    {
        WORD thez=p[i]->thez;
        if (p[i]->thex>x)	        continue;
        if (p[i]->they>y)	        continue;
        if (p[i]->thex+p[i]->thew<x)	continue;
        if (p[i]->they+p[i]->theh<y)	continue;
        if ((ret==0xffff)||(thez>upper)) {
            upper=thez;
            ret=i;
        }
    }
	//MyLog(LOG_DEBUG, L"detectwhere, ret: 0x%x\n", ret);
    return ret;
}



int ifmousemsg(struct DM_MESSAGE *pdmmessage)
{
    if (NULL != pdmmessage && pdmmessage->message > 0xfeff)
        return YES;
    return NO;
}

int ifkeymsg(struct DM_MESSAGE *pdmmessage)
{
    if (NULL != pdmmessage && pdmmessage->message<  0xff)
        return YES;
    return NO;
}


///////////////////////////////combobox begin///////////////////////////
//item 最长50个ANSII字符
//每个字符宽度14
//编辑框高度20
//下拉按钮的高度16

//显示边框里的当前选择字符串
int Combobox_showstr(PCOMBOBOX pcombo,WORD iffocus)
{
    WORD x = pcombo->controlinfo.thex;
    WORD y = pcombo->controlinfo.they;
    DWORD forcolor,backcolor;
	BYTE bMouseDisplay = hidemouse();

    if (iffocus==1)
    {
        forcolor=COLOR_WHITE;
        backcolor=COMBO_SELECT_COLOR;
    }
    else if (iffocus==2)//disable
    {
        forcolor = SCROLLBOX_LINE_COLOR;
        backcolor = SCROLLBOX_RANGE_COLOR;
    }
    else
    {
        forcolor=COLOR_BLACK;
        backcolor=COLOR_WHITE;
    }
    FillRect(x+2,y+2,pcombo->controlinfo.thew-22,17,backcolor);//显示编辑框
    if (pcombo->whichselect != 0XFFFF)                               //显示当前所选项
    {
        //    	CopyMem(onestr,(POINTER)((u32)pcombo->name+(u32)pcombo->whichselect*size) ,size);
        //  onestr[size-1]=0;
        DisplayString(x+3,y+3,forcolor,(CHAR16*)((PBYTE)pcombo->name+pcombo->whichselect*pcombo->size_of_item));
    }

	if(bMouseDisplay)
		showmouse();

    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//显示状态为未激活且listbox未展开的combobox的函数
//
void Combobox_show(struct CONTROL * pCombobox)
{
    WORD x,y,w,h;
    struct COMBOBOX * pcombo=( struct COMBOBOX *)pCombobox;
	BYTE bMouseDisplay;

    x = pCombobox->thex;
    y = pCombobox->they;
    w = pCombobox->thew;
    h = pCombobox->theh;

    bMouseDisplay = hidemouse();

    Linebox(x,y,w,h,COMBO_COLOR_FRAME); 				   //显示编框色
    //DisplayImg(x+w-18,y+2,IMG_FILE_DROPDOWNA,FALSE); //显示下拉按钮

	if(bMouseDisplay)
		showmouse();
    Combobox_showstr(pcombo,0);
}

void drapdown_msgfunc(PCONTROL pcontrol, struct DM_MESSAGE * pdmessage)
{
    Listbox_DispatchMsg(pcontrol,pdmessage);
    if (pdmessage->message==WM_LEFTBUTTONPRESSED)
        pdmessage->message=WM_LISTBOX_SELECT;
    if (pdmessage->message==WM_LISTBOX_SELECT)
        Window_exit(pcontrol->pwin,0);
}

void drapdown_winmsgfunc(PWINDOW pwin, struct DM_MESSAGE * pdmessage)
{
    Window_Dispatchmsg(pwin,pdmessage);
    if (pwin->current==0xffff) //下拉框失去焦点，退出
        Window_exit(pwin, IDCANCEL);

}
#define COMBO_LISTBOX_WIDTH  18
void Combobox_drapdown(PCOMBOBOX pcombo)
{
    WORD lines,i;
    LISTBOX listbox;
    SCROLL_BOX scroll;
    WINDOW win;
//	POINTER imgbuff;
    VOID * imgbuff;
//	HANDLE handle;
    WORD thex,they,thew,theh;

    Window_init(&win,Window_showctls);
    win.msgfunc=drapdown_winmsgfunc;
    lines=pcombo->num;
    if (lines>5)//需要滚动条
        lines=5;
    thex=pcombo->controlinfo.thex;
    they=pcombo->controlinfo.they+23;
    thew=pcombo->controlinfo.thew;
    theh=lines*20;
    Listbox_init(&listbox,thex,they,thew-COMBO_LISTBOX_WIDTH,theh,lines,50*2*pcombo->num);
    listbox.controlinfo.sendmessage=drapdown_msgfunc;
    for (i=0;i<pcombo->num;i++) {
        Listbox_Addoneline(&listbox);
        Listbox_Addoneunit(&listbox,ITEM_TYPE_CHAR_STRING,(CHAR16 *)((PBYTE)pcombo->name+i*pcombo->size_of_item));
    }
    Window_addctl(&win,(PCONTROL)&listbox);
    if (pcombo->num>lines) {
        Scrollbox_init(&scroll,(PCONTROL)&listbox,pcombo->num,lines);
        listbox.pscroll=&scroll;
        Window_addctl(&win,(PCONTROL)&scroll);
    }
    //分配内存
    imgbuff=AllocatePool(0x200000);
//	handle=Mallocmem(&(POINTER)imgbuff,NULL,"img",CommonMEM,0x10000);
    //保存背景
    hidemouse();

    //DisplayImg(thex+thew-18,pcombo->controlinfo.they+2,IMG_FILE_DROPDOWNB,FALSE); //显示下拉按钮
//	GetImg(thex,they,thew,theh+1,imgbuff);
    GetImgEx(thex,they,thew,theh+1,imgbuff);
    showmouse();
    Window_run(&win);
    pcombo->whichselect=listbox.firstitem_inwindow+listbox.whichselect;

    //恢复背景
    hidemouse();

    PutImgEx(thex,they,thew,theh+1,imgbuff);
    //DisplayImg(thex+thew-18,pcombo->controlinfo.they+2,IMG_FILE_DROPDOWNA,FALSE); //显示下拉按钮

    showmouse();
    //释放内存
    FreePool(imgbuff);
    Combobox_show((PCONTROL)pcombo);
    Combobox_showstr(pcombo,1);
    Listbox_destroy( &listbox);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//combobox的消息分派函数
//
void Combobox_DispatchMsg(struct CONTROL* pCombobox,struct DM_MESSAGE * pdmessage)
{

    struct COMBOBOX * pcombo = (struct COMBOBOX *)pCombobox;
    WORD oldindex=pcombo->whichselect;
    switch (pdmessage->message)
    {
    case WM_LEFTBUTTONPRESSED:
    case CHAR_RETURN :
    case L' ' :
		//MyLog(LOG_DEBUG, L"========Combobox_DispatchMsg: Pressed RETURN or space key.===========");
        Combobox_drapdown(pcombo);
        break;
    case CHAR_DOWN:
        if (pcombo->whichselect<(pcombo->num-1)) {
            pcombo->whichselect++;
            Combobox_showstr(pcombo,1);
        }
        break;

    case CHAR_UP :
        if (pcombo->whichselect!=0) {
            pcombo->whichselect--;
            Combobox_showstr(pcombo,1);
        }
        break;
    case WM_CTL_DISABLE:
        Combobox_showstr(pcombo,2);
        break;
    case WM_CTL_ACTIVATE:
        Combobox_showstr(pcombo,1);
        break;
    case WM_CTL_ENABLE:
    case WM_CTL_INACTIVATE:
        Combobox_showstr(pcombo,0);
        break;
    default:
        break;
    }
    if (pcombo->whichselect!=oldindex)
        pdmessage->message=WM_CONTEXT_CHANGE;

    return ;
}


//currentsel=当前选择哪项，＝－1为空
int Combobox_init(struct COMBOBOX * pcomboctrl,WORD x,WORD y,WORD width,VOID  * itemsname,WORD itemlen,WORD  itemnum,WORD curselect)
{
    pcomboctrl->controlinfo.thex=x;
    pcomboctrl->controlinfo.they=y;
    pcomboctrl->controlinfo.thew=width;
    pcomboctrl->controlinfo.theh=20;
    pcomboctrl->controlinfo.status=0;
    pcomboctrl->controlinfo.type=IDC_COMBOBOX;
    pcomboctrl->controlinfo.thez=0;
    pcomboctrl->controlinfo.sendmessage=Combobox_DispatchMsg;
    pcomboctrl->controlinfo.show=Combobox_show;
    pcomboctrl->name =itemsname;
    pcomboctrl->size_of_item=itemlen;
    pcomboctrl->num=itemnum;
    pcomboctrl->whichselect	=curselect;

    return 0;
}

///////////////////////////////button begin/////////////////////////////


// status=0 unfocus, =1 focus, =2 pressed
void Button_show(struct CONTROL * pCtrl)
{
	struct BUTTON *pbnctrl = (struct BUTTON *)pCtrl;
    if (!pbnctrl->btn_unfocuspcx)//纯文字按钮
        return;
    hidemouse();

	if(pbnctrl->clear_background)
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_CLIENT);

    if (pbnctrl->controlinfo.state == CONTROL_STATE_NORMAL) {
        DisplayImg(pbnctrl->controlinfo.thex,pbnctrl->controlinfo.they, pbnctrl->btn_unfocuspcx ,FALSE);
    } else if (pbnctrl->controlinfo.state == CONTROL_STATE_FOCUS) {// draw linebox
        DisplayImg(pbnctrl->controlinfo.thex,pbnctrl->controlinfo.they, pbnctrl->btn_focuspcx ,FALSE);
        //DotLinebox(pbnctrl->controlinfo.thex+4,pbnctrl->controlinfo.they+4,pbnctrl->controlinfo.thew-8,pbnctrl->controlinfo.theh-8,COLOR_BLACK);
    } else if (pbnctrl->controlinfo.state == CONTROL_STATE_PRESSED) {
        DisplayImg(pbnctrl->controlinfo.thex,pbnctrl->controlinfo.they, pbnctrl->btn_pressedpcx ,FALSE);
	} else if(pbnctrl->controlinfo.state == CONTROL_STATE_DISABLE) {
		if(pbnctrl->btn_disablepcx)
			DisplayImg(pbnctrl->controlinfo.thex,pbnctrl->controlinfo.they, pbnctrl->btn_disablepcx, FALSE);
	}
    
    if(pbnctrl->btn_title) {
		//DisplayImgInRect(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, 
        //	pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, 
        //	pbnctrl->btn_title, TRUE);
		DisplayImg(pbnctrl->controlinfo.thex,pbnctrl->controlinfo.they, 
			pbnctrl->btn_title ,FALSE);
    }
    showmouse();
    return ;
}

void Button_showEx(struct CONTROL * pCtrl)
{
	struct BUTTON *pbnctrl = (struct BUTTON *)pCtrl;
	if (!pbnctrl->btn_unfocuspcx)//纯文字按钮
		return;
	hidemouse();

	if (pbnctrl->clear_background)
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_DEFAULT_BACKGROUND);

	if (pbnctrl->controlinfo.state == CONTROL_STATE_NORMAL) 
	{
		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, 
			pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, pbnctrl->btn_unfocuspcx, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_FOCUS) {// draw linebox
		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, 
			pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, pbnctrl->btn_focuspcx, FALSE);
		//DotLinebox(pbnctrl->controlinfo.thex+4,pbnctrl->controlinfo.they+4,pbnctrl->controlinfo.thew-8,pbnctrl->controlinfo.theh-8,COLOR_BLACK);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_PRESSED) {
		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, 
			pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, pbnctrl->btn_pressedpcx, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_DISABLE) 
	{
		if (pbnctrl->btn_disablepcx)
			DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, 
				pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, pbnctrl->btn_disablepcx, FALSE);
	}

	if (pbnctrl->btn_title) {
		//DisplayImgInRect(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, 
		//	pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, 
		//	pbnctrl->btn_title, TRUE);
		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they, 
			pbnctrl->controlinfo.thew, pbnctrl->controlinfo.theh, pbnctrl->btn_title, FALSE);
	}
	showmouse();
	return;
}

void ButtonSplicing_showEx(struct CONTROL * pCtrl)
{
	//MyLog(LOG_MESSAGE, L"ButtonSplicing_showEx");
	struct BUTTON *pbnctrl = (struct BUTTON *)pCtrl;
	if (!pbnctrl->btn_unfocuspcx)//纯文字按钮
		return;
	hidemouse();
	if (pbnctrl->clear_background)
	{
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_DEFAULT_BACKGROUND);
	}
		
	if (pbnctrl->controlinfo.state == CONTROL_STATE_NORMAL)
	{
		//l:52
		//S:26
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_L); //IMG_FILE_BTNSPL_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_M); //IMG_FILE_BTNSPL_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_R); //IMG_FILE_BTNSPL_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}
		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex+ iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew-(iWidth*2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex+ pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_FOCUS) {// draw linebox
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_F_L); // IMG_FILE_BTNSPL_F_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_F_M); //IMG_FILE_BTNSPL_F_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_F_R); //IMG_FILE_BTNSPL_F_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}

		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew - (iWidth * 2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_PRESSED) {
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_P_L); // IMG_FILE_BTNSPL_P_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_P_M); // IMG_FILE_BTNSPL_P_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_P_R); // IMG_FILE_BTNSPL_P_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}

		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew - (iWidth * 2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_DISABLE)
	{
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_D_L); //IMG_FILE_BTNSPL_D_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_D_M); //IMG_FILE_BTNSPL_D_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_D_R); //IMG_FILE_BTNSPL_D_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}

		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew - (iWidth * 2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}

	BUTTON_INFO* btnInfo;
	btnInfo = FindBtnInfoFromID(pbnctrl->controlinfo.control_id);
	StringInfo stStringInfoCOMPLETE;
	stStringInfoCOMPLETE.enumFontType = FZLTHJW;
	stStringInfoCOMPLETE.iFontSize = CalcFontSize(40); //(int)(40 * g_data.fMinscale);
	DWORD dwTextLen = 0;
	GetStringLen(btnInfo->pText, &stStringInfoCOMPLETE, &dwTextLen);
	if (pbnctrl->controlinfo.state == CONTROL_STATE_DISABLE)
	{
		DisplayStringEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - dwTextLen) / 2, pbnctrl->controlinfo.they + (pbnctrl->controlinfo.theh - stStringInfoCOMPLETE.iFontSize) / 2, COLOR_TEXT_DISABLE, btnInfo->pText, &stStringInfoCOMPLETE, TRUE);
	}
	else
	{
		DisplayStringEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - dwTextLen) / 2, pbnctrl->controlinfo.they + (pbnctrl->controlinfo.theh - stStringInfoCOMPLETE.iFontSize) / 2, COLOR_TEXT_TITLE, btnInfo->pText, &stStringInfoCOMPLETE, TRUE);
	}
	
	showmouse();
	return;
}

void ButtonSplicingCountdown_showEx(struct CONTROL * pCtrl)
{
	//MyLog(LOG_DEBUG, L"ButtonSplicingCountdown_showEx...%d", g_dCountDown);

	struct BUTTON *pbnctrl = (struct BUTTON *)pCtrl;
	if (!pbnctrl->btn_unfocuspcx)//纯文字按钮
		return;
	hidemouse();
	if (pbnctrl->clear_background)
	{
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_DEFAULT_BACKGROUND);
	}

	if (pbnctrl->controlinfo.state == CONTROL_STATE_NORMAL)
	{
		//l:52
		//S:26
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_L); //IMG_FILE_BTNSPL_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_M); //IMG_FILE_BTNSPL_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_R); //IMG_FILE_BTNSPL_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}
		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew - (iWidth * 2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_FOCUS) {// draw linebox
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_F_L); // IMG_FILE_BTNSPL_F_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_F_M); //IMG_FILE_BTNSPL_F_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_F_R); //IMG_FILE_BTNSPL_F_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}

		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew - (iWidth * 2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_PRESSED) {
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_P_L); // IMG_FILE_BTNSPL_P_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_P_M); // IMG_FILE_BTNSPL_P_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_P_R); // IMG_FILE_BTNSPL_P_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}

		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew - (iWidth * 2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_DISABLE)
	{
		WORD iDataL = GetImageFileId(IMG_FILE_BTNSPL_D_L); //IMG_FILE_BTNSPL_D_L;
		WORD iDataM = GetImageFileId(IMG_FILE_BTNSPL_D_M); //IMG_FILE_BTNSPL_D_M;
		WORD iDataR = GetImageFileId(IMG_FILE_BTNSPL_D_R); //IMG_FILE_BTNSPL_D_R;
		WORD iWidth = (int)(52 * g_data.fMinscale + 0.5);
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			iWidth = (int)(52 * 0.5 + 0.5);
		}

		DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataL, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + iWidth, pbnctrl->controlinfo.they,
			pbnctrl->controlinfo.thew - (iWidth * 2), pbnctrl->controlinfo.theh, iDataM, FALSE);
		DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iWidth, pbnctrl->controlinfo.they,
			iWidth, pbnctrl->controlinfo.theh, iDataR, FALSE);
	}

	BUTTON_INFO* btnInfo;
	btnInfo = FindBtnInfoFromID(pbnctrl->controlinfo.control_id);
	StringInfo stStringInfoCOMPLETE;
	stStringInfoCOMPLETE.enumFontType = FZLTHJW;
	stStringInfoCOMPLETE.iFontSize = CalcFontSize(40); //(int)(40 * g_data.fMinscale);

	CHAR16 btnText[30] = { 0 };
	
	if (g_dCountDown >= 0)
	{
		SPrint(btnText, 30, L"%s (%d)", btnInfo->pText, g_dCountDown);
	}
	//else
	//{
	//	SPrint(btnText, 30, L"%s", btnInfo->pText);
	//}

	DWORD dwTextLen = 0;
	GetStringLen(btnText, &stStringInfoCOMPLETE, &dwTextLen);
	//MyLog(LOG_DEBUG, L"dwTextLen: %d", dwTextLen);

	if (pbnctrl->controlinfo.state == CONTROL_STATE_DISABLE)
	{
		DisplayStringEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - dwTextLen) / 2, 
			pbnctrl->controlinfo.they + (pbnctrl->controlinfo.theh - stStringInfoCOMPLETE.iFontSize) / 2, 
			COLOR_TEXT_DISABLE, 
			btnText,
			&stStringInfoCOMPLETE, 
			TRUE);
	}
	else
	{
		DisplayStringEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - dwTextLen) / 2, 
			pbnctrl->controlinfo.they + (pbnctrl->controlinfo.theh - stStringInfoCOMPLETE.iFontSize) / 2, 
			COLOR_TEXT_TITLE, 
			btnText,
			&stStringInfoCOMPLETE, 
			TRUE);
	}

	showmouse();
	return;
}

void ButtonInHomePage_showEx(struct CONTROL * pCtrl)
{

	struct BUTTON *pbnctrl = (struct BUTTON *)pCtrl;

	if (!pbnctrl->btn_unfocuspcx)//纯文字按钮
		return;
	hidemouse();

	if (pbnctrl->clear_background)
	{
		DrawLine(pCtrl->thex, pCtrl->they, pCtrl->thew, pCtrl->theh, COLOR_DEFAULT_BACKGROUND);
	}

	WORD iDataLT = IMG_FILE_HOMEBTN_LT;
	WORD iDataRT = IMG_FILE_HOMEBTN_RT;
	WORD iDataLB = IMG_FILE_HOMEBTN_LB;
	WORD iDataRB = IMG_FILE_HOMEBTN_RB;
	WORD iDataTM = IMG_FILE_HOMEBTN_TM;
	WORD iDataBM = IMG_FILE_HOMEBTN_BM;
	WORD iDataLM = IMG_FILE_HOMEBTN_LM;
	WORD iDataRM = IMG_FILE_HOMEBTN_RM;
	WORD iDataIcon = IMG_FILE_HOMEBTN_ICON_BACKUP;
	WORD iDataRestoreIcon = IMG_FILE_HOMEBTN_ICON_RESTORE;
	WORD iCornerW = 78 * g_data.fMinscale;
	WORD iCornerH = 78 * g_data.fMinscale;
	WORD iMH = 78 * g_data.fMinscale;
	WORD iMW = 78 * g_data.fMinscale;
	WORD iIconW = 202 * g_data.fMinscale;
	WORD iIconH = 174 * g_data.fMinscale;
	WORD iIconYOffSet = 197 * g_data.fMinscale;
	WORD iTxtYOffSetFromBtm = 144 * g_data.fMinscale;
	DWORD iColor = COLOR_MAIN_PAGE_INFO;
	if (pbnctrl->controlinfo.state == CONTROL_STATE_NORMAL)
	{
		iDataLT = GetImageFileId(IMG_FILE_HOMEBTN_LT); //IMG_FILE_HOMEBTN_LT;
		iDataRT = GetImageFileId(IMG_FILE_HOMEBTN_RT); //IMG_FILE_HOMEBTN_RT;
		iDataLB = GetImageFileId(IMG_FILE_HOMEBTN_LB); //IMG_FILE_HOMEBTN_LB;
		iDataRB = GetImageFileId(IMG_FILE_HOMEBTN_RB); //IMG_FILE_HOMEBTN_RB;
		iDataTM = GetImageFileId(IMG_FILE_HOMEBTN_TM); //IMG_FILE_HOMEBTN_TM;
		iDataBM = GetImageFileId(IMG_FILE_HOMEBTN_BM); //IMG_FILE_HOMEBTN_BM;
		iDataLM = GetImageFileId(IMG_FILE_HOMEBTN_LM); //IMG_FILE_HOMEBTN_LM;
		iDataRM = GetImageFileId(IMG_FILE_HOMEBTN_RM); //IMG_FILE_HOMEBTN_RM;
		iDataIcon = GetImageFileId(IMG_FILE_HOMEBTN_ICON_BACKUP); //IMG_FILE_HOMEBTN_ICON_BACKUP;
		iDataRestoreIcon = GetImageFileId(IMG_FILE_HOMEBTN_ICON_RESTORE); //IMG_FILE_HOMEBTN_ICON_RESTORE;
		iColor = COLOR_MAIN_PAGE_INFO;

	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_FOCUS) {
		iDataLT = GetImageFileId(IMG_FILE_HOMEBTN_LT_H); //IMG_FILE_HOMEBTN_LT_H;
		iDataRT = GetImageFileId(IMG_FILE_HOMEBTN_RT_H); //IMG_FILE_HOMEBTN_RT_H;
		iDataLB = GetImageFileId(IMG_FILE_HOMEBTN_LB_H); //IMG_FILE_HOMEBTN_LB_H;
		iDataRB = GetImageFileId(IMG_FILE_HOMEBTN_RB_H); //IMG_FILE_HOMEBTN_RB_H;
		iDataTM = GetImageFileId(IMG_FILE_HOMEBTN_TM_H); //IMG_FILE_HOMEBTN_TM_H;
		iDataBM = GetImageFileId(IMG_FILE_HOMEBTN_BM_H); //IMG_FILE_HOMEBTN_BM_H;
		iDataLM = GetImageFileId(IMG_FILE_HOMEBTN_LM_H); //IMG_FILE_HOMEBTN_LM_H;
		iDataRM = GetImageFileId(IMG_FILE_HOMEBTN_RM_H); //IMG_FILE_HOMEBTN_RM_H;
		iDataIcon = GetImageFileId(IMG_FILE_HOMEBTN_ICON_BACKUP_H); //IMG_FILE_HOMEBTN_ICON_BACKUP_H;
		iDataRestoreIcon = GetImageFileId(IMG_FILE_HOMEBTN_ICON_RESTORE_H); //IMG_FILE_HOMEBTN_ICON_RESTORE_H;
		iColor = COLOR_EXIT_MSG_BOX_BK;

	}
	else if (pbnctrl->controlinfo.state == CONTROL_STATE_PRESSED) {
		iDataLT = GetImageFileId(IMG_FILE_HOMEBTN_LT_P); //IMG_FILE_HOMEBTN_LT_P;
		iDataRT = GetImageFileId(IMG_FILE_HOMEBTN_RT_P); //IMG_FILE_HOMEBTN_RT_P;
		iDataLB = GetImageFileId(IMG_FILE_HOMEBTN_LB_P); //IMG_FILE_HOMEBTN_LB_P;
		iDataRB = GetImageFileId(IMG_FILE_HOMEBTN_RB_P); //IMG_FILE_HOMEBTN_RB_P;
		iDataTM = GetImageFileId(IMG_FILE_HOMEBTN_TM_P); //IMG_FILE_HOMEBTN_TM_P;
		iDataBM = GetImageFileId(IMG_FILE_HOMEBTN_BM_P); //IMG_FILE_HOMEBTN_BM_P;
		iDataLM = GetImageFileId(IMG_FILE_HOMEBTN_LM_P); //IMG_FILE_HOMEBTN_LM_P;
		iDataRM = GetImageFileId(IMG_FILE_HOMEBTN_RM_P); //IMG_FILE_HOMEBTN_RM_P;
		iDataIcon = GetImageFileId(IMG_FILE_HOMEBTN_ICON_BACKUP_P); //IMG_FILE_HOMEBTN_ICON_BACKUP_P;
		iDataRestoreIcon = GetImageFileId(IMG_FILE_HOMEBTN_ICON_RESTORE_P); //IMG_FILE_HOMEBTN_ICON_RESTORE_P;
		iColor = COLOR_EXIT_MSG_BOX_BK;
	}
	//LT
	DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they,
		iCornerW, iCornerH, iDataLT, FALSE);
	//RT
	DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iCornerW, pbnctrl->controlinfo.they,
		iCornerW, iCornerH, iDataRT, FALSE);
	//LB
	DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they + pbnctrl->controlinfo.theh- iCornerH,
		iCornerW, iCornerH, iDataLB, FALSE);
	//RB
	DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iCornerW, pbnctrl->controlinfo.they + pbnctrl->controlinfo.theh - iCornerH,
		iCornerW, iCornerH, iDataRB, FALSE);
	//T
	DisplayImgEx(pbnctrl->controlinfo.thex + iCornerW, pbnctrl->controlinfo.they,
		pbnctrl->controlinfo.thew - iCornerW * 2, iCornerH, iDataTM, FALSE);
	//B
	DisplayImgEx(pbnctrl->controlinfo.thex + iCornerW, pbnctrl->controlinfo.they + pbnctrl->controlinfo.theh - iCornerH,
		pbnctrl->controlinfo.thew - iCornerW * 2, iCornerH, iDataBM, FALSE);
	//L
	DisplayImgEx(pbnctrl->controlinfo.thex, pbnctrl->controlinfo.they + iCornerH,
		iCornerW, pbnctrl->controlinfo.theh - iCornerH * 2, iDataLM, FALSE);
	//R
	DisplayImgEx(pbnctrl->controlinfo.thex + pbnctrl->controlinfo.thew - iCornerW, pbnctrl->controlinfo.they + iCornerH,
		iCornerW, pbnctrl->controlinfo.theh - iCornerH * 2, iDataRM, FALSE);
	//M
	DrawLine(pbnctrl->controlinfo.thex + iCornerW, pbnctrl->controlinfo.they + iCornerH,
		pbnctrl->controlinfo.thew - iCornerW * 2, pbnctrl->controlinfo.theh - iCornerH * 2, iColor);
	
	//TEXT
	StringInfo stStringInfoText;
	stStringInfoText.enumFontType = FZLTZHJW;
	stStringInfoText.iFontSize = CalcFontSize(60); //60 * g_data.fMinscale;
	DWORD dwTextLen = 0;
	if (pbnctrl->controlinfo.control_id == IDB_BACKUP)
	{
		GetStringLen(STRING_HOMEBTNTEXT_BACKUP, &stStringInfoText, &dwTextLen);
		SetTextBKColor(iColor);
		DisplayStringEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - dwTextLen) / 2, pbnctrl->controlinfo.they + pbnctrl->controlinfo.theh - iTxtYOffSetFromBtm - stStringInfoText.iFontSize, COLOR_TEXT_TITLE, STRING_HOMEBTNTEXT_BACKUP, &stStringInfoText, TRUE);
		//ICON
		DisplayImgEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - iIconW) / 2, pbnctrl->controlinfo.they + iIconYOffSet,
			iIconW, iIconH, iDataIcon, FALSE);
	}
	else
	{
		GetStringLen(STRING_HOMEBTNTEXT_RESTORE, &stStringInfoText, &dwTextLen);
		SetTextBKColor(iColor);
		DisplayStringEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - dwTextLen) / 2, pbnctrl->controlinfo.they + pbnctrl->controlinfo.theh - iTxtYOffSetFromBtm - stStringInfoText.iFontSize, COLOR_TEXT_TITLE, STRING_HOMEBTNTEXT_RESTORE, &stStringInfoText, TRUE);
		//ICON
		DisplayImgEx(pbnctrl->controlinfo.thex + (pbnctrl->controlinfo.thew - iIconW) / 2, pbnctrl->controlinfo.they + iIconYOffSet,
			iIconW, iIconH, iDataRestoreIcon, FALSE);
	}

	showmouse();
	return;
}


void  Button_dispatchmsg(struct CONTROL * pbuttonctrl,struct DM_MESSAGE *dmessage)//click,判断范围
{
    struct BUTTON * pbnctrl = (struct BUTTON *)pbuttonctrl;
	BOOL bRedraw = TRUE;

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
		//MyLog(LOG_DEBUG, L"==Press ENTER or Space.\n");
        break;
	case WM_CTL_DISABLE:
		pbnctrl->controlinfo.state = CONTROL_STATE_DISABLE;
		break;
    case WM_CTL_ACTIVATE:
		pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
        break;
    case WM_CTL_INACTIVATE:
	case WM_CTL_ENABLE:
		pbnctrl->controlinfo.state = CONTROL_STATE_NORMAL;
        break;

	case WM_MOUSEMOVEIN: //鼠标移入
		pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
		pbnctrl->ifpressed = NO;
		break;
	case WM_MOUSEMOVEOUT: //鼠标移出
		pbnctrl->controlinfo.state = CONTROL_STATE_NORMAL;
		pbnctrl->ifpressed = NO;
		break;
	case WM_TIME_COUNT_DOWN:
		if (pbnctrl->bIsCountDownBtn)
		{
			//MyLog(LOG_MESSAGE, L"===WM_TIME_COUNT_DOWN");
			if (g_dCountDown > 0)
			{
				g_dCountDown--;
				bRedraw = TRUE;
				dmessage->message = WM_TIME_COUNT_DOWN;
			}
			else
			{
				//pbnctrl->controlinfo.state = CONTROL_STATE_FOCUS;
				dmessage->message = WM_BUTTON_CLICK;
			}
		}
		break;
	default:
		bRedraw = FALSE;
		break;
    }

	if (bRedraw)
	{
		if (TRUE == pbnctrl->bIsCountDownBtn)
		{
			ButtonSplicingCountdown_showEx(&pbnctrl->controlinfo);
			return;
		}
		//else
		//{
		//	MyLog(LOG_DEBUG, L"FALSE pbnctrl->bIsCountDownBtn");
		//}

		if (0 == pbnctrl->btn_title)
		{
			Button_showEx(&pbnctrl->controlinfo);
		}
		else if (1 == pbnctrl->btn_title)
		{
			ButtonSplicing_showEx(&pbnctrl->controlinfo);
		}
		else if (2 == pbnctrl->btn_title)
		{
			ButtonInHomePage_showEx(&pbnctrl->controlinfo);
		}
	}
		

    return ;
}

void btn_default(PCONTROL pcon,struct DM_MESSAGE* msg)
{
	Button_dispatchmsg(pcon,msg); // 默认处理函数

	if (msg->message == WM_BUTTON_CLICK)
	{
		//MyLog(LOG_DEBUG, L"========btn_default pcon->pwin: 0x%x, pcon->control_id: 0x%x", pcon->pwin, pcon->control_id);
		Window_exit(pcon->pwin, pcon->control_id);
	}
}

//
int Button_init(struct BUTTON *pbnctrl,WORD x,WORD y,WORD width,WORD height,
		WORD presspcx, WORD focuspcx, WORD unfocuspcx, WORD title)
{
	mymemset(pbnctrl, 0, sizeof(BUTTON));
    pbnctrl->controlinfo.thex=x;
    pbnctrl->controlinfo.they=y;
    pbnctrl->controlinfo.thew=width;
    pbnctrl->controlinfo.theh=height;
    pbnctrl->controlinfo.status=0;
    pbnctrl->controlinfo.type=IDC_BUTTON;
    pbnctrl->controlinfo.thez=0;
    pbnctrl->controlinfo.sendmessage = Button_dispatchmsg;
	pbnctrl->bIsCountDownBtn = FALSE;

	if (0 == title)
	{
		pbnctrl->controlinfo.show = Button_showEx;
	}
	else if (1 == title)
	{
		pbnctrl->controlinfo.show = ButtonSplicing_showEx;
	}
	else if (2 == title)
	{
		pbnctrl->controlinfo.show = ButtonInHomePage_showEx;
	}
    if(!unfocuspcx && title)
    {
    	pbnctrl->btn_unfocuspcx = IMG_FILE_BTN_NORMAL;
    	//pbnctrl->btn_focuspcx = IMG_FILE_BTN_FOCUS;
    	pbnctrl->btn_pressedpcx = IMG_FILE_BTN_NORMAL;
    }
    else
    {
    	pbnctrl->btn_unfocuspcx=unfocuspcx;
    	pbnctrl->btn_focuspcx=focuspcx;
    	pbnctrl->btn_pressedpcx=presspcx;
    }
	pbnctrl->btn_disablepcx = 0;
    pbnctrl->btn_title = title;
    pbnctrl->ifpressed = 0;
	pbnctrl->clear_background = 0;
	pbnctrl->pbindctl = NULL;

    return 0;
}

////////////////////////////////////////////////////////////////// listbox
void Listbox_delallitem(PLISTBOX plistbox)
{
	if (plistbox != NULL)
	{
		if (plistbox->item_num == 0)
		{
			return;
		}
		plistbox->item_num = 0;
		plistbox->whichselect = 0;
		plistbox->firstitem_inwindow = 0;
		plistbox->buffer = plistbox->itemlist[0].item_buff;
	}
}

void Listbox_SetbkColor(PLISTBOX plistbox, DWORD backgroundColor, DWORD frameColor, DWORD selectItemBkColor)
{
	if (plistbox != NULL)
	{
		plistbox->backgroundColor = backgroundColor;
		plistbox->frameColor = frameColor;
		plistbox->selectItemBkColor = selectItemBkColor;
	}
}

WORD  Listbox_Addoneline(PLISTBOX plistbox)
{
	WORD item_index = plistbox->item_num;

	plistbox->itemlist[item_index].item_buff = plistbox->buffer;
	plistbox->itemlist[item_index].columns = 0;
	return (++plistbox->item_num);
}

WORD  Listbox_Addoneunit(PLISTBOX plistbox,WORD type,VOID *buff)
{
    WORD len = 0;
//	char buff1[4096];
    WORD  *curr_buffer=(WORD  *)plistbox->buffer;
    ASSERT(plistbox->item_num!=0);

	if (type&ITEM_TYPE_ISCONTROL)
	{
		len = sizeof(PBYTE);
	}
	else if (type&ITEM_TYPE_IMG)
	{
		len = (WORD)StrLength(buff) * 2 + 2;
	}
	else if (type&ITEM_TYPE_CHAR_STRING) 
	{
		len = (WORD)StrLength(buff) * 2 + 2;
		//	StrnCpy(buff1,buff,4096);
	}
	else if (type&ITEM_TYPE_DISKBTN)
	{
		len = (WORD)StrLength(buff) * 2 + 2;
		//	StrnCpy(buff1,buff,4096);
	}
    curr_buffer[0]=type;
    curr_buffer[1]=len;
    CopyMem(((PBYTE)curr_buffer+4),buff,len);
    plistbox->buffer=(VOID *)(len+4+(PBYTE)plistbox->buffer);

    return (++plistbox->itemlist[plistbox->item_num-1].columns);

//	return 0;
}


void Listbox_destroy(PLISTBOX plistbox)
{
	  if(plistbox->orgbuffer)
	  {
	  	 FreePool(plistbox->orgbuffer);
	  	 plistbox->orgbuffer = plistbox->buffer = NULL;
	  }
 //   FreePool(plistbox->buffer);

}
////最多显示100字符
//void PutstrinWidth(WORD x,WORD y,CHAR16 *str,DWORD forecolor,WORD width, StringInfo *pstStringInfo)
//{
//	//MyLog(LOG_DEBUG, L"PutstrinWidth");
//
//    CHAR16 buff[200] = { 0 };
//    DWORD i;
//    DWORD len=(DWORD)StrLength(str);
//	DWORD maxchars = width / (PIXEL_PER_CHAR * 2);// PIXEL_PER_CHAR;
//
//    if (len<=maxchars) {
//		DisplayStringEx(x, y, forecolor, str, pstStringInfo, TRUE);
//        return;
//    }
//    CopyMem(buff,str,200);
//    buff[maxchars]=0;
//    for (i=(maxchars-3);i<(maxchars);i++) {
//        if (i<0)
//            continue;
//        buff[i]=0x002e;
//    }
//    //fix 乱码
//    i=0;
//    while (i<maxchars) {
//        if (buff[i]<0x80)
//            i++;
//        else {
//            if (buff[i+1]<0x80) {
//                buff[i]=0x20;
//            }
//            i+=2;
//        }
//    }
//
//	DisplayStringEx(x, y, forecolor, buff, pstStringInfo, TRUE);
//    return;
//
//}
// 得到字符串打印需要多宽（象素）
WORD GetWidthofString(CHAR16 *str)
{
    return (WORD)PIXEL_PER_CHAR*(WORD)StrLength(str);//12*12字体
}

int Listbox_showoneitem(PLISTBOX plistbox ,WORD item_index,WORD current_select,DWORD backcolor,DWORD forecolor)
{
    BYTE buff[200] = { 0 };
    WORD *str;
    WORD j;
    WORD x,y;
    WORD width;
	if (item_index >= plistbox->item_num)
	{
		return 0;
	}

	WORD dwSpan = LIST_SPAN * g_data.fMinscale;
	if (plistbox->item_num > plistbox->items_per_win)
	{
		dwSpan = 0;
	}

	x = plistbox->controlinfo.thex;// +dwSpan;
	y = current_select * plistbox->height_peritem + plistbox->controlinfo.they;

    //Linebox(x,y,plistbox->controlinfo.thew,plistbox->height_peritem,COLOR_BLACK); //DEBUG
	CopyMem(&buff, plistbox->itemlist[item_index].item_buff, 200);

//	if (backcolor!=COLOR_WHITE)
	FillRect(x + 1, y + 1, plistbox->controlinfo.thew - 2 + dwSpan, plistbox->height_peritem, backcolor);
	DrawLine(x + LIST_SPAN * g_data.fMinscale, y + plistbox->height_peritem, plistbox->controlinfo.thew - LIST_SPAN * g_data.fMinscale, 1, COLOR_LIST_SPLIT_LINE);

    str=(WORD *)buff;

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
		WORD dwTextTop = y + (SEL_REC_POINT_LIST_ITEM_HEIGHT * g_data.fMinscale - stStringInfo.iFontSize)  / 2;

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
				CHAR16 buffer[100];
				SetMem(buffer, 200, 0);
				WORD *strText = (WORD *)(str[1] + 4 + (PBYTE)str);
				SPrint(buffer, 100, L"%s", (CHAR16 *)((PBYTE)strText + 4));
				
				if (StrnCmp(buffer, STRING_ORI_DISK, StrLen(STRING_ORI_DISK)) == 0)
				{
					dwImageID = GetImageFileId(IMG_FILE_ORI_DISK_SEL); //IMG_FILE_ORI_DISK_SEL;
				}
				if (StrnCmp(buffer, STRING_NEW_DISK, StrLen(STRING_NEW_DISK)) == 0)
				{
					dwImageID = GetImageFileId(IMG_FILE_NEW_DISK_SEL); //IMG_FILE_NEW_DISK_SEL;
				}

				strText = NULL;
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

// 重新显示全部行
void Listbox_showitems(PLISTBOX plistbox ,int focus )
{
	if (NULL != plistbox)
	{
		//MyLog(LOG_DEBUG, L"Listbox_showitems.\n");
		if (g_isListViewUpdating)
		{
			return;
		}
		g_isListViewUpdating = TRUE;
		int width = plistbox->controlinfo.thew;
		WORD i;
		DWORD forecolor, backcolor;
		WORD item = plistbox->firstitem_inwindow;

		if (focus == YES)
		{
			backcolor = plistbox->selectItemBkColor;
			forecolor = COLOR_TEXT_CONTENT;
			//  	backcolor = COLOR_CLIENT;
			//	forecolor = COLOR_LIST_ACTIVATE;
			//	forecolor = COLOR_LIST_INACTIVATE;
		}
		else
		{
			//backcolor=LISTBOX_SELLINE_COLOR_INAC;
			//forecolor=COLOR_BLACK;
			backcolor = COLOR_LIST_BK; // COLOR_LIST_SELECTED;
			forecolor = COLOR_TEXT_CONTENT;
		}

		hidemouse();
		//background
		//FillRect(plistbox->controlinfo.thex + LIST_SPAN * g_data.fMinscale,plistbox->controlinfo.they+1,plistbox->controlinfo.thew - LIST_SPAN * g_data.fMinscale,plistbox->controlinfo.theh-2,COLOR_LIST_BK);
		FillRect(plistbox->controlinfo.thex + 1, plistbox->controlinfo.they + 1, plistbox->controlinfo.thew - 1, plistbox->controlinfo.theh - 2, plistbox->backgroundColor);
		// FillRect(plistbox->controlinfo.thex+1,plistbox->controlinfo.they+1,plistbox->controlinfo.thew-1,plistbox->controlinfo.theh-2,COLOR_CLIENT);

		if (plistbox->item_num != 0)
		{
			for (i = 0; i < plistbox->items_per_win; i++)
			{
				if (i == plistbox->whichselect)
				{
					plistbox->showoneitem(plistbox, item, i, backcolor, forecolor);
				}
				else
				{
					plistbox->showoneitem(plistbox, item, i, plistbox->backgroundColor, COLOR_TEXT_CONTENT);
				}
				item++;
				if (item >= plistbox->item_num)
				{
					break;
				}
			}
		}
		showmouse();
		g_isListViewUpdating = FALSE;
	}
    return ;
}

// 重新显示全部行
void Listbox_showitems_ex(PLISTBOX plistbox, int focus)
{
	if (NULL != plistbox)
	{
		//MyLog(LOG_DEBUG, L"Listbox_showitems.\n");
		if (g_isListViewUpdating)
		{
			return;
		}
		g_isListViewUpdating = TRUE;

		int width = plistbox->controlinfo.thew;
		WORD i;
		DWORD forecolor, backcolor;
		WORD item = plistbox->firstitem_inwindow;

		if (focus == YES)
		{
			backcolor = plistbox->selectItemBkColor;
			forecolor = COLOR_TEXT_CONTENT;
		}
		else
		{
			backcolor = COLOR_LIST_BK; // COLOR_LIST_SELECTED;
			forecolor = COLOR_TEXT_CONTENT;
		}

		hidemouse();

		BufferData stBuffData;
		stBuffData.pBuffData = AllocatePool(plistbox->controlinfo.thew * plistbox->controlinfo.theh * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
		stBuffData.buffWidth = plistbox->controlinfo.thew;
		stBuffData.buffHeight = plistbox->controlinfo.theh;
		if (NULL != stBuffData.pBuffData)
		{
			BufferDrawLine(stBuffData.pBuffData, 0, 0, plistbox->controlinfo.thew, plistbox->controlinfo.theh, plistbox->backgroundColor);

			if (plistbox->item_num != 0)
			{
				EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffDataPos = stBuffData.pBuffData;

				plistbox->pstBuffData = &stBuffData;

				for (i = 0; i < plistbox->items_per_win; i++)
				{
					if (i == plistbox->whichselect)
					{
						plistbox->showoneitem(plistbox, item, i, backcolor, forecolor);
					}
					else
					{
						plistbox->showoneitem(plistbox, item, i, plistbox->backgroundColor, COLOR_TEXT_CONTENT);
					}
					item++;
					plistbox->pstBuffData->pBuffData += plistbox->height_peritem * stBuffData.buffWidth;

					if (item >= plistbox->item_num)
					{
						break;
					}
				}

				plistbox->pstBuffData->pBuffData = NULL;
				plistbox->pstBuffData = NULL;

				stBuffData.pBuffData = buffDataPos;

				PutImgEx(plistbox->controlinfo.thex, plistbox->controlinfo.they, stBuffData.buffWidth, stBuffData.buffHeight, stBuffData.pBuffData);
				FreePool(stBuffData.pBuffData);
			}
		}

		showmouse();
		g_isListViewUpdating = FALSE;
	}
	return;
}


// 显示列表框界面，处于未激活状态。
void Listbox_show(struct CONTROL * pcontrol  )
{
	if (NULL != pcontrol)
	{
		PLISTBOX plistbox = (PLISTBOX)pcontrol;
		if (NULL != plistbox)
		{
			if (pcontrol->type != IDC_LISTBOX)
				ASSERT(0);

			hidemouse();
			//Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh-1,0xD7D5D6 );
			DrawLine(pcontrol->thex, pcontrol->they, pcontrol->thew, pcontrol->theh, plistbox->backgroundColor);
			Linebox(pcontrol->thex, pcontrol->they, pcontrol->thew, pcontrol->theh, plistbox->frameColor);
			showmouse();

			plistbox->showitems(plistbox, YES);
		}
	}

    return ;
}
void Listbox_activate(struct CONTROL * pcontrol  )
{
	if (NULL != pcontrol)
	{
		PLISTBOX plistbox = (PLISTBOX)pcontrol;
		if (NULL != plistbox)
		{
			hidemouse();
			plistbox->showoneitem(plistbox, plistbox->whichselect + plistbox->firstitem_inwindow,
				plistbox->whichselect, plistbox->selectItemBkColor, COLOR_TEXT_CONTENT);
			showmouse();
		}
	}
    return;

}

void Listbox_inactivate(struct CONTROL * pcontrol  )
{
	if (NULL != pcontrol)
	{
		PLISTBOX plistbox = (PLISTBOX)pcontrol;
		if (NULL != plistbox)
		{
			hidemouse();
			plistbox->showoneitem(plistbox, plistbox->whichselect + plistbox->firstitem_inwindow,
				plistbox->whichselect, plistbox->selectItemBkColor, COLOR_TEXT_CONTENT);
			showmouse();
		}
	}
    return ;

}
void Changeline(PLISTBOX plistbox,WORD newline)
{
	if (NULL != plistbox)
	{
		MyLog(LOG_DEBUG, L"Changeline %d", newline);
		hidemouse();

		if (plistbox->controlinfo.pwin->windowID == 501)
		{
			plistbox->whichselect = newline;
			plistbox->showitems(plistbox, YES);
		}
		else
		{
			plistbox->showoneitem(plistbox, plistbox->whichselect + plistbox->firstitem_inwindow,
				plistbox->whichselect, plistbox->backgroundColor, COLOR_TEXT_CONTENT);	//COLOR_WHITE,COLOR_BLACK);
		   // plistbox->showoneitem( plistbox, plistbox->whichselect+plistbox->firstitem_inwindow,plistbox->whichselect ,COLOR_WHITE,COLOR_BLACK);
			plistbox->whichselect = newline;
			plistbox->showoneitem(plistbox, plistbox->whichselect + plistbox->firstitem_inwindow,
				plistbox->whichselect, plistbox->selectItemBkColor, COLOR_TEXT_CONTENT);
		}


		if (plistbox->pBindCtrlListbox != NULL)
		{
			//MyLog(LOG_DEBUG, L"update pListbox");
			//更新绑定项目
			struct DM_MESSAGE msg;
			int index = plistbox->firstitem_inwindow + plistbox->whichselect;
			msg.thex = g_seldisk_list_info[index].disknum; //plistbox->whichselect;
			//MyLog(LOG_DEBUG, L"update pListbox disknum: %d", msg.thex);
			msg.they = plistbox->item_num;
			msg.message = WM_UPDATE_PART_LIST;
			Control_sendmsg((PCONTROL)plistbox->pBindCtrlListbox, &msg);
		}
		showmouse();
	}
}
//当列表框的内容发生变化时，需要调用此函数更新显示
void Listbox_updateview(PLISTBOX plistbox)
{
	if (NULL != plistbox)
	{
		//MyLog(LOG_DEBUG, L"Listbox_updateview");
		plistbox->showitems(plistbox, YES);
		//MyLog(LOG_DEBUG,L"after showitem");
		//	getch();
		if (plistbox->pscroll != NULL) { //更新滚动条
			struct DM_MESSAGE msg;
			msg.thex = plistbox->firstitem_inwindow;
			msg.they = plistbox->item_num;
			msg.message = WM_SCROLLBOX_UPDATE;
			//	((PCONTROL)(plistbox->pscroll))->sendmessage((PCONTROL)plistbox->pscroll,&msg);
			//debug(100,100,WM_SCROLLBOX_UPDATE);
			//getch();
			Control_sendmsg((PCONTROL)plistbox->pscroll, &msg);
		}
		if (plistbox->pBindCtrlListbox != NULL)
		{
			//MyLog(LOG_DEBUG, L"update pListbox");
			//更新绑定项目
			struct DM_MESSAGE msg;
			int index = plistbox->firstitem_inwindow + plistbox->whichselect;
			msg.thex = g_seldisk_list_info[index].disknum; //plistbox->whichselect;
			//MyLog(LOG_DEBUG, L"update pListbox disknum: %d", msg.thex);
			msg.they = plistbox->item_num;
			msg.message = WM_UPDATE_PART_LIST;
			Control_sendmsg((PCONTROL)plistbox->pBindCtrlListbox, &msg);
		}
		//MyLog(LOG_DEBUG,L"after update pscroll");
		//getch();
	}
}
int Listbox_DispatchMsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmessage)
{
	if (NULL != pcontrol && NULL != dmessage)
	{
		PLISTBOX plistbox = (PLISTBOX)pcontrol;
		if (NULL != plistbox)
		{
			int ifneedupdate = 0;
			//debug(0,0,dmessage->message);
			if (g_isListViewUpdating)
			{
				return 0;
			}
			if (plistbox->item_num == 0)
				return 0;
			switch (dmessage->message) {
			case CHAR_UP:
				if (plistbox->whichselect == 0) {
					if (plistbox->firstitem_inwindow == 0)
					{
						dmessage->message = WM_LISTBOX_SELCHANGE;
						break;
					}
					plistbox->firstitem_inwindow--;
					ifneedupdate = 1;
				}
				else {
					Changeline(plistbox, plistbox->whichselect - 1);
				}
				dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				break;
			case CHAR_DOWN:
				if (plistbox->whichselect == (plistbox->item_num - 1))//只有一屏,且已经到最后
				{
					dmessage->message = WM_LISTBOX_SELCHANGE;
					break;
				}
				if (plistbox->whichselect == (plistbox->items_per_win - 1))
				{
					if ((plistbox->firstitem_inwindow + plistbox->items_per_win) == plistbox->item_num)
					{
						dmessage->message = WM_LISTBOX_SELCHANGE;
						break;
					}
					plistbox->firstitem_inwindow++;
					ifneedupdate = 1;
				}
				else {

					Changeline(plistbox, plistbox->whichselect + 1);
				}
				dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				break;
			case CHAR_PGUP:
				if ((plistbox->firstitem_inwindow == 0) || (plistbox->item_num <= plistbox->items_per_win))
					break;
				if (plistbox->firstitem_inwindow >= plistbox->items_per_win)
					plistbox->firstitem_inwindow -= plistbox->items_per_win;
				else
					plistbox->firstitem_inwindow = 0;

				ifneedupdate = 1;
				dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息

				break;
			case CHAR_PGDOWN:
			{
				WORD nextpg_item = plistbox->firstitem_inwindow + plistbox->items_per_win;
				if ((nextpg_item == plistbox->item_num) || (plistbox->item_num <= plistbox->items_per_win))
					break;
				if ((plistbox->item_num - nextpg_item) > plistbox->items_per_win)//下一屏够显示
					plistbox->firstitem_inwindow = nextpg_item;
				else
					plistbox->firstitem_inwindow = plistbox->item_num - plistbox->items_per_win;
				ifneedupdate = 1;
				dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				break;
			}
			case WM_LEFTBUTTONPRESSED:
			{
				//MyLog(LOG_DEBUG, L"WM_LEFTBUTTONPRESSED \n");
				WORD clickitem = ((WORD)dmessage->they - pcontrol->they) / plistbox->height_peritem;
				if (clickitem >= plistbox->item_num) //当前行没有数据
				{
					break;
				}

				//MyLog(LOG_DEBUG, L"clickitem %d firstitem:%d items:%d\n", clickitem, plistbox->firstitem_inwindow, plistbox->items_per_win);

				if (clickitem == plistbox->items_per_win)
				{
					if (plistbox->whichselect == (plistbox->item_num - 1))//只有一屏,且已经到最后
					{
						dmessage->message = WM_LISTBOX_SELCHANGE;
						break;
					}

					Changeline(plistbox, plistbox->items_per_win - 1);

					if (plistbox->whichselect == (plistbox->items_per_win - 1))
					{
						if ((plistbox->firstitem_inwindow + plistbox->items_per_win) == plistbox->item_num)
						{
							dmessage->message = WM_LISTBOX_SELCHANGE;
							break;
						}
						plistbox->firstitem_inwindow++;
						ifneedupdate = 1;
					}
					dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				}
				else
				{
					Changeline(plistbox, clickitem);
				}

				dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				break;
			}
			case WM_CTL_ACTIVATE:

				Listbox_activate((PCONTROL)plistbox);
				break;
			case WM_CTL_INACTIVATE:
				Listbox_inactivate((PCONTROL)plistbox);
				break;
			case WM_SCROLL_UP: //窗口向上移动一行
			case WM_MOUSE_MID_SCROLL_UP:
				//MyLog(LOG_MESSAGE, L"WM_SCROLL_UP...");
				if (plistbox->firstitem_inwindow != 0) {
					plistbox->firstitem_inwindow--;
					if (plistbox->firstitem_inwindow < 0)
					{
						plistbox->firstitem_inwindow = 0;
					}
					ifneedupdate = 1;
					dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				}
				break;
			case WM_SCROLL_DOWN: //窗口向下移动一行
			case WM_MOUSE_MID_SCROLL_DOWN:
				//MyLog(LOG_MESSAGE, L"WM_SCROLL_DOWN...");
				if ((plistbox->firstitem_inwindow + plistbox->items_per_win) < plistbox->item_num) {
					plistbox->firstitem_inwindow++;
					ifneedupdate = 1;
					dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				}
				break;
			case WM_SCROLL_MOVE: //拖动滚动条
				//MyLog(LOG_DEBUG, L"WM_SCROLL_MOVE %d %d\n", plistbox->firstitem_inwindow, dmessage->thex);
				if (plistbox->firstitem_inwindow != dmessage->thex) {
					plistbox->firstitem_inwindow = (WORD)dmessage->thex;
					plistbox->showitems(plistbox, YES);
					dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				}
				break;
			case ' ':
				//	case WM_DOUBLECLICKED:
			case CHAR_RETURN:
				//MyLog(LOG_DEBUG, L"====Listbox_DispatchMsg: Pressed RETURN or space key.===========");
				dmessage->message = WM_LISTBOX_SELCHANGE;// 删除已经处理的消息
				dmessage->thex = plistbox->firstitem_inwindow + plistbox->whichselect;
				break;
			}

			if (ifneedupdate == 1) {
				Listbox_updateview(plistbox);
			}
		}
	}
    return 0;
}

//滚动条在次函数调用完之后设置。
int Listbox_init(PLISTBOX plistbox,WORD x,WORD y,WORD width,WORD hight,WORD items_per_win,DWORD buffer_size)
{
    plistbox->item_num=0;
    plistbox->whichselect=0;
    plistbox->firstitem_inwindow=0;
    plistbox->buffer = AllocatePool(buffer_size);
    plistbox->orgbuffer = plistbox->buffer;
    plistbox->pscroll=NULL;
    plistbox->ptitle=NULL;
	plistbox->pBindCtrlListbox = NULL;
    plistbox->controlinfo.status=0;
    plistbox->controlinfo.show=Listbox_show;
    plistbox->controlinfo.type=IDC_LISTBOX;
    plistbox->controlinfo.sendmessage=Listbox_DispatchMsg;
    plistbox->controlinfo.thex=x;
    plistbox->controlinfo.they=y;
    plistbox->controlinfo.thez=0;
    plistbox->controlinfo.thew=width;
    plistbox->controlinfo.theh=hight;
	plistbox->showoneitem = Listbox_showoneitem;
	plistbox->showitems = Listbox_showitems;
    plistbox->items_per_win=items_per_win;
    if(items_per_win == 0){     		//解决除数为零的问题
      return EFI_INVALID_PARAMETER;
     }
    plistbox->height_peritem=hight/items_per_win;
	plistbox->backgroundColor = COLOR_LIST_BK;
	plistbox->frameColor = COLOR_LIST_SPLIT_LINE;
	plistbox->selectItemBkColor = COLOR_LIST_SELECTED;
	plistbox->pstBuffData = NULL;

    return 0;
}

int Listbox_initEx(PLISTBOX plistbox, WORD x, WORD y, WORD width, WORD hight, WORD height_peritem, DWORD buffer_size)
{
	plistbox->item_num = 0;
	plistbox->whichselect = 0;
	plistbox->firstitem_inwindow = 0;
	plistbox->buffer = AllocatePool(buffer_size);
	plistbox->orgbuffer = plistbox->buffer;
	plistbox->pscroll = NULL;
	plistbox->ptitle = NULL;
	plistbox->pBindCtrlListbox = NULL;
	plistbox->controlinfo.status = 0;
	plistbox->controlinfo.show = Listbox_show;
	plistbox->controlinfo.type = IDC_LISTBOX;
	plistbox->controlinfo.sendmessage = Listbox_DispatchMsg;
	plistbox->controlinfo.thex = x;
	plistbox->controlinfo.they = y;
	plistbox->controlinfo.thez = 0;
	plistbox->controlinfo.thew = width;
	plistbox->controlinfo.theh = hight;
	plistbox->showoneitem = Listbox_showoneitem;
	plistbox->showitems = Listbox_showitems;
	if (height_peritem == 0) {     		//解决除数为零的问题
		return EFI_INVALID_PARAMETER;
	}
	plistbox->items_per_win = (DWORD)(hight / height_peritem);// items_per_win;
	plistbox->height_peritem = height_peritem;// hight / items_per_win;
	plistbox->backgroundColor = COLOR_LIST_BK;
	plistbox->frameColor = COLOR_LIST_SPLIT_LINE;
	plistbox->selectItemBkColor = COLOR_LIST_SELECTED;
	plistbox->pstBuffData = NULL;

	return 0;
}

///////////////////////////////////////////////// SCROLL BOX

void _scrollbox_calcmoveheight(PSCROLL_BOX pscroll);

//滚动条可以和任何控件绑定，例如listbox,以后的文本框等



//显示滚动条移动，只部分显示,更新move_y,begin_y
//distence: 新的拖动条位置
//是否第一次显示

void _scrollbox_movebox_show(PSCROLL_BOX pscroll,WORD distence,int ifinitshow )
{
    WORD max_y,curr_y;
    WORD x=pscroll->controlinfo.thex;
    DWORD color;


	curr_y = (WORD)pscroll->move_y + distence;
	max_y = pscroll->range_height + pscroll->range_y - pscroll->move_height;
	if (curr_y > max_y)
		curr_y = max_y;
	else if (curr_y < (WORD)pscroll->range_y)
		curr_y = (WORD)pscroll->range_y;

	if (pscroll->drag_status == 1)
		color = SCROLLBOX_DRAGBOX_SEL_COLOR;
	else
		color = SCROLLBOX_DRAGBOX_COLOR;

	hidemouse();

	if (ifinitshow == 1)
	{
		//FillRect(x + 1, pscroll->range_y, pscroll->controlinfo.thew - 2, pscroll->range_height - 1, pscroll->backgroundColor);//显示背景
		FillRect(x, pscroll->range_y, pscroll->controlinfo.thew, pscroll->range_height - 1, pscroll->backgroundColor);//显示背景
	}
	else
	{
		//overwrite dragbox
		WORD y, height;
		if (pscroll->move_y > curr_y)
		{
			y = curr_y + pscroll->move_height + 1;
			height = pscroll->move_y - curr_y;
		}
		else if (pscroll->move_y < curr_y)
		{
			y = pscroll->move_y;
			height = curr_y - y;
		}
		else
		{
			height = 0;
		}
		if (height)
		{
			//FillRect(x+1,pscroll->move_y,pscroll->controlinfo.thew-2,pscroll->move_height,SCROLLBOX_RANGE_COLOR);//显示拖动条
			//FillRect(x + 1, y, pscroll->controlinfo.thew - 2, height, pscroll->backgroundColor);//显示拖动条
			FillRect(x, y, pscroll->controlinfo.thew, height, pscroll->backgroundColor);//显示拖动条
		}
	}

    pscroll->move_y=curr_y;
    if (pscroll->move_height!=0) 
	{
        //FillRect(x+1,curr_y,pscroll->controlinfo.thew-2,pscroll->move_height,color);//显示拖动条
		FillRect(x, curr_y, pscroll->controlinfo.thew, pscroll->move_height, color);//显示拖动条
    }
    showmouse();
}

void Scrollbox_show(struct CONTROL * pcontrol )
{
	//MyLog(LOG_DEBUG, L"Scrollbox_show.\n");

    PSCROLL_BOX pscroll=(PSCROLL_BOX)pcontrol;

	if (pscroll->totalline <= pscroll->line_perwin)
	{
		//MyLog(LOG_MESSAGE, L"Scrollbox_show totalline<=line_perwin does not show scroll.");
		return;
	}

    //Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh-1, SCROLLBOX_LINE_COLOR);
    //pscroll->high_btn.controlinfo.status = 1;
    //pscroll->low_btn.controlinfo.status = 1;
    //pscroll->high_btn.controlinfo.show((struct CONTROL*)&pscroll->high_btn);
    //pscroll->low_btn.controlinfo.show((struct CONTROL*)&pscroll->low_btn);
    _scrollbox_movebox_show(pscroll,0,1);
    //Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh - 1, pscroll->frameColor);
	//DrawLine(pcontrol->thex, pcontrol->they, 1, pcontrol->theh, pscroll->backgroundColor);

    return ;
}


//void Scrollbox_activate(struct CONTROL * pcontrol )
//{
//	return;
//	PSCROLL_BOX pscroll=(PSCROLL_BOX)pcontrol;
//	pscroll->plistbox->controlinfo.activate((struct CONTROL*)pscroll->plistbox);

//}

//void Scrollbox_inactivate(struct CONTROL * pcontrol )
//{
//	return;
//	PSCROLL_BOX pscroll=(PSCROLL_BOX)pcontrol;
//	pscroll->plistbox->controlinfo.inactivate((struct CONTROL*)pscroll->plistbox);

//}

// 当滚动条情况发生改变时，由列表框调用，或由滚动条自身调用此函数，用于部分
//更新滚动条的状态。
//函数根据列表框当前状态，计算滚动条位置,并更新move_y
//todo 只更新拖动条
void Scrollbox_updatemove(PSCROLL_BOX pscroll, WORD firstitem_inwindow, WORD totalline)
{
	WORD new_y;

	if (pscroll->totalline <= pscroll->line_perwin)
	{
		//MyLog(LOG_MESSAGE, L"Scrollbox_updatemove totalline <= line_perwin  does not show scroll.");
		//MyLog(LOG_MESSAGE, L"totalline %d line_perwin %d", pscroll->totalline, pscroll->line_perwin);
		return;
	}

	if (totalline != pscroll->totalline)
	{ 
		//重新计算拖动条大小
		pscroll->totalline = totalline;
		_scrollbox_calcmoveheight(pscroll);
		//pscroll->move_height=(pscroll->line_perwin*pscroll->range_height)/totalline;
	}

	if (totalline == 0) 
	{
		_scrollbox_movebox_show(pscroll, 0, 1);
	}
	else 
	{
		new_y = (pscroll->range_height*firstitem_inwindow) / pscroll->totalline;
		//	debug(100,0,firstitem_inwindow);
		//	debug(100,20,new_y+pscroll->range_y);

		if ((new_y + pscroll->move_height) > pscroll->range_height)
			new_y = pscroll->range_height - pscroll->move_height; //不能超出range范围内
		new_y += pscroll->range_y;//绝对坐标

		//	if (new_y==pscroll->move_y)
		//		return; //no need update
		_scrollbox_movebox_show(pscroll, (int)new_y - (int)pscroll->move_y, 1);
	}
}

//void extern_control_dispatchmsg(struct CONTROL * pcontrol, struct DM_MESSAGE *dmmessage)
//{
//	WORD y;
//	struct DM_MESSAGE mymsg;
//	EXTERN_BUTTON *pextern_button = (EXTERN_BUTTON *)pcontrol;
//	BOOL bRedraw = TRUE;
//
//	// 	Button_dispatchmsg(pcontrol,dmmessage); // 默认处理函数
//
//	//   MyLog(LOG_MESSAGE, L"extern_message %x.",dmmessage->message);
//
//	switch (dmmessage->message) {
//
//		//   case CHAR_LEFT:
//		//   case CHAR_DOWN:
//		//   case CHAR_RIGHT:
//
//		//        break; 
//	   //		case WM_DOUBLECLICKED:
//	case WM_LEFTBUTTONPRESSED:
//		pextern_button->controlinfo.state = CONTROL_STATE_PRESSED;
//		dmmessage->message = 0;
//		break;
//	case WM_LEFTBUTTONRELEASE:
//
//		pextern_button->controlinfo.state = CONTROL_STATE_NORMAL;
//
//		switch (pextern_button->controlinfo.control_id)
//		{
//		case IDB_PAGE_UP:
//			pextern_button->controlinfo.status = 1;
//
//			mymsg.message = CHAR_UP;
//
//			Control_sendmsg(pextern_button->pbindctl, &mymsg);
//			dmmessage->message = 0;
//
//			break;
//		case IDB_PAGE_DOWN:
//			pextern_button->controlinfo.status = 1;
//
//			mymsg.message = CHAR_DOWN;
//			Control_sendmsg(pextern_button->pbindctl, &mymsg);
//			dmmessage->message = 0;
//
//			break;
//		}
//
//		break;
//	default:  //其余消息均发给绑定的控件
//		bRedraw = FALSE;
//		Control_sendmsg((PCONTROL)pextern_button->pbindctl, dmmessage);
//		break;
//	}
//
//	if (bRedraw)
//	{
//		Button_show(&pextern_button->controlinfo);
//	}
//
//	//  MyLog(LOG_MESSAGE, L" extern control over\n");
//
//}



void Scrollbox_dispatchmsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmmessage)
{
    WORD y;
    struct DM_MESSAGE mymsg;
    PSCROLL_BOX pscroll=(PSCROLL_BOX)pcontrol;

	WORD z;

	//MyLog(LOG_DEBUG, L"Scrollbox msg %x\n", mymsg.message);

//	if ((ifmousemsg(dmmessage) ||
//		(dmmessage->message == WM_MOUSEMOVEOUT)) &&
//		(pscroll->drag_status == 1) &&
//		(dmmessage->message != WM_MOUSEDRAG) &&
//		(dmmessage->message != WM_MOUSE_MID_SCROLL_UP) &&
//		(dmmessage->message != WM_MOUSE_MID_SCROLL_DOWN))
//	{
//		//MyLog(LOG_DEBUG, L"finish drag send WM_SCROLL_MOVE\n");
//
//		//结束拖动
//		mymsg.message = WM_SCROLL_MOVE;
//		//更新拖动条
//		pscroll->drag_status = 0;
//		_scrollbox_movebox_show(pscroll, 0, 0);
//		//发送消息给绑定控件
//		mymsg.thex = ((WORD)(pscroll->move_y - pscroll->range_y)*(WORD)(pscroll->totalline)) / (WORD)(pscroll->range_height);//窗口第一个item
////				debug(200,20,pscroll->move_y);
////				debug(200,40,pscroll->range_y);
////				debug(200,60,pscroll->range_height);
////
////				debug(200,0,mymsg.thex);
//		//pscroll->pbindctl->sendmessage((PCONTROL)pscroll->pbindctl,&mymsg);
//		//Control_sendmsg((PCONTROL)pscroll->pbindctl, &mymsg);
//		return;
//	}

	switch (dmmessage->message)
	{
		//		case WM_DOUBLECLICKED:
	case WM_LEFTBUTTONPRESSED:
		//MyLog(LOG_DEBUG, L"WM_LEFTBUTTONPRESSED\n");
		y = (WORD)dmmessage->they;
		pscroll->whichbutton = 0xffff;

		//if (y <= pscroll->range_y) 
		//{ 
		//	MyLog(LOG_DEBUG, L"Up button y: %d <= pscroll->range_y: %d\n", y, pscroll->range_y);
		//	//上按钮
		//	//pscroll->high_btn.controlinfo.sendmessage((PCONTROL)&pscroll->high_btn,dmmessage);
		//	Control_sendmsg((PCONTROL)&pscroll->high_btn, dmmessage);
		//	pscroll->whichbutton = 0;
		//}
		//else if (y >= pscroll->low_btn.controlinfo.they) 
		//{ 
		//	MyLog(LOG_DEBUG, L"Down button y: %d >= pscroll->low_btn.controlinfo.they: %d\n", y, pscroll->low_btn.controlinfo.they);
		//	//下按钮
		// //pscroll->low_btn.controlinfo.sendmessage((PCONTROL)&pscroll->low_btn,dmmessage);
		//	Control_sendmsg((PCONTROL)&pscroll->low_btn, dmmessage);
		//	pscroll->whichbutton = 1;
		//}
		//else 

		if (y < pscroll->move_y) 
		{ 
			//MyLog(LOG_DEBUG, L"CHAR_PGUP y: %d < pscroll->move_y: %d\n", y, pscroll->move_y);
			//拖动条上方
			mymsg.message = CHAR_PGUP;
			//pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
			Control_sendmsg(pscroll->pbindctl, &mymsg);
			pscroll->whichbutton = 0;
		}
		else if (y > (pscroll->move_y + pscroll->move_height))
		{
			//MyLog(LOG_DEBUG, L"CHAR_PGDOWN y: %d > (pscroll->move_y: %d + pscroll->move_height: %d)\n", y, pscroll->move_y, pscroll->move_height);
			//拖动条下方
			mymsg.message = CHAR_PGDOWN;
			//pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
			Control_sendmsg(pscroll->pbindctl, &mymsg);
			pscroll->whichbutton = 1;
		}
		else 
		{ 
			//MyLog(LOG_DEBUG, L"Drag\n");
			//拖动条
			pscroll->whichbutton=2;
			pscroll->drag_status = 1;
			pscroll->drag_begin_y = (WORD)dmmessage->they;
			//显示
			_scrollbox_movebox_show(pscroll, 0, 0);
		}
		break;
	case WM_LEFTBUTTONRELEASE:
		//MyLog(LOG_DEBUG, L"WM_LEFTBUTTONRELEASE whichbutton:%d\n", pscroll->whichbutton);
	{
		//switch (pscroll->whichbutton) 
		//{
		//case 0:
		//	//MyLog(LOG_DEBUG, L"WM_SCROLL_UP\n");
		//	//pscroll->high_btn.controlinfo.sendmessage((PCONTROL)&pscroll->high_btn,dmmessage);
		//	//Control_sendmsg((PCONTROL)&pscroll->high_btn, dmmessage);
		//	mymsg.message = WM_SCROLL_UP;
		//	//pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
		//	Control_sendmsg(pscroll->pbindctl, &mymsg);
		//	break;
		//case 1:
		//	//MyLog(LOG_DEBUG, L"WM_SCROLL_DOWN\n");
		//	//pscroll->low_btn.controlinfo.sendmessage((PCONTROL)&pscroll->low_btn,dmmessage);
		//	//Control_sendmsg((PCONTROL)&pscroll->low_btn, dmmessage);
		//	mymsg.message = WM_SCROLL_DOWN;
		//	//pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
		//	Control_sendmsg(pscroll->pbindctl, &mymsg);
		//	break;
		//case 2:
		//	y = (WORD)dmmessage->they;

		//	if (y < pscroll->move_y)
		//	{
		//		//拖动条上方
		//		//MyLog(LOG_DEBUG, L"WM_SCROLL_UP\n");
		//		//Control_sendmsg((PCONTROL)&pscroll->high_btn, dmmessage);
		//		mymsg.message = CHAR_PGUP;				
		//		Control_sendmsg(pscroll->pbindctl, &mymsg);
		//	}
		//	else if (y > (pscroll->move_y + pscroll->move_height))
		//	{
		//		//拖动条下方
		//		//MyLog(LOG_DEBUG, L"WM_SCROLL_DOWN\n");
		//		//Control_sendmsg((PCONTROL)&pscroll->low_btn, dmmessage);
		//		mymsg.message = WM_SCROLL_DOWN;
		//		Control_sendmsg(pscroll->pbindctl, &mymsg);
		//	}

		//	break;
		//}
		pscroll->whichbutton = 0xffff;
	}
		break;
	case WM_MOUSE_MID_SCROLL_UP:
	{
		//MyLog(LOG_DEBUG, L"WM_MOUSE_MID_SCROLL_UP\n");
		mymsg.message = WM_SCROLL_UP;
		Control_sendmsg(pscroll->pbindctl, &mymsg);

		break;
	}
	case WM_MOUSE_MID_SCROLL_DOWN:
	{
		//MyLog(LOG_DEBUG, L"WM_MOUSE_MID_SCROLL_DOWN \n");
		mymsg.message = WM_SCROLL_DOWN;
		Control_sendmsg(pscroll->pbindctl, &mymsg);

		break;
	}
	case WM_MOUSEDRAG:
		//MyLog(LOG_DEBUG, L"WM_MOUSEDRAG %d\n", pscroll->drag_status);
		if (pscroll->drag_status == 1 && pscroll->whichbutton == 2)
		{
			_scrollbox_movebox_show(pscroll, (WORD)dmmessage->they - pscroll->drag_begin_y, 0);
			pscroll->drag_begin_y = (WORD)dmmessage->they;
			/////////////
			//pscroll->whichbutton = 2;
			mymsg.message = WM_SCROLL_MOVE;
			if ((WORD)(pscroll->range_height != 0))
			{
				mymsg.thex = ((WORD)(pscroll->move_y - pscroll->range_y)*(WORD)(pscroll->totalline)) / (WORD)(pscroll->range_height);
			}
			else
			{
				mymsg.thex = 0;
			}
			Control_sendmsg(pscroll->pbindctl, &mymsg);
		}
		break;
	case WM_SCROLLBOX_UPDATE:
		//MyLog(LOG_DEBUG, L"WM_SCROLLBOX_UPDATE\n");
		Scrollbox_updatemove(pscroll, (WORD)dmmessage->thex, (WORD)dmmessage->they);
		break;
	default:  //其余消息均发给绑定的控件
		//MyLog(LOG_DEBUG, L"Default\n");
		//pscroll->pbindctl->sendmessage((PCONTROL)pscroll->pbindctl,dmmessage);
		Control_sendmsg((PCONTROL)pscroll->pbindctl, dmmessage);
		break;
	}

}
void _scrollbox_calcmoveheight(PSCROLL_BOX pscroll)
{
	//MyLog(LOG_DEBUG, L"_scrollbox_calcmoveheight\n");
    WORD height;
    if (pscroll->totalline==0) {
        pscroll->move_height=0;
        return;
    }
    height=(pscroll->line_perwin*pscroll->range_height)/pscroll->totalline;
    if (height>=pscroll->range_height) //不能太大，太大就取消滚动条。
        height=0;
    else if (height<=5) //不能小于5个象素，否则看不清楚。
        height=5;
    pscroll->move_height=height;
}

// totalitem : 总的行数,
// window: 一屏的行数.
int _scrollbox_init (PSCROLL_BOX pscroll, WORD x,WORD y,WORD width,WORD height,WORD totalitem,WORD window)
{
    pscroll->controlinfo.thex=x;
    pscroll->controlinfo.they=y;
    pscroll->controlinfo.thew=width;
    pscroll->controlinfo.theh=height;
    pscroll->controlinfo.thez=0;

//	pscroll->controlinfo.activate=NULL;
//	pscroll->controlinfo.inactivate=NULL;
    pscroll->controlinfo.show=Scrollbox_show;
    pscroll->controlinfo.type=IDC_SCROLLBOX;
    pscroll->controlinfo.status=CON_STATUS_TABNOTSTOP;
    pscroll->controlinfo.sendmessage=Scrollbox_dispatchmsg;

    //Button_init(&pscroll->high_btn,x,y,width,width,IMG_FILE_UPARROWB,IMG_FILE_UPARROWA, IMG_FILE_UPARROWA, 0);
    //Button_init(&pscroll->low_btn,x,y+height-width,width,width,IMG_FILE_DOWNARROWB,IMG_FILE_DOWNARROWA,IMG_FILE_DOWNARROWA, 0);
    
	pscroll->range_height = height;// -2 * width;
	pscroll->range_y = y;// +width;
    pscroll->line_perwin=window;
    pscroll->totalline=totalitem;

    _scrollbox_calcmoveheight(pscroll);
	pscroll->move_y = y;// +width;
    pscroll->drag_status=0;
    return 0;
}


// bindctl: 需要绑定的控件,此控件必须先初始化，且已经additem。
// width: 滚动条宽度
int Scrollbox_init(PSCROLL_BOX pscroll,PCONTROL bindctl,WORD totalitem,WORD item_per_win)
{
    _scrollbox_init (pscroll,bindctl->thex+bindctl->thew + (26 - 16) / 2 * g_data.fMinscale,bindctl->they,\
                     (DWORD)(SCROLLBOX_WIDTH * g_data.fMinscale),bindctl->theh,totalitem,item_per_win);

    pscroll->pbindctl = bindctl;
	pscroll->frameColor = ((PLISTBOX)bindctl)->frameColor;
	pscroll->backgroundColor = ((PLISTBOX)bindctl)->backgroundColor;
//	bindctl->pscroll=pscroll;
    return 0;
}

void Titlebox_show(struct CONTROL * pcontrol )
{
	WORD i;
	WORD width = pcontrol->thew;
	WORD thex = pcontrol->thex;
	WORD they = pcontrol->they;
	PTITLEBOX ptitle = (PTITLEBOX)pcontrol;
	hidemouse();

	//DrawLine(thex, they + 1, width, 1, TITLEBOX_HIGHLINE_COLOR);
	//FillRect(thex, they + 2, width, pcontrol->theh - 5, TITLEBOX_BG_COLOR);

	//DrawLine(thex, they + 23, width, 1, TITLEBOX_LOWHLINE1_COLOR);
	//DrawLine(thex, they + 24, width, 1, TITLEBOX_LOWHLINE2_COLOR);
	//DrawLine(thex, they + 25, width, 1, TITLEBOX_LOWHLINE3_COLOR);

	FillRect(thex, they, width, pcontrol->theh, TITLEBOX_BG_COLOR);
	Linebox(thex, they, width, pcontrol->theh, COLOR_LIST_SPLIT_LINE);

	WORD dwTextHeight = TITLEBOX_TEXT_HEIGHT * g_data.fMinscale;
	WORD dwTextTop = they + (pcontrol->theh - dwTextHeight) / 2;

	WORD dwLineTop = they + 4 * g_data.fMinscale;
	WORD dwLineHeight = pcontrol->theh - 8 * g_data.fMinscale;

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(FONT_LIST_TITLE_TEXT_SIZE); //(int)(FONT_LIST_TITLE_TEXT_SIZE * g_data.fFontScale);

	for (i = 0; i < ptitle->column_num; i++)
	{
		WORD x = ptitle->column_list[i].begin_x;

		//PutstrinWidth(x + 4, dwTextTop, ptitle->column_list[i].column_name, COLOR_TEXT_LIST_TITLE, ptitle->column_list[i].curr_width - 4, &stStringInfo);
		DisplayStringInWidthEx(x + 4, dwTextTop, COLOR_TEXT_LIST_TITLE, ptitle->column_list[i].column_name, &stStringInfo, ptitle->column_list[i].curr_width - 4, 1, TRUE);
		if (i == 0)
		{
			continue;
		}
		////Hide the drag line
		//DrawLine(x, dwLineTop, 1, dwLineHeight, COLOR_LIST_SPLIT_LINE);
		//DrawLine(x + 1, dwLineTop, 1, dwLineHeight, COLOR_WHITE);
	}
	showmouse();

	return;

}
void Titlebox_move(PTITLEBOX ptitle,int dx)
{
    WORD i;
    WORD move;
    PONECOLUMN pcon=ptitle->column_list;
    WORD cur_col=ptitle->drag_col;
    if (!ptitle->drag_status)
        return;
    if (dx>0) {//向右拉
        for (i=(ptitle->column_num-1);i>=cur_col;i--) {
            move=pcon[i].curr_width-TITLEBOX_LEAST_WIDTH;
            if (dx<move)
                move=(WORD)dx;
            pcon[i-1].curr_width+=move;
            pcon[i].curr_width-=move;
            pcon[i].begin_x+=move;
        }
        Titlebox_show((PCONTROL)ptitle);
    } else if (dx<0) {

        move=pcon[cur_col-1].curr_width-TITLEBOX_LEAST_WIDTH;
        dx=-dx;
        if (move>dx)
            move=(WORD)dx;
        pcon[cur_col-1].curr_width-=move;
        for (i=cur_col;i<ptitle->column_num;i++) {
            pcon[i].begin_x-=move;
        }
        pcon[ptitle->column_num-1].curr_width+=move;
        Titlebox_show((PCONTROL)ptitle);
    }

}

void Titlebox_dispatchmsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmmessage)
{
    PTITLEBOX ptitle=(PTITLEBOX)pcontrol;
    WORD whichcol=0xffff;
    WORD i;
    struct DM_MESSAGE msg;
    msg.message=WM_CTL_ACTIVATE;
//	debug(300,0,dmmessage->message);

//	debug(300,20,dmmessage->thex);
//	debug(300,40,dmmessage->they);
//	debug(300,60,ptitle->drag_status);
//
//
    if ( (ifmousemsg(dmmessage)==NO) && (dmmessage->message!=WM_MOUSEMOVEOUT) )
        return ;
    if (ptitle->drag_status==1) {
        if (dmmessage->message!=WM_MOUSEDRAG) { //拖动结束
            //更新列表框
            int dx=dmmessage->thex-ptitle->drag_begin_x;
            Titlebox_move(ptitle,dx);

            ptitle->plistbox->controlinfo.show((PCONTROL)ptitle->plistbox);
            //ptitle->plistbox->controlinfo.sendmessage((PCONTROL)ptitle->plistbox,&msg);
            Control_sendmsg((PCONTROL)ptitle->plistbox,&msg);

            ptitle->drag_status=0;
            dmmessage->message=0;
            //下面更新光标
        } else {  		//正在拖动
            // 更新标题框
//			int dx=dmmessage->thex-ptitle->drag_begin_x;
//			for(i=plistbox->drag_col;i<plistbox->column_num;i++){
//				ptitle->column_list[i].begin_x+=dx;
//			}
//			ptitle->drag_begin_x=dmmessage->thex;
//			Titlebox_update();
            return;
        }
    }

	WORD dwTitleboxHeight = (WORD)(TITLEBOX_HEIGHT * g_data.fMinscale);
    // 修改光标
    if (ptitle->drag_status==0) 
	{
		if ((dmmessage->they < pcontrol->they) || 
			(dmmessage->they >= (pcontrol->they + dwTitleboxHeight)))
		{
			goto nochange_cursor;
		}
		////Hide the drag line, does not change cursor
  //      //是否在敏感区域内
		//for (i = 1; i < ptitle->column_num; i++) 
		//{
		//	if ((dmmessage->thex > (ptitle->column_list[i].begin_x - 5)) && 
		//		(dmmessage->thex < (ptitle->column_list[i].begin_x + 5))) 
		//	{
		//		whichcol = i;
		//		//在敏感区域内，改变形状
		//		if (ptitle->cursor_changed == NO)
		//		{
		//			//					    debug(0,500,(u16)&Cursor_move);
		//			//					    getch();
		//			ptitle->old_curosr = change_cursor(&Cursor_move);
		//			ptitle->cursor_changed = YES;
		//		}
		//	}
		//}
nochange_cursor:
        //不在敏感区域,改变光标形状
        if (whichcol==0Xffff)
		{
            if (ptitle->cursor_changed) 
			{
//		        debug(0,750,(u16)&Cursor_move);
//			    getch( );
                change_cursor(ptitle->old_curosr);
                ptitle->cursor_changed=NO;
            }
        }

    }
    //当不在拖动时，并且鼠标在敏感区域点击时，标记开始拖动
    if (dmmessage->message==WM_LEFTBUTTONPRESSED) {
        if ( whichcol!=0xffff) {
            ptitle->drag_status=1;
            ptitle->drag_col=whichcol;
            ptitle->drag_begin_x=(WORD)dmmessage->thex;
        }
    }
//	debug(300,60,ptitle->drag_status);
//	debug(300,80,whichcol);


}
// 高度固定，y位置随plistbox确定；
int Titlebox_init(PTITLEBOX ptitle,PLISTBOX plistbox, WORD width,PONECOLUMN cols,WORD cols_num)
{
    WORD i,x;

	WORD dwTitleboxHeight = (WORD)(TITLEBOX_HEIGHT * g_data.fMinscale);
    x=plistbox->controlinfo.thex;
    ptitle->controlinfo.thex=x;
    ptitle->controlinfo.they= plistbox->controlinfo.they - dwTitleboxHeight;
    ptitle->controlinfo.thew=width;
    ptitle->controlinfo.theh= dwTitleboxHeight;
    ptitle->controlinfo.thez=0;
    ptitle->controlinfo.status=CON_STATUS_TABNOTSTOP;
    ptitle->controlinfo.type=IDC_TITLEBOX;
//	ptitle->controlinfo.inactivate=NULL;
//	ptitle->controlinfo.activate=NULL;
    ptitle->controlinfo.show=Titlebox_show;
    ptitle->controlinfo.sendmessage=Titlebox_dispatchmsg;

    ptitle->column_num=cols_num;
    ptitle->column_list=cols;
    ptitle->drag_status=0;
    ptitle->cursor_changed=0;

    for (i=0;i<cols_num;i++) {
        width=ptitle->column_list[i].orig_width;
        ptitle->column_list[i].curr_width=width;
        ASSERT(width>TITLEBOX_LEAST_WIDTH);
        ptitle->column_list[i].begin_x=x + (WORD)(LIST_SPAN * g_data.fMinscale);
        x+=width;
    }
    ptitle->plistbox=plistbox;
    plistbox->ptitle=ptitle;
    return 0;
}



///////////////////////////////////////////////////////////////////////////////// edit box

void DisplayDefaultChar(PEDITBOX peditbox)
{
	//MyLog(LOG_DEBUG, L"DisplayDefaultChar...\n");

	int iStrLen = (WORD)StrLength(peditbox->name);
	//MyLog(LOG_DEBUG, L"iStrLen %d\n", iStrLen);

	if (iStrLen <= 0)
	{
		WORD x;
		WORD i;
		CHAR16 ch;

		x = peditbox->controlinfo.thex + EDIT_BEGIN_X;
		i = 0;

		StringInfo stStringInfo;
		stStringInfo.enumFontType = FZLTHJW;
		stStringInfo.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

		DWORD dwTop = (peditbox->controlinfo.theh - stStringInfo.iFontSize) / 2;

		int iCharSize = 0;

		while (ch = peditbox->defaultName[i]) {
			if (peditbox->type & EDIT_MODE_PASSWORD)
				ch = L'*';
			DisplayChar(x, peditbox->controlinfo.they + dwTop, 0x999999, ch, &iCharSize, &stStringInfo);
			x += iCharSize;
			//if (ch > 0x80)
			//	x += iCharSize;// 2 * EDIT_PIXEL_PER_CHAR;
			//else
			//	x += EDIT_PIXEL_PER_CHAR;
			i++;
		}

		peditbox->defaultName_len = i;

		//MyLog(LOG_DEBUG, L"defaultName_len %d\n", peditbox->defaultName_len);
	}
}

//在指定位置上插入一个字符,pos可以从0-current_len
int  insertachar(PEDITBOX  peditbox,WORD pos,CHAR16 ch)
{
	//MyLog(LOG_ERROR, L"insertachar...\n");

    CHAR16 *p=peditbox->name;
    int i;

    if ((peditbox->insmode==0)&&(pos!=peditbox->current_len)) { //如果处于中间，则直接覆盖，不需要移动
        p[pos]=ch;
        return 0;
    }
    if (peditbox->current_len==peditbox->num)
        return -1;

    //将这个位置上的字符，和后面的字符全部后移，包括\0
    for (i=(peditbox->current_len);i>=(int)pos;i--) {
        p[i+1]=p[i];
    }

    p[pos]=ch;
    peditbox->current_len++;
    return 0;
}

//在指定位置上删除一个字符
int deleteachar(PEDITBOX  peditbox,WORD pos)
{
	//MyLog(LOG_ERROR, L"deleteachar...\n");

    CHAR16 *p=peditbox->name;
    WORD i;

    if (p[pos]==0)//当前已经是末尾
        return -1;
    //将下个字符，和后面的字符全部前移，包括\0
    for (i=(pos+1);i<=(peditbox->current_len);i++) {
        p[i-1]=p[i];
    }
    peditbox->current_len--;
    return 0;
}
//显示下标,当pos=0xffff表示隐藏光标,插入模式显示竖线，替换模式显示横线
void textcusor_move(PEDITBOX  peditbox,WORD pos)
{
	//MyLog(LOG_ERROR, L"textcusor_move...\n");

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	//MyLog(LOG_DEBUG, L"peditbox->posofcursor %d.pos %d\n", peditbox->posofcursor, pos);

	CHAR16 posCursorName[100] = { 0 };
	CopyMem(posCursorName, peditbox->name, peditbox->posofcursor * sizeof(CHAR16));

	DWORD dwTextLength = 0;
	GetStringLen(posCursorName, &stStringInfo, &dwTextLength);
	//MyLog(LOG_DEBUG, L"dwTextLength %d.\n", dwTextLength);

	DWORD dwCursorTextLength = 0;
	if (pos != 0xffff)
	{
		if (pos == peditbox->posofcursor)
		{
			dwCursorTextLength = dwTextLength;
		}
		else
		{

			CHAR16 cursorName[100] = { 0 };
			CopyMem(cursorName, peditbox->name, pos * sizeof(CHAR16));
			GetStringLen(cursorName, &stStringInfo, &dwCursorTextLength);
		}
	}

	//MyLog(LOG_DEBUG, L"dwCursorTextLength %d.\n", dwCursorTextLength);

	WORD oldthex = peditbox->controlinfo.thex + dwTextLength + EDIT_BEGIN_X;
	WORD newthex = peditbox->controlinfo.thex + dwCursorTextLength + EDIT_BEGIN_X;
	DWORD dwTop = (peditbox->controlinfo.theh - stStringInfo.iFontSize) / 2;
	DWORD dwHeight = stStringInfo.iFontSize;
    hidemouse();
    if (peditbox->insmode==1) {
        DrawLine(oldthex,peditbox->controlinfo.they+ dwTop,1, dwHeight, EDITBOX_BACKGROUND);
        if (pos!=0xffff) {
            DrawLine(newthex,peditbox->controlinfo.they+ dwTop,1, dwHeight, COLOR_BLACK);
        }
    } else {
		int iCharSize = 0;

		DisplayChar(oldthex, peditbox->controlinfo.they + dwTop + 2, EDITBOX_BACKGROUND, L'_', &iCharSize, &stStringInfo);
		if (pos != 0xffff) {
			DisplayChar(newthex, peditbox->controlinfo.they + dwTop + 2, COLOR_BLACK, L'_', &iCharSize, &stStringInfo);
		}
    }
    if (pos!=0xffff)
        peditbox->posofcursor=pos;
    showmouse();
}
//显示所有字符,把光标也清除
void puteditstr(PEDITBOX peditbox,DWORD bkcolor,DWORD charcolor)
{
	//MyLog(LOG_ERROR, L"puteditstr...\n");
    WORD i, x;
    CHAR16 ch;

    FillRect(peditbox->controlinfo.thex+1, peditbox->controlinfo.they+1,
		peditbox->controlinfo.thew-2,peditbox->controlinfo.theh-2,bkcolor);

	x = peditbox->controlinfo.thex + EDIT_BEGIN_X;
	i = 0;

	StringInfo stStringInfo;
	stStringInfo.enumFontType = FZLTHJW;
	stStringInfo.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);

	DWORD dwTop = (peditbox->controlinfo.theh - stStringInfo.iFontSize) / 2;
	   
	int iCharSize = 0;

    while(ch = peditbox->name[i]) {
		if (peditbox->type & EDIT_MODE_PASSWORD)
			ch = L'*';
		DisplayChar(x, peditbox->controlinfo.they + dwTop, charcolor, ch, &iCharSize, &stStringInfo);
		x += iCharSize;
		//MyLog(LOG_DEBUG, L"iCharSize %d", iCharSize);
		i++;
    }

	peditbox->current_len = i;
}

void Editbox_show(struct CONTROL * pcontrol)
{
	//MyLog(LOG_ERROR, L"Editbox_show...\n");
    PEDITBOX peditbox=(PEDITBOX)pcontrol;
    hidemouse();
    
    DrawSinkableRectangle(peditbox->controlinfo.thex, peditbox->controlinfo.they,
    	peditbox->controlinfo.thew, peditbox->controlinfo.theh, COLOR_LIST_SPLIT_LINE, EDITBOX_BACKGROUND);
    //FillRect(peditbox->controlinfo.thex,peditbox->controlinfo.they+1,peditbox->controlinfo.thew,peditbox->controlinfo.theh,EDITBOX_BACKGROUND);
    
//	Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh,COLOR_EIDTBOX_UNFOCUS);
    puteditstr(peditbox,EDITBOX_BACKGROUND, COLOR_TEXT_CONTENT);
	DisplayDefaultChar(peditbox);
    showmouse();
    return;
}

void Editbox_Disable(PEDITBOX peditbox)
{
    hidemouse();
    
    DrawSinkableRectangle(peditbox->controlinfo.thex,peditbox->controlinfo.they,
    	peditbox->controlinfo.thew,peditbox->controlinfo.theh, COLOR_SHADOW_GREY, SCROLLBOX_RANGE_COLOR);
    
    //FillRect(peditbox->controlinfo.thex+1,peditbox->controlinfo.they+1,peditbox->controlinfo.thew-1,peditbox->controlinfo.theh-1,SCROLLBOX_RANGE_COLOR);
//	Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh,COLOR_EIDTBOX_UNFOCUS);
    puteditstr(peditbox,SCROLLBOX_RANGE_COLOR, COLOR_TEXT_DISABLE);
	//DisplayDefaultChar(peditbox);
    showmouse();
    return;
}

BOOL IsValidFileName(WCHAR c)
{
	if (c < 0x20  ||
		c == '\"' ||
		c == '\'' ||
		c == '*'  ||
		c == '/'  ||
		c == ':'  ||
		c == '<'  ||
		c == '>'  ||
		c == '?'  ||
		c == '\\' ||
		c == '|'
		) {
			return FALSE;
	}
	return TRUE;
}

void Edit_dispatchmsg(struct CONTROL * pcontrol,struct DM_MESSAGE * pdmsg)
{
    WORD thex,newpos;
    PEDITBOX peditbox=(PEDITBOX)pcontrol;
    int ifneedupdate=0;
    //先判断字母按键
    if (ifkeymsg(pdmsg)) {
        CHAR16 ch=pdmsg->message;
        if ((ch<0x20)||(ch>0x7e)) //no ascii
            goto next_disp;
        if ((peditbox->type & EDIT_MODE_DIGIT)&&((ch<L'0')||(ch>L'9')))
            goto next_disp;
		if(!IsValidFileName(ch))
			goto next_disp;
        //可以处理
        if (insertachar(peditbox,peditbox->posofcursor,ch)<0)
            return; //插入失败

        //	textcusor_move(peditbox,peditbox->posofcursor+1);
        peditbox->posofcursor++;
        ifneedupdate=1;
        pdmsg->message=0;
    }

next_disp:
    switch (pdmsg->message)
    {
    case WM_LEFTBUTTONPRESSED:  //移动下标
		thex = ((WORD)pdmsg->thex - pcontrol->thex);
		if (thex > EDIT_BEGIN_X)
		{
			thex -= EDIT_BEGIN_X;

			StringInfo stStringInfo;
			stStringInfo.enumFontType = FZLTHJW;
			stStringInfo.iFontSize = CalcFontSize(30); //(int)(30 * g_data.fFontScale + 0.5);
			DWORD dwTextLength = 0;
			GetStringLen(peditbox->name, &stStringInfo, &dwTextLength);

			//newpos = thex / g_pixelPerChar;
			if (thex >= dwTextLength)
			{
				newpos = peditbox->current_len;
			}
			else
			{
				int iPos = 1;// thex / dwTextLength * peditbox->num;
				CHAR16 cursorName[100] = { 0 };

				do
				{
					SetMem(cursorName, 100, 0);
					CopyMem(cursorName, peditbox->name, iPos * sizeof(CHAR16));
					DWORD dwStrLen = 0;
					GetStringLen(cursorName, &stStringInfo, &dwStrLen);
					if (thex > dwStrLen)
					{
						if (iPos <= peditbox->current_len)
						{
							iPos++;
							continue;
						}
						else
						{
							newpos = peditbox->current_len;
							break;
						}
					}
					else
					{
						newpos = iPos;
						break;
					}
				} while (TRUE);
			}
		}
		else
		{
			newpos = 0;
		}
        textcusor_move(peditbox,newpos);
        break;
        //	case WM_DOUBLECLICKED:

//			break;

    case CHAR_DELETE:
        deleteachar(peditbox,peditbox->posofcursor);
        ifneedupdate=1;
        break;
    case CHAR_INSERT:
        peditbox->insmode=!peditbox->insmode;
        ifneedupdate=1;
        break;
    case CHAR_HOME:
        textcusor_move(peditbox,0);
        break;
    case CHAR_END:
        textcusor_move(peditbox,peditbox->current_len);
        break;

    case CHAR_LEFT:
        if (peditbox->posofcursor!=0)
            textcusor_move(peditbox,peditbox->posofcursor-1);

        pdmsg->message=0;//消息置空
        break;

    case CHAR_RIGHT:
        if (peditbox->posofcursor!=peditbox->current_len)
            textcusor_move(peditbox,peditbox->posofcursor+1);
        pdmsg->message=0;//消息置空
        break;

    case CHAR_BACKSPACE:
        if (peditbox->posofcursor==0)
            break;
        deleteachar(peditbox,peditbox->posofcursor-1);
        peditbox->posofcursor--;
        //	textcusor_move(peditbox,peditbox->posofcursor-1);
        ifneedupdate=1;
        break;

    case WM_CTL_ACTIVATE:
		//MyLog(LOG_ERROR, L"WM_CTL_ACTIVATE...\n");

        hidemouse();
		Linebox(peditbox->controlinfo.thex, peditbox->controlinfo.they, peditbox->controlinfo.thew, peditbox->controlinfo.theh, COLOR_LIST_SPLIT_LINE);
		int iStrLen = (WORD)StrLength(peditbox->name);
		if (iStrLen <= 0)
		{
			FillRect(peditbox->controlinfo.thex + 1, peditbox->controlinfo.they + 1,
				peditbox->controlinfo.thew - 2, peditbox->controlinfo.theh - 2, EDITBOX_BACKGROUND);
		}

        showmouse();
        textcusor_move(peditbox,peditbox->posofcursor);
        break;
    case WM_CTL_INACTIVATE:
		//MyLog(LOG_ERROR, L"WM_CTL_INACTIVATE...\n");

        hidemouse();
        //Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh+1,COLOR_EIDTBOX_UNFOCUS);
        Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh, COLOR_LIST_SPLIT_LINE);
		DisplayDefaultChar(peditbox);
        showmouse();
        textcusor_move(peditbox,0xffff);
        break;

    case WM_MOUSEMOVEIN: //鼠标移入
        change_cursor(&Cursor_text);
        break;

    case WM_MOUSEMOVEOUT: //鼠标移出
		if (g_data.fMinscale == 1)
		{
			change_cursor(&Cursor_normal_4k);
		}
		else
		{
			change_cursor(&Cursor_normal);
		}

        break;
    case WM_CTL_DISABLE:
		//MyLog(LOG_ERROR, L"WM_CTL_DISABLE...\n");
        Editbox_Disable(peditbox);
        break;
    case WM_CTL_ENABLE:
		//MyLog(LOG_ERROR, L"WM_CTL_ENABLE...\n");
        Editbox_show(pcontrol);
//		default:
//			pdmsg->message=0;//消息置空
//			break;
    }
    //更新显示字符和光标
    if (ifneedupdate==1) {
        hidemouse();
        puteditstr(peditbox,COLOR_WHITE, COLOR_TEXT_CONTENT);
		//DisplayDefaultChar(peditbox);
        showmouse();
        textcusor_move(peditbox,peditbox->posofcursor);
    }
    return;
}

void EditLincencebox_init(struct EDITBOX * peditbox, WORD x, WORD y, WORD width, WORD hight, CHAR16 * str, BYTE type, WORD num)
{
	peditbox->controlinfo.thex = x;
	peditbox->controlinfo.they = y;
	peditbox->controlinfo.thew = width;
	peditbox->controlinfo.theh = hight;
	peditbox->controlinfo.status = 0;
	peditbox->controlinfo.type = IDC_EDITBOX;
	peditbox->controlinfo.thez = 0;
	peditbox->controlinfo.control_id = IDB_EDIT_BTN;

	peditbox->controlinfo.sendmessage = Edit_dispatchmsg;
	peditbox->controlinfo.show = Editbox_show;

	peditbox->name = str;
	peditbox->type = type;
	peditbox->ifselected = NO;//默认不选中
	peditbox->num = num;
	peditbox->current_len = (WORD)StrLength(str);
	peditbox->posofcursor = peditbox->current_len;	//当前下标在末尾
	peditbox->insmode = 1; //默认 插入模式
	return;
}

void Editbox_init(struct EDITBOX * peditbox,WORD x,WORD y,WORD width, WORD height, CHAR16 * str, CHAR16 * defaultName, BYTE type,WORD num)
{
    peditbox->controlinfo.thex=x;
    peditbox->controlinfo.they=y;
    peditbox->controlinfo.thew=width;
	peditbox->controlinfo.theh = height;// 33;
    peditbox->controlinfo.status=0;
    peditbox->controlinfo.type=IDC_EDITBOX;
    peditbox->controlinfo.thez=0;
    peditbox->controlinfo.control_id=IDB_EDIT_BTN;

    peditbox->controlinfo.sendmessage=Edit_dispatchmsg;
    peditbox->controlinfo.show=Editbox_show;

    peditbox->name=str;
	peditbox->defaultName = defaultName;
    peditbox->type=type;
    peditbox->ifselected=NO;//默认不选中
    peditbox->num=num;
    peditbox->current_len=(WORD)StrLength(str);
	peditbox->defaultName_len = (WORD)StrLength(defaultName);
    peditbox->posofcursor=peditbox->current_len;	//当前下标在末尾
    peditbox->insmode=1; //默认 插入模式
    return;
}


/////////////////////////////////////////////////////////////////  Radio box 控件////////////////////////

//设置此项选中
void Radio_show( PCONTROL pcontrol )
{
    PRADIOBOX pradio=(PRADIOBOX)pcontrol;
    FillRect(pcontrol->thex+1,pcontrol->they+2,pcontrol->thew-2,pcontrol->theh-4,RADIO_BG_COLOR);
    //if (pradio->status==0)
        //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_RADIOB,FALSE);
    //else
        //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_RADIOA,FALSE);
    hidemouse();

    DisplayString(pcontrol->thex+20,pcontrol->they+2,COLOR_BLACK,pradio->name);
    showmouse();
    return ;
}
//设置此项选中
int Radio_set(PRADIOBOX pradio )
{
    PRADIOBOX p;
    if (pradio->status==1)
        return 0;

    pradio->status=1;
    ((PCONTROL)pradio)->show((PCONTROL)pradio);
    p=pradio->next;
    while (p!=pradio) {
        p->status=0;
        ((PCONTROL)p)->show((PCONTROL)p);
        p=p->next;
    }
    return 0;
}
//获得多个radio中哪个是选中的，返回序号（指针序列号)
int Radio_get(PRADIOBOX pradio)
{
    int i=0;
    PRADIOBOX p;
    p=pradio;

    while (p->next!=pradio) {
        if (p->status==1)
            break;
        p=p->next;
        i++;
    }
    return i;
}

void Radio_Disable(PCONTROL pcontrol)
{
    PRADIOBOX pradio=(PRADIOBOX)pcontrol;
    //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_RADIOC,FALSE);

    hidemouse();
    DisplayString(pcontrol->thex+20+1,pcontrol->they+2+1,COLOR_WHITE,pradio->name);
    DisplayString(pcontrol->thex+20,pcontrol->they+2,SCROLLBOX_LINE_COLOR,pradio->name);
    showmouse();
    return ;
}
//void Radio_Enable(PCONTROL pcontrol)
//{
//	PRADIOBOX pradio=(PRADIOBOX)pcontrol;
//	hidemouse();
//	PutString(pcontrol->thex+20-1,pcontrol->they+2-1,(WORD)pradio->name,DIALOG_BG_COLOR,0);//去掉
//	PutString(pcontrol->thex+20,pcontrol->they+2,(WORD)pradio->name,COLOR_BLACK,0);
//	showmouse();
//	return ;
//}

void Radio_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage)
{
    PRADIOBOX pradio=(PRADIOBOX)pcontrol;
    switch (dmessage->message ) {
    case WM_LEFTBUTTONPRESSED:
        Radio_set(pradio);
//			hidemouse();
//			DisplayImg(pcontrol->thex+3,pcontrol->they,pradio->radiopcx_sel,0,0,0);
//			showmouse();
        break;
    case WM_CTL_ACTIVATE:
        hidemouse();
//			DrawDotLine( pcontrol->thex,pcontrol->they,pcontrol->thew,1,COLOR_BLACK );
//			DrawDotLine( pcontrol->thex+pcontrol->thew,pcontrol->they,1,pcontrol->theh,COLOR_BLACK );
//			DrawDotLine( pcontrol->thex,pcontrol->they+pcontrol->theh,pcontrol->thew,1,COLOR_BLACK );
//			DrawDotLine( pcontrol->thex,pcontrol->they,1,pcontrol->theh,COLOR_BLACK);

        DotLinebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh,COLOR_BLACK);
        showmouse();
        break;
    case WM_CTL_INACTIVATE:
        hidemouse();
//			DrawLine( pcontrol->thex,pcontrol->they,pcontrol->thew,1,RADIO_BG_COLOR);
//			DrawLine( pcontrol->thex+pcontrol->thew,pcontrol->they,1,pcontrol->theh,RADIO_BG_COLOR);
//			DrawLine( pcontrol->thex,pcontrol->they+pcontrol->theh,pcontrol->thew,1,RADIO_BG_COLOR );
//			DrawLine( pcontrol->thex,pcontrol->they,1,pcontrol->theh,RADIO_BG_COLOR);

        Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh,RADIO_BG_COLOR);
        showmouse();
        break;
    case L' ':
    case CHAR_RETURN:
		//MyLog(LOG_DEBUG, L"====Checkbox_dispatchmsg: Pressed RETURN or space key.===========");
        Radio_set(pradio);
        break;
    case WM_CTL_DISABLE:
        Radio_Disable(pcontrol);
        break;
    case WM_CTL_ENABLE:
        Radio_show(pcontrol);
        //	Radio_Enable(pcontrol);
        break;

    }

}
int Radio_init(PRADIOBOX pradio,WORD x,WORD y,CHAR16 *name,PRADIOBOX attach,BYTE status)
{
    pradio->controlinfo.thex=x;
    pradio->controlinfo.they=y;
    pradio->controlinfo.thew=GetWidthofString(name)+23;
    pradio->controlinfo.theh=20;
    pradio->controlinfo.status=0;
    pradio->controlinfo.type=IDC_RADIOBOX;
    pradio->controlinfo.thez=0;

    pradio->controlinfo.sendmessage=Radio_dispatchmsg;
    pradio->controlinfo.show=Radio_show;
    pradio->status=status;
    if (attach==NULL)
        pradio->next=pradio;
    else {
        PRADIOBOX p;
        p=attach->next;
        attach->next=pradio;
        pradio->next=p;
    }
    pradio->name=name;


    return 0;

}

///////////////////////////////////////////////////////////////////////  CHECK BOX



void Checkbox_show( PCONTROL pcontrol )
{
    PCHECKBOX pcheck=(PCHECKBOX)pcontrol;
    FillRect(pcontrol->thex+1,pcontrol->they+2,pcontrol->thew-2,pcontrol->theh-4,CHECK_BG_COLOR);

   // if (pcheck->status==1)
        //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXA,FALSE);
    //else
        //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXB,FALSE);
    hidemouse();

    DisplayString(pcontrol->thex+20,pcontrol->they+2,COLOR_BLACK,pcheck->name);
    showmouse();
    return ;
}

int Checkbox_Change(PCHECKBOX pcheck)
{
    PCONTROL pcontrol=(PCONTROL)pcheck;
    if (pcheck->status) {
        pcheck->status=0;
        //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXB,FALSE);

    } else {
        pcheck->status=1;
        //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXA,FALSE);

    }
    return 0;

}
int Checkbox_Disable(PCONTROL pcontrol)
{
    PCHECKBOX pcheck=(PCHECKBOX)pcontrol;
    //DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXC,FALSE);

    hidemouse();
    DisplayString(pcontrol->thex+20+1,pcontrol->they+2+1,COLOR_WHITE,pcheck->name);
    DisplayString(pcontrol->thex+20,pcontrol->they+2,SCROLLBOX_LINE_COLOR,pcheck->name);
    showmouse();
    return 0;
}

//int Checkbox_Enable(PCONTROL pcontrol)
//{
//	PCHECKBOX pcheck=(PCHECKBOX)pcontrol;
//	hidemouse();
//	PutString(pcontrol->thex+20-1,pcontrol->they+2-1,(WORD)pcheck->name,DIALOG_BG_COLOR,0);//除掉
//	PutString(pcontrol->thex+20,pcontrol->they+2,(WORD)pcheck->name,COLOR_BLACK,0);
//	showmouse();
//	return 0;
//
//}

void Checkbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage)
{
    PCHECKBOX pcheck=(PCHECKBOX)pcontrol;
    switch (dmessage->message )
    {


    case WM_CTL_ACTIVATE:
        hidemouse();
//			Linex( pcontrol->thex,pcontrol->they,pcontrol->thew,COLOR_BLACK,Line_DOT );
//			Liney( pcontrol->thex+pcontrol->thew,pcontrol->they,pcontrol->theh,COLOR_BLACK,Line_DOT );
//			Linex( pcontrol->thex,pcontrol->they+pcontrol->theh,pcontrol->thew,COLOR_BLACK,Line_DOT );
//			Liney( pcontrol->thex,pcontrol->they,pcontrol->theh,COLOR_BLACK,Line_DOT );
        DotLinebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh,COLOR_BLACK);

        showmouse();
        break;
    case WM_CTL_INACTIVATE:
        hidemouse();
//			Linex( pcontrol->thex,pcontrol->they,pcontrol->thew,RADIO_BG_COLOR,0 );
//			Liney( pcontrol->thex+pcontrol->thew,pcontrol->they,pcontrol->theh,CHECK_BG_COLOR,0 );
//			Linex( pcontrol->thex,pcontrol->they+pcontrol->theh,pcontrol->thew+1,CHECK_BG_COLOR,0 );
//			Liney( pcontrol->thex,pcontrol->they,pcontrol->theh,RADIO_BG_COLOR,0 );
        DotLinebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh,RADIO_BG_COLOR);
        showmouse();
        break;
    case WM_LEFTBUTTONPRESSED:
    case L' ':
    case CHAR_RETURN:
		//MyLog(LOG_DEBUG, L"====Checkbox_dispatchmsg: Pressed RETURN or space key.===========");
        Checkbox_Change(pcheck);
        dmessage->message=WM_CONTEXT_CHANGE;
        break;
    case WM_CTL_DISABLE:
        Checkbox_Disable(pcontrol);
        break;
    case WM_CTL_ENABLE:
        //		pcheck->status=!(pcheck->status);
        //		Checkbox_Change(pcheck);
        //		Checkbox_Enable(pcontrol);
        Checkbox_show((PCONTROL)pcheck);
        break;

    }

}
int Checkbox_init(PCHECKBOX pcheck,WORD x,WORD y,CHAR16 *name,WORD status)
{
    pcheck->controlinfo.thex=x;
    pcheck->controlinfo.they=y;
    pcheck->controlinfo.thew=GetWidthofString(name)+23;
    pcheck->controlinfo.theh=20;
    pcheck->controlinfo.status=0;
    pcheck->controlinfo.type=IDC_CHECKBOX;
    pcheck->controlinfo.thez=0;

    pcheck->controlinfo.sendmessage=Checkbox_dispatchmsg;
    pcheck->controlinfo.show=Checkbox_show;
    pcheck->status=status;

    pcheck->name=name;

    return 0;

}

////////////////////////////////////////////////////////////////////////// process box
void Processbox_show( PCONTROL pcontrol )
{
	//MyLog(LOG_DEBUG, L"Processbox_show\n");
    WORD  i;
    WORD thex=pcontrol->thex;
    WORD they=pcontrol->they;
    WORD thew=pcontrol->thew;
    PPROCESSBOX pproc=(PPROCESSBOX)pcontrol;

    hidemouse();
	
	float fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}

	FillRect(thex, they + 1, thew, pcontrol->theh - 2, COLOR_PROCESS_BK);
	Linebox(pcontrol->thex, pcontrol->they, pcontrol->thew, pcontrol->theh, COLOR_PROCESS_LINE);

	DWORD dwFrameWidth = (int)(PROGRESS_FILL_FRAME_HEAD_WIDTH * fMinscale + 0.5);// PROGRESS_FILL_FRAME_HEAD_WIDTH * fMinscale;
	WORD iDataHead = GetImageFileId(IMG_FILE_PROGRESS_FRAME_HEAD); //IMG_FILE_PROGRESS_FRAME_HEAD;
	DisplayImgEx(pcontrol->thex, pcontrol->they, dwFrameWidth, pcontrol->theh, iDataHead, FALSE);

	DWORD dwLeft = pcontrol->thex + pcontrol->thew - dwFrameWidth;
	WORD iDataFramTail = GetImageFileId(IMG_FILE_PROGRESS_FRAME_TAIL); //IMG_FILE_PROGRESS_FRAME_TAIL;
	DisplayImgEx(dwLeft, pcontrol->they, dwFrameWidth, pcontrol->theh, iDataFramTail, FALSE);

	DWORD dwFillHeight = PROGRESS_FILL_HEIGHT * fMinscale;
	DWORD dwFillTop = pcontrol->they + (pcontrol->theh - dwFillHeight) / 2;
	DWORD dwFillLeft = pcontrol->thex + (DWORD)(PROGRESS_FILL_SPAN * fMinscale);
	DWORD dwWidth = PROGRESS_FILL_HEAD_WIDTH * fMinscale;

	//MyLog(LOG_DEBUG, L"pproc->length %d\n", pproc->length);
	if (pproc->length == 0)
	{
		WORD iData = GetImageFileId(IMG_FILE_PROGRESS_HEAD); //IMG_FILE_PROGRESS_HEAD;
		DisplayImgEx(dwFillLeft, dwFillTop, dwWidth, dwFillHeight, iData, FALSE);

		dwWidth = PROGRESS_FILL_TAIL_WIDTH * fMinscale;
		WORD iDataProTail = GetImageFileId(IMG_FILE_PROGRESS_TAIL); //IMG_FILE_PROGRESS_TAIL;
		DisplayImgEx(dwFillLeft + dwWidth - 1, dwFillTop, dwWidth, dwFillHeight, iDataProTail, FALSE);
	}

	dwWidth = PROGRESS_FILL_TAIL_WIDTH * fMinscale;
	dwFillLeft = pcontrol->thex + PROGRESS_FILL_SPAN * fMinscale + dwWidth - 1;
	WORD iDataProTail = GetImageFileId(IMG_FILE_PROGRESS_TAIL); //IMG_FILE_PROGRESS_TAIL;

	for (i = 1; i < pproc->length; i++)
	{
		DisplayImgEx(dwFillLeft + i, dwFillTop, dwWidth, dwFillHeight, iDataProTail, FALSE);
		//delayEx(1);
	}

    showmouse();

    return ;
}

void Processbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage)
{
    WORD newlen, i;
    PPROCESSBOX pproc = (PPROCESSBOX)pcontrol;

	switch (dmessage->message) {
	case WM_PROCESS_RUN:
		//MyLog(LOG_DEBUG, L"WM_PROCESS_RUN %d %%\n", dmessage->thex);
		if (dmessage->thex > 100)
			break;

		float fMinscale = g_data.fMinscale;
		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			fMinscale = 0.5;
		}

		DWORD dwFillHeight = PROGRESS_FILL_HEIGHT * fMinscale;
		DWORD dwFillTop = pcontrol->they + (pcontrol->theh - dwFillHeight) / 2;
		DWORD dwFillLeft = pcontrol->thex + (DWORD)(PROGRESS_FILL_SPAN * fMinscale);
		DWORD dwFillWidth = pcontrol->thew - (DWORD)(PROGRESS_FILL_SPAN * fMinscale * 2);
		DWORD dwWidth = PROGRESS_FILL_HEAD_WIDTH * fMinscale;

		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			dwFillHeight = PROGRESS_FILL_HEIGHT * 0.5;
			dwFillTop = pcontrol->they + (pcontrol->theh - dwFillHeight) / 2;
			dwWidth = PROGRESS_FILL_HEAD_WIDTH * 0.5;
		}

		newlen = (WORD)dmessage->thex * dwFillWidth / 100;
		i = pproc->length;
		if (g_data.bDebug) {
			if (i + pcontrol->thex + 1 < LOG_GRAPHIC_X + LOG_GRAPHIC_WIDTH) {
				i = LOG_GRAPHIC_X + LOG_GRAPHIC_WIDTH - (pcontrol->thex + 1);
			}
		}
		hidemouse();
		//for (;i<newlen;i++)
		//    DisplayImg(pcontrol->thex+1+i, pcontrol->they+1,IMG_FILE_PROC,FALSE);


		//MyLog(LOG_DEBUG, L"i %d pproc->length %d newlen %d\n", i, pproc->length, newlen);
		if (i == 0)
		{
			WORD iDataProHead = GetImageFileId(IMG_FILE_PROGRESS_HEAD);
			DisplayImgEx(dwFillLeft, dwFillTop, dwWidth, dwFillHeight, iDataProHead, FALSE);
			dwWidth = PROGRESS_FILL_TAIL_WIDTH * fMinscale;
			WORD iData = GetImageFileId(IMG_FILE_PROGRESS_TAIL);

			g_ProgressPos = dwFillLeft + dwWidth - 1 + newlen;

			for (int iIndex = 1; iIndex < newlen; iIndex++)
			{
				DisplayImgEx(dwFillLeft + dwWidth - 1 + iIndex, dwFillTop, dwWidth, dwFillHeight, iData, FALSE);
			}

			//MyLog(LOG_DEBUG, L"wdPos %d\n", g_ProgressPos);
		}
		else
		{
			//PBYTE buff = NULL;
			//if (!g_data.bDebug && buff != NULL)
			//{
			//	buff = AllocatePool(400 * 400 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
			//}

			WORD iData = GetImageFileId(IMG_FILE_PROGRESS_TAIL);
			//WORD wSize = 500 * fMinscale;
			for (; i < newlen; i++)
			{
				//MyLog(LOG_ERROR, L"pcontrol->thex + i %d wdPos %d\n", pcontrol->thex + i, g_ProgressPos);
				if (pcontrol->thex + i > g_ProgressPos)
				{
					//MyLog(LOG_ERROR, L"pcontrol->thex: %d, i: %d\n", pcontrol->thex, i);
					DisplayImgEx(pcontrol->thex + i, dwFillTop, dwWidth, dwFillHeight, iData, FALSE);
					MyLog(LOG_ERROR, L".");
					if (!g_data.bDebug && g_data.dbgBackground != NULL)
					{
						//用于进度条平滑	
						//GetImgEx(0, 0, wSize, wSize, g_data.dbgBackground);
						GetImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
						//MyLog(LOG_ERROR, L".\n");
						//FillRect(0, 0, 10, 10, COLOR_DEFAULT_BACKGROUND);
						PutImgEx(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_WIDTH, LOG_GRAPHIC_HEIGHT, g_data.dbgBackground);
						//PutImgEx(0, 0, wSize, wSize, g_data.dbgBackground);
					}
				}
			}
			//if (!g_data.bDebug && buff != NULL)
			//{
			//	FreePool(buff);
			//}
		}

        pproc->length = newlen;
        showmouse();
        break;
    case WM_PROCESS_INIT:
        pproc->length=0;
        pcontrol->show(pcontrol);
        break;
    }

}

int Processbox_init(PPROCESSBOX pproc,WORD x,WORD y,WORD width, WORD height)
{
    pproc->controlinfo.thex=x;
    pproc->controlinfo.they=y;
    pproc->controlinfo.thew=width;
	float	fMinscale = g_data.fMinscale;
	if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
	{
		fMinscale = 0.5;
	}
	if ((height - (DWORD)(PROGRESS_FILL_HEIGHT * fMinscale)) % 2 != 0)
	{
		height += 1;
	}
    pproc->controlinfo.theh= height;
    pproc->controlinfo.status=CON_STATUS_TABNOTSTOP;
    pproc->controlinfo.type=IDC_PROCESSBOX;
    pproc->controlinfo.thez=0;

    pproc->controlinfo.sendmessage=Processbox_dispatchmsg;
    pproc->controlinfo.show=Processbox_show;
    pproc->length=0;
    return 0;

}


