#include "motor.h"
#include "eport_drv.h"  
#include "timer.h"
#include "lpm.h"
#include "audio.h"
#include "lock_record.h"
#include "bat.h"
#include "uart_back.h"

volatile uint16_t MotorRunStartTime=0;
volatile uint16_t MotorDelayCloseStartTime=0;
uint16_t ResetMotorDecCnt=0;


void Motor_Init(void)
{
		EPORT_ConfigGpio(MOTOR1_PIN,GPIO_OUTPUT);
		EPORT_PullConfig(MOTOR1_PIN,EPORT_DISPULL); 
		EPORT_ConfigGpio(MOTOR2_PIN,GPIO_OUTPUT);
		EPORT_PullConfig(MOTOR2_PIN,EPORT_DISPULL); 
		EPORT_WriteGpioData(MOTOR1_PIN,Bit_RESET);
		EPORT_WriteGpioData(MOTOR2_PIN,Bit_RESET);
}

void Motor_DeInit(void)
{
		EPORT_ConfigGpio(MOTOR1_PIN,GPIO_OUTPUT);
		EPORT_PullConfig(MOTOR1_PIN,EPORT_DISPULL); 
		EPORT_ConfigGpio(MOTOR2_PIN,GPIO_OUTPUT);
		EPORT_PullConfig(MOTOR2_PIN,EPORT_DISPULL); 
		EPORT_WriteGpioData(MOTOR1_PIN,Bit_RESET);
		EPORT_WriteGpioData(MOTOR2_PIN,Bit_RESET);
}


void Motor_Open(void)
{
		if(MotorDelayCloseStartTime>0 || MotorRunStartTime>0)
		{
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Door open":"锁已开"),UNBREAK);
				return;
		}
		EPORT_WriteGpioData(MOTOR2_PIN,Bit_RESET);
		EPORT_WriteGpioData(MOTOR1_PIN,Bit_SET);
		__disable_irq();
		MotorRunStartTime=350;
		if(SysConfig.Bits.KeepOpen!=1)MotorDelayCloseStartTime=6000;
		else MotorDelayCloseStartTime=0;
		__enable_irq();
		LPM_SetStopMode(LPM_MOTOR_ID,LPM_Disable);
}

void Motor_Close(void)
{
		EPORT_WriteGpioData(MOTOR1_PIN,Bit_RESET);
		EPORT_WriteGpioData(MOTOR2_PIN,Bit_SET);
		__disable_irq();
		MotorRunStartTime=350;
		MotorDelayCloseStartTime=0;
		__enable_irq();
		LPM_SetStopMode(LPM_MOTOR_ID,LPM_Disable);
}

void Motor_Stop(void)
{
		EPORT_WriteGpioData(MOTOR1_PIN,Bit_RESET);
		EPORT_WriteGpioData(MOTOR2_PIN,Bit_RESET);
		MotorRunStartTime=0;
		if(MotorDelayCloseStartTime==0)
		{
				LPM_SetStopMode(LPM_MOTOR_ID,LPM_Enable);
		}
}


void Motor_Running(void)
{
		if(MotorRunStartTime>0)
		{
				if(--MotorRunStartTime==0)
				{
						LockStat.Lock=EPORT_ReadGpioData(MOTOR1_PIN);       //1为开
						Motor_Stop();
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?(LockStat.Lock==0?"Door close":"Door open"):(LockStat.Lock==0?("锁已关"):"锁已开")),UNBREAK);
						if(BatLow==1 && LockStat.Lock==1)           //开锁+低电量
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Low battery, please replace it":"电量不足,请更换电池"),UNBREAK);
						}
				}
		}
		if(MotorDelayCloseStartTime>0)
		{
				if(--MotorDelayCloseStartTime==0)
				{
						UserControlLock(CTL_CLOSE_LOCK,0xff,0xff);   
				}
		}
		if(ResetMotorDecCnt>0)
		{
				if(--ResetMotorDecCnt==0)
				{
						UserControlLock(CTL_CLOSE_LOCK,0xff,0xff); 
				}
		}
}



















