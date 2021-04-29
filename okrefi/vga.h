/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef _OKR_VGA_H_
#define _OKR_VGA_H_

#include "commondef.h"

#define WINDOW_BK_WIDTH		900		//����ͼƬ�Ĵ�С - ���
#define WINDOW_BK_HEIGHT	546		//����ͼƬ�Ĵ�С - �߶�


#define COLOR_BLACK			0	//	;��ɫ
#define COLOR_RED			0x00FF0000	//	;��ɫ
#define COLOR_GREEN			0x0000FF00	//	;��ɫ
#define COLOR_BLUE			0x000000FF	//  ;��ɫ
#define COLOR_SHADOW_GREY	0x00848284	//  ;��ɫ��Ӱ  
#define COLOR_SHADOW_BLACK	0x00424142	//  ;��ɫ��Ӱ
#define COLOR_GREY			0x00D6D3CE	//  ;��ɫ

#define COLOR_WHITE			0x00FFFFFF	//	;��ɫ

#define COLOR_TRANSPARENT	0xff00ff	//;͸����ɫ
#define COLOR_TRANSPARENT_R	0xff	//;͸����ɫ
#define COLOR_TRANSPARENT_G	0x0		//;͸����ɫ
#define COLOR_TRANSPARENT_B	0xff	//;͸����ɫ

#define COLOR_TITLE_L_BLUE	 0x5B8AD1	// light blue	add by wh
#define COLOR_SEC_BLUE	     0x2F557C   // gray blue	
#define COLOR_CLIENT		 0xE9F1F4	// white color
#define COLOR_BUTTOM_L_BLUE	 0x5B8AD1	// light blue	add by wh

#define COLOR_LIST_ACTIVATE		0xF49005
#define COLOR_LIST_INACTIVATE	0xF49005
#define COLOR_LIST_BACK_WHITE	0xFFFFFF
//#define COLOR_SMOOTH 0x11
//#define TRUECOLOR_TRANSPARENT 0xff00ff

#define COLOR_TEXT			COLOR_WHITE	 
#define COLOR_TEXT_BKG		0x00FEFEFE	// back color of the text	add by wh
#define COLOR_MESSAGEBOX_BK_LINE_DRAK_BLUE 0x2F557C

/*
//�����������״
#define MOUSE_CURSOR_NORMAL		0
#define MOUSE_CURSOR_CROSS		1
#define MOUSE_CURSOR_RUNNING	2
#define MOUSE_CURSOR_EDIT		3
*/
//;bit            description
//;0              left button down / up
//;1              right button down / up
//;2              middle button down / up
//;3              mouse move or not
//;4              double clicked
/*#define MOUSE_LBUTTON_DOWN      1           //;�������
#define MOUSE_RBUTTON_DOWN      2           //;�����Ҽ�
#define MOUSE_MIDDLE_DOWN       4           //;�����м�
#define MOUSE_MOVE              8           //;����ƶ���
#define MOUSE_DOUBLE_CLICKED    0x10        //;˫��
#define MOUSE_LBUTTON_PRESS     0x80        //  ;�������
#define MOUSE_RBUTTON_PRESS     0x81        //  ;�����Ҽ�
#define MOUSE_DRAG              (MOUSE_MOVE | MOUSE_LBUTTON_DOWN)	//       ;��ק
*/
/*//����������ַ�
#define CHAR_ALLMOUSE		0xfffe
#define CHAR_COPYMOUSE		0xfffd   		
#define CHAR_CLOSEICON		0xfffc
#define CHAR_UPARROW   		0xfffb
#define CHAR_DOWNARROW		0xfffa
#define CHAR_LEFTARROW		0xfff9
#define CHAR_RIGHTARROW		0xfff8
#define CHAR_DOTTEDICON		0xfff7
#define CHAR_CHECKBOXICON	0xfff6
#define CHAR_SHORTMOUSE		0xfff0

//define size of picture-elements.
#define TITLE_HEIGHT        22
#define LINE_HEIGHT         20
#define BUTTON_WIDTH        68
#define BUTTON_HEIGHT       24
#define FONT_HEIGHT	        16
#define FONT_WIDTH	        16
#define FONT_ASCII_WIDTH    8
#define LIST_TITLE_HEIGHT   18
#define SCROLL_BAR_WIDTH    16
#define SCROLL_BAR_HEIGHT   16
*/
//;ͼ������
#define BMP_UNKNOWN			0
#define BMP_MONO			1
#define BMP_COLOR16			4
#define BMP_COLOR256		8
#define BMP_COLOR64K		16
#define BMP_TRUECOLOR		24

//

#define VGA_BUFSIZE	0x300000

#pragma pack(1)

//Default PCX file HEAD's format!
//PCX�ĵ�ɫ�����ļ��������256ɫPCX�ļ�Ϊ����������769���ֽ�Ϊ��ɫ���ı�ʶ��
//256ʱ���ֽڱ���Ϊ12��ʣ�µ�768��256*3��Ϊ��ɫ���RGBֵ��
typedef struct {
	BYTE Manufacturer;	//0
	BYTE Version;
	BYTE Encoding;
	BYTE BitsPerPixel;
	WORD XMin,YMin;		//4
	WORD XMax,YMax;		//8,
	WORD HRes;			//0xC,width
	WORD VRes;			//0xE,height
	BYTE Palette[48];	//0x10
	BYTE Reservered;	//0x40
	BYTE ColorPlanes;	//0x41,��ɫ����ɫλƽ������
	WORD BytesPerLine;	//0x42ÿ�У�ÿλƽ����Ҫ���ֽڣ����Կ�ȣ��͵õ���ÿһ���أ�ÿһλƽ�棬�ж���λ����
	WORD PaletteType;	//0x44
	BYTE Filler[58];	//
}PCXHEAD, *PPCXHEAD;

typedef struct tagBITMAPFILEHEADER
{ 
	WORD    bfType; 			//0, "BM"
	DWORD   bfSize; 			//2
	WORD    bfReserved1; 		//6
	WORD    bfReserved2; 		//8
	DWORD   bfOffBits;			//0x0A
	DWORD   biSize;				//0x0E
	DWORD   biWidth; 			//0x12
	DWORD   biHeight; 			//0x16
	WORD    biPlanes; 			//0x1A
	WORD    biBitCount; 		//0x1C
	DWORD   biCompression; 		//0x1E
	DWORD   biSizeImage; 		//0x22
	DWORD   biXPelsPerMeter; 	//0x26
	DWORD   biYPelsPerMeter; 	//0x2A
	DWORD   biClrUsed; 			//0x2E
	DWORD   biClrImportant;		//0x32
}BITMAPFILEHEADER, *PBITMAPFILEHEADER;

//Default resource block HEAD's format!
typedef struct{
	WORD  ResType;          //0 ��Դ����
	WORD  ResHeadSize;      //2 �ṹͷ��С
	WORD  X;                //4 ��Դ���õ����Ͻ�X����
	WORD  Y;                //6 ��Դ���õ����Ͻ�Y����
	WORD  Width;            //4 ���
	WORD  Height;           //6 �߶�
	DWORD Sizes;            //8 ��С
	BYTE  Compress;         //12 �Ƿ�ѹ����ѹ����ʽ,��Ϊ1����Ϊ0��
	BYTE  PaletteType;      //13 ��ɫ�巽ʽ��0=�ڰף�1=�Ҷȷ�ʽ��2=16ɫ��3=���
	WORD  BytesPerLine;     //14 ÿһλƽ���ÿһˮƽ��������ֽ���,
	BYTE  BitsPerPixel;      //16 ÿ��λƽ��ÿ�����ص�λ��   16ɫ
	BYTE  ColorPlanes;      //17 λƽ����                   16ɫ
	BYTE  Reserved[58];     //18 ����         ����16ɫ��ɫ��Ϊ128�ֽ�
	BYTE  PCXPalette[48];   //80 ��ɫ����Ϣ
	                        //128 //�����256/64Kɫͼ�Σ���ResHeadSize����ͬʱ��ɫ����Ϣ
	                        //;�ӳ�����ɫ����Ϣ��׷���������
	                        //;
}RESHEAD, *PRESHEAD;

//Default fileheader of RESOURCE file. 4K bytes������������960����Դ��ͼƬ��
typedef struct{
	DWORD magic;                    //��־ = 0x62330586
	DWORD checksum;                 //�ļ�ͷУ���
	DWORD IndexSize;                //����ռ���ֽ���
	DWORD Filesize;                 //�ļ��ֽ���
	DWORD ItemNumbers;              //��Դ����
	char  reserved[0x100-20];       //����256�ֽ�
	DWORD pEntry[0x3c0];            //ָ��, (4K - 0x100)/4 = 0x3c0
}RES_FILE_HEADER, *PRES_FILE_HEADER;

typedef struct RECT{
	WORD x;
	WORD y;
	WORD width;
	WORD height;
}RECT, *LPRECT;

typedef struct POINT{
        WORD x;
        WORD y;
}POINT, *LPPOINT;

#pragma pack()

//included in vga.c
DWORD IndexToColor(WORD Index, PBYTE Palette);
BOOL VgaInit();
EFI_STATUS VgaRelease();
VOID DrawLine(WORD x, WORD y, WORD Width, WORD Height, DWORD Color);
VOID DrawDotLine(WORD x, WORD y, WORD Width, WORD Height, DWORD Color);
VOID DotLinebox(WORD x, WORD y, WORD Width, WORD Height, DWORD Color);
VOID Linebox(WORD x, WORD y, WORD Width, WORD Height, DWORD Color);
void DrawSinkableRectangle(WORD x, WORD y, WORD width, WORD height, DWORD framecolor, DWORD color);

int DisplayStringInRect(WORD x, WORD y, WORD width,WORD height, DWORD color, CHAR16 *str);
int DisplayStringInWidth(WORD x, WORD y, WORD width, DWORD color, CHAR16 *str);
int DisplayString(WORD x,WORD y, DWORD color,CHAR16 * wstring);
int DisplayStringA(WORD x,WORD y, DWORD color, char * string);
int DisplayChar(WORD x,WORD y, DWORD color,CHAR16 ch);
INTN UnloadResource();
INTN LoadResource();
VOID GetImgEx(WORD x ,WORD y, WORD Width,WORD Height,VOID *buffer);
VOID PutImgEx(WORD x, WORD y, WORD Width, WORD Height,VOID *buffer);
INTN DisplayImg(WORD x, WORD y, DWORD fileid, BOOLEAN transparent);
INTN DisplayImgInRect(WORD x, WORD y, WORD width, WORD height, DWORD fileid, 
		BOOLEAN bCenter);
VOID Disp32 (WORD x,WORD y,DWORD var);
void GetBmpRect(DWORD fileid, RECT *rc);

#define line_x(x,y,width,color) DrawLine(x,y,width,1,color)
#define line_y(x,y,height,color) DrawLine(x,y,1,height,color)
#define FillRect(x, y, width, height, color ) DrawLine(x,y,width,height,color)
#define FillScreenWithColor(color) DrawLine(0,0,g_WindowWidth,g_WindowHeight,color)

#define save_VGA_RAM(x,y,width,height,buf) \
	if(g_vga) {g_vga->Blt(g_vga, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)buf, EfiBltVideoToBltBuffer, x, y, 0, 0, width, height, 0);}

#define restore_VGA_RAM(x,y,width,height,buf)	\
	if(g_vga) {g_vga->Blt(g_vga, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)buf, EfiBltBufferToVideo, 0, 0, x, y, width, height, 0);}

#define IS_VGA_INIT() (g_BitsPerPixel != 0)
VOID DrawLinearRect(DWORD begincolor,DWORD endcolor,WORD x,WORD y,WORD width,WORD height);

VOID DrawRect(DWORD color, WORD x, WORD y, WORD width, WORD height);

void ClearScreen( void );

//void checkres(DWORD fileid, DWORD context);

//void restore_VGA_RAM( WORD x, WORD y, WORD width, WORD height, void * buf);
//void save_VGA_RAM( WORD x, WORD y, WORD width, WORD height, void * buf);
#include <Protocol/GraphicsOutput.h>
#include <Protocol/UgaDraw.h>

extern EFI_GRAPHICS_OUTPUT_PROTOCOL	*g_vga ;

extern WORD	g_WindowWidth ;
extern WORD	g_WindowHeight ;
#endif
