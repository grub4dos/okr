/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

// draw jpeg function 
//header file
#include "okr.h"

static short Zig_Zag[8][8]={{0,1,5,6,14,15,27,28},
						  {2,4,7,13,16,26,29,42},
						  {3,8,12,17,25,30,41,43},
						  {9,11,18,24,37,40,44,53},
						  {10,19,23,32,39,45,52,54},
						  {20,22,33,38,46,51,55,60},
						  {21,34,37,47,50,56,59,61},
						  {35,36,48,49,57,58,62,63}
						 };
						 
//macro definition
#define PI 3.1415926535

//define return value of function
#define FUNC_OK 0
#define FUNC_MEMORY_ERROR 1
#define FUNC_FILE_ERROR 2
#define FUNC_FORMAT_ERROR 3
#define MAKEWORD(a, b)   ((WORD)(((a) & 0xff) | (((WORD)((b) & 0xff)) << 8)))

//////////////////////////////////////////////////

static DWORD              LineBytes;
static DWORD              ImgWidth=0 , ImgHeight=0;
static char*              lpPtr;
//////////////////////////////////////////////////
//variables used in jpeg function
static short			SampRate_Y_H,SampRate_Y_V;
static short			SampRate_U_H,SampRate_U_V;
static short			SampRate_V_H,SampRate_V_V;
static short			H_YtoU,V_YtoU,H_YtoV,V_YtoV;
static short			Y_in_MCU,U_in_MCU,V_in_MCU;
static unsigned char   *lpJpegBuf;
static unsigned char   *lpCurrentJpgData;
static short			qt_table[3][64];
static short			comp_num;
static BYTE			comp_index[3];
static BYTE		    YDcIndex,YAcIndex,UVDcIndex,UVAcIndex;
static BYTE			HufTabIndex;
static short		    *YQtTable,*UQtTable,*VQtTable;
static BYTE			And[12]={0,1,3,7,0xf,0x1f,0x3f,0x7f,0xff};
static short		    code_pos_table[4][16],code_len_table[4][16];
static unsigned short	code_value_table[4][256];
static unsigned short	huf_max_value[4][16],huf_min_value[4][16];
static short			BitPos,CurByte;
static short			rrun,vvalue;
static short			MCUBuffer[10*64];
static int				QtZzMCUBuffer[10*64];
static short			BlockBuffer[64];
static short			ycoef,ucoef,vcoef;
static BOOL			IntervalFlag;
static short			interval=0;
static int				Y[4*64],U[4*64],V[4*64];
static DWORD		    sizei,sizej;
static short 			restart;
static long	iclip[1024];
static long	*iclp;

/////////////////////////////////////////////////////////////////////////////////////////
void  GetYUV(short flag)
{
	short	H=0,VV=0;
	short	i,j,k,h;
	int		*buf = NULL;
	int		*pQtZzMCU = NULL;

	switch(flag){
	case 0:
		H=SampRate_Y_H;
		VV=SampRate_Y_V;
		buf=Y;
		pQtZzMCU=QtZzMCUBuffer;
		break;
	case 1:
		H=SampRate_U_H;
		VV=SampRate_U_V;
		buf=U;
		pQtZzMCU=QtZzMCUBuffer+Y_in_MCU*64;
		break;
	case 2:
		H=SampRate_V_H;
		VV=SampRate_V_V;
		buf=V;
		pQtZzMCU=QtZzMCUBuffer+(Y_in_MCU+U_in_MCU)*64;
		break;
	}
	for (i=0;i<VV;i++)
		for(j=0;j<H;j++)
			for(k=0;k<8;k++)
				for(h=0;h<8;h++)
					buf[(i*8+k)*SampRate_Y_H*8+j*8+h]=*pQtZzMCU++;
}
///////////////////////////////////////////////////////////////////////////////
void StoreBuffer()
{
	short i,j;
	unsigned char  *lpbmp;
	unsigned int R,G,B;
	int y,u,v,rr,gg,bb;

	for(i=0;i<SampRate_Y_V*8;i++){
		if((sizei+i)<ImgHeight){
			lpbmp=((unsigned char *)lpPtr+(DWORD)(ImgHeight-sizei-i-1)*LineBytes+sizej*3);
			for(j=0;j<SampRate_Y_H*8;j++){
				if((sizej+j)<ImgWidth){
					y=Y[i*8*SampRate_Y_H+j];
					u=U[(i/V_YtoU)*8*SampRate_Y_H+j/H_YtoU];
					v=V[(i/V_YtoV)*8*SampRate_Y_H+j/H_YtoV];
					rr=((y<<8)+18*u+367*v)>>8;
					gg=((y<<8)-159*u-220*v)>>8;
					bb=((y<<8)+411*u-29*v)>>8;
					R=(unsigned char)rr;
					G=(unsigned char)gg;
					B=(unsigned char)bb;
					if (rr&0xffffff00) if (rr>255) R=255; else if (rr<0) R=0;
					if (gg&0xffffff00) if (gg>255) G=255; else if (gg<0) G=0;
					if (bb&0xffffff00) if (bb>255) B=255; else if (bb<0) B=0;
					*lpbmp++=B;
					*lpbmp++=G;
					*lpbmp++=R;
				}
				else  break;
			}
		}
		else break;
	}
}

///////////////////////////////////////////////////////////////////////////////
BYTE  ReadByte()
{
	BYTE  i;

	i=*(lpCurrentJpgData++);
	if(i==0xff)
		lpCurrentJpgData++;
	BitPos=8;
	CurByte=i;
	return i;
}
///////////////////////////////////////////////////////////////////////
void Initialize_Fast_IDCT()
{
	short i;

	iclp = iclip+512;
	for (i= -512; i<512; i++)
		iclp[i] = (i<-256) ? -256 : ((i>255) ? 255 : i);
}
////////////////////////////////////////////////////////////////////////
void idctrow(int * blk)
{
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;
	//intcut
	if (!((x1 = blk[4]<<11) | (x2 = blk[6]) | (x3 = blk[2]) |
		(x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3])))
	{
		blk[0]=blk[1]=blk[2]=blk[3]=blk[4]=blk[5]=blk[6]=blk[7]=blk[0]<<3;
		return;
	}
	x0 = (blk[0]<<11) + 128; // for proper rounding in the fourth stage 
	//first stage
	x8 = W7*(x4+x5);
	x4 = x8 + (W1-W7)*x4;
	x5 = x8 - (W1+W7)*x5;
	x8 = W3*(x6+x7);
	x6 = x8 - (W3-W5)*x6;
	x7 = x8 - (W3+W5)*x7;
	//second stage
	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6*(x3+x2);
	x2 = x1 - (W2+W6)*x2;
	x3 = x1 + (W2-W6)*x3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	//third stage
	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181*(x4+x5)+128)>>8;
	x4 = (181*(x4-x5)+128)>>8;
	//fourth stage
	blk[0] = (x7+x1)>>8;
	blk[1] = (x3+x2)>>8;
	blk[2] = (x0+x4)>>8;
	blk[3] = (x8+x6)>>8;
	blk[4] = (x8-x6)>>8;
	blk[5] = (x0-x4)>>8;
	blk[6] = (x3-x2)>>8;
	blk[7] = (x7-x1)>>8;
}
//////////////////////////////////////////////////////////////////////////////
void idctcol(int * blk)
{
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;
	//intcut
	if (!((x1 = (blk[8*4]<<8)) | (x2 = blk[8*6]) | (x3 = blk[8*2]) |
		(x4 = blk[8*1]) | (x5 = blk[8*7]) | (x6 = blk[8*5]) | (x7 = blk[8*3])))
	{
		blk[8*0]=blk[8*1]=blk[8*2]=blk[8*3]=blk[8*4]=blk[8*5]
			=blk[8*6]=blk[8*7]=iclp[(blk[8*0]+32)>>6];
		return;
	}
	x0 = (blk[8*0]<<8) + 8192;
	//first stage
	x8 = W7*(x4+x5) + 4;
	x4 = (x8+(W1-W7)*x4)>>3;
	x5 = (x8-(W1+W7)*x5)>>3;
	x8 = W3*(x6+x7) + 4;
	x6 = (x8-(W3-W5)*x6)>>3;
	x7 = (x8-(W3+W5)*x7)>>3;
	//second stage
	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6*(x3+x2) + 4;
	x2 = (x1-(W2+W6)*x2)>>3;
	x3 = (x1+(W2-W6)*x3)>>3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	//third stage
	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181*(x4+x5)+128)>>8;
	x4 = (181*(x4-x5)+128)>>8;
	//fourth stage
	blk[8*0] = iclp[(x7+x1)>>14];
	blk[8*1] = iclp[(x3+x2)>>14];
	blk[8*2] = iclp[(x0+x4)>>14];
	blk[8*3] = iclp[(x8+x6)>>14];
	blk[8*4] = iclp[(x8-x6)>>14];
	blk[8*5] = iclp[(x0-x4)>>14];
	blk[8*6] = iclp[(x3-x2)>>14];
	blk[8*7] = iclp[(x7-x1)>>14];
}


///////////////////////////////////////////////////////////////////////////////
void Fast_IDCT(int * block)
{
	short i;

	for (i=0; i<8; i++)
		idctrow(block+8*i);

	for (i=0; i<8; i++)
		idctcol(block+i);
}

//////////////////////////////////////////////////////////////////////////////////////////
void IQtIZzBlock(short  *s ,int * d,short flag)
{
	short i,j;
	short tag;
	short *pQt = NULL;
	int buffer2[8][8];
	int *buffer1 = NULL;
	short offset = 0;

	switch(flag){
	case 0:
		pQt=YQtTable;
		offset=128;
		break;
	case 1:
		pQt=UQtTable;
		offset=0;
		break;
	case 2:
		pQt=VQtTable;
		offset=0;
		break;
	}

	for(i=0;i<8;i++)
		for(j=0;j<8;j++){
			tag = (short)Zig_Zag[i][j];
			buffer2[i][j]=(int)s[tag]*(int)pQt[tag];
		}
	buffer1=(int *)buffer2;
	Fast_IDCT(buffer1);
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
			d[i*8+j]=buffer2[i][j]+offset;
}

/////////////////////////////////////////////////////////////////////////////////////
void IQtIZzMCUComponent(short flag)
{
	short H = 0, VV = 0;
	short i,j;
	int *pQtZzMCUBuffer = NULL;
	short  *pMCUBuffer = NULL;

	switch(flag){
	case 0:
		H=SampRate_Y_H;
		VV=SampRate_Y_V;
		pMCUBuffer=MCUBuffer;
		pQtZzMCUBuffer=QtZzMCUBuffer;
		break;
	case 1:
		H=SampRate_U_H;
		VV=SampRate_U_V;
		pMCUBuffer=MCUBuffer+Y_in_MCU*64;
		pQtZzMCUBuffer=QtZzMCUBuffer+Y_in_MCU*64;
		break;
	case 2:
		H=SampRate_V_H;
		VV=SampRate_V_V;
		pMCUBuffer=MCUBuffer+(Y_in_MCU+U_in_MCU)*64;
		pQtZzMCUBuffer=QtZzMCUBuffer+(Y_in_MCU+U_in_MCU)*64;
		break;
	}
	for(i=0;i<VV;i++)
		for (j=0;j<H;j++)
			IQtIZzBlock(pMCUBuffer+(i*H+j)*64,pQtZzMCUBuffer+(i*H+j)*64,flag);
}

//////////////////////////////////////////////////////////////////////////////
BOOL DecodeElement()
{
	int thiscode,tempcode;
	unsigned int temp,valueex;
	short codelen;
	BYTE hufexbyte,runsize,tempsize,sign;
	BYTE newbyte,lastbyte;

	if(BitPos>=1){
		BitPos--;
		thiscode=(BYTE)CurByte>>BitPos;
		CurByte=CurByte&And[BitPos];
	}
	else{
		lastbyte=ReadByte();
		BitPos--;
		newbyte=CurByte&And[BitPos];
		thiscode=lastbyte>>7;
		CurByte=newbyte;
	}
	codelen=1;
	while ((thiscode<huf_min_value[HufTabIndex][codelen-1])||
		  (code_len_table[HufTabIndex][codelen-1]==0)||
		  (thiscode>huf_max_value[HufTabIndex][codelen-1]))
	{
		if(BitPos>=1){
			BitPos--;
			tempcode=(BYTE)CurByte>>BitPos;
			CurByte=CurByte&And[BitPos];
		}
		else{
			lastbyte=ReadByte();
			BitPos--;
			newbyte=CurByte&And[BitPos];
			tempcode=(BYTE)lastbyte>>7;
			CurByte=newbyte;
		}
		thiscode=(thiscode<<1)+tempcode;
		codelen++;
		if(codelen>16)
			return FALSE;
	}  //while
	temp = (short)(thiscode - huf_min_value[HufTabIndex][codelen-1] + code_pos_table[HufTabIndex][codelen-1]);
	hufexbyte=(BYTE)code_value_table[HufTabIndex][temp];
	rrun=(short)(hufexbyte>>4);
	runsize=hufexbyte&0x0f;
	if(runsize==0){
		vvalue=0;
		return TRUE;
	}
	tempsize=runsize;
	if(BitPos>=runsize){
		BitPos-=runsize;
		valueex=(BYTE)CurByte>>BitPos;
		CurByte=CurByte&And[BitPos];
	}
	else{
		valueex=CurByte;
		tempsize -= (BYTE)BitPos;
		while(tempsize>8){
			lastbyte=ReadByte();
			valueex=(valueex<<8)+(BYTE)lastbyte;
			tempsize-=8;
		}  //while
		lastbyte=ReadByte();
		BitPos-=tempsize;
		valueex=(valueex<<tempsize)+(lastbyte>>BitPos);
		CurByte=lastbyte&And[BitPos];
	}  //else
	sign = (BYTE)(valueex>>(runsize-1));
	if(sign)
		vvalue= (short)valueex;
	else{
		valueex=valueex^0xffff;
		temp=0xffff<<runsize;
		vvalue=-(short)(valueex^temp);
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////
BOOL HufBlock(BYTE dchufindex,BYTE achufindex)
{
	short count=0;
	short i;
	BOOL bRet;

	//dc
	HufTabIndex=dchufindex;
	bRet = DecodeElement();
	if(!bRet)
		return FALSE;
  if(count < 63 )
	{
		BlockBuffer[count++]=vvalue;
	}
	//ac
	HufTabIndex=achufindex;
	while (count<64){
		bRet = DecodeElement();
		if(!bRet)
			return FALSE;
		if ((rrun==0)&&(vvalue==0)){
			for (i=count;i<64;i++)
				BlockBuffer[i]=0;
			count=64;
		}
		else{
			for (i=0;i<rrun;i++)
				BlockBuffer[count++]=0;
	    if(count < 63 )
			   BlockBuffer[count++]=vvalue;
		}
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL DecodeMCUBlock()
{
	short *lpMCUBuffer;
	short i,j;
	BOOL bRet;

	if (IntervalFlag){
		lpCurrentJpgData += 2;
		ycoef=ucoef=vcoef=0;
		BitPos=0;
		CurByte=0;
	}
	switch(comp_num){
	case 3:
		lpMCUBuffer=MCUBuffer;
		for (i=0;i<SampRate_Y_H*SampRate_Y_V;i++)  //Y
		{
			bRet = HufBlock(YDcIndex,YAcIndex);
			if (!bRet)
				return FALSE;
				
			BlockBuffer[0]=BlockBuffer[0]+ycoef;
			ycoef=BlockBuffer[0];
			for (j=0;j<64;j++)
				*lpMCUBuffer++=BlockBuffer[j];
		}
		for (i=0;i<SampRate_U_H*SampRate_U_V;i++)  //U
		{
			bRet = HufBlock(UVDcIndex,UVAcIndex);
			if (!bRet)
				return FALSE;
			BlockBuffer[0]=BlockBuffer[0]+ucoef;
			ucoef=BlockBuffer[0];
			for (j=0;j<64;j++)
				*lpMCUBuffer++=BlockBuffer[j];
		}
		for (i=0;i<SampRate_V_H*SampRate_V_V;i++)  //V
		{
			bRet = HufBlock(UVDcIndex,UVAcIndex);
			if (!bRet)
				return FALSE;
			BlockBuffer[0]=BlockBuffer[0]+vcoef;
			vcoef=BlockBuffer[0];
			for (j=0;j<64;j++)
				*lpMCUBuffer++=BlockBuffer[j];
		}
		break;
	case 1:
		lpMCUBuffer=MCUBuffer;
		bRet = HufBlock(YDcIndex,YAcIndex);
		if (!bRet)
			return FALSE;
		BlockBuffer[0]=BlockBuffer[0]+ycoef;
		ycoef=BlockBuffer[0];
		for (j=0;j<64;j++)
			*lpMCUBuffer++=BlockBuffer[j];
		for (i=0;i<128;i++)
			*lpMCUBuffer++=0;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
BOOL Decode()
{
	BOOL bRet;

	Y_in_MCU=SampRate_Y_H*SampRate_Y_V;
	U_in_MCU=SampRate_U_H*SampRate_U_V;
	V_in_MCU=SampRate_V_H*SampRate_V_V;
	H_YtoU=SampRate_Y_H/SampRate_U_H;
	V_YtoU=SampRate_Y_V/SampRate_U_V;
	H_YtoV=SampRate_Y_H/SampRate_V_H;
	V_YtoV=SampRate_Y_V/SampRate_V_V;
	Initialize_Fast_IDCT();
	
	while((bRet = DecodeMCUBlock()) != FALSE) {
		interval++;
    //if((restart)&&(interval % restart==0))
    //IntervalFlag=TRUE;
    //else
    //IntervalFlag=FALSE;
    
		if(restart){                         //解决除数为零的问题
       if((interval % restart==0)){
			 IntervalFlag=TRUE;
		   }else{
			 IntervalFlag=FALSE;
       }
    }else{
			IntervalFlag=FALSE;
      }
		IQtIZzMCUComponent(0);
		IQtIZzMCUComponent(1);
		IQtIZzMCUComponent(2);
		GetYUV(0);
		GetYUV(1);
		GetYUV(2);
		StoreBuffer();
		sizej+=SampRate_Y_H*8;
		if(sizej>=ImgWidth){
			sizej=0;
			sizei+=SampRate_Y_V*8;
		}
		if ((sizej==0)&&(sizei>=ImgHeight))
			break;
	}
	
	return bRet;
}

////////////////////////////////////////////////////////////////////////////////
BOOL InitTag()
{
	BOOL finish=FALSE;
	BYTE id;
	short  llength;
	short  i,j,k;
	short  huftab1,huftab2;
	short  huftabindex;
	BYTE hf_table_index;
	BYTE qt_table_index;
	BYTE comnum;

	unsigned char  *lptemp;
	short  ccount;

	lpCurrentJpgData = lpJpegBuf + 2;

	while (!finish){
		id = *(lpCurrentJpgData + 1);
		lpCurrentJpgData += 2;
		switch (id){
		case M_APP0:
			llength = MAKEWORD(*(lpCurrentJpgData + 1), *lpCurrentJpgData);
			lpCurrentJpgData += llength;
			break;
		case M_DQT:
			llength = MAKEWORD(*(lpCurrentJpgData+1), *lpCurrentJpgData);
			qt_table_index = (*(lpCurrentJpgData + 2))&0x0f;
			lptemp = lpCurrentJpgData + 3;
			if(llength<80){
				for(i=0;i<64;i++)
					qt_table[qt_table_index][i]=(short)*(lptemp++);
			}
			else{
				for(i=0;i<64;i++)
					qt_table[qt_table_index][i]=(short)*(lptemp++);
                qt_table_index=(*(lptemp++))&0x0f;
  				for(i=0;i<64;i++)
					qt_table[qt_table_index][i]=(short)*(lptemp++);
  			}
  			lpCurrentJpgData += llength;
			break;
		case M_SOF0:
	 		llength=MAKEWORD(*(lpCurrentJpgData + 1), *lpCurrentJpgData);
	 		ImgHeight=MAKEWORD(*(lpCurrentJpgData + 4), *(lpCurrentJpgData + 3));
	 		ImgWidth=MAKEWORD(*(lpCurrentJpgData + 6),*(lpCurrentJpgData + 5));
            comp_num=*(lpCurrentJpgData + 7);
		    if((comp_num!=1)&&(comp_num!=3))
	  			return FUNC_FORMAT_ERROR;
			if(comp_num==3){
				comp_index[0]=*(lpCurrentJpgData+8);
	  			SampRate_Y_H=(*(lpCurrentJpgData+9))>>4;
	  			SampRate_Y_V=(*(lpCurrentJpgData+9))&0x0f;
	  			YQtTable=(short *)qt_table[*(lpCurrentJpgData + 10)];

				comp_index[1]=*(lpCurrentJpgData+11);
				SampRate_U_H=(*(lpCurrentJpgData+12))>>4;
	  			SampRate_U_V=(*(lpCurrentJpgData+12))&0x0f;
	  			UQtTable=(short *)qt_table[*(lpCurrentJpgData+13)];

	  			comp_index[2]=*(lpCurrentJpgData+14);
	  			SampRate_V_H=(*(lpCurrentJpgData+15))>>4;
	  			SampRate_V_V=(*(lpCurrentJpgData+15))&0x0f;
				VQtTable=(short *)qt_table[*(lpCurrentJpgData+16)];
	  		}
			else{
	  			comp_index[0]=*(lpCurrentJpgData+8);
				SampRate_Y_H=(*(lpCurrentJpgData+9))>>4;
	  			SampRate_Y_V=(*(lpCurrentJpgData+9))&0x0f;
	  			YQtTable=(short *)qt_table[*(lpCurrentJpgData+10)];

				comp_index[1]=*(lpCurrentJpgData+8);
	  			SampRate_U_H=1;
	  			SampRate_U_V=1;
	  			UQtTable=(short *)qt_table[*(lpCurrentJpgData+10)];

				comp_index[2]=*(lpCurrentJpgData+8);
				SampRate_V_H=1;
	  			SampRate_V_V=1;
	  			VQtTable=(short *)qt_table[*(lpCurrentJpgData+10)];
			}
  			lpCurrentJpgData += llength;
			break;
		case M_DHT:             
			llength=MAKEWORD(*(lpCurrentJpgData+1),*lpCurrentJpgData);
			if (llength<0xd0){
				huftab1=(short)(*(lpCurrentJpgData+2))>>4;     //huftab1=0,1
		 		huftab2=(short)(*(lpCurrentJpgData+2))&0x0f;   //huftab2=0,1
				huftabindex=huftab1*2+huftab2;
		 		lptemp=lpCurrentJpgData+3;
				for (i=0; i<16; i++)
					code_len_table[huftabindex][i]=(short)(*(lptemp++));
				j=0;
				for (i=0; i<16; i++)
					if(code_len_table[huftabindex][i]!=0){
						k=0;
						while(k<code_len_table[huftabindex][i]){
							code_value_table[huftabindex][k+j]=(short)(*(lptemp++));
							k++;
						}
						j+=k;	
					}
				i=0;
				while (code_len_table[huftabindex][i]==0)
		 			i++;
				for (j=0;j<i;j++){
					huf_min_value[huftabindex][j]=0;
					huf_max_value[huftabindex][j]=0;
				}
				huf_min_value[huftabindex][i]=0;
				huf_max_value[huftabindex][i]=code_len_table[huftabindex][i]-1;
				for (j=i+1;j<16;j++){
					huf_min_value[huftabindex][j]=(huf_max_value[huftabindex][j-1]+1)<<1;
					huf_max_value[huftabindex][j]=huf_min_value[huftabindex][j]+code_len_table[huftabindex][j]-1;
				}
				code_pos_table[huftabindex][0]=0;
				for (j=1;j<16;j++)
		  			code_pos_table[huftabindex][j]=code_len_table[huftabindex][j-1]+code_pos_table[huftabindex][j-1];
		  		lpCurrentJpgData += llength;
			}  //if
			else{
	 			hf_table_index=*(lpCurrentJpgData+2);
				lpCurrentJpgData+=2;
				while (hf_table_index!=0xff){
					huftab1=(short)hf_table_index>>4;     //huftab1=0,1
			 		huftab2=(short)hf_table_index&0x0f;   //huftab2=0,1
					huftabindex=huftab1*2+huftab2;
					lptemp=lpCurrentJpgData+1;
					ccount=0;
					for (i=0; i<16; i++){
						code_len_table[huftabindex][i]=(short)(*(lptemp++));
						ccount+=code_len_table[huftabindex][i];
					}
					ccount+=17;	
					j=0;
					for (i=0; i<16; i++)
						if(code_len_table[huftabindex][i]!=0){
							k=0;
							while(k<code_len_table[huftabindex][i])
							{
								code_value_table[huftabindex][k+j]=(short)(*(lptemp++));
								k++;
							}
							j+=k;
						}
					i=0;
					while (code_len_table[huftabindex][i]==0)
						i++;
					for (j=0;j<i;j++){
						huf_min_value[huftabindex][j]=0;
						huf_max_value[huftabindex][j]=0;
					}
					huf_min_value[huftabindex][i]=0;
					huf_max_value[huftabindex][i]=code_len_table[huftabindex][i]-1;
					for (j=i+1;j<16;j++){
						huf_min_value[huftabindex][j]=(huf_max_value[huftabindex][j-1]+1)<<1;
						huf_max_value[huftabindex][j]=huf_min_value[huftabindex][j]+code_len_table[huftabindex][j]-1;
					}
					code_pos_table[huftabindex][0]=0;
					for (j=1;j<16;j++)
						code_pos_table[huftabindex][j]=code_len_table[huftabindex][j-1]+code_pos_table[huftabindex][j-1];
					lpCurrentJpgData += ccount;
					hf_table_index = *lpCurrentJpgData;
				}  //while
			}  //else
			break;
		case M_DRI:
			llength=MAKEWORD(*(lpCurrentJpgData+1),*lpCurrentJpgData);
			restart=MAKEWORD(*(lpCurrentJpgData+3),*(lpCurrentJpgData+2));
			lpCurrentJpgData += llength;
			break;
		case M_SOS:
			llength=MAKEWORD(*(lpCurrentJpgData+1),*lpCurrentJpgData);
			comnum=*(lpCurrentJpgData+2);
			if(comnum!=comp_num)
				return FUNC_FORMAT_ERROR;
			lptemp=lpCurrentJpgData+3;
			for (i=0;i<comp_num;i++){
				if(*lptemp==comp_index[0]){
					YDcIndex=(*(lptemp+1))>>4;   //Y
					YAcIndex=((*(lptemp+1))&0x0f)+2;
				}
				else{
					UVDcIndex=(*(lptemp+1))>>4;   //U,V
					UVAcIndex=((*(lptemp+1))&0x0f)+2;
				}
				lptemp+=2;
			}
			lpCurrentJpgData += llength;
			finish=TRUE;
			break;
		case M_EOI:    
			return FUNC_FORMAT_ERROR;
			break;
		default:
 			if ((id&0xf0)!=0xd0){
				llength=MAKEWORD(*(lpCurrentJpgData+1),*lpCurrentJpgData);
	 			lpCurrentJpgData+=llength;
			}
			else lpCurrentJpgData+=2;
			break;
  		}  //switch
	} //while
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////
void InitTable()
{
	short i,j;
	
	sizei = sizej = 0;
	ImgWidth = ImgHeight = 0;
	rrun=vvalue=0;
	BitPos=0;
	CurByte=0;
	IntervalFlag=FALSE;
	restart=0;
	for(i=0;i<3;i++)
		for(j=0;j<64;j++)
			qt_table[i][j]=0;
	comp_num=0;
	HufTabIndex=0;
	for(i=0;i<3;i++)
		comp_index[i]=0;
	for(i=0;i<4;i++)
		for(j=0;j<16;j++){
			code_len_table[i][j]=0;
			code_pos_table[i][j]=0;
			huf_max_value[i][j]=0;
			huf_min_value[i][j]=0;
		}
	for(i=0;i<4;i++)
		for(j=0;j<256;j++)
			code_value_table[i][j]=0;
	
	for(i=0;i<10*64;i++){
		MCUBuffer[i]=0;
		QtZzMCUBuffer[i]=0;
	}
	for(i=0;i<64;i++){
		Y[i]=0;
		U[i]=0;
		V[i]=0;
		BlockBuffer[i]=0;
	}
	ycoef=ucoef=vcoef=0;
}

//ExtractJpegData将jpg文件数据解压成bitmap格式的图形，由于bitmap仅仅是像素点信息，需要一个描述符，所以需要提供一个
//JpgHeader来保存图像宽度、高度等信息
BOOL ExtractJpegData(PBYTE JpgData, JPGHEAD *JpgHeader, PBYTE *ppBitmap)
{
	DWORD 		       imgSize;

	lpJpegBuf = JpgData;
	
	InitTable();

	if( !InitTag() ){
		//Print(L"ExtractJpegData, InitTag failed\n");
		return FALSE;
	}
	
	if( !SampRate_Y_H || !SampRate_Y_V ){
		//Print(L"ExtractJpegData, SampRate_Y_V = 0\n");
		return FALSE;
	}
	
	if(JpgHeader) {
		JpgHeader->X = 0;
		JpgHeader->Y = 0;
		JpgHeader->Width = (WORD)ImgWidth;
		JpgHeader->Height = (WORD)ImgHeight;
	}
	
	if(ppBitmap) {
		LineBytes = (((ImgWidth * 3) + 3) & ~3);	//每行占用字节数对齐4字节
		imgSize = (DWORD)LineBytes * ImgHeight;
		lpPtr = (char *)myalloc(imgSize);
		if(lpPtr) {
			if(Decode()) {
				*ppBitmap = lpPtr;
				return TRUE;
			}
			myfree(lpPtr);
			//Print(L"ExtractJpegData, Decode failed\n");
		} 
		return FALSE;
	}
	//else {
	//	Print(L"ExtractJpegData, imgSize %d\n", imgSize);
	//}
	return TRUE;
}

__inline BOOLEAN IsTransparentColor(EFI_GRAPHICS_OUTPUT_BLT_PIXEL *dot)
{
	if(*(DWORD*)dot == COLOR_TRANSPARENT)
		return TRUE;
	//if(dot->Green >= COLOR_TRANSPARENT_G && dot->Green < COLOR_TRANSPARENT_G + 10)
	//{
	//	if(dot->Red > COLOR_TRANSPARENT_R - 0x20 && dot->Red <= COLOR_TRANSPARENT_R)
	//		if(dot->Blue >= COLOR_TRANSPARENT_B - 0x20 && dot->Blue <= COLOR_TRANSPARENT_B)
	//			return TRUE;
	//}
	return FALSE;
}

//DrawBitmapData是在指定位置上以Bitmap格式的图形数据进行绘画
//bitmap有24位和32位的，jpg解压出来的是24位的；我们自己的压缩程序解压出来的是32位的，带透明度的
//通过bAlpha来判断是否带有透明度，
//= 0 表示24位的bmp图形，不透明；
//< 255 表示24位的bmp图形，alpha值是整体透明度，暂时不支持，目前的行为和255一样；
//= 255 表示24位的bmp图形，不透明，但是有透明色；
//>= 256 表示是32位bmp图形，alpha采用图形中的alpha的值
BOOL DrawBitmapData(int x, int y, int width, int height, int srcwidth, int srcheight, PBYTE pBitmap, 
		int bAlpha)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer;
	UINT32		clImageSize;
	long i, j, bytesperline, bytesperpixel, offset;
	long x1, y1;
	long alpha;

	if(bAlpha < 256)
		bytesperpixel = 3;
	else
		bytesperpixel = 4;
	bytesperline = (((srcwidth * bytesperpixel) + 3) & ~3);
	
	clImageSize = (UINT32)(width * height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	bltBuffer = myalloc(clImageSize);
	if (NULL == bltBuffer)
	{
		return FALSE;
	}

	ZeroMem(bltBuffer, clImageSize);
	
    if (bAlpha) { 
    	//表示有alpha通道，那么需要先从背景中取出图形，然后计算
        GetImgEx(x, y, width, height, bltBuffer);
    }
		
	for(i=0;i<height;i++)
	{
		if(width == srcwidth && height == srcheight)
			y1 = i;
		else
			y1 = i * (long)srcheight / (long)height;
			
		for(j=0;j<width;j++)
		{
			if(width == srcwidth && height == srcheight)
				x1 = j;
			else
				x1 = j * (long)srcwidth / (long)width;
				
			offset = (srcheight-y1-1)*bytesperline+x1*bytesperpixel;
			if (bytesperpixel == 4)
			{
				EFI_GRAPHICS_OUTPUT_BLT_PIXEL dot;
				alpha = pBitmap[offset+3];
				
				bltBuffer[i*width+j].Red	= (pBitmap[offset+2]*alpha + bltBuffer[i*width+j].Red*(255-alpha))/255;
				bltBuffer[i*width+j].Green	= (pBitmap[offset+1]*alpha + bltBuffer[i*width+j].Green*(255-alpha))/255;
				bltBuffer[i*width+j].Blue	= (pBitmap[offset]*alpha + bltBuffer[i*width+j].Blue*(255-alpha))/255;
			}
			else
			{
				if(bAlpha)
				{
					EFI_GRAPHICS_OUTPUT_BLT_PIXEL dot;
					dot.Red		= pBitmap[offset+2];
					dot.Green	= pBitmap[offset+1];
					dot.Blue	= pBitmap[offset];
					dot.Reserved= 0;
					if (!IsTransparentColor(&dot))
						*(DWORD*)(bltBuffer + i*width + j) = *(DWORD*)(&dot);
				} 
				else 
				{
					bltBuffer[i*width+j].Red	= pBitmap[offset+2];
					bltBuffer[i*width+j].Green	= pBitmap[offset+1];
					bltBuffer[i*width+j].Blue	= pBitmap[offset];
				}
			}
		}
	}
		
	if (g_vga)
	{
		g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0,0, x, y, width, height, 0);
	}
	if (NULL != bltBuffer)
	{
		myfree(bltBuffer);
	}
		
	return TRUE;
}

////不显示某种颜色，其余不显示，实现图片的透明度。
BOOL DrawBitmapDataFilterColor(int x, int y, int width, int height, int srcwidth, int srcheight, PBYTE pBitmap,
	int bAlpha)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bltBuffer;
	UINT32		clImageSize;
	long i, j, bytesperline, bytesperpixel, offset;
	long x1, y1;
	long alpha;

	if (bAlpha < 256)
		bytesperpixel = 3;
	else
		bytesperpixel = 4;
	bytesperline = (((srcwidth * bytesperpixel) + 3) & ~3);

	clImageSize = (UINT32)(width * height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	bltBuffer = myalloc(clImageSize);
	if (NULL == bltBuffer)
	{
		return FALSE;
	}
	ZeroMem(bltBuffer, clImageSize);

	GetImgEx(x, y, width, height, bltBuffer);

	//if (bAlpha) {
	//	//表示有alpha通道，那么需要先从背景中取出图形，然后计算
	//	GetImgEx(x, y, width, height, bltBuffer);
	//}

	for (i = 0; i < height; i++)
	{
		if (width == srcwidth && height == srcheight)
			y1 = i;
		else
			y1 = i * (long)srcheight / (long)height;

		for (j = 0; j < width; j++)
		{
			if (width == srcwidth && height == srcheight)
				x1 = j;
			else
				x1 = j * (long)srcwidth / (long)width;

			offset = (srcheight - y1 - 1)*bytesperline + x1 * bytesperpixel;
			if (bytesperpixel == 4)
			{
				EFI_GRAPHICS_OUTPUT_BLT_PIXEL dot;
				alpha = pBitmap[offset + 3];

				bltBuffer[i*width + j].Red = (pBitmap[offset + 2] * alpha + bltBuffer[i*width + j].Red*(255 - alpha)) / 255;
				bltBuffer[i*width + j].Green = (pBitmap[offset + 1] * alpha + bltBuffer[i*width + j].Green*(255 - alpha)) / 255;
				bltBuffer[i*width + j].Blue = (pBitmap[offset] * alpha + bltBuffer[i*width + j].Blue*(255 - alpha)) / 255;
			}
			else
			{
				if (bAlpha)
				{
					EFI_GRAPHICS_OUTPUT_BLT_PIXEL dot;
					dot.Red = pBitmap[offset + 2];
					dot.Green = pBitmap[offset + 1];
					dot.Blue = pBitmap[offset];
					dot.Reserved = 0;
					if (!IsTransparentColor(&dot))
						*(DWORD*)(bltBuffer + i * width + j) = *(DWORD*)(&dot);
				}
				else
				{
					if (pBitmap[offset + 2] == 0 &&
						pBitmap[offset + 1] == 120 &&
						pBitmap[offset] == 215)
					{
						bltBuffer[i*width + j].Red = bltBuffer[i*width + j].Red;
						bltBuffer[i*width + j].Green = bltBuffer[i*width + j].Green;
						bltBuffer[i*width + j].Blue = bltBuffer[i*width + j].Blue;
					}
					else
					{
						bltBuffer[i*width + j].Red = 255;// pBitmap[offset + 2];
						bltBuffer[i*width + j].Green = 255;//pBitmap[offset + 1];
						bltBuffer[i*width + j].Blue = 255;//pBitmap[offset];
					}
				}
			}
		}
	}

	//AlphaTransparent(x, y, width, height, bltBuffer, 10);

	if (g_vga)
	{
		g_vga->Blt(g_vga, bltBuffer, EfiBltBufferToVideo, 0, 0, x, y, width, height, 0);
	}

	if (NULL != bltBuffer)
	{
		myfree(bltBuffer);
	}

	return TRUE;
}

//DrawJpegData是在指定位置上以jpeg格式的图形数据进行绘画
BOOL DrawJpegData(int x, int y, int width, int height, PBYTE JpegData, int bAlpha)
{
	PBYTE		pBitmap;
	JPGHEAD		jpgheader;
	
	if(ExtractJpegData(JpegData, &jpgheader, &pBitmap))
	{
		if(width == 0 || height == 0)
			DrawBitmapData(x, y, jpgheader.Width, jpgheader.Height, jpgheader.Width, jpgheader.Height, pBitmap, bAlpha);
		else
			DrawBitmapData(x, y, width, height, jpgheader.Width, jpgheader.Height, pBitmap, bAlpha);
		myfree(pBitmap);
		
		return TRUE;
	}
	return FALSE;
}
