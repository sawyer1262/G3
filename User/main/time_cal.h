#ifndef  __TIME_CAL_H_
#define  __TIME_CAL_H_


#include <stdint.h>
#include "rtc_drv.h"


#define UTC_BASE_YEAR        1970
#define MONTH_PER_YEAR       12
#define DAY_PER_YEAR         365
#define SEC_PER_DAY          86400
#define SEC_PER_HOUR         3600
#define SEC_PER_MIN          60
#define START_YEARS	   		   1973
#define END_YEARS				     2053   //END_YEARS - START_YEARS <= 80


typedef struct{
	// date and time components
	uint16_t years;
	uint8_t  months;
	uint8_t  days;
	uint8_t  weeks;
	uint8_t  hours;
	uint8_t  minutes;
	uint8_t  seconds;
}t_ctimes;


void utc_to_mytime(uint32_t utc_sec, t_ctimes *result, uint8_t daylightSaving);
uint32_t mytime_to_utc(t_ctimes *currTime,uint8_t daylightSaving);
void rtctime_to_mytime(t_ctimes *current_stamp_time,tm rtc_times);
void mytime_to_rtc(t_ctimes init_stamp_time, tm *rtc_times);
uint8_t get_weekday(uint16_t year, uint8_t month, uint8_t day);


#endif


