/*
 * time_stamp_api.c
 *
 *  Created on: 2019年5月17日
 *      Author: fjzhang
 */
#include "time_stamp_api.h"

unsigned int months_table[13] = {0,31,59,90,120,151,181,212,243,273,304,334,367};
unsigned char week_table[7] = {6,0,1,2,3,4,5};//START_YEARS=1973
int8_t week[15] = "一二三四五六日";

unsigned short Get_Days(t_ctimes init_time)
{
	unsigned char nonleap_years;
	unsigned char leap_years;
	unsigned char full_years;
//	unsigned char  months;
	unsigned short days;
	
	full_years = (init_time.years - START_YEARS);
	leap_years = full_years/4;
	nonleap_years = full_years - leap_years;
	
	days = leap_years*366 + nonleap_years*365;
	
	days += months_table[init_time.months-1];
	
	days += init_time.days;
	
	if((0 == init_time.years%4)&& (2 < init_time.months))
	{
	 days += 1;
	}
	
	return days;
	
}

void Set_Time(t_ctimes init_stamp_time, tm *rtc_times)
{
	unsigned char nonleap_years;
	unsigned char leap_years;
	unsigned char full_years;
	
	if((init_stamp_time.years <= START_YEARS) || (init_stamp_time.years >= END_YEARS))
	{
		init_stamp_time.years = START_YEARS;
	}
	
	full_years = (init_stamp_time.years - START_YEARS);
    
    leap_years = full_years/4;
	
	nonleap_years = full_years - leap_years;
	
	rtc_times->day = (leap_years*366 + nonleap_years*365);
	
	rtc_times->day += months_table[init_stamp_time.months-1];
	
	rtc_times->day += init_stamp_time.days;
	
	if((0 == init_stamp_time.years%4)&& (2 < init_stamp_time.months))
	{
        rtc_times->day += 1;
	}
	
	rtc_times->hour = init_stamp_time.hours;
	rtc_times->minute = init_stamp_time.minutes;
	rtc_times->second = init_stamp_time.seconds; 
}

void Get_Time(t_ctimes *current_stamp_time,tm rtc_times)
{
	unsigned short days;
	unsigned char leap_year_num;
//	unsigned char year_num;
	unsigned char i;
	
//	RTC_GetTime(&rtc_times);
	
	current_stamp_time->seconds = rtc_times.second;
	current_stamp_time->minutes = rtc_times.minute;
	current_stamp_time->hours = rtc_times.hour;
	
	current_stamp_time->weeks = week_table[rtc_times.day%7];
    
    leap_year_num = (rtc_times.day)/1461;
	
	days = (rtc_times.day%1461); //
    
    if(0 == days)
    {
        current_stamp_time->years = 3+ (leap_year_num-1)*4 +START_YEARS;
        current_stamp_time->months = 12;
		current_stamp_time->days = 31;
    }
    else
    {
        if(1155 <= days)//365*3+31+29=1155
        {
            current_stamp_time->years = 3+ leap_year_num*4 +START_YEARS;
            days = (days -365*3)%366;
            months_table[2] = 60;
            months_table[3] = 91;
            months_table[4] = 121;
            months_table[5] = 152;
            months_table[6] = 182;
            months_table[7] = 213;
            months_table[8] = 244;
            months_table[9] = 274;
            months_table[10] = 305;
            months_table[11] = 335;
        }
        else
        {
            current_stamp_time->years = (rtc_times.day%1461-1)/365 + leap_year_num*4 +START_YEARS;
            days = days%365;
            months_table[2] = 59;
            months_table[3] = 90;
            months_table[4] = 120;
            months_table[5] = 151;
            months_table[6] = 181;
            months_table[7] = 212;
            months_table[8] = 243;
            months_table[9] = 273;
            months_table[10] = 304;
            months_table[11] = 334;
        }

        i = 0;
        
        while(months_table[i] < days)
        {
            i ++;
        }
        
        if(i == 0)
        {
            current_stamp_time->months = 12;
            current_stamp_time->days = 31;
        }
        else
        {
            current_stamp_time->months = i;
            current_stamp_time->days = (days - months_table[i-1]);
        }
    
    }
}

void RTC_TimeStamp_Demo(void)
{
	unsigned char second = 0;
	t_ctimes gt;
	t_ctimes st = {2020,01,01,0,23,59,58};
	tm rtc_times;
	
	Set_Time(st,&rtc_times);
	
	RTC_Init(EXTERNAL_CLK_SEL);
//	RTC_Init(INTERNAL_CLK_SEL);
	
	DelayMS(500);
	RTC_SetTime(rtc_times);
	
	while(1)
	{
		DelayMS(50);
		RTC_GetTime(&rtc_times);	//获取芯片RTC时间
		Get_Time(&gt,rtc_times);	//获取时间戳
		
		if(second != gt.seconds)
		{
			printf("Data: %04d年%02d月%02d日 星期%c%c %02d:%02d:%02d\r\n",gt.years,gt.months,gt.days,week[(gt.weeks)*2],week[(gt.weeks)*2+1],gt.hours,gt.minutes,gt.seconds);
			second = gt.seconds;
		}
		
	}
	
}



