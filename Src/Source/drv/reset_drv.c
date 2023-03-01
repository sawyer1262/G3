// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : reset_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "stdio.h"
#include "reset_drv.h"
#include "cpm_drv.h"
#include "eport_drv.h"
#include "tsi_drv.h"

/*******************************************************************************
* Function Name  : soft_reset
* Description    : 芯片软件复位
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void Soft_Reset(void)
{
	CHIP_RESET->RCR |= SOFTRST;
}

/*******************************************************************************
* Function Name  : get_reset_status
* Description    : 获取芯片复位状态
* 				   bit7 bit6 bit5  bit4  bit3  bit2  bit1  bit0
*					   TCR  VD   SOFT  WDR   POR    FD   CR     NU
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
RST_STATUS Get_Reset_Status(void)
{
	return (RST_STATUS)(CHIP_RESET->RSR);
}

/*******************************************************************************
* Function Name  : Set_POR_Reset
* Description    : 产生一个Power on reset
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void Set_POR_Reset(void)
{
	CPM_Write_CoreTestKey(1);
	CPM->CPM_VCCCTMR |= (1<<3);
	CPM_Write_CoreTestKey(0);
}


/*******************************************************************************
* Function Name  : Disable_SmartCardReset
* Description    : 禁止smart card 复位芯片复位
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void Disable_SmartCardReset(void)
{
	CHIP_RESET->RCR &= ~(CRE);
}

/**
 * @brief update SDIO pin status for Wakeup-function when sleep
 * (User-Reg | cpm-flag)
 * 1. 00 and 11 do not anything.
 * 2. 10 update status
 * 3. 01 wakeup
 * @pre reset_updateSDIO
 */
void reset_updateSDIO_onSleep(volatile uint32_t *reg, const uint8_t cnt, const uint16_t delayMS)
{
    const uint8_t offset = 2; // cpm bit offset
    uint8_t cnt_cmp = 0;
    volatile uint32_t flag = CPM->CPM_PADWKINTCR;
    if (flag & (1u << offset))
    {
        /* 11 */
        if (*reg & (1u << RST_SOURCE_USER_SDIO0_STATUS))
        {
            return;
        }
        /* 01 wakeup */
        else
        {
            /* check shake */
            for (uint8_t i = 0; i < cnt; i++)
            {
                CPM->CPM_PADWKINTCR |= (1u << offset); //clear CPM interrup flag
                TC_Sleep_DelayMS(delayMS);
                flag = CPM->CPM_PADWKINTCR;
                if (flag & (1u << offset))
                {
                    cnt_cmp++;
                }
                else
                {
                    break;
                }
            }
            if (cnt_cmp == cnt) // wakeup from sleep
            {
                *reg |= (1u << RST_SOURCE_USER_SDIO0_STATUS);
                *reg |= (1u << RST_SOURCE_USER_SDIO0_FLAG);
            }
            else
            {
                *reg &= ~(1u << RST_SOURCE_USER_SDIO0_STATUS);
                *reg &= ~(1u << RST_SOURCE_USER_SDIO0_FLAG);
            }
        }
    }
    else
    {
        /* 10 update status */
        if (*reg & (1u << RST_SOURCE_USER_SDIO0_STATUS))
        {
            /* check shake */
            for (uint8_t i = 0; i < cnt; i++)
            {
                CPM->CPM_PADWKINTCR |= (1u << offset); //clear CPM interrup flag
                TC_Sleep_DelayMS(delayMS);
                flag = CPM->CPM_PADWKINTCR;
                if ((flag & (1u << offset)) == 0)
                {
                    cnt_cmp++;
                }
                else
                {
                    break;
                }
            }
            if (cnt_cmp == cnt) // wakeup from sleep
            {
                *reg &= ~(1u << RST_SOURCE_USER_SDIO0_STATUS);
            }
            else
            {
                *reg |= (1u << RST_SOURCE_USER_SDIO0_STATUS);
            }
            *reg &= ~(1u << RST_SOURCE_USER_SDIO0_FLAG);
        }
        /* 00 */
        else
        {
            return;
        }
    }
}

/**
 * @brief update Wakeup pin status for Wakeup-function when sleep
 * (User-Reg | cpm-flag)
 * 1. 00 and 11 do not anything.
 * 2. 10 update status
 * 3. 01 wakeup
 * @pre reset_updateWakeup
 */
void reset_updateWakeup_onSleep(volatile uint32_t *reg, const uint8_t cnt, const uint16_t delayMS)
{
    const uint8_t offset = 1; // cpm bit offset
    uint8_t cnt_cmp = 0;
    volatile uint32_t flag = CPM->CPM_PADWKINTCR;
    if (flag & (1u << offset))
    {
        /* 11 */
        if (*reg & (1u << RST_SOURCE_USER_WAKEUP_STATUS))
        {
            return;
        }
        /* 01 wakeup */
        else
        {
            /* check shake */
            for (uint8_t i = 0; i < cnt; i++)
            {
                CPM->CPM_PADWKINTCR |= (1u << offset); //clear CPM interrup flag
                TC_Sleep_DelayMS(delayMS);
                flag = CPM->CPM_PADWKINTCR;
                if (flag & (1u << offset))
                {
                    cnt_cmp++;
                }
                else
                {
                    break;
                }
            }
            if (cnt_cmp == cnt) // wakeup from sleep
            {
                *reg |= (1u << RST_SOURCE_USER_WAKEUP_STATUS);
                *reg |= (1u << RST_SOURCE_USER_WAKEUP_FLAG);
            }
            else
            {
                *reg &= ~(1u << RST_SOURCE_USER_WAKEUP_STATUS);
                *reg &= ~(1u << RST_SOURCE_USER_WAKEUP_FLAG);
            }
        }
    }
    else
    {
        /* 10 update status */
        if (*reg & (1u << RST_SOURCE_USER_WAKEUP_STATUS))
        {
            /* check shake */
            for (uint8_t i = 0; i < cnt; i++)
            {
                CPM->CPM_PADWKINTCR |= (1u << offset); //clear CPM interrup flag
                TC_Sleep_DelayMS(delayMS);
                flag = CPM->CPM_PADWKINTCR;
                if ((flag & (1u << offset)) == 0)
                {
                    cnt_cmp++;
                }
                else
                {
                    break;
                }
            }
            if (cnt_cmp == cnt) // wakeup from sleep
            {
                *reg &= ~(1u << RST_SOURCE_USER_WAKEUP_STATUS);
            }
            else
            {
                *reg |= (1u << RST_SOURCE_USER_WAKEUP_STATUS);
            }
            *reg &= ~(1u << RST_SOURCE_USER_WAKEUP_FLAG);
        }
        /* 00 */
        else
        {
            return;
        }
    }
}

/**
 * @brief update SDIO pin status for Wakeup-function
 * 1. do not need to clear shake
 * 2. update Status
 * 3. clear user flag
 */
void reset_updateSDIO(volatile uint32_t *reg)
{
    if (CPM->CPM_PADWKINTCR & (1u << 2)) // SDIO0-pin
    {
        *reg |= (1u << RST_SOURCE_USER_SDIO0_STATUS);
    }
    else
    {
        *reg &= ~(1u << RST_SOURCE_USER_SDIO0_STATUS);
    }
    *reg &= ~(1u << RST_SOURCE_USER_SDIO0_FLAG);
}

/**
 * @brief update Wakeup pin status for Wakeup-function
 * 1. do not need to clear shake
 * 2. update Status
 * 3. clear user flag
 */
void reset_updateWakeup(volatile uint32_t *reg)
{
    if (CPM->CPM_PADWKINTCR & (1u << 1)) // wakeup-pin
    {
        *reg |= (1u << RST_SOURCE_USER_WAKEUP_STATUS);
    }
    else
    {
        *reg &= ~(1u << RST_SOURCE_USER_WAKEUP_STATUS);
    }
    *reg &= ~(1u << RST_SOURCE_USER_WAKEUP_FLAG);
}
