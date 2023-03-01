/**
	******************************************************************************
             Copyright(c) 2019 China Core Co. Ltd.
                      All Rights Reserved
  ******************************************************************************
  * @file    libTSI.h
  * @author  System Application Team
  * @version V1.3
  * @date    2021.01.12
  * @brief   Header file of Software-Algotithm application.
  * 1. o2优化
  * 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIBTSI_H
#define __LIBTSI_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

    /** @addtogroup Hal
  * @{
  */

    /** @addtogroup Software-Algotithm
  * @{
  */

    /*** 宏定义 *******************************************************************/
    /** @addtogroup Software-Algotithm Exported Macros
  * @{
  */

    /**
  * @}
  */

    /*** 结构体、枚举变量定义 *****************************************************/
    /** @addtogroup Software-Algotithm Exported Types
  * @{
  */

    /**
  * @}
  */

    /*** 全局变量声明 **************************************************************/
    /** @addtogroup Software-Algotithm Exported Variables
  * @{
  */

    /**
  * @}
  */

    /*** 函数声明 ******************************************************************/
    /** @addtogroup Software-Algotithm Exported Functions
  * @{
  */
    extern float hal_algorithm_sum(uint8_t *data_in, const uint16_t len);
    extern float hal_algorithm_average(uint8_t *data_in, const uint16_t len);
    extern void hal_algorithm_algMeanRoll(uint8_t *data_in, const uint16_t len, uint8_t *data_out, const uint16_t size_window);
    extern void hal_algorithm_variance(uint8_t *data_in, const uint16_t len, float *data_out, const uint8_t adjustment_para);
    extern void hal_algorithm_exp(uint8_t *data_in, const uint16_t len, float *data_out, float *base, const float preposition_para);
    extern void hal_algorithm_filter_highpass(volatile uint8_t *in_buf, uint16_t buf_len, uint8_t base, uint8_t threshold);
    extern void hal_algorithm_filter_lowpass(volatile uint8_t *in_buf, uint16_t buf_len, uint8_t default_val, uint8_t threshold);
    extern void hal_algorithm_filter_NormalDistribution(volatile uint8_t *in_buf, const uint16_t buf_len, volatile uint8_t *out_buf, const uint8_t percentage);

    /**
  * @}
  */

    /**
  * @}
  */

    /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __LIBTSI_H */

/************************ (C) COPYRIGHT C*Core *****END OF FILE****/
