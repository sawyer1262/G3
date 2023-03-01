/*
 * icc_protocol.c
 *
 *  Created on: 2017年5月26日
 *      Author: YangWenfeng
 */

#include "iccemv.h"
#include "usi_drv.h"
#include "delay.h"

#define LCLE
/*******************************************************************************
* Function Name  : SC_Reset
* Description    : SIM Card 复位
* Input          : mode  复位方式：热复位或者冷复位
*
* Output         : None
* Return         : 返回复位状态  0：复位成功   -1复位失败
******************************************************************************/
char SC_Reset(UINT8 mode)
{
	UINT8            flag=1;
	SC_ADPU_Commands locAdpu;
	SC_ADPU_Responce locResponse;
	SC_State        *locState;

	locState=&(SC_Slot_Active->SC_CurState);

	if (mode == COLD_RESET)
	{
		*locState=SC_POWER_ON;
	}
	else
	{
		SC_HotInit();
		*locState=SC_RESET_LOW;
	}

	SC_Slot_Active->SC_T1PCB=0;
	SC_Slot_Active->SC_Convention=0;
	locAdpu.Header.CLA = 0x00;
	locAdpu.Header.INS = SC_GET_A2R;
	locAdpu.Header.P1 = 0x00;
	locAdpu.Header.P2 = 0x00;
	locAdpu.Body.LC = 0x00;
	guc_hotrstFlag = 0;//清热复位标志
	asyncard_info[0].resetstatus=0;
	while(flag)
	{
		SC_Handler(&locAdpu, &locResponse);
		if (*locState== SC_ACTIVE_ON_T0||*locState == SC_ACTIVE_ON_T1)
		{
			if (*locState== SC_ACTIVE_ON_T0)
			{
				SC_T0_T1_Status = 0x00;                //T=0
			}
			else
			{
				SC_T0_T1_Status = 0x01;                //T=1
			}
			SC_A2R.SW1=0x90;
			SC_A2R.SW2=0x00;
			flag=0;

			return 0;
		}
		if (*locState == SC_POWER_OFF)
		{
			SC_A2R.SW1=0x6F;
			SC_A2R.SW2=0xF0;
			flag=0;

			//DelayMS(20);		//复位出错后延时20ms再下电，20140526
			//SC_DeInit();   //20170215，避免重复下电
			return (char)(-1);
		}
	}
	return (char)(-1);
}

/*******************************************************************************
* Function Name  : SC_Command
* Description    : Handles all Smartcard states and serves to send and receive all
*                  communication data between Smartcard and reader.
* Input          : - locadpu: pointer to an adpu buffer.
*                : adpulen : adpu'length
* Output         : - locresp: pointer to a Responce buffer.
*                : resplen : response's length which includes SW1&SW2
* Return         : SW1 & SW2
*******************************************************************************/
 UINT16 SC_Command(UINT16 adpulen, UINT8 * locadpu, UINT16 * resplen, UINT8 * locresp)
{
	UINT16            ret = 0;
	SC_ADPU_Commands  *SC_pADPU;
	SC_ADPU_Responce  * SC_pResponce;

	SC_pADPU = (SC_ADPU_Commands *) locadpu;
	SC_pResponce = (SC_ADPU_Responce  *) locresp;

	resplen[0] = 0;
	respLength = 0;

	if (adpulen<5)
	{
		if (adpulen==4)
		{
			SC_pADPU->Body.LE = 0;
			SC_pADPU->Body.LC = 0;
		}
		else
		{
			return   0x6700;
		}
	}
	else if  (adpulen ==5)		//le类型
	{
		SC_pADPU->Body.LE = locadpu[4];
		//guc_Leflag = 1;
		SC_pADPU->Body.LC = 0;
	}
	else	//lc+le类型
	{
		SC_pADPU->Body.LE = 0;
#ifdef LCLE
		if (adpulen==(SC_pADPU->Body.LC+6))		//cla ins p1 p2 lc data le
		{
			SC_pADPU->Body.LE=locadpu[adpulen-1];
			guc_Leflag = 1;
		}
		if (adpulen>(SC_pADPU->Body.LC+6))
			return   0x6700;
#endif

#ifndef LCLE
		if (adpulen!=(SC_pADPU->Body.LC+5))
			return   0x6700;
#endif
	}

	SC_Handler(SC_pADPU, SC_pResponce);

	resplen[0] =  respLength;		//如果通信出错，如超时则respLength为0

	if (respLength<2)
	{
		//DelayMS(20);		//通信出错后延时20ms再下电，20140526
		//SC_DeInit();   	//20170215，避免重复下电
		return  0x6FF0;
	}

	SC_pResponce->Data[respLength-2] = SC_pResponce->SW1;
	SC_pResponce->Data[respLength-1] = SC_pResponce->SW2;

	ret =  SC_pResponce->SW1;
	ret = (ret << 8) + SC_pResponce->SW2;

	return  ret;

}

/*******************************************************************************
* Function Name  : SC_Handler
* Description    : Handles all Smartcard states and serves to send and receive all
*                  communication data between Smartcard and reader.
* Input          : - SCState: pointer to an SC_State enumeration that will contain
*                    the Smartcard state.
*                  - SC_ADPU: pointer to an SC_ADPU_Commands structure that will be
*                    initialized.
*                  - SC_Response: pointer to a SC_ADPU_Responce structure which will
*                    be initialized.
* Output         : None
* Return         : None
*******************************************************************************/
void SC_Handler(SC_ADPU_Commands *SC_ADPU, SC_ADPU_Responce *SC_Response)
{
	UINT32 i = 0;
	SC_State *SCState;
	int ret;//110706返回值

	SCState=&(SC_Slot_Active->SC_CurState);

	switch(*SCState)
	{
		case SC_POWER_ON:
		{
			if (SC_ADPU->Header.INS == SC_GET_A2R)
			{
				/* Reset Data from SC buffer -----------------------------------------*/
				for (i = 0; i < 40; i++)
				{
					SC_ATR_Table[i] = 0;
				}

				/* Next State --------------------------------------------------------*/
				*SCState = SC_RESET_LOW;
			}
			break;
		}
		case SC_RESET_LOW:
		{
			/*Set smart card reset low*/
			if(guc_hotrstFlag)		//需要热复位
			{
				guc_hotrstFlag=0;//清0
				SC_HotInit();
				SC_RSTSET(0);
			}
			else	//冷复位
			{
				Open_ICCard_VCC();	//IC卡上电
				DelayMS(1);

				SC_Init();  //先上电，后开CLK
				SC_RSTSET(0);
			}
			DelayNETU(120);			//40000~45000个时钟周期, 40000/372=108etu
			*SCState = SC_RESET_HIGH;
			break;
		}
		case SC_RESET_HIGH:
		{
			SC_RSTSET(1);
			ret = Icc_Reset(0,(UINT8 *)SC_ATR_Table);

			if(ret==STATUS_SUCCESS)
			{
				*SCState = SC_ACTIVE;
			}
			else
			{
				if(guc_hotrstFlag)		//需要热复位
				{
					*SCState = SC_RESET_LOW;
				}
				else		//下电
				{
					*SCState= SC_POWER_OFF;
				}
			}
			break;
		}
		case SC_ACTIVE:
		{
			if (SC_ADPU->Header.INS == SC_GET_A2R)
			{
				if(SC_Slot_Active->SC_Protocol == T0_PROTOCOL)
				{
					(*SCState) = SC_ACTIVE_ON_T0;
				}
				else if(SC_Slot_Active->SC_Protocol == T1_PROTOCOL)
				{
					(*SCState) = SC_ACTIVE_ON_T1;
				}
				else
				{
					(*SCState) = SC_POWER_OFF;
				}
			}
			break;
		}
		case SC_ACTIVE_ON_T0:
		{
			ret = Icc_Command(0,SC_ADPU,SC_Response);
			if (ret)
			{
				(*SCState) = SC_POWER_OFF;
			}
			break;
		}
		case SC_ACTIVE_ON_T1:
		{
			ret = Icc_Command(0,SC_ADPU,SC_Response);
			if (ret)
			{
				(*SCState) = SC_POWER_OFF;
			}
			break;
		}
		case SC_POWER_OFF:
		{
			/* Disable Smartcard interface */
			//SC_DeInit();         //20170215，函数外有下电过程，避免重复下电
			break;
		}
		default:
		{
			(*SCState) = SC_POWER_OFF;
			break;
		}
	}
}

/*******************************************************************************
* Function Name  : SC_PollCard_Init
* Description    : SIM card insertion init detection
* Input          : None
*
* Output         : None
* Return         : None
*******************************************************************************/
void SC_PollCard(void)
{
	if(ICCARD_PRESENT)  //有卡
	{
		ICCardInsert = 1;
		if(ICCardResetOK == 0)  //复位没有成功
		{
			SC_DeInit();               //将7816口关闭，否则无法重新上电复位
			SC_VoltageConfig(3);		//5.0v，只是配置，没有上电
			DelayMS(100);
			if(SC_Reset(COLD_RESET)== 0)
			{
				ICCardResetOK = 1;
			}
			else
			{
				ICCardResetOK = 0;
			}
		}
	}
	else   //卡拔出
	{
		ICCardInsert = 0;
		ICCardResetOK = 0;
		DelayMS(100);
	}
	//USBDev_SendCardInfo(ICCardInsert,AUTO);		//只根据卡到位信号发送中断数据，而不根据是否读卡成功
}
