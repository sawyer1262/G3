/*
 * apdu_demo.c
 *
 *  Created on: 2018年8月13日
 *      Author: YangWenfeng
 */
#include <stdio.h>
#include "apdu_demo.h"
#include "apdu_dispatcher.h"
#include "apdu_drv.h"
#include "sys.h"
#include "common.h"
#include "cpm_drv.h"
#include "delay.h"

#define USI_MODE_TEST		1

void USI_GPIO_Test(void)
{
#if 0
	USI_ConfigGpio(USI, USI_RST, GPIO_OUTPUT);
	USI_ConfigGpio(USI, USI_CLK, GPIO_OUTPUT);
	USI_ConfigGpio(USI, USI_DAT, GPIO_OUTPUT);

	while(1)
	{
		USI_WriteGpioData(USI, USI_RST, Bit_SET);
		USI_WriteGpioData(USI, USI_CLK, Bit_SET);
		USI_WriteGpioData(USI, USI_DAT, Bit_SET);		
		DelayMS(1);
		USI_WriteGpioData(USI, USI_RST, Bit_RESET);
		USI_WriteGpioData(USI, USI_CLK, Bit_RESET);
		USI_WriteGpioData(USI, USI_DAT, Bit_RESET);
		DelayMS(1);
	}
	
#else
	USI_ConfigGpio(USI, USI_RST, GPIO_INPUT);
	USI_ConfigGpio(USI, USI_CLK, GPIO_INPUT);
	USI_ConfigGpio(USI, USI_DAT, GPIO_INPUT);
	
	while(1)
	{
		printf("USI_RST: %02x\r\n",USI_ReadGpioData(USI, USI_RST));
		printf("USI_CLK: %02x\r\n",USI_ReadGpioData(USI, USI_CLK));
		printf("USI_DAT: %02x\r\n",USI_ReadGpioData(USI, USI_DAT));
		printf("****************************\r\n");
		DelayMS(1000);
	}
#endif
}


void APDU_Demo(void)
{
	
#if USI_MODE_TEST
	
	Disable_Interrupts;
	if(USI_BASE_ADDR == USI2_BASE_ADDR)
	{
	//如果启动USI2作为从，则需要打开USI2的VCC，如果为USI1则不需要
		usiDev_init();
	}

	InitAPDU(USI_BASE_ADDR);

	NVIC_Init(3, 3, USI_IRQn, 2);
	
	Enable_Interrupts;

	while(1)
	{
		usi_main();
	}
	
#else	//USI GPIO test

	if(USI_BASE_ADDR == USI2_BASE_ADDR)
	{
		usiDev_init();
	}

	USI_GPIO_Test();
#endif

}
