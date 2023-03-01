
#include "sys.h"

#include "cpm_drv.h"
#include "delay.h"

/*******************************************************************************
* Function Name  : DelayMS
* Description    : Delay n MS
* Input          : delaymS£ºDelay Parame
* Output         : None
* Return         : None
*******************************************************************************/
void DelayMS(vu32 delaymS)
{
//	UINT32 counter=0;
	UINT32 k = 0;
	vu32 counter_ms = g_sys_clk/6000;
	
	while(delaymS --)
	{
		while(k < counter_ms)k ++;
		k = 0;
	}
}

/*******************************************************************************
* Function Name  : delay
* Description    : Delay n nop
* Input          : time: n npo
* Output         : None
* Return         : None
*******************************************************************************/
void delay(vu32 time)
{
	while(time--);
}

