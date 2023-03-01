#include "rtc.h"
#include "rtc_drv.h"
#include "debug.h"
#include "delay.h"
#include "timer.h"
#include "lock_config.h"
#include "flash.h"
#include "wdt_drv.h"
#include "password.h"
#include "audio.h"
#include "bat.h"
#include "uart_back.h"
#include "password.h"
#include "time_cal.h"
#include "motor.h"


t_ctimes RtcSetTime={2022,04,17,3,18,20,55};               //RTC起始时间


//芯片上电初始时间：1972/12/31/00:00:00


/******************************************************************************/
/*
//RTC初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void RtcInit(void)
{
		t_ctimes gt={0};
		t_ctimes st = RtcSetTime;  
		tm rtc_times={0}; 
	
		RTC_Init(EXTERNAL_CLK_SEL);           //使能RTC外部晶振 
		
		RTC_GetTime(&rtc_times);	            //获取芯片RTC时间
		printf("Data: %04d %02d:%02d:%02d\r\n",rtc_times.day,rtc_times.hour,rtc_times.minute,rtc_times.second);
		
		rtctime_to_mytime(&gt,rtc_times);	    //转换成自定义时间
		printf("Data: %04d %02d %02d %02d:%02d:%02d\r\n",gt.years,gt.months,gt.days,gt.hours,gt.minutes,gt.seconds);
		printf("weekday:%02d\n",gt.weeks);
		if(gt.years==1972)                    //芯片初始时间转换后为1972年
		{
				printf("set time\n");
				//重新上电，关锁
				ResetMotorDecCnt=2000;
				//重新上电，清锁标志
				SysConfig.Bits.FaceLock=0;
				SysConfig.Bits.FpLock=0;
				SysConfig.Bits.CardLock=0;
				SysConfig.Bits.KeyPADLock=0;
				PeripLock.Perip.FailTime[0]=0;
				PeripLock.Perip.FailTime[1]=0;
				PeripLock.Perip.FailTime[2]=0;
				PeripLock.Perip.FailTime[3]=0;
				//重设时间
				mytime_to_rtc(st,&rtc_times);
				RTC_SetTime(rtc_times);
		}
}
/******************************************************************************/
/*
//更新外设验证失败次数，此处没有对FLASH进行写操作，放在休眠前更新FLASH。
input:   idx--外设序号：
						PERIP_FACE_INDEX        0-人脸
						PERIP_FP_INDEX          1-指纹
						PERIP_CODE_INDEX        2-键盘密码
						PERIP_CARD_INDEX        3-卡片

				 stat--外设状态 0-清0计数，1-计数+1
output   none
return   none 
*/
/******************************************************************************/
void LockUpdatePeripLockStat(uint8_t idx,uint8_t stat)
{
		if(idx>PERIP_CARD_INDEX)return;
	
		if(stat==0)
		{
				PeripLock.Perip.FailTime[idx]=0;
				return;
		}
		//判断外设是否已锁，如果已经锁定就不处理
		switch(idx)
		{
				case PERIP_FACE_INDEX:
						if(SysConfig.Bits.FaceLock==1)return;
						break;
				case PERIP_FP_INDEX:
						if(SysConfig.Bits.FpLock==1)return;
						break;
				case PERIP_CODE_INDEX:
						if(SysConfig.Bits.KeyPADLock==1)return;
						break;
				case PERIP_CARD_INDEX:
						if(SysConfig.Bits.CardLock==1)return;
						break;
				default:return;
		}
		tm rtc_times={0};
		if(PeripLock.Perip.FailTime[idx]<5)PeripLock.Perip.FailTime[idx]++;
		if(PeripLock.Perip.FailTime[idx]==1)               //首次验证失败，记录时间
		{
				RTC_GetTime(&rtc_times);
				PeripLock.Perip.FailStart[idx].day=rtc_times.day;
				PeripLock.Perip.FailStart[idx].hour=rtc_times.hour;
				PeripLock.Perip.FailStart[idx].minute=rtc_times.minute;
				PeripLock.Perip.FailStart[idx].second=rtc_times.second;
		}
		else if(PeripLock.Perip.FailTime[idx]>=5)         //超过规定次数，锁定外设，并记录锁定起始时间 
		{
				switch(idx)
				{
						case PERIP_FACE_INDEX:
								SysConfig.Bits.FaceLock=1;
								break;
						case PERIP_FP_INDEX:
								SysConfig.Bits.FpLock=1;
								break;
						case PERIP_CODE_INDEX:
								SysConfig.Bits.KeyPADLock=1;
								break;
						case PERIP_CARD_INDEX:
								SysConfig.Bits.CardLock=1;         //lock mask 0x00000780
								break;
						default:return;
				}
				RTC_GetTime(&rtc_times);
				PeripLock.Perip.FailTime[idx]=0;
				PeripLock.Perip.LockStart[idx].day=rtc_times.day;
				PeripLock.Perip.LockStart[idx].hour=rtc_times.hour;
				PeripLock.Perip.LockStart[idx].minute=rtc_times.minute;
				PeripLock.Perip.LockStart[idx].second=rtc_times.second;
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Too many fails, locked for 3 minutes":"验证错误次数过多,系统锁定3分钟"),UNBREAK); 
		}
}
/******************************************************************************/
/*
//更新外设锁定状态,1S检测一次
input:   none
output   none
return   none 
*/
/******************************************************************************/
void LockCheckPeripLockStat(void)
{
		int32_t elapset=0;
		
		if(b1sFlag!=1)return;                  //1S检测一次
		b1sFlag=0;
	
		//电池电压计算与检测
		Bat_PrintVoltage();
		//验证人脸锁
		if(SysConfig.Bits.FaceLock==1)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.LockStart[PERIP_FACE_INDEX]);
			
				if(elapset>180 || elapset<0)        //锁定3分钟结束，或者重新上电，接触锁定
				{
						SysConfig.Bits.FaceLock=0;
				}
		}
		else if(PeripLock.Perip.FailTime[PERIP_FACE_INDEX]>0)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.FailStart[PERIP_FACE_INDEX]);
				if(elapset>300 || elapset<0)        //5分钟超时，或重新上电，复位失败计数
				{
						PeripLock.Perip.FailTime[PERIP_FACE_INDEX]=0; 
				}
		}
		//验证指纹锁
		if(SysConfig.Bits.FpLock==1)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.LockStart[PERIP_FP_INDEX]);
				if(elapset>180 || elapset<0)
				{
						SysConfig.Bits.FpLock=0;
				}
		}
		else if(PeripLock.Perip.FailTime[PERIP_FP_INDEX]>0)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.FailStart[PERIP_FP_INDEX]);
				if(elapset>300 || elapset<0)      //5分钟清0
				{
						PeripLock.Perip.FailTime[PERIP_FP_INDEX]=0; 
				}
		}
		//验证键盘锁
		if(SysConfig.Bits.KeyPADLock==1)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.LockStart[PERIP_CODE_INDEX]);
				if(elapset>180 || elapset<0)
				{
						SysConfig.Bits.KeyPADLock=0;
				}
		}
		else if(PeripLock.Perip.FailTime[PERIP_CODE_INDEX]>0)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.FailStart[PERIP_CODE_INDEX]);
				if(elapset>300 || elapset<0)      //5分钟清0
				{
						PeripLock.Perip.FailTime[PERIP_CODE_INDEX]=0; 
				}
		}
		//验证刷卡锁
		if(SysConfig.Bits.CardLock==1)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.LockStart[PERIP_CARD_INDEX]);
				if(elapset>180 || elapset<0)
				{
						SysConfig.Bits.CardLock=0;
						printf("card Lock resume\n");
				}
		}
		else if(PeripLock.Perip.FailTime[PERIP_CARD_INDEX]>0)
		{
				elapset=GetPeripLockTimeElapse(&PeripLock.Perip.FailStart[PERIP_CARD_INDEX]);
				if(elapset>300 || elapset<0)      //5分钟清0
				{
						printf("alapset: %02d\n",elapset);
						PeripLock.Perip.FailTime[PERIP_CARD_INDEX]=0; 
				}
		}
		//验证一次性密码时效
		if(SysConfig.Bits.OTCode==1)     //一次性24小时失效
		{
				elapset=GetPeripLockTimeElapse(&OneTimeCode.StartTime);
				if(elapset>86400 || elapset<0)          //24小时失效
				{
						SysConfig.Bits.OTCode=0;
						printf("onetime code fail\n");
				}
		}
		//验证临时密码是否失效
		CheckTempCodeValid();
}
/******************************************************************************/
/*
//获取当前时间与指定时间之间的时间差

input:   stime--指定时间
output   none
return   时间差：
            -1 ----- 当前时间小于指定时间
            >0 ----- 时间差
*/
/******************************************************************************/
int32_t GetPeripLockTimeElapse(mytm_t *stime)
{
		tm rtc_times={0};
		uint32_t timNow=0;
		uint32_t timStart=stime->hour*3600+stime->minute*60+stime->second;
		
		RTC_GetTime(&rtc_times);
		
		timNow=rtc_times.hour*3600+rtc_times.minute*60+rtc_times.second;
		
		if(rtc_times.day>=stime->day)
		{
				timNow+=(rtc_times.day-stime->day)*86400;
				return (timNow-timStart);
		}
		else
		{
				return -1;
		}
}
/******************************************************************************/
/*
//与服务器同步时间，1S调用一次

input:   stime--指定时间
output   none
return   时间差：
            -1 ----- 当前时间小于指定时间
            >0 ----- 时间差
*/
/******************************************************************************/
void SnycNetTime(void)
{
		static uint8_t GetTimeCmdSend=0;
		mytm_t tm={0};
		
		if(NetTime.NT.Stat==0)                           //未同步过时间
		{
				if(GetTimeCmdSend==0)
				{
						GetTimeCmdSend=1;
						UartBack_SendCmdData(REQ_TIME,NULL,0,0,1000);     //获取时间指令，不重发
				}
		}
		else if(GetTimeCmdSend==0)                       
		{
				tm.day=NetTime.NT.Day;
				tm.hour=NetTime.NT.Hour;
				tm.minute=NetTime.NT.Minute;
				tm.second=NetTime.NT.Second;
				if(GetPeripLockTimeElapse(&tm)>28800)                 //8小时对时一次
				{
						UartBack_SendCmdData(REQ_TIME,NULL,0,0,1000);
						GetTimeCmdSend=1;
				}
		}
}



