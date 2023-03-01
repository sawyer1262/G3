/*
 * icc_hardware.c
 *
 *  Created on: 2017年5月26日
 *      Author: YangWenfeng
 */
#include "delay.h"
#include "iccemv.h"
#include "cpm_drv.h"
#include "usi_drv.h"
#include "sys.h"

/*******************************************************************************
* Function Name  : InverseChar
* Description    : Inverse UINT8 data
* Input          : ch:UINT8 data
* Output         : None
* Return         : Inversed data
*******************************************************************************/
static UINT8 InverseChar(UINT8 ch)
{
	UINT8 tmp=0;
	UINT8 i;
	ch =~ch;
	for(i=0;i<8;i++)
	{
		tmp<<=1;
		tmp|=(ch&1);
		ch>>=1;
	}
	return(tmp);
}

/*******************************************************************************
* Function Name  : delayETU
* Description    : Delay a ETU
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void delay1ETU(void)
{
	vu32 counter=0;		//一定要定义成volatile类型，防止编译器对代码进行优化而延时不准确
	
//	if (changeTa1 == 0)		//93us
//	{
//		for (counter=0; counter<(g_sys_clk/2000000*5); counter++);
//	}
//	else if (changeTa1 == 2)
//	{
//		for (counter=0; counter<(g_sys_clk/4000000*3); counter++);
//	}
//	else if (changeTa1 == 4)
//	{
//		for (counter=0; counter<(g_sys_clk/5000000*3); counter++);
//	}
	
	if (g_sys_clk == EFLASH_SYS_CLK_120M)
	{
		if (changeTa1 == 0)		//93us
		{
			for (counter=0; counter<244; counter++);
		}
		else if (changeTa1 == 2)
		{
			for (counter=0; counter<122; counter++);
		}
		else if (changeTa1 == 4)
		{
			for (counter=0; counter<61; counter++);
		}
	}
}

/*******************************************************************************
* Function Name  : DelayNETU
* Description    : Delay N ETUs
* Input          : count: Delay Parame
* Output         : None
* Return         : None
*******************************************************************************/
void DelayNETU(UINT16 count)
{
	ushort i;
	for(i=0;i<count;i++)
	{
		delay1ETU();
	}
}
/*******************************************************************************
* Function Name  : SC_VoltageConfig
* Description    : Configures the card power voltage.
* Input          : - SC_Voltage: specifies the card power voltage.
*                    This parameter can be one of the following values:
*                        - MB525 SC_Voltage_5V: 5V cards.
*                        - MB525 SC_Voltage_3V: 3V cards.
*                        - NCN6001 0x01 1.8V
*                        - NCN6001 0x02 3V
*                        - NCN6001 0x03 5V
* Output         : None
* Return         : None
*******************************************************************************/
void SC_VoltageConfig(u32 SC_Voltage)
{
	guc_Class = SC_Voltage;
}

/*******************************************************************************
* Function Name  : Open_ICCard_Vcc_3V
* Description    : open ic card vcc = 5V
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_Vcc_5V(void)
{
	OPEN_ICCARD_5V;
}

/*******************************************************************************
* Function Name  : Open_ICCard_Vcc_3V
* Description    : open ic card vcc = 3V
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_Vcc_3V(void)
{
	OPEN_ICCARD_3V;
}

/*******************************************************************************
* Function Name  : Open_ICCard_Vcc_1V8
* Description    : open ic card vcc = 1V8
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_Vcc_1V8(void)
{
	OPEN_ICCARD_1V8;
}


/*******************************************************************************
* Function Name  : Close_ICCard_Vcc
* Description    : close ic card vcc
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Close_ICCard_Vcc(void)
{
	CLOSE_ICCARD_5V;
	CLOSE_ICCARD_3V;
	CLOSE_ICCARD_1V8;
}

/*******************************************************************************
* Function Name  : Open_ICCard_VCC
* Description    : open ic card vcc
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_VCC(void)
{
	if (guc_Class == 1)		// 1.8v
	{
		Open_ICCard_Vcc_1V8();
	}
	else if (guc_Class == 2)	// 3v
	{
		Open_ICCard_Vcc_3V();
	}
	else		// 5v
	{
		Open_ICCard_Vcc_5V();
	}
}

/*******************************************************************************
* Function Name  : SC_Init
* Description    : Initializes all peripheral used for Smartcard interface.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SC_Init(void)
{
	USIMasterInit();
}

/*******************************************************************************
* Function Name  : SC_HotInit
* Description    : Smart card 热复位初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SC_HotInit(void)
{
	USIMasterHotInit();
}

/*******************************************************************************
* Function Name  : SC_DeInit
* Description    : Deinitializes all ressources used by the Smartcard interface.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SC_DeInit(void)
{
	USI_DeInit();
	DelayMS(1);
	Close_ICCard_Vcc();		//关闭7816电源
}

/*******************************************************************************
* Function Name  : SC_RSTSET
* Description    : Sets or clears the Smartcard reset pin.
* Input          : - ResetState: this parameter specifies the state of the Smartcard
*                    reset pin.
*                    BitVal must be one of the BitAction enum values:
*                       - Bit_RESET: to clear the port pin.
*                       - Bit_SET: to set the port pin.
* Output         : None
* Return         : None
*******************************************************************************/
void SC_RSTSET(UINT8 ResetState)
{
	if(!ResetState)
		g_USIReg->USIPDR &= ~0x01; //isorst L
	else
		g_USIReg->USIPDR |= 0x01; //isorst H
}


/*******************************************************************************
* Function Name  : USART_ByteReceive
* Description    : Receives a new data while the time out not elapsed.
* Input          : None
* Output         : None
* Return         : An ErrorStatus enumuration value:
*                         - SUCCESS: New data has been received
*                         - ERROR: time out was elapsed and no further data is
*                                  received
*******************************************************************************/
ErrorStatus USART_ByteReceive(UINT8 *Data, UINT32 TimeOut)
{
	UINT32 Counter = 0;
	UINT8  recetimes=0, temp;
	
	temp = temp;

accept1:
	k_IccErrPar=0;
	k_timeover=0;
	k_Ic_TimeCount=0x00;//每个字符接收前清0

	//使用WTC
	while( (g_USIReg->USISR & USISR_SB_MASK) != USISR_SB_MASK && (Counter != TimeOut))
	{
		Counter++;
		if ((g_USIReg->USISR&USISR_WTO_MASK) == USISR_WTO_MASK)
		{
			*Data = g_USIReg->USIRDR;	//清除缓冲区中可能存在的数据
			return ERROR;
		}
		if (k_total_timeover==1)		//总时间超时
		{
			k_total_timeover=0;
			return ERROR;
		}
	}

	if(Counter != TimeOut)
	{
		USI_OpenWTC(k_IcMax_EtuCount);		//配置收下一个字节的等待时间。WTC在收到起始位时自动加载，所以为避免误差，在收到SB信号后再启动WTC
		//等待收完整一个字节
		Counter = 0;
		while( (g_USIReg->USISR & USISR_RDRF_MASK) != USISR_RDRF_MASK && (Counter != TimeOut))
		{
			Counter++;
		}
	}

	if((g_USIReg->USISR & 0X08) == 0X08)
	{
		k_IccErrPar=1;//奇偶错误*/
		//清除错误标志(reading USISR and then reading USIRDR)
		temp=g_USIReg->USISR;

		if((k_IccErrPar==1)&&((Parity_Check_Enable==1)))//只有奇偶错误且为T=0协议情况下
		{
			temp=g_USIReg->USIRDR;		//继续清除错误标志
			if(recetimes<4)
			{
				recetimes++;
				goto accept1;
			}
			else
			{  return ERROR;}///110620
		}
	}

	if(Counter != TimeOut)
	{
		*Data = g_USIReg->USIRDR;
		if (SC_Slot_Active->SC_Convention==1) *Data=InverseChar(*Data);
			return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}


/*******************************************************************************
* Function Name  : USART_TsReceive
* Description    : Receives a new data while the time out not elapsed.
* Input          : None
* Output         : None
* Return         : An ErrorStatus enumuration value:
*                         - SUCCESS: New data has been received
*                         - ERROR: time out was elapsed and no further data is
*                                  received
*******************************************************************************/

ErrorStatus USART_TsReceive(UINT8 *Data, UINT32 TimeOut)
{
	UINT32 Counter = 0;
	k_IccErrPar=0;
	k_timeover=0;
	k_Ic_TimeCount=0x00;//每个字符接收前清0

	//FPGA
	g_USIReg->USICR2 = 0x80;			//接收模式奇偶校验错时自动产生错误信号，只T0有作用
	g_USIReg->USIFIFOINTCON |= 0x20;	//关闭奇偶校验

#ifdef ISO7816_AUTO_RETRANSMIT
	g_USIReg->USICR2 |= 0x0C; 			//打开错误自动重发功能，重发4次，即共发送5次
#endif

	//使用WTC
	while( (g_USIReg->USISR & USISR_SB_MASK) != USISR_SB_MASK && (Counter != TimeOut))
	{
		Counter++;
		if ((g_USIReg->USISR&USISR_WTO_MASK) == USISR_WTO_MASK)
		{
			*Data = g_USIReg->USIRDR;	//清除缓冲区中可能存在的数据
			return ERROR;
		}
	}

	if(Counter != TimeOut)
	{
		USI_OpenWTC(k_IcMax_EtuCount);		//配置收下一个字节的等待时间。WTC在收到起始位时自动加载，所以为避免误差，在收到SB信号后再启动WTC
		//等待收完整一个字节
		Counter = 0;
		while( (g_USIReg->USISR & USISR_RDRF_MASK) != USISR_RDRF_MASK && (Counter != TimeOut))
		{
			Counter++;
		}
	}

	if(Counter != TimeOut)
	{
		*Data = g_USIReg->USIRDR;

		g_USIReg->USIFIFOINTCON &= ~0x20;		//打开奇偶校验

		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}

/*******************************************************************************
* Function Name  : SC_GetByte
* Description    : Get byte from IO
* Input          : Data pointer
* Output         : None
* Return         : None
*******************************************************************************/

ErrorStatus SC_GetByte(UINT8 *Data)
{
	UINT32 TimeOut;

	TimeOut=0x8FFFFF;
	return USART_ByteReceive(Data, TimeOut);
}

/*******************************************************************************
* Function Name  : SC_PutByte
* Description    : Put byte
* Input          : Data to send
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 SC_PutByte(UINT8 Data)
{
#ifndef ISO7816_AUTO_RETRANSMIT		//使用自动重发
	UINT8  sendtimes = 0;
	UINT16 i;
#endif
	UINT8  test1;
	
	test1 = test1;
	k_IccErrPar = 0;

#ifdef ISO7816_AUTO_RETRANSMIT		//使用自动重发
	if (SC_Slot_Active->SC_Convention==1) Data=InverseChar(Data);
	TransmitByte(Data);

	if(Parity_Check_Enable==0)  		//T=1不重发
	{
		//T=1接收方检测到奇偶校验错时不会产生错误信号(低电平)，所以在发送时发送方不知道接收方是否接收正确
		return 0x00;
	}
	if((g_USIReg->USISR & 0X08) == 0X08)
	{
		k_IccErrPar=1;//奇偶错误*/

		 //清除错误标志(reading USISR and then writing 1 to ERROR bit)
		test1=g_USIReg->USISR;
		g_USIReg->USISR |= 0x08;
		return 0xff;
	}

	return 0x00;		//成功

#else		//使用手动重发
resendb:
	if (SC_Slot_Active->SC_Convention==1) Data=InverseChar(Data);
	TransmitByte(Data);

	if((g_USIReg->USISR & 0X08) == 0X08)
	{
		k_IccErrPar=1;//奇偶错误*/

		 //清除错误标志(reading USISR and then writing 1 to ERROR bit)
		test1=g_USIReg->USISR;
		g_USIReg->USISR |= 0x08;
	}

	if(Parity_Check_Enable==0)  return 0x00;//T=1不重发
	if((Parity_Check_Enable==1)&&(k_IccErrPar==1))//T=0重发
	{
		if(sendtimes<4)
		{
			k_IccErrPar=0;//奇偶错误
			sendtimes++;
			goto resendb;
		}
		else
		{
			return 0xff;
		}
	}
	return 0x00;		//成功


#endif
}



