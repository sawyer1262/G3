#include "time_cal.h"
#include "debug.h"

const uint8_t g_day_per_mon[MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};     //0-11
uint16_t months_table[13] = {0,31,59,90,120,151,181,212,243,273,304,334,367};
/******************************************************************************/
/*
//判断指定年份是不是闰年
input:   year-年份，比如2022
output   none
return   0-平年 1-闰年 
*/
/******************************************************************************/
uint8_t is_leap_year(uint16_t year)
{
		if((year%400)==0)
		{
				return 1;
		} 
		else if((year%100)==0) 
		{
				return 0;
		} 
		else if((year%4)==0) 
		{
				return 1;
		} 
		else 
		{
				return 0;
		}
}

/******************************************************************************/
/*
//判断指定年份是不是闰年
input:   month-月数1-12
         year--年份
output   none
return   0-无效月份  其他值为正确的天数 
*/
/******************************************************************************/
uint8_t get_day_of_mon(uint8_t month, uint16_t year)
{
		if ((month==0)||(month>12)) 
		{
				return 0;
		}

		if (month!=2) 
		{
				return g_day_per_mon[month-1];
		} 
		else 
		{
				return g_day_per_mon[1]+is_leap_year(year);
		}
}

/******************************************************************************/
/*
//计算给定日期是星期几
input:   	
         year--年份
				 month-月数1-12
				 day---日	
output   none
return   星期几：0-6    0-星期天
*/
/******************************************************************************/
uint8_t get_weekday(uint16_t year, uint8_t month, uint8_t day)
{
		char century_code=0,year_code=0,month_code=0,day_code=0;
		int32_t week=0;

		if((month==1)||(month==2)) 
		{
				century_code=(year-1)/100;
				year_code=(year-1)%100;
				month_code=month+12;
				day_code=day;
		} 
		else 
		{
				century_code=year/100;
				year_code=year%100;
				month_code=month;
				day_code=day;
		}
		week=year_code+(year_code/4)+(century_code/4)-(2*century_code)+(26*( month_code+1)/10)+day_code-1;
		week=week%7;
		return week;
}

/******************************************************************************/
/*
//UTC时间戳转换为自定义时间
input:   	
         utc_sec--UTC时间戳
				 daylightSaving---是否夏令时
output   result-输出自定义时间
return   none
*/
/******************************************************************************/
void utc_to_mytime(uint32_t utc_sec, t_ctimes *result, uint8_t daylightSaving)
{
		int32_t sec, day;
		uint16_t y;
		uint8_t m;
		uint16_t d;

		if(daylightSaving) 
		{
				utc_sec+=SEC_PER_HOUR;
		}
		/* hour, min, sec */
		/* hour */
		sec=utc_sec%SEC_PER_DAY;
		result->hours=sec/SEC_PER_HOUR;

		/* min */
		sec %= SEC_PER_HOUR;
		result->minutes=sec/SEC_PER_MIN;

		/* sec */
		result->seconds=sec%SEC_PER_MIN;

		/* year, month, day */
		/* year */
		/* year */
		day=utc_sec/SEC_PER_DAY;
		for(y=UTC_BASE_YEAR;day>0;y++) 
		{
				d=(DAY_PER_YEAR+is_leap_year(y));
				if(day>=d)
				{
						day-=d;
				}
				else
				{
						break;
				}
		}
		result->years=y;
		for(m=1;m<MONTH_PER_YEAR;m++) 
		{
				d=get_day_of_mon(m,y);
				if(day>=d) 
				{
						day-=d;
				} 
				else 
				{
						break;
				}
		}
		result->months=m;
		result->days=(uint8_t)(day+1);
		result->weeks=get_weekday(result->years,result->months,result->days);
}

/******************************************************************************/
/*
//自定义时间转UTC时间戳
input:   	
         currTime--自定义时间
				 daylightSaving---是否夏令时
output   none
return   UTC时间戳
*/
/******************************************************************************/
uint32_t mytime_to_utc(t_ctimes *currTime,uint8_t daylightSaving)
{
		uint16_t i;
		uint32_t no_of_days = 0;
		uint32_t utc_time;

		if(currTime->years<UTC_BASE_YEAR) 
		{
				return 0;
		}
		/* year */
		for(i=UTC_BASE_YEAR;i<currTime->years;i++) 
		{
				no_of_days+=(DAY_PER_YEAR+is_leap_year(i));
		}
		/* month */
		for(i=1;i<currTime->months;i++) 
		{
				no_of_days+=get_day_of_mon((uint8_t)i,currTime->years);
		}
		/* day */
		no_of_days+=(currTime->days-1);
		/* sec */
		utc_time=(no_of_days*SEC_PER_DAY)+((uint32_t)currTime->hours*SEC_PER_HOUR)+((uint32_t)currTime->minutes*SEC_PER_MIN)+currTime->seconds;
		if(daylightSaving) 
		{
				utc_time-=SEC_PER_HOUR;
		}
		return utc_time;
}

/******************************************************************************/
/*
//自定义时间转RTC时间
input:   init_stamp_time--自定义时间
output   rtc_times rtc时间
return   none
*/
/******************************************************************************/
void mytime_to_rtc(t_ctimes init_stamp_time, tm *rtc_times)
{
		uint8_t nonleap_years;
		uint8_t leap_years;
		uint8_t full_years;

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

/******************************************************************************/
/*
//RTC时间转自定义时间
intput   rtc_times---芯片rtc时间
output:  current_stamp_time--自定义时间
return   none
*/
/******************************************************************************/
void rtctime_to_mytime(t_ctimes *current_stamp_time,tm rtc_times)
{
		uint16_t days;
		uint8_t leap_year_num;
		uint16_t i;
		
		current_stamp_time->seconds = rtc_times.second;
		current_stamp_time->minutes = rtc_times.minute;
		current_stamp_time->hours = rtc_times.hour;

		current_stamp_time->weeks = rtc_times.day%7;    //week_table[rtc_times.day%7];
    
    leap_year_num = (rtc_times.day)/1461;         //1461天=4年，4年一润，闰年数
	
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


