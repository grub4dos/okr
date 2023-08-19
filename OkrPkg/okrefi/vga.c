/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

//vga.c about the monitor display method 
#include "okr.h"

//#include "fonts.h"
#include "res.h"
#include "faceUIDefine.h"

#include "./font_resource/hei-9.h"
#include "./font_resource/hei-11.h"
#include "./font_resource/hei-12.h"
#include "./font_resource/hei-13.h"
#include "./font_resource/hei-14.h"
#include "./font_resource/hei-15.h"
#include "./font_resource/hei-17.h"
#include "./font_resource/hei-19.h"
#include "./font_resource/hei-20.h"
#include "./font_resource/hei-23.h"
#include "./font_resource/hei-26.h"
#include "./font_resource/hei-27.h"
#include "./font_resource/hei-28.h"
#include "./font_resource/hei-30.h"
#include "./font_resource/hei-34.h"
#include "./font_resource/hei-40.h"

#include "./font_resource/zhong-hei-9.h"
#include "./font_resource/zhong-hei-11.h"
#include "./font_resource/zhong-hei-12.h"
#include "./font_resource/zhong-hei-13.h"
#include "./font_resource/zhong-hei-14.h"
#include "./font_resource/zhong-hei-17.h"
#include "./font_resource/zhong-hei-18.h"
#include "./font_resource/zhong-hei-20.h"
#include "./font_resource/zhong-hei-21.h"
#include "./font_resource/zhong-hei-23.h"
#include "./font_resource/zhong-hei-25.h"
#include "./font_resource/zhong-hei-27.h"
#include "./font_resource/zhong-hei-30.h"
#include "./font_resource/zhong-hei-32.h"
#include "./font_resource/zhong-hei-33.h"
#include "./font_resource/zhong-hei-38.h"
#include "./font_resource/zhong-hei-40.h"
#include "./font_resource/zhong-hei-45.h"
#include "./font_resource/zhong-hei-50.h"
#include "./font_resource/zhong-hei-60.h"
#include "./font_resource/zhong-hei-90.h"

extern WORD  g_x_begin;	//窗口的x坐标
extern WORD  g_y_begin;	//窗口的y坐标

EFI_GUID gEfiConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;

//EFI_GUID_STRING (&gEfiConsoleControlProtocolGuid, "Console Control Protocol", "Console Control Protocol");
EFI_GRAPHICS_OUTPUT_PROTOCOL	*g_vga = NULL;
EFI_CONSOLE_CONTROL_PROTOCOL    *g_Console = NULL;
BYTE							*g_vgabuffer = NULL;	//用于临时存储显存数据的，大小为64K，超过64K
DWORD	g_VgaModeNumber = 0;
WORD	g_WindowWidth = 0;
WORD	g_WindowHeight = 0;
//BYTE 	g_BackGroundColor = COLOR_TRANSPARENCE;
BYTE    g_BitsPerPixel = 0;
DWORD	g_textBKColor = 0xffffff;

/*
const BYTE g_Palette16[48] = {
	0,   0,   0,		//0,color_black, 从240开始的颜色保留给系统使用
	255, 0,   0,		//1,color_red
	0,   255, 0,		//2,color_green
	0,   0,   255,		//3,color_blue

	132, 130, 132,		//4,color_shadow_grey
	66,  65,  66,		//5,color_shadow_black
	214, 211, 206,		//6,color_grey
	57,  109, 165,		//7,color_background

	140, 134, 140,		//8,color_title_grey, 灰色标题
	16,  40,  115,		//9,color_title_blue，蓝色标题
	234, 234, 234,		//a,color_scroll	  滚动条
	192, 192, 0,		//b,

	49,  98,  142,		//c,
	255, 0,   255,		//d
	255, 255, 0,		//e
	255, 255, 255,		//f,color_white


};
*/
#define BITMAP_SIZE 16
//24*24bit
#pragma pack(1)
 
//TODO

typedef struct {
    WORD encoding;
    BYTE width;
    BYTE bitmap[BITMAP_SIZE*BITMAP_SIZE/8];
} CHARBITMAP;

typedef struct stCharData
{
	WORD usEncoding;
	//WORD usBitmapDataSize;
	BYTE ucarrBitmap[1];
}CHAR_DATA, *PCHAR_DATA;

typedef struct stDataHeader
{
	BYTE ucFontType;
	BYTE ucReserved;
	WORD usDataCount;
	BYTE ucAreaSize;			//height is the same as width.
	BYTE ucFontSize;
} DATA_HEADER, *PDATA_HEADER;


#define BITMAP_SIZE_88 96
typedef struct {
	WORD encoding;
	BYTE width;
	BYTE bitmap[1];
} CHARBITMAPEx;

#pragma pack()

//字库文件在内存中的指针
CHARBITMAP * g_font_bitmap = NULL;
INTN  g_font_bitmap_size = 0;

CHARBITMAPEx * g_font_bitmap_ex = NULL;
INTN  g_font_bitmap_size_ex = 0;

FontTextData g_font_1_9; 
FontTextData g_font_1_11; 
FontTextData g_font_1_12; 
FontTextData g_font_1_13; 
FontTextData g_font_1_14; 
FontTextData g_font_1_15;
FontTextData g_font_1_17;
FontTextData g_font_1_19;
FontTextData g_font_1_20;
FontTextData g_font_1_23;
FontTextData g_font_1_26;
FontTextData g_font_1_27;
FontTextData g_font_1_28;
FontTextData g_font_1_30;
FontTextData g_font_1_34;
FontTextData g_font_1_40;

FontTextData g_font_2_9;
FontTextData g_font_2_11;
FontTextData g_font_2_12;
FontTextData g_font_2_13;
FontTextData g_font_2_14;
FontTextData g_font_2_17;
FontTextData g_font_2_18;
FontTextData g_font_2_20;
FontTextData g_font_2_21;
FontTextData g_font_2_23;
FontTextData g_font_2_25;
FontTextData g_font_2_27;
FontTextData g_font_2_30;
FontTextData g_font_2_32;
FontTextData g_font_2_33;
FontTextData g_font_2_38;
FontTextData g_font_2_40;
FontTextData g_font_2_45;
FontTextData g_font_2_50;
FontTextData g_font_2_60;
FontTextData g_font_2_90;

FontTextConfig fontTextConfig[] =
{
	{ FZLTH_9, sizeof(FZLTH_9) / sizeof(unsigned char), L"", &g_font_1_9 },
	{ FZLTH_11, sizeof(FZLTH_11) / sizeof(unsigned char), L"", &g_font_1_11 },
	{ FZLTH_12, sizeof(FZLTH_12) / sizeof(unsigned char), L"", &g_font_1_12 },
	{ FZLTH_13, sizeof(FZLTH_13) / sizeof(unsigned char), L"", &g_font_1_13 },
	{ FZLTH_14, sizeof(FZLTH_14) / sizeof(unsigned char), L"", &g_font_1_14 },
	{ FZLTH_15, sizeof(FZLTH_15) / sizeof(unsigned char), L"", &g_font_1_15 },
	{ FZLTH_17, sizeof(FZLTH_17) / sizeof(unsigned char), L"", &g_font_1_17 },
	{ FZLTH_19, sizeof(FZLTH_19) / sizeof(unsigned char), L"", &g_font_1_19 },
	{ FZLTH_20, sizeof(FZLTH_20) / sizeof(unsigned char), L"", &g_font_1_20 },
	{ FZLTH_23, sizeof(FZLTH_23) / sizeof(unsigned char), L"", &g_font_1_23 },
	{ FZLTH_26, sizeof(FZLTH_26) / sizeof(unsigned char), L"", &g_font_1_26 },
	{ FZLTH_27, sizeof(FZLTH_27) / sizeof(unsigned char), L"", &g_font_1_27 },
	{ FZLTH_28, sizeof(FZLTH_28) / sizeof(unsigned char), L"", &g_font_1_28 },
	{ FZLTH_30, sizeof(FZLTH_30) / sizeof(unsigned char), L"", &g_font_1_30 },
	{ FZLTH_34, sizeof(FZLTH_34) / sizeof(unsigned char), L"", &g_font_1_34 },
	{ FZLTH_40, sizeof(FZLTH_40) / sizeof(unsigned char), L"", &g_font_1_40 },


	{ FZLTZH_9, sizeof(FZLTZH_9) / sizeof(unsigned char), L"", &g_font_2_9 },
	{ FZLTZH_11, sizeof(FZLTZH_11) / sizeof(unsigned char), L"", &g_font_2_11 },
	{ FZLTZH_12, sizeof(FZLTZH_12) / sizeof(unsigned char), L"", &g_font_2_12 },
	{ FZLTZH_13, sizeof(FZLTZH_13) / sizeof(unsigned char), L"", &g_font_2_13 },
	{ FZLTZH_14, sizeof(FZLTZH_14) / sizeof(unsigned char), L"", &g_font_2_14 },
	{ FZLTZH_17, sizeof(FZLTZH_17) / sizeof(unsigned char), L"", &g_font_2_17 },
	{ FZLTZH_18, sizeof(FZLTZH_18) / sizeof(unsigned char), L"", &g_font_2_18 },
	{ FZLTZH_20, sizeof(FZLTZH_20) / sizeof(unsigned char), L"", &g_font_2_20 },
	{ FZLTZH_21, sizeof(FZLTZH_21) / sizeof(unsigned char), L"", &g_font_2_21 },
	{ FZLTZH_23, sizeof(FZLTZH_23) / sizeof(unsigned char), L"", &g_font_2_23 },
	{ FZLTZH_25, sizeof(FZLTZH_25) / sizeof(unsigned char), L"", &g_font_2_25 },
	{ FZLTZH_27, sizeof(FZLTZH_27) / sizeof(unsigned char), L"", &g_font_2_27 },
	{ FZLTZH_30, sizeof(FZLTZH_30) / sizeof(unsigned char), L"", &g_font_2_30 },
	{ FZLTZH_32, sizeof(FZLTZH_32) / sizeof(unsigned char), L"", &g_font_2_32 },
	{ FZLTZH_33, sizeof(FZLTZH_33) / sizeof(unsigned char), L"", &g_font_2_33 },
	{ FZLTZH_38, sizeof(FZLTZH_38) / sizeof(unsigned char), L"", &g_font_2_38 },
	{ FZLTZH_40, sizeof(FZLTZH_40) / sizeof(unsigned char), L"", &g_font_2_40 },
	{ FZLTZH_45, sizeof(FZLTZH_45) / sizeof(unsigned char), L"", &g_font_2_45 },
	{ FZLTZH_50, sizeof(FZLTZH_50) / sizeof(unsigned char), L"", &g_font_2_50 },
	{ FZLTZH_60, sizeof(FZLTZH_60) / sizeof(unsigned char), L"", &g_font_2_60 },
	{ FZLTZH_90, sizeof(FZLTZH_90) / sizeof(unsigned char), L"", &g_font_2_90 },
};

GraphicsInfo g_supportGraphics[] =
{
	{3840, 2160, -1},
	{2560, 1440, -1},
	{1920, 1080, -1},
	{1680, 1050, -1},
	{1400, 1050, -1},
	{1280, 1024, -1},
	{1440, 900, -1},
	{1024, 768, -1},
};

//GraphicsInfo * g_pmachienGraphics = NULL;

//DATA_HEADER *g_pDataHeader = NULL;
//CHAR_DATA * g_pCharData = NULL;
//int g_pCharDataSize = 0;

//int ParseTextData()
//{
//	//g_font_bitmap_size_ex = (DWORD)sizeof(g_fonts_ex);
//	//g_font_bitmap_ex = myalloc(g_font_bitmap_size_ex);
//
//	//CopyMem(g_font_bitmap_ex, g_fonts_ex, g_font_bitmap_size_ex);
//	g_pDataHeader = (DATA_HEADER *)g_fonts_ex;
//
//	g_pCharDataSize = g_pDataHeader->ucAreaSize * g_pDataHeader->ucAreaSize / 8 * g_pDataHeader->usDataCount;
//
//	return 0;
//}

//将调色板索引号转化为真彩色
DWORD IndexToColor(WORD Index, PBYTE Palette)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL color;

    if (!Palette)
    {
        ASSERT(0);
        //	Palette = (PBYTE)g_Palette16;
        //if(Index >= 16)
        	//return 0;
        	Index &= 15;
    }
    else
    {
        //if(Index >= 256)
        //	return 0;
        Index &= 255;
    }

    color.Red      = Palette[Index*3];
    color.Green    = Palette[Index*3+1];
    color.Blue     = Palette[Index*3+2];
    color.Reserved = 0;
    return *(DWORD*)(&color);
}

VOID getRectImage(IN UINTN x1, IN UINTN y1, IN UINTN Width, IN UINTN Height,
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *ImageBuffer)
{
	if (g_vga)
	{
		g_vga->Blt(g_vga, ImageBuffer, EfiBltVideoToBltBuffer, x1, y1, 0, 0, Width, Height, 0);
	}
}
//Function Name: AlphaTransparent
//Input: x1,y1,Width,Height,*ImagePointer,Alpha
//Output:
//Description: ImagePointer,指向Width*Height大小的图像缓冲区，其大小必定为Width*Height*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
//实现透明效果，使用Alpha方式，Alpha值可调,从0-16可调
VOID AlphaTransparent(UINTN x, UINTN y, UINTN Width, UINTN Height,
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *ImagePointer,
	UINT16 Alpha)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pBltBuffer;
	UINT32                        BltBufferSize;
	UINT32 i;

	//1 申请同样大小的内存，准备使用
	BltBufferSize = Width * Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

	MyLog(LOG_MESSAGE, L"AlphaTransparent BltBufferSize: %d", BltBufferSize);

	gBS->AllocatePool(
		EfiRuntimeServicesData,
		BltBufferSize,
		(VOID **)&pBltBuffer
	);

	if (pBltBuffer != NULL)
	{

		//2 要覆盖区域的图像提取
		getRectImage(x, y, Width, Height, pBltBuffer);
		//3 对两段内存进行处理,选取Alpha最大分母16,便于计算   
		//Alpha混合公式如下：(A覆盖在B上)
		  //                  R(C)=(1-alpha)*R(B) + alpha*R(A)
		  //                  G(C)=(1-alpha)*G(B) + alpha*G(A)
		  //                  B(C)=(1-alpha)*B(B) + alpha*B(A)

		for (i = 0; i < ((UINT32)Width * (UINT32)Height); i++)
		{
			pBltBuffer[i].Blue = (UINT8)(((16 - Alpha)* COLOR_MSG_MASK_R + Alpha * pBltBuffer[i].Blue) >> 4);
			pBltBuffer[i].Green = (UINT8)(((16 - Alpha)* COLOR_MSG_MASK_R + Alpha * pBltBuffer[i].Green) >> 4);
			pBltBuffer[i].Red = (UINT8)(((16 - Alpha)* COLOR_MSG_MASK_R + Alpha * pBltBuffer[i].Red) >> 4);
			pBltBuffer[i].Reserved = (UINT8)(((16 - Alpha) * 255 + Alpha * pBltBuffer[i].Reserved) >> 4);
		}
		//3 显示
		//putRectImage(x, y, Width, Height, pBltBuffer);
		//gGraphicsOutput->Blt(gGraphicsOutput, ImageBuffer, EfiBltBufferToVideo, 0, 0, x1, y1, Width, Height, 0);
		if (g_vga)
		{
			g_vga->Blt(g_vga, pBltBuffer, EfiBltBufferToVideo, 0, 0, x, y, Width, Height, 0);
		}

		if (NULL != gBS)
		{
			gBS->FreePool(pBltBuffer);
		}
		
		pBltBuffer = NULL;
	}
}

EFI_STATUS SwitchGraphicsMode(BOOL bGraphics)
{
    EFI_STATUS status = EFI_SUCCESS;

    if (g_Console)
    {
        if (bGraphics)
        {
            status = g_Console->SetMode(g_Console,EfiConsoleControlScreenGraphics);
        }
        else
        {
            status = g_Console->SetMode(g_Console,EfiConsoleControlScreenText);
        }
    }
    return status;
}

void Sort(GraphicsInfo * pAllGraphic, int iAllGraphicLen)
{
	//MyLog(LOG_DEBUG, L"Sort...\r\n");

	int iSupportGraphicLen = iAllGraphicLen;
	int iMaxIndex = 0;

	//MyLog(LOG_DEBUG, L"iSupportGraphicLen %d\r\n", iSupportGraphicLen);

	GraphicsInfo temp;
	if (iSupportGraphicLen > 0)
	{
		for (int iIndex = 0; iIndex < iSupportGraphicLen - 1; iIndex++)
		{
			iMaxIndex = iIndex;
			for (int jIndex = iIndex + 1; jIndex < iSupportGraphicLen; jIndex++)
			{
				if (pAllGraphic[jIndex].HorizontalResolution > pAllGraphic[iIndex].HorizontalResolution &&
					pAllGraphic[jIndex].VerticalResolution > pAllGraphic[iIndex].VerticalResolution)
				{
					iMaxIndex = jIndex;

					temp.HorizontalResolution = pAllGraphic[iIndex].HorizontalResolution;
					temp.VerticalResolution = pAllGraphic[iIndex].VerticalResolution;
					temp.ModeIndex = pAllGraphic[iIndex].ModeIndex;

					pAllGraphic[iIndex].HorizontalResolution = pAllGraphic[iMaxIndex].HorizontalResolution;
					pAllGraphic[iIndex].VerticalResolution = pAllGraphic[iMaxIndex].VerticalResolution;
					pAllGraphic[iIndex].ModeIndex = pAllGraphic[iMaxIndex].ModeIndex;

					pAllGraphic[iMaxIndex].HorizontalResolution = temp.HorizontalResolution;
					pAllGraphic[iMaxIndex].VerticalResolution = temp.VerticalResolution;
					pAllGraphic[iMaxIndex].ModeIndex = temp.ModeIndex;
				}
			}
		}

		////////Debug///////
		//for (int iIndex = 0; iIndex < iSupportGraphicLen; iIndex++)
		//{
		//	MyLog(LOG_DEBUG, L"After sort: Mode %d: %d*%d\r\n",
		//		pAllGraphic[iIndex].ModeIndex,
		//		pAllGraphic[iIndex].HorizontalResolution,
		//		pAllGraphic[iIndex].VerticalResolution
		//	);
		//}
	}
}

EFI_STATUS GetAllGraphicMode(EFI_GRAPHICS_OUTPUT_PROTOCOL  *gGraphicsOutput, GraphicsInfo * pAllGraphic, int iAllGraphicLen)
{
	EFI_STATUS Status;
	UINT32 ModeIndex = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *Info;
	UINTN                                  SizeOfInfo;

	if (NULL != gGraphicsOutput)
	{
		int iAllGraphicNum = 0;
		UINT32 maxMode = gGraphicsOutput->Mode->MaxMode;
		for (ModeIndex = 0; ModeIndex < maxMode; ModeIndex++)
		{
			Status = gGraphicsOutput->QueryMode(gGraphicsOutput, ModeIndex, &SizeOfInfo, &Info);
			if (!EFI_ERROR(Status))
			{
				MyLog(LOG_DEBUG, L"Mode %d: %d*%d, PixelsPerScanLine %d, Format %x, ver %x\r\n",
					ModeIndex,
					Info->HorizontalResolution, Info->VerticalResolution,
					Info->PixelsPerScanLine,
					Info->PixelFormat,
					Info->Version
				);

				pAllGraphic[ModeIndex].HorizontalResolution = Info->HorizontalResolution;
				pAllGraphic[ModeIndex].VerticalResolution = Info->VerticalResolution;
				pAllGraphic[ModeIndex].ModeIndex = ModeIndex;

				iAllGraphicNum++;

				if (iAllGraphicNum >= iAllGraphicLen)
				{
					break;
				}
			}
		}

		Sort(pAllGraphic, iAllGraphicNum);
	}

	return EFI_SUCCESS;
}

EFI_STATUS PrintAllGraphicMode()
{
	EFI_STATUS Status;
	UINT32 ModeIndex = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *Info;
	UINTN                                  SizeOfInfo = 0;

	if (NULL != g_vga)
	{
		UINT32 maxMode = g_vga->Mode->MaxMode;
		for (ModeIndex = 0; ModeIndex < maxMode; ModeIndex++)
		{
			Status = g_vga->QueryMode(g_vga, ModeIndex, &SizeOfInfo, &Info);
			if (!EFI_ERROR(Status))
			{
				MyLog(LOG_DEBUG, L"Mode %d: %d*%d, PixelsPerScanLine %d, Format %x, ver %x\r\n",
					ModeIndex,
					Info->HorizontalResolution, Info->VerticalResolution,
					Info->PixelsPerScanLine,
					Info->PixelFormat,
					Info->Version
				);
			}
		}
	}

	return EFI_SUCCESS;
}

EFI_STATUS GetMaxGraphicModeNumber(
	EFI_GRAPHICS_OUTPUT_PROTOCOL  *	gGraphicsOutput,
	OUT	UINT32 *MaxHorizontalResolution,
	OUT	UINT32 *MaxVerticalResolution,
	OUT UINT32 *MaxModeNum
)
{
	EFI_STATUS Status;
	UINT32 ModeIndex = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *Info;
	UINTN                                  SizeOfInfo;

	*MaxHorizontalResolution = 0;
	*MaxVerticalResolution = 0;
	*MaxModeNum = 0;
	//*PixelsPerScanLine = 0;

	UINT32 maxMode = gGraphicsOutput->Mode->MaxMode;
	for (ModeIndex = 0; ModeIndex < maxMode; ModeIndex++)
	{
		Status = gGraphicsOutput->QueryMode(gGraphicsOutput, ModeIndex, &SizeOfInfo, &Info);
		if (!EFI_ERROR(Status))
		{
			MyLog(LOG_DEBUG, L"Mode %d: %d*%d, PixelsPerScanLine %d, Format %x, ver %x\r\n",
				ModeIndex,
				Info->HorizontalResolution, Info->VerticalResolution,
				Info->PixelsPerScanLine,
				Info->PixelFormat,
				Info->Version
			);
			if (*MaxHorizontalResolution == 0)
			{
				*MaxHorizontalResolution = Info->HorizontalResolution;
				*MaxVerticalResolution = Info->VerticalResolution;
				*MaxModeNum = ModeIndex;
				//*PixelsPerScanLine = Info->PixelsPerScanLine;
			}
			else if (*MaxHorizontalResolution < Info->HorizontalResolution)
			{
				*MaxHorizontalResolution = Info->HorizontalResolution;
				*MaxVerticalResolution = Info->VerticalResolution;
				*MaxModeNum = ModeIndex;
				//*PixelsPerScanLine = Info->PixelsPerScanLine;
			}
			else if (*MaxHorizontalResolution == Info->HorizontalResolution)
			{
				if (*MaxVerticalResolution < Info->VerticalResolution)
				{
					*MaxHorizontalResolution = Info->HorizontalResolution;
					*MaxVerticalResolution = Info->VerticalResolution;
					*MaxModeNum = ModeIndex;
					//*PixelsPerScanLine = Info->PixelsPerScanLine;
				}
			}
		}
	}
	return EFI_SUCCESS;
}

int PowerFun(int iData, int iCount)
{
	int value = 1;
	int i = 1;
	if (iCount == 0)
	{
		value = 1;
	}
	else
	{
		while (i++ <= iCount)
		{
			value *= iData;
		}
	}
	return value;
}

float CalcMinscale(WORD wHeight, WORD wBaseHeight, int iRemainNum)
{
	float fMinscale = (float)wHeight / wBaseHeight;

	int iMinscale = (int)(fMinscale * PowerFun(10, iRemainNum + 1));

	if (iMinscale % 10 <= 4)
	{
		iMinscale = iMinscale - iMinscale % 10;
	}
	else
	{
		iMinscale += 10;
		iMinscale = iMinscale - iMinscale % 10;
	}

	fMinscale = iMinscale * 1.0 / PowerFun(10, iRemainNum + 1);

	return fMinscale;
}

BOOL IsSupportResolution(GraphicsInfo * pGraphicsInfo)
{
	//MyLog(LOG_DEBUG, L"iSupportGraphicLen... \n");
	BOOL bRet = FALSE;
	
	if (NULL != pGraphicsInfo)
	{
		for (int iIndex = 0; iIndex < sizeof(g_supportGraphics) / sizeof(GraphicsInfo); iIndex++)
		{
			if (pGraphicsInfo->HorizontalResolution == g_supportGraphics[iIndex].HorizontalResolution &&
				pGraphicsInfo->VerticalResolution == g_supportGraphics[iIndex].VerticalResolution)
			{
				bRet = TRUE;
				//MyLog(LOG_DEBUG, L"Find support resolution:ModeIndex %d  %d*%d\n", g_supportGraphics[iIndex].ModeIndex, g_supportGraphics[iIndex].HorizontalResolution, g_supportGraphics[iIndex].VerticalResolution);
				break;
			}
		}
	}

	return bRet;
}

//#define X_RES 1920
//#define Y_RES 1080
//#define X_RES 1024
//#define Y_RES 768
//#define X_RES 800
//#define Y_RES 600
BOOL VgaInit()
{
    EFI_STATUS						status = EFI_NOT_FOUND;
    EFI_HANDLE                 		handle;
    EFI_CONSOLE_CONTROL_PROTOCOL  * consoleControl;
    EFI_GRAPHICS_OUTPUT_PROTOCOL  *	graphicsOutput;
    //EFI_UGA_DRAW_PROTOCOL         *	ugaDraw;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION	*info;
    UINT32							modeNumber, maxMode;
	//UINT32							vgamode[5];	//存储1280*1024, 1024*768, 1280*800, 1600*900 1366*768
	WORD							width, height;
    UINTN							infoLength;
    UINT32                          horizontalResolution;
    UINT32                          verticalResolution;
    UINT32                          colorDepth;
    UINT32                          refreshRate;

	UINT32 MaxHorizontalResolution;
	UINT32 MaxVerticalResolution;
	UINT32 MaxModeNum;

    handle = gST->ConsoleOutHandle;

    //首先要获得控制台接口, 通过LocateProtocol找到gEfiConsoleControlProtocolGuid的第一个instanace，用该函数获得的protocol不用关闭
    status = gBS->LocateProtocol(&gEfiConsoleControlProtocolGuid, NULL, (VOID **)&consoleControl);
    if (EFI_ERROR(status))
    {
		MyLog(LOG_DEBUG, L"LocateProtocol failed with error 0x%x\r\n", status);
		//该protocol并不重要，有些机器即便没有该protocol，依然能正常切换图形显示，所以不用返回失败
        //return FALSE;
    }
    g_Console = consoleControl;

    graphicsOutput = NULL;

    //首先尝试获得EfiGraphicsOutputProtocol，如果失败，则尝试获得EfiUgaDrawProtocol
    status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&graphicsOutput);
    if (EFI_ERROR (status) || (graphicsOutput == NULL))
    {
        graphicsOutput = NULL;
        MyLog(LOG_ERROR, L"LocateProtocol(GraphicsOutput) failed with error 0x%x\r\n", status);
		return FALSE;
    }

    if (graphicsOutput)
    {
		BOOL bHasSupportGraphics = FALSE;

		GraphicsInfo machineGraphics[20];
		SetMem(machineGraphics, sizeof(GraphicsInfo) * 20, 0);

		status = GetAllGraphicMode(graphicsOutput, machineGraphics, 20);
		if (EFI_SUCCESS == status)
		{
			int iSupportGraphicLen = sizeof(machineGraphics) / sizeof(GraphicsInfo);
			//MyLog(LOG_DEBUG, L"iSupportGraphicLen %d\r\n", iSupportGraphicLen);
			for (int iIndex = 0; iIndex < iSupportGraphicLen; iIndex++)
			{
				if (TRUE == IsSupportResolution(&machineGraphics[iIndex]))
				{
					g_VgaModeNumber = (WORD)machineGraphics[iIndex].ModeIndex;
					g_BitsPerPixel = 32;
					g_WindowWidth = machineGraphics[iIndex].HorizontalResolution;
					g_WindowHeight = machineGraphics[iIndex].VerticalResolution;

					bHasSupportGraphics = TRUE;
					//MyLog(LOG_DEBUG, L"bHasSupportGraphics true\r\n");
					break;
				}
			}

			if (FALSE == bHasSupportGraphics)
			{
				//MyLog(LOG_DEBUG, L"bHasSupportGraphics false\r\n");
				g_VgaModeNumber = (WORD)machineGraphics[0].ModeIndex;
				g_BitsPerPixel = 32;
				g_WindowWidth = machineGraphics[0].HorizontalResolution;
				g_WindowHeight = machineGraphics[0].VerticalResolution;
			}
		}
		//MyLog(LOG_DEBUG, L"g_VgaModeNumber %d Width*Height: %d*%d\r\n", g_VgaModeNumber, g_WindowWidth, g_WindowHeight);


        //MyLog(LOG_DEBUG, L"MaxMode %d, Mode: %d, Info %p, infosize %d, FrameBufferBase %lx, FrameBufferSize %x\r\n",
        //	graphicsOutput->Mode->MaxMode, graphicsOutput->Mode->Mode,
        //	graphicsOutput->Mode->Info, graphicsOutput->Mode->SizeOfInfo,
        //	graphicsOutput->Mode->FrameBufferBase, graphicsOutput->Mode->FrameBufferSize
        //	);

		//vgamode[0] = vgamode[1] = vgamode[2] = vgamode[3] = vgamode[4] = -1;
		//width = height = 0xffff;
		//maxMode = graphicsOutput->Mode->MaxMode;
		//for (modeNumber = 0; modeNumber < maxMode; modeNumber++)
		//{
		//	status = graphicsOutput->QueryMode(graphicsOutput, modeNumber, &infoLength, &info);
		//	if (!EFI_ERROR(status))
		//	{
		//		MyLog(LOG_DEBUG, L"Mode %d: %d*%d, PixelsPerScanLine %d, Format %x, ver %x\r\n",
		//			modeNumber,
		//			info->HorizontalResolution, info->VerticalResolution,
		//			info->PixelsPerScanLine,
		//			info->PixelFormat,
		//			info->Version
		//			//info->PixelInformation.RedMask, info->PixelInformation.GreenMask,
		//			//info->PixelInformation.BlueMask, info->PixelInformation.ReservedMask
		//		);
		//		getch();
		//		if (info->HorizontalResolution == X_RES && info->VerticalResolution == Y_RES)
		//		{
		//			g_VgaModeNumber = (WORD)modeNumber;
		//			g_BitsPerPixel = 32;
		//			g_WindowWidth = X_RES;
		//			g_WindowHeight = Y_RES;
		//			break;
		//		}
		//		else if (info->HorizontalResolution == 1600 && info->VerticalResolution == 900)
		//			vgamode[0] = modeNumber;
		//		else if (info->HorizontalResolution == 1366 && info->VerticalResolution == 768)
		//			vgamode[1] = modeNumber;
		//		else if (info->HorizontalResolution == 1280 && info->VerticalResolution == 800)
		//			vgamode[2] = modeNumber;
		//		else if (info->HorizontalResolution == 1024 && info->VerticalResolution == 768)
		//			vgamode[3] = modeNumber;
		//		else if (info->HorizontalResolution > 1024 && info->VerticalResolution > 768) 
		//		{
		//			if (vgamode[4] == -1 || (width >= (WORD)info->HorizontalResolution && height >= info->VerticalResolution)) 
		//			{
		//				width = (WORD)info->HorizontalResolution;
		//				height = (WORD)info->VerticalResolution;
		//				vgamode[4] = modeNumber;
		//			}
		//		}
		//		if (vgamode[0] != -1 && vgamode[1] != -1 && vgamode[2] != -1 && vgamode[3] != -1 && vgamode[4] != -1)
		//			break;
		//		FreePool(info);
		//	}
		//	else
		//	{
		//		MyLog(LOG_ERROR, L"Failed to query mode %d with error 0x%x\r\n", modeNumber, status);
		//		break;
		//	}
		//}
		//if (!g_WindowWidth)
		//{
		//	//说明没有找到我们想要的分辨率，则从备选的四个中选择一个
		//	for (modeNumber = 0; modeNumber < sizeof(vgamode) / sizeof(UINT32); modeNumber++)
		//	{
		//		if (vgamode[modeNumber] != -1)
		//		{
		//			g_VgaModeNumber = (WORD)vgamode[modeNumber];
		//			g_BitsPerPixel = 32;
		//			if (modeNumber == 0)
		//			{
		//				g_WindowWidth = 1600;
		//				g_WindowHeight = 900;
		//			}
		//			else if (modeNumber == 1)
		//			{
		//				g_WindowWidth = 1366;
		//				g_WindowHeight = 768;
		//			}
		//			else if (modeNumber == 2)
		//			{
		//				g_WindowWidth = 1280;
		//				g_WindowHeight = 800;
		//			}
		//			else if (modeNumber == 3)
		//			{
		//				g_WindowWidth = 1024;
		//				g_WindowHeight = 768;
		//			}
		//			else if (modeNumber == 4)
		//			{
		//				g_WindowWidth = width;
		//				g_WindowHeight = height;
		//			}
		//			MyLog(LOG_DEBUG, L"g_VgaModeNumber %d g_WindowWidth %d g_WindowHeight %d\r\n", g_VgaModeNumber, g_WindowWidth, g_WindowHeight);
		//			break;
		//		}
		//	}
		//}
		//if (!g_WindowWidth) 
		//{
		//	//说明没有找到合适的分辨率
		//	PrintLogBuffer();
		//	return FALSE;
		//}

		///获取最大支持的分辨率，查看是否是支持的分辨率
		//status = GetMaxGraphicModeNumber(graphicsOutput, &MaxHorizontalResolution, &MaxVerticalResolution, &MaxModeNum);
		//if (TRUE == IsSupportResolution(MaxHorizontalResolution, MaxVerticalResolution))
		//{
		//	g_VgaModeNumber = (WORD)MaxModeNum;
		//	g_BitsPerPixel = 32;
		//	g_WindowWidth = MaxHorizontalResolution;
		//	g_WindowHeight = MaxVerticalResolution;
		//}
		//else
		//{
		//	///查找合适的分辨率

		//}
		
		//g_VgaModeNumber = 1;// (WORD)MaxModeNum;
		//g_BitsPerPixel = 32;
		//g_WindowWidth = 1024;// MaxHorizontalResolution;
		//g_WindowHeight = 768;// MaxVerticalResolution;
		
		status = SwitchGraphicsMode(TRUE);
		MyLog(LOG_DEBUG, L"SwitchGraphicsMode returns 0x%x\n", status);
		status = graphicsOutput->SetMode(graphicsOutput, g_VgaModeNumber);
		MyLog(LOG_DEBUG, L"SetMode g_VgaModeNumber %d returns 0x%x\n", g_VgaModeNumber, status);
		//MyLog(LOG_DEBUG, L"Width*Height: %d*%d\r\n", g_WindowWidth, g_WindowHeight);
		if (EFI_ERROR(status))
		{
			MyLog(LOG_ERROR, L"Setmode(%d) failed with error 0x%x\r\n", g_VgaModeNumber, status);
			return FALSE;
		}

#if OKR_DEBUG
		getch();
#endif

		g_vga = graphicsOutput;
		if (!g_vgabuffer)
			g_vgabuffer = AllocatePool(VGA_BUFSIZE);
		if (!g_vgabuffer)
			return FALSE;
	}

	{
		//GetBmpRect(IMG_FILE_LOGO, &g_data.rWnd);
		WORD WndSize = g_WindowWidth < g_WindowHeight ? g_WindowWidth : g_WindowHeight;

		g_data.rWnd.width = WndSize; 
		g_data.rWnd.height = WndSize;
		g_data.rWnd.x = (g_WindowWidth - g_data.rWnd.width) / 2;	//窗口的x坐标
		g_data.rWnd.y = (g_WindowHeight - g_data.rWnd.height) / 2;	//窗口的y坐标
		g_data.fMinscale = (float)WndSize / 2160; //CalcMinscale(WndSize, 2160, 1); //
		g_data.fFontScale = g_data.fMinscale;

		if ((0.45 < g_data.fMinscale) && (0.5 >= g_data.fMinscale))
		{
			g_data.fMinscale = 0.5;
		}

		if ((0.42 < g_data.fFontScale) && (0.5 >= g_data.fFontScale))
		{
			g_data.fFontScale = 0.5;
		}

		if (g_WindowWidth <= 1024 && g_WindowHeight <= 768)
		{
			g_data.fFontScale = 0.5;
		}
		else if (g_WindowWidth > 1024 && g_WindowHeight > 768 &&
			g_WindowWidth <= 1920 && g_WindowHeight <= 1080)
		{
			g_data.fFontScale = 0.5;
			g_data.fMinscale = 0.5;
		}
		else if (g_WindowWidth > 1920 && g_WindowHeight > 1080 &&
			g_WindowWidth <= 2560 && g_WindowHeight <= 1440)
		{
			g_data.fFontScale = (float)1440 / 2160;
			g_data.fMinscale = (float)1440 / 2160;
		}
		else if (g_WindowWidth > 2560 && g_WindowHeight > 1440 &&
			g_WindowWidth <= 3840 && g_WindowHeight <= 2160)
		{
			g_data.fFontScale = 1;
			g_data.fMinscale = 1;
		}
		else if (g_WindowWidth > 3840 && g_WindowHeight > 2160)
		{
			g_data.fFontScale = 1;
			g_data.fMinscale = 1;
		}
		else
		{
			if (1.0 <= g_data.fMinscale)
			{
				g_data.fMinscale = 1.0;
				g_data.fFontScale = 1;
			}
			else if (0.7 <= g_data.fMinscale && 1.0 >= g_data.fMinscale)
			{
				g_data.fMinscale = 1.0;
				g_data.fFontScale = 1;
			}
			else if (0.6 <= g_data.fMinscale && 0.7 >= g_data.fMinscale)
			{
				g_data.fMinscale = (float)1440 / 2160;
				g_data.fFontScale = (float)1440 / 2160;
			}
			else if (0.5 <= g_data.fMinscale && 0.6 >= g_data.fMinscale)
			{
				g_data.fMinscale = 0.5;
				g_data.fFontScale = 0.5;
			}
		}

		g_x_begin = g_data.rWnd.x;
		g_y_begin = g_data.rWnd.y;
	}

	return TRUE;
}

PBYTE ExtractZipFile(PBYTE data, DWORD *size)
{
	DWORD datasize;

	datasize = *size;
	*size = g_data.bmpsize;
	return Decompress(data, datasize, g_data.bmp, size);
}

void ReleaseBmp(PBYTE bmp)
{
	if(bmp != g_data.bmp) {
		myfree(bmp);
	}
}

INTN UnloadResource()
{
	if(g_font_bitmap)
		myfree(g_font_bitmap);
	g_font_bitmap = NULL;

	if (g_font_bitmap_ex)
		myfree(g_font_bitmap_ex);
	g_font_bitmap_ex = NULL;

    return 0;
}

//void checkres(DWORD fileid, DWORD context)
//{
//    PIMAGE_FILE_INFO pfile;
//
//    pfile = FindImgFileFromName(fileid);
//	if(*(DWORD*)pfile->pmem >= 0x10000)
//	{
//		MyLog(LOG_DEBUG,L"checkres(%d, %x) failed, pmem %p, data %x\n",
//			fileid, context, pfile->pmem, *(DWORD*)pfile->pmem);
//		getch();
//	}
//	else
//	{
//		MyLog(LOG_DEBUG,L"checkres(%d, %x) success, pmem %p, data %x\n",
//			fileid, context, pfile->pmem, *(DWORD*)pfile->pmem);
//	}
//}

int pow(int a, int b)
{
	int ret = 1;
	int i = 0;
	for (i = 0; i < b; i++)
	{
		ret *= 2;
	}
	return ret;
}

void AddBitData2CharData(
	unsigned char* pBitData,
	int bitDataTotalLen,
	int bitDataLen,
	unsigned char* pCharData,
	int charDataLen)
{
	int i = 0;
	int j = 0;
	//int textCout = bitDataTotalLen * 8 / bitDataLen;
	int textCout = charDataLen;
	int grayLeves = pow(2, bitDataLen - 1);
	if (textCout <= bitDataTotalLen * 8 / bitDataLen)
	{
		for (i = 0; i < textCout; i++)
		{
			int charIndex = (i * bitDataLen) / 8;
			int bitIndex = (i * bitDataLen) % 8;

			unsigned char value = 0;
			for (j = bitDataLen - 1; j >= 0; j--)
			{
				int writeBitIndex = (bitIndex + j) % 8;
				int charOffSet = (bitIndex + j) > 7 ? 1 : 0;
				CharData charDataTemp;
				charDataTemp.charValue = pBitData[charIndex + charOffSet];
				value = value << 1;
				switch (writeBitIndex)
				{
				case 0:
					value += charDataTemp.bitValue.c0;
					break;
				case 1:
					value += charDataTemp.bitValue.c1;
					break;
				case 2:
					value += charDataTemp.bitValue.c2;
					break;
				case 3:
					value += charDataTemp.bitValue.c3;
					break;
				case 4:
					value += charDataTemp.bitValue.c4;
					break;
				case 5:
					value += charDataTemp.bitValue.c5;
					break;
				case 6:
					value += charDataTemp.bitValue.c6;
					break;
				case 7:
					value += charDataTemp.bitValue.c7;
					break;
				default:
					break;
				}
			}
			pCharData[i] = value * 255 / grayLeves;
		}
	}
}

void InitFontTtextData(
	unsigned char* pTextData,
	int dataLen,
	FontTextData *pFontTextData)
{
	int index = 0;
	int i = 0;
	pFontTextData->realTextDataCout = 0;
	if (dataLen > 4)
	{
		HeaderFont *pTemp = (HeaderFont *)pTextData;
		pFontTextData->header = *pTemp;
		
		index += 4;
		if (pFontTextData->header.textCout > 0)
		{
			pFontTextData->ppTextDataList = (TextData **)AllocatePool(sizeof(TextData *) * pFontTextData->header.textCout);
			for (i = 0; i < pFontTextData->header.textCout && pFontTextData->realTextDataCout < pFontTextData->header.textCout; i++)
			{
				TextData *pTempTextData = (TextData *)AllocatePool(sizeof(TextData));
				if (index + 10 < dataLen)
				{
					TextHeader *pTextHeader = (TextHeader *)(pTextData + index);
					pTempTextData->textHeader = *pTextHeader;
					pTempTextData->pdata = NULL;
					index += 10;
					if (pTempTextData->textHeader.dataLen > 0 && index + pTempTextData->textHeader.dataLen <= dataLen)
					{
						int newDataLen = pTempTextData->textHeader.realWidth * pTempTextData->textHeader.realHeight;
						pTempTextData->pdata = AllocatePool(newDataLen);
						SetMem(pTempTextData->pdata, newDataLen, 0x00);
						AddBitData2CharData(
							pTextData + index,
							pTempTextData->textHeader.dataLen,
							pFontTextData->header.usedbitCout,
							pTempTextData->pdata,
							newDataLen);
						index += pTempTextData->textHeader.dataLen;
					}
					else if (pTempTextData->textHeader.text == 0x20) //空格
					{
						//
					}
					else
					{
						break;
					}
					pFontTextData->ppTextDataList[pFontTextData->realTextDataCout] = pTempTextData;
					pFontTextData->realTextDataCout++;
				}
				else
				{
					break;
				}
			}
		}
	}
}

void InitAllFontData()
{
	int i = 0;
	for (i = 0; i < sizeof(fontTextConfig) / sizeof(FontTextConfig); i++)
	{
		fontTextConfig[i].pFontTextData->realTextDataCout = 0;
		InitFontTtextData(
			fontTextConfig[i].pTextData,
			fontTextConfig[i].dataLen,
			fontTextConfig[i].pFontTextData);
	}
}

//load pcx and font bitmap to memory
// device: the disk ,curdir: the program running directory,with '\' ending
INTN LoadResource()
{
    INTN size;
    INTN index=0;
    PBYTE pRes;

	//解压fonts
	{
    	//PVOID data;
    	//DWORD datasize = (DWORD)sizeof(g_fonts);
    	//data = ExtractZipFile((PBYTE)g_fonts, &datasize);
    	//g_font_bitmap = myalloc(datasize);
    	//CopyMem(g_font_bitmap, data, datasize);
    	//g_font_bitmap_size = datasize;

		InitAllFontData();
    }
	
	///////////////////////////////
    //load pcx file to memory
    index = 0;
    pRes = g_res;
    while (g_Img_file_list[index].id != 0) {

        g_Img_file_list[index].pmem = pRes;
        g_Img_file_list[index].size = (*(DWORD*)pRes) + 8;
        pRes += g_Img_file_list[index].size;
        
        if (!g_Img_file_list[index].pmem) {
            MyLog(LOG_ERROR,L"Can't Load resource file %d\n",g_Img_file_list[index].id);
            return -1;
		}
		
		//MyLog(LOG_DEBUG,L"loadres(%d), pmem %p, data %x\n",
		//	g_Img_file_list[index].id,
		//	g_Img_file_list[index].pmem,
		//	*(DWORD*)g_Img_file_list[index].pmem);
		        
        index++;
    }
    //checkres(IMG_FILE_9, 0x1111);
	//getch();    

    return 0;
}

EFI_STATUS VgaRelease()
{
    EFI_HANDLE handle;
    EFI_STATUS status = EFI_SUCCESS;
//Print(L"VgaRelease, g_vga %p, g_uga %p, g_vgabuffer %p\r\n",
//	g_vga, g_uga, g_vgabuffer);

    SwitchGraphicsMode(FALSE);

    //if(g_vga)
    //{
    //	handle = gST->ConsoleOutHandle;
    //	gBS->CloseProtocol (
    //		      handle,
    //		      &gEfiGraphicsOutputProtocolGuid,
    //		      gImageHandle,
    //		      NULL
    //		      );
    g_vga = NULL;
    //}
 

//Print(L"myfree(g_vgabuffer %p)\r\n", g_vgabuffer);
    if (g_vgabuffer)
    {
        FreePool(g_vgabuffer);
        g_vgabuffer = NULL;
    }

//Print(L"SetMode(gST->ConOut %p)\r\n", gST->ConOut);
    //设置文本模式
    status = gST->ConOut->SetMode(gST->ConOut, 0);

//Print(L"SetMode(gST->ConOut %p) erturns %x\r\n", gST->ConOut, status);
//getch();
    return EFI_SUCCESS;
}

VOID DrawDot(WORD x, WORD y,DWORD Color)
{
    DrawLine(x,y,1,1,Color);
    return;
}

VOID DrawDotEx(WORD x, WORD y, DWORD Color, DWORD gray)
{
	BYTE R = (Color >> 16) & 0xff;
	BYTE G = (Color >> 8) & 0xff;
	BYTE B = Color & 0xff;
	BYTE Back_R = (g_textBKColor >> 16) & 0xff;
	BYTE Back_G = (g_textBKColor >> 8) & 0xff;
	BYTE Back_B = g_textBKColor & 0xff;
	float grayScale = ((float)((255 - gray) * 1.0)) / 255;
	R = R + (int)((Back_R - R) * grayScale);
	G = G + (int)((Back_G - G) * grayScale);
	B = B + (int)((Back_B - B) * grayScale);
	Color = 0;
	Color += R;
	Color = Color << 8;
	Color += G;
	Color = Color << 8;
	Color += B;
	DrawLine(x, y, 1, 1, Color);
	return;
}


VOID BufferDrawDotEx(EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *ColorBuf, DWORD Color, DWORD gray)
{
	if (NULL != ColorBuf)
	{
		BYTE R = (Color >> 16) & 0xff;
		BYTE G = (Color >> 8) & 0xff;
		BYTE B = Color & 0xff;
		BYTE Back_R = (g_textBKColor >> 16) & 0xff;
		BYTE Back_G = (g_textBKColor >> 8) & 0xff;
		BYTE Back_B = g_textBKColor & 0xff;
		float grayScale = ((float)((255 - gray) * 1.0)) / 255;
		R = R + (int)((Back_R - R) * grayScale);
		G = G + (int)((Back_G - G) * grayScale);
		B = B + (int)((Back_B - B) * grayScale);
		Color = 0;
		Color += R;
		Color = Color << 8;
		Color += G;
		Color = Color << 8;
		Color += B;

		ColorBuf->Red = R;
		ColorBuf->Green = G;
		ColorBuf->Blue = B;
		ColorBuf->Reserved = 0;
	}

	return;
}

VOID DrawDotLine(WORD x, WORD y, WORD Width, WORD Height, DWORD Color)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL color;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)g_vgabuffer;
    DWORD i, j;

    if (g_vga)
        g_vga->Blt(g_vga, bltBuffer, EfiBltVideoToBltBuffer, x, y, 0, 0, Width, Height, 0);

    *(DWORD*)(&color) = Color;
    for (i=0; i<Height; i++)	//FONT_HEIGHT
    {
        for (j=0;j<Width;j++)
        {
            if ((j + (i & 1)) & 1)
            {
                //	*(DWORD*)(bltBuffer + j) = *(DWORD*)(&color);
                *(DWORD*)(bltBuffer + i*Width + j) = *(DWORD*)(&color);
            }
        }

        //			if(g_vga)
        //				g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y+i, Width, 1, 0);
        //			else
        //				g_uga->Blt(g_uga, (EFI_UGA_PIXEL*)bltBuffer, EfiUgaBltBufferToVideo, 0,0, x, y+i, Width, 1, 0);

    }

    if (g_vga)
        g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y, Width, Height, 0);
}
//画水平线, 垂直线
VOID DrawLine(WORD x, WORD y, WORD Width, WORD Height, DWORD Color)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer = NULL;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL color;
    DWORD nPixels;
    DWORD i, j;
    BOOL  bDrawLine = FALSE;

    bltBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)g_vgabuffer;
//	if(Width * Height *4> VGA_BUFSIZE)	//提供的缓冲区不足以容纳矩形区域 gjg??
//		bDrawLine = TRUE;
    /*
    	if(ColorIndex & MODE_DOTTEDLINE)
    	{
    		if(!bDrawLine)
    		{
    			if(g_vga)
    				g_vga->Blt(g_vga, bltBuffer, EfiBltVideoToBltBuffer, x, y, 0, 0, Width, Height, 0);
    			else
    				g_uga->Blt(g_uga, (EFI_UGA_PIXEL*)bltBuffer, EfiUgaVideoToBltBuffer, x, y, 0, 0, Width, Height, 0);
    		}

    		*(DWORD*)(&color) = IndexToColor(ColorIndex, NULL);
    		for(i=0; i<Height; i++)	//FONT_HEIGHT
    		{
    			for(j=0;j<Width;j++)
    			{
    				if((j + (i & 1)) & 1)
    				{
    					if(bDrawLine)
    						*(DWORD*)(bltBuffer + j) = *(DWORD*)(&color);
    					else
    						*(DWORD*)(bltBuffer + (i*Width) + j) = *(DWORD*)(&color);
    				}
    			}
    			if(bDrawLine)
    			{
    				if(g_vga)
    					g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y+i, Width, 1, 0);
    				else
    					g_uga->Blt(g_uga, (EFI_UGA_PIXEL*)bltBuffer, EfiUgaBltBufferToVideo, 0,0, x, y+i, Width, 1, 0);
    			}
    		}
    		if(!bDrawLine)
    		{
    			if(g_vga)
    				g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y, Width, Height, 0);
    			else
    				g_uga->Blt(g_uga, (EFI_UGA_PIXEL*)bltBuffer, EfiUgaBltBufferToVideo, 0,0, x, y, Width, Height, 0);
    		}
    	}
    	else if(ColorIndex == COLOR_SMOOTH)
    	{
    		for(i=0; i<Height; i++)	//FONT_HEIGHT
    		{
    			*(DWORD*)(&color) = TRUE_COLOR_TITLE_BLUE;
    			for(j=0;j<Width;j++)
    			{
    				if((Width >= 300 && (j&1)) || Width < 300)
    				{
    					color.Red++;
    					if(color.Red >= 173)
    						color.Red = 173;
    					color.Green++;
    					if(color.Green >= 207)
    						color.Green = 207;
    					color.Blue++;
    					if(color.Blue == 0)
    						color.Blue = 0xff;
    				}
    				if(bDrawLine)
    					*(DWORD*)(bltBuffer + j) = *(DWORD*)(&color);
    				else
    					*(DWORD*)(bltBuffer + i*Width + j) = *(DWORD*)(&color);
    			}
    			if(bDrawLine)
    			{
    				if(g_vga)
    					g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y+i, Width, 1, 0);
    				else
    					g_uga->Blt(g_uga, (EFI_UGA_PIXEL*)bltBuffer, EfiUgaBltBufferToVideo, 0,0, x, y+i, Width, 1, 0);
    			}
    		}
    		if(!bDrawLine)
    		{
    			if(g_vga)
    				g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y, Width, Height, 0);
    			else
    				g_uga->Blt(g_uga, (EFI_UGA_PIXEL*)bltBuffer, EfiUgaBltBufferToVideo, 0,0, x, y, Width, Height, 0);
    		}
    	}
    	else
    	*/
    {
        //设置颜色
        //	*(DWORD*)(&color) = IndexToColor(ColorIndex, NULL);
        *(DWORD*)(&color)=Color;
        if (g_vga)
            g_vga->Blt(g_vga, &color, EfiBltVideoFill, 0,0, x, y, Width, Height, 0);
    }
}

//画水平线, 垂直线
VOID BufferDrawLine(EFI_GRAPHICS_OUTPUT_BLT_PIXEL* ColorBuf, WORD x, WORD y, WORD Width, WORD Height, DWORD Color)
{
	if (NULL != ColorBuf)
	{
		EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffDataPos = ColorBuf + y * Width + x;
		if (NULL != buffDataPos)
		{
			EFI_GRAPHICS_OUTPUT_BLT_PIXEL color;
			*(DWORD *)(&color) = Color;

			for (int i = 0; i < Height; i++)
			{
				for (int j = 0; j < Width; j++)
				{
					buffDataPos[i * Width + j + y] = color;
				}
			}
		}
	}
}

VOID DrawLineAlpha(WORD x, WORD y, WORD Width, WORD Height, UINT16 Alpha)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer = NULL;

	bltBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)g_vgabuffer;
	{
		AlphaTransparent(x, y, Width, Height, bltBuffer, Alpha);

		//if (g_vga)
		//	g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0, 0, x, y, Width, Height, 0);
	}
}

VOID DrawLinearRect(DWORD begincolor, DWORD endcolor, WORD x, WORD y, WORD width, WORD height)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer = NULL;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL color;
    WORD  i, j;
    WORD step=height/(   ((EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)(&begincolor))->Red-((EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)(&endcolor))->Red);
    WORD current=0;

    *(DWORD *)(&color)=begincolor;
    bltBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)g_vgabuffer;
    for (i=0;i<height;i++) {
        for (j=0;j<width;j++) {
            *(DWORD *)((DWORD *)bltBuffer+j+width*i)=*(DWORD*)(&color);
        }
        current++;
        if (current>=step) {

            color.Red--;
            color.Green--;
            color.Blue--;
		 
            current=0;
        }
    }
    if (g_vga)
        g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y, width, height, 0);
}

VOID DrawRect(DWORD _color,  WORD x, WORD y, WORD width, WORD height)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer = NULL;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL color;
	WORD  i, j;
 
	*(DWORD *)(&color) = _color;
	bltBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)g_vgabuffer;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			*(DWORD *)((DWORD *)bltBuffer + j + width*i) = *(DWORD*)(&color);
		}
	}
	if (g_vga)
		g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0, 0, x, y, width, height, 0);
}

VOID Linebox(WORD x, WORD y, WORD Width, WORD Height, DWORD Color)
{
    DrawLine(x, y, 1, Height, Color);
    DrawLine(x, y, Width, 1, Color);
    DrawLine(x+Width-1, y, 1, Height, Color);
    DrawLine(x, y+Height-1, Width, 1, Color);
}

VOID DotLinebox(WORD x, WORD y, WORD Width, WORD Height, DWORD Color)
{
    DrawDotLine(x, y, 1, Height, Color);
    DrawDotLine(x, y, Width, 1, Color);
    DrawDotLine(x+Width-1, y, 1, Height, Color);
    DrawDotLine(x, y+Height-1, Width, 1, Color);
}

//画下沉的矩形
void DrawSinkableRectangle(WORD x, WORD y, WORD width, WORD height, DWORD framecolor, DWORD color)
{
    //COLOR_SHADOW_GREY
    DrawLine(x,y,width,1,framecolor);				//top
    //DrawLine(x+1,y+1,width-2,1,COLOR_SHADOW_BLACK);		//top
    DrawLine(x,y,1, height,framecolor);			//left
    //DrawLine(x+1,y+1,1,height-2,COLOR_SHADOW_BLACK);	//left
    //if(color != COLOR_TRANSPARENCE)
    FillRect(x+1, y+1, width-2, height-2, color);	//center
    //DrawLine(x+1,y+height-2,width-2,1,COLOR_GREY);		//bottom
    DrawLine(x,y+height-1,width,1,framecolor);			//bottom
    //DrawLine(x+width-2,y+1,1,height-2,COLOR_GREY);		//right
    DrawLine(x+width-1,y,1,height,framecolor);			//right
}

#ifdef _DRAW_CIRCLE_
void DrawCircle(int quadrant, int x, int y, DWORD Color)	//画圆弧，第1象限
{
	BYTE circle_line[]={3,2,1,2};
	BYTE circle_r = 13;
	int i, j, x1, y1;

	for(i=0;i<2;i++)
	{
		for(j=0;j<sizeof(circle_line);j++)
		{
			if( quadrant == 1 )
			{
				if(i == 0)
				{
					x1 --;
					y1 -= circle_line[j];
					DrawDotLine(x1, y1, 1, circle_line[j], Color);
				}
				else
				{
					x1 -= circle_line[sizeof(circle_line) - j - 1];
					y1 --;
					DrawDotLine(x1, y1, circle_line[sizeof(circle_line) - j - 1], 1, Color);
				}
			}
			if( quadrant == 2 )
			{
				if(i == 0)
				{
					x1 -= circle_line[j];
					y1 ++;
					DrawDotLine(x1, y1, circle_line[j], 1, Color);
				}
				else
				{
					x1 --; 
					if(j == 0)
						y1 ++;
					DrawDotLine(x1, y1, 1, circle_line[sizeof(circle_line) - j - 1], Color);
					y1 += circle_line[sizeof(circle_line) - j - 1];
				}
			}
			if( quadrant == 3 )
			{
				if(i == 0)
				{
					x1 ++;
					if(j == 0)
						y1 ++;
					DrawDotLine(x1, y1, 1, circle_line[j], Color);
					y1 += circle_line[j];
				}
				else
				{
					DrawDotLine(x1, y1, circle_line[sizeof(circle_line) - j - 1], 1, Color);
					x1 += circle_line[sizeof(circle_line) - j - 1];
					y1 ++;
				}
			}
			if( quadrant == 4 )
			{
				if(i == 0)
				{
					if(j == 0)
						x1 ++;
					y1 --;
					DrawDotLine(x1, y1, circle_line[j], 1, Color);
					x1 += circle_line[j];
				}
				else
				{
					y1 -= circle_line[sizeof(circle_line) - j - 1];
					DrawDotLine(x1, y1, 1, circle_line[sizeof(circle_line) - j - 1], Color);
					x1 ++; 
				}
			}			
		}
	}
}

//圆角矩形
VOID DotLineCircleBox(WORD x, WORD y, WORD Width, WORD Height, DWORD Color)
{
	//圆角矩形
	//BYTE CicleDelta_Big[]={3,2,1,2};	//注意对称性，只要4组就可以了
	//BYTE CicleDelta_Small[]={2,1];
	
	if(Height > 100)
	{
    	DrawDotLine(x+Width-1, y+13, 1, Height-26, Color);	//right line
		DrawCicle(1, x+Width-1, y+13, Color);	//画圆弧，第1象限
		DrawDotLine(x+13, y, Width-26, 1, Color);	//top line
    	DrawCicle(2, x+13, y, Color);	//画圆弧，第2象限
    	DrawDotLine(x, y+13, 1, Height-26, Color);	//left line
    	DrawCicle(3, x, y+Height-13, Color);	//画圆弧，第3象限
    	DrawDotLine(x+13, y+Height-1, Width-26, 1, Color);	//bottom line
    	DrawCicle(4, x+Width-13, y+Height-1, Color);	//画圆弧，第4象限
    }
    else
    {
    	//DrawDotLine(x+5, y, Width-10, 1, Color);
    	//DotLinebox();
    }
}
#endif

//画图，width、height为0时表示按照pcx图形的原来大小来绘制，否则按照指定大小绘制
//如果是画PCX图形，则PcxHeader有效，ResHeader无效
//如果是画RES图形，则ResHeader有效，PcxHeader无效
//PcxImage指向图形数据
BOOL DrawPCXImage(WORD x, WORD y,  PCXHEAD *PcxHeader, UINTN Pcxsize,BOOL ifTransParent)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer;
    BYTE		*palette;
    UINT32		iScanLineSize, clImageSize, iPlanes, bytesPerLine;
    UINTN		imageType;
    EFI_STATUS  status;
    WORD Width,Height;
    PBYTE PcxImage=(PBYTE)PcxHeader+0x80;

    imageType = BMP_UNKNOWN;

    if (PcxHeader->BitsPerPixel == 1 && PcxHeader->ColorPlanes == 4)
        imageType = BMP_COLOR16;
    else if (PcxHeader->BitsPerPixel == 4 && PcxHeader->ColorPlanes == 1)
        imageType = BMP_COLOR16;
    else if (PcxHeader->BitsPerPixel == 8)
    {
        if (PcxHeader->ColorPlanes == 1)
            imageType = BMP_COLOR256;
        else if (PcxHeader->ColorPlanes == 3)
            imageType = BMP_TRUECOLOR;
    }
    // Each scan line MUST have a size that can be divided by a 'long' data type
    if (imageType==BMP_COLOR256)
        palette=(PBYTE)PcxHeader+Pcxsize-768;
    else
        palette = PcxHeader->Palette;

    iPlanes = PcxHeader->ColorPlanes;
    bytesPerLine = PcxHeader->BytesPerLine;
    iScanLineSize = ((iPlanes * bytesPerLine  + sizeof(long) -1) & ~(sizeof(long) -1));
    Height = PcxHeader->YMax - PcxHeader->YMin + 1;
    Width = PcxHeader->XMax - PcxHeader->XMin + 1;

    if (imageType == BMP_UNKNOWN)
        return FALSE;

    clImageSize = Width * Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    bltBuffer = AllocatePool(clImageSize);
//Print (L"ImageType %d, size %d, rect(%d,%d,%d,%d). bltbuffer=%x",
//	imageType, clImageSize, x, y, Width, Height,bltBuffer);
//getch();
    if (!bltBuffer)
        return FALSE;
    {
        UINT32	iX, iY, i, uiValue;
        BYTE line[0x1000];
        DWORD linePos;
        DWORD lPos;
        BYTE color;
        ZeroMem(bltBuffer, clImageSize);

        if (ifTransParent) { //get bitmap from video
            GetImgEx(x , y,  Width, Height,bltBuffer);
        }

        lPos = 0;
        for ( iY=0; iY < Height; iY++ )
        {
            linePos = 0;
            // Decompress the scan line
            for ( iX=0; iX < bytesPerLine*iPlanes; )
            {
                uiValue = PcxImage[lPos++];
                if ( uiValue > 192 )
                {
                    uiValue -= 192;            // Repeat how many times?
                    color = PcxImage[lPos++];  // What color?
                    SetMem(line+linePos, uiValue, color);
                    iX += uiValue;
                    linePos += uiValue;
                }
                else
                {
                    line[linePos++] = (BYTE)uiValue;
                    iX++;
                }
            }

            // Pad the rest with zeros
            if ( linePos < iScanLineSize )
            {
                ZeroMem(line+linePos, iScanLineSize-linePos);
            }

            //将这一行数据转存到bltBuffer中
            for (iX=0;iX<Width;iX++)
            {
                if (imageType == BMP_TRUECOLOR)
                {
                    if (ifTransParent) { //skip transparent color
                        EFI_GRAPHICS_OUTPUT_BLT_PIXEL newcolor;
                        newcolor.Reserved=0;
                        newcolor.Red=line[iX];
                        newcolor.Green=line[iX + bytesPerLine];
                        newcolor.Blue= line[iX + bytesPerLine*2];
                        if (*(DWORD *)(&newcolor)!=COLOR_TRANSPARENT) {
                            *(DWORD*)(bltBuffer + iY*Width + iX) =*(DWORD *)(&newcolor);
                        }
                    } else {
                        bltBuffer[iY*Width+iX].Red =   line[iX];
                        bltBuffer[iY*Width+iX].Green = line[iX + bytesPerLine];
                        bltBuffer[iY*Width+iX].Blue	 = line[iX + bytesPerLine*2];
                    }
                }
                else if (imageType == BMP_COLOR256)
                {
                    if (ifTransParent) { //skip transparent color
                        DWORD newcolor=IndexToColor(line[iX], palette);
                        if (newcolor!=COLOR_TRANSPARENT)
                            *(DWORD*)(bltBuffer + iY*Width + iX)=newcolor;
                    } else
                        *(DWORD*)(bltBuffer + iY*Width + iX) = IndexToColor(line[iX], palette);
                }
            }
        }
        PutImgEx(x,y,Width,Height,bltBuffer);

//PrintXY (8, 32, NULL, NULL, L"Blt returns %x", status);
//getch();
        FreePool(bltBuffer);
    }

    return TRUE;
}

BOOL DrawBmpImage(WORD x, WORD y, BITMAPFILEHEADER *BmpHeader, int bAlpha)
{
	DrawBitmapData(x, y, BmpHeader->biWidth, BmpHeader->biHeight, 
		BmpHeader->biWidth, BmpHeader->biHeight, 
		(PBYTE)BmpHeader + BmpHeader->bfOffBits, 
		BmpHeader->biBitCount==0x20?256:bAlpha);
	return TRUE;
}

BOOL DrawBmpImageEx(WORD x, WORD y, WORD width, WORD height, BITMAPFILEHEADER *BmpHeader, int bAlpha)
{
	DrawBitmapData(x, y, width, height,
		BmpHeader->biWidth, BmpHeader->biHeight,
		(PBYTE)BmpHeader + BmpHeader->bfOffBits,
		BmpHeader->biBitCount == 0x20 ? 256 : bAlpha);
	return TRUE;
}

BOOL DrawBmpImageFilterColorEx(WORD x, WORD y, WORD width, WORD height, BITMAPFILEHEADER *BmpHeader, int bAlpha)
{
	DrawBitmapDataFilterColor(x, y, width, height,
		BmpHeader->biWidth, BmpHeader->biHeight,
		(PBYTE)BmpHeader + BmpHeader->bfOffBits,
		BmpHeader->biBitCount == 0x20 ? 256 : bAlpha);
	return TRUE;
}

CHARBITMAP * FindCharBitmap(WORD encoding)
{
    CHARBITMAP *p = g_font_bitmap;
    INTN n=0;
    while (n < g_font_bitmap_size) {
        //	Print(L"code:p=%x;%x ",p,p->encoding);
        if (p->encoding==encoding)
            return p;
        p++;
        n+=sizeof(CHARBITMAP);
    }
    return NULL;

}

WORD GetLimitTextIndex_1_ex(CHAR16 * pSrcStr, CHAR16 * pTarStr, WORD limitLen, StringInfo * pstStringInfo)
{
	WORD index = 0;
	if (pSrcStr != NULL && pTarStr != NULL)
	{
		WORD Len = (WORD)StrLength(pSrcStr);
		WORD tarIndex = 0;
		WORD textLen = 0;

		while (index < Len)
		{
			textLen += GetCharWidth(pSrcStr[index], pstStringInfo);
			if (textLen <= limitLen)
			{
				pTarStr[tarIndex] = pSrcStr[index];
				tarIndex++;
			}
			else
			{
				//不能作为句子开头的符号  ， 。 ？ ！ ： 、 ；
				if (isCanBeginStr(pSrcStr[index]) == FALSE)
				{
					index = index - 1;
				}
				break;
			}
			index++;
		}
	}
	return index;
}


//WORD GetLimitTextIndex_1(CHAR16 * pSrcStr, CHAR16 * pTarStr, WORD limitLen)
//{
//	WORD index = 0;
//	if (pSrcStr != NULL && pTarStr != NULL)
//	{
//		WORD Len = (WORD)StrLength(pSrcStr);
//		WORD tarIndex = 0;
//		WORD textLen = 0;
//		while (index < Len)
//		{
//			CHARBITMAP * pBitMap = FindCharBitmap(pSrcStr[index]);
//			if (pBitMap != NULL)
//			{
//				textLen += pBitMap->width;
//				if (textLen <= limitLen)
//				{
//					pTarStr[tarIndex] = pSrcStr[index];
//					tarIndex++;
//				}
//				else
//				{
//					break;
//				}
//			}
//			index++;
//		}
//	}
//	return index;
//}


WORD GetLimitTextIndex_2_ex(CHAR16 * pSrcStr, CHAR16 * pTarStr, WORD limitLen, StringInfo * pstStringInfo)
{
	WORD index = 0;

	if (pSrcStr != NULL && pTarStr != NULL)
	{
		CHAR16 endChar = L'\n';
		CHAR16 espaceChar = L' ';
		WORD Len = (WORD)StrLength(pSrcStr);
		WORD tarIndex = 0;
		WORD textLen = 0;
		WORD lastEspaceIndex = 0;

		int iCharWidth = 0;
		int iCharHeight = 0;

		while (index < Len)
		{
			if (pSrcStr[index] == endChar)
			{
				pTarStr[tarIndex] = espaceChar;
				tarIndex++;
				index++;
				break;
			}
			else
			{
				if (pSrcStr[index] == espaceChar)
				{
					lastEspaceIndex = index;
				}

				iCharWidth = GetCharWidth(pSrcStr[index], pstStringInfo);
				//if (pSrcStr[index] == 0x20)
				//{
				//	if (iCharWidth % 8 == 0)
				//	{
				//		iCharWidth = iCharWidth / 2;
				//	}
				//	else
				//	{
				//		iCharWidth = (iCharWidth + 8 - iCharWidth % 8) / 2;
				//	}
				//}

				textLen += iCharWidth;
				if (textLen <= limitLen)
				{
					if (tarIndex == 0 && pSrcStr[index] == espaceChar)
					{

					}
					else
					{
						pTarStr[tarIndex] = pSrcStr[index];
						tarIndex++;
					}
				}
				else
				{
					if (index < Len)  //分词
					{
						if (pSrcStr[index] != espaceChar)
						{
							WORD last = index - lastEspaceIndex;
							pTarStr[tarIndex - last] = 0;
							index = lastEspaceIndex + 1;
						}
					}
					break;
				}
				index++;
			}
		}
	}
	return index;
}

//WORD GetLimitTextIndex_2(CHAR16 * pSrcStr, CHAR16 * pTarStr, WORD limitLen)
//{
//	WORD index = 0;
//	if (pSrcStr != NULL && pTarStr != NULL)
//	{
//		CHAR16 endChar = L'\n';
//		CHAR16 espaceChar = L' ';
//		WORD Len = (WORD)StrLength(pSrcStr);
//		WORD tarIndex = 0;
//		WORD textLen = 0;
//		WORD lastEspaceIndex = 0;
//		while (index < Len)
//		{
//			if (pSrcStr[index] == endChar)
//			{
//				pTarStr[tarIndex] = espaceChar;
//				tarIndex++;
//				index++;
//				break;
//			}
//			else
//			{
//				if (pSrcStr[index] == espaceChar)
//				{
//					lastEspaceIndex = index;
//				}
//				CHARBITMAP * pBitMap = FindCharBitmap(pSrcStr[index]);
//				if (pBitMap != NULL)
//				{
//					textLen += pBitMap->width;
//					if (textLen <= limitLen)
//					{
//						if (tarIndex == 0 && pSrcStr[index] == espaceChar)
//						{
//
//						}
//						else
//						{
//							pTarStr[tarIndex] = pSrcStr[index];
//							tarIndex++;
//						}
//					}
//					else
//					{
//						if (index < Len)  //分词
//						{
//							if (pSrcStr[index] != espaceChar)
//							{
//								WORD last = index - lastEspaceIndex;
//								pTarStr[tarIndex - last] = 0;
//								index = lastEspaceIndex + 1;
//							}
//						}
//						break;
//					}
//				}
//				index++;
//			}
//		}
//	}
//	return index;
//}

FontTextData * GetSpecialTextData(StringInfo * pstStringInfo)
{
	FontTextData *pRet = NULL;
	if (pstStringInfo != NULL)
	{
		
		if (pstStringInfo->enumFontType == FZLTHJW)
		{
			switch (pstStringInfo->iFontSize)
			{
			case 9:
				pRet = &g_font_1_9;
				break;
			case 11:
				pRet = &g_font_1_11;
				break;
			case 12:
				pRet = &g_font_1_12;
				break;
			case 13:
				pRet = &g_font_1_13;
				break;
			case 14:
				pRet = &g_font_1_14;
				break;
			case 15:
				pRet = &g_font_1_15;
				break;
			case 17:
				pRet = &g_font_1_17;
				break;
			case 19:
				pRet = &g_font_1_19;
				break;
			case 20:
				pRet = &g_font_1_20;
				break;
			case 23:
				pRet = &g_font_1_23;
				break;
			case 26:
				pRet = &g_font_1_26;
				break;
			case 27:
				pRet = &g_font_1_27;
				break;
			case 28:
				pRet = &g_font_1_28;
				break;
			case 30:
				pRet = &g_font_1_30;
				break;
			case 34:
				pRet = &g_font_1_34;
				break;
			case 40:
				pRet = &g_font_1_40;
				break;
			default:
				break;
			}
		}
		else if (pstStringInfo->enumFontType == FZLTZHJW)
		{
			switch (pstStringInfo->iFontSize)
			{
			case 9:
				pRet = &g_font_2_9;
				break;
			case 11:
				pRet = &g_font_2_11;
				break;
			case 12:
				pRet = &g_font_2_12;
				break;
			case 13:
				pRet = &g_font_2_13;
				break;
			case 14:
				pRet = &g_font_2_14;
				break;
			case 17:
				pRet = &g_font_2_17;
				break;
			case 18:
				pRet = &g_font_2_18;
				break;
			case 20:
				pRet = &g_font_2_20;
				break;
			case 21:
				pRet = &g_font_2_21;
				break;
			case 23:
				pRet = &g_font_2_23;
				break;
			case 25:
				pRet = &g_font_2_25;
				break;
			case 27:
				pRet = &g_font_2_27;
				break;
			case 30:
				pRet = &g_font_2_30;
				break;
			case 32:
				pRet = &g_font_2_32;
				break;
			case 33:
				pRet = &g_font_2_33;
				break;
			case 38:
				pRet = &g_font_2_38;
				break;
			case 40:
				pRet = &g_font_2_40;
				break;
			case 45:
				pRet = &g_font_2_45;
				break;
			case 50:
				pRet = &g_font_2_50;
				break;
			case 60:
				pRet = &g_font_2_60;
				break;
			case 90:
				pRet = &g_font_2_90;
				break;
			default:
				break;
			}
		}
	}
	return pRet;
}

TextData* GetTextData(WORD encoding, StringInfo * pstStringInfo)
{
	//MyLog(LOG_DEBUG, L"Size:%d font:%d", pstStringInfo->iFontSize, pstStringInfo->enumFontType);
	int iRet = -1;
	int i = 0;
	FontTextData *pFontTextData = GetSpecialTextData(pstStringInfo);
	if (pFontTextData != NULL)
	{
		for (i = 0; i < pFontTextData->realTextDataCout; i++)
		{
			TextData *pTemp = pFontTextData->ppTextDataList[i];
			if (pTemp != NULL)
			{
				if (pTemp->textHeader.text == encoding)
				{
					return pTemp;
				}
			}
		}
	}
	return NULL;
}

int GetCharWidth(WORD encoding, StringInfo * pstStringInfo)
{
	TextData* pTextData = GetTextData(encoding, pstStringInfo);
	if (pTextData != NULL)
	{
		return pTextData->textHeader.width;
	}
	return 0;
}

void PutOutChar(WORD x, WORD y,DWORD color,CHARBITMAP *pf, DWORD dwCharSize)
{
    WORD line,dot;
    for (line = 0; line < dwCharSize; line++) {

        dot=0;
        while (dot < dwCharSize) {
            BYTE c=pf->bitmap[line*dwCharSize /8+dot/8];

            //PRINT 8 DOT
            BYTE mask=0x80;
            WORD index=0;
            //       Print(L"   %x  "  ,c);
            //        getch();
            for (index=0;index<8;index++) {
                if (mask & c) {
                    DrawDot(dot+x+index,line+y,color);
                    //   Print(L" %x %d %d  ",mask,dot+x+index,line+y);
                }
                mask>>=1;

            }
            //    Print(L"next bytes,line %d,dot %d ,c=%d",line, dot,c);
            dot+=8;
        }
    }
}

void PutOutCharEx(WORD x, WORD y, DWORD color, TextData *pTextData)
{
	int i, j;
	if (pTextData != NULL)
	{
		x += pTextData->textHeader.offsetX;
		y += pTextData->textHeader.offsetY;
		int grayTotalValue = 0;
		int grayCout = 0;
		for (i = 0; i < pTextData->textHeader.realHeight; i++)
		{
			for (j = 0; j < pTextData->textHeader.realWidth; j++)
			{
				if (pTextData->pdata[i * pTextData->textHeader.realWidth + j] > 0)
				{
					grayTotalValue += pTextData->pdata[i * pTextData->textHeader.realWidth + j];
					grayCout++;
					DrawDotEx(x + j, y + i, color, pTextData->pdata[i * pTextData->textHeader.realWidth + j]);
				}
			}
		}

//#if OKR_DEBUG
//		MyLog(LOG_DEBUG, L"text=%d, width=%d, heigth=%d, offsetX=%d",
//			pTextData->textHeader.text,
//			pTextData->textHeader.width,
//			pTextData->textHeader.heigth,
//			pTextData->textHeader.offsetX);
//		MyLog(LOG_DEBUG, L"offsetY=%d, realWidth=%d, realHeight=%d, dataLen=%d",
//			pTextData->textHeader.offsetY,
//			pTextData->textHeader.realWidth,
//			pTextData->textHeader.realHeight,
//			pTextData->textHeader.dataLen);
//		MyLog(LOG_DEBUG, L"grayTotalValue=%d, grayCout=%d",
//			grayTotalValue,
//			grayCout);
//#endif

	}
}

void BufferPutOutCharEx(BufferData* pstBuffData, DWORD color, TextData *pTextData)
{
	int i = 0;
	int j = 0;

	if (pTextData != NULL && pTextData->pdata != NULL && pstBuffData != NULL && pstBuffData->pBuffData != NULL)
	{
		EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffDataPos = pstBuffData->pBuffData + pTextData->textHeader.offsetY * pstBuffData->buffWidth + pTextData->textHeader.offsetX;
		if (NULL != buffDataPos)
		{
			int grayTotalValue = 0;
			int grayCout = 0;
			for (i = 0; i < pTextData->textHeader.realHeight; i++)
			{
				for (j = 0; j < pTextData->textHeader.realWidth; j++)
				{
					if (pTextData->pdata[i * pTextData->textHeader.realWidth + j] > 0)
					{
						grayTotalValue += pTextData->pdata[i * pTextData->textHeader.realWidth + j];
						grayCout++;
						BufferDrawDotEx(&(buffDataPos[i * pstBuffData->buffWidth + j]), color, pTextData->pdata[i * pTextData->textHeader.realWidth + j]);
					}
				}
			}
		}
	}
}

void GetCharSize(WORD encoding, StringInfo * pstStringInfo, BYTE *pTextData, int TextDataLength, WORD dwCharSize, WORD *pdwCharWidth, WORD *pdwCharHeight)
{
	if (encoding == 0x20 || 
		(encoding >= 0xFF01 && encoding < 0xFF5E) ||
		encoding == 0x201C ||	
		encoding == 0x201D ||
		encoding == 0x2192 ||
		encoding == 0x3001 ||
		encoding == 0x3002 ||
		encoding == 0x3010 ||
		encoding == 0x3011)
	{
		if (pstStringInfo != NULL)
		{
			*pdwCharWidth = pstStringInfo->iFontSize;
			*pdwCharHeight = pstStringInfo->iFontSize;
		}
		return;
	}

	if (*pdwCharWidth != NULL)
	{
		*pdwCharWidth = 0;
		*pdwCharHeight = 0;
	}

	if (pTextData != NULL)
	{
		WORD line, dot;

		int iTop = -1;
		int iLeft = -1;
		int iRight = -1;
		int iBottom = -1;

		int iTextWidth = 0;
		int iHeight = 0;

		for (line = 0; line < dwCharSize; line++)
		{
			dot = 0;
			while (dot < dwCharSize)
			{
				BYTE c = pTextData[line*dwCharSize / 8 + dot / 8];

				//PRINT 8 DOT
				BYTE mask = 0x80;
				WORD index = 0;

				for (index = 0; index < 8; index++)
				{
					if (mask & c)
					{
						if (iTop == -1)
						{
							iTop = line;
						}

						if (iLeft == -1)
						{
							iLeft = dot + index;
						}

						if (iBottom == -1)
						{
							iBottom = line;
						}

						if (iRight == -1)
						{
							iRight = dot + index;
						}

						if (line < iTop)
						{
							iTop = line;
						}

						if ((dot * 8 + index) < iLeft)
						{
							iLeft = dot + index;
						}

						if (line > iBottom)
						{
							iBottom = line;
						}

						if ((dot + index) > iRight)
						{
							iRight = dot + index;
						}
					}

					mask >>= 1;
				}
				//    Print(L"next bytes,line %d,dot %d ,c=%d",line, dot,c);
				dot += 8;
			}
		}

		*pdwCharWidth = iRight - iLeft + 2;
		*pdwCharHeight = iBottom - iTop + 2;

		//if (*pdwCharWidth == 0)
		//{
		//	*pdwCharWidth = dwCharSize;
		//}

		//if (*pdwCharHeight == 0)
		//{
		//	*pdwCharHeight = dwCharSize;
		//}
	}
}

////pdwStrLen will add plus, so it needs init 0 when call function.
int GetStringLen(CHAR16 * wstring, StringInfo * pstStringInfo, DWORD *pdwStrLen)
{
	//CHARBITMAPEx *pf;
	unsigned int i = 0;

	if (NULL == pstStringInfo)
	{
		return -1;
	}

	if (NULL == pdwStrLen)
	{
		return -1;
	}

	while (i < 10000)
	{
		//	Print(L"process %x ",wstring[i]);
		if (wstring[i] == 0)
			break;

		*pdwStrLen += GetCharWidth(wstring[i], pstStringInfo);
		i++;
	}
	return 0;
}


//DWORD GetNextStringLen(CHAR16 *str , INT32  *Curlen )
//{
//	CHAR16 *tempstr = str;
//	CHARBITMAP *pf;
//	DWORD StrLength =0;
//	WORD next_width = 0;
//	INT32 templen =0;
//	
//	while((*tempstr >= L'a' && *tempstr <= L'z') || (*tempstr >= L'A' || *tempstr <= L'Z'))
//	{
//		if(*tempstr == 0)
//			break;
//		if(*tempstr == L' ')
//			break;
//		if(*tempstr == L'-')
//			break;
//		if(*tempstr == L'\n')
//			break;
//		if(*tempstr == L'/')
//			break;
//		if(*tempstr == L'.')
//			break;
//		//Print(L"-%c",*tempstr);
//		templen++;
//		pf = FindCharBitmap((WORD)*tempstr);
//		
//		if (pf == NULL)
//			{
//				next_width = 8;
//			}
//		else 
//			{
//        	 
//        	next_width = pf->width;
//			}
//			
//		StrLength = StrLength + next_width;
//		tempstr++;
//	}
//	*Curlen = templen;
//	//Print(L"Strwidth=%d templen =%d \n",StrLength,templen);
//	return StrLength ;
//}

BOOL isCanBeginStr(WORD encode)
{
	BOOL bRet = TRUE;
	//不能作为句子开头的符号  ， 。 ？ ！ ： 、 ；
	if (encode == 0x2C ||
		encode == 0xFF0C ||
		encode == 0x2E ||
		encode == 0x3002 ||
		encode == 0x3F ||
		encode == 0xFF1F ||
		encode == 0x21 ||
		encode == 0xFF01 ||
		encode == 0x3A ||
		encode == 0xFF1A ||
		encode == 0x3001 ||
		encode == 0x3B ||
		encode == 0xFF1B
		)
	{
		bRet = FALSE;
	}

	return bRet;
}

// Display utf-16 string to position x,y ,with color
//Display Type: 长度超出的处理方法  0:舍弃剩余字符    1：显示...
int DisplayStringInRectEx(
	WORD x, 
	WORD y, 
	DWORD width, 
	DWORD Height, 
	DWORD color, 
	CHAR16 * wstring, 
	StringInfo * pstStringInfo, 
	WORD displayType)
{
	//MyLog(LOG_DEBUG, L"DisplayStringInRectEx:size %d Font:%d", pstStringInfo->iFontSize, pstStringInfo->enumFontType);

	unsigned int i = 0;

	if (NULL == pstStringInfo)
	{
		return -1;
	}

	DWORD dwWordLen = 0;
	DWORD dwWordHeight = pstStringInfo->iFontSize;

	int iCharWidth = 0;
	int iCharHeight = 0;

	DWORD dwTop = y;
	DWORD dwLeft = x;

	WORD len = (WORD)StrLength(wstring);

	while (i < 10000)
	{
		//	Print(L"process %x ",wstring[i]);
		if (wstring[i] == 0)
			break;


		iCharWidth = GetCharWidth(wstring[i], pstStringInfo);
		if (dwWordLen + iCharWidth < width && wstring[i] != 0x0A)
		{
			PutOutCharEx(x, y, color, GetTextData(wstring[i], pstStringInfo));
			x += iCharWidth;
			dwWordLen += iCharWidth;
		}
		else
		{
			if (isCanBeginStr((WORD)wstring[i]) == FALSE)
			{
				PutOutCharEx(x, y, color, GetTextData(wstring[i], pstStringInfo));
				x += iCharWidth;// + 2;
				dwWordLen += iCharWidth;// + 2;
				i++;
				continue;
			}

			///new line
			if (dwWordHeight + pstStringInfo->iFontSize < Height)
			{
				x = dwLeft;
				dwWordLen = 0;
				y += pstStringInfo->iFontSize + 14 * g_data.fMinscale;
				dwWordHeight += pstStringInfo->iFontSize + 14 *g_data.fMinscale;

				if (wstring[i] == 0x0A)
				{
					//MyLog(LOG_DEBUG, L"wstring[%d] is change line,continue \r\n", i);
					i++;
					continue;
				}

				PutOutCharEx(x, y, color, GetTextData(wstring[i], pstStringInfo));
				x += iCharWidth;// + 2;
				dwWordLen += iCharWidth;// + 2;
			}
			else
			{
				//does not have new line space
				if (displayType == 1)
				{
					if (dwWordLen + pstStringInfo->iFontSize * 3 > width)
					{
						for (int iFlag = 0; iFlag < 3; iFlag++)
						{
							PutOutCharEx(x, y, color, GetTextData(0x2e, pstStringInfo));
							x += iCharWidth;
							dwWordLen += iCharWidth;
						}

						break;
					}
				}
				else
				{
					//does not have sapce for new line
					break;
				}
			}
		}
		i++;
	}
	g_textBKColor = 0xffffff;
	return 0;
}

//#define LINE_SPACE 20
//int DisplayStringInRect(WORD ax, WORD ay, WORD a_width,WORD height, DWORD color, CHAR16 *str)
//{
//    CHARBITMAP *pf;
//	WORD posx = ax;
//	WORD posy = ay;
//	WORD next_width = 0;
//    WORD remainwidth = a_width;
//	WORD remainheight = height;
//    WORD len = (WORD)StrLength(str);
//    INT32 CurStrlen =0;
//    while (*str != 0 && remainheight >= LINE_SPACE )
//    {
//		
//		 
//		if((*str >= L'a' && *str <= L'z') || (*str >= L'A' && *str <= L'Z'))
//		{
//			//Print(L"s=%c ",*str);
//			if(CurStrlen ==0)
//			{
//				if(remainwidth < GetNextStringLen(str,&CurStrlen))
//				{
//				remainwidth = a_width;
//				remainheight -= LINE_SPACE ; //10+16 10  line space
//				posx = ax;
//				posy = posy + LINE_SPACE;	
//				}
//			}
//		}
//		if(remainheight < LINE_SPACE)
//			break;
//		if(CurStrlen>0)
//			CurStrlen--;
//		
//		 if(*str == L'\n')
//		{
//       	    remainwidth = a_width;
//			remainheight -= LINE_SPACE ; //10+16 10  line space
//			posx = ax;
//			posy = posy + LINE_SPACE;
//			
//			len--;
//			str++;
//		
//			continue;
//		}
//		
//        pf = FindCharBitmap((WORD)*str);
//        if (pf == NULL)
//			{
//				next_width = 8;
//			}
//		else 
//			{
//        	PutOutChar(posx,posy,color,pf, BITMAP_SIZE);
//        	next_width = pf->width;
//			}
//       	posx += next_width;
//		
//       	 if(remainwidth < 16)
//		{
//       	    remainwidth = a_width;
//			remainheight -= LINE_SPACE ; //10+16 10  line space
//			posx = ax;
//			posy = posy + LINE_SPACE;
//		}
//		else 
//       	    remainwidth -= next_width;
//       	
//		
//		
//        len--;
//        str++;
//    }
//
//    //if ((g_BackGroundColor != COLOR_TRANSPARENCE)&&(remainwidth != 0))
//    //    FillRect(x, y, remainwidth, FONT_HEIGHT, g_BackGroundColor);	//center
//    return 0;
//}


// Display utf-16 string to position x,y ,with color
//Display Type: 长度超出的处理方法  0:舍弃剩余字符    1：显示...
int DisplayStringInWidthEx(
	WORD x, 
	WORD y, 
	DWORD color, 
	CHAR16 * wstring, 
	StringInfo * pstStringInfo, 
	DWORD width, 
	WORD displayType, 
	BOOL bUseASCIISpaceSize)
{
	//MyLog(LOG_DEBUG, L"DisplayStringInWidthEx width: %d", width);

	unsigned int i = 0;

	if (NULL == pstStringInfo)
	{
		return -1;
	}

	DWORD dwWordLen = 0;

	int iCharWidth = 0;

	WORD len = (WORD)StrLength(wstring);

	WORD dwWordLength = 0;
	GetStringLen(wstring, pstStringInfo, &dwWordLength);

	while (i < 10000)
	{
		//	Print(L"process %x ",wstring[i]);
		if (wstring[i] == 0)
			break;

		iCharWidth = GetCharWidth(wstring[i], pstStringInfo);
		if (displayType == 0)
		{
			if (dwWordLen + iCharWidth < width)
			{
				PutOutCharEx(x, y, color, GetTextData(wstring[i], pstStringInfo));
				x += iCharWidth;
				dwWordLen += iCharWidth;
			}
			else
			{
				break;
			}
		}
		else if (displayType == 1)
		{
			if (dwWordLength > width)
			{
				if (dwWordLen + pstStringInfo->iFontSize * 3 > width)
				{
					for (int iFlag = 0; iFlag < 3; iFlag++)
					{
						PutOutCharEx(x, y, color, GetTextData(0x2e, pstStringInfo));
						x += iCharWidth;
						dwWordLen += iCharWidth;
					}

					break;
				}
			}

			if (dwWordLen + iCharWidth < width)
			{
				PutOutCharEx(x, y, color, GetTextData(wstring[i], pstStringInfo));
				x += iCharWidth;// + 2;
				dwWordLen += iCharWidth;// + 2;
			}
		}

		i++;
	}
	g_textBKColor = 0xffffff;
	return 0;
}


int DisplayStringInWidth(WORD x, WORD y, WORD width, DWORD color, CHAR16 *str)
{
    CHARBITMAP *pf;
    WORD remainwidth = width;
    WORD len = (WORD)StrLength(str);

    while (*str != 0 && remainwidth >= 8)
    {
        pf = FindCharBitmap((WORD)*str);
        if (pf == NULL) {
        	width = 8;
		} else {
        	PutOutChar(x,y,color,pf, BITMAP_SIZE);
        	width = pf->width;
       	}
       	x += width;
       	if (remainwidth > width)
       	    remainwidth -= width;
       	else
       	    remainwidth = 0;
        len--;
        str++;
    }

    //if ((g_BackGroundColor != COLOR_TRANSPARENCE)&&(remainwidth != 0))
    //    FillRect(x, y, remainwidth, FONT_HEIGHT, g_BackGroundColor);	//center
    return 0;
}

// Display utf-16 string to position x,y ,with color
int DisplayString(WORD x,WORD y, DWORD color,CHAR16 * wstring)
{
    CHARBITMAP *pf;
    unsigned int i=0;
    if (!g_font_bitmap)
        return -1;

    while (i<10000) {
        //	Print(L"process %x ",wstring[i]);
        if (wstring[i]==0)
            break;
        pf=FindCharBitmap((WORD)wstring[i]);
        if (pf) {
			PutOutChar(x,y,color,pf, BITMAP_SIZE);
			x += pf->width;
        } else {
			x += 8;
		}
        i++;
    }
    return 0;
}

void SetTextBKColor(DWORD color)
{
	g_textBKColor = color;
}

// Display utf-16 string to position x,y ,with color
int DisplayStringEx(
	WORD x, 
	WORD y, 
	DWORD color, 
	CHAR16 * wstring, 
	StringInfo * pstStringInfo, 
	BOOL bUseASCIISpaceSize)
{
	//MyLog(LOG_DEBUG, L"DisplayStringEx: %s", wstring);
	//MyLog(LOG_DEBUG, L"DisplayStringEx:size %d Font:%d", pstStringInfo->iFontSize, pstStringInfo->enumFontType);

	//CHARBITMAPEx *pf;
	unsigned int i = 0;

	if (NULL == pstStringInfo)
	{
		return -1;
	}
	while (i < 10000)
	{
		//	Print(L"process %x ",wstring[i]);
		if (wstring[i] == 0)
			break;

		PutOutCharEx(x, y, color, GetTextData(wstring[i], pstStringInfo));
		x += GetCharWidth(wstring[i], pstStringInfo);

		i++;
	}
	g_textBKColor = 0xffffff;
	return 0;
}

// Display utf-16 string to position x,y ,with color
int BufferDisplayStringEx(
	BufferData* pstBuffData,
	WORD x,
	WORD y,
	DWORD color,
	CHAR16 * wstring,
	StringInfo * pstStringInfo,
	BOOL bUseASCIISpaceSize)
{
	unsigned int i = 0;

	if (NULL == pstStringInfo || NULL == pstBuffData || NULL == pstBuffData->pBuffData)
	{
		return -1;
	}

	WORD xOri = x;
	WORD yOri = y;

	EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffDataPos = pstBuffData->pBuffData;

	while (i < 10000)
	{
		if (wstring[i] == 0)
		{
			break;
		}

		TextData *pTextData = GetTextData(wstring[i], pstStringInfo);
		if (NULL != pTextData)
		{
			BufferPutOutCharEx(pstBuffData, color, pTextData);

			x += pTextData->textHeader.width;
			pstBuffData->pBuffData += pTextData->textHeader.width;
		}
		i++;
	}

	pstBuffData->pBuffData = buffDataPos;

	g_textBKColor = 0xffffff;
	return 0;
}

// Display utf-16 string to position x,y ,with color
int DisplayChar(WORD x,WORD y, DWORD color,CHAR16 ch, int *pCharSize, StringInfo *pstStringInfo)
{
    CHAR16 buff[2];
    buff[0]=ch;
    buff[1]=L'\0';

	PutOutCharEx(x, y, color, GetTextData(buff[0], pstStringInfo));
	*pCharSize = GetCharWidth(buff[0], pstStringInfo);

	//MyLog(LOG_DEBUG, L"DisplayChar: w: %d  h: %d", iCharWidth, iCharHeight);
	return 0;// DisplayStringEx(x, y, color, buff, pstStringInfo);
}

void GetBmpRect(DWORD fileid, RECT *rc)
{
    PIMAGE_FILE_INFO pfile;
    BITMAPFILEHEADER *bmpHeader;
    PCXHEAD			*pcxHeader;

    pfile = FindImgFileFromName(fileid);
    
    if (pfile->type == IMG_FILE_PCX)	//pcx
    {
    	pcxHeader = (PCXHEAD *)pfile->pmem;
        rc->width = pcxHeader->XMax - pcxHeader->XMin; 
        rc->height = pcxHeader->YMax - pcxHeader->YMin;
    }
    else if(pfile->type == IMG_FILE_BMP)	//bmp
    {
    	bmpHeader = (BITMAPFILEHEADER *)pfile->pmem;
        rc->width = bmpHeader->biWidth; 
        rc->height = bmpHeader->biHeight;
    }
    else if(pfile->type == IMG_FILE_JPG)	//jpg
    {
    	JPGHEAD		jpgheader;
    	ExtractJpegData(pfile->pmem, &jpgheader, NULL);
        rc->width = jpgheader.Width; 
        rc->height = jpgheader.Height;
	}
    else if(pfile->type >= IMG_FILE_LZ77)	//== IMG_FILE_PCX_LZ77 || pfile->type == IMG_FILE_BMP_LZ77)
    {
    	PVOID bmp;
    	DWORD bmpsize = pfile->size;
    	bmp = ExtractZipFile(pfile->pmem, &bmpsize);
    	if(bmp)
    	{
    		if(pfile->type == IMG_FILE_PCX_LZ77)
    		{
    			pcxHeader = (PCXHEAD *)bmp;
        		rc->width = pcxHeader->XMax - pcxHeader->XMin; 
        		rc->height = pcxHeader->YMax - pcxHeader->YMin;
    		}
    		else if(pfile->type == IMG_FILE_BMP_LZ77)
    		{
		    	bmpHeader = (BITMAPFILEHEADER *)bmp;
        		rc->width = bmpHeader->biWidth; 
		        rc->height = bmpHeader->biHeight;
    		}
    		else
    		{
    			JPGHEAD		jpgheader;
    			ExtractJpegData(bmp, &jpgheader, NULL);
        		rc->width = jpgheader.Width; 
        		rc->height = jpgheader.Height;
    		}
    		ReleaseBmp(bmp);
    	}
    	else
    	{
			if(g_data.bDebug)
				MyLog(LOG_ERROR,L"GetBmpRect:Extract(%d) failed.", pfile->id);
    	}
    }	
}

//在指定矩形框里显示图形，自动伸缩, 目前仅仅处理jpg图形的伸缩
//INTN DisplayImgInRect(WORD x, WORD y, WORD width, WORD height, DWORD fileid, 
//		BOOLEAN bCenter)
//{
//    BOOLEAN ret;
//    PIMAGE_FILE_INFO pfile;
//
//    pfile = FindImgFileFromName(fileid);
//
//    if(pfile->type >= IMG_FILE_LZ77)	//== IMG_FILE_PCX_LZ77 || pfile->type == IMG_FILE_BMP_LZ77)
//    {
//    	PVOID bmp;
//    	DWORD bmpsize = pfile->size;
//    	bmp = ExtractZipFile(pfile->pmem, &bmpsize);
//    	if(bmp)
//    	{
//    		if(pfile->type == IMG_FILE_PCX_LZ77)
//    		{
//    			ret = DrawPCXImage(x, y, bmp, bmpsize, 0);
//    		}
//    		else if(pfile->type == IMG_FILE_BMP_LZ77)
//    		{
//				BITMAPFILEHEADER *BmpHeader = (BITMAPFILEHEADER *)bmp;
//				x += (width > BmpHeader->biWidth)?(width - BmpHeader->biWidth)/2:0;
//				y += (height > BmpHeader->biHeight)?(height - BmpHeader->biHeight)/2:0;
//				ret = DrawBmpImage(x, y, bmp, 0);
//    		}
//    		else
//    		{
//				ret = DrawJpegData(x, y, width, height, bmp, 0);
//    		}
//    		ReleaseBmp(bmp);
//    	}
//#if OKR_DEBUG
//    	else
//    	{
//    		MyLog(LOG_DEBUG,L"DisplayImgInRect, extract(%d) failed.", pfile->id);
//    	}
//#endif
//    }
//    else
//    {
//		if(!bCenter)
//		{
//	    	ret = DrawJpegData(x, y, width, height, pfile->pmem, 0);
//	    }
//    }
//
//    if (ret)
//        return 0;
//    else
//        return -1;
//}

INTN DisplayImg(WORD x, WORD y, DWORD fileid, BOOLEAN transparent)
{
    BOOLEAN ret;
    PIMAGE_FILE_INFO pfile;

    pfile = FindImgFileFromName(fileid); 

    ASSERT(pfile);
    ASSERT(pfile->pmem);
//    Print(L"Find file in mem %x ",pfile->pmem);
    if (pfile->type == IMG_FILE_PCX)	//pcx
        ret = DrawPCXImage( x,  y,  pfile->pmem, pfile->size,transparent);
    else if(pfile->type == IMG_FILE_BMP)	//bmp
    	ret = DrawBmpImage( x, y, pfile->pmem, (int)transparent);
    else if(pfile->type == IMG_FILE_JPG)	//jpg
        ret = DrawJpegData( x, y, 0, 0, pfile->pmem, (int)transparent);
    else if(pfile->type >= IMG_FILE_LZ77)	//== IMG_FILE_PCX_LZ77 || pfile->type == IMG_FILE_BMP_LZ77)
    {
    	PVOID bmp;
    	DWORD bmpsize = pfile->size;
    	bmp = ExtractZipFile(pfile->pmem, &bmpsize);
    	if(bmp)
    	{
    		if(pfile->type == IMG_FILE_PCX_LZ77)
    			ret = DrawPCXImage(x, y, bmp, bmpsize, transparent);
    		else if(pfile->type == IMG_FILE_BMP_LZ77)
    			ret = DrawBmpImage(x, y, bmp, (int)transparent);
    		else
    			ret = DrawJpegData(x, y, 0, 0, bmp, (int)transparent);
    		ReleaseBmp(bmp);
    	}
#if OKR_DEBUG
    	else
    	{
    		MyLog(LOG_DEBUG,L"DisplayImg, extract(%d) failed.", pfile->id);
    		getch();
    	}
#endif
    }

    if (ret)
        return 0;
    else
        return -1;
}

INTN DisplayImgEx(WORD x, WORD y, WORD Width, WORD Height, DWORD fileid, BOOLEAN transparent)
{
	//MyLog(LOG_MESSAGE, L"DisplayImgEx x: %d y: %d Width: %d Height: %d", x, y, Width, Height);
	//MyLog(LOG_MESSAGE, L"fileid: %d", fileid);

	BOOLEAN ret;
	PIMAGE_FILE_INFO pfile;

	pfile = FindImgFileFromName(fileid);

	ASSERT(pfile);
	ASSERT(pfile->pmem);
	//MyLog(LOG_DEBUG, L"Find file in mem %x ", pfile->pmem);
	if (pfile->type == IMG_FILE_PCX)	//pcx
		ret = DrawPCXImage(x, y, pfile->pmem, pfile->size, transparent);
	else if (pfile->type == IMG_FILE_BMP)	//bmp
		ret = DrawBmpImageEx(x, y, Width, Height, pfile->pmem, (int)transparent);
	else if (pfile->type == IMG_FILE_JPG)	//jpg
		ret = DrawJpegData(x, y, 0, 0, pfile->pmem, (int)transparent);
	else if (pfile->type >= IMG_FILE_LZ77)	//== IMG_FILE_PCX_LZ77 || pfile->type == IMG_FILE_BMP_LZ77)
	{
		PVOID bmp;
		DWORD bmpsize = pfile->size;
		bmp = ExtractZipFile(pfile->pmem, &bmpsize);
		if (bmp)
		{
			if (pfile->type == IMG_FILE_PCX_LZ77)
				ret = DrawPCXImage(x, y, bmp, bmpsize, transparent);
			else if (pfile->type == IMG_FILE_BMP_LZ77)
				ret = DrawBmpImageEx(x, y, Width, Height, bmp, (int)transparent);
			else
				ret = DrawJpegData(x, y, 0, 0, bmp, (int)transparent);
			ReleaseBmp(bmp);
		}
#if OKR_DEBUG
		else
		{
			MyLog(LOG_DEBUG, L"DisplayImg, extract(%d) failed.", pfile->id);
			getch();
		}
#endif
	}

	if (ret)
		return 0;
	else
		return -1;
}

INTN DisplayImgFilterColorEx(WORD x, WORD y, WORD Width, WORD Height, DWORD fileid, BOOLEAN transparent)
{
	//MyLog(LOG_MESSAGE, L"DisplayImgFilterColorEx x: %d y: %d Width: %d Height: %d", x, y, Width, Height);

	BOOLEAN ret;
	PIMAGE_FILE_INFO pfile;

	pfile = FindImgFileFromName(fileid);

	ASSERT(pfile);
	ASSERT(pfile->pmem);
	//MyLog(LOG_DEBUG, L"Find file in mem %x ", pfile->pmem);
	if (pfile->type == IMG_FILE_PCX)	//pcx
		ret = DrawPCXImage(x, y, pfile->pmem, pfile->size, transparent);
	else if (pfile->type == IMG_FILE_BMP)	//bmp
		ret = DrawBmpImageFilterColorEx(x, y, Width, Height, pfile->pmem, (int)transparent);
	else if (pfile->type == IMG_FILE_JPG)	//jpg
		ret = DrawJpegData(x, y, 0, 0, pfile->pmem, (int)transparent);
	else if (pfile->type >= IMG_FILE_LZ77)	//== IMG_FILE_PCX_LZ77 || pfile->type == IMG_FILE_BMP_LZ77)
	{
		PVOID bmp;
		DWORD bmpsize = pfile->size;
		bmp = ExtractZipFile(pfile->pmem, &bmpsize);
		if (bmp)
		{
			if (pfile->type == IMG_FILE_PCX_LZ77)
				ret = DrawPCXImage(x, y, bmp, bmpsize, transparent);
			else if (pfile->type == IMG_FILE_BMP_LZ77)
				ret = DrawBmpImageFilterColorEx(x, y, Width, Height, bmp, (int)transparent);
			else
				ret = DrawJpegData(x, y, 0, 0, bmp, (int)transparent);
			ReleaseBmp(bmp);
		}
#if OKR_DEBUG
		else
		{
			MyLog(LOG_DEBUG, L"DisplayImg, extract(%d) failed.", pfile->id);
			getch();
		}
#endif
	}

	if (ret)
		return 0;
	else
		return -1;
}

VOID PutImgEx(WORD x, WORD y, WORD Width, WORD Height,VOID *buffer)
{
    if (g_vga)
        g_vga->Blt(g_vga, buffer, EfiBltBufferToVideo, 0,0, x, y, Width, Height, 0);
}

VOID GetImgEx(WORD x ,WORD y, WORD Width,WORD Height,VOID *buffer)
{
    if (g_vga)
        g_vga->Blt(g_vga, buffer, EfiBltVideoToBltBuffer, x,y, 0, 0, Width, Height, 0);
}

void ClearScreen()
{
	BYTE bMouseDisplay;
	bMouseDisplay = hidemouse();

	DrawLine(0, 0, g_WindowWidth, g_WindowHeight, COLOR_DEFAULT_BACKGROUND);
	 
	if(bMouseDisplay)
		showmouse();
	g_data.bRefeshDebugInfo = TRUE;
}

int CalcFontSize(int iFontSize)
{
	return (int)(iFontSize * g_data.fFontScale + 0.5);
}