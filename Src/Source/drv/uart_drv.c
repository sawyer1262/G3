// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : uart_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "debug.h"
#include "cpm_drv.h"
#include "iomacros.h"
#include "uart_drv.h"
#include "uart_reg.h"
#include "dmac_drv.h"
#include "stddef.h"
#include "stdarg.h"
#include "stdlib.h"

volatile UINT8 uart_idle_flag = 0;


UartStruct_t UartStruct[3];//uartx buffer
//UartStruct_t g_Uart2RecvBufStruct = {RX_IDLE,TX_IDLE,0};//uartx buffer
//UartStruct_t g_Uart3RecvBufStruct = {RX_IDLE,TX_IDLE,0};//uartx buffer

/*******************************************************************************
* Function Name  : UART_ISR
* Description    : UART中断处理
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/

void UART_ISR(UART_TypeDef *UARTx, UartStruct_t *UARTxRecvBufStruct)
{
		if((UARTx->SCISR1&SCISR1_RDRF_MASK) == SCISR1_RDRF_MASK)   //单字节接收
		{
				if(UARTxRecvBufStruct->RxStat==RX_IDLE)
				{
						UARTxRecvBufStruct->RxStat=RX_RUNNING;
				}
				UARTxRecvBufStruct->dat[UARTxRecvBufStruct->wp] = UARTx->SCIDRL;
				UARTxRecvBufStruct->wp += 1;
				if (UARTxRecvBufStruct->wp >= UART_RECV_MAX_LEN)
				{
						UARTxRecvBufStruct->wp = 0;
				}
				UARTxRecvBufStruct->RxTime=GetTimerCount();   //复位超时
				UARTxRecvBufStruct->ret_val = STATUS_OK;
		}
		if((UARTx->SCISR1&SCISR1_TDRE_MASK) == SCISR1_TDRE_MASK && (UARTx->SCICR2&SCICR2_TIE_MASK)==SCICR2_TIE_MASK)   //发送缓存空
		{
				UARTx->SCIDRL = *(UARTxRecvBufStruct->pTxStart++);
				if(UARTxRecvBufStruct->pTxStart>=UARTxRecvBufStruct->pTxEnd)
				{
						UARTx->SCICR2&=~SCICR2_TIE_MASK;                //关TDRE中断
						UARTx->SCICR2|=SCICR2_TCIE_MASK; 
				}
				UARTxRecvBufStruct->TxTime=GetTimerCount();   //复位超时
		}
		if((UARTx->SCISR1&SCISR1_TC_MASK) == SCISR1_TC_MASK && (UARTx->SCICR2&SCICR2_TCIE_MASK)==SCICR2_TCIE_MASK)       //发送完成
		{
				UARTx->SCICR2&=~SCICR2_TCIE_MASK;                   //关TC中断
				//UARTxRecvBufStruct->TxStat=TX_IDLE;
				UARTxRecvBufStruct->TxTime=GetTimerCount();  //帧结束 
		}
		if((UARTx->SCISR1&SCISR1_OR_MASK) == SCISR1_OR_MASK)
    {
        UARTxRecvBufStruct->ret_val |= STATUS_UART_OVERRUN;
        UINT8 tmp = UARTx->SCIDRL;   //clear error flag
    }

    if(UARTx->SCIFSR2&SCISR2_FOR_MASK)
    {  
        UINT8 tmp = UARTx->SCIDRL;
        UARTx->SCIFSR2 |= SCISR2_FOR_MASK;
        UARTxRecvBufStruct->ret_val |= STATUS_UART_FIFO_OVERRUN;
    }

    //timeout
    if((UARTx->SCIFSR & SCIFSR_RTOS_MASK ) && (UARTx->SCIFCR2 & SCIFCR2_RXFTOIE))
    {
        UINT8 tmp = UARTx->SCIDRL;
        UARTx->SCIFCR2 |= (SCIFCR2_RXFCLR);
        UARTxRecvBufStruct->ret_val |= STATUS_UART_TIMEOUT;
    }
}


//static void UART_ISR(UART_TypeDef *UARTx, UartRecvBufStruct *UARTxRecvBufStruct)
//{
//    UINT32 timeout = UART_TIMEOUT(1000);
//    
//    UARTxRecvBufStruct->ret_val = STATUS_OK;

//	if(UART_DMA_MODE == UARTxRecvBufStruct->recv_mode)
//	{
//		if(UARTx->SCISR1 & SCISR1_IDLE_MASK)
//		{
//			while(((UARTx->SCIFSR)&SCIFSR_REMPTY_MASK) == 0) // 等待接收的数据向DMA传完
//			{
//				if((UARTx->SCIFSR)&SCIFSR_RTOS_MASK)
//				{
//					UARTx->SCIFSR2 = 0x0F;
//					break;
//				}

//                if(!timeout)
//                {
//                    UARTxRecvBufStruct->ret_val |= STATUS_UART_TIMEOUT;
//                    return;
//                }
//                timeout --;
//			}
//			uart_idle_flag = 1;
//		}
//	}
//    
//	if(UART_INT_MODE == UARTxRecvBufStruct->recv_mode)
//	{
//		if((UARTx->SCIFCR&SCIFCR_RFEN) || (UARTx->SCIFCR&SCIFCR_TFEN))     //FIFO模式
//		{
//	#if 0  //接收数据需要和FIFO RFTS中断对应，否则会丢数据
//			if((UARTx->SCIFSR & SCIFSR_RFTS_MASK) == SCIFSR_RFTS_MASK)
//			{
//				for(UINT8 loop = 0;; loop<UART_FIFO_TRIGGER_LEVEL; loop ++)
//				{
//					UARTxRecvBufStruct->dat[UARTxRecvBufStruct->wp + loop] = UARTx->SCIDRL;
//				}
//				UARTxRecvBufStruct->wp += loop;
//				if (UARTxRecvBufStruct->wp >= UART_RECV_MAX_LEN)
//					UARTxRecvBufStruct->wp = 0;
//				UARTxRecvBufStruct->ret_val = STATUS_OK;
//			}
//	#endif
//			
//			while((!(UARTx->SCIFSR & SCIFSR_REMPTY_MASK)) && (timeout))
//			{
//				UARTxRecvBufStruct->dat[UARTxRecvBufStruct->wp] = UARTx->SCIDRL;
//				UARTxRecvBufStruct->wp += 1;
//				if (UARTxRecvBufStruct->wp >= UART_RECV_MAX_LEN)
//					UARTxRecvBufStruct->wp = 0;
//				UARTxRecvBufStruct->ret_val = STATUS_OK;
//				timeout --;
//			}
//            		
//			if(!timeout)
//			{
//				UARTxRecvBufStruct->ret_val |= STATUS_UART_TIMEOUT;
//			}
//		
//		}
//		else
//		{
//			//reveice data
//			if((UARTx->SCISR1&SCISR1_RDRF_MASK) == SCISR1_RDRF_MASK)   //单字节收发模式
//			{
//				UARTxRecvBufStruct->dat[UARTxRecvBufStruct->wp] = UARTx->SCIDRL;
//				UARTxRecvBufStruct->wp += 1;
//				if (UARTxRecvBufStruct->wp >= UART_RECV_MAX_LEN)
//					UARTxRecvBufStruct->wp = 0;
//				UARTxRecvBufStruct->ret_val = STATUS_OK;
//			}
//			
//		} 
//	}

//    if((UARTx->SCISR1&SCISR1_OR_MASK) == SCISR1_OR_MASK)
//    {
//        UARTxRecvBufStruct->ret_val |= STATUS_UART_OVERRUN;
//        UINT8 tmp = UARTx->SCIDRL;   //clear error flag
//    }

//    if(UARTx->SCIFSR2&SCISR2_FOR_MASK)
//    {  
//        UINT8 tmp = UARTx->SCIDRL;
//        UARTx->SCIFSR2 |= SCISR2_FOR_MASK;
//        UARTxRecvBufStruct->ret_val |= STATUS_UART_FIFO_OVERRUN;
//    }

//    //timeout
//    if((UARTx->SCIFSR & SCIFSR_RTOS_MASK ) && (UARTx->SCIFCR2 & SCIFCR2_RXFTOIE))
//    {
//        UINT8 tmp = UARTx->SCIDRL;
//        UARTx->SCIFCR2 |= (SCIFCR2_RXFCLR);
//        UARTxRecvBufStruct->ret_val |= STATUS_UART_TIMEOUT;
//    }
//    
//}

/*******************************************************************************
* Function Name  : UART3_ISR
* Description    : UART2中断处理
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void SCI1_IRQHandler(void)
{
	UART_ISR(SCI1, &UartStruct[0]);
}

/*******************************************************************************
* Function Name  : UART4_ISR
* Description    : UART4中断处理
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void SCI2_IRQHandler(void)
{
	UART_ISR(SCI2, &UartStruct[1]);
}

/*******************************************************************************
* Function Name  : UART2_ISR
* Description    : UART2中断处理
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void SCI3_IRQHandler(void)
{
	UART_ISR(SCI3, &UartStruct[2]);
}


/*******************************************************************************
* Function Name  : UART_Init
* Description    : UART初始化
* Input          : - UARTx: 取值SCI2 or SCI_BT
*                  - UART_InitStruct：UARTx  initialization parameter structure Poniter
*
* Output         : None
* Return         : None
******************************************************************************/
void UART_Init(UART_TypeDef *UARTx, UART_InitTypeDef *UART_InitStruct)
{
	UINT16 bandrate_I;
	UINT8 bandrate_F;
	UINT8 bandrate_h;
	UINT8 bandrate_l;

	if (SCI1 == UARTx)
	{
		UartStruct[0].wp = 0;
		UartStruct[0].rp = 0; 
		if ((UART_InitStruct->UART_Mode == UART_INT_MODE)||
            (UART_InitStruct->UART_Mode == UART_DMA_MODE))
		{
			NVIC_Init(3, 3, SCI1_IRQn, 2);
		}
	}
	else if (SCI2 == UARTx)
	{
		UartStruct[1].wp = 0;
		UartStruct[1].rp = 0; 
		if ((UART_InitStruct->UART_Mode == UART_INT_MODE)||
            (UART_InitStruct->UART_Mode == UART_DMA_MODE))
		{
			NVIC_Init(3, 3, SCI2_IRQn, 2);
		}
	}
	else
	{
		UartStruct[2].wp = 0;
		UartStruct[2].rp = 0; 
		if ((UART_InitStruct->UART_Mode == UART_INT_MODE)||
            (UART_InitStruct->UART_Mode == UART_DMA_MODE))
		{
			NVIC_Init(3, 3, SCI3_IRQn, 2);
		}
	}

	bandrate_I = ((UINT16)(g_ips_clk*4/UART_InitStruct->UART_BaudRate))>>6;
	bandrate_F = (((UINT16)(g_ips_clk*8/UART_InitStruct->UART_BaudRate)+1)/2)&0x003f;
	bandrate_h = (UINT8)((bandrate_I>>8)&0x00ff);
	bandrate_l = (UINT8)(bandrate_I&0x00ff);

	UARTx->SCIBRDF = bandrate_F;		//Write float before Interger
	UARTx->SCIBDH = bandrate_h;
	UARTx->SCIBDL = bandrate_l;

	UARTx->SCICR1 = 0x00;
	UARTx->SCICR1 |= UART_InitStruct->UART_FrameLength; //11 bit frame
	if ((UART_InitStruct->UART_FrameLength == UART_DATA_FRAME_LEN_11BIT) &&
		(UART_InitStruct->UART_StopBits == 2))
	{
		UARTx->SCIDRH |= (1u << 6);
	}

	if (UART_InitStruct->UART_Parity == UART_PARITY_NONE)
	{
		UARTx->SCICR1 |= ParityDIS;	//parity disable
	}
	else
	{
		UARTx->SCICR1 |= ParityEN;	//parity enable
		UARTx->SCICR1 |= UART_InitStruct->UART_Parity;//odd parity
	}
 
	UARTx->SCICR2 = 0;
    UARTx->SCIFCR2 = 0;
//    UARTx->SCIRXTOCTR = UART_InitStruct->UART_TimeoutCounter;

    if (UART_InitStruct->UART_Mode == UART_INT_MODE)
		{
				if (SCI1 == UARTx)
				{
						UartStruct[0].recv_mode = UART_INT_MODE;
				}
				else if(SCI2 == UARTx)
				{
						UartStruct[1].recv_mode = UART_INT_MODE;
				}
				else
				{
						UartStruct[2].recv_mode = UART_INT_MODE;
				}
			
				//Setup SCI interrupt
				UARTx->SCICR2 |= SCICR2_RIE_MASK|SCICR2_RE_MASK|SCICR2_TE_MASK;             //发送接收使能，接收中断使能
				//UARTx->SCIFCR2 |= (SCIFCR2_RXFTOE|SCIFCR2_RXFTOIE|SCIFCR2_RXFCLR);        //
				//UARTx->SCIFCR = (SCIFCR_RFEN|SCIFCR_RXFLSEL_1_8);
		} 
    
    if(UART_InitStruct->UART_Mode == UART_DMA_MODE)
    {
        if (SCI1 == UARTx)
        {
            UartStruct[0].recv_mode = UART_DMA_MODE;
        }
        else if(SCI2 == UARTx)
        {
            UartStruct[1].recv_mode = UART_DMA_MODE;
        }
        else
        {
            UartStruct[2].recv_mode = UART_DMA_MODE;
        }
				UARTx->SCIFCR = 0;
				UARTx->SCIFCR  |= (SCIFCR_RFEN|SCIFCR_TFEN); 
				UARTx->SCIFCR2 |= (SCIFCR2_TXFCLR|SCIFCR2_RXFCLR|SCIFCR_RXFLSEL_1_8|SCIFCR_TXFLSEL_1_8);//enable fifo 
				UARTx->SCICR1  |=  SCICR1_ILT_MASK;
				UARTx->SCICR2  |= (SCICR2_RE_MASK|SCICR2_ILIE_MASK);
    }
} 

/*******************************************************************************
* Function Name  : UART_ByteRecieved
* Description    : UART接收到数据
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*
* Output         : - None
*
* Return         : - TRUE:接收到数据
*                  - FALSE:没有接收到数据
******************************************************************************/
INT8 UART_ByteRecieved(UART_TypeDef *UARTx)
{
	UartStruct_t *UARTxRecvBufStruct;
    
    if (SCI1 == UARTx)
    {
        UARTxRecvBufStruct = &UartStruct[0];
    }
    else if(SCI2 == UARTx)
    {
        UARTxRecvBufStruct = &UartStruct[1];
    }
    else
    {
        UARTxRecvBufStruct = &UartStruct[2];
    }


	if (UARTxRecvBufStruct->wp != UARTxRecvBufStruct->rp)
	{
		return TRUE;
	}
	return FALSE;
}
/*******************************************************************************
* Function Name  : UART_RecvByte
* Description    : UART接收一个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*
*
* Output         : - dat：接收数据缓冲
*
* Return         : - STATUS_OK:接收到数据
*                  - STATUS_NULL:没有接收到数据
******************************************************************************/
UINT16 UART_RecvByte(UART_TypeDef *UARTx, UINT8 *dat)
{
	UartStruct_t *UARTxRecvBufStruct; 
    UINT16 ret;
    
    if (SCI1 == UARTx)
    {
        UARTxRecvBufStruct = &UartStruct[0];
    }
    else if(SCI2 == UARTx)
    {
        UARTxRecvBufStruct = &UartStruct[1];
    }
    else
    {
        UARTxRecvBufStruct = &UartStruct[2];
    }

	if (UARTxRecvBufStruct->wp != UARTxRecvBufStruct->rp)
	{
		*dat = UARTxRecvBufStruct->dat[UARTxRecvBufStruct->rp];
		UARTxRecvBufStruct->rp += 1;
		if (UARTxRecvBufStruct->rp >= UART_RECV_MAX_LEN)
			UARTxRecvBufStruct->rp = 0;
		return STATUS_OK;
	}
    
    if(STATUS_OK != UARTxRecvBufStruct->ret_val)
    {
        ret = UARTxRecvBufStruct->ret_val;
//        UARTxRecvBufStruct->ret_val = STATUS_OK; 
    }
    else
    {
        ret = STATUS_NULL;
    }
    
	return ret;
}

/*******************************************************************************
* Function Name  : UART_ReceiveByte
* Description    : UART接收一个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*
* Output         : None
* Return         : 接收的一个字节数据
******************************************************************************/
UINT8 UART_ReceiveByte(UART_TypeDef *UARTx)
{  
	UINT8 tmp;
	UINT32 timeout = UART_TIMEOUT(1000);
    
    UartStruct[0].ret_val = STATUS_OK;
    UartStruct[1].ret_val = STATUS_OK;
    UartStruct[2].ret_val = STATUS_OK;

	while(0 == (UARTx->SCISR1 & SCISR1_RDRF_MASK))
	{
		if(!timeout)
		{
            if (SCI1 == UARTx)
            {
                UartStruct[0].ret_val = STATUS_UART_TIMEOUT;
            }
            else if(SCI2 == UARTx)
            {
                UartStruct[1].ret_val = STATUS_UART_TIMEOUT;
            }
            else
            {
                UartStruct[2].ret_val = STATUS_UART_TIMEOUT;
            }
			return 0;
		}
		timeout --;
	}

	tmp = UARTx->SCIDRL;

	return tmp;

}
/*******************************************************************************
* Function Name  : UART_ReceiveData
* Description    : UART接收多个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - recv_buf：接收数据缓冲
*                  - len：接收的数据长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 UART_ReceiveData(UART_TypeDef *UARTx, UINT8 *recv_buf, UINT32 len)
{
	UINT32 i;
    UINT16 status = STATUS_OK;

//    g_UartRecvBufStruct.ret_val = STATUS_OK;

	SCI_ENABLE_RECEIVER(UARTx);
	for(i = 0; i < len; i++)
	{
		recv_buf[i] = UART_ReceiveByte(UARTx);
        if (SCI1 == UARTx)
        {
            status = UartStruct[0].ret_val;
        }
        else if(SCI2 == UARTx)
        {
            status = UartStruct[1].ret_val;
        }
        else
        {
            status = UartStruct[2].ret_val;
        }
        
        if(STATUS_OK != status)
        {
            break;
        }

	}
	SCI_DISABLE_RECEIVER(UARTx);

    return status;
}

/*******************************************************************************
* Function Name  : UART_SendByte
* Description    : UART发送一个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - SendByte：发送的字节
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 UART_SendByte(UART_TypeDef *UARTx, UINT8 outbyte)
{
    UINT32 timeout = UART_TIMEOUT(1000);

	while((UARTx->SCISR1 & SCISR1_TC_MASK)==0)
    {
        if(!timeout)
        {
            return STATUS_UART_TIMEOUT;
        }
        timeout --;
    }
	while((UARTx->SCISR1 & SCISR1_TDRE_MASK)==0)
    {
        if(!timeout)
        {
            return STATUS_UART_TIMEOUT;
        }
        timeout --;
    }
	UARTx->SCIDRL = outbyte&0xff;
	while((UARTx->SCISR1 & SCISR1_TC_MASK)==0)
    {
        if(!timeout)
        {
            return STATUS_UART_TIMEOUT;
        }
        timeout --;
    }
    return STATUS_OK;
}

/*******************************************************************************
* Function Name  : UART_SendData
* Description    : UART发送多个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - send_buf：发送数据缓冲
*                  - len：发送的数据长度
*
* Output         : None
* Return         : None
******************************************************************************/
UINT16 UART_SendData(UART_TypeDef *UARTx, UINT8 *send_buf, UINT32 len)
{
	UINT16 ret;
    UINT32 i;

	//SCI_ENABLE_TRANSMITTER(UARTx);
	for (i = 0; i < len; i++)
	{
        ret = UART_SendByte(UARTx, send_buf[i]);
		if(STATUS_OK != ret)
        {
            break;
        }
	}
//	SCI_DISABLE_TRANSMITTER(UARTx);

    return ret;
}


/*******************************************************************************
* Function Name  : UART_ConfigGpio
* Description    : UART配置成GPIO用途
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - UART_Pin对应的PIN脚，取值UART_TX、UART_RX
*                  - UART_Dir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
void UART_ConfigGpio(UART_TypeDef *UARTx, UART_PINx UART_Pin, UINT8 UART_Dir)
{
	UARTx->SCICR1 = 0x80;
	UARTx->SCICR2 &= ~0x0c;

	if (UART_Dir == GPIO_OUTPUT)
	{
		UARTx->SCIDDR |= (1<<UART_Pin);//output
	}
	else if (UART_Dir == GPIO_INPUT)
	{
		UARTx->SCIDDR &= (~(1<<UART_Pin));//input
	}

}

/*******************************************************************************
* Function Name  : UART_ReadGpioData
* Description    : 获取UART_Pin对应引脚的电平
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - UART_Pin对应的PIN脚，取值UART_TX、UART_RX
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平
******************************************************************************/
INT8 UART_ReadGpioData(UART_TypeDef *UARTx, UART_PINx UART_Pin)
{
	INT8 bitstatus = 0x00;

	bitstatus = UARTx->SCIPORT;

	if (bitstatus &(Bit_SET<<UART_Pin))
		bitstatus = Bit_SET;
	else
		bitstatus = Bit_RESET;

	return bitstatus;
}

/*******************************************************************************
* Function Name  : UART_WriteGpioData
* Description    : 设置UART_Pin对应引脚的电平
* Input          : - - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - UART_Pin对应的PIN脚，取值UART_TX、UART_RX
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : None
******************************************************************************/
void UART_WriteGpioData(UART_TypeDef *UARTx, UART_PINx UART_Pin, UINT8 bitVal)
{
	if (bitVal == Bit_SET)
		UARTx->SCIPORT |= (Bit_SET<<UART_Pin);
	else
		UARTx->SCIPORT &= (~(Bit_SET<<UART_Pin));
}

/*******************************************************************************
* Function Name  : UART_DMAReceiveData
* Description    : UART DMA接收数据
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - channel:DMA通道
*                  - len：接收数据长度
*
* Output         : - dest：接收数据缓存
*                  
* Return         : STATUS
******************************************************************************/
UINT16 UART_DMAReceiveData(UART_TypeDef *UARTx,UINT8 channel,UINT8 *dest,UINT32 len)
{
    UINT16 ret;

	SCI_ENABLE_RECEIVER(UARTx);
  SCI_ENABLE_DMA(UARTx);
	ret = dma_uartRec(UARTx,channel,dest,len);
	SCI_DISABLE_DMA(UARTx);
	SCI_DISABLE_RECEIVER(UARTx);
	SCI_FIFO_CLR(UARTx);

    return ret;
}

/*******************************************************************************
* Function Name  : UART_DMASendData
* Description    : UART DMA发送数据
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - channel:DMA通道
*                  - dest：发送数据缓存
*                  - len：发送数据长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 UART_DMASendData(UART_TypeDef *UARTx,UINT8 channel,UINT8 *src,UINT32 len)
{
    UINT16 ret; 

	SCI_DISABLE_RECEIVER(UARTx);
	SCI_ENABLE_TRANSMITTER(UARTx);
	SCI_ENABLE_DMA(UARTx);
	ret = dma_uartTram(UARTx,channel,src,len);
	SCI_DISABLE_DMA(UARTx);

    return ret;
}


