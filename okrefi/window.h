/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef _OKR_WINDOW_H_
#define _OKR_WINDOW_H_


#define WM_NOMESSAGE 0x0000
#define WM_MOUSEMOVE  0xff00           //鼠标移动
#define WM_LEFTBUTTONPRESSED  0xff01   //鼠标左键按下
#define WM_LEFTBUTTONRELEASE  0xff02   //鼠标左键弹起
#define WM_RIGHTBUTTONPRESSED  0xff03  //鼠标右键按下
#define WM_RIGHTBUTTONRELEASE  0xff04  //鼠标右键弹起
//#define WM_DOUBLECLICKED  0xff05       //左键双击，此消息之前已经有左键按下，弹起消息
#define WM_MOUSEDRAG  0xff06           //鼠标左键按下，并移动


// 以下是自定义消息
#define  WM_MOUSEMOVEOUT 0xfe30  //鼠标移到出控件外，当前鼠标x,y坐标分别在thex,they中
#define  WM_MOUSEMOVEIN 0xfe31  //鼠标移到控件内，x,y坐标分别在thex,they中
#define  WM_CTL_ACTIVATE 0xfe32 //当控件激活时，WINDOW发消息给控件
#define  WM_CTL_INACTIVATE 0xfe33 //从激活转成不激活，WINDOW发消息给控件

#define  WM_BUTTON_CLICK   0XFE34 //在按钮上点击鼠标，或者按空格，回车，产生此消息，此消息由BUTTON控件转换

#define  WM_SCROLL_UP   0xfe35  //用户点击滚动条的上按钮，此消息由滚动条发给绑定控件
#define  WM_SCROLL_DOWN  0XFE36 //用户点击滚动条的上按钮，此消息由滚动条发给绑定控件
#define  WM_SCROLL_MOVE  0XFE37 //用户拖动滚动条，滚动条发送给控件,thex=起始行数，此消息由滚动条发给绑定控件

#define  WM_SCROLLBOX_UPDATE 0XFE38  //滚动条控件处理此消息,更新滚动条,THEX=起始行数,they总行数。

#define  WM_PROCESS_RUN      0XFE39  //进度条控件处理此消息,更新进度条，thex=百分之几，范围0～100
#define  WM_PROCESS_INIT     0XFE3a  //进度条控件处理此消息,进度归0

#define  WM_CTL_DISABLE      0xfe3B // 当需要禁用某个控件时，调用Control_diable函数，函数发生消息给被禁用的控件
#define  WM_CTL_ENABLE      0xfe3C // 重新启用控件，流程同上

#define  WM_TIMEOUT	    0XFE3D

#define WM_CONTEXT_CHANGE 0XFE3E   //COMBOBOX,或CHECKBOX的内容发生变化，这些内容变化可由外部改变，也可用户操作后自身填入。
#define WM_LISTBOX_SELECT 0XFE3F  // LISTBOX 用户按ENTER,SPACE,双击，会产生此消息。
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

#define IMG_FILE_PCX		0	//pcx格式
#define IMG_FILE_BMP		1	//没有压缩的bmp格式
#define IMG_FILE_JPG		2	//jpg格式
#define IMG_FILE_LZ77		3	//压缩的图形
#define IMG_FILE_PCX_LZ77	4	//压缩的pcx
#define IMG_FILE_BMP_LZ77	5	//压缩的bmp
#define IMG_FILE_JPG_LZ77	6	//压缩的jpg
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
	VOID	*pdata;		//解压后的数据
} IMAGE_FILE_INFO, *PIMAGE_FILE_INFO;


/////////////////////////////////////////////////////////////////////////////////////

typedef struct CURSOR_INFO
{
	DWORD pcx;
	//图片高和宽
	WORD width; 
	WORD height;
	//鼠标焦点在图片的哪个位置
	WORD focus_x; //x位置
	WORD focus_y;//y位置
} CURSOR_INFO ,*PCURSOR_INFO;

/*
struct M_MESSAGE {
 WORD    TheX  ;
 WORD    TheY  ;
 WORD    MESSAGE ;
};
*/
//消息结构
struct DM_MESSAGE
{
	INT32 thex;					//鼠标x位置
	INT32 they;					//鼠标y位置
	WORD message;			//鼠标和键盘消息，如果是鼠标消息见上，如果是键盘消息，则是键盘的虚拟键值
};


struct CONTROL;
//是否键盘消息
int ifkeymsg(struct DM_MESSAGE *pdmmessage);
//是否鼠标消息 
int ifmousemsg(struct DM_MESSAGE *pdmmessage);
//判断当前鼠标消息处于哪个控件内
WORD detectwhere(struct CONTROL * p[],WORD totalnum,struct DM_MESSAGE *msg);
void nop();
void detectmessage(struct DM_MESSAGE *,EFI_SIMPLE_POINTER_PROTOCOL *mouse);
////////////////////////////////////////////////////////////////////////////// 通用控件结构
/*
   每种控件都有一个CONTROL的结构变量，并且必须置于控件结构的头部，类似于c++继承关系，CONTROL结构
相当于基类,每个控件都从基类继承。
   CONTROL结构具有控件基本属性：
   1 位置信息，大小信息(x,y,w,h)，用于判断鼠标消息应该发送到哪个控件；
   2 状态信息(status),判断控件是否支持TAB键，是否已经禁用
   3 window指针，记录控件所属的WINDOW
   4 消息处理函数指针sendmessage，用于处理发给控件的消息(包括键盘鼠标,各种自定义消息)；类似于win sdk的窗口处理函数。
   控件使用者，自定义处理函数，类似于c++对象重载。
   5 控件显示函数指针，此变量其实可以合并到sendmessage函数中，只需要添加一个WM_SHOW消息即可，这个函数用于根据当前的
   控件状态，完整的显示控件外观。此函数并不是仅仅在初始化时调用，当需要更新控件显示时就会调用。此函数显示的控件状态
   不是控件的激活状态。
   
   
*/

// STATUS 按位定义，例如bit0=1表示TABNOTSTOP;BIT1=1表示DISABLE
#define CON_STATUS_TABNOTSTOP		1  //控件不支持激活状态，tab键不停留；比如滚动条。
#define CON_STATUS_DISABLE			2  //控件已经禁用，不响应任何消息。
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
	
	BYTE thez;					//z坐标
	BYTE type; 					//控件类型
	BYTE status;        		//控件状态，按位定义
	BYTE state;					//控件当前状态，三种：focus，pressed，normal
	
	WORD control_index;         //控件索引号（在window结构中）
	WORD control_id;			//控件id，返回值
	struct WINDOW *pwin;        //指向控件所属的window窗体,由窗口初始化时填入。
	void (* sendmessage)(struct CONTROL *, struct DM_MESSAGE *);
	void (* show)(struct CONTROL *);  //显示
	
} CONTROL, * PCONTROL;

//控件基本类型

#define IDC_BUTTON  1
#define IDC_LISTBOX 2
#define IDC_SCROLLBOX 3
#define IDC_TITLEBOX 4
#define IDC_EDITBOX 5
#define IDC_RADIOBOX 6
#define IDC_CHECKBOX 7
#define IDC_COMBOBOX 8
#define IDC_PROCESSBOX 9
//封装函数，对控件发送消息
void Control_sendmsg(PCONTROL pctrl,struct DM_MESSAGE *dmmessage);
//禁用控件
int Control_disable(PCONTROL pctl);
//启用控件
int Control_enable(PCONTROL pctl);

//int Control_ShowUI(PCONTROL pctl);
// 
//int Control_HideUI(PCONTROL pctl);


////////////////////////////////////////////////////////////////////////////////// WINDOW结构
/* 
    WINDOW是多个CONTROL的容器(container),其行为类似于CONTROL,核心功能仍是消息处理。所不同的是window
 处理整体界面中消息。他负责将消息分发到每个子控件，或者直接忽略。另外还处理一些默认消息，例如TAB键，
 绘制一些固定的元素，比如背景。
    使用者还可以定制消息处理函数msgfunc，去响应一些特殊消息（例如热键）
    window结构中，有几个变量对外开放：
    current: 当前焦点控件；
    ret_data: 整个window的返回值，由Window_exit()赋值。
    msgfunc： 消息处理函数
    show:     功能同control结构的show,显示窗口的所有元素，不止在初始化时调用。
    control_list: 每个子控件的指针。
    
WINDOW使用流程：
    1. WINDOW中各个CONTROL初始化。
    2. Window_init,需要时自定义消息处理函数msgfunc
    3. Window_addctl，添加各个子控件
    4. Window_run,进入消息处理
    5. 当控件调用Window_exit后，window退出，判断Window_run的返回值。
*/
//每个window最多显示30个控件
#define MAX_CONTRL_INWIN  30
typedef struct WINDOW
{
	PCONTROL control_list[MAX_CONTRL_INWIN]; //控件数组,最多30个控件
	WORD control_num;
	void (* show)(struct WINDOW *);  //显示
	
//	struct MSG_CALLBACK **msg_callback; //自定义消息处理数组
//	WORD msg_callback_num;
	WORD current; //当前控件，消息发送控键，
	WORD precurrent; //前一个控件
	WORD current_cursor;
	BYTE ifexit;
	void  (*msgfunc )(struct WINDOW *, struct DM_MESSAGE *); //消息处理函数
	
	DWORD ret_data; //记录整个界面的返回值

	WORD selected; //当前控件，消息发送控键，

} WINDOW , *PWINDOW;

//只显示window中包含的所有控件
void Window_showctls(PWINDOW pwin);
//初始化window结构，可指定show函数指针，如果=NULL,使用默认显示函数
//用户如果需要自定义消息处理，可更改msgfunc函数指针
int Window_init(PWINDOW pwin,void (* show)(struct WINDOW *));
//默认的显示函数，显示背景和所有控件
void Window_show(PWINDOW pwin);
//控件调用此函数，让整个WINDOW退出，RET=window的返回值
int Window_exit(PWINDOW pwin,DWORD ret);
//进入window的消息循环，直至控件调用Window_exit
DWORD Window_run(PWINDOW pwin);
//添加一个控件，返回控件索引号,当window已经开始运行时，不可添加
int Window_addctl(PWINDOW pwin,PCONTROL ctl);
//window的默认消息处理函数
void  Window_Dispatchmsg(PWINDOW pwin,struct DM_MESSAGE *dmmessage);


/////////////////////////////////////////////////////////////////////////////////  列表框控件
//列表框的一行对应一个item； 一列对应一个column;
//每个单元格对应一个unit;

//以下是每个unit的类型，Listbox_Addoneunit时指定，按位定义（可组合）
#define ITEM_TYPE_CHAR_STRING  1 //bit0=1,普通字符串
#define ITEM_TYPE_NOCOLUMN     2 //BIT1=1,一行只有一个unit,不受titlebox控件控制,特殊字体
#define ITEM_TYPE_ISCONTROL    4 //此单元内嵌一个控件
#define ITEM_TYPE_IMG 8  //图片
//colors
#define LISTBOX_SELLINE_COLOR 0X3C82C8
#define LISTBOX_SELLINE_COLOR_INAC 0XC0C0C0
#define LISTBOX_LINE_COLOR 0x827864
#define COLOR_LISTBOX_SPECIAL 0Xffa000
//ONEITEM描述一行
//一行最多200byte
typedef struct ONEITEM {
//item_buff这样一个缓冲区:
//	类型(2byte),长度(2byte),内容.../类型(2byte),长度(2byte),内容...
	VOID * item_buff; 
	WORD    columns;  //一共几个列
} ONEITEM, * PONEITEM;

struct SCROLL_BOX ;
struct PTITLEBOX;
struct LISTBOX;
typedef struct LISTBOX
{
	struct CONTROL controlinfo; 		//控件的位置信息
	struct ONEITEM itemlist[70];        //一行一个item,一个item用一个结构表示，目前最多50行，以后这个数组会动态分配。
	// 整个行的长度不能超过100个字节。//TODO 
	WORD  item_num;						//列表框里总的行数
	WORD  whichselect;			        // 当前选中第几行(0～items_per_win)，从0开始,
	WORD  firstitem_inwindow;           // 在窗口中显示的第一行是哪个item.
	struct TITLEBOX *ptitle;                    // 标题框控件，可以为NULL,如果空，则每行全部打印。
	VOID * buffer;             //存放字符串的缓冲区位置,用于存放可分配的内存起始地址
	VOID *orgbuffer;
	struct SCROLL_BOX* pscroll;        //滚动条
	WORD height_peritem;             //每项高多少
	WORD items_per_win;              //窗口能显示多少行
int  (* showoneitem)(struct LISTBOX* plistbox ,WORD item_index,WORD current_select,DWORD backcolor,DWORD forecolor);

} LISTBOX , *PLISTBOX;


int Listbox_DispatchMsg(struct CONTROL * pcontrol ,struct DM_MESSAGE *dmessage);
//列表框退出时需要注销，释放内存
void Listbox_destroy(PLISTBOX plistbox);
//item_per_win: 窗口能显示几行，内部缓存区大小，用于存放所有行的字符串
int Listbox_init(PLISTBOX plistbox,WORD x,WORD y,WORD width,WORD hight,WORD items_per_win,DWORD buffer_size);
//添加一个单元，item_index是行数，len是字符串长度（此函数不推荐使用，用Listbox_Addoneline，Listbox_Addoneunit代替）
int Listbox_AddoneColInItem(PLISTBOX plistbox, WORD item_index,VOID  *buff,WORD type,WORD len);
//添加一行
WORD  Listbox_Addoneline(PLISTBOX plistbox);
//在当前行里，添加一个单元格，buff是字符串(或内嵌控件指针)的地址。先添加一行，再添加一个个添加单元格
WORD  Listbox_Addoneunit(PLISTBOX plistbox,WORD type,void  *buff);
//当列表框在运行过程中，行的内容发生变化，需调用此函数更新显示（包含滚动条）
void Listbox_updateview(PLISTBOX plistbox);
//删除所有行，因为初始化完之后，无法添加删除中间的行；必须先全部删除，再重新添加一遍
void Listbox_delallitem(PLISTBOX plistbox);

/*
列表框使用指南
初始化：
1 调用Listbox_init初始化
2 Listbox_Addoneline添加行,然后Listbox_Addoneunit添加单元格，反复调用，注意每一行不能超过100个字符
3 如需要，初始化SCROLLBOX控件,并绑定listbox,设置listbox->pscroll
4 如果需要，初始化titlebox控件

如果程序运行过程中，行的内容发生变化：
1 Listbox_delallitem删除所有行
2 Listbox_Addoneline添加行,然后Listbox_Addoneunit添加单元格，反复调用
3 Listbox_updateview更新显示
*/

////////////////////////////////////////////////////////////////////////         按钮控件
typedef struct BUTTON
{
	 struct CONTROL controlinfo; 	//控件的位置信息
	 BYTE ifpressed;
	 BYTE clear_background;
     WORD btn_unfocuspcx;
     WORD btn_focuspcx;
     WORD btn_pressedpcx;
	 WORD btn_disablepcx;
     WORD btn_title;
     
     void* pbindctl;
     
} BUTTON ,*PBUTTON,EXTERN_BUTTON ,*PEXTERN_BUTTON;

//presspcx,focuspcx,unfocuspcx分别对应按钮的三种状态
int Button_init(struct BUTTON * pbnctrl,WORD x,WORD y,WORD width,WORD height,
		WORD presspcx, WORD focuspcx, WORD unfocuspcx, WORD title);
void Button_show(struct CONTROL * pCtrl);

//通用处理函数，返回按钮索引号
void btn_default(PCONTROL pcon,struct DM_MESSAGE* msg);
void  Button_dispatchmsg(struct CONTROL * pbuttonctrl,struct DM_MESSAGE *dmessage);


///////////////////////////////////////////////////////////////////////////  滚动条控件
typedef struct SCROLL_BOX{
	struct CONTROL controlinfo; 		//控件的位置信息
  struct BUTTON high_btn;    //上面按钮
  struct BUTTON low_btn;     //底部按钮
  WORD   move_y;  //拖动块当前的y
  WORD   move_height;//拖动块的高
  WORD   range_y;   //滚动范围的其实位置
  WORD   range_height;//滚动范围的高度
  PCONTROL pbindctl; //和滚动条绑定的控件
  WORD   whichbutton;
  WORD   totalline; //一共几行
  WORD   line_perwin; //窗口能显示几行
  BYTE   drag_status;
  WORD   drag_begin_y;
} SCROLL_BOX, * PSCROLL_BOX;

#define  SCROLLBOX_WIDTH 16
#define  SCROLLBOX_RANGE_COLOR 0xEEEEEE
#define  SCROLLBOX_LINE_COLOR  0xCAC8C9
#define  SCROLLBOX_DRAGBOX_COLOR	  0xCDCDCD
#define  SCROLLBOX_DRAGBOX_SEL_COLOR  0x606060
#define  SCROLLBOX_DRAGBOX_OUT_COLOR  0x606060
//bindctl: 需要和滚动条绑定的控件
//totalitem: 当前绑定控件的总行数，以后可变化
//item_per_win: 一个窗口能显示几行，固定不变
//当用户使滚动条发生变化后，滚动条会发送消息(WM_SCROLL_UP,WM_SCROLL_DOWN,WM_SCROLL_MOVE )给绑定的控件。
//而控件需要使滚动条变化，则需要发送消息(WM_SCROLLBOX_UPDATE)给滚动条
int Scrollbox_init(PSCROLL_BOX pscroll,PCONTROL bindctl,WORD totalitem,WORD item_per_win);


////////////////////////////////////////////////////////////////////标题框控件，和列表框绑定
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
	CHAR16 *column_name; //名字，此项必须在初始化时确定
	WORD orig_width;  //初始宽度，此项必须在初始化时确定
	WORD begin_x;     //起始x坐标，内部使用
	WORD curr_width;  //当前宽度，内部使用
	
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
//cols: 列数组
//cols_num: 一共几列
//列表框必须先初始化，然后才能调用此函数
int Titlebox_init(PTITLEBOX ptitle,PLISTBOX plistbox,WORD width,PONECOLUMN cols,WORD cols_num);


//////////////////////////////////////////////////////////////////////////////// edit box

///////////////editbox
#define EDITBOX_STRING_ADDY 3
#define EDITBOX_CURSOR_ADDY 4
#define EDITBOX_BACKGROUND 0XF5F3F4
//以下是编辑框模式，按位定义（可以组合）
#define EDIT_MODE_DIGIT 2  //只能输入数字
#define EDIT_MODE_PASSWORD 1 //输入的字符全部显示"*"
#define EDIT_MODE_NORMAL 0  //一般模式

#define EDIT_BEGIN_X 5
#define COLOR_EIDTBOX_FOCUS 0xf13326
#define COLOR_EIDTBOX_UNFOCUS EDITBOX_BACKGROUND

#define EDIT_PIXEL_PER_CHAR  8
typedef struct EDITBOX
{
	  struct CONTROL controlinfo; 	//控件的位置信息
      CHAR16 * name;					//存放name字符串
      BYTE type;                    //控间类型
      BYTE ifselected;				//是否被选中.第一次单击时选中;双击时选中
      BYTE lbtnclicknum;			//左键点击得次数
      BYTE insmode;					//插入模式和覆盖模式切换
	  WORD current_len;               //当前字符串长度，不包含0;
      WORD num;                     //字符串最大长度,不包含0
      WORD posofcursor;             //光标在字符串中的位置
}EDITBOX, *PEDITBOX;

/*
  str: 编辑框存放字符串的缓存区，包含0
  type: 编辑框属性，见上
  num: 最多输入多少字符，不包含0
*/
void EditLincencebox_init(struct EDITBOX * peditbox, WORD x, WORD y, WORD width, WORD hight, CHAR16 * str, BYTE type, WORD num);
void Editbox_init(struct EDITBOX * peditbox,WORD x,WORD y,WORD width,CHAR16 * str,BYTE type,WORD num);
void Edit_dispatchmsg(struct CONTROL * pcontrol,struct DM_MESSAGE * pdmsg);
/////////////////////////////////////////////////////////////////////////////// radio box
struct RADIOBOX;
#define RADIO_BG_COLOR 0XEBE8E1

typedef struct RADIOBOX
{
	  struct CONTROL controlinfo; 	//控件的位置信息
      CHAR16 * name;					//存放name字符串
      struct RADIOBOX* next;
      BYTE status;
      WORD radiopcx_sel;
      WORD radiopcx;
      WORD radiopcx_dis;//disable
} RADIOBOX, *PRADIOBOX;
//status: 当前是否选定；＝1选定 ＝0不选定
//name: 显示字符串
//attach: 和其他radio控件绑定，比如有三个radio：1，2，3，初始化时1为NULL,2绑定1，3绑定2；
int Radio_init(PRADIOBOX pradio,WORD x,WORD y,CHAR16 *name,PRADIOBOX attach,BYTE status);
//获得用户当前选择的是哪个radio，返回索引号（即初始化时绑定的顺序）
int Radio_get(PRADIOBOX pradio );

/////////////////////////////////////////////////////////////////////////////////// check box
#define CHECK_BG_COLOR 0XEBE8E1
#define DIALOG_BG_COLOR 0XEDE9E3

typedef struct CHECKBOX
{
      struct CONTROL controlinfo; 	//控件的位置信息
      CHAR16 * name;			//存放name字符串
   //   struct RADIOBOX* next;
      WORD status;

} CHECKBOX, *PCHECKBOX;
//status: 当前是否选定；＝1选定 ＝0不选定
//name: 显示字符串
int Checkbox_init(PCHECKBOX pcheck,WORD x,WORD y,CHAR16 *name,WORD status);
void Checkbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage);

/////////////////////////////////////////////////////////////////////////////// 进度条
//进度条需要滚动时，发送消息 WM_PROCESS_RUN
//进度条回到0％时，发送  WM_PROCESS_INIT
typedef struct PROCESSBOX
{
	struct CONTROL controlinfo; 	//控件的位置信息
	WORD length; //已经跑了多少像素
} PROCESSBOX, *PPROCESSBOX;

int Processbox_init(PPROCESSBOX pproc,WORD x,WORD y,WORD width);
void Processbox_dispatchmsg(PCONTROL pcontrol,struct DM_MESSAGE *dmessage);
void Processbox_show( PCONTROL pcontrol );

#define COLOR_PROC_FRAME 0x736a5E
#define COLOR_PROC_HIGHLINE 0xc0b8ab
#define COLOR_PROC_FILL 0XCAC1B3

///////////////////////////////////////////////////////////////////////////////下拉列表框控件

typedef struct COMBOBOX 
{
    struct CONTROL controlinfo; 	//控件的位置信息
    VOID  *name;                     //该字段存放下拉列表框的元素，每个字符串以\0结束，长度固定，不超过50
    WORD size_of_item;                  //每个元素等长 && 长度为size_of_item
    WORD  num ;                         //数组元素个数
    WORD  whichselect;                   //当前选中哪项
//    WORD originalw;                     //原始编辑框宽度
//    WORD originalh;                     //原始编辑框宽度
//    POINTER  logaddr ;                  //动态分配内存的地址
//    HANDLE   comboret ;                 //动态分配内存的句柄                
} COMBOBOX , *PCOMBOBOX;

//itemsname: 存放字符串的缓存区地址，每项都是一个字符串，长度固定；
//itemlen:每项的长度（字节数）
//itemnum:一共多少项
//currentsel:当前选择哪项，＝0XFFFF为空
int Combobox_init(struct COMBOBOX * pcomboctrl,WORD x,WORD y,WORD width,VOID  * itemsname,WORD itemlen,WORD  itemnum,WORD curselect);
void  Combobox_DispatchMsg(struct CONTROL* pCombobox,struct DM_MESSAGE * pdmessage);

#define COMBO_COLOR_FRAME  0x827864
#define COMBO_SELECT_COLOR 0x3c82c8


struct DrvButtonList;
typedef struct DRVBOX
{
	struct CONTROL controlinfo; 	//控件的位置信息
	CHAR16  volname[256];			//存放
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

//在指定的宽度内打印字符串，如不全则显示'...'
void PutstrinWidth(WORD x,WORD y,CHAR16 *str,DWORD forecolor,WORD width);
//获得字符串宽度（象素）
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

extern EFI_KEY_DATA gKeyData;	//上一次调用getch获得的键值
extern EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *gpSimpleEx;

#endif
