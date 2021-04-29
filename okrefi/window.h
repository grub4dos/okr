/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef _OKR_WINDOW_H_
#define _OKR_WINDOW_H_


#define WM_NOMESSAGE 0x0000
#define WM_MOUSEMOVE  0xff00           //����ƶ�
#define WM_LEFTBUTTONPRESSED  0xff01   //����������
#define WM_LEFTBUTTONRELEASE  0xff02   //����������
#define WM_RIGHTBUTTONPRESSED  0xff03  //����Ҽ�����
#define WM_RIGHTBUTTONRELEASE  0xff04  //����Ҽ�����
//#define WM_DOUBLECLICKED  0xff05       //���˫��������Ϣ֮ǰ�Ѿ���������£�������Ϣ
#define WM_MOUSEDRAG  0xff06           //���������£����ƶ�


// �������Զ�����Ϣ
#define  WM_MOUSEMOVEOUT 0xfe30  //����Ƶ����ؼ��⣬��ǰ���x,y����ֱ���thex,they��
#define  WM_MOUSEMOVEIN 0xfe31  //����Ƶ��ؼ��ڣ�x,y����ֱ���thex,they��
#define  WM_CTL_ACTIVATE 0xfe32 //���ؼ�����ʱ��WINDOW����Ϣ���ؼ�
#define  WM_CTL_INACTIVATE 0xfe33 //�Ӽ���ת�ɲ����WINDOW����Ϣ���ؼ�

#define  WM_BUTTON_CLICK   0XFE34 //�ڰ�ť�ϵ����꣬���߰��ո񣬻س�����������Ϣ������Ϣ��BUTTON�ؼ�ת��

#define  WM_SCROLL_UP   0xfe35  //�û�������������ϰ�ť������Ϣ�ɹ����������󶨿ؼ�
#define  WM_SCROLL_DOWN  0XFE36 //�û�������������ϰ�ť������Ϣ�ɹ����������󶨿ؼ�
#define  WM_SCROLL_MOVE  0XFE37 //�û��϶������������������͸��ؼ�,thex=��ʼ����������Ϣ�ɹ����������󶨿ؼ�

#define  WM_SCROLLBOX_UPDATE 0XFE38  //�������ؼ��������Ϣ,���¹�����,THEX=��ʼ����,they��������

#define  WM_PROCESS_RUN      0XFE39  //�������ؼ��������Ϣ,���½�������thex=�ٷ�֮������Χ0��100
#define  WM_PROCESS_INIT     0XFE3a  //�������ؼ��������Ϣ,���ȹ�0

#define  WM_CTL_DISABLE      0xfe3B // ����Ҫ����ĳ���ؼ�ʱ������Control_diable����������������Ϣ�������õĿؼ�
#define  WM_CTL_ENABLE      0xfe3C // �������ÿؼ�������ͬ��

#define  WM_TIMEOUT	    0XFE3D

#define WM_CONTEXT_CHANGE 0XFE3E   //COMBOBOX,��CHECKBOX�����ݷ����仯����Щ���ݱ仯�����ⲿ�ı䣬Ҳ���û��������������롣
#define WM_LISTBOX_SELECT 0XFE3F  // LISTBOX �û���ENTER,SPACE,˫�������������Ϣ��
#define  WM_LISTBOX_SELCHANGE 0XFE40

#define  WM_BTN_SELCHANGE 0XFF10

/////////////////////////////////////////// control char +0x80
//SEE simpletextin.h
#define CHAR_UP         0x0081
#define CHAR_DOWN       0x0082
#define CHAR_RIGHT      0x0083
#define CHAR_LEFT       0x0084
#define CHAR_HOME       0x0085
#define CHAR_END        0x0086
#define CHAR_INSERT     0x0087
#define CHAR_DELETE     0x0088
#define CHAR_PGUP       0x0089
#define CHAR_PGDOWN     0x008A
#define CHAR_ESC        0x0097
#define CHAR_RETURN  CHAR_CARRIAGE_RETURN  

#define CHAR_F1			0x8B
#define CHAR_F12		0x96


#define YES 1
#define NO 0

#define PIXEL_PER_CHAR  8

#define IMG_FILE_PCX		0	//pcx��ʽ
#define IMG_FILE_BMP		1	//û��ѹ����bmp��ʽ
#define IMG_FILE_JPG		2	//jpg��ʽ
#define IMG_FILE_LZ77		3	//ѹ����ͼ��
#define IMG_FILE_PCX_LZ77	4	//ѹ����pcx
#define IMG_FILE_BMP_LZ77	5	//ѹ����bmp
#define IMG_FILE_JPG_LZ77	6	//ѹ����jpg
typedef struct 
{
	WORD	id;
	WORD	type;		//pcx=0,jpg=1, png... reserved
	DWORD	size;		// file size in bytes
	union {
		VOID				*pmem;
		PCXHEAD 			*pcx;
		BITMAPFILEHEADER	*bmp;
	};
	VOID	*pdata;		//��ѹ�������
} IMAGE_FILE_INFO, *PIMAGE_FILE_INFO;


/////////////////////////////////////////////////////////////////////////////////////

typedef struct CURSOR_INFO
{
	DWORD pcx;
	//ͼƬ�ߺͿ�
	WORD width; 
	WORD height;
	//��꽹����ͼƬ���ĸ�λ��
	WORD focus_x; //xλ��
	WORD focus_y;//yλ��
} CURSOR_INFO ,*PCURSOR_INFO;

/*
struct M_MESSAGE {
 WORD    TheX  ;
 WORD    TheY  ;
 WORD    MESSAGE ;
};
*/
//��Ϣ�ṹ
struct DM_MESSAGE
{
	INT32 thex;					//���xλ��
	INT32 they;					//���yλ��
	WORD message;			//���ͼ�����Ϣ������������Ϣ���ϣ�����Ǽ�����Ϣ�����Ǽ��̵������ֵ
};


struct CONTROL;
//�Ƿ������Ϣ
int ifkeymsg(struct DM_MESSAGE *pdmmessage);
//�Ƿ������Ϣ 
int ifmousemsg(struct DM_MESSAGE *pdmmessage);
//�жϵ�ǰ�����Ϣ�����ĸ��ؼ���
WORD detectwhere(struct CONTROL * p[],WORD totalnum,struct DM_MESSAGE *msg);
void nop();
void detectmessage(struct DM_MESSAGE *,EFI_SIMPLE_POINTER_PROTOCOL *mouse);
////////////////////////////////////////////////////////////////////////////// ͨ�ÿؼ��ṹ
/*
   ÿ�ֿؼ�����һ��CONTROL�Ľṹ���������ұ������ڿؼ��ṹ��ͷ����������c++�̳й�ϵ��CONTROL�ṹ
�൱�ڻ���,ÿ���ؼ����ӻ���̳С�
   CONTROL�ṹ���пؼ��������ԣ�
   1 λ����Ϣ����С��Ϣ(x,y,w,h)�������ж������ϢӦ�÷��͵��ĸ��ؼ���
   2 ״̬��Ϣ(status),�жϿؼ��Ƿ�֧��TAB�����Ƿ��Ѿ�����
   3 windowָ�룬��¼�ؼ�������WINDOW
   4 ��Ϣ������ָ��sendmessage�����ڴ������ؼ�����Ϣ(�����������,�����Զ�����Ϣ)��������win sdk�Ĵ��ڴ�������
   �ؼ�ʹ���ߣ��Զ��崦������������c++�������ء�
   5 �ؼ���ʾ����ָ�룬�˱�����ʵ���Ժϲ���sendmessage�����У�ֻ��Ҫ���һ��WM_SHOW��Ϣ���ɣ�����������ڸ��ݵ�ǰ��
   �ؼ�״̬����������ʾ�ؼ���ۡ��˺��������ǽ����ڳ�ʼ��ʱ���ã�����Ҫ���¿ؼ���ʾʱ�ͻ���á��˺�����ʾ�Ŀؼ�״̬
   ���ǿؼ��ļ���״̬��
   
   
*/

// STATUS ��λ���壬����bit0=1��ʾTABNOTSTOP;BIT1=1��ʾDISABLE
#define CON_STATUS_TABNOTSTOP		1  //�ؼ���֧�ּ���״̬��tab����ͣ���������������
#define CON_STATUS_DISABLE			2  //�ؼ��Ѿ����ã�����Ӧ�κ���Ϣ��
#define CON_STATUS_HIDE				4

#define CONTROL_STATE_NORMAL		0
#define CONTROL_STATE_FOCUS			1
#define CONTROL_STATE_PRESSED		2
#define CONTROL_STATE_DISABLE		3
struct WINDOW;
typedef struct CONTROL
{
	WORD thex;
	WORD they;
	WORD thew;
	WORD theh;
	
	BYTE thez;					//z����
	BYTE type; 					//�ؼ�����
	BYTE status;        		//�ؼ�״̬����λ����
	BYTE state;					//�ؼ���ǰ״̬�����֣�focus��pressed��normal
	
	WORD control_index;         //�ؼ������ţ���window�ṹ�У�
	WORD control_id;			//�ؼ�id������ֵ
	struct WINDOW *pwin;        //ָ��ؼ�������window����,�ɴ��ڳ�ʼ��ʱ���롣
	void (* sendmessage)(struct CONTROL *, struct DM_MESSAGE *);
	void (* show)(struct CONTROL *);  //��ʾ
	
} CONTROL, * PCONTROL;

//�ؼ���������

#define IDC_BUTTON  1
#define IDC_LISTBOX 2
#define IDC_SCROLLBOX 3
#define IDC_TITLEBOX 4
#define IDC_EDITBOX 5
#define IDC_RADIOBOX 6
#define IDC_CHECKBOX 7
#define IDC_COMBOBOX 8
#define IDC_PROCESSBOX 9
//��װ�������Կؼ�������Ϣ
void Control_sendmsg(PCONTROL pctrl,struct DM_MESSAGE *dmmessage);
//���ÿؼ�
int Control_disable(PCONTROL pctl);
//���ÿؼ�
int Control_enable(PCONTROL pctl);

//int Control_ShowUI(PCONTROL pctl);
// 
//int Control_HideUI(PCONTROL pctl);


////////////////////////////////////////////////////////////////////////////////// WINDOW�ṹ
/* 
    WINDOW�Ƕ��CONTROL������(container),����Ϊ������CONTROL,���Ĺ���������Ϣ��������ͬ����window
 ���������������Ϣ����������Ϣ�ַ���ÿ���ӿؼ�������ֱ�Ӻ��ԡ����⻹����һЩĬ����Ϣ������TAB����
 ����һЩ�̶���Ԫ�أ����米����
    ʹ���߻����Զ�����Ϣ������msgfunc��ȥ��ӦһЩ������Ϣ�������ȼ���
    window�ṹ�У��м����������⿪�ţ�
    current: ��ǰ����ؼ���
    ret_data: ����window�ķ���ֵ����Window_exit()��ֵ��
    msgfunc�� ��Ϣ������
    show:     ����ͬcontrol�ṹ��show,��ʾ���ڵ�����Ԫ�أ���ֹ�ڳ�ʼ��ʱ���á�
    control_list: ÿ���ӿؼ���ָ�롣
    
WINDOWʹ�����̣�
    1. WINDOW�и���CONTROL��ʼ����
    2. Window_init,��Ҫʱ�Զ�����Ϣ������msgfunc
    3. Window_addctl����Ӹ����ӿؼ�
    4. Window_run,������Ϣ����
    5. ���ؼ�����Window_exit��window�˳����ж�Window_run�ķ���ֵ��
*/
//ÿ��window�����ʾ30���ؼ�
#define MAX_CONTRL_INWIN  30
typedef struct WINDOW
{
	PCONTROL control_list[MAX_CONTRL_INWIN]; //�ؼ�����,���30���ؼ�
	WORD control_num;
	void (* show)(struct WINDOW *);  //��ʾ
	
//	struct MSG_CALLBACK **msg_callback; //�Զ�����Ϣ��������
//	WORD msg_callback_num;
	WORD current; //��ǰ�ؼ�����Ϣ���Ϳؼ���
	WORD precurrent; //ǰһ���ؼ�
	WORD current_cursor;
	BYTE ifexit;
	void  (*msgfunc )(struct WINDOW *, struct DM_MESSAGE *); //��Ϣ������
	
	DWORD ret_data; //��¼��������ķ���ֵ

	WORD selected; //��ǰ�ؼ�����Ϣ���Ϳؼ���

} WINDOW , *PWINDOW;

//ֻ��ʾwindow�а��������пؼ�
void Window_showctls(PWINDOW pwin);
//��ʼ��window�ṹ����ָ��show����ָ�룬���=NULL,ʹ��Ĭ����ʾ����
//�û������Ҫ�Զ�����Ϣ�����ɸ���msgfunc����ָ��
int Window_init(PWINDOW pwin,void (* show)(struct WINDOW *));
//Ĭ�ϵ���ʾ��������ʾ���������пؼ�
void Window_show(PWINDOW pwin);
//�ؼ����ô˺�����������WINDOW�˳���RET=window�ķ���ֵ
int Window_exit(PWINDOW pwin,DWORD ret);
//����window����Ϣѭ����ֱ���ؼ�����Window_exit
DWORD Window_run(PWINDOW pwin);
//���һ���ؼ������ؿؼ�������,��window�Ѿ���ʼ����ʱ���������
int Window_addctl(PWINDOW pwin,PCONTROL ctl);
//window��Ĭ����Ϣ������
void  Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage);


/////////////////////////////////////////////////////////////////////////////////  �б��ؼ�
//�б���һ�ж�Ӧһ��item�� һ�ж�Ӧһ��column;
//ÿ����Ԫ���Ӧһ��unit;

//������ÿ��unit�����ͣ�Listbox_Addoneunitʱָ������λ���壨����ϣ�
#define ITEM_TYPE_CHAR_STRING  1 //bit0=1,��ͨ�ַ���
#define ITEM_TYPE_NOCOLUMN     2 //BIT1=1,һ��ֻ��һ��unit,����titlebox�ؼ�����,��������
#define ITEM_TYPE_ISCONTROL    4 //�˵�Ԫ��Ƕһ���ؼ�
#define ITEM_TYPE_IMG 8  //ͼƬ
//colors
#define LISTBOX_SELLINE_COLOR 0X3C82C8
#define LISTBOX_SELLINE_COLOR_INAC 0XC0C0C0
#define LISTBOX_LINE_COLOR 0x827864
#define COLOR_LISTBOX_SPECIAL 0Xffa000
//ONEITEM����һ��
//һ�����200byte
typedef struct ONEITEM {
//item_buff����һ��������:
//	����(2byte),����(2byte),����.../����(2byte),����(2byte),����...
	VOID * item_buff; 
	WORD    columns;  //һ��������
} ONEITEM, * PONEITEM;

struct SCROLL_BOX ;
struct PTITLEBOX;
struct LISTBOX;
typedef struct LISTBOX
{
	struct CONTROL controlinfo; 		//�ؼ���λ����Ϣ
	struct ONEITEM itemlist[70];        //һ��һ��item,һ��item��һ���ṹ��ʾ��Ŀǰ���50�У��Ժ��������ᶯ̬���䡣
	// �����еĳ��Ȳ��ܳ���100���ֽڡ�//TODO 
	WORD  item_num;						//�б�����ܵ�����
	WORD  whichselect;			        // ��ǰѡ�еڼ���(0��items_per_win)����0��ʼ,
	WORD  firstitem_inwindow;           // �ڴ�������ʾ�ĵ�һ�����ĸ�item.
	struct TITLEBOX *ptitle;                    // �����ؼ�������ΪNULL,����գ���ÿ��ȫ����ӡ��
	VOID * buffer;             //����ַ����Ļ�����λ��,���ڴ�ſɷ�����ڴ���ʼ��ַ
	VOID *orgbuffer;
	struct SCROLL_BOX* pscroll;        //������
	WORD height_peritem;             //ÿ��߶���
	WORD items_per_win;              //��������ʾ������
int  (* showoneitem)(struct LISTBOX* plistbox ,WORD item_index,WORD current_select,DWORD backcolor,DWORD forecolor);

} LISTBOX , *PLISTBOX;


int Listbox_DispatchMsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmessage);
//�б���˳�ʱ��Ҫע�����ͷ��ڴ�
void Listbox_destroy(PLISTBOX plistbox);
//item_per_win: ��������ʾ���У��ڲ���������С�����ڴ�������е��ַ���
int Listbox_init(PLISTBOX plistbox,WORD x,WORD y,WORD width,WORD hight,WORD items_per_win,DWORD buffer_size);
//���һ����Ԫ��item_index��������len���ַ������ȣ��˺������Ƽ�ʹ�ã���Listbox_Addoneline��Listbox_Addoneunit���棩
int Listbox_AddoneColInItem(PLISTBOX plistbox, WORD item_index,VOID  *buff,WORD type,WORD len);
//���һ��
WORD  Listbox_Addoneline(PLISTBOX plistbox);
//�ڵ�ǰ������һ����Ԫ��buff���ַ���(����Ƕ�ؼ�ָ��)�ĵ�ַ�������һ�У������һ������ӵ�Ԫ��
WORD  Listbox_Addoneunit(PLISTBOX plistbox,WORD type,void  *buff);
//���б�������й����У��е����ݷ����仯������ô˺���������ʾ��������������
void Listbox_updateview(PLISTBOX plistbox);
//ɾ�������У���Ϊ��ʼ����֮���޷����ɾ���м���У�������ȫ��ɾ�������������һ��
void Listbox_delallitem(PLISTBOX plistbox);

/*
�б��ʹ��ָ��
��ʼ����
1 ����Listbox_init��ʼ��
2 Listbox_Addoneline�����,Ȼ��Listbox_Addoneunit��ӵ�Ԫ�񣬷������ã�ע��ÿһ�в��ܳ���100���ַ�
3 ����Ҫ����ʼ��SCROLLBOX�ؼ�,����listbox,����listbox->pscroll
4 �����Ҫ����ʼ��titlebox�ؼ�

����������й����У��е����ݷ����仯��
1 Listbox_delallitemɾ��������
2 Listbox_Addoneline�����,Ȼ��Listbox_Addoneunit��ӵ�Ԫ�񣬷�������
3 Listbox_updateview������ʾ
*/

////////////////////////////////////////////////////////////////////////         ��ť�ؼ�
typedef struct BUTTON
{
	 struct CONTROL controlinfo; 	//�ؼ���λ����Ϣ
	 BYTE ifpressed;
	 BYTE clear_background;
     WORD btn_unfocuspcx;
     WORD btn_focuspcx;
     WORD btn_pressedpcx;
	 WORD btn_disablepcx;
     WORD btn_title;
     
     void* pbindctl;
     
} BUTTON ,*PBUTTON,EXTERN_BUTTON ,*PEXTERN_BUTTON;

//presspcx,focuspcx,unfocuspcx�ֱ��Ӧ��ť������״̬
int Button_init(struct BUTTON * pbnctrl,WORD x,WORD y,WORD width,WORD height,
		WORD presspcx, WORD focuspcx, WORD unfocuspcx, WORD title);
void Button_show(struct CONTROL * pCtrl);

//ͨ�ô����������ذ�ť������
void btn_default(PCONTROL pcon,struct DM_MESSAGE* msg);
void  Button_dispatchmsg(struct CONTROL * pbuttonctrl,struct DM_MESSAGE *dmessage);


///////////////////////////////////////////////////////////////////////////  �������ؼ�
typedef struct SCROLL_BOX{
	struct CONTROL controlinfo; 		//�ؼ���λ����Ϣ
  struct BUTTON high_btn;    //���水ť
  struct BUTTON low_btn;     //�ײ���ť
  WORD   move_y;  //�϶��鵱ǰ��y
  WORD   move_height;//�϶���ĸ�
  WORD   range_y;   //������Χ����ʵλ��
  WORD   range_height;//������Χ�ĸ߶�
  PCONTROL pbindctl; //�͹������󶨵Ŀؼ�
  WORD   whichbutton;
  WORD   totalline; //һ������
  WORD   line_perwin; //��������ʾ����
  BYTE   drag_status;
  WORD   drag_begin_y;
} SCROLL_BOX, * PSCROLL_BOX;

#define  SCROLLBOX_WIDTH 16
#define  SCROLLBOX_RANGE_COLOR 0xEEEEEE
#define  SCROLLBOX_LINE_COLOR  0xCAC8C9
#define  SCROLLBOX_DRAGBOX_COLOR	  0xCDCDCD
#define  SCROLLBOX_DRAGBOX_SEL_COLOR  0x606060
#define  SCROLLBOX_DRAGBOX_OUT_COLOR  0x606060
//bindctl: ��Ҫ�͹������󶨵Ŀؼ�
//totalitem: ��ǰ�󶨿ؼ������������Ժ�ɱ仯
//item_per_win: һ����������ʾ���У��̶�����
//���û�ʹ�����������仯�󣬹������ᷢ����Ϣ(WM_SCROLL_UP,WM_SCROLL_DOWN,WM_SCROLL_MOVE )���󶨵Ŀؼ���
//���ؼ���Ҫʹ�������仯������Ҫ������Ϣ(WM_SCROLLBOX_UPDATE)��������
int Scrollbox_init(PSCROLL_BOX pscroll,PCONTROL bindctl,WORD totalitem,WORD item_per_win);


////////////////////////////////////////////////////////////////////�����ؼ������б���
#define TITLEBOX_HEIGHT 26
#define TITLEBOX_BG_COLOR 0XEBE8E1
#define TITLEBOX_GREYLINE_COLOR 0XBEB9B4
#define TITLEBOX_FRAMELINE_COLOR 0x827864

#define TITLEBOX_HIGHLINE_COLOR 0xFAF5F5
#define TITLEBOX_LOWHLINE1_COLOR 0xE7E4DE
#define TITLEBOX_LOWHLINE2_COLOR 0xE2DED8
#define TITLEBOX_LOWHLINE3_COLOR 0xC8D9D3

#define TITLEBOX_LEAST_WIDTH  21

#define RES_DRAG_CURSOR  RES_CURMOVE_PCX 


typedef struct ONECOLUMN {  
	CHAR16 *column_name; //���֣���������ڳ�ʼ��ʱȷ��
	WORD orig_width;  //��ʼ��ȣ���������ڳ�ʼ��ʱȷ��
	WORD begin_x;     //��ʼx���꣬�ڲ�ʹ��
	WORD curr_width;  //��ǰ��ȣ��ڲ�ʹ��
	
} ONECOLUMN, * PONECOLUMN;

typedef struct TITLEBOX
{
	struct CONTROL controlinfo;
	PONECOLUMN column_list;
	WORD column_num;
	PLISTBOX plistbox;
	WORD drag_col;
	BYTE drag_status;
	WORD drag_begin_x;
	BYTE cursor_changed;
	PCURSOR_INFO old_curosr;
} TITLEBOX, *PTITLEBOX;
//cols: ������
//cols_num: һ������
//�б������ȳ�ʼ����Ȼ����ܵ��ô˺���
int Titlebox_init(PTITLEBOX ptitle,PLISTBOX plistbox,WORD width,PONECOLUMN cols,WORD cols_num);


//////////////////////////////////////////////////////////////////////////////// edit box

///////////////editbox
#define EDITBOX_STRING_ADDY 3
#define EDITBOX_CURSOR_ADDY 4
#define EDITBOX_BACKGROUND 0XF5F3F4
//�����Ǳ༭��ģʽ����λ���壨������ϣ�
#define EDIT_MODE_DIGIT 2  //ֻ����������
#define EDIT_MODE_PASSWORD 1 //������ַ�ȫ����ʾ"*"
#define EDIT_MODE_NORMAL 0  //һ��ģʽ

#define EDIT_BEGIN_X 5
#define COLOR_EIDTBOX_FOCUS 0xf13326
#define COLOR_EIDTBOX_UNFOCUS EDITBOX_BACKGROUND

#define EDIT_PIXEL_PER_CHAR  8
typedef struct EDITBOX
{
	  struct CONTROL controlinfo; 	//�ؼ���λ����Ϣ
      CHAR16 * name;					//���name�ַ���
      BYTE type;                    //�ؼ�����
      BYTE ifselected;				//�Ƿ�ѡ��.��һ�ε���ʱѡ��;˫��ʱѡ��
      BYTE lbtnclicknum;			//�������ô���
      BYTE insmode;					//����ģʽ�͸���ģʽ�л�
	  WORD current_len;               //��ǰ�ַ������ȣ�������0;
      WORD num;                     //�ַ�����󳤶�,������0
      WORD posofcursor;             //������ַ����е�λ��
}EDITBOX, *PEDITBOX;

/*
  str: �༭�����ַ����Ļ�����������0
  type: �༭�����ԣ�����
  num: �����������ַ���������0
*/
void EditLincencebox_init(struct EDITBOX * peditbox, WORD x, WORD y, WORD width, WORD hight, CHAR16 * str, BYTE type, WORD num);
void Editbox_init(struct EDITBOX * peditbox,WORD x,WORD y,WORD width,CHAR16 * str,BYTE type,WORD num);
void Edit_dispatchmsg(struct CONTROL * pcontrol,struct DM_MESSAGE * pdmsg);
/////////////////////////////////////////////////////////////////////////////// radio box
struct RADIOBOX;
#define RADIO_BG_COLOR 0XEBE8E1

typedef struct RADIOBOX
{
	  struct CONTROL controlinfo; 	//�ؼ���λ����Ϣ
      CHAR16 * name;					//���name�ַ���
      struct RADIOBOX* next;
      BYTE status;
      WORD radiopcx_sel;
      WORD radiopcx;
      WORD radiopcx_dis;//disable
} RADIOBOX, *PRADIOBOX;
//status: ��ǰ�Ƿ�ѡ������1ѡ�� ��0��ѡ��
//name: ��ʾ�ַ���
//attach: ������radio�ؼ��󶨣�����������radio��1��2��3����ʼ��ʱ1ΪNULL,2��1��3��2��
int Radio_init(PRADIOBOX pradio,WORD x,WORD y,CHAR16 *name,PRADIOBOX attach,BYTE status);
//����û���ǰѡ������ĸ�radio�����������ţ�����ʼ��ʱ�󶨵�˳��
int Radio_get(PRADIOBOX pradio );

/////////////////////////////////////////////////////////////////////////////////// check box
#define CHECK_BG_COLOR 0XEBE8E1
#define DIALOG_BG_COLOR 0XEDE9E3

typedef struct CHECKBOX
{
      struct CONTROL controlinfo; 	//�ؼ���λ����Ϣ
      CHAR16 * name;			//���name�ַ���
   //   struct RADIOBOX* next;
      WORD status;

} CHECKBOX, *PCHECKBOX;
//status: ��ǰ�Ƿ�ѡ������1ѡ�� ��0��ѡ��
//name: ��ʾ�ַ���
int Checkbox_init(PCHECKBOX pcheck,WORD x,WORD y,CHAR16 *name,WORD status);
void Checkbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage);

/////////////////////////////////////////////////////////////////////////////// ������
//��������Ҫ����ʱ��������Ϣ WM_PROCESS_RUN
//�������ص�0��ʱ������  WM_PROCESS_INIT
typedef struct PROCESSBOX
{
	struct CONTROL controlinfo; 	//�ؼ���λ����Ϣ
	WORD length; //�Ѿ����˶�������
} PROCESSBOX, *PPROCESSBOX;

int Processbox_init(PPROCESSBOX pproc,WORD x,WORD y,WORD width);
void Processbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage);
void Processbox_show( PCONTROL pcontrol );

#define COLOR_PROC_FRAME 0x736a5E
#define COLOR_PROC_HIGHLINE 0xc0b8ab
#define COLOR_PROC_FILL 0XCAC1B3

///////////////////////////////////////////////////////////////////////////////�����б��ؼ�

typedef struct COMBOBOX 
{
    struct CONTROL controlinfo; 	//�ؼ���λ����Ϣ
    VOID  *name;                     //���ֶδ�������б���Ԫ�أ�ÿ���ַ�����\0���������ȹ̶���������50
    WORD size_of_item;                  //ÿ��Ԫ�صȳ� && ����Ϊsize_of_item
    WORD  num ;                         //����Ԫ�ظ���
    WORD  whichselect;                   //��ǰѡ������
//    WORD originalw;                     //ԭʼ�༭����
//    WORD originalh;                     //ԭʼ�༭����
//    POINTER  logaddr ;                  //��̬�����ڴ�ĵ�ַ
//    HANDLE   comboret ;                 //��̬�����ڴ�ľ��                
} COMBOBOX , *PCOMBOBOX;

//itemsname: ����ַ����Ļ�������ַ��ÿ���һ���ַ��������ȹ̶���
//itemlen:ÿ��ĳ��ȣ��ֽ�����
//itemnum:һ��������
//currentsel:��ǰѡ�������0XFFFFΪ��
int Combobox_init(struct COMBOBOX * pcomboctrl,WORD x,WORD y,WORD width,VOID  * itemsname,WORD itemlen,WORD  itemnum,WORD curselect);
void  Combobox_DispatchMsg(struct CONTROL* pCombobox,struct DM_MESSAGE * pdmessage);

#define COMBO_COLOR_FRAME  0x827864
#define COMBO_SELECT_COLOR 0x3c82c8


struct DrvButtonList;
typedef struct DRVBOX
{
	struct CONTROL controlinfo; 	//�ؼ���λ����Ϣ
	CHAR16  volname[256];			//���
	CHAR16  capname[50];
	BOOL  ifremove;
	//   struct RADIOBOX* next;
	WORD status;
	struct DrvButtonList * plist;
	WORD indexinlist;
} DRVBOX, *PDRVBOX;
struct DrvButtonList {
	DRVBOX icons[64];
	INTN num;//64 max
	WORD partindex[64];
	INTN selected;//-1 no selected
	PCONTROL bindctl;//listbox

};


//////////////////////////////////////////////////////////////////////////////////

//��ָ���Ŀ���ڴ�ӡ�ַ������粻ȫ����ʾ'...'
void PutstrinWidth(WORD x,WORD y,CHAR16 *str,DWORD forecolor,WORD width);
//����ַ�����ȣ����أ�
WORD GetWidthofString(CHAR16 *str);
BYTE hidemouse();
BYTE showmouse();
BYTE EnableMouse(BYTE bEnable);
BOOL MouseInit();
void MouseRelease();

BOOL IsAltPressed();
BOOL IsCtrlPressed();
BOOL KeyboardInit();
VOID ClearKeyboard();

void extern_control_dispatchmsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmmessage);

void  SelectDisk_Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage);
void sel_disk_extern_control_dispatchmsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmmessage);
void Recover_Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage);
void New_CopyRight_contro_pre_dispatchmsg(struct CONTROL * pcontrol, struct DM_MESSAGE *dmmessage);
void New_CopyRight_contro_Next_dispatchmsg(struct CONTROL * pcontrol, struct DM_MESSAGE *dmmessage);

extern EFI_KEY_DATA gKeyData;	//��һ�ε���getch��õļ�ֵ
extern EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *gpSimpleEx;

#endif
