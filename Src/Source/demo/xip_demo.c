#include <string.h>
#include "type.h"
#include "debug.h"
#include "ssi_demo.h"
#include "xip_demo.h"
#include "xip_drv.h"


//Just remap SSI xip space for data access, NO code execution for this version!
void SSI_XIP_Demo(SSI_TypeDef *SSIx)
{
	unsigned char buf[512];
    unsigned char buf2[512]={0};
	unsigned short ID;
	int j;
	
	if(SSIx!=SSI1)
	{
		printf("only SSI1 supports XIP remap!\r\n");
	  return;
	}	
	
	Read_ID_Test(SSIx);
    
	printf("MID: 0x%02x, CID: 0x%02x\r\n", ID&0XFF,ID>>8);
	
	printf("\r\nbefore erase:\r\n");
	SSI_EFlash_Read(SSIx, 0x00, buf2, 256);	
	for(j=0; j<256; j++)
	{
	   printf("%02x ", buf2[j]);
	}
	
	SSI_EFlash_Sector_Erase(SSIx, 0x00);

	printf("\r\nafter erase:\r\n");
	SSI_EFlash_Read(SSIx, 0x00, buf2, 256);	
	for(j=0; j<256; j++)
	{
	   printf("%02x ", buf2[j]);
	}	
	
	for(j=255; j>=0; j--)
	{
	   buf[255-j]=j;
	}
	
	printf("\r\nprogramming ...\r\n");
	SSI_EFlash_Program(SSIx, 0x00, buf, 250);
	
    printf("\r\nafter program:\r\n");
	SSI_EFlash_Read(SSIx, 0x00, buf2, 256);	
	for(j=0; j<256; j++)
	{
	   printf("%02x ", buf2[j]);
	}
	
	if(memcmp(buf, buf2, 250)!=0x00)
	{
		printf("\r\nSTD test: failed!\r\n\r\n");
	}	
	else
	{
	  printf("\r\nSTD test: passed!\r\n\r\n");
	}	
	
#ifdef XIP_REMAP_DEBUG
	printf("\r\n!!!Hard fault is coming ...\r\n");
	for(j=0; j<256; j++)
	{
	   printf("%02x ", *(unsigned char *)(XIP_REMAP_ADDR+j));
	}
	printf("Never got here!\r\n");
#endif
	
	printf("\r\nSSI XIP init ...\r\n");
	SSI_XIP_Switch(SSIx);
	
//	SSI_XIP_Data_Cache_Enable();
//	SSI_XIP_Instruction_Cache_Enable();//SPI FLASH START CONFIG
	
	printf("\r\ndump XIP remap space ...\r\n");
	for(j=0; j<256; j++)
	{
	   printf("%02x ", *(unsigned char *)(XIP_REMAP_ADDR+j));
	}
	
	if(memcmp(buf, (void *)XIP_REMAP_ADDR, 250)!=0x00)
	{
		printf("\r\nXIP remap test: failed!\r\n\r\n");
	}	
	else
	{
	  printf("\r\nXIP remap test: passed!\r\n\r\n");
		printf("\r\n!!!!!For rerunning this test, please power off the board first!\r\n");
	}		
}	
 
 
 
void XIP_Demo(void)
{
    SSI_XIP_Demo(SSI1);
}	
