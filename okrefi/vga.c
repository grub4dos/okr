//vga.c about the monitor display method 
#include "okr.h"

#include "fonts.h"
#include "res.h"

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
typedef struct {
    WORD encoding;
    BYTE width;
    BYTE bitmap[BITMAP_SIZE*BITMAP_SIZE/8];
} CHARBITMAP;

#pragma pack()

//字库文件在内存中的指针
CHARBITMAP * g_font_bitmap = NULL;
INTN  g_font_bitmap_size = 0;

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

#define X_RES 1920
#define Y_RES 1080
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
    UINT32							vgamode[5];	//存储1280*1024, 1024*768, 1280*800, 1600*900 1366*768
	WORD							width, height;
    UINTN							infoLength;
    UINT32                          horizontalResolution;
    UINT32                          verticalResolution;
    UINT32                          colorDepth;
    UINT32                          refreshRate;

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
        MyLog(LOG_DEBUG, L"MaxMode %d, Mode: %d, Info %p, infosize %d, FrameBufferBase %lx, FrameBufferSize %x\r\n",
        	graphicsOutput->Mode->MaxMode, graphicsOutput->Mode->Mode,
        	graphicsOutput->Mode->Info, graphicsOutput->Mode->SizeOfInfo,
        	graphicsOutput->Mode->FrameBufferBase, graphicsOutput->Mode->FrameBufferSize
        	);

        vgamode[0] = vgamode[1] = vgamode[2] = vgamode[3] = vgamode[4] = -1;
		width = height = 0xffff;
        maxMode = graphicsOutput->Mode->MaxMode;
        for (modeNumber=0;modeNumber<maxMode;modeNumber++)
        {
            status = graphicsOutput->QueryMode(graphicsOutput, modeNumber, &infoLength, &info);
            if (!EFI_ERROR (status))
            {
                MyLog(LOG_DEBUG, L"Mode %d: %d*%d, PixelsPerScanLine %d, Format %x, ver %x\r\n",
                	modeNumber,
                	info->HorizontalResolution, info->VerticalResolution,
                	info->PixelsPerScanLine,
                	info->PixelFormat,
                	info->Version
                	//info->PixelInformation.RedMask, info->PixelInformation.GreenMask,
                	//info->PixelInformation.BlueMask, info->PixelInformation.ReservedMask
                	);
                //getch();
                if (info->HorizontalResolution == X_RES && info->VerticalResolution == Y_RES)
                {
                    g_VgaModeNumber = (WORD)modeNumber;
                    g_BitsPerPixel = 32;
                    g_WindowWidth = X_RES;
                    g_WindowHeight = Y_RES;
                    break;
                }
                else if (info->HorizontalResolution == 1600 && info->VerticalResolution == 900)
                    vgamode[0] = modeNumber;
				else if (info->HorizontalResolution == 1366 && info->VerticalResolution == 768)
                    vgamode[1] = modeNumber;
                else if (info->HorizontalResolution == 1280 && info->VerticalResolution == 800)
                    vgamode[2] = modeNumber;
                else if (info->HorizontalResolution == 1024 && info->VerticalResolution == 768)
                    vgamode[3] = modeNumber;
				else if (info->HorizontalResolution > 1024 && info->VerticalResolution > 768) {
					if(vgamode[4] == -1 || (width >= (WORD)info->HorizontalResolution && height >= info->VerticalResolution)) {
						width = (WORD)info->HorizontalResolution;
						height = (WORD)info->VerticalResolution;
						vgamode[4] = modeNumber;
					}
				}
				if(vgamode[0] != -1 && vgamode[1] != -1 && vgamode[2] != -1 && vgamode[3] != -1 && vgamode[4] != -1)
					break;
				FreePool(info);
            }
            else
            {
				MyLog(LOG_ERROR, L"Failed to query mode %d with error 0x%x\r\n", modeNumber, status);
                break;
            }
        }

        if (!g_WindowWidth)
        {
            //说明没有找到我们想要的分辨率，则从备选的四个中选择一个
            for (modeNumber=0;modeNumber<sizeof(vgamode)/sizeof(UINT32);modeNumber++)
            {
                if (vgamode[modeNumber] != -1)
                {
                    g_VgaModeNumber = (WORD)vgamode[modeNumber];
                    g_BitsPerPixel = 32;
                    if (modeNumber == 0)
                    {
                        g_WindowWidth = 1600;
                        g_WindowHeight = 900;
                    }
					 else if (modeNumber == 1)
                    {
                        g_WindowWidth = 1366;
                        g_WindowHeight = 768;
                    }
                    else if (modeNumber == 2)
                    {
                        g_WindowWidth = 1280;
                        g_WindowHeight = 800;
                    }
                    else if (modeNumber == 3)
                    {
                        g_WindowWidth = 1024;
                        g_WindowHeight = 768;
                    }
					else if (modeNumber == 4)
					{
						g_WindowWidth = width;
						g_WindowHeight = height;
					}
                    break;
                }
            }
        }

		if (!g_WindowWidth) {
			//说明没有找到合适的分辨率
			PrintLogBuffer();
			return FALSE;
		}

        status = SwitchGraphicsMode(TRUE);
        MyLog(LOG_DEBUG,L"SwitchGraphicsMode returns 0x%x\n", status);
        status = graphicsOutput->SetMode(graphicsOutput, g_VgaModeNumber);
        if (EFI_ERROR (status))
        {
        	MyLog(LOG_ERROR,L"Setmode(%d) failed with error 0x%x\r\n", g_VgaModeNumber, status);
			return FALSE;
        }

        g_vga = graphicsOutput;
        if (!g_vgabuffer)
            g_vgabuffer = AllocatePool(VGA_BUFSIZE);
		if (!g_vgabuffer)
			return FALSE;
    }

    {
    	//GetBmpRect(IMG_FILE_LOGO, &g_data.rWnd);
    	g_data.rWnd.width = 808;
    	g_data.rWnd.height = 490;
    	g_data.rWnd.x = (g_WindowWidth - g_data.rWnd.width)/2;	//窗口的x坐标
    	g_data.rWnd.y = (g_WindowHeight - g_data.rWnd.height)/2;	//窗口的y坐标
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

//load pcx and font bitmap to memory
// device: the disk ,curdir: the program running directory,with '\' ending
INTN LoadResource()
{
    INTN size;
    INTN index=0;
    PBYTE pRes;

	//解压fonts
	{
    	PVOID data;
    	DWORD datasize = (DWORD)sizeof(g_fonts);
    	data = ExtractZipFile((PBYTE)g_fonts, &datasize);
    	g_font_bitmap = myalloc(datasize);
    	CopyMem(g_font_bitmap, data, datasize);
    	g_font_bitmap_size = datasize;
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
VOID DrawLinearRect(DWORD begincolor,DWORD endcolor,WORD x,WORD y,WORD width,WORD height)
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
void PutOutChar(WORD x, WORD y,DWORD color,CHARBITMAP *pf)
{
    WORD line,dot;
    for (line = 0; line < BITMAP_SIZE; line++) {

        dot=0;
        while (dot <BITMAP_SIZE) {
            BYTE c=pf->bitmap[line*BITMAP_SIZE/8+dot/8];

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

DWORD GetNextStringLen(CHAR16 *str , INT32  *Curlen )
{
	CHAR16 *tempstr = str;
	CHARBITMAP *pf;
	DWORD StrLength =0;
	WORD next_width = 0;
	INT32 templen =0;
	
	while((*tempstr >= L'a' && *tempstr <= L'z') || (*tempstr >= L'A' || *tempstr <= L'Z'))
	{
		if(*tempstr == 0)
			break;
		if(*tempstr == L' ')
			break;
		if(*tempstr == L'-')
			break;
		if(*tempstr == L'\n')
			break;
		if(*tempstr == L'/')
			break;
		if(*tempstr == L'.')
			break;
		//Print(L"-%c",*tempstr);
		templen++;
		pf = FindCharBitmap((WORD)*tempstr);
		
		if (pf == NULL)
			{
				next_width = 8;
			}
		else 
			{
        	 
        	next_width = pf->width;
			}
			
		StrLength = StrLength + next_width;
		tempstr++;
	}
	*Curlen = templen;
	//Print(L"Strwidth=%d templen =%d \n",StrLength,templen);
	return StrLength ;
}

#define LINE_SPACE 20
int DisplayStringInRect(WORD ax, WORD ay, WORD a_width,WORD height, DWORD color, CHAR16 *str)
{
    CHARBITMAP *pf;
	WORD posx = ax;
	WORD posy = ay;
	WORD next_width = 0;
    WORD remainwidth = a_width;
	WORD remainheight = height;
    WORD len = (WORD)StrLength(str);
    INT32 CurStrlen =0;
    while (*str != 0 && remainheight >= LINE_SPACE )
    {
		
		 
		if((*str >= L'a' && *str <= L'z') || (*str >= L'A' && *str <= L'Z'))
		{
			//Print(L"s=%c ",*str);
			if(CurStrlen ==0)
			{
				if(remainwidth < GetNextStringLen(str,&CurStrlen))
				{
				remainwidth = a_width;
				remainheight -= LINE_SPACE ; //10+16 10  line space
				posx = ax;
				posy = posy + LINE_SPACE;	
				}
			}
		}
		if(remainheight < LINE_SPACE)
			break;
		if(CurStrlen>0)
			CurStrlen--;
		
		 if(*str == L'\n')
		{
       	    remainwidth = a_width;
			remainheight -= LINE_SPACE ; //10+16 10  line space
			posx = ax;
			posy = posy + LINE_SPACE;
			
			len--;
			str++;
		
			continue;
		}
		
        pf = FindCharBitmap((WORD)*str);
        if (pf == NULL)
			{
				next_width = 8;
			}
		else 
			{
        	PutOutChar(posx,posy,color,pf);
        	next_width = pf->width;
			}
       	posx += next_width;
		
       	 if(remainwidth < 16)
		{
       	    remainwidth = a_width;
			remainheight -= LINE_SPACE ; //10+16 10  line space
			posx = ax;
			posy = posy + LINE_SPACE;
		}
		else 
       	    remainwidth -= next_width;
       	
		
		
        len--;
        str++;
    }

    //if ((g_BackGroundColor != COLOR_TRANSPARENCE)&&(remainwidth != 0))
    //    FillRect(x, y, remainwidth, FONT_HEIGHT, g_BackGroundColor);	//center
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
        	PutOutChar(x,y,color,pf);
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
			PutOutChar(x,y,color,pf);
			x += pf->width;
        } else {
			x += 8;
		}
        i++;
    }
    return 0;
}

int DisplayStringA(WORD x,WORD y, DWORD color, char * string)
{
    CHARBITMAP *pf;
    unsigned int i=0;
    if (!g_font_bitmap)
        return -1;

    while (i<10000) {
        if (string[i]==0)
            break;
        pf = FindCharBitmap((WORD)string[i]);
        if (pf) {
			PutOutChar(x,y,color,pf);
			x+=pf->width;
		} else {
			x += 8;
		}

        i++;
    }
    return 0;
}

// Display utf-16 string to position x,y ,with color
int DisplayChar(WORD x,WORD y, DWORD color,CHAR16 ch)
{
    CHAR16 buff[2];
    buff[0]=ch;
    buff[1]=L'\0';

    return DisplayString(x,y,color,buff);
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
INTN DisplayImgInRect(WORD x, WORD y, WORD width, WORD height, DWORD fileid, 
		BOOLEAN bCenter)
{
    BOOLEAN ret;
    PIMAGE_FILE_INFO pfile;

    pfile = FindImgFileFromName(fileid);

    if(pfile->type >= IMG_FILE_LZ77)	//== IMG_FILE_PCX_LZ77 || pfile->type == IMG_FILE_BMP_LZ77)
    {
    	PVOID bmp;
    	DWORD bmpsize = pfile->size;
    	bmp = ExtractZipFile(pfile->pmem, &bmpsize);
    	if(bmp)
    	{
    		if(pfile->type == IMG_FILE_PCX_LZ77)
    		{
    			ret = DrawPCXImage(x, y, bmp, bmpsize, 0);
    		}
    		else if(pfile->type == IMG_FILE_BMP_LZ77)
    		{
				BITMAPFILEHEADER *BmpHeader = (BITMAPFILEHEADER *)bmp;
				x += (width > BmpHeader->biWidth)?(width - BmpHeader->biWidth)/2:0;
				y += (height > BmpHeader->biHeight)?(height - BmpHeader->biHeight)/2:0;
				ret = DrawBmpImage(x, y, bmp, 0);
    		}
    		else
    		{
				ret = DrawJpegData(x, y, width, height, bmp, 0);
    		}
    		ReleaseBmp(bmp);
    	}
#if OKR_DEBUG
    	else
    	{
    		MyLog(LOG_DEBUG,L"DisplayImgInRect, extract(%d) failed.", pfile->id);
    	}
#endif
    }
    else
    {
		if(!bCenter)
		{
	    	ret = DrawJpegData(x, y, width, height, pfile->pmem, 0);
	    }
    }

    if (ret)
        return 0;
    else
        return -1;
}

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

	// DrawLine(0, 0, g_WindowWidth, 85 , 0x5B8AD1);//COLOR_TITLE
  //  DrawLine(0, 85, g_WindowWidth, 165 , 0x2F557C  );//COLOR_TITLE

	//DrawLine(0, 0, g_WindowWidth, g_WindowHeight / 3 - 40-48, 0x2F557C);//COLOR_TITLE
 // DrawLine(0, g_WindowHeight / 3 - 40 - 48, g_WindowWidth, g_WindowHeight * 2 / 3 + 20 , 0xE9F1F4  ); //COLOR_CLIENT
 // DrawLine(0, g_WindowHeight * 2 / 3 + 20, g_WindowWidth, g_WindowHeight * 2 / 3 + 50, 0x5B8AD1); //COLOR_CLIENT

	WORD  four = 1;

	MyLog(LOG_DEBUG, L"g_WindowWidth: %d, g_WindowHeight: %d\r\n", g_WindowWidth, g_WindowHeight);

	if (g_WindowHeight <= 800)
	{
		four = 0;
	}

	if ( four )
	{
		//1,2,34,3  the index declare the draw order  ; add by wh
		DrawLine(0, 0, g_WindowWidth, (g_WindowHeight / 3 - 60) / 2 -10, COLOR_TITLE_L_BLUE);//1 top info rectangle = light blue

		DrawLine(0, (g_WindowHeight / 3 - 60) / 2 - 10, g_WindowWidth, g_WindowHeight / 3 - 40 - 80, COLOR_SEC_BLUE );//2 the second rectangle
		
																													  //3~4 the all rectangle to the bottom  = light blue
	    DrawLine(0, g_WindowHeight   / 3 - 40, g_WindowWidth, g_WindowHeight * 2 / 3 + 40, COLOR_BUTTOM_L_BLUE); 

        //3 re-bitblt the main part （middle while color） =》there is a bug for draw buttom after main part  
		DrawLine(0, (g_WindowHeight / 3 - 60) / 2 + 70, g_WindowWidth, g_WindowHeight * 2 / 3+20  , COLOR_CLIENT);
		

		//DisplayImg((g_WindowWidth - 500) / 2 + 30 + 10, g_WindowHeight - 65, IMAG_FILE_COYPY_RIGHT, 0); //  copyright.bmp
	} 
	else
	{
		DrawLine(0, 0, g_WindowWidth, g_WindowHeight, COLOR_CLIENT);//2 the second rectangle
		DrawLine(0, 0, g_WindowWidth, g_WindowHeight / 3 - 40 - 126, COLOR_SEC_BLUE);//2 the second rectangle
	}


	//DrawLine(0, 0, g_WindowWidth,  50, 0x0000FC);//COLOR_TITLE
	//DrawLine(0, 50, g_WindowWidth, 80, 0x002F5C);//COLOR_TITLE
	//DrawLine(0, g_WindowHeight / 3 - 40 - 80-30, g_WindowWidth, g_WindowHeight / 3 - 40-80, 0x2F0000);//COLOR_TITLE
 	//DrawLine(0, g_WindowHeight / 3 - 40 - 80, g_WindowWidth, g_WindowHeight *2/ 3 - 40, 0x2F557C  );//COLOR_TITLE
 
//	DrawLine(0, 550, g_WindowWidth, 570, 0x5B8AD1 ); //COLOR_CLIENT
	
	 
	if(bMouseDisplay)
		showmouse();
	g_data.bRefeshDebugInfo = TRUE;
}
