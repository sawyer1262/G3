// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : dac_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <string.h>
#include "Iomacros.h"
#include "memmap.h"
#include "ccm4202s.h"
#include "dac_drv.h"
#include "dac_reg.h"
#include "dmac_reg.h"
#include "dmac_drv.h"
#include "cpm_drv.h"
#include "pit32_drv.h"
#include "dac_demo.h"
#include "debug.h"
#include "sys.h"

//#define DAC_CPU_MODE
#define DAC_DMA_MODE		//栈及全局变量放在0x20000000之后，即IRAM：Start0x20000000,size0x38000
#ifdef DAC_DMA_MODE
	DAC_TRIGGER_SEL dac_trigger_sel = TRIGGER_SOFTWARE;
//	DAC_TRIGGER_SEL dac_trigger_sel = TRIGGER_EXTERNAL;
//	DAC_TRIGGER_SEL dac_trigger_sel = TRIGGER_PIT;
#endif

volatile UINT32 DMA_tran_complete = 1;

DMA_LLI g_dma_lli[100];

extern unsigned char *data_buf;
extern unsigned int data_len;
extern unsigned int data_s;
extern unsigned char data_en;

extern unsigned int g_sys_clk;

void make_buf(unsigned char o,unsigned char e,unsigned char *buf,unsigned int l)
{
	float z;
	unsigned int i;
	if(o>e)
	{
		z=((float)(o-e))/l;
		for(i=0;i<l;i++)
		{
			buf[i]=o-z*i;
		}
	}
	else if(o==e)
	{
		memset(buf,o,l);
	}
	else
	{
		z=((float)(e-o))/l;
		for(i=0;i<l;i++)
		{
			buf[i]=o+z*i;
		}	
	}
}

void dac_tran_cpu(unsigned char *buf,unsigned int l,unsigned int rate,unsigned char mul)
{
//	unsigned int g_fp_delayclk = g_sys_clk/(1000*1000);
//	unsigned int us;
//	us=1000000/rate;
	data_s=0;
	data_buf=buf;
	data_len=l;
	data_en=1;
	
	DAC_Init(RIGHTALIGNED_8BITS,TRIGGER_SOFTWARE,DET_ON_LOW);
	//SysTick_Config(g_fp_delayclk*us);
	while(1)         //可以做其他事  
	{
//		while(data_s<data_len){}  
//		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  
//		data_en=0;
			Send_DAC_data(data_buf[data_s++]);	
			if(data_s == 256)
			{
				data_s = 0;
			}
	}		
}


void DAC_DMA_ISR(void)
{
	DMA_tran_complete=0;
	DMA_dis(0);
	DAC_close();
	PIT32_Stop(PIT2);
}


void dac_tran_dma(unsigned char *buf,unsigned int l,unsigned int freq,unsigned char mul)
{
	UINT32 data_addr,data_len;
//	UINT32 data_point,tran_l;
//	UINT32 i,lli_num;
	
	data_addr=(UINT32)buf;
	data_len=l;
	DMA_tran_complete=1;
	
	DAC_Init(RIGHTALIGNED_8BITS, dac_trigger_sel, DET_ON_RISING);
	DMA_REG_Init(DMA2_BASE_ADDR);
	NVIC_Init(3, 3, DMA2_IRQn, 2);

	DMA_DAC_Tran(0,data_addr,data_len);
	DAC_Start(dac_trigger_sel,freq);
	
	if(dac_trigger_sel == TRIGGER_SOFTWARE)
	{
		
		while( (DAC->DAC_FSR&(3<<24))!=0x00 )
		{
			DAC_SW_Trig();
			DAC_Wait_Load_Done();
//			printf("DAC_DOR: 0x%08x\r\n", DAC->DAC_DOR);	
//			printf("DAC_FSR-1: 0x%08x\r\n", DAC->DAC_FSR);
			if( (DAC->DAC_FSR&(1<<29))==0x00 )
			{	
				DAC_Wait_Load_Cear();
			}	
			if(DMA_tran_complete == 0)
			{
				break;
			}
//			printf("DAC_FSR-1: 0x%08x\r\n\r\n", DAC->DAC_FSR);	
		}
	}
	else if(dac_trigger_sel == TRIGGER_PIT)
	{
		if(mul)
		{
			while(DMA_tran_complete);
		}
	}
	else
	{}

	

	
	
//	if(data_len>0xfff)
//	{
//		tran_l=0xfff;
//		data_point=0;
//		lli_num=data_len/0xfff;
//		if(data_len%0xfff)
//		{
//			lli_num++;
//		}
//		for(i=0;i<lli_num;i++)
//		{
//			g_dma_lli[i].src_addr=data_addr+data_point;
//			g_dma_lli[i].dst_addr=0x40021004;
//			if(i<(lli_num-1))
//			{
//				g_dma_lli[i].next_lli=(UINT32)&g_dma_lli[i+1];
//			}
//			else
//			{
//				g_dma_lli[i].next_lli=0;
//			}
//			g_dma_lli[i].control0= SIEC|DNOCHG|M2P_DMA|DWIDTH_B|SWIDTH_B|INTEN|LLP_SRC_EN;
//			g_dma_lli[i].len=tran_l;
//			data_point+=tran_l;
//			data_len-=tran_l;
//			if(data_len>0xfff)
//			{
//				tran_l=0xfff;
//			}
//			else
//			{
//				tran_l=data_len;
//			}
//		}
//		dma_lli_reg_init(0,&g_dma_lli[0]);
//	}
//	else
//	{	
//		tran_l=data_len;
//		data_point=0;
//		data_len-=tran_l;
//		DMA_DAC_Tran(0,data_addr+data_point,tran_l);
//	}
//	DAC_Start(dac_trigger_sel,freq);
//	if(mul)
//	{
//		while(DMA_tran_complete);
//	}
}

void DAC_Demo(void)
{
	UINT8 data_buf[256];
	UINT32 i;
	for(i = 0; i < 256; i ++)
	{
		data_buf[i] = i;
	}

#ifdef DAC_CPU_MODE
	printf("DAC CPU mode test\r\n");
	while(1)
	{
		dac_tran_cpu(data_buf, 256,44100,1);   //CPU方式  
	}	
#endif
		
#ifdef DAC_DMA_MODE 
	printf("DAC DMA mode test\r\n");
	while(1)
	{
		dac_tran_dma(data_buf,256,44100,1);   //DMA方式 
	}
#endif

	
}

