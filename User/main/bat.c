#include "adc_drv.h"
#include "debug.h"
#include "bat.h"
#include "audio.h"
#include "led.h"
#include "lpm.h"
#include "timer.h"
#include "lock_config.h"
#include "uart_back.h"

uint8_t  BatLow=0;                          //低电压标志
uint8_t BatPerCent=0;
uint16_t BatRaw[BAT_RAW_MAX]={0};           //电压AD值缓存
uint16_t PowerOnCheckBatDelay=0;


const uint16_t BatPercentCalTab[11]={6500,6840,7020,7160,7240,7300,7360,7460,7580,7740,7940};


#define   ADC_REF       33					  //参考电压3.3V
#define   BAT_ADC_CHL 	ADCCH_1       //电池检查ADC通道



//上电在灯亮之前测试下电池电压
//电池电压一直测，过程中跳变太大得数据直接丢弃，休眠前保存上次得电压值，
//不允许电池电压掉电太快，1分钟只允许掉电一定得数值（比如0.1V），电压持续测试
//滑动滤波,休眠前要把电压值存入ROM
/*****************************************************************************/
/* 
   锂电池电池电量与电池电压粗略对照表格
11	 100%----4.20V
10　 90%-----3.97V          100
09　 80%-----3.87V          90   
08　 70%-----3.79V          80   
07　 60%-----3.73V          70
06　 50%-----3.68V          60 
05　 40%-----3.65V          50
04　 30%-----3.62V          40
03　 20%-----3.58V          30 
02　 10%-----3.51V          20
01　 5%------3.42V          10
00　 0%------3.00V          0     

	 干电池电池电量与电池电压粗略对照表格
*/
/******************************************************************************/

/******************************************************************************/
/*
//读电池电压--100ms一次
input:   none
output   none
return   none 
*/
/******************************************************************************/
void Bat_GetVoltage(void)
{
		uint32_t resVC=0,i=0;
		
		resVC = ADC_GetConversionValue(BAT_ADC_CHL)&0xFFFF;         //Vref引脚接的3.3V
		
	//	resVC = 323*ADC_REF*resVC/4095+30;                          //(33/4095)*res;y=kx+b
//		resVC = 26*resVC/10+30; 
//			resVC+=500;                                             //误差0.9，人为加上0.9V偏执 
		
		for(i=(BAT_RAW_MAX-1);i>0;i--)
		{
				BatRaw[i]=BatRaw[i-1];
		}
		BatRaw[0]=resVC;
}

/******************************************************************************/
/*
//获取电池百分比
input:   none
output   none
return   none 
*/
/******************************************************************************/
uint8_t GetVoltagePercent(uint16_t volt)
{
		uint8_t retc=0;
#ifdef  LIPOWER	
		uint8_t i=0;
		for(i=0;i<11;i++)
		{
				if(volt<BatPercentCalTab[i])break;
		}
		if(i==0)
		{
				retc=0;
		}
		else if(i<11)
		{
				retc=(uint8_t)((uint16_t)(volt-BatPercentCalTab[i-1])*10/(BatPercentCalTab[i]-BatPercentCalTab[i-1]))+(i-1)*10;
		}
		else 
		{
				retc=100;
		}
#else
		if(volt<AAABAT_LOW_VAL)       //
		{
				retc=0;
		}
		else if(volt<AAABAT_HI_VAL)
		{
				retc=(volt-AAABAT_LOW_VAL)*100/(AAABAT_HI_VAL-AAABAT_LOW_VAL);
		}
		else
		{
				retc=100;
		}
#endif
		return retc;
}

/******************************************************************************/
/*
//计算电池电压：1S一次
input:   none
output   none
return   none 
*/
/******************************************************************************/
void Bat_PrintVoltage(void)
{
		uint16_t i=0,min=0,max=0;
		uint16_t numsum=0,numcal=0;
		uint32_t volsum=0;
		uint16_t batval=0;
		uint16_t batvolatge=0;
		tm rtc_times={0};
		static uint16_t lowCount=0;
	
		min=max=BatRaw[0];            
		
		for(i=0;i<BAT_RAW_MAX;i++)
		{
				if(BatRaw[i]==0)break;
				if(BatRaw[i]<min)min=BatRaw[i];
				if(BatRaw[i]>max)max=BatRaw[i];
		}
		if(i==0)return;             //缓存中未有数据：初次上电延时5S
		numcal=numsum=i;
		for(i=0;i<numsum;i++)
		{
				if(BatRaw[i]<(max-BAT_CHK_GAP))      //电流变化过大，值丢弃
				{
						numcal--;
				}
				else
				{
						volsum+=BatRaw[i];
				}
		}
		if(numcal==0)return;        //缓存中没有有效数据
		batval=volsum/numcal;
		batvolatge = 26*batval/10; 
		
//		printf("numsum:%2d;numcal:%2d\n",numsum,numcal);
//		printf("batval:%4d\n",batval);
		printf("batvoltage:%4d\n",batvolatge);
		
		if(LowBatAlarm.Bat.Value==0)                    //首次上电
		{
				RTC_GetTime(&rtc_times);               
				LowBatAlarm.Bat.Day=rtc_times.day;
				LowBatAlarm.Bat.Hour=rtc_times.hour;
				LowBatAlarm.Bat.Minute=rtc_times.minute;
				LowBatAlarm.Bat.Second=rtc_times.second;
				LowBatAlarm.Bat.Value=batvolatge;
				IntFlashWrite(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t));
		}
		
		if(batvolatge<(LowBatAlarm.Bat.Value-100))     //掉电超过0.1V，判断时间是否超过1分钟
		{
				mytm_t tm={0};
				tm.day=LowBatAlarm.Bat.Day;
				tm.hour=LowBatAlarm.Bat.Hour;
				tm.minute=LowBatAlarm.Bat.Minute;
				tm.second=LowBatAlarm.Bat.Second;
				if(GetPeripLockTimeElapse(&tm)>60)
				{
						RTC_GetTime(&rtc_times);               
						LowBatAlarm.Bat.Day=rtc_times.day;
						LowBatAlarm.Bat.Hour=rtc_times.hour;
						LowBatAlarm.Bat.Minute=rtc_times.minute;
						LowBatAlarm.Bat.Second=rtc_times.second;
						if(batvolatge<LowBatAlarm.Bat.Value-150)
						{
								LowBatAlarm.Bat.Value-=150;
						}
						else
						{
								LowBatAlarm.Bat.Value=batvolatge;
						}
						IntFlashWrite(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t));
				}
		}
		//计算电池百分比
		BatPerCent=GetVoltagePercent(LowBatAlarm.Bat.Value);
		//printf("vol percent:%02d\n",BatPerCent);
#ifdef   LIPOWER
		if(batvolatge<LIBAT_LOW_VAL)       //低压,两个一起检测，防止人为调高电源而报警
#else
		if(batvolatge<AAABAT_LOW_VAL) 
#endif
		{
				if(++lowCount<3)return;
		}

		else
		{
				lowCount=0;
		}
#ifdef   LIPOWER
		if(LowBatAlarm.Bat.Value<LIBAT_LOW_VAL && batvolatge<LIBAT_LOW_VAL)       //低压,两个一起检测，防止人为调高电源而报警
#else
		if(LowBatAlarm.Bat.Value<AAABAT_LOW_VAL && batvolatge<AAABAT_LOW_VAL) 
#endif
		{
				if(BatLow==0)
				{
						uint8_t temp[4]={1,batvolatge%256,batvolatge/256,BatPerCent};
						BatLow=1;
						UartBack_SendCmdData(ALARM_REPORT,temp,4,1,1000);
				}
		}
}






















