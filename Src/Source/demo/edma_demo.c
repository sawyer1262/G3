#include <string.h>
#include "edma_drv.h"
#include "trng_drv.h"
#include "type.h"
#include "debug.h"
#include "edma_demo.h"

static void Init_Buf(UINT8 *u8buf, UINT32 buflen, UINT8 inittype, UINT8 initval)
{
	unsigned int i;

	if(inittype == 0)
	{
		memset(u8buf,initval,buflen);
	}
	else if(inittype == 1)
	{
		for(i=0;i<buflen;i++)
		{
			u8buf[i] = (UINT8)GetRandomWord();
		}
	}
	else
	{
		for(i=0;i<buflen;i++)
		{
			u8buf[i] = i;
		}
	}
}

void EDMA_Demo(void)
{
	UINT32 i = 0;
	UINT8 edma_src_data[0x100];//source bufer addr
	UINT8 edma_dst_data[0x100];//destination bufer addr

	Init_Trng();
	
	printf("EDMA0 CH0 TEST START\r\n");
	Init_Buf(edma_src_data,0x100,2,0xff);
	memset(edma_dst_data,0xff,0x100);

	EDMA_CHx(EDMA_CH0, (UINT32)edma_src_data, (UINT32)edma_dst_data, EDMAC_TTYPE0, EDMA_SRAM, 0x100);

	printf("edma_src_data\r\n");
	
	for (i = 0; i < 0x100; i++)
		printf("0x%02x ", edma_src_data[i]);
	printf("\r\n");
	
	printf("edma_dst_data\r\n");
	
	for (i = 0; i < 0x100; i++)
		printf("0x%02x ", edma_dst_data[i]);
	printf("\r\n");
	
	if(memcmp(edma_src_data,edma_dst_data,0x100) != 0)
	{
		printf("EDMA0 CH0 TEST FAILE\r\n");
	}

	printf("EDMA0 CH0 TEST END\r\n");

	printf("EDMA0 CH1 TEST START\r\n");
	Init_Buf(edma_src_data,0x100,2,0xff);
	memset(edma_dst_data,0x00,0x100);

	EDMA_CHx(EDMA_CH1, (UINT32)edma_src_data, (UINT32)edma_dst_data, EDMAC_TTYPE0, EDMA_SRAM, 0x100);

	printf("edma_src_data\r\n");
	for (i = 0; i < 0x100; i++)
		printf("0x%02x ", edma_src_data[i]);
	printf("\r\n");
	printf("edma_dst_data\r\n");
		for (i = 0; i < 0x100; i++)
			printf("0x%02x ", edma_dst_data[i]);
	printf("\r\n");
	
		if(memcmp(edma_src_data,edma_dst_data,0x100) != 0)
	{
		printf("EDMA0 CH1 TEST FAILE\r\n");
	}

	printf("EDMA0 CH1 TEST END\r\n");
}
