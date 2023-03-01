// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : eflash_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "libEFlash.h"
#include "eflash_drv.h"
#include "eflash_demo.h"
#include "type.h"
#include "debug.h"
#include "cpm_drv.h"

#define DATA_FIELDS_BEGIN_ADDR   0x08010000
#define DTAT_FIELDS_PAGE_NUM     0x100	

void EFLASH_Words_Read(UINT32 addr, UINT32 *des, UINT32 len);	

void EFlash_Demo(void)
{
	UINT32 page = 0;
	UINT32 eflash_clk;
	UINT32 success_counts_page_erase = 0;
	UINT32 success_counts_program = 0;
	UINT32 success_counts_compare = 0;
	UINT32 failure_counts_page_erase = 0;
	UINT32 failure_counts_program = 0;
	UINT32 failure_counts_compare = 0;
	UINT32 page_addr = DATA_FIELDS_BEGIN_ADDR;
	UINT32 write_buf[EFLASH_PAGE_SIZE_WORD];
	UINT32 read_buf[EFLASH_PAGE_SIZE_WORD];
	UINT8 st;
	UINT8 i;
	
	printf ("*******************Eflash Test start*******************\r\n");
	
	eflash_clk = g_sys_clk;
	EFLASH_Init(eflash_clk/1000);
//	EFLASH_SetWritePermission();
	
	for (page = 0; page < DTAT_FIELDS_PAGE_NUM; page++, page_addr +=  0x200)
	{
		st = EFLASH_PageErase(page_addr);
		if (st == FALSE)
		{
			failure_counts_page_erase ++;
			printf("EFLASH_PageErase [page_addr = 0x%08x] Failure \r\n", page_addr);
			continue;
		}
		success_counts_page_erase ++;
		//printf("EFLASH_PageErase [page_addr = 0x%08x] success \r\n", page_addr);
		//printf("write data: \r\n");
		for (i = 0; i < EFLASH_PAGE_SIZE_WORD; i++)
		{
			write_buf[i] = (page<<8)+i;;
			//printf("0x%08x ", write_buf[i]);
		}
		//printf("\r\n");
		st = EFLASH_WordsProg(page_addr,write_buf,EFLASH_PAGE_SIZE_WORD);
		if (st == FALSE)
		{
			failure_counts_program ++;
			printf("EFLASH_WordsProg [page_addr = 0x%08x] Failure \r\n", page_addr);
			//continue;
		}
		else
			success_counts_program ++;
			//printf("EFLASH_WordsProg [page_addr = 0x%08x] success \r\n", page_addr);
		
		EFLASH_Words_Read(page_addr, read_buf, EFLASH_PAGE_SIZE_WORD);

		st = 0;
		//printf("read data: \r\n");
		for (i = 0; i < EFLASH_PAGE_SIZE_WORD; i++)
		{
			//printf("0x%08x ", read_buf[i]);
			if (write_buf[i] != read_buf[i])
			{
				st = 1;
			}
		}
		//printf("\r\n");
		if (!st)
		{
			success_counts_compare ++;
			//printf("***********Test EFLASH Success****************\r\n");
		}
		else
		{
			failure_counts_compare ++;
			printf("Compare [page_addr = 0x%08x] Failure \r\n", page_addr);
			//printf("***********Test EFLASH Failure****************\r\n");
		}
		EFLASH_PageErase(page_addr);
	}
	
    printf("Test page address        :0x%08x\r\n", DATA_FIELDS_BEGIN_ADDR);
	printf("Test page counts         :0x%08x\r\n", DTAT_FIELDS_PAGE_NUM);
	printf("Page erase failure counts:0x%08x\r\n", failure_counts_page_erase);
	printf("Program failure counts   :0x%08x\r\n", failure_counts_program);
	printf("Compare failure counts   :0x%08x\r\n", failure_counts_compare);
	printf("Page erase success counts:0x%08x\r\n", success_counts_page_erase);
	printf("Program success counts   :0x%08x\r\n", success_counts_program);
	printf("Compare success counts   :0x%08x\r\n", success_counts_compare);
	printf("*******************Eflash Test End*******************\r\n");
}

/*******************************************************************************
* Function Name  : EFLASH_WordsRead
* Description    :  从EFLASH目标地址读取长度为len的word数据
* Input          : - addr: EFLASH destination address
*                  - len:  需要读取word数据的长度
*
* Output         : - des:  读取的word数据缓冲
* Return         : None
******************************************************************************/
void EFLASH_Words_Read(UINT32 addr, UINT32 *des, UINT32 len)
{
	UINT32 i;

	for(i = 0; i < len; i ++)
	{
		des[i] = (*(volatile UINT32 *)(addr+i*4));
	}
}

