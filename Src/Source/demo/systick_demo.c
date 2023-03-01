
#include "sys.h"
#include "delay.h"
#include "debug.h"
#include "systick_drv.h"
#include "systick_demo.h"


void SysTick_Demo(void)
{
	//≥ı ºªØ
	g_systickCounter = 0;
	SysTick_Config(g_sys_clk/1000);  //1ms
	
	SysTick_Enable(TRUE);
	while(1)
	{
		if((g_systickCounter%1000) == 0)
		{
			printf("Systick Counter = %d\r\n",g_systickCounter/1000);
		}	
	}
	
	
	
}
