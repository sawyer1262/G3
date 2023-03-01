/*
 * iccemv.c
 *
 *  Created on: 2017年5月26日
 *      Author: YangWenfeng
 */

#include "stddef.h"
#include "stdarg.h"
#include "stdlib.h"
#include "string.h"
#include "iccemv.h"
#include "pit32_drv.h"
#include "debug.h"

#define EMV_Co_Test
#define   EMV_SW1            asyncard_info[current_slot].sw1
#define   EMV_SW2            asyncard_info[current_slot].sw2

/********************************************************************
 ----------------------------全局变量定义----------------------------
 ********************************************************************/
volatile UINT32          k_Ic_TotalTimeCount = 0;         //字符组总超时计数中间变量
volatile UINT32          k_Ic_TimeCount = 0;              //字符间隔超时计数中间变量
volatile UINT32          k_IcMax_TotalTimeCount = 0;      //字组总超时上限值
volatile UINT32          k_IcMax_TimeCount = 0;           //字符间隔超时上限值
volatile int             k_timeover,k_total_timeover = 0; //字符超时，总超时标志
volatile UINT32          k_IcMax_EtuCount = 0;           //字符间隔超时上限值，单位ETU


static   UINT8           card_Outputdata[ICMAXBUF];
static   UINT8           rstbuf[40];
volatile static UINT8    WTX;
volatile static UINT32   WWT,CWT,BWT;
volatile  UINT8          current_slot = 0;
volatile  UINT8          Parity_Check_Enable; /* 偶校验错误单个字符重发允许标志  */
volatile  UINT8          k_IccErrPar = 0;       /* 奇偶校验    */

vu16                     respLength = 0;
volatile UINT8           SC_ATR_Table[40];
volatile UINT8           SC_T0_T1_Status = 0;
volatile UINT8           SC_ATR_Len = 0;
volatile UINT8           guc_hotrstFlag = 0;
volatile UINT8           guc_Leflag = 0;

volatile UINT8           changeTa1     = 0;          //20110729
volatile UINT8           guc_AdjustETU = 0;		//发送字符间的ETU需要加1减1来调整
volatile UINT8           guc_GuardTime = 0;		//20131220，必须为int类型(与原值类型匹配)，否则配置完GT寄存器为默认12ETU
volatile UINT8           guc_Class = 0;
volatile UINT8           ICCardResetOK = 0;
volatile UINT8           ICCardInsert = 0;
volatile UINT8           ICCardInfoSent = 0;            //ic卡插拔信息发送：00 初始状态   01 卡插入   02 卡移除
volatile UINT8           g_usbCommOK = 0;
ASYNCARD_INFO            asyncard_info[IC_MAXSLOT];

SC_State                 *locState;  //test
SC_ATR                   SC_A2R;
SC_ADPU_Commands         SC_ADPU;
SC_ADPU_Responce         SC_Responce;
SC_READER_SET            SC_Slot_Default= {0,0,0x0A,0x0D,0x04,22,1,2,0x20,0,0,0,0,0,0x6fffff,0x6fffff,SC_POWER_ON};
SC_READER_SET            *SC_Slot_Active=&SC_Slot_Default;

#ifndef ISO7816_PBOC_CARD
const UINT16 FDSelection_Table[30] = {744,0x01,372,0x02,186,0x03,93,0x04,46,0x05,23,0x06,12,0x07,31,0x08,512,0x09,256,0x0A,128,0x0B,64,0x0C,32,0x0D,16,0x0E,8,0x0F};
const UINT16 FI_Table[24] = {0x00,372,0x01,372,0x02,558,0x03,744,0x04,1116,0x05,1488,0x06,1860,0x09,512,0x0A,768,0x0B,1024,0x0C,1536,0x0D,2048};
const UINT16 DI_Table[18] = {0x01,1,0x02,2,0x03,4,0x04,8,0x05,16,0x06,32,0x07,64,0x08,12,0x09,20};

//返回0表示TA1错误
BOOLEAN Get_FiDi(UINT8 TA1, UINT16 *Fi, UINT8 *Di)
{
	UINT8 i;
	UINT8 temp;

	temp = TA1>>4;	
	for (i=0;i<12;i++)
	{
		if (temp == FI_Table[2*i])
		{
			*Fi = FI_Table[2*i+1];
			break;
		}
	}
	if (i == 12)
		return FALSE;
	
	temp = TA1&0x0F;
	for (i=0;i<9;i++)
	{
		if (temp == DI_Table[2*i])
		{
			*Di = DI_Table[2*i+1];
			break;
		}
	}
	if (i == 9)
		return FALSE;

	return TRUE;
}

//返回0表示TA1错误
UINT8 Get_FD_Selection(UINT16 Fi, UINT8 Di)
{
	UINT8 i;
	UINT8 temp;
	UINT16 FdivD;

	FdivD = Fi/Di;		//取整数部分即可
	for (i=0;i<15;i++)
	{
		if (FdivD == FDSelection_Table[2*i])
		{
			temp = FDSelection_Table[2*i+1];
			break;
		}
	}
	if (i == 15)
		return 0;

	return temp;
}

//返回值: 0表示错误;1表示返回TA1，使用新FD；2表示没有返回TA1，使用默认的FD;
UINT8 PPS_Process(uchar slot)
{
	u8 ret;
	u8 PPS_Requst[4];
	u8 PPS_Response[4];
	u8 i;

	//GT和WT与ATR相同
	
	PPS_Requst[0] = 0xFF;
	PPS_Requst[1] = 0x10+(asyncard_info[slot].TD1&0x0f);
	PPS_Requst[2] = asyncard_info[slot].TA1;
	PPS_Requst[3] = PPS_Requst[0]^PPS_Requst[1]^PPS_Requst[2];

	Parity_Check_Enable=0x00;		//错误不重发
	for (i=0;i<4;i++)
	{
		ret=SC_PutByte(PPS_Requst[i]);
		if(ret)
		{
			return 0;
		}
	}

	if(asyncard_info[slot].TD1==0)
		Parity_Check_Enable=0x01; 		//错误重收
	else 
		Parity_Check_Enable=0x00;
	
	//接收PPSS
	if(SC_GetByte(&PPS_Response[0]) == ERROR)
	{
		return 0;			//超时或错误重发失败后都会到这里
	}
	if (PPS_Response[0] != 0xFF)
		return 0;

	//接收PPS0
	if(SC_GetByte(&PPS_Response[1]) == ERROR)
	{
		return 0;			//超时或错误重发失败后都会到这里
	}
	if ((PPS_Response[1]&0x0F) != (PPS_Requst[1]&0x0F))
		return 0;

	if ((PPS_Response[1]&0x10) == 0x10)	//有PPS1
	{
		//接收PPS1
		if(SC_GetByte(&PPS_Response[2]) == ERROR)
		{
			return 0;			//超时或错误重发失败后都会到这里
		}
		if (PPS_Response[2] != PPS_Requst[2])
			return 0;

		//接收XOR
		if(SC_GetByte(&PPS_Response[3]) == ERROR)
		{
			return 0;			//超时或错误重发失败后都会到这里
		}
		if (PPS_Response[3] != PPS_Requst[3])
			return 0;

		return 1;
	}
	else		//没有PPS1
	{
		//接收XOR
		if(SC_GetByte(&PPS_Response[2]) == ERROR)
		{
			return 0;			//超时或错误重发失败后都会到这里
		}
		if ((PPS_Response[0]^PPS_Response[1]) != PPS_Response[2])
			return 0;

		return 2;
	}
}
#endif

/*******************************************************************************
* Function Name  : PIT2Open
* Description    : PIT2启动
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT2Open()
{
	PIT32_Init(PIT2, PIT32_CLK_DIV_1024, 58,TRUE);   //1ms  --ywf:太小导致CASE1702无法测试通过，每次开启PTI必须重新开始计数
}

/*******************************************************************************
* Function Name  : PIT2Close
* Description    : PIT2关闭
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT2Close()
{
	PIT32_Stop(PIT2);
}

/*******************************************************************************
* Function Name  : Icc_Reset
* Description    :初始化SLOT通道的IC卡，包括对卡上电，复位，判断卡的协议类型：
*                 （T=1或T=0），并将ATR（复位应答）读入*ATR指针外。
* Input          : slot 需要初始化卡通道号
*                  ATR  ATR保持BUF
*
* Output         : None
* Return         : None
******************************************************************************/
int Icc_Reset(UINT8 slot,UINT8 *ATR)
{
 	int ret=0, temp=0;
#ifdef debug_print
 	int i=0;
#endif
#ifndef ISO7816_PBOC_CARD
	u8 usibound;
#endif

 	ClearAtr(slot);//清ATR

	asyncard_info[slot].T=0;
	guc_AdjustETU = 0;
	guc_GuardTime = 0;
	changeTa1 = 0;

	ret=GetAtr(rstbuf);
	PIT2Close();

#ifdef debug_print
	printf ("atr:");
	for (i=0; i<rstbuf[0]; i++)
	{
		printf ("%02x",rstbuf[1+i]);
	}
	printf ("\n");
#endif

	// 20131231，1CE079，挪到前面来，避免在热复位前修改了寄存器
	if(asyncard_info[slot].resetstatus==0)
	{
		asyncard_info[slot].resetstatus=0x01;
	}
	else
	{
		asyncard_info[slot].resetstatus=0x02;
	}
	if(ret)
	{
#ifdef debug_print
		printf ("atr error!\n");
		printf ("ret = %4d!\n",ret);
#endif

		if((ret!=ICC_ATR_TSERR)&&(ret!=ICC_ATR_TIMEOUT)
		 &&(ret!=ICC_PARERR)   &&(ret!=ICC_ATR_TCKERR)
		 &&(asyncard_info[slot].resetstatus==0x01) )
		{
			guc_hotrstFlag = 1;
		}
		// 对于TS错误，超时、校验、TCK错误则立即进行触点释放序列
		return ret;
	}
#ifdef debug_print
		printf ("atr ok!\n");
#endif


#ifndef ISO7816_PBOC_CARD		//执行PPS操作
	if(asyncard_info[slot].TA & 0X02)		//特殊模式
	{
		 if(asyncard_info[slot].TA2 & 0x10)	//按照默认参数，不做修改
		 {
		 }
		 else		//按照接口字节参数，即ATR中的参数
		 {
			if(asyncard_info[slot].TA & 0X01)		//TA1存在
			{
				usibound = Get_FD_Selection(asyncard_info[slot].F, asyncard_info[slot].D);
				if (usibound != 0)
				{
					g_USIReg->USIBDR = usibound;
				}
			}
		 }
	}
	else		//协商模式，发PPS
	{
		if(asyncard_info[slot].TA & 0X01)		//TA1存在
		{
			usibound = Get_FD_Selection(asyncard_info[slot].F, asyncard_info[slot].D);
		}
		else		//不存在TA1，尝试使用TA1=18的速率
		{
			asyncard_info[slot].TA1 = 0x18;
			if (Get_FiDi(asyncard_info[slot].TA1, &asyncard_info[slot].F, &asyncard_info[slot].D) == FALSE)
			{
				return ICC_PARERR;
			}
			usibound = Get_FD_Selection(asyncard_info[slot].F, asyncard_info[slot].D);
		}
		if (usibound != 0)
		{
			DelayNETU(22+2);		//反向保护时间
			//执行PPS过程
			temp = PPS_Process(slot);
			if (temp == 0)
			{				
				return ICC_PARERR;		//执行下电
			}
			else if (temp == 1)	//修改速率
			{
				g_USIReg->USIBDR = usibound;
			}
		}
	}
#else		//PBOC卡
	//通讯速率转换
	if((asyncard_info[current_slot].D==1)
	 ||(asyncard_info[current_slot].D==2)
	 ||(asyncard_info[current_slot].D==4))//增加通讯速率转换
	{
		changeTa1=0;
		g_USIReg->USIBDR = 2;
		if(asyncard_info[current_slot].D==2)
		{
			changeTa1=2;
			g_USIReg->USIBDR = 3;
		}
		if(asyncard_info[current_slot].D==4)
		{
			changeTa1=4;
			g_USIReg->USIBDR = 4;
		}
	}
#endif

	//通讯协议转换
	if ((asyncard_info[slot].TD1&0x0f)==0x01) //
	{
		g_USIReg->USICR1 = g_USIReg->USICR1 | 0x08;	// t=1;
		asyncard_info[slot].T=1;
		asyncard_info[slot].term_pcb=0x00;
		SC_Slot_Active->SC_Protocol=T1_PROTOCOL;
	}
	else if ((asyncard_info[slot].TD1&0x0f)==0x00)
	{
		g_USIReg->USICR1 = g_USIReg->USICR1 & 0xf7;	// t=0;
		asyncard_info[slot].T=0;
		SC_Slot_Active->SC_Protocol=T0_PROTOCOL;
	}
	else
	{
		return ICC_PROTOCALERR;
	}

	 // RSTBUF格式:长度+TS+TO+TA......
	for( temp=0 ; temp<rstbuf[0] ; temp++ )
	{
		*(ATR+temp) = rstbuf[temp+1];
	}
	SC_ATR_Len = rstbuf[0];

	//通讯ETU设置
	if(asyncard_info[current_slot].TC1 != 0xff)
	{
		temp = asyncard_info[current_slot].TC1 + 12;
		guc_GuardTime = temp;
	}
	else
	{
		if(asyncard_info[slot].T==1)
		{
			temp = 11;
			guc_GuardTime = temp;
		}
		if(asyncard_info[slot].T==0)
		{
			temp = 12;
			guc_GuardTime = temp;
		}
	}
	guc_GuardTime = temp;
	g_USIReg->USIGTRL = (unsigned char)(temp & 0xff);
	g_USIReg->USIGTRH = (unsigned char)((temp>>8) & 0xff);

	//asyncard_info[slot].open =1;

	//T1协商IFS
	if(asyncard_info[slot].T==1)
	{
		//预置从IC卡接收的I字组确认PCB控制字，顺序号置1
		asyncard_info[slot].card_pcb=0x40;
		ret=T1_IFSD_command(0);
		return ret;
	}
	return ICC_SUCCESS;
}

/*******************************************************************************
* Function Name  : Icc_Command
* Description    :发送命令
*                 支持T0协议与T1协议
*                 不支持CASE2类型的命令发送
* Input          : slot     被操作端口
*                  ApduSend APDU命令
*                  ApduResp 应答数据
*
* Output         : None
* Return         : 返回操作结果
******************************************************************************/
int Icc_Command(UINT8 slot,SC_ADPU_Commands * ApduSend,SC_ADPU_Responce * ApduResp)
{
	UINT8   Cla,Ins,P1,P2;
	UINT16  Le,Lc,iT1Le,recelen;
	int     result;
	//UINT8   *ptr;

	EMV_SW1=0;
	EMV_SW2=0;
	ApduResp->SW1=0;
	ApduResp->SW2=0;
	if((ApduSend==NULL)||(ApduResp==NULL))
		return ICC_PARAMETERERR;

	if(asyncard_info[current_slot].T==0)		 //对于T=0协议卡片应且有偶校验错误重发功能
		Parity_Check_Enable=0x01;
	else
		Parity_Check_Enable=0x00;

	Cla=ApduSend->Header.CLA;
	Ins=ApduSend->Header.INS;
	P1= ApduSend->Header.P1;
	P2= ApduSend->Header.P2;
	Lc= (ushort )(ApduSend->Body.LC);//发送的命令数据长度
	if(Lc>ICMAXBUF)
	{
		Parity_Check_Enable=0x00;
		return ICC_DATA_LENTHERR;
	}
	//接收的响应数据长度
	Le= (ushort)(ApduSend->Body.LE);
	if (Le > 256)
	{
		Le=256;
	}
	if( guc_Leflag == 1)//需要下发LE标志
	{
		guc_Leflag = 0;
		Le = 256;
	}

	result=IccSendCommand( Cla,Ins, P1, P2,  Lc,ApduSend->Body.Data,Le,card_Outputdata);
	Parity_Check_Enable=0x00;
	//ptr=ApduResp->Data;
	if((asyncard_info[current_slot].T==1) && (result==0))
	{
		//如果为T=1协议且接收正确时进行接收数据格式转换
		iT1Le = card_Outputdata[0]*256 +card_Outputdata[1];
		recelen=iT1Le-2;
		if(recelen>0)
		memcpy(ApduResp->Data,&card_Outputdata[2],recelen);

		ApduResp->SW1 = card_Outputdata[iT1Le];
		ApduResp->SW2 = card_Outputdata[iT1Le+1];
		respLength = recelen+2;
	}
	else if(result==0)
	{
		//T=0协议接收且接收正确时进行格式转换
		recelen=card_Outputdata[0]*256+card_Outputdata[1];
		if(recelen>0)
		memcpy(ApduResp->Data,&card_Outputdata[2],recelen);

		respLength = recelen+2;

		ApduResp->SW1=EMV_SW1;
		ApduResp->SW2=EMV_SW2;
	}
	else
	{
		// respLength初始为0
		EMV_SW1=0;    EMV_SW2=0;

		ApduResp->SW1=0;
		ApduResp->SW2=0;

		if((result==ICC_T0_TIMEOUT)||(result==ICC_T1_BWTERR)||(result==ICC_T1_CWTERR))
			return ICC_ICCMESSOVERTIME;
	}
	return  result;
}

/*******************************************************************************
* Function Name  : exp2_n
* Description    :计算2的N次方函数
* Input          :en : 幂数
*
* Output         : None
* Return         : 返回操作结果
******************************************************************************/
UINT16 exp2_n(UINT8 en)
{
	switch(en)
	{
		case 0:  return 1;
		case 1:  return 2;
		case 2:  return 4;
		case 3:  return 8;
		case 4:  return 16;
		case 5:  return 32;
		case 6:  return 64;
		case 7:  return 128;
		case 8:  return 256;
		case 9:  return 512;
		case 10:  return 1024;
		case 11:  return 2048;
		case 12:  return 4096;
		case 13:  return 8192;
		case 14:  return 16384;
		case 15:  return 32768;
	}
	return 0xffff;
}

/*******************************************************************************
* Function Name  : GetAtr
* Description    :读取IC卡的复位应答信号ATR
* Input          :Buff : ATR的存放BUFF
*
* Output         : None
* Return         : 返回操作结果：0x00：成功
******************************************************************************/
int GetAtr(UINT8 *Buff)
{
	UINT8   i/*,len*/,Flag,x;
	UINT8   *Lenth_Point;
	//UINT8   *ptr_atr;
	UINT8   T,edc,TCK,CWI,BWI;
	UINT16  N,j;
	UINT8   Lenth=2;
	UINT8  locData;

	edc=0;T=0;
	asyncard_info[current_slot].TA=0;
	asyncard_info[current_slot].TB=0;
	asyncard_info[current_slot].TC=0;
	asyncard_info[current_slot].TD=0;
	asyncard_info[current_slot].TS=0xff;
	//if(asyncard_info[current_slot].TS == 0xff)

	Lenth_Point=Buff;
	Parity_Check_Enable=0x00; //在复位应答中禁止偶校验错误重发
	*Buff=0x00;

	//总时间参数初始化
	k_Ic_TotalTimeCount=0;//每次总时间清0
	k_IcMax_TotalTimeCount=0;
	k_total_timeover=0x00;

	k_IcMax_TimeCount = 0;
	k_IcMax_EtuCount = 116;		// 10.6ms约为114个ETU
	USI_OpenWTC(k_IcMax_EtuCount);
	//配置收下下个字节的等待时间，在收到下个字节的起始位时配置WTC
	k_IcMax_TimeCount = 0;
	k_IcMax_EtuCount = 9600+480+50;

	if((USART_TsReceive(&locData, SC_Receive_Timeout)) == SUCCESS)
	{
	}
	else
	{
		return(ICC_ATR_TIMEOUT);	// 1ce 054 20131217
	}

	SC_Slot_Active->SC_Convention = 0x00;
	/*As to 7816-3 protocol,TS must be 0x3B or 0x03*/
	if (locData==0x3B||locData==0x03)
	{
		/*反向字节协议的判断*/
		if(locData==0x03)
		{
			SC_Slot_Active->SC_Convention=0x01;
			*(++Buff)=0x3F;
			g_USIReg->USICR1 |= 0x01; // 奇校验
		}
		else
		{
			*(++Buff)=locData;
			g_USIReg->USICR1 &= (~0x01); // 偶校验
		}
	}
	else
	{
		return ICC_ATR_TSERR;		// 20140102,TS错应下电
	}

	edc=0;
	//ptr_atr=Buff;

	k_IcMax_TotalTimeCount=1875;		//总时间从TS起始位开始计算,等待时间>19200+480 etu, 在19200+4800 ETU内下电，1ETU=93us, 即19200+480+480=1875ms	,要比最小值稍微大点
//	k_IcMax_TotalTimeCount=1950;		//总时间从TS起始位开始计算,等待时间>19200+480 etu, 在19200+4800 ETU内下电，1ETU=93us, 即19200+480+480=1875ms	,要比最小值稍微大点
//	k_IcMax_TotalTimeCount=1939;
	PIT2Open();		//ATR总时间用PIT定时

	if((USART_ByteReceive(&locData, SC_Receive_Timeout)) == SUCCESS)//T0
	{
		if(k_IccErrPar)
		{
			return ICC_PARERR;
		}
		*(++Buff) = locData;
	}
	else
	{
		return(ICC_ATR_TIMEOUT);
	}

	edc=edc^*Buff;
	Flag=*Buff;
	x=Flag&0x0f;
	i = 1;
	for(;;)
	{
		// TA
		if((Flag&0x10)==0x10)
		{
			if((USART_ByteReceive(&locData, SC_Receive_Timeout)) == SUCCESS)
			{
				if(k_IccErrPar)
				{
					return ICC_PARERR;
				}
				*(++Buff) = locData;
			}
			else
			{
				return(ICC_ATR_TIMEOUT);
			}

			if(i==1){
				asyncard_info[current_slot].TA1 = *Buff;
				asyncard_info[current_slot].TA|=0X01;
			}
			else if(i==2){
				asyncard_info[current_slot].TA2 = *Buff;
				asyncard_info[current_slot].TA|=0X02;
			}
			else if(i==3){
				asyncard_info[current_slot].TA3 = *Buff;
				asyncard_info[current_slot].TA|=0X04;
			}
			else    asyncard_info[current_slot].TA4 = *Buff;
			edc=edc^*Buff;
			Lenth++;
		}
		//TB
		if((Flag&0x20)==0x20)
		{
			if((USART_ByteReceive(&locData, SC_Receive_Timeout)) == SUCCESS)
			{
				if(k_IccErrPar)
				{
					return ICC_PARERR;
				}
				*(++Buff) = locData;
			}
			else
			{
				return(ICC_ATR_TIMEOUT);
			}

			if(i==1){
				asyncard_info[current_slot].TB1 = *Buff;
				asyncard_info[current_slot].TB|=0X01;
			}
			else if(i==2){
				asyncard_info[current_slot].TB2 = *Buff;
				asyncard_info[current_slot].TB|=0X02;
			}
			else if(i==3) {
				asyncard_info[current_slot].TB3 = *Buff;
				asyncard_info[current_slot].TB|=0X04;
			}
			else asyncard_info[current_slot].TB4 = *Buff;
			edc=edc^*Buff;
			Lenth++;
		}
		// TC
		if((Flag&0x40)==0x40)
		{
			if((USART_ByteReceive(&locData, SC_Receive_Timeout)) == SUCCESS)
			{
				if(k_IccErrPar)
				{
					return ICC_PARERR;
				}
				*(++Buff) = locData;
			}
			else
			{
				return(ICC_ATR_TIMEOUT);
			}

			if(i==1){
				asyncard_info[current_slot].TC1 = *Buff;
				asyncard_info[current_slot].TC|=0X01;
			}
			else if(i==2){
				asyncard_info[current_slot].TC2 = *Buff;
				asyncard_info[current_slot].TC|=0X02;
			}
			else if(i==3){
				asyncard_info[current_slot].TC3 = *Buff;
				asyncard_info[current_slot].TC|=0X04;
			}
			else asyncard_info[current_slot].TC4 = *Buff;
			edc=edc^*Buff;
			Lenth++;
		}
		//  no TD
		if((Flag&0x80)==0x00)
		{
			for(i=0;i<x;i++)		//历史字节
			{
				if((USART_ByteReceive(&locData, SC_Receive_Timeout)) == SUCCESS)
				{
					if(k_IccErrPar)
					{
						return ICC_PARERR;
					}
					*(++Buff) = locData;
				}
				else
				{
					return(ICC_ATR_TIMEOUT);
				}
				edc=edc^*Buff;
			}
			Lenth=Lenth+x;

			if(T)	//如果为T<>1协议卡片一定有TCK
			{
				if((USART_ByteReceive(&locData, SC_Receive_Timeout)) == SUCCESS)
				{
					if(k_IccErrPar)
					{
						return ICC_PARERR;
					}
					*(++Buff) = locData;
				}
				else
				{
					return ICC_ATR_TIMEOUT;		//20131205,SC_DeInit太短导致VCC没有被拉低?
				}

				TCK=*Buff;
  			if(edc!=TCK)   return ICC_ATR_TCKERR;
				Lenth++;
			}
			*Lenth_Point=Lenth;

#ifndef ISO7816_PBOC_CARD		//针对非PBOC卡，不做严格的判断以便兼容各种ATR
			//TA1
			if(asyncard_info[current_slot].TA & 0X01)
			{ 
				if (Get_FiDi(asyncard_info[current_slot].TA1, &asyncard_info[current_slot].F, &asyncard_info[current_slot].D) == FALSE)
				{
					return ICC_ATR_TA1ERR;
				}
			}
				
			//TC2  
			if((asyncard_info[current_slot].TC&0X02) == 0X00)
			{                    
				asyncard_info[current_slot].TC2=0x0a; 		//If TC2 is absent, then the default value is WI = 10.
			} 
			WWT=asyncard_info[current_slot].TC2*960;

			//TA3
			if(asyncard_info[current_slot].TA&0X04)
			{
				if(asyncard_info[current_slot].TA3==0xff) return ICC_ATR_TA3ERR;
			}
			else 
			{
				asyncard_info[current_slot].TA3=0x20; 
			}
			asyncard_info[current_slot].IFSC=asyncard_info[current_slot].TA3; 

			//TB3
			if(T==1)
			{  
				if((asyncard_info[current_slot].TB&0X04)==0X00) return ICC_ATR_TB3ERR; 
				CWI=asyncard_info[current_slot].TB3&0x0f; 
				CWT=11+exp2_n(CWI);
				WTX=0; 
				BWI=(asyncard_info[current_slot].TB3 >> 4)&0x0f; 
				BWT=11+960*exp2_n(BWI);
			} 
			

#else		//PBOC卡

			if(asyncard_info[current_slot].TA & 0X02)
			{
				if(asyncard_info[current_slot].TA2 & 0x10) 		//不支持默认的传输参数
					return ICC_ATR_TA2ERR;
			}
			// 支持TA1=‘11’ '12'  '13'值的ATR
			if(asyncard_info[current_slot].TA & 0X01)
			{
#ifndef EMV_Co_Test
				///*////////PBOC 4.3a//////*/

				if((asyncard_info[current_slot].TA1 < 0x11) || (asyncard_info[current_slot].TA1 > 0x13))
				{
					return ICC_ATR_TA1ERR;
				}
#else
				///*////////EMV 4.3c for case 1702/1755//////*/

				if((asyncard_info[current_slot].TA1 != 0x11)
				&& (asyncard_info[current_slot].TA1 != 0x12)
				&& (asyncard_info[current_slot].TA1 != 0x13)
				&& (asyncard_info[current_slot].TA1 != 0x17)
				&& (asyncard_info[current_slot].TA1 != 0xD1)
				&& (asyncard_info[current_slot].TA1 != 0xD9))
				{
					return ICC_ATR_TA1ERR;
				}

#endif
			}
			if(!(asyncard_info[current_slot].TA & 0X02)) asyncard_info[current_slot].TA1 = 0x11;

			if(asyncard_info[current_slot].TA1==0x11)
				asyncard_info[current_slot].D=1;
			else if(asyncard_info[current_slot].TA1==0x12)
				asyncard_info[current_slot].D=2;
			else if(asyncard_info[current_slot].TA1==0x13)
				asyncard_info[current_slot].D=4;
			///*////////EMV 4.3c for case 1702//////*/
			else if(asyncard_info[current_slot].TA1==0x17)
				asyncard_info[current_slot].D=64;
			else if(asyncard_info[current_slot].TA1==0xD1)
				asyncard_info[current_slot].D=1;
			else if(asyncard_info[current_slot].TA1==0xD9)
				asyncard_info[current_slot].D=20;
			///*////////EMV 4.3c for case 1702 end//////*/
			else
			{
				asyncard_info[current_slot].TA1=0x11;
				asyncard_info[current_slot].D=1;
			}

			//  冷复位下无TB1值或TB1！=0时则进行热复位
			if(asyncard_info[current_slot].resetstatus==0x00)
			{
				if(((asyncard_info[current_slot].TB&0X01)==0X00)||(asyncard_info[current_slot].TB1!=0x00))
				{
				return ICC_ATR_TB1ERR;		// 1ce,064_01/03， 20131218
				}
			}
			// 无效的TD值
			if((asyncard_info[current_slot].TD&0X01)==0X01)
			{
				if((asyncard_info[current_slot].TD1 & 0x0f)>1)
				{
					return ICC_ATR_TD1ERR;
				}
				if(asyncard_info[current_slot].TD1 & 0x0f)
				{
					if((asyncard_info[current_slot].TD & 0X02)&&((asyncard_info[current_slot].TD2&0x0f)!=0x01))
					{
						return ICC_ATR_TD2ERR;
					}
				}
				else
				{
					if(((asyncard_info[current_slot].TD&0x02)==0x02)&&((asyncard_info[current_slot].TD2&0x0f)!=0x01)
					&&((asyncard_info[current_slot].TD2&0x0f)!=0x0e))
					{
						return ICC_ATR_TD2ERR;
					}
				}
			}
			// 无效的TB2字节
			if(asyncard_info[current_slot].TB&0X02)
			{
				return ICC_ATR_TB2ERR;
			}

			//无效的TC2字节
			if(asyncard_info[current_slot].TC&0X02)
			{
				if(asyncard_info[current_slot].TC2!=0x0a)
				{
					return ICC_ATR_TC2ERR;
				}
			}
			asyncard_info[current_slot].TC2=0x0a;
			WWT=960*asyncard_info[current_slot].TC2;

			// 无效的TA3字节
			if(asyncard_info[current_slot].TA&0X04)
			{
				if(asyncard_info[current_slot].TA3<0x10)
				{
					return ICC_ATR_TA3ERR;
				}
				if(asyncard_info[current_slot].TA3==0xff)
				{
					return ICC_ATR_TA3ERR;
				}
			}
			else
			{
				asyncard_info[current_slot].TA3=0x20;
			}
			asyncard_info[current_slot].IFSC=asyncard_info[current_slot].TA3;

			// 无效的TB3字节
			if(T==1)
			{
				if((asyncard_info[current_slot].TB&0X04)==0X00)
				{
					return ICC_ATR_TB3ERR;
				}
				CWI=asyncard_info[current_slot].TB3&0x0f;
				if(CWI>5)
				{
					return ICC_ATR_TB3ERR;
				}
				CWT=11+exp2_n(CWI);
				WTX=0;
				BWI=(asyncard_info[current_slot].TB3 >> 4)&0x0f;
				if(BWI>4)
				{
					return ICC_ATR_TB3ERR;
				}
				BWT=11+960*exp2_n(BWI);

				if(asyncard_info[current_slot].TC1==0xff)
				{
					N=0;
				}
				else
				{
					N=asyncard_info[current_slot].TC1+1;
				}
				j=(ushort)exp2_n(CWI);
				if(j<=N)
				{
					return ICC_ATR_TB3ERR;
				}
			}

			// 无效的TC3值
			if(asyncard_info[current_slot].TC & 0X04)
			{
				if(asyncard_info[current_slot].TC3)
				{
					return ICC_ATR_TC3ERR;
				}
			}
#endif
			return ICC_SUCCESS;
		}
		if(Flag & 0x80){ //  TD1  TD2
			if((USART_ByteReceive(&locData, SC_Receive_Timeout)) == SUCCESS)
			{
				if(k_IccErrPar)
				{
					return ICC_PARERR;
				}
				*(++Buff) = locData;
			}
			else
			{
				return(ICC_ATR_TIMEOUT);
			}

			if(i==1){
				asyncard_info[current_slot].TD1=*Buff;
				asyncard_info[current_slot].TD|=0X01;
				T=*Buff&0x0f;
			}
			else if(i==2){
				asyncard_info[current_slot].TD2=*Buff;
				asyncard_info[current_slot].TD|=0X02;
				T|=*Buff&0x0f; // add 2002/04/23
			}
			else{
				asyncard_info[current_slot].TD3=*Buff;
				T|=*Buff&0x0f;
			}
			edc=edc^*Buff;
			Lenth++;
			Flag=*Buff;
		}
		i++;
	}
//	return ICC_SUCCESS;
}

/*******************************************************************************
* Function Name  : ClearAtr
* Description    :清除ATR BUF
* Input          :Channel : 需要清理的ATRbuf的逻辑号
*
* Output         : None
* Return         : None
******************************************************************************/
void ClearAtr(UINT8 Channel)
{
	asyncard_info[Channel].TA1 = 0x11;
	asyncard_info[Channel].D   = 0x01;
	asyncard_info[Channel].TA2 = 0x00;
	asyncard_info[Channel].TA3 = 0x00;
	asyncard_info[Channel].TA4 = 0x00;
	asyncard_info[Channel].TB1 = 0x00;
	asyncard_info[Channel].TB2 = 0x00;
	asyncard_info[Channel].TB3 = 0x00;
	asyncard_info[Channel].TB4 = 0x00;
	asyncard_info[Channel].TC1 = 0x00;
	asyncard_info[Channel].TC2 = 0x00;
	asyncard_info[Channel].TC3 = 0x00;
	asyncard_info[Channel].TC4 = 0x00;
	asyncard_info[Channel].TD1 = 0x00;
	asyncard_info[Channel].TD2 = 0x00;
	asyncard_info[Channel].TD3 = 0x00;
#ifndef ISO7816_PBOC_CARD
	asyncard_info[Channel].F = 372;
#endif
}

/*******************************************************************************
* Function Name  : IccSendCommand
* Description    :分别根据T=1或T=0协议将CLA+INS+P1+P2+LC（LE）+INDATA的命令数据包格式
*                 进行 转换，并按相应的格式与IC卡进行交换，结果数据与状态保存在OUTDATA中
*                 T=1时： OUTDATA： LEN（2）+DATA+SW（1，2）。
*                 T=0时： OUTDATA： LEN（2）+DATA。 SW保存到全局变量中
* Input          :cla : CLA
*                 ins : INS
*                 p1  : P1
*                 p2  : P2
*                 p3  : P3
*                 indata:data
*                 le   :Le
*                 outdata:结果BUF
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int IccSendCommand(UINT8 cla, UINT8 ins,UINT8 p1,UINT8 p2,UINT16 p3,UINT8 *indata,UINT16 le,UINT8 *outdata)
{
	UINT8 test[5];
	UINT8 cmd_case;
	UINT8 inbuf[ICMAXBUF];     //发送命令数据
	UINT8 outbuf[ICMAXBUF];    //接收响应数据
	UINT8 *ptr,len;
	int   result;
	UINT16 i;
	EMV_SW1=0xff;
	EMV_SW2=0xff;
	//根据T=1或T=0协议进行相应的处理
	switch(asyncard_info[current_slot].T)
	{
		case 1:            // T=1，格式: cla ins p1 p2 lc data le
		{
			i=4;
			inbuf[2]=cla;
			inbuf[3]=ins;
			inbuf[4]=p1;
			inbuf[5]=p2;
			ptr=&inbuf[6];
			if((p3==0)&&(le==0))
			{
				i++;
				*ptr++=0;
			}
			if(p3>0)
			{
				len=(UINT8)p3;
				i+=len+1;
				*ptr++=len;
				if(len>0)
				{
					memcpy(ptr,indata,len);
					ptr+=len;
				}
			}
			if(le>0)
			{
				if(le>=256)  le=0x00;
				i++;
				*ptr++=(UINT8)le;
			}
			inbuf[0]=i/256;
			inbuf[1]=i%256;
			result=T1_send_command(0x00,inbuf,outbuf);
			if(result==0)
				memcpy(outdata,&outbuf[1],outbuf[1]*256+outbuf[2]+2);

			return result;
		}
		case 0:             //T=0，格式: cla ins p1 p2 lc data或cla ins p1 p2 le
		{
			cmd_case=0x01;
			if((p3==0)&&(le==0))  cmd_case=0x01;  // case 1 command
			if((p3==0)&&(le>0))   cmd_case=0x02;  // case 2 command
			if((p3>0)&&(le==0))   cmd_case=0x03;  // case 3 command
			if((p3>0)&&(le>0))    cmd_case=0x04;  // case 4 command
			test[0]=cmd_case;

			if(p3>0)		// 表向IC卡发送P3长度的数据，不带LE字节
			{
				len=(uchar)p3;
				memcpy(inbuf,indata,p3);
			}
			else		 //表从IC卡接收LE个长度数据，只LE命令
			{
				if(le>=256)  le=0x00;
				len=(uchar)le;
			}
			result=T0_send_command(cla,ins,p1,p2,len,inbuf,outbuf,test);

			if(result)   return result;
			memcpy(outdata,outbuf,outbuf[0]*256+outbuf[1]+2);
			EMV_SW1=test[0];
			EMV_SW2=test[1];
			return ICC_SUCCESS;
		}
		default:
			break;
	}
	return ICC_PROTOCALERR;
}

/*******************************************************************************
* Function Name  : T1_send_command
* Description    :T=1协议下的字组数据发送与接收响应数据。
*                   注意: T1不支持扩展指令，如果要支持则需要修改
* Input          :NAD : 结点地址（初定为0X00）；
*                 INBUF : 发送的信息字段LEN+INF，INBUF[0]为长度；
*                 OUTBUF: 接收应答数据单元：格式NAD+LEN（2字节）+DATA+SW1+SW2
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int T1_send_command(UINT8 nad,UINT8 *inbuf,UINT8 *outbuf)
{
	UINT8   sour[300],oldsour[300],respbuf[300],oldrblock[5] = {0};
	UINT8   OLDIFSC;
	UINT8   /*NAD,*/PCB/*,slotpcb*/,tmpch,tmpch1;
	UINT16  len,len1/*,totallen,currentlen,remainlen*/;
	UINT16  total_send_len,sent_len,remain_len;
	UINT8   re_send_i_times,re_send_r_times;
//	UINT16  i;
	int     result;

	OLDIFSC=asyncard_info[current_slot].IFSC;

start_t1_exchange:
	re_send_i_times=0;
	re_send_r_times=0;
	total_send_len=inbuf[0]*256+inbuf[1];
	remain_len=inbuf[0]*256+inbuf[1];
	sent_len=0;
	outbuf[0]=0x00;
	outbuf[1]=0x00;
	outbuf[2]=0x00;
	len=0;len1=0;
	asyncard_info[current_slot].sw1=0Xff;
	asyncard_info[current_slot].sw2=0xff;

send_remain_data:
	if(remain_len==0)
	{
		return ICC_ICCMESERR;
	}
	if(remain_len>asyncard_info[current_slot].IFSC)		// 如果数据包长大于ifsc时，则将包进行切分后再发送//
	{
		asyncard_info[current_slot].term_pcb|=0x20; //给I字组加上链接符//
		memcpy(&sour[3],&inbuf[2+sent_len],asyncard_info[current_slot].IFSC);
		sour[2] = asyncard_info[current_slot].IFSC;
	}
	else	//否则只用一包就可完全发出 //
	{
		asyncard_info[current_slot].term_pcb &= 0xdf;		//去掉链接符
		memcpy(&sour[3],&inbuf[2+sent_len],remain_len);
		sour[2]=remain_len;
	}
	sent_len+=sour[2]; // 设定已发送数据长度  //
	remain_len=total_send_len-sent_len;
	sour[1]=asyncard_info[current_slot].term_pcb; // 设定发送包头的PCB协议控制字节//
	sour[0]=0x00;		//nad
	memcpy(oldsour,sour,sour[2]+3); // 备份发送的I字组 //
	asyncard_info[current_slot].term_pcb=sour[1] ^ 0x40;	//设置终端下个要发送的I字组序号
	re_send_i_times=0;

re_send_i_block:
	if(re_send_i_times>2)		//连续发送3个任何块而没有响应则下电
	{
		return ICC_T1_MOREERR;
	}
	re_send_i_times++;
	re_send_r_times=0;
	sour[0]=0x00;
	T1_Send_Block(sour);

receive_one_block:
	result=T1_Rece_Block(respbuf);

	if((result==ICC_T1_CWTERR)||(result==ICC_T1_BWTERR)||(result==ICC_T1_ABORTERR))
	{
		//超时后执行下电操作，不发R块
		return result;
	}

	if((result==ICC_T1_PARITYERR)||(result==ICC_T1_EDCERR))
	{
		if(re_send_r_times!=0)		//上一次有R字组则重发R块,test case 1778/1784 20140521
		{
			//当发送一个R块且接收块无效时,该R块被重发
			sour[0] = oldrblock[0];
			sour[1] = oldrblock[1];
			sour[2] = oldrblock[2];
			//re_send_r_times = 0;
		}
		else
		{
			// 1.当发送一个I块且接收块无效就发送一个R块,R块带有其N(R)请求以使预期的I块N(S)=N(R)
			// 2.发送一个S(...响应)块且接收到的块无效时,就发送一个R块
			tmpch = asyncard_info[current_slot].card_pcb^0x40;	//序号为正在接收卡片的I字组顺序号（即为上次接收的下个序号）//
			sour[1]=((tmpch >> 2) & 0x10) | 0x81; // 置奇偶/EDC错误
			sour[2]=0;
		}
		goto re_send_r_block;
	}
	if(result==ICC_T1_INVALIDBLOCK)
	{
		if(re_send_r_times!=0)		//上一次有R字组则重发R块,test case 1778/1784 20140521
		{
			//当发送一个R块且接收块无效时,该R块被重发
			sour[0] = oldrblock[0];
			sour[1] = oldrblock[1];
			sour[2] = oldrblock[2];
			//re_send_r_times = 0;
			goto re_send_r_block;
		}
		else
		{
			// 1.当发送一个I块且接收块无效就发送一个R块,R块带有其N(R)请求以使预期的I块N(S)=N(R)
			// 2.发送一个S(...响应)块且接收到的块无效时,就发送一个R块
			goto set_rblock_with_othererr;
		}
	}

	PCB=respbuf[1]; // 读取响应数据包头
	//I块
	if((PCB&0X80)==0X00)
	{
		if(remain_len>0) 		//如果还有剩余数据则不应该收到I块
		{
			if(re_send_r_times!=0)		// 20140520
			{
				//当发送一个R块且接收块无效时,该R块被重发
				sour[0] = oldrblock[0];
				sour[1] = oldrblock[1];
				sour[2] = oldrblock[2];
				//re_send_r_times = 0;
				goto re_send_r_block;
			}
			else
			{
				// 1.当发送一个I块且接收块无效就发送一个R块,R块带有其N(R)请求以使预期的I块N(S)=N(R)
				// 2.发送一个S(...响应)块且接收到的块无效时,就发送一个R块
				goto set_rblock_with_othererr;
			}
		}

		//  收到的I字组序列号不同于先前收到的I字组序列号  时表确认接收正确，并保存接收的数据到OUTBUF中。
		if((PCB & 0x40) != (asyncard_info[current_slot].card_pcb & 0x40))
		{
			re_send_r_times=0;
			if(respbuf[2]>0)		//如果接收数据长度不为零则保存接收数据
			{
				len1 = outbuf[1] * 256 + outbuf[2];
				len=len1 + respbuf[2];
				outbuf[1]= len / 256;
				outbuf[2]= len % 256;
				memcpy(&outbuf[len1+3],&respbuf[3],respbuf[2]);
			}
			asyncard_info[current_slot].card_pcb=PCB;		//重新设置成刚收到的序列号

			if((PCB&0X20)==0X20)		// 收到有链的I字组
			{
				tmpch = asyncard_info[current_slot].card_pcb ^ 0x40;		// 设置成下个I_Block顺序号
				sour[1]=((tmpch >>2 ) & 0x10) | 0x80;
				sour[2]=0;
				goto re_send_r_block;
			}
			// 因正确接收到无链接的I字组则退出
			result=ICC_SUCCESS;
			goto end_t1_exchange;
		}
		else 		//序列号错误
		{
			if(re_send_r_times!=0)		// 20140520
			{
				//当发送一个R块且接收块无效时,该R块被重发
				sour[0] = oldrblock[0];
				sour[1] = oldrblock[1];
				sour[2] = oldrblock[2];
				//re_send_r_times = 0;
				goto re_send_r_block;
			}
			else
			{
				// 1.当发送一个I块且接收块无效就发送一个R块,R块带有其N(R)请求以使预期的I块N(S)=N(R)
				// 2.发送一个S(...响应)块且接收到的块无效时,就发送一个R块
				goto set_rblock_with_othererr;
			}
		}
	}
	//R块
	else if((PCB&0XC0)==0X80)
	{
		tmpch=asyncard_info[current_slot].term_pcb;
		if((tmpch&0x40)==((PCB&0x10)<<2))		// 收到的序号等于终端下个要发送的I字组序号表明IC卡请求下一个链接块
		{
			if((PCB&0xef)==0x80)		// r-block无错误信息
			{
				if((oldsour[1]&0x20)==0x20) // 终端刚发送的I块有链接标识
				{
					goto send_remain_data;
				}
				else 	//终端无链  ，错误
				{

					if(re_send_r_times!=0)		// 20140520
					{
						//当发送一个R块且接收块无效时,该R块被重发
						sour[0] = oldrblock[0];
						sour[1] = oldrblock[1];
						sour[2] = oldrblock[2];
						//re_send_r_times = 0;
						goto re_send_r_block;
					}
					else
					{
						//请求无效的块重发，序列号是下一个期望收到的I块的序列号，比如终端初始I块为0，期望收到的I块也为0。
						tmpch = asyncard_info[current_slot].card_pcb ^ 0x40;
						tmpch1=((tmpch >> 2 ) & 0x10) | 0x82;  	//表明无链的错误

						re_send_r_times=0;
						sour[1]=tmpch1;
						sour[2]=0;
						goto re_send_r_block;
					}
				}
			}
			else 		// 带有错误标识的R字组
			{
				if(re_send_r_times!=0)		 // 重发R字组
				{
					//sour[1]=sour[1]&0xf0;		//test case 1772/1809 20140521
					//当发送一个R块且接收块无效时,该R块被重发
					sour[0] = oldrblock[0];
					sour[1] = oldrblock[1];
					sour[2] = oldrblock[2];
					re_send_r_times = 0;			//test case 1774 20140523
					goto re_send_r_block;
				}
				else		// 上一次无R字组
				{
					/*tmpch = asyncard_info[current_slot].card_pcb ^ 0x40;
					sour[1]=((tmpch >>2 ) & 0x10) | 0x80;
					sour[2]=0;
					goto re_send_r_block; */
					goto set_rblock_with_othererr;		// test case 1771, 20140521
				}
			}

		}

		if((tmpch&0x40)!=((PCB&0X10)<<2))		// 如果序号等于终端已发送的I字组序号则重发该I字组
		{
			if((asyncard_info[current_slot].card_pcb&0x20)==0x20) 		//b6固定为0
			{
				goto re_send_r_block;
			}

			if(OLDIFSC!=asyncard_info[current_slot].IFSC)
			{
				//在收到新的IFSC协商后，若收到R字组请求重发时，
				//先恢复终端I字组序列号，再重头发送
				OLDIFSC=asyncard_info[current_slot].IFSC;
				asyncard_info[current_slot].term_pcb=oldsour[1];
				goto start_t1_exchange;
			}
			memcpy(sour,oldsour,oldsour[2]+3);
			goto re_send_i_block;
		}
		// 重发R字组
		//goto re_send_r_block;
	}
	//S块
	else if((PCB&0XC0)==0XC0)
	{
		if((PCB & 0x20)==0X00)		//接收的是IC卡发送的S请求信号/
		{
			//如果收到的是IC卡发送的请求信号则根据传递来的设置
			//参数修改相应的设定并回送S响应字组给IC卡
			sour[1]=0x20 | respbuf[1] ;
			sour[2]=respbuf[2];
			memcpy(&sour[3],&respbuf[3],sour[2]);
			if((respbuf[1] & 0x1f) ==0x03)		// 扩充BWT请求  BWT=BWT * respbuf[3];
			{
				WTX= respbuf[3];
			}
			else if ((respbuf[1] & 0x1f) == 0x01)		 //信息字段容量请求
			{
				asyncard_info[current_slot].IFSC = respbuf[3];
			}
			else if((respbuf[1]&0x1f)==0x00)		 // 同步请求
			{
			}
			sour[0]=0x00;  //回送S响应字组给IC卡
			T1_Send_Block(sour);
			re_send_r_times=0;
			goto receive_one_block;
		}
		else 		//接收的是IC卡发送的S响应信号/
		{
		/*
			if((sour[1]&0xe0)==0xc0)
			{
				// 如果终端发送是请求信息时则根据响应设置相应的参数
				re_send_r_times=0;
				if ((respbuf[1] & 0x1f) == 0x01)		//ifsd response
				{
					asyncard_info[current_slot].IFSD = respbuf[3];
				}
				else if((respbuf[1] & 0x1f) ==0x00)	//再同步响应
				{    //  Reset or deactivation
					asyncard_info[current_slot].term_pcb=0x00;
					asyncard_info[current_slot].card_pcb=0x40;
					goto start_t1_exchange;
				}
				goto end_t1_exchange;
			}
		*/
			if(re_send_r_times!=0)		// test case 1778, 20140521
			{
				//当发送一个R块且接收块无效时,该R块被重发
				sour[0] = oldrblock[0];
				sour[1] = oldrblock[1];
				sour[2] = oldrblock[2];
				//re_send_r_times = 0;
				goto re_send_r_block;
			}
		}
	}

set_rblock_with_othererr:
	tmpch = asyncard_info[current_slot].card_pcb ^ 0x40;
	sour[1]=((tmpch >>2 ) & 0x10) | 0x82; // 置其它错误的R字组
	sour[2]=0;

re_send_r_block:
	if(re_send_r_times!=0)
	{
		if((oldrblock[0]!=sour[0])||(oldrblock[1]!=sour[1])||(oldrblock[2]!=sour[2]))
			re_send_r_times=0;
	}
	if(re_send_r_times==0)
	{
		oldrblock[0]=sour[0];
		oldrblock[1]=sour[1];
		oldrblock[2]=sour[2];
	}
	if(re_send_r_times>1)
	{
		return ICC_T1_MOREERR;
	}
	T1_Send_Block(sour);
	re_send_r_times++;
	goto receive_one_block;

end_t1_exchange:
	if(result!=0)
	{
		//如果通信与处理失败，则置SW1 SW2为0XFF；
		asyncard_info[current_slot].sw1=0Xff;
		asyncard_info[current_slot].sw2=0xff;
		return result;
	}
	len=outbuf[1]*256+outbuf[2]+3;
	// 将接收到的最后两位数据保存到SW1，SW2
	asyncard_info[current_slot].sw1=outbuf[len-2];
	asyncard_info[current_slot].sw2=outbuf[len-1];
	return ICC_SUCCESS;
}

/*******************************************************************************
* Function Name  : T0_send_command
* Description    :T=0协议下发送命令头标CLA+INS+P1+P2+P3与数据包（DATABUF），并读取IC卡响应的数据（OUTBUF）与状态字节（RSLT）。
* Input          :Class : CLA
*                 Ins : INS
*                 p1  : P1
*                 p2  : P2
*                 p3  : P3
*                 databuf：从终端发出的数据包，（其长度为P3；）
*                 outbuf:从IC卡读出的数据内容，[0]为数据长度；
*                 relt:表CASE命令号（1、2、3、4）
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败       返回时存入SW1与SW2状态字节。
******************************************************************************/
int T0_send_command(UINT8 cla,UINT8 ins,UINT8 p1,UINT8 p2,UINT8 p3, UINT8 *databuf,UINT8 *outbuf,UINT8 *rslt)
{
	UINT16  len,i,len1,len2/*,Tc2*/;
	UINT8   ret;
	UINT8   tempsw1,tempsw2;
	UINT8   *ptr1,*ptr2,status[2],cmdbuf[5];
	UINT8   cmd_case,cardtype,auto_resp,is_rece_send;
	UINT8   Change_Status_Word;
#ifdef EMV_Co_Test
	UINT8   GetIns = 0;
#endif
	UINT16  /*NS,*/NR/*, temp*/;
	UINT8   first_error_status = 0;

	NR=16+2;                 // 反向保护时间,额外增加了2个etu
	cmd_case=*rslt;        // 命令CASE号 1，2，3，4//
	cardtype = asyncard_info[current_slot].autoresp;			//是否自动发送GetResponse
	if(cardtype)           // 设置自动应答标志 //
		auto_resp=0x00;   // 非自动 //
	else
		auto_resp=0x01;   // 自动//
	Change_Status_Word=0;  // 更新返回状态字节标志：1-更改 //

	//总时间参数初始化
	k_Ic_TotalTimeCount=0;//每次总时间清0
	k_IcMax_TotalTimeCount=0;
	k_total_timeover=0x00;

	//使用WTC定时字符等待时间
	k_IcMax_TimeCount = 0;
	k_IcMax_EtuCount = (WWT+480+50)*(asyncard_info[current_slot].D);
	USI_OpenWTC(k_IcMax_EtuCount);

	is_rece_send=0;        //当前处于数据包发送与接收状态：0=接收；1=发送
	if(cmd_case==0x01) is_rece_send=0;
	else if(cmd_case==0x02) is_rece_send=0;
	else if(cmd_case==0x03) is_rece_send=1;
	else is_rece_send=1;

	//置命令头标 //
	cmdbuf[0]=cla;
	cmdbuf[1]=ins;
	cmdbuf[2]=p1;
	cmdbuf[3]=p2;
	cmdbuf[4]=p3;
	status[0]=0xff;
	status[1]=0xff;
	len=0;
	ptr1=databuf;
	ptr2=outbuf+2;
	tempsw1=0xff;
	tempsw2=0xff;

	while(1)
	{
flg1:
		len1=(ushort)cmdbuf[4];  // 发送命令数据长度
		len2=(ushort)cmdbuf[4];  // 接收响应数据长度
		if (len2 == 0)		// le=0时恢复成256，20140525
		{
			len2 = 256;
		}

		DelayNETU(NR);		//反向保护时间
		for(i=0;i<5;i++)
		{
			ret=SC_PutByte(cmdbuf[i]);		//发送CLA INS P1 P2 P3
#if 0
   			printf ("%x",cmdbuf[i]);
#endif
			if(ret)
			{
				return ICC_T0_MORESENDERR;
			}
		}

flg2:
		//读取一过程字节
		if(SC_GetByte(&status[0]) == ERROR)		//相反方向，即终端发完后接收IC卡数据
		{
#if 0
   			printf ("%x",status[0]);
#endif

			return ICC_T0_TIMEOUT;			//超时或错误重发失败后都会到这里
		}

		// 过程字节等于INS----发送或接收其余的数据包  //
		if(status[0]==cmdbuf[1])
		{
#ifdef EMV_Co_Test
			GetIns = 1;
#endif
			if(is_rece_send) 		//Lc类型
			{
				DelayNETU(NR);		//反向保护时间
				for(i=0;i<len1;len1--)
				{
					ret=SC_PutByte(*ptr1);
					if(ret)
					{
						return ICC_T0_MORESENDERR;
					}
					ptr1++;
				}

				goto flg2;  // 等待下一个过程字节 //
			}
			else		// Le类型
			{
				len+=len2;
				if(len>256)
				{
					return ICC_DATA_LENTHERR;
				}
				for(i=0;i<len2;len2--)
				{
					if(SC_GetByte(ptr2) == ERROR)
					{
						return ICC_T0_TIMEOUT;			//超时或错误重发失败后都会到这里
					}
					ptr2++;
				}
				goto flg2;
			}
		}

		// 等于INS的补码  接收或发送下一个数据  //
		else if(status[0]==(uchar)(~cmdbuf[1]))
		{
#ifdef EMV_Co_Test
			GetIns = 1;
#endif
			if(is_rece_send)		//Lc类型
			{
				DelayNETU(NR);		//反向保护时间
				ret=SC_PutByte(*ptr1);
				if(ret){
					return ICC_T0_MORESENDERR;
				}
				ptr1++;
				len1--;
				goto flg2;
			}
			else 		//Le类型
			{
				if(SC_GetByte(ptr2) == ERROR)
				{
					return ICC_T0_TIMEOUT;			//超时或错误重发失败后都会到这里
				}

				ptr2++;len++;
				len2--;
				if(len>256){
					return ICC_DATA_LENTHERR;
				}
				goto flg2;
			}
		}
		else if(status[0]==0x60)
		{ //  终端提供附加的工作等待时间  //
			goto flg2;
		}
#ifdef EMV_Co_Test
		else
		{
			if(cmd_case != 4)
			{
				GetIns = 1;
			}
		}
#endif
		if(((status[0]&0xf0)!=0X60)&&((status[0]&0xf0)!=0x90))		//sw1
		{  //收到错误的过程或状态字节后，释放触点
			return ICC_T0_INVALIDSW;
		}
		// 如果SW1为“6X”或“9X”（“60”除外）时，须等待该过程的下一个状态字节SW2
		if(SC_GetByte(&status[1]) == ERROR)
		{
			return ICC_T0_TIMEOUT;			//超时或错误重发失败后都会到这里
		}
#ifdef EMV_Co_Test
		/*如果在没有收到INS前收到SW，那么直接返回状态字*/
		if(auto_resp && GetIns) 		//自动响应  //
		{
			GetIns = 0;
#else
		if(auto_resp) 		//自动响应  //
		{
#endif
			if(status[0]==0x61)
			{    // 如果SW1=0X61则终端须送出一条GET RESPONSE命令头标
				// 给IC卡，其中的P3=SW2 //
				cmdbuf[0]=0x00;cmdbuf[1]=0xc0;cmdbuf[2]=0x00;
				cmdbuf[3]=0x00;cmdbuf[4]=status[1];
				is_rece_send=0;  // 后面的操作只能从IC卡接收数据包//
				if(cmd_case==0x04) cmd_case=0x02;
				goto flg1;       // 发送下个命令头标
			}
			if(status[0]==0x6c)
			{    // 如果SW1=0X6C则终端传送先前的命令头标给IC卡，
				// 其中P3=SW2； //
				cmdbuf[4]=status[1];
				is_rece_send=0;
				if(cmd_case==0x04) cmd_case=0x02;
				goto flg1;
			}
			if(cmd_case==0x04)//&&(FirstByte_Is_Status==0))
			{
				if (first_error_status==0)
				{
					if (status[0]==0x62 && status[1]==0x81)
					{
						first_error_status = 1;		// test case 1744(x=3) 20140524
						goto flg1;		//重发此命令
					}
					if((status[0]==0x62)||(status[0]==0x63)||
						(((status[0]&0xf0)==0x90) && (!(status[0]==0x90 && status[1]==0x00))))   	// sw = 6283/6335/9FFF
					{
						cmdbuf[0]=0x00;cmdbuf[1]=0xc0;cmdbuf[2]=0x00;
						cmdbuf[3]=0x00;cmdbuf[4]=0x00;
						is_rece_send=0;
						Change_Status_Word=1;
						cmd_case=0x02; //修改CASE状态防止重发 //
						tempsw1=status[0];
						tempsw2=status[1];
						goto flg1;
					}
				}
				else
				{
					if (status[0]==0x9F && status[1]==0xFF)
					{
						//退出
					}
					else
					{
						goto flg1;		//重发此命令
					}
				}
			}

		}
		// 接收到其它内容的过程字节时则保存其状态字节并返回
		if(Change_Status_Word)
		{
			*rslt=tempsw1;
			*(rslt+1)=tempsw2;//status[1];//
		}
		else
		{
			*rslt=status[0];
			*(rslt+1)=status[1];
		}
		*outbuf=len/256;
		*(outbuf+1)=len%256; // 保存接收的数据长度 //
		return ICC_SUCCESS;
	}
//	return ICC_SUCCESS;
}

/*******************************************************************************
* Function Name  : T1_IFSD_command
* Description    :在收到正确的复位应答后，对于T=1卡片须发送一个IFSD请求，以指示终端能收取最大字组长度。
* Input          :slot : 卡槽逻辑号
*
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int  T1_IFSD_command(UINT8 slot)
{
	int   ret;
	UINT8 ifsdbuf[10],respbuf[300];
	UINT8 resendtimes=0;
//	UINT8 tmpch;

	if(asyncard_info[slot].T==0)
	{
		return ICC_SUCCESS;
	}
	ifsdbuf[0]=0x00;
	ifsdbuf[1]=0xc1;
	ifsdbuf[2]=0x01;
	ifsdbuf[3]=0xfe;
	ifsdbuf[4]=0x00; //0-3异或 校验值

	while(1)
	{
		if(resendtimes<3)
		{
			ret=T1_Send_Block(ifsdbuf);
		}
		else
		{
			return ICC_T1_IFSDERR;
		}
		resendtimes++;
		ret=T1_Rece_Block(respbuf);
		if((ret==ICC_T1_CWTERR)||(ret==ICC_T1_BWTERR)||(ret==ICC_T1_ABORTERR))
		{
			return ret;
		}

		//发送一个S(...请求)块且接收到的应答不是S(...响应)块(仅IFD)发生时,该S(...请求)块重发
		if((ret==ICC_T1_PARITYERR)||(ret==ICC_T1_EDCERR)||(ret==ICC_T1_INVALIDBLOCK))
			continue;
		if(((respbuf[1]&0xe0)!=0xe0)||((respbuf[1]&0x0f)!=0x01))		//返回值必须为00E101FE1E
			continue;
		if((respbuf[2]!=0x01)||(respbuf[3]!=0xfe))
			continue;

		asyncard_info[slot].IFSD=254;
		return ICC_SUCCESS;
	}
//	return ICC_SUCCESS;
}

/*******************************************************************************
* Function Name  : T1_Send_Block
* Description    :T=1协议下发送一字组
* Input          :Inbuf : 待发送字组的buf
*
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int  T1_Send_Block(UINT8 *Inbuf)
{
	UINT8   edc;
	UINT16  i,len;
	UINT16  NR;

	NR=22+200;		//额外增加了200个etu, Test case 1767_2x 20140530

	len=*(Inbuf+2)+3;   //  发送总长NAD+PCB+LEN+（LEN个DATA）
	edc=0;              //  计算命令与数据元的校码EDC

	DelayNETU(NR);//反向发送保护时间间隔
	for(i=0;i<len;i++)
	{
		SC_PutByte(Inbuf[i]);
		edc=edc ^ Inbuf[i];
	}
	SC_PutByte(edc);

	//T1的接收端如果检测到奇偶校验错不会产生错误信号，所以发送端不会检测到错误信号，直接返回成功
	return ICC_SUCCESS;
}

/*******************************************************************************
* Function Name  : T1_Rece_Block
* Description    :T=1协议下接收一字组
* Input          :Outbuf : 接收数据缓存buf
*
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int  T1_Rece_Block(UINT8 *Outbuf)
{
	UINT8   i,ch,edc;
	UINT8   Parity_Error_Flag;
//	UINT32  T1BWT,T1CWT;
	UINT8   r_nad,r_pcb,r_len,r_inf;
	UINT8   r_len_temp;

	k_IcMax_TotalTimeCount=0x00;
	k_Ic_TotalTimeCount=0x00;
	k_total_timeover=0x00;

	if (WTX!=0)	//IC卡刚发送了WTX命令
	{
		//使用WTC定时字符等待时间
		k_IcMax_TimeCount = 0;
		k_IcMax_EtuCount = ((BWT-11)*(asyncard_info[current_slot].D)+11)*WTX+(960+50)*(asyncard_info[current_slot].D)*WTX;		//BWT*n+D*960*n
		USI_OpenWTC(k_IcMax_EtuCount);

		WTX=0;
	}
	else
	{
		k_IcMax_TimeCount = 0;
		k_IcMax_EtuCount = ((BWT-11)+960+50)*(asyncard_info[current_slot].D)+11;
		USI_OpenWTC(k_IcMax_EtuCount);
	}

		//配置收下下个字节的等待时间，在收到下个字节的起始位时配置WTC
		k_IcMax_TimeCount = 0;
		k_IcMax_EtuCount = CWT+4+1;		//CWT+4<终端范围<CWT+4+2

	edc=0;
	Parity_Error_Flag=0;
	if(SC_GetByte(&ch) == ERROR)		//NAD
	{
		return ICC_T1_BWTERR;
	}

	if(k_IccErrPar) Parity_Error_Flag=0x01;//
	edc=edc ^ ch;
	*Outbuf++=ch;
	r_nad=ch;

	if(SC_GetByte(&ch) == ERROR)		//PCB
	{
		return ICC_T1_CWTERR;
	}
	if(k_IccErrPar) Parity_Error_Flag=0x01;//
	edc=edc ^ ch;
	*Outbuf++=ch;
	r_pcb=ch;

	if(SC_GetByte(&ch) == ERROR)		//获取长度
	{
		return ICC_T1_CWTERR;
	}
	if(k_IccErrPar) 			// 20131218 1CF 086.00/02，应该收完所有字符再下电
	{
		//T1时奇偶校验错也能获取正确数据
		Parity_Error_Flag=0x01;//
	}
	edc=edc ^ ch;
	*Outbuf++=ch;
	r_len=ch;
	r_len_temp=ch;
	if((r_pcb&0xc0)==0x80) 		// R块
	{
	//if(ch) r_len_temp=0;		// 1CF 087.01 20131219
	}
	else if((r_pcb&0xc0)==0xc0)		//S块
	{
	//if(ch>1) r_len_temp=1;		//  1CF 087.01 20131219
	}

	for(i=0;i<r_len_temp;i++)
	{
		if(SC_GetByte(&ch) == ERROR)
		{
			return ICC_T1_CWTERR;
		}
		if(k_IccErrPar) Parity_Error_Flag=0x01;
		edc=edc ^ ch;
		*Outbuf++=ch;
	}
	r_inf=ch;		//S块的信息域只有1个字节
	if(SC_GetByte(&ch) == ERROR)		//EDC
	{
		if(((r_pcb&0x80)==0)&&(r_len==0xff)){ 		//I块的长度为FF，错误
			//           return ICC_T1_INVALIDBLOCK;		//发R块,20131220
			return ICC_T1_ABORTERR;		//下电,20131220
		}
		return ICC_T1_CWTERR;
	}
	if(k_IccErrPar) Parity_Error_Flag=0x01;//
	*Outbuf=ch;
	if(ch!=edc) return ICC_T1_EDCERR;
	if(Parity_Error_Flag) return ICC_T1_PARITYERR;
	if(r_nad) return ICC_T1_INVALIDBLOCK;   //nad!=0

	// I_BLOCK
	if((r_pcb&0x80)==0)
	{
		if(r_len==0xff) 	//I块的信息域长度为FF，错误
		{
#if 0      //发R块    	,20131220
			for (i=0; i<120; i++)		// 12etu*10
			{
			delay1ETU();	// 20131219, 1CF 089.02, 1CF 096.01 可能会发送超过255字节的数据，但实际未接收，需等ICC发完再发R块
			if ((i%12) == 0)		// 20131219, 1CF 092.02/ 1CF 094.06/ 1CF 098.01 清缓冲区,每个字节清一次
			{
			test = USIRDR;		//清除缓冲区，会导致1CF087.02不过?20131219
			}
			}
			return ICC_T1_INVALIDBLOCK;
#else	//下电,20131220
			/*
			for (i=0; i<120; i++)		// 12etu*10
			{
			delay1ETU();	// 20131219, 1CF 089.02, 1CF 096.01 可能会发送超过255字节的数据，但实际未接收，需等ICC发完再下电
			}
			*/
			return ICC_T1_ABORTERR;
#endif
		}
	}
	// R_BLOCK
	else if((r_pcb&0xc0)==0x80)
	{
		if(r_len) return ICC_T1_INVALIDBLOCK;		//r块的信息域长度不为0，错误
		if((r_pcb&0x20)==0x20) return ICC_T1_INVALIDBLOCK;  	//b6固定为0
	}
	// S_BLOCK
	else
	{
		if(r_pcb==0xc1){ // ifs request
			if((r_inf<0x10)||(r_inf>0xfe)) return ICC_T1_INVALIDBLOCK;
		}
		else if(r_pcb==0xc2)  return ICC_T1_ABORTERR;	// abort request
		else if(r_pcb==0xe3) return ICC_T1_INVALIDBLOCK; //S(WTX response)
		else if(r_pcb==0xe2) return ICC_T1_INVALIDBLOCK; //S(ABORT response)
		else if(r_pcb==0xe0) return ICC_T1_INVALIDBLOCK; //S(RESYNCH response)
		if(r_len!=1) return ICC_T1_INVALIDBLOCK;   	//s块的信息域长度不为1，错误
		if((r_pcb&0x1f)>4) return ICC_T1_INVALIDBLOCK; 	//保留的s块命令
	}
	return ICC_SUCCESS;
}
