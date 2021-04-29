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
	WORD  X;                //4 ��Դ���õ����Ͻ�X����
	WORD  Y;                //6 ��Դ���õ����Ͻ�Y����
	WORD  Width;            //4 ���
	WORD  Height;           //6 �߶�
}JPGHEAD, *PJPGHEAD;

//ExtractJpegData��jpg�ļ����ݽ�ѹ��bitmap��ʽ��ͼ�Σ�����bitmap���������ص���Ϣ����Ҫһ����������������Ҫ�ṩһ��
//JpgHeader������ͼ���ȡ��߶ȵ���Ϣ
BOOL ExtractJpegData(PBYTE JpgData, JPGHEAD *JpgHeader, PBYTE *ppBitmap);

//DrawBitmapData����ָ��λ������Bitmap��ʽ��ͼ�����ݽ��л滭
BOOL DrawBitmapData(int x, int y, int width, int height, int srcwidth, int srcheight, PBYTE pBitmap, int bAlpha);

//DrawJpegData����ָ��λ������jpeg��ʽ��ͼ�����ݽ��л滭�������Ƚ�ѹjpeg���ݣ�Ȼ�����DrawBitmapData
BOOL DrawJpegData(int x, int y, int width, int height, PBYTE JpegData, int bAlpha);
