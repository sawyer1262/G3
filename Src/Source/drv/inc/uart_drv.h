// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : uart_drv.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef UART_DRV_H_
#define UART_DRV_H_

#include "memmap.h"
#include "def.h"
#include "uart_reg.h"

#define SCI1		        ((UART_TypeDef *)(SCI1_BASE_ADDR))
#define SCI2		        ((UART_TypeDef *)(SCI2_BASE_ADDR))
#define SCI3		        ((UART_TypeDef *)(SCI3_BASE_ADDR))

#define SCI_BT          SCI2


#define IS_UART_ALL_PERIPH(PERIPH) (((PERIPH) == SCI) || \
                                    ((PERIPH) == SCI_BT))
////////////////////////////////////////////////////////////////////////////////////////

#define SCI2_ENABLE_TRANSMITTER      (SCI2->SCICR2 |= SCICR2_TE_MASK)
#define SCI2_DISABLE_TRANSMITTER     (SCI2->SCICR2 &= ~SCICR2_TE_MASK)

#define SCI2_ENABLE_RECEIVER         (SCI2->SCICR2 |= SCICR2_RE_MASK)
#define SCI2_DISABLE_RECEIVER        (SCI2->SCICR2 &= ~SCICR2_RE_MASK)



////////////////////////////////////////////////////////////////////////////////////////
#define		UART_DATA_FRAME_LEN_10BIT	    0				//10bit frame:----StartBit:1bit DataBit:8bit StopBit:1bit
#define		UART_DATA_FRAME_LEN_11BIT	    SCICR1_M_MASK

#define IS_UART_FRAMW_LENGTH(LENGTH) (((LENGTH) == UART_DATA_FRAME_LEN_10BIT) || \
                                      ((LENGTH) == UART_DATA_FRAME_LEN_11BIT))



#define		ParityDIS	                 0			      	//Parity Disable
#define		ParityEN	                 SCICR1_PE_MASK	//Parity Enable

#define		UART_PARITY_EVE		         0			      	//Even Parity
#define		UART_PARITY_ODD		         SCICR1_PT_MASK	//Odd Parity
#define   UART_PARITY_NONE           2              //NONE Parity


#define IS_UART_PARITY(PARITY) (((PARITY) == UART_PARITY_EVE) || \
                                ((PARITY) == UART_PARITY_ODD) || \
                                ((PARITY) == UART_PARITY_NONE))

#define UART_DELAY               20000
#define UART_TIMEOUT(n)          (n*UART_DELAY)
//#define UART_TIMEOUT_1S              20000000  // sys_clk=120MHz, delay 1MS ≈ sys_clk/6000
/*******************************************************************************
RXFLSEL                     UART_FIFO_LEN
SCIFCR_RXFLSEL_1_8          0x02
SCIFCR_RXFLSEL_1_4          0x04
SCIFCR_RXFLSEL_1_2          0x08
SCIFCR_RXFLSEL_3_4          0x0C
SCIFCR_RXFLSEL_7_8          0x0E
*******************************************************************************/
#define UART_FIFO_TRIGGER_LEVEL            0x02       //由RXFLSEL决定长度
#define UART_RECV_MAX_LEN                  0x200

typedef enum
{
	UART_NORMAL_MODE = 0,
	UART_INT_MODE,
	UART_EDMA_MODE,
	UART_DMA_MODE

}UART_MODE;

//至少选择一种模式
#define SCI_CPU_EN		0 
#define SCI_DMA_EN		0
#define SCI_INT_EN		1

#define IS_UART_MODE(MODE) (((MODE) == UART_NORMAL_MODE) || \
                            ((MODE) == UART_INT_MODE) || \
                            ((MODE) == UART_EDMA_MODE) || \
                            ((MODE) == UART_DMA_MODE) )

/* UART Init Structure definition */
typedef struct
{

  UINT32 UART_BaudRate;
  UINT8  UART_FrameLength;
  UINT8  UART_StopBits;
  UINT8  UART_Parity;
  UINT8  UART_Mode;
  UINT8  UART_TimeoutCounter;

} UART_InitTypeDef;

typedef enum
{
    UART_RX = 0,
    UART_TX,

}UART_PINx;

typedef enum{RX_IDLE=0,RX_RUNNING}RxStat_t; 
typedef enum{TX_IDLE=0,TX_RUNNING}TxStat_t; 

typedef struct _uart_recv_buf
{
		RxStat_t RxStat;                  //uart接收状态
		TxStat_t TxStat;                  //uart发送状态 
		UINT16 rp;                        //read pointer
		UINT16 wp;                        //write pointer
		UINT16 recv_mode;
		UINT16 ret_val;
		uint8_t RxFrameOk;
		uint8_t *pTxStart;                //数据发送起始位置
		uint8_t *pTxEnd;                  //数据发送结束位置
		uint32_t RtxTimeout;               //设定的收发超时时间
		volatile uint32_t RxTime;                   //接收字节超时计数
		volatile uint32_t TxTime;                   //发送字节超时计数
		UINT8  dat[UART_RECV_MAX_LEN];    //buffer
}UartStruct_t;

extern UartStruct_t UartStruct[3];
/*******************************************************************************
* Function Name  : UART_Init
* Description    : UART初始化
* Input          : - UARTx: where x can be 0 to 1 to select the UART peripheral；取值SCI0 or SCI1
*                  - UART_InitStruct：UARTx  initialization parameter structure Poniter
*
* Output         : None
* Return         : None
******************************************************************************/
extern void UART_Init(UART_TypeDef *UARTx, UART_InitTypeDef *UART_InitStruct);

/*******************************************************************************
* Function Name  : UART_ReceiveByte
* Description    : UART接收一个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*
* Output         : None
* Return         : 接收的一个字节数据
******************************************************************************/
extern UINT8 UART_ReceiveByte(UART_TypeDef *UARTx);
/*******************************************************************************
* Function Name  : UART_ReceiveData
* Description    : UART接收多个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - recv_buf：接收数据缓冲
*                  - len：发送的数据长度
*
* Output         : None
* Return         : STATUS
******************************************************************************/
extern UINT16 UART_ReceiveData(UART_TypeDef *UARTx,UINT8 *recv_buf, UINT32 len);

/*******************************************************************************
* Function Name  : UART_SendByte
* Description    : UART发送一个字节
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - SendByte：发送的字节
*
* Output         : None
* Return         : None
******************************************************************************/
extern UINT16 UART_SendByte(UART_TypeDef *UARTx, UINT8 outbyte);

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
extern UINT16 UART_SendData(UART_TypeDef *UARTx, UINT8 *send_buf, UINT32 len);

/*******************************************************************************
* Function Name  : UART_ByteRecieved
* Description    : UART接收到数据
* Input          : - UARTx:取值SCI1、 SCI2
*
* Output         : - None
*
* Return         : - TRUE:接收到数据
*                  - FALSE:没有接收到数据
******************************************************************************/
extern INT8 UART_ByteRecieved(UART_TypeDef *UARTx);

/*******************************************************************************
* Function Name  : UART_RecvByte
* Description    : UART接收一个字节
* Input          : - UARTx: where x can be 0 to 1 to select the UART peripheral；取值SCI0 or SCI1
*
*
* Output         : - dat：接收数据缓冲
*
* Return         : - TRUE:接收到数据
*                  - FALSE:没有接收到数据
******************************************************************************/
extern UINT16 UART_RecvByte(UART_TypeDef *UARTx, UINT8 *dat);

/*******************************************************************************
* Function Name  : UART_ConfigGpio
* Description    : UART配置成GPIO用途
* Input          : - UARTx: where x can be 1, 2  to select the UART peripheral.
*                  - UART_Pin对应的PIN脚，取值UART_TX、UART_RX
*                  - UART_Dir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
extern void UART_ConfigGpio(UART_TypeDef *UARTx, UART_PINx UART_Pin, UINT8 UART_Dir);

/*******************************************************************************
* Function Name  : UART_ReadGpioData
* Description    : 获取UART_Pin对应引脚的电平
* Input          : - UARTx: where x can be 1, 2  to select the UART peripheral.
*                  - UART_Pin对应的PIN脚，取值UART_TX、UART_RX
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平
******************************************************************************/
extern INT8 UART_ReadGpioData(UART_TypeDef *UARTx, UART_PINx UART_Pin);

/*******************************************************************************
* Function Name  : UART_WriteGpioData
* Description    : 设置UART_Pin对应引脚的电平
* Input          : - UARTx: where x can be 1, 2  to select the UART peripheral.
*                  - UART_Pin对应的PIN脚，取值UART_TX、UART_RX
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : None
******************************************************************************/
extern void UART_WriteGpioData(UART_TypeDef *UARTx, UART_PINx UART_Pin, UINT8 bitVal);

/*******************************************************************************
* Function Name  : UART_DMAReceiveData
* Description    : UART DMA接收数据
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - channel:DMA通道
*                  - dest：接收数据缓存
*                  - len：接收数据长度
*
* Output         : None
* Return         : None
******************************************************************************/
extern UINT16 UART_DMAReceiveData(UART_TypeDef *UARTx,UINT8 channel,UINT8 *dest,UINT32 len);
    
/*******************************************************************************
* Function Name  : UART_DMASendData
* Description    : UART DMA发送数据
* Input          : - UARTx: where x can be 1 to 3 to select the UART peripheral；取值UART1/UART2/UART3
*                  - channel:DMA通道
*                  - dest：发送数据缓存
*                  - len：发送数据长度
*
* Output         : None
* Return         : None
******************************************************************************/
extern UINT16 UART_DMASendData(UART_TypeDef *UARTx,UINT8 channel,UINT8 *src,UINT32 len);


#endif /* UART_DRV_H_ */
