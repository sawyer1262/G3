/*
 * time_stamp_api.h
 *
 *  Created on: 2019Äê5ÔÂ17ÈÕ
 *      Author: fjzhang
 */
#ifndef __TIME_STAMP_API_H__
#define __TIME_STAMP_API_H__

#include "rtc_drv.h"
#include "cpm_reg.h"
#include "debug.h"
#include "delay.h"

#define START_YEARS			1973
#define END_YEARS				2053   //END_YEARS - START_YEARS <= 80

typedef struct _t_ctimes{
	// date and time components
	unsigned short years;
	unsigned char  months;
	unsigned char  days;
	unsigned char  weeks;
	unsigned char  hours;
	unsigned char  minutes;
	unsigned char  seconds;
}t_ctimes;

//typedef struct _t_rtc_times{
//	// RTC date and time components
//	unsigned short days;
//	unsigned char  hours;
//	unsigned char  minutes;
//	unsigned char  seconds;
//}t_rtc_times;

extern void RTC_TimeStamp_Demo(void);

#endif /* __TIME_STAMP_API_H__ */


