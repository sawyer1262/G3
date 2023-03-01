// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : rtc_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "rtc_drv.h"
#include "common.h"
#include "debug.h"
#include "delay.h"
#include "cpm_reg.h"



void RTC_DateDemo(void)
{
	tm time = {10, 10, 50, 50};
    
	RTC_Init(EXTERNAL_CLK_SEL);
//	RTC_Init(INTERNAL_CLK_SEL);

	RTC_SetTime(time);
	time.second = 20;
	RTC_SetAlarm(time, SECOND_ALARM);

	while(1)
	{
		printf("g_rtc_int_sta = 0x%08x\n", g_rtc_int_sta);
		if (g_rtc_int_sta&Ala_intf)
		{
			g_rtc_int_sta &= ~Ala_intf;
			printf("alarm interrupt\n");
		}
		RTC_GetTime(&time);
		printf("time : D-%d [%d:%d:%d]\n", time.day, time.hour, time.minute, time.second);
        DelayMS(1000);
	}
}

void RTC_WakeupDemo(void)
{
    tm time = {10, 23, 59, 55};
    
	RTC_Init(EXTERNAL_CLK_SEL);
//	RTC_Init(INTERNAL_CLK_SEL);

    RTC_WakeupConfig(MIN_PULSE_WK);
	RTC_SetTime(time);
    
	while(1)
	{
		RTC_GetTime(&time);
		printf("time : D-%d [%d:%d:%d]\n", time.day, time.hour, time.minute, time.second);
        printf("enter power off 1.5...\r\n");
        CPM_PowerOff_1p5();
	}

}

void RTC_Demo(void)
{
    RTC_DateDemo();
    
//    RTC_WakeupDemo();
}

