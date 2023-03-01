#include "led.h"
#include "pwm_drv.h"
#include "lpm.h"
#include "cpm_drv.h"
#include "bat.h"
#include "key.h"
#include "delay.h"

uint8_t isLedInit=0;
LedVal_t LedVal;                                 //键盘LED灯
LedChl_t LedChl[3];                              //PIR板 RGB灯  
LogoLedStat_t LogoLedStat=LOGO_WAKEUP;           //PIR灯板工作状态

/******************************************************************************/
/*
//LED初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
extern void IO_Latch_Clr(void);

void Led_Init(void)
{
		//LED键盘灯初始化
		if(isLedInit==1)return;
	
		EPORT_ConfigGpio(LED_OE_PIN,GPIO_OUTPUT);
		EPORT_WriteGpioData(LED_OE_PIN,Bit_SET);
		I2C_ConfigGpio(I2C3,LED_POWER_PIN,GPIO_OUTPUT);
		printf("allright here9\n");
		/*********************************************************/
		DelayMS(5);                                                
		/*********************************************************/
		printf("allright here8\n");
		I2C_WriteGpioData(I2C3,LED_POWER_PIN,Bit_RESET);
		DelayMS(5); 
		EPORT_ConfigGpio(LED_SCL_PIN,GPIO_OUTPUT);
		EPORT_ConfigGpio(LED_SDA_PIN,GPIO_OUTPUT);
		EPORT_ConfigGpio(LED_LE_PIN,GPIO_OUTPUT);
		EPORT_WriteGpioData(LED_SCL_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_SDA_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_LE_PIN,Bit_RESET);
		/********************************************************/
		IO_Latch_Clr();   //解锁放在这里，不然会死机
		/********************************************************/
		Led_UpdateVal();
		EPORT_WriteGpioData(LED_OE_PIN,Bit_RESET);       //输出使能
		//PIR led指示灯初始化
		PWM_OutputInit(PWM_R_PIN,9,PWM_CLK_DIV_1,PIR_LED_LIGHT_MAX,PIR_LED_LIGHT_MIN,1);
	  PWM_OutputInit(PWM_G_PIN,9,PWM_CLK_DIV_1,PIR_LED_LIGHT_MAX,PIR_LED_LIGHT_MIN,1);	      //debug口，调试时注意屏蔽
		PWM_OutputInit(PWM_B_PIN,9,PWM_CLK_DIV_1,PIR_LED_LIGHT_MAX,PIR_LED_LIGHT_MIN,1);	
	  printf("allright here6\n");     
		isLedInit=1;
}
/******************************************************************************/
/*
//led除能
input:   none
output   none
return   none
*/
/******************************************************************************/
void Led_DeInit(void)
{
		//关闭16206的输出
//		if(isLedInit==0)return;
		
//		LedVal.ledval=0;
//		Led_UpdateVal();  
	
		PWM_ConfigGpio(PWM_R_PIN,GPIO_OUTPUT,PWM_PULLUP_DIS);
		PWM_ConfigGpio(PWM_G_PIN,GPIO_OUTPUT,PWM_PULLUP_DIS);
		PWM_ConfigGpio(PWM_B_PIN,GPIO_OUTPUT,PWM_PULLUP_DIS);
		
		PWM_WriteGpioData(PWM_R_PIN,Bit_SET);
		PWM_WriteGpioData(PWM_G_PIN,Bit_SET);
		PWM_WriteGpioData(PWM_B_PIN,Bit_SET);
	
		IntFlashRead(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t));  
		if(SysConfig.Bits.PirOn==1)
		{
				PowerOnPir();
		}
		else
		{
				PowerOffPir();
		}
		EPORT_WriteGpioData(LED_OE_PIN,Bit_SET);       //关输出
		I2C_WriteGpioData(I2C3,LED_POWER_PIN,Bit_SET); //关电源
		//管理LED IO口
		EPORT_WriteGpioData(LED_SCL_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_SDA_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_LE_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_OE_PIN,Bit_RESET); 
		//关LOGO灯
		PirLed_SetBrightness(PWM_R_PIN,0);
		PirLed_SetBrightness(PWM_G_PIN,0);
		PirLed_SetBrightness(PWM_B_PIN,0);
		isLedInit=0;
}

void Led_WakeupDeInit(void)
{
		PWM_ConfigGpio(PWM_R_PIN,GPIO_OUTPUT,PWM_PULLUP_DIS);
		PWM_ConfigGpio(PWM_G_PIN,GPIO_OUTPUT,PWM_PULLUP_DIS);
		PWM_ConfigGpio(PWM_B_PIN,GPIO_OUTPUT,PWM_PULLUP_DIS);
		
		PWM_WriteGpioData(PWM_R_PIN,Bit_SET);
		PWM_WriteGpioData(PWM_G_PIN,Bit_SET);
		PWM_WriteGpioData(PWM_B_PIN,Bit_SET);
		
		IntFlashRead(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t));  
		if(SysConfig.Bits.PirOn==1)
		{
				PowerOnPir();
		}
		else
		{
				PowerOffPir();
		}
		EPORT_WriteGpioData(LED_OE_PIN,Bit_SET);       //关输出
		I2C_WriteGpioData(I2C3,LED_POWER_PIN,Bit_SET); //关电源
		//管理LED IO口
		EPORT_WriteGpioData(LED_SCL_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_SDA_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_LE_PIN,Bit_RESET);
		EPORT_WriteGpioData(LED_OE_PIN,Bit_RESET); 
}

/******************************************************************************/
/*
//设置PIR板RGB的亮度
input:   chl -- RGB通道
         percent -- 亮度百分比
output   none
return   none
*/
/******************************************************************************/
void PirLed_SetBrightness(uint8_t chl,uint8_t percent)       //percent:0-100
{
		if(chl>PWM_PORT2 || percent>100)return;
		
		if(LedChl[chl].val==percent)return;
	
		LedChl[chl].val=percent;
		switch(chl)
		{
				case PWM_PORT0:
						PWM->PWM_CR_CH0 &= ~CH0EN;
						PWM->PWM_CMR0=(uint32_t)PIR_LED_LIGHT_MAX*percent/100;
						PWM->PWM_CR_CH0 |= CH0EN;
						break;
				case PWM_PORT1:
						PWM->PWM_CR_CH1 &= ~CH1EN;
						PWM->PWM_CMR1=(uint32_t)PIR_LED_LIGHT_MAX*percent/100;
						PWM->PWM_CR_CH1 |= CH1EN;
						break;
				case PWM_PORT2:
						PWM->PWM_CR_CH2 &= ~CH2EN;
						PWM->PWM_CMR2=(uint32_t)PIR_LED_LIGHT_MAX*percent/100;
						PWM->PWM_CR_CH2 |= CH2EN;
						break;
		}
}
/******************************************************************************/
/*
//获取PIR板RGB的亮度
input:   chl -- RGB通道
output   none
return   chl通道的亮度
*/
/******************************************************************************/
uint8_t PirLed_GetBrightness(uint8_t chl)
{
		return LedChl[chl].val;
}

/******************************************************************************/
/*
//更新键盘LED灯状态
input:   none
output   none
return   none
*/
/******************************************************************************/
void Led_UpdateVal(void)
{
		static uint16_t LedValBk=0xFFFF;
		uint8_t i=0;
	
		if(BatLow==1)LedVal.bits.BAT=1;
		if(SysConfig.Bits.KeyPADLock==1)LedVal.ledval=0;            //如果键盘锁已开，则关闭显示	
		if(LedValBk!=LedVal.ledval)
		{
				//锁存数据
				EPORT_WriteGpioData(LED_LE_PIN,Bit_RESET);
				//传输数据
				printf("Ledval: 0x%02x\n",LedVal.ledval);
				for(i=0;i<16;i++)
				{
						EPORT_WriteGpioData(LED_SCL_PIN,Bit_RESET);
						if(LedVal.ledval&(1<<(15-i)))
						{
								EPORT_WriteGpioData(LED_SDA_PIN,Bit_SET);
						}
						else
						{
								EPORT_WriteGpioData(LED_SDA_PIN,Bit_RESET);
						}	
						EPORT_WriteGpioData(LED_SCL_PIN,Bit_SET);
				}
				//打入数据
				EPORT_WriteGpioData(LED_LE_PIN,Bit_SET); 
				//锁存数据
				EPORT_WriteGpioData(LED_LE_PIN,Bit_RESET);
				LedValBk=LedVal.ledval;
		}
}
/******************************************************************************/
/*
//获取PIR LOGOG灯板的状态
input:   none
output   none
return   LogoLedStat -- logo灯板状态
*/
/******************************************************************************/
LogoLedStat_t PirLed_GetStat(void)
{
		return LogoLedStat;
}

/******************************************************************************/
/*
//设置PIR LOGOG灯板的状态
input:   stat -- 设置的状态
output   none
return   0-OK
*/
/******************************************************************************/
uint8_t PirLed_SetStat(LogoLedStat_t stat)
{
		uint8_t i=0;
	
		LogoLedStat=stat;
		for(i=0;i<3;i++)
		{
				PirLed_SetBrightness(i,0);
				LedChl[i].state=0;
				LedChl[i].val=0; 
				LedChl[i].sttime=0;
		}
		if(stat!=LOGO_OFF)LPM_SetStopMode(LPM_BTN_PIR_ID,LPM_Disable);
		
		return 0;
}

/******************************************************************************/
/*
//PIR灯板状态处理
input:   none
output   none
return   none
*/
/******************************************************************************/
void PirLed_Handle(void)
{
		switch(LogoLedStat)
		{
				case LOGO_OFF:
						PirLed_SetBrightness(PWM_R_PIN,0);
						PirLed_SetBrightness(PWM_G_PIN,0);
						PirLed_SetBrightness(PWM_B_PIN,0);
						LPM_SetStopMode(LPM_BTN_PIR_ID,LPM_Enable);
						break;
				case LOGO_WAKEUP:        //蓝灯
						if(LedChl[PWM_B_PIN].state==0)
						{
								if(LedChl[PWM_B_PIN].sttime==0)
								{
										LedChl[PWM_B_PIN].sttime=GetTimerCount();
								}
								else if(GetTimerElapse(LedChl[PWM_B_PIN].sttime)>=12)
								{
										LedChl[PWM_B_PIN].sttime=0;
										PirLed_SetBrightness(PWM_B_PIN,LedChl[PWM_B_PIN].val+1);      //12000/100
								}
								if(LedChl[PWM_B_PIN].val>=99)
								{
										LedChl[PWM_B_PIN].state=1;
										LedChl[PWM_B_PIN].sttime=0;
								}
						}
						else if(LedChl[PWM_B_PIN].state==1)
						{
								if(LedChl[PWM_B_PIN].sttime==0)
								{
										LedChl[PWM_B_PIN].sttime=GetTimerCount();
								}
								else if(GetTimerElapse(LedChl[PWM_B_PIN].sttime)>=1000)
								{
										LedChl[PWM_B_PIN].sttime=0;
										LedChl[PWM_B_PIN].state=2;
								}
						}
						else if(LedChl[PWM_B_PIN].state==2)
						{
								if(LedChl[PWM_B_PIN].sttime==0)
								{
										LedChl[PWM_B_PIN].sttime=GetTimerCount();
								}
								else if(GetTimerElapse(LedChl[PWM_B_PIN].sttime)>=8)
								{
										LedChl[PWM_B_PIN].sttime=0;
										PirLed_SetBrightness(PWM_B_PIN,LedChl[PWM_B_PIN].val-1);      //12000/100
								}
								if(LedChl[PWM_B_PIN].val>=1)
								{
										PirLed_SetBrightness(PWM_B_PIN,0);  
										PirLed_SetStat(LOGO_OFF);
								}
						}
						break;
				case LOGO_OPEN_SUCCESS:
						if(LedChl[PWM_G_PIN].state==0)
						{
								if(LedChl[PWM_G_PIN].sttime==0)
								{
										LedChl[PWM_G_PIN].sttime=GetTimerCount();
								}
								else if(GetTimerElapse(LedChl[PWM_G_PIN].sttime)>=12)
								{
										LedChl[PWM_G_PIN].sttime=0;
										PirLed_SetBrightness(PWM_G_PIN,LedChl[PWM_G_PIN].val+1);      //12000/100
								}
								if(LedChl[PWM_G_PIN].val>=99)
								{
										LedChl[PWM_G_PIN].state=1;
										LedChl[PWM_G_PIN].sttime=0;
								}
						}
						else if(LedChl[PWM_G_PIN].state==1)
						{
								if(LedChl[PWM_G_PIN].sttime==0)
								{
										LedChl[PWM_G_PIN].sttime=GetTimerCount();
								}
								else if(GetTimerElapse(LedChl[PWM_G_PIN].sttime)>=8)
								{
										LedChl[PWM_G_PIN].sttime=0;
										PirLed_SetBrightness(PWM_G_PIN,LedChl[PWM_G_PIN].val-1);      //12000/100
								}
								if(LedChl[PWM_G_PIN].val>=1)
								{
										PirLed_SetBrightness(PWM_G_PIN,0);  
										PirLed_SetStat(LOGO_OFF);
								}
						}
						break;
				case LOGO_OPEN_SUCCESS_MUTIL:
						if(LedChl[PWM_B_PIN].state>=20)
						{
								PirLed_SetBrightness(PWM_B_PIN,0);
								PirLed_SetStat(LOGO_OFF);
						}
						else if(LedChl[PWM_B_PIN].sttime==0)
						{
								LedChl[PWM_B_PIN].sttime=GetTimerCount();
						}
						else if(GetTimerElapse(LedChl[PWM_B_PIN].sttime)>=300)
						{
								static uint8_t val=0;
								LedChl[PWM_B_PIN].sttime=GetTimerCount();
								PirLed_SetBrightness(PWM_B_PIN,val); 
								LedChl[PWM_B_PIN].state++;
								if(val)val=0;
								else val=100;	
						}
						break;
				case LOGO_OPEN_FAIL:
						if(LedChl[PWM_R_PIN].state==0)
						{
								if(LedChl[PWM_R_PIN].sttime==0)
								{
										LedChl[PWM_R_PIN].sttime=GetTimerCount();
								}
								else if(GetTimerElapse(LedChl[PWM_R_PIN].sttime)>=12)
								{
										LedChl[PWM_R_PIN].sttime=0;
										PirLed_SetBrightness(PWM_R_PIN,LedChl[PWM_R_PIN].val+1);      //12000/100
								}
								if(LedChl[PWM_R_PIN].val>=99)
								{
										LedChl[PWM_R_PIN].state=1;
										LedChl[PWM_R_PIN].sttime=0;
								}
						}
						else if(LedChl[PWM_R_PIN].state==1)
						{
								if(LedChl[PWM_R_PIN].sttime==0)
								{
										LedChl[PWM_R_PIN].sttime=GetTimerCount();
								}
								else if(GetTimerElapse(LedChl[PWM_R_PIN].sttime)>=8)
								{
										LedChl[PWM_R_PIN].sttime=0;
										PirLed_SetBrightness(PWM_R_PIN,LedChl[PWM_R_PIN].val-1);      //12000/100
								}
								if(LedChl[PWM_R_PIN].val>=1)
								{
										PirLed_SetBrightness(PWM_R_PIN,0);  
										PirLed_SetStat(LOGO_OFF);
								}
						}
						break;
		}
}





















