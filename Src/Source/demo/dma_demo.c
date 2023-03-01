// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : dma_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "memmap.h"
#include "dma_demo.h"
#include "dmac_drv.h"
#include "type.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"

void DMA_Demo(void)
{
	UINT32 i;
	BOOL ret;
	UINT8 dma_src_data[0x100];//source bufer addr
	UINT8 dma_dst_data[0x100];//destination bufer addr
	
	printf("damc memory to memory test\r\n");

	for(i = 0;i<0x100;i++)
	{
		dma_src_data[i] = i%0xFF;
	}
	
	DMA_Init(DMA1_BASE_ADDR);

	for(i = DMACCH0;i <= DMACCH3;i++)
	{
		memset(dma_dst_data,0x00,0x100);

		ret = dma_m2mtran(i,WIDTH_HALFWORD,(UINT32)dma_src_data,(UINT32)dma_dst_data,0x100);
		if(ret == FALSE)
		{
			printf("dmac channel %d tran fail\r\n",i);
		}
		if(memcmp(dma_src_data,dma_dst_data,0x100) != 0)
		{
			printf("dmac channel %d test fail\r\n",i);
		}
		else
		{
			printf("dmac channel %d test success\r\n",i);
		}
	}
	printf("dmac test end\r\n");
}

