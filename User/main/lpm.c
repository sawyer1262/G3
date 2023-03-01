#include "lpm.h"
#include "cpm_drv.h"
#include "key.h"
#include "debug.h"
#include "cache_drv.h"
#include "reset_drv.h"
#include "led.h"
#include "card.h"
#include "uart.h"  
#include "audio.h"
#include "uart_face.h"
#include "uart_fp.h"
#include "uart_back.h"
#include "bat.h"
#include "motor.h"
#include "delay.h"
#include "pci_drv.h"
#include "tsi_drv.h"

volatile uint32_t StopModeDisable = 0;                    //低功耗状态
uint32_t ForcePowerDownCount=0;                           //强制休眠计数
uint32_t LpmErrorCount=0;
uint8_t HwDeinit=0;
/******************************************************************************/
/*
//设置外围模块低功耗状态
input:id-----需要设置的模块ID
			mode---LPM_Disable:模块工作未完成，不能进入低功耗
						 LPM_Enable:模块空闲，可以进入低功耗
output:none
*/
/******************************************************************************/
void LPM_SetStopMode(LPM_Id_t id, LPM_SetMode_t mode)    
{
    __disable_irq();
    switch(mode)
    {
        case LPM_Disable:
            StopModeDisable |= (uint32_t)id;
            break;
        case LPM_Enable:
            StopModeDisable &= ~(uint32_t)id;
            break;
        default:break;
    }
    __enable_irq();
}
/******************************************************************************/
/*
//获取当前设备外围低功耗状态
input:none
output:当前低功耗信息
*/
/******************************************************************************/
uint32_t LPM_GetStopModeValue(void)
{
		uint32_t retDat=0;
		retDat=StopModeDisable;
    return retDat;
}

/******************************************************************************/
/*
//复位低功耗值
input:none
output:
*/
/******************************************************************************/
void LPM_ResettopModeValue(void)
{
		__disable_irq();
		StopModeDisable=0;
		__enable_irq();
}

/******************************************************************************/
/*
//异步定时器定时唤醒设置
CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE：定时时间，秒为单位
input:none
output:
*/
/******************************************************************************/
void AsycTimerInit(void)
{
		PCI_Init(EXTERNAL_CLK_SEL);
		PCI->PCI_CR &= ~(0xffu << 24); // NVSRAM don`t Clear
		PCI_InitWakeup();              // PCI reinit.
}

/******************************************************************************/
/*
//进入低功耗外围处理
input:none
output:none
*/
/******************************************************************************/
void HW_DeInit(void)
{
		if(HwDeinit==1)return;
		Motor_DeInit();
		SSI_FLASH_DeInit();      //FLASH进入PD
		TampSetButtonDeInit();   //SET与TAMPER开启唤醒 
		Key_DeInit();            //触摸按键SLEEP
		Led_DeInit();            //关LED
		CardDeInit();            //关闭刷卡 
		Audio_DeInit();          //关闭语音  
		Uart_Deinit();           //关闭相应串口模块
		AsycTimerInit();
		PCI_AsyncTC_Init(CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE*2000);
		HwDeinit=1;
}


void ATE_PowerDownDeinit(void)
{
		//ATE掉电模式，只能指纹唤醒，或者断电再上电
		Motor_DeInit();
		SysConfig.Bits.PirOn=0;  //关PIR，不允许PIR唤醒
		SSI_FLASH_DeInit();      //FLASH进入PD
		Key_DeInit();            //TSI进低功耗
		_CLEAR_IE_AWD(TSI);      //关闭IE	
		_CLEAR_IE_SCANFINISH(TSI);
		_CLEAR_IE_DELAY_CHECK(TSI);
		Led_DeInit();            //关LED
		CardDeInit();            //关闭刷卡 
		Audio_DeInit();          //关闭语音  
		UartFace_Deinit();       //人脸电源口与IO
		UartFp_Deinit();         //指纹电源口与IO
		
		//后板初始化，不允许唤醒，防止意外唤醒烧微安表
		UART_ConfigGpio(UART_BACK_SCI,UART_RX,GPIO_INPUT);         //RX输入，默认上拉
		UART_ConfigGpio(UART_BACK_SCI,UART_TX,GPIO_OUTPUT);        //TX输出高 
		UART_WriteGpioData(UART_BACK_SCI,UART_TX,Bit_SET);
		ATE_CPM_PowerOff_1p5();      //wakeup引脚能唤醒，也即指纹能唤醒 
}
/******************************************************************************/
/*
//循环检测是否进入低功耗
input:none
output:none
*/
/******************************************************************************/
void LPM_EnterLowPower(void)
{                    
		//上电延时10S进入低功耗
		if((LPM_GetStopModeValue()&LPM_POWERON_ID)==LPM_POWERON_ID && GetTimerElapse(0)>10000)   //上电延时10S进低功耗
		{
				LPM_SetStopMode(LPM_POWERON_ID,LPM_Enable);
		}
		//非OTA模式50S强制休眠
		if((StopModeDisable&LPM_OTA_ID)!=LPM_OTA_ID && (StopModeDisable&LPM_KEYPAD_ID)!=LPM_KEYPAD_ID 
				&& (StopModeDisable&LPM_FP_ID)!=LPM_FP_ID && (StopModeDisable&LPM_MOTOR_ID)!=LPM_MOTOR_ID)
		{
				if(ForcePowerDownCount==0)                                       //开始强制休眠计数
				{
						ForcePowerDownCount=GetTimerCount();
				}
				else if(GetTimerElapse(ForcePowerDownCount)>50000)               //主界面超时50S，强制休眠
				{
						//ForcePowerDownCount=0;                                     //不清0
						printf("StopModeDisable: 0x%08x\n",StopModeDisable);
						StopModeDisable=0;
				}
		}
		else
		{
				ForcePowerDownCount=0;
		}
		//WAKEUP引脚为低才能进入低功耗
		if(StopModeDisable == 0 && CPM_ReadGpioData(CPM_WAKEUP_PIN)==0)  
		{         
				uint16_t trycount=0;
				while(1)
				{
						HW_DeInit();
						//进入低功耗，如果此时WAKEUP引脚为高的话，就进不了低功耗，5次进不了低功耗就重启
						printf("goto POFF1.5... %08x\r\n", CPM->CPM_PADWKINTCR);
						CPM_PowerOff_1p5();
						if(++trycount>=5)
						{
								trycount=0;
								Set_POR_Reset();
						}
				}
		}
		else if(StopModeDisable == 0)        
		{
				if(LpmErrorCount == 0)
				{
						LpmErrorCount=GetTimerCount();
				}
				else if(GetTimerElapse(LpmErrorCount)>10000)    //wakeup 引脚为高，超时20S直接休眠。
				{
						LpmErrorCount=0;
						printf("23232323232\n");
						HW_DeInit();
						Set_POR_Reset();
				}
		}
		else if(StopModeDisable>0)
		{
				LpmErrorCount=0;
		}
}




















