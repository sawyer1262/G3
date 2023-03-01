/**
	******************************************************************************
             Copyright(c) 2019 China Core Co. Ltd.
                      All Rights Reserved
  ******************************************************************************
  * @file    tsi_app.h
  * @author  System Application Team
  * @version V4.0.0
  * @date    2022.03.22
  * @brief   Header file of Tsi application.
  * 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TSI_APP_H
#define __TSI_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "tsi_hal.h"
#include "tc_drv.h"
#include "reset_drv.h"
#include "cache_drv.h"
#include "uart_drv.h"
#include "eport_drv.h"
#include "i2c_drv.h"
#include "reset_drv.h"
#include "pci_drv.h"
#include "sys.h"
#include "delay.h"
#include "libTSI.h"
#include "reset_drv.h"

    // <<< Use Configuration Wizard in Context Menu >>>

    /** @addtogroup Application
  * @{
  */

    /** @addtogroup TSI
  * @{
  */

    /*** Macro define *******************************************************************/
    /** @addtogroup TSI Exported Macros
  * @{
  */

    /**
  * @}
  */

    /*** 结构体、枚举变量定义 *****************************************************/
    /** @addtogroup TSI Exported Types
  * @{
  */

    /**
  * @}
  */

    /*** 全局变量声明 **************************************************************/
    /** @addtogroup TSI Exported Variables
  * @{
  */
    extern TSI_HandleTypeDef htsi;
    /**
  * @}
  */

    /*** 函数声明 ******************************************************************/
    /** @addtogroup TSI Exported Functions
  * @{
  */
    extern void app_tsi_preprocess(TSI_HandleTypeDef *htsi);
    extern void app_tsi_updateTouch(TSI_HandleTypeDef *htsi);
    extern void app_tsi_updateBase(TSI_HandleTypeDef *htsi);
    extern void app_tsi_stop(TSI_HandleTypeDef *htsi, const e_TSI_Status_Function status, const uint8_t lossPacketCnt);
    extern uint32_t app_tsi_init(TSI_HandleTypeDef *htsi);
    extern uint32_t app_tsi_handle(TSI_HandleTypeDef *htsi);
    extern void app_tsi_sleep(TSI_HandleTypeDef *htsi);
    /**
  * @}
  */

    /**
  * @}
  */

    /**
  * @}
  */

    // <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif /* __TSI_APP_H */

/************************ (C) COPYRIGHT C*Core *****END OF FILE****/
