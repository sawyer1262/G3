#include "type.h"
#include "delay.h"
#include "debug.h"
#include "ssi_drv.h"
#include "ssi_demo.h"
#include "dmac_drv.h"
#include "trng_drv.h"
#include "ioctrl_drv.h"
#include <string.h>

#define SSI_PAGE_SIZE		0x100
#define SSI_SECTOR_SIZE	0x1000
#define SSI_DATA_LEN		SSI_PAGE_SIZE

volatile unsigned int sector_addr = 0x000000;

unsigned char __attribute__((aligned (4))) src_data[300] ={0} ;//source bufer addr
unsigned char __attribute__((aligned (4))) dst_data[300] ={0};//destination bufer addr

/*
 * init data buf
 * input:
 * 1.DMA_DMACCHCTRL_REG *dmacctrl
 * 2.unsigned int buflen
 * 3.unsigned char inittype
 * 4.unsigned char initval
 * output:
 *
 */
void Init_Buf(unsigned char *u8buf,unsigned int buflen,unsigned char inittype,unsigned char initval)
{
	unsigned int i;

	if(inittype == 0)
	{
		memset(u8buf,initval,buflen);
	}
	else if(inittype == 1)
	{
		Init_Trng();
		for(i=0;i<buflen;i++)
		{
			u8buf[i] = (UINT8)GetRandomWord();
		}
		Trng_Disable();
	}
	else
	{
		for(i=0;i<buflen;i++)
		{
			u8buf[i] = i;
		}
	}
}


/*
 * Note: SSI(QUAD spi eflash demo for Winbond w25q128)
 */
 
unsigned char buf[512];
unsigned char buf2[512]={0};

/*******************************************************************************
* Function Name  : Read_ID_Test
* Description    : 读取SPIFLASH ID 
* Input          :- SSIx   : SSI 基地址							 
* Output         : None
* Return         : None
******************************************************************************/
void Read_ID_Test(SSI_TypeDef *SSIx)
{
    unsigned char ID[2];
    unsigned short ret;

    ret = SSI_Standard_Init(SSIx);
	if(ret)
	{
		printf("ssi init failed,ret:0x%04x\r\n",ret);
        while(1);
	}

    ret = Read_ID(SSIx,ID);
    if(STATUS_OK != ret)
    {
        printf("read id failed,ret:0x%04x\r\n",ret);
        while(1);
    }
    printf("MID: 0x%02x, CID: 0x%02x\r\n", ID[0],ID[1]);
}

void SSI_STD_DMA_Demo(SSI_TypeDef *SSIx)
{
	int i,j,ch=0;
	UINT16 ret = STATUS_OK;

    printf("***********************SSI STD DMA test***********************\r\n"); 

	if(SSIx==SSI1)	
	{
	  DMA_REG_Init(DMA2_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA2_IRQn, 2);
#endif
	}

	if(SSIx==SSI2)	
	{
		DMA_REG_Init(DMA1_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA1_IRQn, 2);
#endif
	}	

	ret = SSI_EFlash_Sector_Erase(SSIx, sector_addr);   
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
    {
        Init_Buf(dst_data,SSI_DATA_LEN,0,0x00); 
        ret = SSI_STD_DMA_Read(SSIx, ch, dst_data, sector_addr+i, SSI_DATA_LEN,1);       
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
        for(j = 0; j < SSI_DATA_LEN; j ++)
        {    
            if(dst_data[j+4] != 0xFF)
            {
                printf("Erase data compare failed!addr:%08x\r\n",(sector_addr+i+j));
                // __ASM volatile("bkpt 0x12");
            }
        }
    }
	  
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{		
		Init_Buf(src_data,SSI_DATA_LEN,1,0);
		ret = SSI_EFlash_Write_Enable(SSIx);	
        if(ret)
        {
            printf("eflash write enable failed.\r\n");
        }
//		printf("\r\nstd DMA write ...\r\n");	 	 
		ret = SSI_STD_DMA_Send(SSIx, ch, src_data, sector_addr+i, SSI_DATA_LEN,1);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
//		SSI_Standard_Init(SSIx);   
		ret = SSI_Wait_Eflash_Idle(SSIx);//wait ssi finish
        if(ret)
        {
            printf("SSI_Wait_Eflash_Idle failed.\r\n");
        }
		
		//DMA read  
		Init_Buf(dst_data,SSI_DATA_LEN,0,0x00); 	 
		ret = SSI_STD_DMA_Read(SSIx, ch, dst_data, sector_addr+i, SSI_DATA_LEN,1);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
//		printf("STD DMA read ...\r\n");
//		for(j=0;j<SSI_DATA_LEN;j++)
//		{
//			printf("%02x ", *(dst_data+j+4));
//		}	 

		if(memcmp(src_data, dst_data+4, SSI_DATA_LEN)!=0x00)
		{
			printf("STD DMA data compare failed!addr:%08x\r\n",(sector_addr+i));
		 // __ASM volatile("bkpt 0x12");
		} 
		else
		{
			printf("STD DMA passed!addr:%08x\r\n",(sector_addr+i));
		} 	 	 
	}
	printf("***********************SSI STD DMA test finish***********************\r\n");
}	

void SSI_STD_Demo(SSI_TypeDef *SSIx)
{
	unsigned int i,j;
    UINT16 ret = STATUS_OK;
	
	Read_ID_Test(SSIx);

    printf("***********************SSI STD test***********************\r\n");

	ret = SSI_EFlash_Sector_Erase(SSIx, sector_addr);
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }
	
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
        Init_Buf(dst_data,SSI_DATA_LEN,0,0x00);
        ret = SSI_EFlash_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
		for(j = 0; j < SSI_DATA_LEN; j ++)
		{ 
			if(dst_data[j] != 0xFF)
			{
				printf("Erase data compare failed!addr:%08x\r\n",(sector_addr+i+j));
				// __ASM volatile("bkpt 0x12");
			}
		}
	}
	

	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
		
		Init_Buf(src_data,SSI_DATA_LEN,1,0);	

//		printf("\r\nSSI STD write...\r\n");
		ret = SSI_EFlash_Program(SSIx, sector_addr+i, src_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("program failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
        }

		Init_Buf(dst_data,SSI_DATA_LEN,0,0x00);
//		printf("SSI STD read...\r\n");
		ret = SSI_EFlash_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }  

		if(memcmp(dst_data, src_data, SSI_DATA_LEN)!=0x00)
		{
			printf("STD data compare failed!addr:%08x\r\n",(sector_addr+i));
            // __ASM volatile("bkpt 0x12");
		} 
		else
		{
			printf("STD passed!addr:%08x\r\n",(sector_addr+i));
		}
		
	}
		
	printf("***********************SSI STD test finish***********************\r\n");
	
}
	
void SSI_DUAL_DMA_Demo(SSI_TypeDef *SSIx)
{
	int i,j,ch=0;
    UINT16 ret = STATUS_OK;

    printf("***********************SSI DUAL DMA test***********************\r\n");

	if(SSIx==SSI1)	
	{
	  DMA_REG_Init(DMA2_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA2_IRQn, 2);
#endif
	}

	if(SSIx==SSI2)	
	{
		DMA_REG_Init(DMA1_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA1_IRQn, 2);
#endif
	}

	ret = SSI_EFlash_Sector_Erase(SSIx, sector_addr);
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
		Init_Buf(dst_data,SSI_DATA_LEN,0,0x00); 
		SSI_DUAL_Init(SSIx,SSI_DATA_LEN,8);
		ret = SSI_DUAL_DMA_Read(SSIx, ch, dst_data, sector_addr+i, SSI_DATA_LEN,1);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
//        printf("DUAL DMA read ...\r\n");
//		for(j=0;j<SSI_DATA_LEN;j++)
//		{
//			printf("%02x ", *(dst_data+j));
//		}			
		for(j = 0; j < SSI_DATA_LEN; j ++)
		{
			if(dst_data[j] != 0xFF)
			{
				printf("Erase data compare failed!addr:%08x\r\n",(sector_addr+i+j));
				// __ASM volatile("bkpt 0x12");
			}
		}
	}
	
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
		Init_Buf(src_data,SSI_DATA_LEN,1,0);
		ret = SSI_EFlash_Write_Enable(SSIx);	
        if(ret)
        {
            printf("SSI_EFlash_Write_Enable failed!\r\n");
        }
//		printf("\r\nstd DMA write ...\r\n");
//        printf("DUAL DMA send ...\r\n");
//		for(j=0;j<SSI_DATA_LEN;j++)
//		{
//			printf("%02x ", *(src_data+j));
//		}        
		ret = SSI_STD_DMA_Send(SSIx, ch, src_data, sector_addr+i, SSI_DATA_LEN,1);
        if(ret)
        {
            printf("send failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
		ret = SSI_Standard_Init(SSIx);   
        if(ret)
        {
            printf("SSI_Standard_Init failed!\r\n");
        }
		ret = SSI_Wait_Eflash_Idle(SSIx);//wait ssi finish
        if(ret)
        {
            printf("SSI_Wait_Eflash_Idle failed!\r\n");
        }
		
		//DMA read  
		Init_Buf(dst_data,SSI_DATA_LEN,0,0x00); 
		SSI_DUAL_Init(SSIx,SSI_DATA_LEN,8);
		ret = SSI_DUAL_DMA_Read(SSIx, ch, dst_data, sector_addr+i, SSI_DATA_LEN,1);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
//        printf("DUAL DMA read ...\r\n");
//		for(j=0;j<SSI_DATA_LEN;j++)
//		{
//			printf("%02x ", dst_data[j]);
//		}
		if(memcmp(src_data, dst_data, SSI_DATA_LEN)!=0x00)
		{
			printf("DUAL DMA data compare failed!addr:%08x\r\n",(sector_addr+i));
		 // __ASM volatile("bkpt 0x12");
		} 
		else
		{
			printf("DUAL DMA passed!addr:%08x\r\n",(sector_addr+i));
		} 	 	 
	}
	printf("***********************SSI DUAL DMA test finish***********************\r\n");
}	

void SSI_DUAL_Demo(SSI_TypeDef *SSIx)
{
	unsigned int i,j;
    UINT16 ret;

    printf("***********************SSI DUAL test***********************\r\n");

	ret = SSI_EFlash_Sector_Erase(SSIx, sector_addr);
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
		Init_Buf(dst_data,SSI_DATA_LEN,0,0x00);
		ret = SSI_EFlash_DUAL_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
		for(j = 0; j < SSI_DATA_LEN; j ++)
		{
			if(dst_data[j] != 0xFF)
			{
				printf("Erase data cpmpare failed!addr:%08x\r\n",(sector_addr+i+j));
				// __ASM volatile("bkpt 0x12");
			}
		}
	}

	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{	
		Init_Buf(src_data,SSI_DATA_LEN,1,0);	

//		printf("\r\nSSI STD write...\r\n");
		ret = SSI_EFlash_Program(SSIx, sector_addr+i, src_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("Program failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }

		Init_Buf(dst_data,SSI_DATA_LEN,0,0x00);
//		printf("SSI STD read...\r\n");
		ret = SSI_EFlash_DUAL_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
        
		if(memcmp(dst_data, src_data, SSI_DATA_LEN)!=0x00)
		{
			printf("DUAL data compare failed!addr:%08x\r\n",(sector_addr+i));
		 // __ASM volatile("bkpt 0x12");
		} 
		else
		{
			printf("DUAL passed!addr:%08x\r\n",(sector_addr+i));
		}
		
	}
		
	printf("***********************SSI DUAL test finish***********************\r\n");
	
}

	
void SSI_QUAD_DMA_Demo(SSI_TypeDef *SSIx)
{
//	unsigned char temp;
	int i,j,ch=0;;
    UINT16 ret;

    printf("******************SSI QUAD DMA test******************\r\n\r\n");
	
	if(SSIx==SSI1)	
	{
	  DMA_REG_Init(DMA2_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA2_IRQn, 2);
#endif
	}

	if(SSIx==SSI2)	
	{
		DMA_REG_Init(DMA1_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA1_IRQn, 2);
#endif
	}

	ret = SSI_EFlash_Sector_Erase(SSIx, sector_addr);
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }

	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
        //quad spi dma read	 
        SSI_EFlash_Prog_Status2(SSIx, 0x02);
		ret= SSI_QUAD_Init(SSIx, 1, SSI_PAGE_SIZE, 8); //flush FIFO
        if(ret)
        {
            printf("SSI_QUAD_Init failed!\r\n");
        }
		Init_Buf(dst_data,SSI_PAGE_SIZE,0,0x00);	 
		ret = SSI_QUAD_DMA_Read(SSIx, ch, dst_data, sector_addr+i, SSI_PAGE_SIZE, 1);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
		for(j = 0; j < SSI_DATA_LEN; j ++)
		{
			if(dst_data[j] != 0xFF)
			{
				printf("Erase data compare failed!addr:%08x\r\n",(sector_addr+i+j));
				// __ASM volatile("bkpt 0x12");
			}
		}

		ret = SSI_EFlash_Write_Enable(SSIx);
        if(ret)
        {
            printf("SSI_EFlash_Write_Enable failed!\r\n");
        }
        ret = SSI_EFlash_Prog_Status2(SSIx, 0x02);    
        if(ret)
        {
            printf("SSI_EFlash_Prog_Status2 failed!\r\n");
        }    
		Init_Buf(src_data,SSI_PAGE_SIZE,1,0x00);	
		ret = SSI_QUAD_Init(SSIx, 0, SSI_PAGE_SIZE, 0); //set to quad
        if(ret)
        {
            printf("SSI_QUAD_Init failed!\r\n");
        }

//	printf("\r\nQUAD SPI DMA write ...\r\n"); 
		ret = SSI_QUAD_DMA_Send(SSIx, ch, src_data, sector_addr+i, SSI_PAGE_SIZE, 1);
        if(ret)
        {
            printf("send failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
		ret = SSI_Standard_Init(SSIx); //wait ssi finish 
        if(ret)
        {
            printf("SSI_Standard_Init failed!\r\n");
        } 
		ret = SSI_Wait_Eflash_Idle(SSIx);	
        if(ret)
        {
            printf("SSI_Wait_Eflash_Idle failed!\r\n");
        } 

		//quad spi dma read
        ret = SSI_EFlash_Prog_Status2(SSIx, 0x02);
        if(ret)
        {
            printf("SSI_EFlash_Prog_Status2 failed!\r\n");
        } 
		ret = SSI_QUAD_Init(SSIx, 1, SSI_PAGE_SIZE, 8); //flush FIFO
        if(ret)
        {
            printf("SSI_EFlash_Prog_Status2 failed!\r\n");
        } 
		Init_Buf(dst_data,SSI_PAGE_SIZE,0,0x00);	 
		ret = SSI_QUAD_DMA_Read(SSIx, ch, dst_data, sector_addr+i, SSI_PAGE_SIZE, 1);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        } 
	 
//		printf("\r\nQUAD SPI DMA read ...\r\n");
//		for(j=0;j<SSI_DATA_LEN;j++)
//		{
//			printf("%02x ", *(dst_data+j));
//		}	

		if(memcmp(src_data, dst_data, SSI_PAGE_SIZE)!=0x00)
		{
			printf("QUAD DMA data compare failed!addr:%08x\r\n",(sector_addr+i));
			// __ASM volatile("bkpt 0x12");
		}	
		else
		{
			printf("QUAD DMA passed!addr:%08x\r\n",(sector_addr+i));
		}	
	}
	printf("******************SSI QUAD DMA test finish******************\r\n\r\n");
}	


void SSI_QUAD_Demo(SSI_TypeDef *SSIx)
{
    UINT16 ret;
	int i,j;
//	unsigned char temp;

    printf("******************SSI QUAD test******************\r\n\r\n");

    ret = SSI_EFlash_Sector_Erase(SSIx, sector_addr);
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }
    
    for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
    {
        Init_Buf(dst_data,SSI_DATA_LEN,0,0x00);

        ret = SSI_EFlash_QUAD_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
		for(j = 0; j < SSI_DATA_LEN; j ++)
		{
			if(dst_data[j] != 0xFF)
			{
				printf("Erase data compeare failed!addr:%08x\r\n",(sector_addr+i+j));
				// __ASM volatile("bkpt 0x12");
			}
		}
    }
	
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
		Init_Buf(src_data,SSI_DATA_LEN,1,0); 		
	
		ret = SSI_EFlash_QUAD_Program(SSIx, sector_addr+i, src_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("Program failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
//		printf("\r\nafter quad program:\r\n");
	
//	printf("\r\nquad read:\r\n");
		ret = SSI_EFlash_QUAD_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
//		for(j=0; j<SSI_DATA_LEN; j++)
//		{
//			 printf("%02x ", dst_data[j]);
//		}	
	
		if(memcmp(src_data, dst_data, SSI_DATA_LEN)!=0x00)
		{
			printf("QUAD test data compare failed!addr:%08x\r\n",(sector_addr+i));
			// __ASM volatile("bkpt 0x12");
		}
		else
		{
			printf("QUAD test passed!addr:%08x\r\n",(sector_addr+i));
		}	
	}		
	
	printf("******************SSI QUAD test finish******************\r\n\r\n");
}	

void SSI_QPI_DMA_Demo(SSI_TypeDef *SSIx)
{
	unsigned char temp;
	int i,j,ch=0;;
    UINT16 ret;
	
	temp = temp;

    printf("***********************SSI QPI DMA test***********************\r\n");
	
	if(SSIx==SSI1)	
	{
	  DMA_REG_Init(DMA2_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA2_IRQn, 2);
#endif
	}

	if(SSIx==SSI2)	
	{
		DMA_REG_Init(DMA1_BASE_ADDR);

#ifdef  SSI_DMA_INT 
		NVIC_Init(3, 3, DMA1_IRQn, 2);
#endif
	}

	ret = SSI_EFlash_QPI_Sector_Erase(SSIx, sector_addr);
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }
    for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
        Init_Buf(dst_data,SSI_DATA_LEN,0,0x00);
		ret = SSI_EFlash_QPI_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
		for(j = 0; j < SSI_DATA_LEN; j ++)
		{
			if(dst_data[j] != 0xFF)
			{
				printf("Erase data compare failed!addr:%08x\r\n",(sector_addr+i+j));
				// __ASM volatile("bkpt 0x12");
			}
		}
    }

	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
		Init_Buf(src_data,SSI_PAGE_SIZE,1,0x00);	
		ret = SSI_EFlash_QPI_Write_Enable(SSIx);
        if(ret)
        {
            printf("SSI_EFlash_QPI_Write_Enable failed!\r\n");
        }
		ret = SSI_QPI_Init(SSIx, DATA_WRITE, SSI_PAGE_SIZE, 2); //set to quad
        if(ret)
        {
            printf("SSI_EFlash_QPI_Write_Enable failed!\r\n");
        }

//	printf("\r\nQUAD SPI DMA write ...\r\n"); 
		ret = SSI_QPI_DMA_Send(SSIx, ch, src_data, sector_addr+i, SSI_PAGE_SIZE, 1);
        if(ret)
        {
            printf("send failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
		ret = SSI_Wait_Eflash_QPI_Idle(SSIx);	
        if(ret)
        {
            printf("SSI_Wait_Eflash_QPI_Idle failed!\r\n");
        }

		//quad spi dma read	 	 
        ret = SSI_EFlash_Set_Read_Para(SSIx,2);
        if(ret)
        {
            printf("SSI_EFlash_Set_Read_Para failed!\r\n");
        }
		ret = SSI_QPI_Init(SSIx, DATA_READ, SSI_PAGE_SIZE, 2); //flush FIFO
        if(ret)
        {
            printf("SSI_QPI_Init failed!\r\n");
        }
		Init_Buf(dst_data,SSI_PAGE_SIZE,0,0x00);	 
		ret = SSI_QPI_DMA_Read(SSIx, ch, dst_data, sector_addr+i, SSI_PAGE_SIZE, 1);	
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
	 
//		printf("\r\nQUAD SPI DMA read ...\r\n");
//		for(j=0;j<SSI_DATA_LEN;j++)
//		{
//			printf("%02x ", *(dst_data+j));
//		}	

		if(memcmp(src_data, dst_data, SSI_PAGE_SIZE)!=0x00)
		{
			printf("QUAD DMA data compare failed!addr:%08x\r\n",(sector_addr+i));
			// __ASM volatile("bkpt 0x12");
		}	
		else
		{
			printf("QPI DMA passed!addr:%08x\r\n",(sector_addr+i));
		}	
	}
	printf("***********************SSI QPI DMA test finish***********************\r\n");
}	

void SSI_QPI_Demo(SSI_TypeDef *SSIx)
{
	int i,j;
    UINT16 ret = STATUS_OK;

    printf("******************SSI QPI test******************\r\n\r\n");
	
	ret = SSI_EFlash_QPI_Sector_Erase(SSIx,sector_addr);
    if(ret)
    {
        printf("Erase failed!addr:%08x,ret = %04x\r\n",sector_addr,ret);
    }
    for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
    {
        Init_Buf(dst_data,SSI_DATA_LEN,0,0x00);
		ret = SSI_EFlash_QPI_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
		for(j = 0; j < SSI_DATA_LEN; j ++)
		{
			if(dst_data[j] != 0xFF)
			{
				printf("Erase data compare failed!addr:%08x\r\n",(sector_addr+i+j));
				// __ASM volatile("bkpt 0x12");
			}
		}
    }
	
	for(i = 0; i < SSI_SECTOR_SIZE; i += SSI_PAGE_SIZE)
	{
		Init_Buf(src_data,SSI_DATA_LEN,1,0);
		ret = SSI_EFlash_QPI_Program(SSIx, sector_addr+i, src_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("Program failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
//		printf("\r\nafter quad program:\r\n");
	
//	printf("\r\nquad read:\r\n");
		ret = SSI_EFlash_QPI_Read(SSIx, sector_addr+i, dst_data, SSI_DATA_LEN);
        if(ret)
        {
            printf("read failed!addr:%08x,ret = %04x\r\n",(sector_addr+i),ret);
        }
//		for(j=0; j<SSI_DATA_LEN; j++)
//		{
//			 printf("%02x ", dst_data[j]);
//		}	
		
		// for(j = 0; j < 256; j ++)
		// {
		// 	if(src_data[j] != dst_data[j])
		// 	{
		// 		printf("j:%08x\r\n",j);
		// 		printf("src_data:%02x\r\n",src_data[j]);
		// 		printf("dst_data:%02x\r\n",dst_data[j]);
		// 		break;
		// 	}
		// }
	
		if(memcmp(src_data, dst_data, SSI_DATA_LEN)!=0x00)
		{
			printf("QPI test data compare failed!addr:%08x\r\n",(sector_addr+i));
			// __ASM volatile("bkpt 0x12");
		}
		else
		{
			printf("QPI test passed!addr:%08x\r\n",(sector_addr+i));
		}	
	}		
	printf("******************SSI QPI test finish******************\r\n\r\n");
}	

void SSI_Demo(void)
{
	UINT32 times = 0,i;
	SSI_TypeDef *ssi;
	ssi = SSI1;
	
	if(ssi==SSI2)	
	{
		IOCTRL->SWAP_CONTROL_REG|=0x18;	//使能SSI2
	}
	
	while(times < 10000)
	{
		for(i = 0x0; i < 0x100; i ++)
		{
			sector_addr = i*SSI_SECTOR_SIZE;

			//STD mode
			SSI_STD_Demo(ssi);
			SSI_STD_DMA_Demo(ssi);
			
			//DUAL mode
			SSI_DUAL_Demo(ssi);
			SSI_DUAL_DMA_Demo(ssi);
			
			//QUAD mode
			SSI_QUAD_Demo(ssi);
			SSI_QUAD_DMA_Demo(ssi);
			
			//QPI modo
			SSI_EFlash_QPI_Enter(ssi);
			SSI_QPI_Demo(ssi);
			SSI_QPI_DMA_Demo(ssi);
			SSI_EFlash_QPI_Exit(ssi);

			printf("times:%05d\r\n",times);
			printf("sector_addr:%08x\r\n",sector_addr);
		}
		times ++;
		sector_addr = 0;
	}
}	
