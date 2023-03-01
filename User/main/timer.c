#include "pit32_drv.h"
#include "sys.h"
#include "timer.h"
#include "uart.h"
#include "keypad_menu.h"
#include "debug.h"
#include "motor.h"
#include "audio.h"


uint32_t timecount=0;
uint16_t time20mscount=0;
uint16_t time100mscount=0;
uint16_t time1scount=0;


volatile uint32_t delaycount=0;

uint8_t b20msFlag=0;
uint8_t b100msFlag=0;
uint8_t b1sFlag=1;

/******************************************************************************/
/*
//定时器初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void Timer_Init(void)
{
		PIT32_Init(PIT1, PIT32_CLK_DIV_2, (g_ips_clk/PIT32_CLK_DIV_2/1000),TRUE);   
		timecount=0;
}
/******************************************************************************/
/*
//定时器停止
input:   none
output   none
return   none
*/
/******************************************************************************/
void Timer_DeInit(void)
{
		PIT32_Stop(PIT1);
}
/******************************************************************************/
/*
//获取当前定时器计数值
input:   none
output   none
return   timecount -- 定时器当前计数值
*/
/******************************************************************************/
uint32_t GetTimerCount(void)
{
		return timecount;
}
/******************************************************************************/
/*
//获取当前时间到起始时间的差值
input:   start -- 起始时间
output   none
return   elapseTime -- 计数差值
*/
/******************************************************************************/
uint32_t GetTimerElapse(uint32_t start)
{
		uint32_t elapseTime=0;
	
		if(start<=timecount)
		{
				elapseTime=timecount-start;
		}
		else
		{
				elapseTime=0xffffffff-start+timecount;
		}
		return elapseTime;
}
/******************************************************************************/
/*
//阻塞延时
input:   ms -- 延时时间：毫秒为单位
output   none
return   none
*/
/******************************************************************************/
void BlockDelayMS(uint32_t ms)
{	
		delaycount=ms;
		while(delaycount);
}
/******************************************************************************/
/*
//定时器中断回调函数
input:   ms -- 延时时间：毫秒为单位
output   none
return   none
*/
/******************************************************************************/
void Timer_IntCallback(void)
{
		timecount++;
		Motor_Running();
		Audio_PowerOnCntInc();
		if(delaycount>0)delaycount--; 
		if(++time20mscount>=20)
		{
				time20mscount=0;
				b20msFlag=1;
		}
		if(++time100mscount>=100)
		{
				time100mscount=0;
				b100msFlag=1;
		}
		if(++time1scount>=1000)
		{
				time1scount=0;
				b1sFlag=1;
		}
}
























