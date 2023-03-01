// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : uart_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "uart_drv.h"
#include "type.h"
#include "debug.h"
#include "string.h"
#include "uart_demo.h"
#include "ioctrl_drv.h"
#include "cpm_drv.h"
#include "eport_drv.h"
#include "tc_drv.h"
#include "wdt_drv.h"
#include "dmac_drv.h"
#include "delay.h"

static UINT8 uart_data_buf[0x1000] = {0};

void Demo_UART_PinSwap(UART_TypeDef *UARTx)
{
	//GINT[5:0]引脚具有SCI复用功能，开启SCI复用功能后，GINT将转化成为对应的SCI功能，同时，SCI引脚将转化成对应的GINT功能(不能做唤醒源)。
	if(UARTx == SCI1)
	{
		//SCI1 GINT0&GINT4 swap enable 
		IO_Ctrl_SCI_Swap(TX1_GINT4,TRUE);
		IO_Ctrl_SCI_Swap(RX1_GINT0,TRUE);
	}
	else if(UARTx == SCI2)
	{
		//SCI2 GINT3&GINT5 swap enable
		IO_Ctrl_SCI_Swap(TX2_GINT5,TRUE);
		IO_Ctrl_SCI_Swap(RX2_GINT3,TRUE);
	}
	else if(UARTx == SCI3)
	{
		//SCI3 GINT1&GINT2 swap enable
		IO_Ctrl_SCI_Swap(TX3_GINT1,TRUE);
		IO_Ctrl_SCI_Swap(RX3_GINT2,TRUE);
	}
	else
	{
		;
	}

	while(1);

}

void Demo_UART_CPUNormalTransfer(UART_TypeDef *UARTx)
{
    UART_InitTypeDef UART_InitStruct;
    UINT16 ret;

    printf("uart cpu normal transfer.\n");	

	//init uart
    UART_InitStruct.UART_BaudRate = 115200;
    UART_InitStruct.UART_Mode = UART_NORMAL_MODE;//采用CPU普通方式
	UART_InitStruct.UART_Parity = UART_PARITY_NONE;
#if 1  //UART_StopBits = 1
	UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_10BIT;
    UART_InitStruct.UART_StopBits = 1;
#else  //UART_StopBits = 2
    UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_11BIT;
    UART_InitStruct.UART_StopBits = 2;
#endif
	
	UART_Init(UARTx, &UART_InitStruct);

    while(1)
    {
        ret = UART_ReceiveData(UARTx,uart_data_buf,16);//串口接收数据
        if(STATUS_OK != ret)
        {
            printf("uart receive failed, ret = %04x\n",ret);
            continue;	
        }
		ret = UART_SendData(UARTx,uart_data_buf,16);//发送接收数据
        if(STATUS_OK != ret)
        {
            printf("uart send failed, ret = %04x\n",ret);
            continue;	
        }
    }

}

void Demo_UART_CPUIntTransfer(UART_TypeDef *UARTx)
{
    UART_InitTypeDef UART_InitStruct;
    UINT8 temp = 0;
    UINT16 ret;
    UINT32 i = 0,j = 0;
    UINT32 receive_len  = 0;

    printf("uart cpu interrupt transfer.\n");	

	//init uart
    UART_InitStruct.UART_BaudRate = 115200;
    UART_InitStruct.UART_Mode = UART_INT_MODE;//采用CPU中断方式
	UART_InitStruct.UART_Parity = UART_PARITY_NONE;
#if 1  //UART_StopBits = 1
	UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_10BIT;
    UART_InitStruct.UART_StopBits = 1;
#else  //UART_StopBits = 2
    UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_11BIT;
    UART_InitStruct.UART_StopBits = 2;
#endif
	
	UART_Init(UARTx, &UART_InitStruct);
    
    receive_len = 16;

    while(1)
    {
        ret = UART_RecvByte(UARTx, &temp);
       
        if(STATUS_OK == ret)  //收到一个字节数据
        {
            uart_data_buf[i++] = temp;

            if(i == receive_len)
            {
                ret = UART_SendData(UARTx, uart_data_buf,i);
                if(STATUS_OK != ret)
                {
                    printf("uart send failed, ret = %04x\n",ret);
                }
                i = 0;
                j = 0;
            }
        }
        else if(STATUS_NULL == ret) //没有数据
        {
            if(j++ > 5000000) //延迟打印
            {
                j = 0;
                printf("uart no data received.\n");
            }
        }
        else  //接收数据错误
        {
            printf("uart receive failed, ret = %04x\n",ret);
        }
        
    }

}

void Demo_UART_DMA_Transfer(UART_TypeDef *UARTx)
{
    UART_InitTypeDef UART_InitStruct;
    UINT8 dma_channel;
    UINT16 ret;
    UINT32 len = 0;

    printf("uart DMA transfer.\n");

	//init uart
    UART_InitStruct.UART_BaudRate = 115200;
    UART_InitStruct.UART_Mode = UART_DMA_MODE;//采用DMA普通方式
	UART_InitStruct.UART_Parity = UART_PARITY_NONE;
#if 1  //UART_StopBits = 1
	UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_10BIT;
    UART_InitStruct.UART_StopBits = 1;
#else  //UART_StopBits = 2
    UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_11BIT;
    UART_InitStruct.UART_StopBits = 2;
#endif
	
	UART_Init(UARTx, &UART_InitStruct);

    while(1)
    {
        dma_channel = 0;
		ret = UART_DMAReceiveData(UARTx,dma_channel,uart_data_buf, DMA_MAX_RECV_LEN);	//接收不定长数据，最大4095B
        if(STATUS_OK != ret)
        {
             printf("uart receive failed, ret = %04x\n",ret);
            continue;
        }
		len = DMA_GetRecvLen(UARTx,dma_channel);
        ret = UART_DMASendData(UARTx,dma_channel,uart_data_buf, len);
        if(STATUS_OK != ret)
        {
            printf("uart send failed, ret = %04x\n",ret);
            continue;	
        }
    }

}

void UART_Demo(void)
{
    UART_TypeDef *uart_type_id;
	uart_type_id = SCI1;
	    
	printf("uart demo...\r\n");		//保持SCI1 
    
	//UART CPU常规模式
    Demo_UART_CPUNormalTransfer(uart_type_id);

	//UART CPU中断模式
//    Demo_UART_CPUIntTransfer(uart_type_id);

	//UART DMA模式，使能了UART idle模式，可以接收不定长数据，最大4095B
//    Demo_UART_DMA_Transfer(uart_type_id);

	//UART 管脚复用切换demo
//    Demo_UART_PinSwap(uart_type_id);
	
}




