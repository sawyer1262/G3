/*
 * usi_dev.h
 *
 *  Created on: 2017年5月26日
 *      Author: YangWenfeng
 */

#ifndef USI_DEV_H_
#define USI_DEV_H_

#include "usi_reg.h"
#include "reset_reg.h"

//#define USI_BASE_ADDR      USI1_BASE_ADDR
#define USI_BASE_ADDR      USI2_BASE_ADDR

#define USI         ((USI_TypeDef *)USI_BASE_ADDR)

#if(USI_BASE_ADDR == USI1_BASE_ADDR)
	#define USI_IRQn    USI1_IRQn
#else
	#define USI_IRQn    USI2_IRQn
#endif

#define USI1         ((USI_TypeDef *)USI1_BASE_ADDR)
#define USI2         ((USI_TypeDef *)USI2_BASE_ADDR)

typedef enum
{
    USI_RST = 0,
    USI_CLK,
    USI_DAT,
}USI_PIN;

#define IS_USI_PINx(pin) (((pin) == USI_RST) || \
                          ((pin) == USI_CLK) ||\
                         ((pin) == USI_DAT) )
                          
#define ISO7816_1    1
#define ISO7816_2    2

#define card_1v8			0x0
#define card_3v				0x1
#define card_3v3			0x2
#define card_5v			    0x3

extern USI_TypeDef * g_USIReg;

/*******************************************************************************
* Function Name  : USI_ConfigGpio
* Description    : USI配置成GPIO用途
* Input          : - USIx    : USI的基地址，USI1或者USI2
*                  - USI_PINx：USI对应的PIN脚，取值USI_RST、USI_CLK、USI_DAT
*                  - GPIO_Dir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
extern void USI_ConfigGpio(USI_TypeDef * USIx, USI_PIN USI_PINx, UINT8 GPIO_Dir);

/*******************************************************************************
* Function Name  : USI_ReadGpioData
* Description    : 获取USI_PINx对应引脚的电平
* Input          : - USIx    : USI的基地址，USI1或者USI2
*                  - USI_PINx：USI对应的PIN脚，取值USI_RST、USI_CLK、USI_DAT
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平
******************************************************************************/
extern UINT8 USI_ReadGpioData(USI_TypeDef * USIx, USI_PIN USI_PINx);

/*******************************************************************************
* Function Name  : USI_WriteGpioData
* Description    : 设置USI_PINx对应引脚的电平
* Input          : - USIx    : USI的基地址，USI1或者USI2
*                  - USI_PINx：SPI对应的PIN脚，取值USI_RST、USI_CLK、USI_DAT
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : None
******************************************************************************/
extern void USI_WriteGpioData(USI_TypeDef * USIx, USI_PIN USI_PINx, UINT8 bitVal);

extern void USI_Init(void);
extern void USI_DeInit(void);

extern void USI_PullLow(void);
extern void USI_Resume(void);

extern void USIMasterInit(void);
extern void USIMasterHotInit(void);

extern void Card2_Power_On(void);
extern void Card2_Power_Off(void);
extern void Card2_Power_Switch(UINT8 mode);

extern void Card3_Power_On(void);
extern void Card3_Power_Off(void);
extern void Card3_Power_Switch(UINT8 mode);

extern void USI_OpenWTC(UINT32 etu);

extern void TransmitByte(UINT8 data);
extern UINT8 ReceiveByte(void);
#endif /* USI_DEV_H_ */
