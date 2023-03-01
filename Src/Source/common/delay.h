
#ifndef __DELAY_H__
#define __DELAY_H__

#include "type.h"

/*******************************************************************************
* Function Name  : delay
* Description    : Delay n nop
* Input          : time: n npo
* Output         : None
* Return         : None
*******************************************************************************/
extern void delay(vu32 time);

/*******************************************************************************
* Function Name  : DelayMS
* Description    : Delay n MS
* Input          : delaymS£ºDelay Parame
* Output         : None
* Return         : None
*******************************************************************************/
extern void DelayMS(vu32 delaymS);
#endif

