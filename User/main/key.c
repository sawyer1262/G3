#include "tsi_app.h"
#include "key.h"
#include "uart_face.h"
#include "uart_fp.h"
#include "timer.h"
#include "string.h"
#include "audio_demo.h"
#include "audio.h"
#include "led.h"
#include "keypad_menu.h"
#include "lpm.h"
#include "ioctrl_drv.h"
#include "card.h"
#include "led.h"
#include "pwm_drv.h"
#include "rtc.h"
#include "wdt_drv.h"
#include "uart.h"
#include "bat.h"
#include "lock_record.h"
#include "uart_back.h"



//防拆,SET按键，与PIR
typedef uint8_t (*GetPinStat)(uint8_t tPin);
typedef struct{
		uint8_t Pin;            //引脚
		uint8_t Dly;            //延时
		uint8_t Stat;          //状态 
		uint32_t HiCnt;         //高电平计数
		uint32_t LoCnt;         //低电平计数
		uint32_t CntBk;         //高低电平计数备份
		GetPinStat Fun;         //获取状态函数 
}BtnStr_t;
BtnStr_t BtnStr[3];


//触摸按键
KeyStat_t KeyStat={0};
uint32_t keydowncount=0;
uint32_t keyupcount=0;
uint32_t KeyDownStartTime=0;
uint32_t KeyUpStartTime=0;
//防拆与报警
uint8_t LockAlarm=0;
uint32_t TampStartTime=0;
//PIR
uint8_t isPirOn=0;
uint32_t lingerstart=0;

/******************************************************************************/
/*
//防拆，SET按键，PIR初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void TampSetButtonInit(void)
{
		if(SysConfig.Bits.PirOn==1)
		{
				PowerOnPir();
		}
		else
		{
				PowerOffPir();
		}
		EPORT_ConfigGpio(KEY_TAMPER_PIN,GPIO_INPUT);         //防拆：低电平中断
		EPORT_PullConfig(KEY_TAMPER_PIN,EPORT_DISPULL);      //关闭上下拉，外部有2M弱上拉
		
		//读防拆相关
		IntFlashRead(TamperAlarm.TampBuf,TAMPER_ADDR,sizeof(TamperAlarm_t)); 
		if(TamperAlarm.Tamp.Flag!=0xAA || TamperAlarm.Tamp.Time==0 || TamperAlarm.Tamp.Time>60)
		{
				TamperAlarm.Tamp.Flag=0xAA;
				TamperAlarm.Tamp.Time=10;                                    //默认10S报警 
				TamperAlarm.Tamp.Stat=EPORT_ReadGpioData(KEY_TAMPER_PIN);    //首次上电，读出状态并保存
				TamperAlarm.Tamp.Day=0;
				TamperAlarm.Tamp.Hour=0;
				TamperAlarm.Tamp.Minute=0;
				TamperAlarm.Tamp.Second=0; 
				IntFlashWrite(TamperAlarm.TampBuf,TAMPER_ADDR,sizeof(TamperAlarm_t));
		}
		EPORT_ConfigGpio(KEY_BACK_PIN,GPIO_INPUT);          //后板按键：低电平中断
		EPORT_PullupConfig(KEY_BACK_PIN,ENABLE);            //使能上拉 
	
		BtnStr[KEY_IDX_SET].Pin=KEY_BACK_PIN;                         //SET按键
		BtnStr[KEY_IDX_SET].Fun=(GetPinStat)EPORT_ReadGpioData;     
	
		BtnStr[KEX_IDX_TAMP].Pin=KEY_TAMPER_PIN;                       //防拆
		BtnStr[KEX_IDX_TAMP].Fun=(GetPinStat)EPORT_ReadGpioData; 
		
		BtnStr[KEX_IDX_PIR].Pin=CPM_WAKEUP_PIN;                       //PIR唤醒
		BtnStr[KEX_IDX_PIR].Fun=(GetPinStat)CPM_ReadGpioData; 
}
/******************************************************************************/
/*
//防拆，SET按键，PIR除能
input:   none
output   none
return   none
*/
/******************************************************************************/
void TampSetButtonDeInit(void)
{
		EPORT_Init(KEY_TAMPER_PIN,RISING_FALLING_EDGE_INT);	
		EPORT_Init(KEY_BACK_PIN,FALLING_EDGE_INT);
}

/******************************************************************************/
/*
//打开PIR电源
input:   none
output   none
return   none
*/
/******************************************************************************/
void PowerOnPir(void)
{
		isPirOn=1;
		EPORT_ConfigGpio(PIR_POWER_PIN,GPIO_OUTPUT);
		EPORT_WriteGpioData(PIR_POWER_PIN,Bit_RESET);
}

/******************************************************************************/
/*
//关闭PIR电源
input:   none
output   none
return   none
*/
/******************************************************************************/
void PowerOffPir(void)
{
		isPirOn=0;
		EPORT_ConfigGpio(PIR_POWER_PIN,GPIO_OUTPUT);
		EPORT_WriteGpioData(PIR_POWER_PIN,Bit_SET);
}

/******************************************************************************/
/*
//触摸按键初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void Key_Init(void)
{
		app_tsi_init(&htsi); 
}
/******************************************************************************/
/*
//触摸按键休眠
input:   none
output   none
return   none
*/
/******************************************************************************/
void Key_DeInit(void)
{
		app_tsi_handle(&htsi);
		app_tsi_sleep(&htsi);
}

/******************************************************************************/
/*
//根据触摸值返回按键值
input:   TchNum -- 触摸值
output   none
return   keyC -- 按键值
*/
/******************************************************************************/
uint8_t Key_GetNum(uint8_t TchNum)
{
		uint8_t keyC=0;
	
		switch(TchNum)
		{
				case 0:                    //5
						keyC=KEY_c;
						break;
				case 1:
						keyC=KEY_5;           //8
						break;
				case 2:
						keyC=KEY_2;        //0
						break;
				case 3:
						keyC=KEY_6;           //7
						break;
				case 4:
						keyC=KEY_3;           //*
						break;
				case 5:
						keyC=KEY_1;          //#
						break;
				case 6:
						keyC=KEY_4;          //9
						break;
				case 7:                  
						keyC=KEY_x;          //3
						break;
				case 8:
						keyC=KEY_7;           //6
						break;
				case 9:
						keyC=KEY_bell;           //bell
						break;
				case 10:
						keyC=KEY_8;           //1
						break;
				case 11:
						keyC=KEY_9;           //4
						break;
				case 12:
						keyC=KEY_0;           //2
						break;
		}
		return keyC;
}
/******************************************************************************/
/*
//按键扫描
input:   none
output   none
return   none
*/
/******************************************************************************/
void Key_Scan(void)
{
		static uint8_t KeyDownBk=0xFF; 
		static KeyStat_t LastKeyStat={0};
	
		if(b20msFlag==1)
		{
				static uint8_t it=0;
				b20msFlag=0;
				TampSetButton_Scan();
				if(++it>=50)
				{
						it=0;
						WDT_FeedDog();
				}
		}
		if(b100msFlag==1)
		{
				b100msFlag=0;
				if(GetTimerElapse(0)>PowerOnCheckBatDelay)             //延时1S再测电压
				{
						Bat_GetVoltage();
				}
				WriteUserOpera();
		}
		app_tsi_handle(&htsi);  
		
		if(KeyDownBk==0xFF)KeyDownBk=htsi.Touch.Current;
		
		if(SysConfig.Bits.KeyPADLock==1)
		{
				LedVal.ledval=0;
				KeyStat.Flag=0;
				if(htsi.Touch.Current < TSI_MULTI_VALID)
				{
						KeyUpStartTime=0;
						LPM_SetStopMode(LPM_KEYPAD_ID,LPM_Disable);
						if(activeAudioNumber!=GetVolIndex(SysConfig.Bits.Language?"Too many fails, locked for 3 minutes":"验证错误次数过多,系统锁定3分钟"))
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Too many fails, locked for 3 minutes":"验证错误次数过多,系统锁定3分钟"),BREAK);
						}
				}
				else
				{
						KeyDownStartTime=0;
						if(KeyUpStartTime==0)KeyUpStartTime=GetTimerCount();
						else if(GetTimerElapse(KeyUpStartTime)>KEY_INPUT_TIMEOUT)
						{
								LPM_SetStopMode(LPM_KEYPAD_ID,LPM_Enable);
						}
				}
		}
		else
		{
				if(htsi.Touch.Current < TSI_MULTI_VALID)               //有按键按下
				{
						LPM_SetStopMode(LPM_KEYPAD_ID,LPM_Disable);
						KeyUpStartTime=0;
						if(htsi.Touch.Current!=KeyDownBk)
						{
								KeyDownBk=htsi.Touch.Current;
								KeyDownStartTime=GetTimerCount();
						}
						else if(KeyDownStartTime>0)
						{
								if(GetTimerElapse(KeyDownStartTime)>10)
								{
										KeyDownStartTime=0;
										
										if(LastKeyStat.Numb!=Key_GetNum(htsi.Touch.Current))
										{
												KeyStat.Numb=Key_GetNum(htsi.Touch.Current);
												KeyStat.Flag=1;
												KeyStat.STime=GetTimerCount();
										}
										else if(GetTimerElapse(LastKeyStat.STime)>200)
										{
												KeyStat.Numb=Key_GetNum(htsi.Touch.Current);
												KeyStat.Flag=1;
												KeyStat.STime=GetTimerCount();
										}
								}
						}
				}
				else                                                
				{		
						KeyDownStartTime=0;
						if(KeyStat.STime==0)KeyStat.STime=GetTimerCount();
					
						if(KeyDownBk!=htsi.Touch.Current)
						{
								KeyUpStartTime=GetTimerCount();
								KeyDownBk=htsi.Touch.Current;
						}
						else if(KeyUpStartTime>0)
						{
								if(GetTimerElapse(KeyUpStartTime)>10)
								{
										KeyUpStartTime=0;
										LastKeyStat=KeyStat;          //按键弹起保存上次的值
										KeyStat.Flag=0;
										KeyStat.STime=GetTimerCount();
								}
						}
				}	
		}		
}
/******************************************************************************/
/*
//防拆与SET按键扫描
input:   none
output   none
return   none
*/
/******************************************************************************/
void TampSetButton_Scan(void)
{
		uint8_t i=0;
		//更新状态
		for(i=0;i<3;i++)
		{
				if(BtnStr[i].Dly>0)                //延时消抖
				{
						BtnStr[i].Dly--;
						continue;
				}
				if(BtnStr[i].Fun(BtnStr[i].Pin))
				{
						BtnStr[i].Stat=1;
						BtnStr[i].CntBk=BtnStr[i].LoCnt;  
						BtnStr[i].LoCnt=0;     
				}
				else
				{
						BtnStr[i].Stat=0;
						BtnStr[i].CntBk=BtnStr[i].HiCnt;
						BtnStr[i].HiCnt=0;
				}
		}
		//更新计数
		for(i=0;i<3;i++)
		{
				if(BtnStr[i].Stat)BtnStr[i].HiCnt++;
				else BtnStr[i].LoCnt++;
		}
		//后板按键处理
		if(BtnStr[0].Stat==0)         //按下
		{
				if(BtnStr[0].LoCnt>=SET_FACTORY_RESET_TIME)         //5s
				{
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Factory reset":"恢复出厂设置"),BREAK);  
						LockFactoryResetOpera();
						//复位人脸模块：因为人脸模块识别时间比较长，在人脸识别过程中对他进行其他操作会出错
						FaceReady.Ready=0;
						UartFace_TxQueueReset();
						UartFace_PowerReset();
					
						FaceDel.DelStat=FACEDEL_READUSER;
						FaceDel.StTime=0;
						FaceDel.Retry=0;
					
						FpDel.LdFpDelStat=FP_DEL_READ_USER;
						FpDel.StTime=0;
						FpDel.ComRetry=0;
					
						uint8_t DeFpOk=0;
						uint8_t DelDone=0;
						uint8_t FpFailRetry=0;
						while(1)
						{
								WDT_FeedDog();               //喂狗
								if(DeFpOk==0)
								{	
										switch(FpDelHandle(0xFF))
										{
												case FP_SUCCESS:       //删除成功
														DeFpOk=1;
														FpFailRetry=0;
														printf("del fp ok\n");
														break;
												case FP_FAIL:          //删除失败，重试
														if(++FpFailRetry>=3)
														{
																FpFailRetry=0;
																DeFpOk=2;
														}
														else
														{
																FpDel.LdFpDelStat=FP_DEL_READ_USER;
																FpDel.StTime=0;
																FpDel.ComRetry=0;
														}
														printf("del fp fail\n");
														break;
												default:break;
										}
								}
								if(DeFpOk==1 && DelDone==0)
								{
										DelDone=1;
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Setting succeed":"设置成功"),UNBREAK);
										FactoryReset();            //删除卡和密码
										LedVal.ledval=0;
								}
								else if(DeFpOk==2 && DelDone==0)
								{
										DelDone=1;
										LedVal.ledval=0;
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),UNBREAK);
								}
								Uart_RtxFrameHandle();
								Audio_RunningHandle();
								Led_UpdateVal();
								if(DelDone==1 && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0 && BtnStr[0].Fun(BtnStr[0].Pin))
								{
										Set_POR_Reset();
								}
						}
				}
				LPM_SetStopMode(LPM_BTN_BACK_ID,LPM_Disable);
		}
		else                      //弹起
		{
				if(BtnStr[0].CntBk>1)           //按键弹起，报验证管理员语音  
				{
						if(KeyPadMode!=KEYPAD_CONFIG )   
						{
								KeyPadMode=KEYPAD_CONFIG;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please verify administrator":"请验证管理员"),UNBREAK);  
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Enter management mode":"进入管理模式"),UNBREAK);
						}
				}
				LPM_SetStopMode(LPM_BTN_BACK_ID,LPM_Enable);
		}
		//防拆处理		
		if(TamperAlarm.Tamp.Stat!=BtnStr[1].Stat)
		{
				if(BtnStr[1].LoCnt>=2 || BtnStr[1].HiCnt>=2)
				{
						TamperAlarm.Tamp.Stat=BtnStr[1].Stat;
						//写入FLASH防拆状态
						tm rtc_times={0};
						RTC_GetTime(&rtc_times);
						TamperAlarm.Tamp.Day=rtc_times.day;
						TamperAlarm.Tamp.Hour=rtc_times.hour;
						TamperAlarm.Tamp.Minute=rtc_times.minute;
						TamperAlarm.Tamp.Second=rtc_times.second;
						IntFlashWrite(TamperAlarm.TampBuf,TAMPER_ADDR,sizeof(TamperAlarm_t)); 
						//如果是报警则写入记录并启动报警
						if(TamperAlarm.Tamp.Stat==0 && SysConfig.Bits.TampAct==1)     //防拆使能打开，才报警
						{
								LockAlarm=1;
								TampStartTime=GetTimerCount();
								LPM_SetStopMode(LPM_TAMP_ID,LPM_Disable);
								LockTampTrigOpera();
						}
				}
		}
		if(LockAlarm==1 && TampStartTime>0)
		{
				if(GetTimerElapse(TampStartTime)<TamperAlarm.Tamp.Time*1000)
				{
						if(activeAudioNumber!=GetVolIndex("防拆报警音"))
						{
								AudioPlayVoice(GetVolIndex("防拆报警音"),BREAK);
						}
				}
				else 
				{
						TampStartTime=0;
						LPM_SetStopMode(LPM_TAMP_ID,LPM_Enable);
				}
		}
		//PIR处理	
}



/******************************************************************************/
/*
//PIR检测
input:   none
output   none
return   none
*/
/******************************************************************************/
void PirLingerCheck(void)
{
		if(SysConfig.Bits.Linger==1)
		{
				if(LingTime.LTT.Stat==0)      //未触发徘徊报警
				{
						if(CPM_ReadGpioData(CPM_WAKEUP_PIN)==1)   //非指纹脚高
						{
								if(lingerstart==0)
								{
										lingerstart=GetTimerCount();
								}
								else if(GetTimerElapse(lingerstart)>(uint32_t)LingTime.LTT.TrigT*1000)
								{
										uint8_t dat=7;
										UartBack_SendCmdData(ALARM_REPORT,&dat,1,1,1000);             //系统锁定上报
										lingerstart=0;
										//保存报警时间
										tm rtc_times={0};
										RTC_GetTime(&rtc_times);
										LingTime.LTT.Stat=1;
										LingTime.LTT.Day=rtc_times.day;
										LingTime.LTT.Hour=rtc_times.hour;
										LingTime.LTT.Minute=rtc_times.minute;
										LingTime.LTT.Second=rtc_times.second;
										LingTime.LTT.Flag=0x55;
										IntFlashWrite(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
								}
						}
				}
		}
		else
		{
				if(LingTime.LTT.Stat==1)                 //未使能徘徊报警的时候清除原来的数据
				{
						LingTime.LTT.Stat=0;
						LingTime.LTT.Day=0;
						LingTime.LTT.Hour=0;
						LingTime.LTT.Minute=0;
						LingTime.LTT.Second=0;
						LingTime.LTT.Flag=0x55;
						IntFlashWrite(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
				}
		}
}













