// mouse contrl and the window 
#include "okr.h"

INT32 g_mouse_x=100;  //last position
INT32 g_mouse_y=100;
BYTE g_mouse_show = FALSE;		//��ǰ�Ƿ���ʾ�����
BYTE g_mouse_enable = TRUE;		//��ǰ����״̬���Ƿ�����ʹ��
BOOLEAN g_mouse_oldlbutton = FALSE;
BOOLEAN g_mouse_oldrbutton = FALSE;
VOID *g_mouse_buf = NULL;

EFI_KEY_DATA gKeyData = {0};	//��һ�ε���getch��õļ�ֵ
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *gpSimpleEx = NULL;

struct CURSOR_INFO Cursor_normal={IMG_FILE_MOUSEA,12,21,0,0 };
struct CURSOR_INFO Cursor_text={IMG_FILE_CURSORTEXT,9,19,4,9};
struct CURSOR_INFO Cursor_move={IMG_FILE_CURSORMOVE,19,9,9,4};
struct CURSOR_INFO Cursor_hand={IMG_FILE_CURSORHAND,17,22,5,0};

struct CURSOR_INFO *g_current_cursor=&Cursor_normal;

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

//�ָ���꾭���ĵط�
BYTE hidemouse()
{
	BYTE bMouseDisplayed = g_mouse_show;
	
	if(g_mouse_show)
	{
    	WORD x = (WORD)g_mouse_x;
    	WORD y = (WORD)g_mouse_y;
    	
    	adjustmousepos(&x,&y);
    	
    	if (g_mouse_buf!=NULL) {
    	    PutImgEx(x,y,g_current_cursor->width,g_current_cursor->height,g_mouse_buf);
    	}
    }
    	
   	//�ָ��˺����ʹ���û����ʾ��״̬
   	g_mouse_show = FALSE;
   	
   	return bMouseDisplayed;
}

//����mouseͼƬ���ڵ�����
void Savemouse()
{
    WORD x = (WORD)g_mouse_x;
    WORD y = (WORD)g_mouse_y;

    adjustmousepos(&x,&y);

    if (g_mouse_buf==NULL) {
        g_mouse_buf=AllocatePool(4*100*100);
    }
    GetImgEx(x,y,g_current_cursor->width,g_current_cursor->height,g_mouse_buf);
}

//��ʾmouse, ������ʾ���ǰ��״̬
BYTE showmouse()
{
	if(!g_mouse_show && g_mouse_enable)
	{
    	WORD x=(WORD)g_mouse_x;
    	WORD y=(WORD)g_mouse_y;
    	
    	Savemouse();
    	
    	adjustmousepos(&x,&y);
    	
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

//�������а�����ʱ����������жϺ��˳��ĺ��� bison
//������
//����������а�����ʱ�򷵻ؼ���ɨ��������״̬��
//timeout: ��ʱ����WM_TIMEOUT��Ϣ������Ϊ��λ��=0ʱ����鳬ʱ��
void detectmessage(struct DM_MESSAGE *pdmmessage,EFI_SIMPLE_POINTER_PROTOCOL *mouse) //ֱ�ӽ����handle���룬�Ķ���С��Ӱ����С
{
    WORD ret=0;
    EFI_STATUS status;
	UINTN i=0;
    //EFI_SIMPLE_POINTER_PROTOCOL *mouse=NULL;
    EFI_SIMPLE_POINTER_STATE mousestate;

    pdmmessage->message=WM_NOMESSAGE;

   // status=gBS->LocateProtocol(&gEfiSimplePointerProtocolGuid,NULL,&mouse);
    if (!EFI_ERROR(status)) {

    } else {
        //	Print(L"Can't find mouse");
    }
    
    for (i=0;i<2;i++) { //��ԭ��������ѭ����Ϊ����ѭ������������û���⣬���Ƕ����״̬�·�ֹĳһ���һֱ����ERROR״̬��������ѭ���������������û����Ӧ�����
        SetMem(&mousestate,sizeof(EFI_SIMPLE_POINTER_STATE),0);

		if(gpSimpleEx)
			status = gpSimpleEx->ReadKeyStrokeEx(gpSimpleEx, &gKeyData);
		else
			status = gST->ConIn->ReadKeyStroke(gST->ConIn, &gKeyData.Key);

        if (!EFI_ERROR(status)) {
            if (gKeyData.Key.UnicodeChar == 0) {
                if (gKeyData.Key.ScanCode < 0x80)
                    pdmmessage->message = gKeyData.Key.ScanCode+0x0080;
            } else
                pdmmessage->message = gKeyData.Key.UnicodeChar;
            break;
        }
        if (!mouse || !g_mouse_enable)
            continue;
            
        status = mouse->GetState(mouse,&mousestate);
        
        if (!EFI_ERROR(status)) {
            INT32 mouseresX,mouseresY;
            
            mouseresX=(INT32)mouse->Mode->ResolutionX;
            mouseresY=(INT32)mouse->Mode->ResolutionY;
			if(!mouseresX || !mouseresY)
				continue;
            
			//Print(L"mouseres(%d,%d), RelativeMovement(%d,%d), Button(%d,%d)\n",
			//	mouseresX, mouseresY,
			//	mousestate.RelativeMovementX,
			//	mousestate.RelativeMovementY,
			//	mousestate.LeftButton,
			//	mousestate.RightButton);
				
            if(mousestate.RelativeMovementX || mousestate.RelativeMovementY) {
            	pdmmessage->thex = g_mouse_x + mousestate.RelativeMovementX * 8 / mouseresX;
            	pdmmessage->they = g_mouse_y + mousestate.RelativeMovementY * 8 / mouseresY;
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
			}

            if (pdmmessage->thex != g_mouse_x || pdmmessage->they != g_mouse_y) {
            	
				BYTE bMouseDisplay = hidemouse();

                pdmmessage->message = WM_MOUSEMOVE;
                if (g_mouse_oldlbutton && mousestate.LeftButton)
                    pdmmessage->message = WM_MOUSEDRAG;

	            g_mouse_x = pdmmessage->thex;
    	        g_mouse_y = pdmmessage->they;
    	        
				if(bMouseDisplay)
        			showmouse();
        	    
            } else {
                if (!g_mouse_oldlbutton && mousestate.LeftButton)
                    pdmmessage->message = WM_LEFTBUTTONPRESSED;
                if (g_mouse_oldlbutton && !mousestate.LeftButton)
                    pdmmessage->message = WM_LEFTBUTTONRELEASE;

                if (!g_mouse_oldrbutton && mousestate.RightButton)
                    pdmmessage->message = WM_RIGHTBUTTONPRESSED;
                if (g_mouse_oldrbutton && !mousestate.RightButton)
                    pdmmessage->message = WM_RIGHTBUTTONRELEASE;

	            g_mouse_oldlbutton = mousestate.LeftButton;
    	        g_mouse_oldrbutton = mousestate.RightButton;
            }

            break;
        }
	}

    return ;
}

// �л���꣬�����µ�ͼ�Σ�������ͼ��,����ʾ����
PCURSOR_INFO change_cursor(PCURSOR_INFO cur)
{
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

//ֻ��ʾ�ؼ�
void Window_showctls(PWINDOW pwin)
{
    WORD i;
    struct DM_MESSAGE msg;

    for (i=0 ;i<pwin->control_num ;i++ )
    {
        if (pwin->control_list[i]->status & CON_STATUS_HIDE)
            continue;
        (pwin->control_list[i])->show((PCONTROL)pwin->control_list[i]);
        if (pwin->control_list[i]->status & CON_STATUS_DISABLE ) {//��ʾdisable�Ŀؼ�
            msg.message = WM_CTL_DISABLE;
            pwin->control_list[i]->sendmessage(pwin->control_list[i],&msg);
        }
    }
////////////////////////////////////////////////////////////////
    msg.message=WM_CTL_ACTIVATE;
    Control_sendmsg(pwin->control_list[pwin->current],&msg);
//  (pwin->control_list[pwin->current])->sendmessage(pwin->control_list[pwin->current],&msg); //���ǰ�ؼ�
    return ;
}

 
//��ʼ��window�ṹ����ָ��show����ָ�룬���=NULL,ʹ��Ĭ����ʾ����
//�û������Ҫ�Զ�����Ϣ�����ɸ���msgfunc����ָ��
int Window_init(PWINDOW pwin,void (* show)(struct WINDOW *) )
{
//	pwin->control_list=con;
    pwin->control_num=0;
    pwin->current=pwin->precurrent=0;
    pwin->current_cursor=0xffff; //��ǰ����ڱ�����
    pwin->ifexit=0;
    pwin->selected = 0;
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

//�ؼ����ô˺����������������˳�����¼����ֵret
int Window_exit(PWINDOW pwin,DWORD ret)
{
    pwin->ifexit=1;
    pwin->ret_data=ret;
    return 0;

}
//���һ���ؼ������ؿؼ�������
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
#define offsetof(TYPE, MEMBER) ((UINTN) &((TYPE *)0)->MEMBER)


//�����û����������Ҽ��󣬿ؼ������ת��
//key: �ؼ������� dir:����,���ң�����Ϊ1������Ϊ-1
//key1: ������������key1ֵ�������ѡ�񣬵�key1ֵ��ͬʱ��keyֵ����
//���ؽ���ؼ�
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

        if (key_dist<=0) //�����Ϸ���
            continue;
        key1_dist=*(INT16 *)((PBYTE )ctl[i]+key1)-*(INT16 *)((PBYTE )ctl[current]+key1);
        if (key1_dist<0)
            key1_dist*=-1; //�õ�Key1�����ϵ���Ծ���

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
// �������пؼ�����Ϣ���������ؼ��л�����λ���λ�ã�������Ϣ���ӿؼ��ȵ�
// ����ǰ��ʼ��pwin,����ʾһ��
void  Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{

    WORD precurrent,current;
    WORD controlnum=pwin->control_num;
    PCONTROL *ctl=pwin->control_list;
    struct DM_MESSAGE msg;
    WORD current_cursor;

    precurrent=pwin->precurrent;
    current=pwin->current;
//	debug(0,500,current);
    //�ж����λ��,�����λ�÷����ı�ʱ�����������Ϣ
    if (ifmousemsg(dmmessage)) {
        current_cursor=detectwhere(ctl,controlnum,dmmessage);
        if (current_cursor!=pwin->current_cursor) {
            struct DM_MESSAGE mymsg;
            mymsg.thex=dmmessage->thex;
            mymsg.they=dmmessage->they;
            if (pwin->current_cursor<pwin->control_num) { //���Ǳ���
                mymsg.message=WM_MOUSEMOVEOUT;
                //->sendmessage(ctl[pwin->current_cursor],&mymsg);
                Control_sendmsg(ctl[pwin->current_cursor],&mymsg);
            }
            if (current_cursor<pwin->control_num) { //���Ǳ���
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

            current++;         //�л�����һ���ؼ�

            if (current>=controlnum)
                current=0;
            //MyLog(LOG_DEBUG, L"win-02, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[current]->status,ctl[current]->control_id);
            if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
            {
            	 continue;
            }
            
            if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP ) && !( ctl[current]->status&CON_STATUS_DISABLE))//�˿ؼ�֧�ּ���״̬
                break;
        }
        dmmessage->message=0; //��Ϣ�Ѿ���Ӧ���ؼ�����Ҫ����tab��Ϣ
        break;
    case CHAR_ESC:	//ȡ��
    	Window_exit(pwin, IDCANCEL);
    	break;
    case WM_LEFTBUTTONPRESSED: //�л��ؼ�

        if ((current_cursor<controlnum)&&(ctl[current_cursor]->status&CON_STATUS_DISABLE) )
            break;
        current=pwin->current_cursor;
        break;
    }
    
  // if( current < controlnum &&  precurrent < controlnum)
  //   MyLog(LOG_DEBUG, L"win-02, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);


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
                current++;         //�л�����һ���ؼ�
               if (current>=controlnum)
                   current=0;
	             if(ctl[current]->control_id  != IDB_PAGE_DOWN && ctl[current]->control_id  != IDB_PAGE_UP )
                   break;
             }  	
       }
    }
    if (precurrent!=current) {   //�л��ؼ�����
        //��Щ�ؼ�û�н���״̬(CON_STATUS_NOACTIVE),����Ҫ����

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
    // ������ƶ����������Ŀؼ����������Ϣ,�������Ϣֻ�е�����ڿؼ���ʱ�Żᷢ
    if ((pwin->current_cursor!=current)&&(ifmousemsg(dmmessage)) ) {

        if ( (pwin->current_cursor!=0xffff) ) {
            //	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
            Control_sendmsg(ctl[pwin->current_cursor],dmmessage);

        }
        dmmessage->message=0;
    }

    if (current>controlnum) //��ǰû�н���
        goto exit_dispatch;
    if (dmmessage->message==0)
        goto exit_dispatch;

    // ��û�н��õĿؼ�����Ϣ
    Control_sendmsg(ctl[current],dmmessage);

    //�����������Ҽ���Ϣ,�л��ؼ�
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
    //�ж����λ��,�����λ�÷����ı�ʱ�����������Ϣ

   // if( current < controlnum &&  precurrent < controlnum)
   //  MyLog(LOG_DEBUG, L"win-01, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[precurrent]->status,ctl[precurrent]->control_id);


    if (ifmousemsg(dmmessage)) {
        current_cursor=detectwhere(ctl,controlnum,dmmessage);
        if (current_cursor!=pwin->current_cursor) {
            struct DM_MESSAGE mymsg;
            mymsg.thex=dmmessage->thex;
            mymsg.they=dmmessage->they;
            if (pwin->current_cursor<pwin->control_num) { //���Ǳ���
                mymsg.message=WM_MOUSEMOVEOUT;
                //->sendmessage(ctl[pwin->current_cursor],&mymsg);
                Control_sendmsg(ctl[pwin->current_cursor],&mymsg);
            }
            if (current_cursor<pwin->control_num) { //���Ǳ���
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

            current++;         //�л�����һ���ؼ�

            if (current>=controlnum)
                current=0;
       //     MyLog(LOG_DEBUG, L"win-02, pre %d,cur:%d,status %x,id:%x\n",precurrent,current,ctl[current]->status,ctl[current]->control_id);
            if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP)
            {
            	 continue;
            }
            
            if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP ) && !( ctl[current]->status&CON_STATUS_DISABLE))//�˿ؼ�֧�ּ���״̬
                break;
        }
        dmmessage->message=0; //��Ϣ�Ѿ���Ӧ���ؼ�����Ҫ����tab��Ϣ
        break;
    case CHAR_ESC:	//ȡ��
    	Window_exit(pwin, IDCANCEL);
    	break;
    case WM_LEFTBUTTONPRESSED: //�л��ؼ�

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
                current++;         //�л�����һ���ؼ�
               if (current>=controlnum)
                   current=0;
	             if(ctl[current]->control_id  != IDB_PAGE_DOWN && ctl[current]->control_id  != IDB_PAGE_UP )
                   break;
             }  	
       }
    }
    if (precurrent!=current) {   //�л��ؼ�����
        //��Щ�ؼ�û�н���״̬(CON_STATUS_NOACTIVE),����Ҫ����

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
    // ������ƶ����������Ŀؼ����������Ϣ,�������Ϣֻ�е�����ڿؼ���ʱ�Żᷢ
    if ((pwin->current_cursor!=current)&&(ifmousemsg(dmmessage)) ) {

        if ( (pwin->current_cursor!=0xffff) ) {
            //	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
            Control_sendmsg(ctl[pwin->current_cursor],dmmessage);

        }
        dmmessage->message=0;
    }

    if (current>controlnum) //��ǰû�н���
        goto exit_dispatch;
    if (dmmessage->message==0)
        goto exit_dispatch;

    // ��û�н��õĿؼ�����Ϣ
    Control_sendmsg(ctl[current],dmmessage);

    //�����������Ҽ���Ϣ,�л��ؼ�
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

// �������пؼ�����Ϣ���������ؼ��л�����λ���λ�ã�������Ϣ���ӿؼ��ȵ�
// ����ǰ��ʼ��pwin,����ʾһ��
void  SelectDisk_Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage)
{

    WORD precurrent,current;
    WORD controlnum=pwin->control_num;
    PCONTROL *ctl=pwin->control_list;
    struct DM_MESSAGE msg;
    WORD current_cursor;
    WORD prev_control_type;
    static int prev_tab_current =-1,tab_current = -1;

    precurrent=pwin->precurrent;
    current=pwin->current;

  //	 MyLog(LOG_DEBUG, L"Drvbox-00, precurrent %d,current:%d,%x\n",precurrent,current,dmmessage->message);

//	debug(0,500,current);
    //�ж����λ��,�����λ�÷����ı�ʱ�����������Ϣ
    if (ifmousemsg(dmmessage)) {
        current_cursor=detectwhere(ctl,controlnum,dmmessage);
        if (current_cursor!=pwin->current_cursor) {
            struct DM_MESSAGE mymsg;
            mymsg.thex=dmmessage->thex;
            mymsg.they=dmmessage->they;
            if (pwin->current_cursor<pwin->control_num) { //���Ǳ���
                mymsg.message=WM_MOUSEMOVEOUT;
                //->sendmessage(ctl[pwin->current_cursor],&mymsg);
                Control_sendmsg(ctl[pwin->current_cursor],&mymsg);
            }
            if (current_cursor<pwin->control_num) { //���Ǳ���
                mymsg.message=WM_MOUSEMOVEIN;
                //ctl[current_cursor] ->sendmessage(ctl[current_cursor],&mymsg);
                Control_sendmsg(ctl[current_cursor],&mymsg);

            }
            pwin->current_cursor=current_cursor;
        }
    }
  
  //MyLog(LOG_DEBUG, L"Drvbox-01, precurrent %d,current:%d,%x\n",precurrent,current,dmmessage->message);
    

    
    switch (dmmessage->message) 
    {
    	  case CHAR_TAB:
 //         MyLog(LOG_DEBUG, L"Drvbox-0, precurrent %d,current:%d\n",precurrent,current);
    		  while (1)
    		  {
               current++;         //�л�����һ���ؼ�            
              if (current>=controlnum)
                  current=0;
              if (!(ctl[current]->status&CON_STATUS_TABNOTSTOP ) && !( ctl[current]->status&CON_STATUS_DISABLE))//�˿ؼ�֧�ּ���״̬
                  break;
         }
         //dmmessage->message=0; //��Ϣ�Ѿ���Ӧ���ؼ�����Ҫ����tab��Ϣ
 //        MyLog(LOG_DEBUG, L"Drvbox-2 prev_tab_current:%d,current:%d\n",prev_tab_current,current);
         break;
    case CHAR_RETURN:
    	   Window_exit(pwin, IDB_BACKUP);
    	   break;
    case CHAR_ESC:	//ȡ��
    	   Window_exit(pwin, IDCANCEL);
        break;
    case WM_LEFTBUTTONPRESSED: //�л��ؼ�
     
 //       MyLog(LOG_DEBUG, L"Drvbox-02, current_cursor %x\n",current_cursor);

        if ((current_cursor<controlnum)&&(ctl[current_cursor]->status&CON_STATUS_DISABLE) )
            break;
        current=pwin->current_cursor;
        break;
    }
    
  //   MyLog(LOG_DEBUG, L"Drvbox-02, precurrent %d,current:%d\n",precurrent,current);
     if( current < controlnum  && precurrent < controlnum)
     { 
             if(ctl[current]->control_id  == IDB_BACKUP && dmmessage->message == WM_LEFTBUTTONPRESSED )
            {
  //    	    	MyLog(LOG_DEBUG, L"Drvbox-5, precurrent %d,current:%d,%d\n",precurrent,current,pwin->current_cursor);

            	 msg.message=CHAR_RETURN;
               Control_sendmsg(ctl[current],&msg);
            	 dmmessage->message=CHAR_RETURN;
            	 goto exit_dispatch;
            }
    }          
    if( current < controlnum  && precurrent < controlnum)
   	{
   		
   	
    if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP ) 
    {
    		if(ctl[precurrent]->control_id  == IDB_BUTTON_SHOW )
    		{   
		 	     Control_sendmsg(ctl[current],dmmessage);
                                                                                     
    			 if( ctl[current]->control_id == IDB_PAGE_UP )
    			 {                                                                                             

    			 	   if(precurrent == 0)
    			 	   {                                                                                             
    			 	   	  current = 0;
    			 	   }                                                                                        
    			 	   else                                                                                      
    			 	   {
    			 	   	 current = precurrent-1;                                                              
    			 	   }
		 	   		//	 MyLog(LOG_DEBUG, L"Drvbox-03, up precurrent %d,current:%d\n",precurrent,current);        
     
   			 	   //	 precurrent = current;
    			 	   dmmessage->message=WM_LEFTBUTTONPRESSED;
   			 	     
   			 	     Control_sendmsg(ctl[current],dmmessage);
	             dmmessage->message=0;
               precurrent = current;
               pwin->selected = current;

   			 	     goto exit_dispatch;
    			 }
    			 else  if( ctl[current]->control_id == IDB_PAGE_DOWN )
    			 {        
    			 	   int show_button_num = 0, i = 0;
    			 	                                                                                        
               for(i=0;i<controlnum;i++)
               {
                   if( ctl[i]->control_id  == IDB_BUTTON_SHOW )
                   {
                   	  show_button_num++;
                   }
               }
    			 	   
    			 	   current = precurrent + 1;
  		 	   		
  		 	   	//	 MyLog(LOG_DEBUG, L"Drvbox-03-0, down precurrent %d,current:%d,%d\n",precurrent,current,show_button_num);        
  			 	  
    			 	   if( current >= show_button_num )
    			 	   {                                                                                             
    			 	   	  current = current - 1;
    			 	   }                    
    			 	                                                                       
               precurrent = current;
		 	   	//		 MyLog(LOG_DEBUG, L"Drvbox-03-1, down precurrent %d,current:%d\n",precurrent,current);        
     
    			 	   dmmessage->message=WM_LEFTBUTTONPRESSED;
   			 	     Control_sendmsg(ctl[current],dmmessage);
	             dmmessage->message=0;
              
               pwin->selected = current;

   			 	     goto exit_dispatch;
    			 }

    		}

    }
    }
    else if( current < controlnum && precurrent == 0xffff )
    {
    	    
        		if( ctl[current]->control_id == IDB_PAGE_UP )
    			  {       
    			  	 current = pwin->selected;
                                                                                      
    			 	   if(current == 0)
    			 	   {                                                                                             
    			 	   }                                                                                        
    			 	   else                                                                                      
    			 	   {
    			 	   	 current = current-1;                                                              
    			 	   }
		 	   		//	 MyLog(LOG_DEBUG, L"Drvbox-03, up precurrent %d,current:%d\n",precurrent,current);        
     
   			 	   //	 precurrent = current;
    			 	   dmmessage->message=WM_LEFTBUTTONPRESSED;
   			 	     Control_sendmsg(ctl[current],dmmessage);
	             dmmessage->message=0;
               pwin->selected = precurrent = current;

   			 	     goto exit_dispatch;
    			 }
    			 else  if( ctl[current]->control_id == IDB_PAGE_DOWN )
    			 {        
    			 	   int show_button_num = 0, i = 0;
    			 	                                                                                        
               for(i=0;i<controlnum;i++)
               {
                   if( ctl[i]->control_id  == IDB_BUTTON_SHOW )
                   {
                   	  show_button_num++;
                   }
               }
    			 	   current = pwin->selected;
    			 	   
    			 	   current = current + 1;
  		 	   		
  		 	   	//	 MyLog(LOG_DEBUG, L"Drvbox-03-0, down precurrent %d,current:%d,%d\n",precurrent,current,show_button_num);        
  			 	  
    			 	   if( current >= show_button_num )
    			 	   {                                                                                             
    			 	   	  current = current - 1;
    			 	   }                    
    			 	                                                                       
               pwin->selected = precurrent = current;
		 	   		//	 MyLog(LOG_DEBUG, L"Drvbox-03-1, down precurrent %d,current:%d\n",precurrent,current);        

    			 	   dmmessage->message=WM_LEFTBUTTONPRESSED;
   			 	     Control_sendmsg(ctl[current],dmmessage);
	             dmmessage->message=0;

   			 	     goto exit_dispatch;
    			 }
	    	
    	    

    }
    else if( precurrent < controlnum && current == 0xffff )
    {
    	
    }      
switch_control:
		 
		// MyLog(LOG_DEBUG, L"Drvbox-1, precurrent %d,current:%d,mes %x\n",precurrent,current,dmmessage->message);
      if( dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN )
      {
      	if( current < controlnum  && precurrent ==current )
      	{
      		     if ( ctl[current]->control_id == IDB_EDIT_BTN )
      		     {	  
      		  	    dmmessage->message=0;
              
   			 	        goto exit_dispatch;
   			 	     }
      	}
      }
      if( dmmessage->message == CHAR_LEFT || dmmessage->message == CHAR_RIGHT )
      {
      	if( current < controlnum  && precurrent < controlnum  && ctl[current]->control_id != IDB_EDIT_BTN )
      	{
      		  	 dmmessage->message=0;
              
               pwin->selected = current;

   			 	     goto exit_dispatch;
      	}
      }
	    if( dmmessage->message == CHAR_TAB && current < controlnum  && precurrent < controlnum )  
      {	 	
       	if(ctl[precurrent]->control_id == IDB_BUTTON_SHOW || ctl[precurrent]->control_id == IDB_BUTTON_SHOW )
        {
          while (1) {
                     //�л�����һ���ؼ�
            if (current>=controlnum)
                current=0;
	          if(ctl[current]->control_id  != IDB_BUTTON_SHOW )
                break;
            current++;
          }  	

        }
      }
	    if( dmmessage->message == CHAR_RIGHT && current < controlnum  && precurrent < controlnum)  
      {	 	
       	if(ctl[precurrent]->control_id == IDB_BUTTON_SHOW || ctl[current]->control_id == IDB_BUTTON_SHOW )
        {
          while (1) {
                     //�л�����һ���ؼ�
            current++;
            if (current>=controlnum)
                current=0;
	          if(ctl[current]->control_id  != IDB_BUTTON_SHOW && ctl[current]->control_id  != IDB_PAGE_DOWN && ctl[current]->control_id != IDB_PAGE_UP)
                break;
            
          }  	

        }
      }
 
 	    if(  current < controlnum  && precurrent < controlnum)  
      {	 	
 	//	  	MyLog(LOG_DEBUG, L"Drvbox-2, pre id %x,cur id:%x\n",ctl[precurrent]->control_id,ctl[current]->control_id);

       	if(ctl[precurrent]->control_id == IDB_BACKUP && ctl[current]->control_id == IDB_BUTTON_SHOW && dmmessage->message !=  WM_LEFTBUTTONPRESSED)
        {
           current = precurrent;
           dmmessage->message = 0;
        }
      }
          
	//  	MyLog(LOG_DEBUG, L"Drvbox-2, precurrent %d,current:%d\n",precurrent,current);
      
      if( current < controlnum  && precurrent < controlnum  )
      {
          if(ctl[current]->control_id == IDB_PAGE_DOWN || ctl[current]->control_id == IDB_PAGE_UP )
          {
              while (1)
              {
           
                if (current>=controlnum)
                    current=0;
	              if(ctl[current]->control_id  != IDB_PAGE_DOWN && ctl[current]->control_id  != IDB_PAGE_UP )
                    break;
                 current++;         //�л�����һ���ؼ�
              }  	
          }
      }
   //   if(current < controlnum  && precurrent < controlnum  )
	//     	MyLog(LOG_DEBUG, L"Drvbox-3, pre %d,cur:%d,%d,mes:%x,%x\n",precurrent,current,pwin->current_cursor,dmmessage->message,ctl[current]->control_id);
/*    
    if(dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN)
   	{
    	    
   	    if (precurrent<controlnum  )
   	    {
  		      dmmessage->message=0;
   	        msg.message = WM_CTL_INACTIVATE;
   	    		
            Control_sendmsg(ctl[precurrent],&msg);
        }
   		  dmmessage->message=0;
   	    msg.message = WM_CTL_ACTIVATE;
   	    
   	    if (current<controlnum  )
        {
        	   Control_sendmsg(ctl[current],&msg);
        
        }
        goto exit_dispatch;
        
   	}
*/
    if (precurrent!=current) {   //�л��ؼ�����
        //��Щ�ؼ�û�н���״̬(CON_STATUS_NOACTIVE),����Ҫ����

        if  ((precurrent<controlnum)  ) {
            msg.message=WM_CTL_INACTIVATE;
            //ctl[precurrent]->sendmessage(ctl[precurrent],&msg);
            if(ctl[precurrent]->control_id == IDB_BUTTON_SHOW)   
            {
            	  if(dmmessage->message == CHAR_UP || dmmessage->message == CHAR_DOWN)
            	  {
            	  	  dmmessage->message=0;
   	                msg.message = WM_CTL_INACTIVATE;
              	    Control_sendmsg(ctl[precurrent],&msg);
            	  }
            	  else  if (current<controlnum)
            	  {
            	  	if ( ctl[current]->control_id == IDB_BUTTON_SHOW )
            	  	{
            	  		
            	  	}
            	  	else
            	  	{
              	    if( current<controlnum && ctl[current]->control_id == IDCANCEL)
            	  		{         	  		  
            	  		}
            	  		else 
            	  		{
            	  			 dmmessage->message=0;
                 	  } 
   	                msg.message = WM_BTN_SELCHANGE;
              	    Control_sendmsg(ctl[precurrent],&msg);           	  		
            	  	}	
            	  }	
            	  else   
            	  {
              	    if( current<controlnum && ctl[current]->control_id == IDCANCEL)
            	  		{         	  		  
            	  		}
            	  		else 
            	  		{
            	  				 dmmessage->message=0;
            	  		}	 
   	                msg.message = WM_BTN_SELCHANGE;
              	    Control_sendmsg(ctl[precurrent],&msg);            	  	
            	  }
  

            }
            else
              	Control_sendmsg(ctl[precurrent],&msg);
  

        }
        if ((current<controlnum)  ) {
            msg.message=WM_CTL_ACTIVATE;
            //ctl[current]->sendmessage(ctl[current],&msg);	     	       
            Control_sendmsg(ctl[current],&msg);
	     	            
            if( precurrent<controlnum)
            {
            	 if	(ctl[precurrent]->control_id  == IDB_BUTTON_SHOW )
               {
                	pwin->selected = current;
               }
            }
            if(ctl[current]->control_id  == IDB_BUTTON_SHOW )
            {
                	pwin->selected = current;
                	
                	if(1)
                	{
                		  dmmessage->message=0;
  	                  msg.message=WM_CTL_INACTIVATE;
              	      Control_sendmsg(ctl[current],&msg);  
  	                  msg.message=WM_CTL_ACTIVATE;
              	      Control_sendmsg(ctl[current],&msg);  
                	}
            }
               
          //  if( precurrent < controlnum  )
          //     Control_enable(ctl[precurrent]);
           /* 
            if(ctl[current]->control_id == IDB_BUTTON_SHOW)   
            {
              	dmmessage->message=0;
   	            //Control_disable(ctl[precurrent]);
   	            msg.message = ' ';
              	Control_sendmsg(ctl[precurrent],&msg);

            }
            else
              	Control_sendmsg(ctl[current],&msg);
          */
        }
        precurrent=current;
    }
    // ������ƶ����������Ŀؼ����������Ϣ,�������Ϣֻ�е�����ڿؼ���ʱ�Żᷢ
    if ((pwin->current_cursor!=current)&&(ifmousemsg(dmmessage)) ) {

        if ( (pwin->current_cursor!=0xffff) ) {
            //	ctl[pwin->current_cursor]->sendmessage(ctl[pwin->current_cursor],dmmessage);
     	  //  	MyLog(LOG_DEBUG, L"Drvbox-8, precurrent %d,current:%d,%d\n",precurrent,current,pwin->current_cursor);
     	    	
            Control_sendmsg(ctl[pwin->current_cursor],dmmessage);
     	    

      
            if(ctl[pwin->current_cursor]->control_id  == IDB_BUTTON_SHOW )
            {
            	pwin->selected = current;
              	      
            }
        }
        dmmessage->message=0;
    }

    if (current>controlnum) //��ǰû�н���
        goto exit_dispatch;
    if (dmmessage->message==0)
        goto exit_dispatch;

   //   if(current < controlnum  && precurrent < controlnum  )
	 //    	MyLog(LOG_DEBUG, L"Drvbox-9, pre %d,cur:%d,%d,mes:%x,%x\n",precurrent,current,pwin->current_cursor,dmmessage->message,ctl[current]->control_id);
    // ��û�н��õĿؼ�����Ϣ
    Control_sendmsg(ctl[current],dmmessage);

    //�����������Ҽ���Ϣ,�л��ؼ�
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
        current=_window_control_move(pwin,offsetof(CONTROL,thex),offsetof(CONTROL,they),-1);
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
DWORD Window_run(PWINDOW pwin)
{
	
	UINT32 Index;
	EFI_HANDLE                            *HandleArray=NULL;
    UINTN                                 HandleArrayCount=0;
	EFI_SIMPLE_POINTER_PROTOCOL *TempPoint=NULL;
	EFI_STATUS Status = EFI_SUCCESS;
	Index = 0;
	
	
	struct DM_MESSAGE dmmessage;
		  
    change_cursor(&Cursor_normal);
    pwin->show(pwin);

	g_mouse_oldlbutton = FALSE;	//��Щ����������
	g_mouse_oldrbutton = FALSE;

    while (1) {

		//ö���������handle������Ķ���С
			   Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  &HandleArrayCount,
                  &HandleArray
                  );
	  if (!EFI_ERROR (Status))
	  {
		for (Index = 0; Index < HandleArrayCount; Index++)
			{
				Status = gBS->HandleProtocol (
                      HandleArray[Index],
                      &gEfiSimplePointerProtocolGuid,
                      (VOID **)&TempPoint
                      );
				if (!EFI_ERROR (Status))
				{
					
				detectmessage(&dmmessage,TempPoint);//ֱ�Ӵ������handle������Ķ���С
				//MyLog(LOG_DEBUG, L"message %x, l %d, r %d, x %x, y %x\n",
				//	dmmessage.message, g_mouse_oldlbutton, g_mouse_oldrbutton, g_mouse_x, g_mouse_y);
				pwin->msgfunc(pwin,&dmmessage);
				
  
				}
			}
			if(HandleArray)
			{
				FreePool(HandleArray);
				HandleArray=NULL;
			}
	  }
	  else //û�������������ֹ���̲��ܲ���
	  {
		  detectmessage(&dmmessage,NULL);//ֱ�Ӵ������handle������Ķ���С
				//MyLog(LOG_DEBUG, L"message %x, l %d, r %d, x %x, y %x\n",
				//	dmmessage.message, g_mouse_oldlbutton, g_mouse_oldrbutton, g_mouse_x, g_mouse_y);
				pwin->msgfunc(pwin,&dmmessage);
	  }
	  
	  if (pwin->ifexit!=0)
				break;

    }
    return pwin->ret_data;
}

//�ж��������������һ����Ӧ,����и��ǣ�thez�������ȣ�
//��������Ӧ�����򷵻�0xffff
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
    return ret;
}



int ifmousemsg(struct DM_MESSAGE *pdmmessage)
{
    if (pdmmessage->message > 0xfeff)
        return YES;
    return NO;
}

int ifkeymsg(struct DM_MESSAGE *pdmmessage)
{
    if (pdmmessage->message<  0xff)
        return YES;
    return NO;
}


///////////////////////////////combobox begin///////////////////////////
//item �50��ANSII�ַ�
//ÿ���ַ����14
//�༭��߶�20
//������ť�ĸ߶�16

//��ʾ�߿���ĵ�ǰѡ���ַ���
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
    FillRect(x+2,y+2,pcombo->controlinfo.thew-22,17,backcolor);//��ʾ�༭��
    if (pcombo->whichselect != 0XFFFF)                               //��ʾ��ǰ��ѡ��
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
//��ʾ״̬Ϊδ������listboxδչ����combobox�ĺ���
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

    Linebox(x,y,w,h,COMBO_COLOR_FRAME); 				   //��ʾ���ɫ
    DisplayImg(x+w-18,y+2,IMG_FILE_DROPDOWNA,FALSE); //��ʾ������ť

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
    if (pwin->current==0xffff) //������ʧȥ���㣬�˳�
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
    if (lines>5)//��Ҫ������
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
    //�����ڴ�
    imgbuff=AllocatePool(0x200000);
//	handle=Mallocmem(&(POINTER)imgbuff,NULL,"img",CommonMEM,0x10000);
    //���汳��
    hidemouse();

    DisplayImg(thex+thew-18,pcombo->controlinfo.they+2,IMG_FILE_DROPDOWNB,FALSE); //��ʾ������ť
//	GetImg(thex,they,thew,theh+1,imgbuff);
    GetImgEx(thex,they,thew,theh+1,imgbuff);
    showmouse();
    Window_run(&win);
    pcombo->whichselect=listbox.firstitem_inwindow+listbox.whichselect;

    //�ָ�����
    hidemouse();

    PutImgEx(thex,they,thew,theh+1,imgbuff);
    DisplayImg(thex+thew-18,pcombo->controlinfo.they+2,IMG_FILE_DROPDOWNA,FALSE); //��ʾ������ť

    showmouse();
    //�ͷ��ڴ�
    FreePool(imgbuff);
    Combobox_show((PCONTROL)pcombo);
    Combobox_showstr(pcombo,1);
    Listbox_destroy( &listbox);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//combobox����Ϣ���ɺ���
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


//currentsel=��ǰѡ���������1Ϊ��
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
    if (!pbnctrl->btn_unfocuspcx)//�����ְ�ť
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

void  Button_dispatchmsg(struct CONTROL * pbuttonctrl,struct DM_MESSAGE *dmessage)//click,�жϷ�Χ
{
    struct BUTTON * pbnctrl = (struct BUTTON *)pbuttonctrl;
	BOOL bRedraw = TRUE;

    switch (dmessage->message) {

    case WM_LEFTBUTTONPRESSED: //��Ӧ�����Ϣ

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
        dmessage->message = WM_BUTTON_CLICK;// ת�����������Ϣ
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

	case WM_MOUSEMOVEIN: //�������
	case WM_MOUSEMOVEOUT: //����Ƴ�
	default:
		bRedraw = FALSE;
		break;
    }

	if(bRedraw)
		Button_show(&pbnctrl->controlinfo);

    return ;
}

void btn_default(PCONTROL pcon,struct DM_MESSAGE* msg)
{
	Button_dispatchmsg(pcon,msg); // Ĭ�ϴ�����

	if (msg->message == WM_BUTTON_CLICK) {
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
    pbnctrl->controlinfo.show = Button_show;
    if(!unfocuspcx && title)
    {
    	pbnctrl->btn_unfocuspcx = IMG_FILE_BTN_NORMAL;
    	pbnctrl->btn_focuspcx = IMG_FILE_BTN_FOCUS;
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
    if (plistbox->item_num==0)
        return ;
    plistbox->item_num=0;
    plistbox->whichselect=0;
    plistbox->firstitem_inwindow=0;
    plistbox->buffer=plistbox->itemlist[0].item_buff;
}


WORD  Listbox_Addoneline(PLISTBOX plistbox)
{

    WORD item_index=plistbox->item_num;

    plistbox->itemlist[item_index].item_buff=plistbox->buffer;
    plistbox->itemlist[item_index].columns=0;
    return (++plistbox->item_num);
}

WORD  Listbox_Addoneunit(PLISTBOX plistbox,WORD type,VOID *buff)
{
    WORD len = 0;
//	char buff1[4096];
    WORD  *curr_buffer=(WORD  *)plistbox->buffer;
    ASSERT(plistbox->item_num!=0);

    if (type&ITEM_TYPE_ISCONTROL)
        len=sizeof(PBYTE);
	else if (type&ITEM_TYPE_IMG)
		len=(WORD)StrLength(buff)*2+2;
    else if (type&ITEM_TYPE_CHAR_STRING) {
        len=(WORD)StrLength(buff)*2+2;
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
//�����ʾ100�ַ�
void PutstrinWidth(WORD x,WORD y,CHAR16 *str,DWORD forecolor,WORD width)
{

    CHAR16 buff[200] = { 0 };
    DWORD i;
    DWORD len=(DWORD)StrLength(str);
    DWORD maxchars=width/PIXEL_PER_CHAR;

    if (len<=maxchars) {
        DisplayString(x,y,forecolor,str);
        return;
    }
    CopyMem(buff,str,200);
    buff[maxchars]=0;
    for (i=(maxchars-3);i<(maxchars);i++) {
        if (i<0)
            continue;
        buff[i]=0x002e;
    }
    //fix ����
    i=0;
    while (i<maxchars) {
        if (buff[i]<0x80)
            i++;
        else {
            if (buff[i+1]<0x80) {
                buff[i]=0x20;
            }
            i+=2;
        }
    }

    DisplayString(x,y,forecolor,buff);
    return;

}
// �õ��ַ�����ӡ��Ҫ������أ�
WORD GetWidthofString(CHAR16 *str)
{
    return (WORD)PIXEL_PER_CHAR*(WORD)StrLength(str);//12*12����
}

int Listbox_showoneitem(PLISTBOX plistbox ,WORD item_index,WORD current_select,DWORD backcolor,DWORD forecolor)
{
    BYTE buff[200] = { 0 };
    WORD *str;
    WORD j;
    WORD x,y;
    WORD width;
    if (item_index>=plistbox->item_num)
        return 0;
    x=plistbox->controlinfo.thex;
    y=current_select*plistbox->height_peritem+plistbox->controlinfo.they;

    //Linebox(x,y,plistbox->controlinfo.thew,plistbox->height_peritem,COLOR_BLACK); //DEBUG
    CopyMem(&buff,plistbox->itemlist[item_index].item_buff,200);
//	if (backcolor!=COLOR_WHITE)
    FillRect(x+1,y+1,plistbox->controlinfo.thew-2,plistbox->height_peritem-2,backcolor);
    str=(WORD *)buff;

    for (j=0;j<plistbox->itemlist[item_index].columns;j++) { //show item 's cols one by one
        if ((plistbox->ptitle==NULL) ) {
            width=plistbox->controlinfo.thew;
        } else {
            width=plistbox->ptitle->column_list[j].curr_width;
            x=plistbox->ptitle->column_list[j].begin_x;
        }
        if (str[0]&ITEM_TYPE_NOCOLUMN) { //�����У�����title���ƣ���ɫ����
            width=plistbox->controlinfo.thew;
            forecolor=COLOR_LISTBOX_SPECIAL;
        }
        if (str[0]&ITEM_TYPE_CHAR_STRING) {
            PutstrinWidth(x+4,y+8,(CHAR16 *)((PBYTE)str+4),forecolor, width-4);
          //  PutstrinWidth(x+4,y+8,(CHAR16 *)((PBYTE)str+4),COLOR_WHITE, width-4);
        } else if (str[0]&ITEM_TYPE_ISCONTROL) {
            struct CONTROL *p=(struct CONTROL *)((PBYTE)str+4);
            p->show(p);
        }
        if (str[0]&ITEM_TYPE_NOCOLUMN)
            break;
        x+=GetWidthofString((CHAR16 *)((PBYTE)str+4))+3; //��û�б����ʱ
        str=(WORD *)(str[1]+4+(PBYTE)str);

    }
    return 0;

}

// ������ʾȫ����
void Listbox_showitems(PLISTBOX plistbox ,int focus )
{
    int width=plistbox->controlinfo.thew;
    WORD i;
    DWORD forecolor,backcolor;
    WORD item=plistbox->firstitem_inwindow;

    if (focus==YES) {
       backcolor=LISTBOX_SELLINE_COLOR;
       forecolor=COLOR_WHITE;
	//  	backcolor = COLOR_CLIENT;
	//	forecolor = COLOR_LIST_ACTIVATE;
	//	forecolor = COLOR_LIST_INACTIVATE;
    } else {
        //backcolor=LISTBOX_SELLINE_COLOR_INAC;
        //forecolor=COLOR_BLACK;
		backcolor = COLOR_CLIENT;
		forecolor = COLOR_WHITE;
    }

    hidemouse();
    //background
    //FillRect(plistbox->controlinfo.thex+1,plistbox->controlinfo.they+1,plistbox->controlinfo.thew-1,plistbox->controlinfo.theh-2,COLOR_WHITE);
   // FillRect(plistbox->controlinfo.thex+1,plistbox->controlinfo.they+1,plistbox->controlinfo.thew-1,plistbox->controlinfo.theh-2,COLOR_CLIENT);
    
    if (plistbox->item_num!=0) {
        for (i=0;i<plistbox->items_per_win;i++) {
            if (i==plistbox->whichselect)
				plistbox->showoneitem( plistbox, item,i,backcolor,forecolor);
            else
				plistbox->showoneitem( plistbox, item,i,COLOR_CLIENT, COLOR_SEC_BLUE);//COLOR_WHITE,COLOR_BLACK

            item++;
            if (item>=plistbox->item_num)
                break;
        }
    }
    showmouse();
    return ;
}

// ��ʾ�б����棬����δ����״̬��
void Listbox_show(struct CONTROL * pcontrol  )
{
    PLISTBOX plistbox=(PLISTBOX)pcontrol;

    if (pcontrol->type!=IDC_LISTBOX)
        ASSERT(0);

    hidemouse();
    //Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh-1,0xD7D5D6 );
    Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh, COLOR_SHADOW_GREY);
    showmouse();

    Listbox_showitems(plistbox,NO);

    return ;
}
void Listbox_activate(struct CONTROL * pcontrol  )
{
    PLISTBOX plistbox=(PLISTBOX)pcontrol;

    hidemouse();
	plistbox->showoneitem( plistbox, plistbox->whichselect+plistbox->firstitem_inwindow,plistbox->whichselect ,LISTBOX_SELLINE_COLOR,COLOR_WHITE);
    showmouse();
    return;

}

void Listbox_inactivate(struct CONTROL * pcontrol  )
{
    PLISTBOX plistbox=(PLISTBOX)pcontrol;
    hidemouse();
	plistbox->showoneitem( plistbox, plistbox->whichselect+plistbox->firstitem_inwindow,plistbox->whichselect ,LISTBOX_SELLINE_COLOR_INAC,COLOR_BLACK);
    showmouse();
    return ;

}
void Changeline(PLISTBOX plistbox,WORD newline)
{

    hidemouse();
	  plistbox->showoneitem( plistbox, plistbox->whichselect+plistbox->firstitem_inwindow,
		plistbox->whichselect ,COLOR_CLIENT, COLOR_SEC_BLUE);	//COLOR_WHITE,COLOR_BLACK);
   // plistbox->showoneitem( plistbox, plistbox->whichselect+plistbox->firstitem_inwindow,plistbox->whichselect ,COLOR_WHITE,COLOR_BLACK);
    plistbox->whichselect=newline;
		plistbox->showoneitem( plistbox, plistbox->whichselect+plistbox->firstitem_inwindow,plistbox->whichselect ,LISTBOX_SELLINE_COLOR,COLOR_WHITE);
    showmouse();
}
//���б������ݷ����仯ʱ����Ҫ���ô˺���������ʾ
void Listbox_updateview(PLISTBOX plistbox)
{
    Listbox_showitems(plistbox,YES);
    //	MyLog(LOG_DEBUG,L"after showitem");
    //	getch();
    if (plistbox->pscroll!=NULL) { //���¹�����
        struct DM_MESSAGE msg;
        msg.thex=plistbox->firstitem_inwindow;
        msg.they=plistbox->item_num;
        msg.message=WM_SCROLLBOX_UPDATE;
        //	((PCONTROL)(plistbox->pscroll))->sendmessage((PCONTROL)plistbox->pscroll,&msg);
//debug(100,100,WM_SCROLLBOX_UPDATE);
//getch();
        Control_sendmsg((PCONTROL)plistbox->pscroll,&msg);
    }
    //		MyLog(LOG_DEBUG,L"after update pscroll");
    //getch();
}
int Listbox_DispatchMsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmessage)
{
    PLISTBOX plistbox=(PLISTBOX)pcontrol;
    int ifneedupdate=0;
    //debug(0,0,dmessage->message);
    if (plistbox->item_num==0)
        return 0;
    switch (dmessage->message) {
    case CHAR_UP:
        if (plistbox->whichselect==0) {
            if ( plistbox->firstitem_inwindow==0 )
            {
  	            	dmessage->message=WM_LISTBOX_SELCHANGE;
            	    break;
            }
            plistbox->firstitem_inwindow--;
            ifneedupdate=1;
        } else {
            Changeline(plistbox,plistbox->whichselect-1);
        }
			dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        break;
    case CHAR_DOWN:
        if (plistbox->whichselect==(plistbox->item_num-1) )//ֻ��һ��,���Ѿ������
        {
            	dmessage->message=WM_LISTBOX_SELCHANGE;
        	    break;
        }
        if (plistbox->whichselect==(plistbox->items_per_win-1))
		{
			if ((plistbox->firstitem_inwindow + plistbox->items_per_win) == plistbox->item_num)
			{
				dmessage->message = WM_LISTBOX_SELCHANGE;
				break;
			}
			plistbox->firstitem_inwindow++;
            ifneedupdate=1;
        } else {

            Changeline(plistbox,plistbox->whichselect+1);
        }
			dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        break;
    case CHAR_PGUP:

        if ((plistbox->firstitem_inwindow==0)||(plistbox->item_num<=plistbox->items_per_win))
            break;
        if (plistbox->firstitem_inwindow>=plistbox->items_per_win)
            plistbox->firstitem_inwindow-=plistbox->items_per_win;
        else
            plistbox->firstitem_inwindow=0;

        ifneedupdate=1;
			dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ

        break;
    case CHAR_PGDOWN:
    {
        WORD nextpg_item=plistbox->firstitem_inwindow+plistbox->items_per_win;
        if (( nextpg_item==plistbox->item_num ) || (plistbox->item_num<=plistbox->items_per_win))
            break;
        if ((plistbox->item_num-nextpg_item)>plistbox->items_per_win)//��һ������ʾ
            plistbox->firstitem_inwindow=nextpg_item;
        else
            plistbox->firstitem_inwindow=plistbox->item_num-plistbox->items_per_win;
        ifneedupdate=1;
			dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        break;
    }
    case WM_LEFTBUTTONPRESSED:
    {
        WORD clickitem=((WORD)dmessage->they-pcontrol->they)/plistbox->height_peritem;
        if (clickitem>=plistbox->item_num) //��ǰ��û������
            break;
        Changeline(plistbox,clickitem);
		dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        break;
    }
    case WM_CTL_ACTIVATE:

        Listbox_activate((PCONTROL)plistbox);
        break;
    case WM_CTL_INACTIVATE:
        Listbox_inactivate((PCONTROL)plistbox);
        break;
    case WM_SCROLL_UP: //���������ƶ�һ��
        if (plistbox->firstitem_inwindow!=0) {
            plistbox->firstitem_inwindow--;
            ifneedupdate=1;
				dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        }
        break;
    case WM_SCROLL_DOWN: //���������ƶ�һ��
        if ((plistbox->firstitem_inwindow+plistbox->items_per_win)<plistbox->item_num) {
            plistbox->firstitem_inwindow++;
            ifneedupdate=1;
				dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        }
        break;
    case WM_SCROLL_MOVE: //�϶�������
        if (plistbox->firstitem_inwindow!=dmessage->thex) {
            plistbox->firstitem_inwindow=(WORD)dmessage->thex;
            Listbox_showitems(plistbox,YES);
				dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        }
        break;
    case ' ':
        //	case WM_DOUBLECLICKED:
    case CHAR_RETURN:
			dmessage->message=WM_LISTBOX_SELCHANGE;// ɾ���Ѿ��������Ϣ
        dmessage->thex=plistbox->firstitem_inwindow+plistbox->whichselect;
        break;
    }

    if (ifneedupdate==1) {
        Listbox_updateview(plistbox);
    }
    return 0;
}

//�������ڴκ���������֮�����á�
int Listbox_init(PLISTBOX plistbox,WORD x,WORD y,WORD width,WORD hight,WORD items_per_win,DWORD buffer_size)
{
    plistbox->item_num=0;
    plistbox->whichselect=0;
    plistbox->firstitem_inwindow=0;
    plistbox->buffer = AllocatePool(buffer_size);
    plistbox->orgbuffer = plistbox->buffer;
    plistbox->pscroll=NULL;
    plistbox->ptitle=NULL;
    plistbox->controlinfo.status=0;
    plistbox->controlinfo.show=Listbox_show;
    plistbox->controlinfo.type=IDC_LISTBOX;
    plistbox->controlinfo.sendmessage=Listbox_DispatchMsg;
    plistbox->controlinfo.thex=x;
    plistbox->controlinfo.they=y;
    plistbox->controlinfo.thez=0;
    plistbox->controlinfo.thew=width;
    plistbox->controlinfo.theh=hight;
	plistbox->showoneitem=Listbox_showoneitem;
    plistbox->items_per_win=items_per_win;
    plistbox->height_peritem=hight/items_per_win;

    return 0;
}

///////////////////////////////////////////////// SCROLL BOX

void _scrollbox_calcmoveheight(PSCROLL_BOX pscroll);

//���������Ժ��κοؼ��󶨣�����listbox,�Ժ���ı����



//��ʾ�������ƶ���ֻ������ʾ,����move_y,begin_y
//distence: �µ��϶���λ��
//�Ƿ��һ����ʾ

void _scrollbox_movebox_show(PSCROLL_BOX pscroll,WORD distence,int ifinitshow )
{
    WORD max_y,curr_y;
    WORD x=pscroll->controlinfo.thex;
    DWORD color;


    curr_y=(WORD)pscroll->move_y+distence;
    max_y=pscroll->range_height+pscroll->range_y-pscroll->move_height;
    if ( curr_y>max_y )
        curr_y=max_y;
    else if (curr_y<(WORD)pscroll->range_y)
        curr_y=(WORD)pscroll->range_y;

    if (pscroll->drag_status==1)
        color=	SCROLLBOX_DRAGBOX_SEL_COLOR;
    else
        color=	SCROLLBOX_DRAGBOX_COLOR;

    hidemouse();

    if (ifinitshow==1) {
        FillRect(x+1,pscroll->range_y,pscroll->controlinfo.thew-2,pscroll->range_height,SCROLLBOX_RANGE_COLOR);//��ʾ����
    } else {//overwrite dragbox
		WORD y, height;
		if(pscroll->move_y > curr_y) {
			y = curr_y + pscroll->move_height + 1;
			height = pscroll->move_y - curr_y;
		} else if(pscroll->move_y < curr_y){
			y = pscroll->move_y;
			height = curr_y - y;
		} else {
			height = 0;
		}
		if(height) {
			//FillRect(x+1,pscroll->move_y,pscroll->controlinfo.thew-2,pscroll->move_height,SCROLLBOX_RANGE_COLOR);//��ʾ�϶���
			FillRect(x+1,y,pscroll->controlinfo.thew-2,height,SCROLLBOX_RANGE_COLOR);//��ʾ�϶���
		}
    }

    pscroll->move_y=curr_y;
    if (pscroll->move_height!=0) {
        FillRect(x+1,curr_y,pscroll->controlinfo.thew-2,pscroll->move_height,color);//��ʾ�϶���
    }
    showmouse();
}

void Scrollbox_show(struct CONTROL * pcontrol )
{
    PSCROLL_BOX pscroll=(PSCROLL_BOX)pcontrol;

    //Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh-1, SCROLLBOX_LINE_COLOR);
    pscroll->high_btn.controlinfo.status = 1;
    pscroll->low_btn.controlinfo.status = 1;
    pscroll->high_btn.controlinfo.show((struct CONTROL*)&pscroll->high_btn);
    pscroll->low_btn.controlinfo.show((struct CONTROL*)&pscroll->low_btn);
    _scrollbox_movebox_show(pscroll,0,1);
    Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh, COLOR_SHADOW_GREY);
   
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

// ����������������ı�ʱ�����б����ã����ɹ�����������ô˺��������ڲ���
//���¹�������״̬��
//���������б��ǰ״̬�����������λ��,������move_y
//todo ֻ�����϶���
void Scrollbox_updatemove(PSCROLL_BOX pscroll,WORD firstitem_inwindow,WORD totalline)
{
    WORD new_y;

    if (totalline!=pscroll->totalline) { //���¼����϶�����С
        pscroll->totalline=totalline;
        _scrollbox_calcmoveheight(pscroll);
        //pscroll->move_height=(pscroll->line_perwin*pscroll->range_height)/totalline;
    }

    if (totalline==0) {
        _scrollbox_movebox_show(pscroll,0,1);
    } else {
        new_y=(pscroll->range_height*firstitem_inwindow)/pscroll->totalline;
        //	debug(100,0,firstitem_inwindow);
        //	debug(100,20,new_y+pscroll->range_y);

        if ((new_y+pscroll->move_height)>pscroll->range_height)
            new_y=pscroll->range_height-pscroll->move_height; //���ܳ���range��Χ��
        new_y+=pscroll->range_y;//��������

        //	if (new_y==pscroll->move_y)
        //		return; //no need update
        _scrollbox_movebox_show(pscroll,(int)new_y-(int)pscroll->move_y,1);
    }
}

void extern_control_dispatchmsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmmessage)
{
    WORD y;
    struct DM_MESSAGE mymsg;
    EXTERN_BUTTON *pextern_button=(EXTERN_BUTTON *)pcontrol;
	  BOOL bRedraw = TRUE;

 // 	Button_dispatchmsg(pcontrol,dmmessage); // Ĭ�ϴ�����

 //   MyLog(LOG_MESSAGE, L"extern_message %x.",dmmessage->message);

    switch (dmmessage->message) {

 //   case CHAR_LEFT:
 //   case CHAR_DOWN:
 //   case CHAR_RIGHT:
    	  
 //        break; 
//		case WM_DOUBLECLICKED:
    case WM_LEFTBUTTONPRESSED:
      
		    pextern_button->controlinfo.state = CONTROL_STATE_PRESSED;
        dmmessage->message = 0;
        break;
    case WM_LEFTBUTTONRELEASE:
    		
    		pextern_button->controlinfo.state = CONTROL_STATE_NORMAL;

        switch ( pextern_button->controlinfo.control_id) {
        case IDB_PAGE_UP:
            pextern_button->controlinfo.status = 1;

            mymsg.message=CHAR_UP;

            Control_sendmsg(pextern_button->pbindctl,&mymsg);
            dmmessage->message = 0;

            break;
        case IDB_PAGE_DOWN:
            pextern_button->controlinfo.status = 1;
     				
            mymsg.message=CHAR_DOWN;
            Control_sendmsg(pextern_button->pbindctl,&mymsg);
            dmmessage->message = 0;
            
            break;
        }

        break;
    default:  //������Ϣ�������󶨵Ŀؼ�
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



void Scrollbox_dispatchmsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmmessage)
{
    WORD y;
    struct DM_MESSAGE mymsg;
    PSCROLL_BOX pscroll=(PSCROLL_BOX)pcontrol;

    if ((ifmousemsg(dmmessage)||(dmmessage->message==WM_MOUSEMOVEOUT))\
            &&(pscroll->drag_status==1)&&(dmmessage->message!=WM_MOUSEDRAG)) { //�����϶�
        mymsg.message=WM_SCROLL_MOVE;
        //�����϶���
        pscroll->drag_status=0;
        _scrollbox_movebox_show(pscroll,0,0);
        //������Ϣ���󶨿ؼ�
        mymsg.thex=((WORD)(pscroll->move_y-pscroll->range_y)*(WORD)(pscroll->totalline))/(WORD)(pscroll->range_height);//���ڵ�һ��item
//				debug(200,20,pscroll->move_y);
//				debug(200,40,pscroll->range_y);
//				debug(200,60,pscroll->range_height);
//
//				debug(200,0,mymsg.thex);
        //pscroll->pbindctl->sendmessage((PCONTROL)pscroll->pbindctl,&mymsg);
        Control_sendmsg((PCONTROL)pscroll->pbindctl,&mymsg);
        return ;
    }
    switch (dmmessage->message) {
//		case WM_DOUBLECLICKED:
    case WM_LEFTBUTTONPRESSED:
        y=(WORD)dmmessage->they;
        pscroll->whichbutton=0xffff;
        if (y<=pscroll->range_y) { //�ϰ�ť
            //pscroll->high_btn.controlinfo.sendmessage((PCONTROL)&pscroll->high_btn,dmmessage);
            Control_sendmsg((PCONTROL)&pscroll->high_btn,dmmessage);
            pscroll->whichbutton=0;
        } else if (y>=pscroll->low_btn.controlinfo.they) { //�°�ť
            //pscroll->low_btn.controlinfo.sendmessage((PCONTROL)&pscroll->low_btn,dmmessage);
            Control_sendmsg((PCONTROL)&pscroll->low_btn,dmmessage);
            pscroll->whichbutton=1;
        } else if (y<pscroll->move_y) { //�϶����Ϸ�
            mymsg.message=CHAR_PGUP;

            //pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
            Control_sendmsg(pscroll->pbindctl,&mymsg);
        } else if (y>(pscroll->move_y+pscroll->move_height)) { //�϶����·�
            mymsg.message=CHAR_PGDOWN;
            //pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
            Control_sendmsg(pscroll->pbindctl,&mymsg);
        } else { //�϶���
            //	pscroll->whichbutton=2;
            pscroll->drag_status=1;
            pscroll->drag_begin_y=(WORD)dmmessage->they;
            //��ʾ
            _scrollbox_movebox_show(pscroll,0,0);
        }
        break;
    case WM_LEFTBUTTONRELEASE:
        switch ( pscroll->whichbutton) {
        case 0:
            //pscroll->high_btn.controlinfo.sendmessage((PCONTROL)&pscroll->high_btn,dmmessage);
            Control_sendmsg((PCONTROL)&pscroll->high_btn,dmmessage);
            mymsg.message=WM_SCROLL_UP;
            //pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
            Control_sendmsg(pscroll->pbindctl,&mymsg);
            break;
        case 1:
            //pscroll->low_btn.controlinfo.sendmessage((PCONTROL)&pscroll->low_btn,dmmessage);
            Control_sendmsg((PCONTROL)&pscroll->low_btn,dmmessage);
            mymsg.message=WM_SCROLL_DOWN;
            //pscroll->pbindctl->sendmessage(pscroll->pbindctl,&mymsg);
            Control_sendmsg(pscroll->pbindctl,&mymsg);
            break;
        }
        break;
    case WM_MOUSEDRAG:
        if (pscroll->drag_status==1) {
            _scrollbox_movebox_show(pscroll,(WORD)dmmessage->they-pscroll->drag_begin_y,0);
            pscroll->drag_begin_y=(WORD)dmmessage->they;
            /////////////
        }
        break;
    case WM_SCROLLBOX_UPDATE:
        Scrollbox_updatemove(pscroll,(WORD)dmmessage->thex,(WORD)dmmessage->they);
        break;
    default:  //������Ϣ�������󶨵Ŀؼ�
        //pscroll->pbindctl->sendmessage((PCONTROL)pscroll->pbindctl,dmmessage);
        Control_sendmsg((PCONTROL)pscroll->pbindctl,dmmessage);
        break;
    }

}
void _scrollbox_calcmoveheight(PSCROLL_BOX pscroll)
{
    WORD height;
    if (pscroll->totalline==0) {
        pscroll->move_height=0;
        return;
    }
    height=(pscroll->line_perwin*pscroll->range_height)/pscroll->totalline;
    if (height>=pscroll->range_height) //����̫��̫���ȡ����������
        height=0;
    else if (height<=5) //����С��5�����أ����򿴲������
        height=5;
    pscroll->move_height=height;
}

// totalitem : �ܵ�����,
// window: һ��������.
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

    Button_init(&pscroll->high_btn,x,y,width,width,IMG_FILE_UPARROWB,IMG_FILE_UPARROWA, IMG_FILE_UPARROWA, 0);
    Button_init(&pscroll->low_btn,x,y+height-width,width,width,IMG_FILE_DOWNARROWB,IMG_FILE_DOWNARROWA,IMG_FILE_DOWNARROWA, 0);
    
    pscroll->range_height=height-2*width;
    pscroll->range_y=y+width;
    pscroll->line_perwin=window;
    pscroll->totalline=totalitem;

    _scrollbox_calcmoveheight(pscroll);
    pscroll->move_y=y+width;
    pscroll->drag_status=0;
    return 0;
}


// bindctl: ��Ҫ�󶨵Ŀؼ�,�˿ؼ������ȳ�ʼ�������Ѿ�additem��
// width: ���������
int Scrollbox_init(PSCROLL_BOX pscroll,PCONTROL bindctl,WORD totalitem,WORD item_per_win)
{
    _scrollbox_init (pscroll,bindctl->thex+bindctl->thew,bindctl->they,\
                     SCROLLBOX_WIDTH,bindctl->theh,totalitem,item_per_win);
    pscroll->pbindctl = bindctl;
//	bindctl->pscroll=pscroll;
    return 0;
}

void Titlebox_show(struct CONTROL * pcontrol )
{
    WORD i;
    WORD width=pcontrol->thew;
    WORD thex=pcontrol->thex;
    WORD they=pcontrol->they;
    PTITLEBOX ptitle=(PTITLEBOX)pcontrol;
    hidemouse();

    DrawLine(thex,they+1,width,1,TITLEBOX_HIGHLINE_COLOR);
    FillRect(thex,they+2,width,pcontrol->theh-5,TITLEBOX_BG_COLOR);

    DrawLine(thex,they+23,width,1,TITLEBOX_LOWHLINE1_COLOR);
    DrawLine(thex,they+24,width,1,TITLEBOX_LOWHLINE2_COLOR);
    DrawLine(thex,they+25,width,1,TITLEBOX_LOWHLINE3_COLOR);

    Linebox(thex,they, width,pcontrol->theh,TITLEBOX_FRAMELINE_COLOR);
    for (i=0;i<ptitle->column_num;i++ ) {
        WORD x=ptitle->column_list[i].begin_x;

        PutstrinWidth(x+4,they+6,ptitle->column_list[i].column_name,COLOR_BLACK,ptitle->column_list[i].curr_width-4);
        if (i==0)
            continue;
        DrawLine(x,they+4,1,18,TITLEBOX_GREYLINE_COLOR);
        DrawLine(x+1,they+4,1,18,COLOR_WHITE);
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
    if (dx>0) {//������
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
        if (dmmessage->message!=WM_MOUSEDRAG) { //�϶�����
            //�����б��
            int dx=dmmessage->thex-ptitle->drag_begin_x;
            Titlebox_move(ptitle,dx);

            ptitle->plistbox->controlinfo.show((PCONTROL)ptitle->plistbox);
            //ptitle->plistbox->controlinfo.sendmessage((PCONTROL)ptitle->plistbox,&msg);
            Control_sendmsg((PCONTROL)ptitle->plistbox,&msg);

            ptitle->drag_status=0;
            dmmessage->message=0;
            //������¹��
        } else {  		//�����϶�
            // ���±����
//			int dx=dmmessage->thex-ptitle->drag_begin_x;
//			for(i=plistbox->drag_col;i<plistbox->column_num;i++){
//				ptitle->column_list[i].begin_x+=dx;
//			}
//			ptitle->drag_begin_x=dmmessage->thex;
//			Titlebox_update();
            return;
        }
    }

    // �޸Ĺ��
    if (ptitle->drag_status==0) {
        if ( (dmmessage->they<pcontrol->they)||( dmmessage->they>=(pcontrol->they+TITLEBOX_HEIGHT )) )
            goto nochange_cursor;
        //�Ƿ�������������
        for (i=1;i<ptitle->column_num;i++) {
            if ( (dmmessage->thex>(ptitle->column_list[i].begin_x-5))&&(dmmessage->thex<(ptitle->column_list[i].begin_x+5) )) {
                whichcol=i;
                //�����������ڣ��ı���״
                if (ptitle->cursor_changed==NO)	{
//					    debug(0,500,(u16)&Cursor_move);
//					    getch();
                    ptitle->old_curosr=change_cursor(&Cursor_move );
                    ptitle->cursor_changed=YES;
                }
            }
        }
nochange_cursor:
        //������������,�ı�����״
        if (whichcol==0Xffff) {
            if (ptitle->cursor_changed) {
//		        debug(0,750,(u16)&Cursor_move);
//			    getch( );
                change_cursor(ptitle->old_curosr);
                ptitle->cursor_changed=NO;
            }
        }

    }
    //�������϶�ʱ���������������������ʱ����ǿ�ʼ�϶�
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
// �߶ȹ̶���yλ����plistboxȷ����
int Titlebox_init(PTITLEBOX ptitle,PLISTBOX plistbox, WORD width,PONECOLUMN cols,WORD cols_num)
{
    WORD i,x;
    x=plistbox->controlinfo.thex;
    ptitle->controlinfo.thex=x;
    ptitle->controlinfo.they= plistbox->controlinfo.they - TITLEBOX_HEIGHT;
    ptitle->controlinfo.thew=width;
    ptitle->controlinfo.theh=TITLEBOX_HEIGHT;
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
        ptitle->column_list[i].begin_x=x;
        x+=width;
    }
    ptitle->plistbox=plistbox;
    plistbox->ptitle=ptitle;
    return 0;
}



///////////////////////////////////////////////////////////////////////////////// edit box



//��ָ��λ���ϲ���һ���ַ�,pos���Դ�0-current_len
int  insertachar(PEDITBOX  peditbox,WORD pos,CHAR16 ch)
{
    CHAR16 *p=peditbox->name;
    int i;

    if ((peditbox->insmode==0)&&(pos!=peditbox->current_len)) { //��������м䣬��ֱ�Ӹ��ǣ�����Ҫ�ƶ�
        p[pos]=ch;
        return 0;
    }
    if (peditbox->current_len==peditbox->num)
        return -1;

    //�����λ���ϵ��ַ����ͺ�����ַ�ȫ�����ƣ�����\0
    for (i=(peditbox->current_len);i>=(int)pos;i--) {
        p[i+1]=p[i];
    }

    p[pos]=ch;
    peditbox->current_len++;
    return 0;
}

//��ָ��λ����ɾ��һ���ַ�
int deleteachar(PEDITBOX  peditbox,WORD pos)
{
    CHAR16 *p=peditbox->name;
    WORD i;

    if (p[pos]==0)//��ǰ�Ѿ���ĩβ
        return -1;
    //���¸��ַ����ͺ�����ַ�ȫ��ǰ�ƣ�����\0
    for (i=(pos+1);i<=(peditbox->current_len);i++) {
        p[i-1]=p[i];
    }
    peditbox->current_len--;
    return 0;
}
//��ʾ�±�,��pos=0xffff��ʾ���ع��,����ģʽ��ʾ���ߣ��滻ģʽ��ʾ����
void textcusor_move(PEDITBOX  peditbox,WORD pos)
{
    WORD oldthex=peditbox->controlinfo.thex+peditbox->posofcursor*EDIT_PIXEL_PER_CHAR+EDIT_BEGIN_X;
    WORD newthex=peditbox->controlinfo.thex+pos*EDIT_PIXEL_PER_CHAR+EDIT_BEGIN_X;

    hidemouse();
    if (peditbox->insmode==1) {
        DrawLine(oldthex-1,peditbox->controlinfo.they+9,1,16,EDITBOX_BACKGROUND);
        if (pos!=0xffff) {
            DrawLine(newthex-1,peditbox->controlinfo.they+9,1,16,COLOR_BLACK);
        }
    } else {
        DisplayChar(oldthex,peditbox->controlinfo.they+11,EDITBOX_BACKGROUND,L'_');
        if (pos!=0xffff) {
            DisplayChar(newthex,peditbox->controlinfo.they+11,COLOR_BLACK,L'_');
        }
    }
    if (pos!=0xffff)
        peditbox->posofcursor=pos;
    showmouse();
}
//��ʾ�����ַ�,�ѹ��Ҳ���
void puteditstr(PEDITBOX peditbox,DWORD bkcolor,DWORD charcolor)
{
    WORD i, x;
    CHAR16 ch;

    FillRect(peditbox->controlinfo.thex+1, peditbox->controlinfo.they+1,
		peditbox->controlinfo.thew-2,peditbox->controlinfo.theh-2,bkcolor);

	x = peditbox->controlinfo.thex + EDIT_BEGIN_X;
	i = 0;
    while(ch = peditbox->name[i]) {
		if (peditbox->type & EDIT_MODE_PASSWORD)
			ch = L'*';
		DisplayChar(x, peditbox->controlinfo.they+9, charcolor, ch);
		if(ch > 0x80)
			x += 2*EDIT_PIXEL_PER_CHAR;
		else
			x += EDIT_PIXEL_PER_CHAR;
		i++;
    }

	peditbox->current_len = i;
}

void Editbox_show(struct CONTROL * pcontrol)
{
    PEDITBOX peditbox=(PEDITBOX)pcontrol;
    hidemouse();
    
    DrawSinkableRectangle(peditbox->controlinfo.thex, peditbox->controlinfo.they,
    	peditbox->controlinfo.thew, peditbox->controlinfo.theh, COLOR_SHADOW_GREY, EDITBOX_BACKGROUND);
    //FillRect(peditbox->controlinfo.thex,peditbox->controlinfo.they+1,peditbox->controlinfo.thew,peditbox->controlinfo.theh,EDITBOX_BACKGROUND);
    
//	Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh,COLOR_EIDTBOX_UNFOCUS);
    puteditstr(peditbox,EDITBOX_BACKGROUND,COLOR_BLACK);
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
    puteditstr(peditbox,SCROLLBOX_RANGE_COLOR,SCROLLBOX_LINE_COLOR);
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
    //���ж���ĸ����
    if (ifkeymsg(pdmsg)) {
        CHAR16 ch=pdmsg->message;
        if ((ch<0x20)||(ch>0x7e)) //no ascii
            goto next_disp;
        if ((peditbox->type & EDIT_MODE_DIGIT)&&((ch<L'0')||(ch>L'9')))
            goto next_disp;
		if(!IsValidFileName(ch))
			goto next_disp;
        //���Դ���
        if (insertachar(peditbox,peditbox->posofcursor,ch)<0)
            return; //����ʧ��

        //	textcusor_move(peditbox,peditbox->posofcursor+1);
        peditbox->posofcursor++;
        ifneedupdate=1;
        pdmsg->message=0;
    }

next_disp:
    switch (pdmsg->message)
    {
    case WM_LEFTBUTTONPRESSED:  //�ƶ��±�
        thex=((WORD)pdmsg->thex-pcontrol->thex);
        if (thex>EDIT_BEGIN_X) {
            thex-=EDIT_BEGIN_X;
            newpos=thex/PIXEL_PER_CHAR;
            if (newpos>peditbox->current_len) {
                newpos=peditbox->current_len;
            }
        } else
            newpos=0;
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

        pdmsg->message=0;//��Ϣ�ÿ�
        break;

    case CHAR_RIGHT:
        if (peditbox->posofcursor!=peditbox->current_len)
            textcusor_move(peditbox,peditbox->posofcursor+1);
        pdmsg->message=0;//��Ϣ�ÿ�
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
        hidemouse();
        Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh,COLOR_EIDTBOX_FOCUS);
        showmouse();
        textcusor_move(peditbox,peditbox->posofcursor);
        break;
    case WM_CTL_INACTIVATE:
        hidemouse();
        //Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh+1,COLOR_EIDTBOX_UNFOCUS);
        Linebox(peditbox->controlinfo.thex,peditbox->controlinfo.they,peditbox->controlinfo.thew,peditbox->controlinfo.theh,COLOR_SHADOW_GREY);
        showmouse();
        textcusor_move(peditbox,0xffff);
        break;

    case WM_MOUSEMOVEIN: //�������
        change_cursor(&Cursor_text);
        break;

    case WM_MOUSEMOVEOUT: //����Ƴ�
        change_cursor(&Cursor_normal);

        break;
    case WM_CTL_DISABLE:
        Editbox_Disable(peditbox);
        break;
    case WM_CTL_ENABLE:
        Editbox_show(pcontrol);
//		default:
//			pdmsg->message=0;//��Ϣ�ÿ�
//			break;
    }
    //������ʾ�ַ��͹��
    if (ifneedupdate==1) {
        hidemouse();
        puteditstr(peditbox,COLOR_WHITE,COLOR_BLACK);
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
	peditbox->ifselected = NO;//Ĭ�ϲ�ѡ��
	peditbox->num = num;
	peditbox->current_len = (WORD)StrLength(str);
	peditbox->posofcursor = peditbox->current_len;	//��ǰ�±���ĩβ
	peditbox->insmode = 1; //Ĭ�� ����ģʽ
	return;
}

void Editbox_init(struct EDITBOX * peditbox,WORD x,WORD y,WORD width,CHAR16 * str,BYTE type,WORD num)
{
    peditbox->controlinfo.thex=x;
    peditbox->controlinfo.they=y;
    peditbox->controlinfo.thew=width;
    peditbox->controlinfo.theh=33;
    peditbox->controlinfo.status=0;
    peditbox->controlinfo.type=IDC_EDITBOX;
    peditbox->controlinfo.thez=0;
    peditbox->controlinfo.control_id=IDB_EDIT_BTN;

    peditbox->controlinfo.sendmessage=Edit_dispatchmsg;
    peditbox->controlinfo.show=Editbox_show;

    peditbox->name=str;
    peditbox->type=type;
    peditbox->ifselected=NO;//Ĭ�ϲ�ѡ��
    peditbox->num=num;
    peditbox->current_len=(WORD)StrLength(str);
    peditbox->posofcursor=peditbox->current_len;	//��ǰ�±���ĩβ
    peditbox->insmode=1; //Ĭ�� ����ģʽ
    return;
}


/////////////////////////////////////////////////////////////////  Radio box �ؼ�

//���ô���ѡ��
void Radio_show( PCONTROL pcontrol )
{
    PRADIOBOX pradio=(PRADIOBOX)pcontrol;
    FillRect(pcontrol->thex+1,pcontrol->they+2,pcontrol->thew-2,pcontrol->theh-4,RADIO_BG_COLOR);
    if (pradio->status==0)
        DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_RADIOB,FALSE);
    else
        DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_RADIOA,FALSE);
    hidemouse();

    DisplayString(pcontrol->thex+20,pcontrol->they+2,COLOR_BLACK,pradio->name);
    showmouse();
    return ;
}
//���ô���ѡ��
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
//��ö��radio���ĸ���ѡ�еģ�������ţ�ָ�����к�)
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
    DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_RADIOC,FALSE);

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
//	PutString(pcontrol->thex+20-1,pcontrol->they+2-1,(WORD)pradio->name,DIALOG_BG_COLOR,0);//ȥ��
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

    if (pcheck->status==1)
        DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXA,FALSE);
    else
        DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXB,FALSE);
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
        DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXB,FALSE);

    } else {
        pcheck->status=1;
        DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXA,FALSE);

    }
    return 0;

}
int Checkbox_Disable(PCONTROL pcontrol)
{
    PCHECKBOX pcheck=(PCHECKBOX)pcontrol;
    DisplayImg(pcontrol->thex+1,pcontrol->they+2,IMG_FILE_CHECKBOXC,FALSE);

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
//	PutString(pcontrol->thex+20-1,pcontrol->they+2-1,(WORD)pcheck->name,DIALOG_BG_COLOR,0);//����
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
    WORD  i;
    WORD thex=pcontrol->thex;
    WORD they=pcontrol->they;
    WORD thew=pcontrol->thew;
    PPROCESSBOX pproc=(PPROCESSBOX)pcontrol;

    hidemouse();

    FillRect(thex,they+4,thew,16,COLOR_PROC_FILL);
    FillRect(thex,they+1,thew,3,COLOR_PROC_HIGHLINE);
    Linebox(pcontrol->thex,pcontrol->they,pcontrol->thew,pcontrol->theh-1,COLOR_PROC_FRAME);
    for (i=0;i<pproc->length;i++)
        DisplayImg(thex+1+i, they+1,IMG_FILE_PROC,FALSE);

    showmouse();

    return ;
}

void Processbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage)
{
    WORD newlen, i;
    PPROCESSBOX pproc = (PPROCESSBOX)pcontrol;

    switch (dmessage->message ) {
    case WM_PROCESS_RUN:
        if (dmessage->thex > 100)
            break;

		newlen = (WORD)dmessage->thex * (pcontrol->thew-2)/100;
		i = pproc->length;
		if(g_data.bDebug) {
			if(i + pcontrol->thex + 1 < LOG_GRAPHIC_X + LOG_GRAPHIC_WIDTH) {
				i = LOG_GRAPHIC_X + LOG_GRAPHIC_WIDTH - (pcontrol->thex + 1);
			}
		}
        hidemouse();
        for (;i<newlen;i++)
            DisplayImg(pcontrol->thex+1+i, pcontrol->they+1,IMG_FILE_PROC,FALSE);
        pproc->length = newlen;
        showmouse();
        break;
    case WM_PROCESS_INIT:
        pproc->length=0;
        pcontrol->show(pcontrol);
        break;
    }

}
int Processbox_init(PPROCESSBOX pproc,WORD x,WORD y,WORD width)
{
    pproc->controlinfo.thex=x;
    pproc->controlinfo.they=y;
    pproc->controlinfo.thew=width;
    pproc->controlinfo.theh=20;
    pproc->controlinfo.status=CON_STATUS_TABNOTSTOP;
    pproc->controlinfo.type=IDC_PROCESSBOX;
    pproc->controlinfo.thez=0;

    pproc->controlinfo.sendmessage=Processbox_dispatchmsg;
    pproc->controlinfo.show=Processbox_show;
    pproc->length=0;
    return 0;

}


