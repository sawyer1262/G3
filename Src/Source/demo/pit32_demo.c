
#include "debug.h"
#include "pit32_drv.h"
#include "pit32_demo.h"
#include "delay.h"

void PIT32_Demo(void)
{
    PIT32_Init(PIT1, PIT32_CLK_DIV_64, 0xfffff,TRUE);
    PIT32_Init(PIT2, PIT32_CLK_DIV_16, 0xfffff,TRUE);
	
	while(1)
	{
		DelayMS(100);
		printf("pit1 count: 0x%05x\r\n", PIT32_ReadCNTR(PIT1));
        printf("pit2 count: 0x%05x\r\n", PIT32_ReadCNTR(PIT2));
	}
}

