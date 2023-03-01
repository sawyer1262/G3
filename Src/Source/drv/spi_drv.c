// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : spi_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "debug.h"
#include "edma_drv.h"
#include "dmac_drv.h"
#include "spi_drv.h"

/*******************************************************************************
* Function Name  : SPI_StructInit
* Description    : Fills each SPI_InitStruct member with its default value.
* Input          : - SPI_InitStruct : pointer to a SPI_InitTypeDef structure
*                    which will be initialized.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_StructInit(UINT8 mode, SPI_InitTypeDef *SPI_InitStruct)
{
	/*--------------- Reset SPI init structure parameters values -----------------*/
	/* Initialize the SPI_Direction member */
	SPI_InitStruct->SPI_Direction = SPI_Direction_2Lines_FullDuplex;

	/* initialize the SPI_Mode member */
	SPI_InitStruct->SPI_Mode = mode;

	/* initialize the SPI_DataSize member */
	SPI_InitStruct->SPI_DataSize = SPI_DataSize_8b;

	/* Initialize the SPI_CPOL member */
	SPI_InitStruct->SPI_CPOL = SPI_CPOL_Low;

	/* Initialize the SPI_CPHA member */
	SPI_InitStruct->SPI_CPHA = SPI_CPHA_1Edge;

	/* Initialize the SPI_BaudRatePrescaler member */
	SPI_InitStruct->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

	/* Initialize the SPI_FirstBit member */
	SPI_InitStruct->SPI_FirstBit = SPI_FirstBit_MSB;

	/* Initialize the SPI_TimeoutCounter member, 5bits*/
	//    SPI_InitStruct->SPI_TimeoutCounter = 0x20;
}

/*******************************************************************************
* Function Name  : SPI_DMA_ISR
* Description    : SPI 中断响应函数
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void SPI_DMA_ISR(void)
{
	if(m_dma_control->DMA_STATTFR&0x01) dma_isr_flag=1;
	if(m_dma_control->DMA_STATTFR&0x02) dma_isr_flag=2;
}

static void SPI_ISR(SPI_TypeDef *SPIx)
{
}

void SPI1_IRQHandler(void)
{
	SPI_ISR(SPI1);
}

void SPI2_IRQHandler(void)
{
	SPI_ISR(SPI2);
}

void SPI3_IRQHandler(void)
{
	SPI_ISR(SPI3);
}

/*******************************************************************************
* Function Name  : SPI_Init
* Description    : SPI初始化
*
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - SPI_InitStruct: pointer to a SPI_InitTypeDef structure that
*                    contains the configuration information for the specified
*                    SPI peripheral.
* Output         : None
* Return         : None
******************************************************************************/
void SPI_Init(SPI_TypeDef *SPIx, SPI_InitTypeDef *SPI_InitStruct)
{
	UINT8 tmpreg = 0;

	//clear possible errors
	SPIx->SPISRHW = 0xffff;

	//flush FIFO
	SPIx->SPITXFCR = SPITXFCR_TXFCLR_MASK;
	SPIx->SPIRXFCR = SPIRXFCR_RXFCLR_MASK;

	tmpreg = SPIx->SPIFR;
	tmpreg &= ~SPIFR_FMSZ_MASK;
	tmpreg |= SPI_InitStruct->SPI_DataSize;
	SPIx->SPIFR = tmpreg;

	/*---------------------------- SPIx CR1 Configuration ------------------------*/
	/* Get the SPIx CR1 value */
	tmpreg = SPIx->SPICR1;
	/* Clear SPIE, SPE, SWOM, MSTR , MSTR, CPOL , CPHA , SSOE and LSBFirstbits */
	tmpreg &= SPICR1_CLEAR_MASK;
	tmpreg |= (UINT8)(SPI_InitStruct->SPI_CPHA |
					  SPI_InitStruct->SPI_CPOL |
					  SPI_InitStruct->SPI_FirstBit |
					  SPI_InitStruct->SPI_Mode);
	/* Write to SPIx CR1 */
	SPIx->SPICR1 = tmpreg;

	if (SPI_InitStruct->SPI_Mode == SPI_Mode_Master)
	{
		SPIx->SPIBR = SPI_InitStruct->SPI_BaudRatePrescaler;

		//SS high
		SPIx->SPIPORT |= SPIPORT_SS_MASK;
		SPIx->SPIPURD = 0x00;
		//SPI DATA Direction: ss is output  sck is output  MOSI is output MISO is input
		SPIx->SPIDDR = SPIDDR_SS_MASK | SPIDDR_SCK_MASK | SPIDDR_MOSI_MASK;
	}
	else
	{
		SPIx->SPIPURD = 0x00;
		if (SPI_InitStruct->SPI_Direction == SPI_Direction_1Line_RxOrTx)
			SPIx->SPIDDR &= ~(SPIDDR_SS_MASK | SPIDDR_MISO_MASK | SPIPORT_SCK_MASK);
		else
			SPIx->SPIDDR = SPIDDR_MISO_MASK; //SPI DATA Direction: ss is input  sck is input  MOSI is input MISO is output
	}

	if (SPI_InitStruct->SPI_Direction == SPI_Direction_1Line_RxOrTx) //
		SPIx->SPICR2 |= SPICR2_SPC0_MASK;							 //enables the bidirectional pin configuration
	else
		SPIx->SPICR2 &= ~(SPICR2_SPC0_MASK);

	//Timeout
	//    SPIx->SPITXFTOCTR = (SPITXFTOCTR_TXFTOE_MASK|SPI_InitStruct->SPI_TimeoutCounter);
	//    SPIx->SPIRXFTOCTR = (SPIRXFTOCTR_RXFTOE_MASK|SPI_InitStruct->SPI_TimeoutCounter);

	/* Activate the SPI mode*/
	SPIx->SPICR1 |= SPI_ENABLE;
}

UINT16 SPI_GetStatusErr(SPI_TypeDef *SPIx)
{
	UINT16 ret = 0;
	if (SPIx->SPISRHW & 0x6640)
	{
		if (SPIx->SPISRHW & SPISR_TXFOVF_MASK)
		{
			SPIx->SPISRHW |= SPISR_TXFOVF_MASK;
			ret = STATUS_SPI_TXFIFO_OVERFLOW_ERR;
		}

		if (SPIx->SPISRHW & SPISR_TXFUDF_MASK)
		{
			SPIx->SPISRHW |= SPISR_TXFUDF_MASK;
			ret |= STATUS_SPI_TXFIFO_UNDERFLOW_ERR;
		}

		if (SPIx->SPISRHW & SPISR_RXFOVF_MASK)
		{
			SPIx->SPISRHW |= SPISR_RXFOVF_MASK;
			ret |= STATUS_SPI_RXFIFO_OVERFLOW_ERR;
		}

		if (SPIx->SPISRHW & SPISR_RXFUDF_MASK)
		{
			SPIx->SPISRHW |= SPISR_RXFUDF_MASK;
			ret |= STATUS_SPI_RXFIFO_UNDERFLOW_ERR;
		}

		if (SPIx->SPISRHW & SPISR_FLOST_MASK)
		{
			SPIx->SPISRHW |= SPISR_FLOST_MASK;
			ret |= STATUS_SPI_FRAME_LOSR_ERR;
		}
	}
	return ret;
}

/**
 * @brief SPI-Master同时收发函数.
 * 
 * @param SPIx[in] 
 * @param Send[in] pointer of send buffer
 * @param Recv[out] pointer of receive buffer
 * @param Len[in] send/receive len
 */
UINT8 SPI_MasterTransceive(SPI_TypeDef* SPIx, UINT8 *Revc, UINT16 Len)
{
	UINT32 timeout = SPI_TIMEOUT_MS(500);
	while(Len != 0)
	{
		while(((SPIx->SPISRHW)&(SPISR_TXFFULL_MASK)) && timeout>0)
		{
				timeout--;
		}
		SPIx->SPIDR = *Revc;
		timeout = SPI_TIMEOUT_MS(500);
		while(!(0x80 & SPIx->SPISR) && timeout>0)
		{
				timeout--;
		}
		timeout = SPI_TIMEOUT_MS(500);
		while(((SPIx->SPISRHW) & (SPISR_RXFEMP_MASK)) && timeout>0)
		{
				timeout--;
		}

		*Revc = SPIx->SPIDR;
		Revc++;
		Len--;
	}
	return 0;
}

//uint16_t SPI_MasterSendRecvData(SPI_TypeDef *SPIx, UINT8 *Send, UINT8 *Recv, UINT32 Len)
//{
//	UINT32 timeout = SPI_TIMEOUT_MS(500);

//	while (Len != 0)
//	{
//		while (((SPIx->SPISRHW) & (SPISR_TXFFULL_MASK)))
//		{
//			if ((SPIx->SPISRHW & SPISR_TXFTO_MASK) || (!timeout))
//			{
//				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
//				SPIx->SPISRHW |= SPISR_TXFTO_MASK; //clear timeout flag
//				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
//			}
//			timeout--;
//		}

//		SPIx->SPIDR = *Send++;

//		while (!(0x80 & SPIx->SPISR))
//		{
//			if (!timeout)
//			{
//				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
//				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
//			}
//			timeout--;
//		}

//		while (((SPIx->SPISRHW) & (SPISR_RXFEMP_MASK)))
//		{
//			if ((SPIx->SPISRHW & SPISR_RXFTO_MASK) || (!timeout))
//			{
//				SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK;
//				SPIx->SPISRHW |= SPISR_RXFTO_MASK; //clear timeout flag
//				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
//			}
//			timeout--;
//		}
//		*Recv++ = SPIx->SPIDR;
//		Len--;
//	}

//	return STATUS_OK;
//}

/*******************************************************************************
* Function Name  : SPI_MasterSendData
* Description    : SPI作为主设备发送数据
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - Send: 发送数据缓冲
*                  - Len：发送数据的长度
*
* Output         : None
* Return         : None
******************************************************************************/
UINT16 SPI_MasterSendData(SPI_TypeDef *SPIx, UINT8 *Send, UINT32 Len)
{
	UINT8 ucTemp = 0;
	UINT32 timeout = SPI_TIMEOUT_MS(500);

	ucTemp = ucTemp;

	while (Len != 0)
	{
		while (((SPIx->SPISRHW) & (SPISR_TXFFULL_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_TXFTO_MASK) || (!timeout))
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_TXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}

		SPIx->SPIDR = *Send++;

		while (!(0x80 & SPIx->SPISR))
		{
			if (!timeout)
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}

		while (((SPIx->SPISRHW) & (SPISR_RXFEMP_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_RXFTO_MASK) || (!timeout))
			{
				SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_RXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}
		ucTemp = SPIx->SPIDR;
		Len--;
	}

	return STATUS_OK;
}

/*******************************************************************************
* Function Name  : SPI_MasterFixedSendData
* Description    : SPI作为主设备发送长度Len的相同Send数据
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - Send: 发送的数据
*                  - Len：发送数据的长度
*
* Output         : None
* Return         : None
******************************************************************************/
UINT16 SPI_MasterFixedSendData(SPI_TypeDef *SPIx, UINT8 Send, UINT16 Len)
{
	UINT8 ucTemp = 0;
	UINT32 timeout = SPI_TIMEOUT_MS(500);

	ucTemp = ucTemp;

	while (Len != 0)
	{
		while (((SPIx->SPISRHW) & (SPISR_TXFFULL_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_TXFTO_MASK) || (!timeout))
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_TXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}
		SPIx->SPIDR = Send;
		while (!(0x80 & SPIx->SPISR))
		{
			if (!timeout)
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}

		while (((SPIx->SPISRHW) & (SPISR_RXFEMP_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_RXFTO_MASK) || (!timeout))
			{
				SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_RXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}
		ucTemp = SPIx->SPIDR;
		Len--;
	}
	return STATUS_OK;
}

/*******************************************************************************
* Function Name  : SPI_MasterReceiveData
* Description    : SPI作为主设备接收来自从设备数据
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*
*                  - Len：需要接收数据的长度
*
* Output         : - Revc: 接收数据缓冲
* Return         : None
******************************************************************************/
UINT16 SPI_MasterReceiveData(SPI_TypeDef *SPIx, UINT8 *Revc, UINT16 Len)
{
	UINT8 ucTemp = 0xa5;
	UINT32 timeout = SPI_TIMEOUT_MS(500);

	while (Len != 0)
	{
		while (((SPIx->SPISRHW) & (SPISR_TXFFULL_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_TXFTO_MASK) || (!timeout))
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_TXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_RXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}

		SPIx->SPIDR = ucTemp;
		while (!(0x80 & SPIx->SPISR))
		{
			if (!timeout)
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				return (STATUS_SPI_RXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}

		while (((SPIx->SPISRHW) & (SPISR_RXFEMP_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_RXFTO_MASK) || (!timeout))
			{
				SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_RXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_RXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}

		*Revc++ = SPIx->SPIDR;
		Len--;
	}

	return STATUS_OK;
}

/*******************************************************************************
* Function Name  : SPI_MasterSendData
* Description    : SPI作为从设备发送数据
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - Send: 发送数据缓冲
*                  - Len：发送数据的长度
*
* Output         : None
* Return         : None
******************************************************************************/
UINT16 SPI_SlaveSendData(SPI_TypeDef *SPIx, UINT8 *Send, UINT16 Len)
{
	UINT32 timeout = SPI_TIMEOUT_MS(500);

	while (Len != 0)
	{
		while (((SPIx->SPISRHW) & (SPISR_TXFFULL_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_TXFTO_MASK) || (!timeout))
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_TXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}
		SPIx->SPIDR = *Send++;
		while (!(0x80 & SPIx->SPISR))
		{
			if (!timeout)
			{
				SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
				return (STATUS_SPI_TXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}

		Len--;
	}
	SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK;
	return STATUS_OK;
}

/*******************************************************************************
* Function Name  : SPI_SlaveReceiveData
* Description    : SPI作为从设备接收来自主设备数据
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*
*                  - Len：需要接收数据的长度
*
* Output         : - Revc: 接收数据缓冲
* Return         : None
******************************************************************************/
UINT16 SPI_SlaveReceiveData(SPI_TypeDef *SPIx, UINT8 *Revc, UINT16 Len)
{
	UINT32 timeout = SPI_TIMEOUT_MS(1000);

	while (Len != 0)
	{
		while (((SPIx->SPISRHW) & (SPISR_RXFEMP_MASK)))
		{
			if ((SPIx->SPISRHW & SPISR_RXFTO_MASK) || (!timeout))
			{
				SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK;
				SPIx->SPISRHW |= SPISR_RXFTO_MASK; //clear timeout flag
				return (STATUS_SPI_RXFIFO_TIMEOUT | SPI_GetStatusErr(SPIx));
			}
			timeout--;
		}
		*Revc++ = SPIx->SPIDR;
		Len--;
	}
	return STATUS_OK;
}

/*******************************************************************************
* Function Name  : SPI_Cmd
* Description    : Enables or disables the specified SPI peripheral.
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - NewState: new state of the SPIx peripheral.
*                    This parameter can be: ENABLE or DISABLE.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Cmd(SPI_TypeDef *SPIx, FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		/* Enable the selected SPI peripheral */
		SPIx->SPICR1 |= CR1_SPE_Set;
	}
	else
	{
		/* Disable the selected SPI peripheral */
		SPIx->SPICR1 &= CR1_SPE_Reset;
	}
}

/*******************************************************************************
* Function Name  : SPI_ConfigGpio
* Description    : SPI配置成GPIO用途
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - SPI_Pin：SPI对应的PIN脚，取值SPI_MISO、SPI_MOSI、SPI_SCK、SPI_SS
*                  - SPI_Dir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
void SPI_ConfigGpio(SPI_TypeDef *SPIx, SPI_PINx SPI_Pin, UINT8 SPI_Dir)
{

	if (SPI_Dir == GPIO_OUTPUT)
	{
		//详见手册Table 18-4
		//		if((SPI_Pin == SPI_MISO) || (SPI_Pin == SPI_SS))//xukai20181209实际应用中，ss可能单独配置为GPIO，其余SPI引脚仍然用作SPI功能，所以配置SS时不能设置SPICR2为0x01
		if (SPI_Pin == SPI_MISO)
		{
			SPIx->SPICR2 = 0x01;
		}
		else if ((SPI_Pin == SPI_MOSI) || (SPI_Pin == SPI_SCK))
		{
			SPIx->SPICR1 = 0x10;
			SPIx->SPICR2 = 0x01;
		}

		SPIx->SPIDDR |= (1 << SPI_Pin); //output
	}
	else if (SPI_Dir == GPIO_INPUT)
	{
		SPIx->SPIDDR &= (~(1 << SPI_Pin)); //input
	}
}

/*******************************************************************************
* Function Name  : SPI_ReadGpioData
* Description    : 获取SPI_Pin对应引脚的电平
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - SPI_Pin：SPI对应的PIN脚，取值SPI_MISO、SPI_MOSI、SPI_SCK、SPI_SS
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平
******************************************************************************/
UINT8 SPI_ReadGpioData(SPI_TypeDef *SPIx, SPI_PINx SPI_Pin)
{
	UINT8 bitstatus = 0x00;

	bitstatus = SPIx->SPIPORT;

	if (bitstatus & (Bit_SET << SPI_Pin))
		bitstatus = Bit_SET;
	else
		bitstatus = Bit_RESET;

	return bitstatus;
}

/*******************************************************************************
* Function Name  : SPI_WriteGpioData
* Description    : 设置SPI_Pin对应引脚的电平
* Input          : - SPIx: where x can be 1, 2  to select the SPI peripheral.
*                  - SPI_Pin：SPI对应的PIN脚，取值SPI_MISO、SPI_MOSI、SPI_SCK、SPI_SS
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : None
******************************************************************************/
void SPI_WriteGpioData(SPI_TypeDef *SPIx, SPI_PINx SPI_Pin, UINT8 bitVal)
{
	if (bitVal == Bit_SET)
		SPIx->SPIPORT |= (Bit_SET << SPI_Pin);
	else
		SPIx->SPIPORT &= (~(Bit_SET << SPI_Pin));
}

/*******************************************************************************
* Function Name  : SPI_MasterSendDataByEDMA
* Description    : SPI作为主设备发送并接收数据
* Input          : - SPIx: where x can be 1, 2 ,3 to select the SPI peripheral.
*                  - sendAddr: 发送数据缓冲首地址
*                  - Len：发送数据的长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SPI_MasterSendDataByEDMA(SPI_TypeDef *SPIx, UINT32 sendAddr, UINT32 Len)
{
	UINT16 ret;

	SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK; //操作完需清理FIFO
	SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK; //操作完需清理FIFO

	if (SPIx == SPI1)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, sendAddr, EDMAC_TTYPE2, EDMA_SPI1, Len);
	else if (SPIx == SPI2)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, sendAddr, EDMAC_TTYPE2, EDMA_SPI2, Len);
	else if (SPIx == SPI3)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, sendAddr, EDMAC_TTYPE2, EDMA_SPI3, Len);
	else
		return STATUS_ID_ERR;

	return ret;
}

/*******************************************************************************
* Function Name  : SPI_MasterRecvDataByEDMA
* Description    : SPI作为主设备发送并接收数据
* Input          : - SPIx: where x can be 1, 2 ,3 to select the SPI peripheral.
*                  - recvAddr: 接收数据缓冲首地址
*                  - Len：发送数据的长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SPI_MasterRecvDataByEDMA(SPI_TypeDef *SPIx, UINT32 recvAddr, UINT32 Len)
{
	UINT16 ret;

	SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK; //操作完需清理FIFO
	SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK; //操作完需清理FIFO

	if (SPIx == SPI1)
		ret = EDMA_CHx(EDMA_CH0, recvAddr, recvAddr, EDMAC_TTYPE3, EDMA_SPI1, Len);
	else if (SPIx == SPI2)
		ret = EDMA_CHx(EDMA_CH0, recvAddr, recvAddr, EDMAC_TTYPE3, EDMA_SPI2, Len);
	else if (SPIx == SPI3)
		ret = EDMA_CHx(EDMA_CH0, recvAddr, recvAddr, EDMAC_TTYPE3, EDMA_SPI3, Len);
	else
		return STATUS_ID_ERR;

	return ret;
}

/*******************************************************************************
* Function Name  : SPI_SlaveSendDataByEDMA
* Description    : SPI从设备发送数据，不会改变源buf数据
* Input          : - SPIx: where x can be 1, 2 ,3 to select the SPI peripheral.
*                  - sendAddr: 发送数据缓冲首地址
*                  - Len：发送数据的长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SPI_SlaveSendDataByEDMA(SPI_TypeDef *SPIx, UINT32 sendAddr, UINT32 Len)
{
	UINT16 ret;

	SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK; //操作完需清理FIFO
	SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK; //操作完需清理FIFO

	if (SPIx == SPI1)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, sendAddr, EDMAC_TTYPE2, EDMA_SPI1, Len);
	else if (SPIx == SPI2)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, sendAddr, EDMAC_TTYPE2, EDMA_SPI2, Len);
	else if (SPIx == SPI3)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, sendAddr, EDMAC_TTYPE2, EDMA_SPI3, Len);
	else
		return STATUS_ID_ERR;

	return ret;
}

/*******************************************************************************
* Function Name  : SPI_SlaveRecvDataByEDMA
* Description    : SPI从设备接收来自主设备数据
* Input          : - SPIx: where x can be 1, 2, 3  to select the SPI peripheral.
*				   - recvAddr: 接收数据缓冲首地址
*                  - Len：需要接收数据的长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SPI_SlaveRecvDataByEDMA(SPI_TypeDef *SPIx, UINT32 recvAddr, UINT32 Len)
{
	UINT16 ret;

	SPIx->SPIRXFCR |= SPIRXFCR_RXFCLR_MASK; //操作完需清理FIFO
	SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK; //操作完需清理FIFO

	if (SPIx == SPI1)
		ret = EDMA_CHx(EDMA_CH0, recvAddr, recvAddr, EDMAC_TTYPE1, EDMA_SPI1, Len);
	else if (SPIx == SPI2)
		ret = EDMA_CHx(EDMA_CH0, recvAddr, recvAddr, EDMAC_TTYPE1, EDMA_SPI2, Len);
	else if (SPIx == SPI3)
		ret = EDMA_CHx(EDMA_CH0, recvAddr, recvAddr, EDMAC_TTYPE1, EDMA_SPI3, Len);
	else
		return STATUS_ID_ERR;

	return ret;
}

/*******************************************************************************
* Function Name  : EDMA_SPITransfer
* Description    : SPI主或从发送并接收数据
* Input          : - SPIx: where x can be 1, 2 ,3 to select the SPI peripheral.
*                  - sendAddr: 发送数据缓冲首地址
*                  - recvAddr: 接收数据缓冲首地址
*                  - Len：发送数据的长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 EDMA_SPITransfer(SPI_TypeDef *SPIx, UINT32 sendAddr, UINT32 recvAddr, UINT32 Len)
{
	UINT16 ret;

	//    SPIx->SPICR1 |= SPI_ENABLE;

	if (SPIx == SPI1)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, recvAddr, EDMAC_TTYPE3, EDMA_SPI1, Len);
	else if (SPIx == SPI2)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, recvAddr, EDMAC_TTYPE3, EDMA_SPI2, Len);
	else if (SPIx == SPI3)
		ret = EDMA_CHx(EDMA_CH0, sendAddr, recvAddr, EDMAC_TTYPE3, EDMA_SPI3, Len);
	else
		return STATUS_ID_ERR;

	SPIx->SPITXFCR |= SPITXFCR_TXFCLR_MASK; //操作完需清理FIFO
  SPIx->SPITXFCR |= SPIRXFCR_RXFCLR_MASK;
	//    SPIx->SPICR1 &= ~SPI_ENABLE;    //SPI EDMA 传输完需要关闭SPI

	return ret;
}

/*******************************************************************************
* Function Name  : SPI_EnableDMA
* Description    : SPI DMA功能的使能
* Input          : - SPIx: where x can be 1, 2, 3  to select the SPI peripheral.
*				   - enable: 是否使能
*
* Output         : None
* Return         : None
******************************************************************************/
void SPI_EnableDMA(SPI_TypeDef *SPIx, bool enable)
{
	if (enable == TRUE)
	{
		SPIx->SPIDMACR = SPISR_TXDMAE_MASK | SPISR_RXDMAE_MASK;
	}
	else
	{
		SPIx->SPIDMACR &= (~(SPISR_TXDMAE_MASK | SPISR_RXDMAE_MASK));
	}
}
/*******************************************************************************
* Function Name  : SPI_StructInit_NFC
* Description    : Fills each SPI_InitStruct member with its default value.
* Input          : - SPI_InitStruct : pointer to a SPI_InitTypeDef structure
*                    which will be initialized.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_StructInit_NFC(UINT8 mode, SPI_InitTypeDef *SPI_InitStruct)
{
	/*--------------- Reset SPI init structure parameters values -----------------*/
	/* Initialize the SPI_Direction member */
	SPI_InitStruct->SPI_Direction = SPI_Direction_2Lines_FullDuplex;

	/* initialize the SPI_Mode member */
	SPI_InitStruct->SPI_Mode = mode;

	/* initialize the SPI_DataSize member */
	SPI_InitStruct->SPI_DataSize = SPI_DataSize_8b;

	/* Initialize the SPI_CPOL member */
	SPI_InitStruct->SPI_CPOL = SPI_CPOL_Low;

	/* Initialize the SPI_CPHA member */
	SPI_InitStruct->SPI_CPHA = SPI_CPHA_1Edge;

	/* Initialize the SPI_BaudRatePrescaler member */
	SPI_InitStruct->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

	/* Initialize the SPI_FirstBit member */
	SPI_InitStruct->SPI_FirstBit = SPI_FirstBit_MSB;
}
