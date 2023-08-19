/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_EOI   0xd9
#define M_SOS   0xda
#define M_DQT   0xdb
#define M_DRI   0xdd
#define M_APP0  0xe0


#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */


typedef struct {
	WORD  X;                //4 资源放置的左上角X坐标
	WORD  Y;                //6 资源放置的左上角Y坐标
	WORD  Width;            //4 宽度
	WORD  Height;           //6 高度
}JPGHEAD, *PJPGHEAD;

//ExtractJpegData将jpg文件数据解压成bitmap格式的图形，由于bitmap仅仅是像素点信息，需要一个描述符，所以需要提供一个
//JpgHeader来保存图像宽度、高度等信息
BOOL ExtractJpegData(PBYTE JpgData, JPGHEAD *JpgHeader, PBYTE *ppBitmap);

//DrawBitmapData是在指定位置上以Bitmap格式的图形数据进行绘画
BOOL DrawBitmapData(int x, int y, int width, int height, int srcwidth, int srcheight, PBYTE pBitmap, int bAlpha);

////只显示某种颜色(白色)，其余不显示，实现图片的透明度。
BOOL DrawBitmapDataFilterColor(int x, int y, int width, int height, int srcwidth, int srcheight, PBYTE pBitmap,int bAlpha);

//DrawJpegData是在指定位置上以jpeg格式的图形数据进行绘画，等于先解压jpeg数据，然后调用DrawBitmapData
BOOL DrawJpegData(int x, int y, int width, int height, PBYTE JpegData, int bAlpha);
