// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : reset_drv.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef __RESET_DRV_H__
#define __RESET_DRV_H__

#include "reset_reg.h"


typedef enum
{
	/*save status*/
	RST_SOURCE_USER_WAKEUP_STATUS = 0,
	RST_SOURCE_USER_SDIO0_STATUS,
	RST_SOURCE_USER_TSI_STATUS,
	/*save flag*/
	RST_SOURCE_USER_WAKEUP_FLAG = RST_SOURCE_USER_WAKEUP_STATUS + 8,
	RST_SOURCE_USER_SDIO0_FLAG,
	RST_SOURCE_USER_TSI_FLAG
} RST_SOURCE_USER;

typedef enum _RST_STATUS_
{
	RST_RESERVED = 0,
	RST_SIM_CARD = 2,
	RST_FREQUENCE_DETECT = 4,
	RST_POR = 8,
	RST_WATCH_DOG_TIMER = 0x10,
	RST_SOFT = 0x20,
	RST_VOLTAGE_DETECT = 0x40,
	RST_TC_TIMER = 0x80,
} RST_STATUS,
	*P_RST_STATUS;

/*******************************************************************************
* Function Name  : Soft_Reset
* Description    : 芯片软件复位
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void Soft_Reset(void);

/*******************************************************************************
* Function Name  : Get_Reset_Status
* Description    : 获取芯片复位状态
* 				   bit7 bit6 bit5  bit4  bit3  bit2  bit1  bit0
*				   TCR  VD   SOFT  WDR   POR    FD   CR     NU
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern RST_STATUS Get_Reset_Status(void);

/*******************************************************************************
* Function Name  : Set_POR_Reset
* Description    : 产生一个Power on reset
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void Set_POR_Reset(void);

/*******************************************************************************
* Function Name  : Disable_SmartCardReset
* Description    : 禁止smart card 复位芯片复位
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void Disable_SmartCardReset(void);

extern void reset_updateSDIO_onSleep(volatile uint32_t *reg, const uint8_t cnt, const uint16_t delayMS);
extern void reset_updateWakeup_onSleep(volatile uint32_t *reg, const uint8_t cnt, const uint16_t delayMS);
extern void reset_updateSDIO(volatile uint32_t *reg);
extern void reset_updateWakeup(volatile uint32_t *reg);
#endif /* __RESET_DRV_H__ */
