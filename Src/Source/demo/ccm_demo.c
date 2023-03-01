
#include "debug.h"
#include "ccm_demo.h"
#include "ccm_drv.h"
#include "delay.h"

void CCM_Demo(void)
{
	UINT16 cid;
	
	read_chip_cid(&cid);
	
	printf("chip CID = %04x\r\n",cid);
	
	printf("config GPIO OUT\r\n");
	
	CCM_ConfigGpio(CLOCK_OUT,GPIO_OUTPUT);
	CCM_ConfigGpio(RESET_OUT,GPIO_OUTPUT);
	while(1)
	{
		CCM_WriteGpioData(CLOCK_OUT,Bit_SET);
		CCM_WriteGpioData(RESET_OUT,Bit_SET);
		delay(500000);
		CCM_WriteGpioData(CLOCK_OUT,Bit_RESET);
		CCM_WriteGpioData(RESET_OUT,Bit_RESET);
		delay(500000);
	}
}
