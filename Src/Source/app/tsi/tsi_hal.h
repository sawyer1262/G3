/**
  ******************************************************************************
             Copyright(c) 2019 China Core Co. Ltd.
                      All Rights Reserved
  ******************************************************************************
  * @file    tsi_hal.h
  * @author  System Application Team
  * @version V4.0.0
  * @date    2022.03.22
  * @brief   Header file of TSI HAL module.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TSI_HAL_H
#define __TSI_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "tsi_drv.h"
#include "cpm_drv.h"
#include "stdio.h "
  // <<< Use Configuration Wizard in Context Menu >>>

  /** @addtogroup HAL Driver
   * @{
   */

  /** @addtogroup TSI
   * @{
   */

  /*** 宏定义 *******************************************************************/
  /** @addtogroup TSI Exported Macros
   * @{
   */

// <h> enable channels
// <q> TSI_CH0_ENABLE
#ifndef TSI_CH0_ENABLE
#define TSI_CH0_ENABLE 1
#endif
// <q> TSI_CH1_ENABLE
#ifndef TSI_CH1_ENABLE
#define TSI_CH1_ENABLE 1
#endif
// <q> TSI_CH2_ENABLE
#ifndef TSI_CH2_ENABLE
#define TSI_CH2_ENABLE 1
#endif
// <q> TSI_CH3_ENABLE
#ifndef TSI_CH3_ENABLE
#define TSI_CH3_ENABLE 1
#endif
// <q> TSI_CH4_ENABLE
#ifndef TSI_CH4_ENABLE
#define TSI_CH4_ENABLE 1
#endif
// <q> TSI_CH5_ENABLE
#ifndef TSI_CH5_ENABLE
#define TSI_CH5_ENABLE 1
#endif
// <q> TSI_CH6_ENABLE
#ifndef TSI_CH6_ENABLE
#define TSI_CH6_ENABLE 1
#endif
// <q> TSI_CH7_ENABLE
#ifndef TSI_CH7_ENABLE
#define TSI_CH7_ENABLE 1
#endif
// <q> TSI_CH8_ENABLE
#ifndef TSI_CH8_ENABLE
#define TSI_CH8_ENABLE 1
#endif
// <q> TSI_CH9_ENABLE
#ifndef TSI_CH9_ENABLE
#define TSI_CH9_ENABLE 1
#endif
// <q> TSI_CH10_ENABLE
#ifndef TSI_CH10_ENABLE
#define TSI_CH10_ENABLE 1
#endif
// <q> TSI_CH11_ENABLE
#ifndef TSI_CH11_ENABLE
#define TSI_CH11_ENABLE 1
#endif
// <q> TSI_CH12_ENABLE
#ifndef TSI_CH12_ENABLE
#define TSI_CH12_ENABLE 1
#endif
// <q> TSI_CH13_ENABLE
#ifndef TSI_CH13_ENABLE
#define TSI_CH13_ENABLE 0
#endif
// <q> TSI_CH14_ENABLE
#ifndef TSI_CH14_ENABLE
#define TSI_CH14_ENABLE 0
#endif
// <q> TSI_CH15_ENABLE
#ifndef TSI_CH15_ENABLE
#define TSI_CH15_ENABLE 0
#endif
// </h>

// <h> set Initial
// <o> initial cbase is
#define TSI_INIT_CBASEIS 0
// <o> initial coarse is
#define TSI_INIT_COARSEIS 0
// <h> set TSI effective area1
// <o> initial comfort zone of fine1, upper limit
#define TSI_INIT_FINE1_UPPER 0x50
// <o> initial comfort zone of fine1, low limit
#define TSI_INIT_FINE1_LOW 0x08
// </h>
// <h> set TSI effective area2
// <o> initial comfort zone of fine2, upper limit
#define TSI_INIT_FINE2_UPPER 0xD8
// <o> initial comfort zone of fine2, low limit
#define TSI_INIT_FINE2_LOW 0x90
// </h>
// <o> coarse dynamic adjust stepping
#define TSI_COARSE_STEP_DYNAMICA_DJUST 8
// </h>

// <h> set Lowpower
// <o> float up delta register
#ifndef TSI_LP_DELTA_REG_INCREASE
#define TSI_LP_DELTA_REG_INCREASE 1
#endif
// </h>

// <h> set Baseline
// <o> Baseline cache length
#ifndef BASELINE_CAHCE_LEN
#define BASELINE_CAHCE_LEN 3
#endif
// <o> Baseline default
#ifndef BASELINE_DEFAULT_VAL
#define BASELINE_DEFAULT_VAL 255
#endif
// <o> Variance Threshold
#ifndef BASELINE_VARIANCE_TH
#define BASELINE_VARIANCE_TH 1
#endif
// <o> Baseline Approach Scale factor
#ifndef BASELINE_APPROACH_SCALE
#define BASELINE_APPROACH_SCALE 4
#endif
// </h>

#define  TOUCH_WITH_EV

#ifdef   TOUCH_WITH_EV

#define         MY_THIN_VAL      5


// <h> set Touch
// <h> Button threshold
// <i> if tsi_ch0_fine > Baseline + TSI_CHx_THIN_VAL:
// <i>    then tsi_chx be touched
// <o> TSI_CH0_THIN_VAL
#ifndef TSI_CH0_THIN_VAL
#define TSI_CH0_THIN_VAL (MY_THIN_VAL)                          //c
#endif
// <o> TSI_CH1_THIN_VAL
#ifndef TSI_CH1_THIN_VAL
#define TSI_CH1_THIN_VAL (MY_THIN_VAL)                          //5
#endif
// <o> TSI_CH2_THIN_VAL
#ifndef TSI_CH2_THIN_VAL
#define TSI_CH2_THIN_VAL (MY_THIN_VAL)                          //2 
#endif
// <o> TSI_CH3_THIN_VAL
#ifndef TSI_CH3_THIN_VAL
#define TSI_CH3_THIN_VAL (MY_THIN_VAL)                          //6 
#endif
// <o> TSI_CH4_THIN_VAL
#ifndef TSI_CH4_THIN_VAL
#define TSI_CH4_THIN_VAL (MY_THIN_VAL)                        //3
#endif
// <o> TSI_CH5_THIN_VAL
#ifndef TSI_CH5_THIN_VAL
#define TSI_CH5_THIN_VAL (MY_THIN_VAL)                          //1 
#endif
// <o> TSI_CH6_THIN_VAL
#ifndef TSI_CH6_THIN_VAL
#define TSI_CH6_THIN_VAL (MY_THIN_VAL)                          //4
#endif
// <o> TSI_CH7_THIN_VAL
#ifndef TSI_CH7_THIN_VAL
#define TSI_CH7_THIN_VAL (MY_THIN_VAL)                          //*
#endif
// <o> TSI_CH8_THIN_VAL
#ifndef TSI_CH8_THIN_VAL
#define TSI_CH8_THIN_VAL (MY_THIN_VAL)                          //7
#endif
// <o> TSI_CH9_THIN_VAL
#ifndef TSI_CH9_THIN_VAL
#define TSI_CH9_THIN_VAL (MY_THIN_VAL)                          //bell
#endif
// <o> TSI_CH10_THIN_VAL
#ifndef TSI_CH10_THIN_VAL
#define TSI_CH10_THIN_VAL  (MY_THIN_VAL)                        //8
#endif
// <o> TSI_CH11_THIN_VAL
#ifndef TSI_CH11_THIN_VAL
#define TSI_CH11_THIN_VAL (MY_THIN_VAL)                         //9
#endif
// <o> TSI_CH12_THIN_VAL
#ifndef TSI_CH12_THIN_VAL
#define TSI_CH12_THIN_VAL (MY_THIN_VAL)                         //0
#endif
// <o> TSI_CH13_THIN_VAL
#ifndef TSI_CH13_THIN_VAL
#define TSI_CH13_THIN_VAL MY_THIN_VAL
#endif
// <o> TSI_CH14_THIN_VAL
#ifndef TSI_CH14_THIN_VAL
#define TSI_CH14_THIN_VAL MY_THIN_VAL
#endif
// <o> TSI_CH15_THIN_VAL
#ifndef TSI_CH15_THIN_VAL
#define TSI_CH15_THIN_VAL MY_THIN_VAL
#endif

#else

#define         MY_THIN_VAL      10


// <h> set Touch
// <h> Button threshold
// <i> if tsi_ch0_fine > Baseline + TSI_CHx_THIN_VAL:
// <i>    then tsi_chx be touched
// <o> TSI_CH0_THIN_VAL
#ifndef TSI_CH0_THIN_VAL
#define TSI_CH0_THIN_VAL (MY_THIN_VAL-3)                          //8
#endif
// <o> TSI_CH1_THIN_VAL
#ifndef TSI_CH1_THIN_VAL
#define TSI_CH1_THIN_VAL (MY_THIN_VAL-1)                          //5
#endif
// <o> TSI_CH2_THIN_VAL
#ifndef TSI_CH2_THIN_VAL
#define TSI_CH2_THIN_VAL (MY_THIN_VAL-1)                          //2 
#endif
// <o> TSI_CH3_THIN_VAL
#ifndef TSI_CH3_THIN_VAL
#define TSI_CH3_THIN_VAL (MY_THIN_VAL+1)                          //6 
#endif
// <o> TSI_CH4_THIN_VAL
#ifndef TSI_CH4_THIN_VAL
#define TSI_CH4_THIN_VAL (MY_THIN_VAL+1)                          //3
#endif
// <o> TSI_CH5_THIN_VAL
#ifndef TSI_CH5_THIN_VAL
#define TSI_CH5_THIN_VAL (MY_THIN_VAL)                          //1 
#endif
// <o> TSI_CH6_THIN_VAL
#ifndef TSI_CH6_THIN_VAL
#define TSI_CH6_THIN_VAL (MY_THIN_VAL-1)                          //4
#endif
// <o> TSI_CH7_THIN_VAL
#ifndef TSI_CH7_THIN_VAL
#define TSI_CH7_THIN_VAL (MY_THIN_VAL-1)                          //*
#endif
// <o> TSI_CH8_THIN_VAL
#ifndef TSI_CH8_THIN_VAL
#define TSI_CH8_THIN_VAL (MY_THIN_VAL-2)                          //7
#endif
// <o> TSI_CH9_THIN_VAL
#ifndef TSI_CH9_THIN_VAL
#define TSI_CH9_THIN_VAL (MY_THIN_VAL-1)                          //bell
#endif
// <o> TSI_CH10_THIN_VAL
#ifndef TSI_CH10_THIN_VAL
#define TSI_CH10_THIN_VAL  (MY_THIN_VAL)                         //C
#endif
// <o> TSI_CH11_THIN_VAL
#ifndef TSI_CH11_THIN_VAL
#define TSI_CH11_THIN_VAL (MY_THIN_VAL-1)                         //9
#endif
// <o> TSI_CH12_THIN_VAL
#ifndef TSI_CH12_THIN_VAL
#define TSI_CH12_THIN_VAL (MY_THIN_VAL-3)                         //0
#endif
// <o> TSI_CH13_THIN_VAL
#ifndef TSI_CH13_THIN_VAL
#define TSI_CH13_THIN_VAL MY_THIN_VAL
#endif
// <o> TSI_CH14_THIN_VAL
#ifndef TSI_CH14_THIN_VAL
#define TSI_CH14_THIN_VAL MY_THIN_VAL
#endif
// <o> TSI_CH15_THIN_VAL
#ifndef TSI_CH15_THIN_VAL
#define TSI_CH15_THIN_VAL MY_THIN_VAL
#endif

#endif
// </h>
// </h>


// <h> set Filter
 //<h> SOFTWARE_FILTER
// <o> LOOP_SCAN_NUM
#ifndef LOOP_SCAN_NUM // 每次循环扫描的次数
#define LOOP_SCAN_NUM 11
#endif

// <o> CACHE_DATA_NUM
// <i> cache for fine buffers. Ex. 16 * 4 * LOOP_SCAN_NUM * CACHE_DATA_NUM
#ifndef CACHE_DATA_NUM
#define CACHE_DATA_NUM 3
#endif

// <o> Normal-Distribution filtering percent
#ifndef FILTER_NORMALDISTRI_PER
#define FILTER_NORMALDISTRI_PER 5
#endif
// <h> Rolling-Mean filtering
// <o> Windows size
#ifndef MEAN_WINDOWS_SIZE
#define MEAN_WINDOWS_SIZE 8
#endif
  // </h>
// </h>

  //<h> PRE_HARDWARE_FILTER
// <o>Set PRE_HARDWARE_FILTER
//  <i>Default: NO_FILTER
//  <0=> NO_FILTER
//  <1=> IIR_FILTER
//  <2=> MEDIAN_FILTER
//  <3=> MEAN_FILTER
//  <4=> IIR_MEDIAN_FILTER
//  <5=> IIR_MEAN_FILTER
//  <6=> MEDIAN_MEAN_FILTER
//  <7=> IIR_MEDIAN_MEAN_FILTER
#define PRE_HARDWARE_FILTER 0

//  <o> IIR_FILTER_N <0x0-0xFF>
#ifndef IIR_FILTER_N
#define IIR_FILTER_N 0x80
#endif

// <o>Set MEAN_FILTER_WIDTH
//  <i>Default: MEAN_FILTER_WIDTH_4
//  <0=> MEAN_FILTER_WIDTH_4
//  <1=> MEAN_FILTER_WIDTH_8
//  <2=> MEAN_FILTER_WIDTH_16
//  <3=> MEAN_FILTER_WIDTH_32
#define MEAN_FILTER_WIDTH 1


  // </h>
// </h>
  // <h> set printf log
  // <q> TSI_PRINTF_ENABLE
#ifndef TSI_PRINTF_ENABLE
#define TSI_PRINTF_ENABLE 0
#endif

// <o>PRINT TSI_REAL_TIME_DATA_CURVE
//  <i>Default: NO_FILTER
//  <0=> PRINT_NONE
//  <1=> PRINT_CHANNEL_0_7
//  <2=> PRINT_CHANNEL_8_15
#define TSI_REAL_TIME_DATA_CURVE 0

  // </h>
// <h> set TSI-Hal function
// <h> Set Backup-data Adress
// <h> Algorithm Base
// <o> Channle0
#ifndef TSI_BACKUP_ALGBASE_CH0
#define TSI_BACKUP_ALGBASE_CH0 0x400230D0
#endif
// <o> Channle1
#ifndef TSI_BACKUP_ALGBASE_CH1
#define TSI_BACKUP_ALGBASE_CH1 0x400230D1
#endif
// <o> Channle2
#ifndef TSI_BACKUP_ALGBASE_CH2
#define TSI_BACKUP_ALGBASE_CH2 0x400230D2
#endif
// <o> Channle3
#ifndef TSI_BACKUP_ALGBASE_CH3
#define TSI_BACKUP_ALGBASE_CH3 0x400230D3
#endif
// <o> Channle4
#ifndef TSI_BACKUP_ALGBASE_CH4
#define TSI_BACKUP_ALGBASE_CH4 0x400230D4
#endif
// <o> Channle5
#ifndef TSI_BACKUP_ALGBASE_CH5
#define TSI_BACKUP_ALGBASE_CH5 0x400230D5
#endif
// <o> Channle6
#ifndef TSI_BACKUP_ALGBASE_CH6
#define TSI_BACKUP_ALGBASE_CH6 0x400230D6
#endif
// <o> Channle7
#ifndef TSI_BACKUP_ALGBASE_CH7
#define TSI_BACKUP_ALGBASE_CH7 0x400230D7
#endif
// <o> Channle8
#ifndef TSI_BACKUP_ALGBASE_CH8
#define TSI_BACKUP_ALGBASE_CH8 0x400230D8
#endif
// <o> Channle9
#ifndef TSI_BACKUP_ALGBASE_CH9
#define TSI_BACKUP_ALGBASE_CH9 0x400230D9
#endif
// <o> Channle10
#ifndef TSI_BACKUP_ALGBASE_CH10
#define TSI_BACKUP_ALGBASE_CH10 0x400230DA
#endif
// <o> Channle11
#ifndef TSI_BACKUP_ALGBASE_CH11
#define TSI_BACKUP_ALGBASE_CH11 0x400230DB
#endif
// <o> Channle12
#ifndef TSI_BACKUP_ALGBASE_CH12
#define TSI_BACKUP_ALGBASE_CH12 0x400230DC
#endif
// <o> Channle13
#ifndef TSI_BACKUP_ALGBASE_CH13
#define TSI_BACKUP_ALGBASE_CH13 0x400230DD
#endif
// <o> Channle14
#ifndef TSI_BACKUP_ALGBASE_CH14
#define TSI_BACKUP_ALGBASE_CH14 0x400230DE
#endif
// <o> Channle15
#ifndef TSI_BACKUP_ALGBASE_CH15
#define TSI_BACKUP_ALGBASE_CH15 0x400230DF
#endif
// </h>
// <h> Trend
// <o> Channle0
#ifndef TSI_BACKUP_TREND_CH0
#define TSI_BACKUP_TREND_CH0 0x400230E0
#endif
// <o> Channle1
#ifndef TSI_BACKUP_TREND_CH1
#define TSI_BACKUP_TREND_CH1 0x400230E1
#endif
// <o> Channle2
#ifndef TSI_BACKUP_TREND_CH2
#define TSI_BACKUP_TREND_CH2 0x400230E2
#endif
// <o> Channle3
#ifndef TSI_BACKUP_TREND_CH3
#define TSI_BACKUP_TREND_CH3 0x400230E3
#endif
// <o> Channle4
#ifndef TSI_BACKUP_TREND_CH4
#define TSI_BACKUP_TREND_CH4 0x400230E4
#endif
// <o> Channle5
#ifndef TSI_BACKUP_TREND_CH5
#define TSI_BACKUP_TREND_CH5 0x400230E5
#endif
// <o> Channle6
#ifndef TSI_BACKUP_TREND_CH6
#define TSI_BACKUP_TREND_CH6 0x400230E6
#endif
// <o> Channle7
#ifndef TSI_BACKUP_TREND_CH7
#define TSI_BACKUP_TREND_CH7 0x400230E7
#endif
// <o> Channle8
#ifndef TSI_BACKUP_TREND_CH8
#define TSI_BACKUP_TREND_CH8 0x400230E8
#endif
// <o> Channle9
#ifndef TSI_BACKUP_TREND_CH9
#define TSI_BACKUP_TREND_CH9 0x400230E9
#endif
// <o> Channle10
#ifndef TSI_BACKUP_TREND_CH10
#define TSI_BACKUP_TREND_CH10 0x400230EA
#endif
// <o> Channle11
#ifndef TSI_BACKUP_TREND_CH11
#define TSI_BACKUP_TREND_CH11 0x400230EB
#endif
// <o> Channle12
#ifndef TSI_BACKUP_TREND_CH12
#define TSI_BACKUP_TREND_CH12 0x400230EC
#endif
// <o> Channle13
#ifndef TSI_BACKUP_TREND_CH13
#define TSI_BACKUP_TREND_CH13 0x400230ED
#endif
// <o> Channle14
#ifndef TSI_BACKUP_TREND_CH14
#define TSI_BACKUP_TREND_CH14 0x400230EE
#endif
// <o> Channle15
#ifndef TSI_BACKUP_TREND_CH15
#define TSI_BACKUP_TREND_CH15 0x400230EF
#endif

  // </h>
  // </h>
  // </h>


  /**
   * @}
   */

  /*** 结构体、枚举变量定义 *****************************************************/
  /** @addtogroup TSI Exported Types
   * @{
   */
  typedef enum
  {
    CH0 = 0,
    CH1 = 1,
    CH2 = 2,
    CH3 = 3,
    CH4 = 4,
    CH5 = 5,
    CH6 = 6,
    CH7 = 7,
    CH8 = 8,
    CH9 = 9,
    CH10 = 10,
    CH11 = 11,
    CH12 = 12,
    CH13 = 13,
    CH14 = 14,
    CH15 = 15,
    TSI_MULTI_VALID,
    TSI_NONE
  } e_TSI_Status;

  typedef enum
  {
    TSI_FUCTION_NONE = 0,
    TSI_FUCTION_STOP, /*!< TSI立即停止收发数据 */
    TSI_FUCTION_WAIT, /*!< TSI延后收发数据 */
    TSI_FUCTION_RUN,
    TSI_FUCTION_SLEEP
  } e_TSI_Status_Function;

  typedef enum
  {
    TSI_CBASE_DECREASE = 0, /*!< Cbase-- */
    TSI_CBASE_INCREASE,     /*!< Cbase++ */
    TSI_CBASE_OUTRANGE,     /*!< Cbase&Coarse out of range */
    TSI_CBASE_NONE          /*!< Cbase not need change */
  } e_TSI_CBASE_STATUS;

  typedef enum
  {
    TSI_INIT_NONE = 0, /*!< TSI Cbase&Coarse 初始化 */
    TSI_INIT_ADJUST,   /*!< TSI Cbase&Coarse 调整中 */
    TSI_INIT_FINISHED  /*!< TSI Cbase&Coarse 调整完毕 */
  } e_TSI_InitStatus;

  typedef enum
  {
    TSI_PRE_HARDWARE_NO_FILTER = 0,         /*!< 不打卡硬件滤波器 */
    TSI_PRE_HARDWARE_IIR_FILTER,            /*!< 打开IIR滤波器 */
    TSI_PRE_HARDWARE_MEDIAN_FILTER,         /*!< 打开中值滤波器 */
    TSI_PRE_HARDWARE_MEAN_FILTER,           /*!< 打开均值滤波器 */
    TSI_PRE_HARDWARE_IIR_MEDIAN_FILTER,     /*!< 打开IIR与中值滤波器 */
    TSI_PRE_HARDWARE_IIR_MEAN_FILTER,       /*!< 打开IIR与均值滤波器 */
    TSI_PRE_HARDWARE_MEDIAN_MEAN_FILTER,    /*!< 打开中值与均值滤波器 */
    TSI_PRE_HARDWARE_IIR_MEDIAN_MEAN_FILTER /*!< 打开IIR，中值，均值滤波器 */
  } e_TSI_Hardware_Filter;

  typedef struct
  {
    e_TSI_Hardware_Filter Hardware_filter;
    uint8_t IIR_filter_N;
    uint8_t mean_filter_width;
  } TSI_Filter_TypeDef;

  typedef struct
  {
    uint8_t ch0;
    uint8_t ch1;
    uint8_t ch2;
    uint8_t ch3;
    uint8_t ch4;
    uint8_t ch5;
    uint8_t ch6;
    uint8_t ch7;
    uint8_t ch8;
    uint8_t ch9;
    uint8_t ch10;
    uint8_t ch11;
    uint8_t ch12;
    uint8_t ch13;
    uint8_t ch14;
    uint8_t ch15;
  } TSI_Init_CbaseTypeDef, TSI_Init_CoarseTypeDef, TSI_Init_AdjustDirectionTypeDef, TSI_Init_LowpowerDelta,
      TSI_Algorithm_Delta, TSI_Algorithm_TouchCount, TSI_Algorithm_LeaveCount, TSI_Algorithm_BaseCount,
      TSI_Touch_Continue;

  typedef struct
  {
    volatile float ch0;
    volatile float ch1;
    volatile float ch2;
    volatile float ch3;
    volatile float ch4;
    volatile float ch5;
    volatile float ch6;
    volatile float ch7;
    volatile float ch8;
    volatile float ch9;
    volatile float ch10;
    volatile float ch11;
    volatile float ch12;
    volatile float ch13;
    volatile float ch14;
    volatile float ch15;
  } TSI_Baseline_DataTypeDef,
      TSI_Algorithm_VarianceTypeDef, TSI_Algorithm_AverageTypeDef;

  typedef struct
  {
    volatile uint8_t *ch0;
    volatile uint8_t *ch1;
    volatile uint8_t *ch2;
    volatile uint8_t *ch3;
    volatile uint8_t *ch4;
    volatile uint8_t *ch5;
    volatile uint8_t *ch6;
    volatile uint8_t *ch7;
    volatile uint8_t *ch8;
    volatile uint8_t *ch9;
    volatile uint8_t *ch10;
    volatile uint8_t *ch11;
    volatile uint8_t *ch12;
    volatile uint8_t *ch13;
    volatile uint8_t *ch14;
    volatile uint8_t *ch15;
  } TSI_Backup_AlgBaseTypeDef, TSI_Backup_TrendTypeDef;

  typedef struct
  {

    e_TSI_Status_Function Status; /*!< 当前TSI运行状态 */
    uint8_t lossPacketCnt;        /*!< TSI丢包倒计时, T=4.09ms. when fun=TSI_FUCTION_WAIT */
  } TSI_FunctionTypeDef;

  typedef struct
  {
    uint8_t Ready;                                  /*!< =0,数据没有准备好,算法不可以处理 */
    volatile uint8_t Buffer[16][4 * LOOP_SCAN_NUM]; /*!< fine数据存储结构体 */
  } TSI_DataTypeDef;

  typedef struct
  {
    TSI_DataTypeDef Data[CACHE_DATA_NUM]; /*!< TSI算法处理数据单元 */
    uint8_t Index_Cache;                  /*!< 当前TSI数据缓存页号 */
    uint8_t Index_Read;                   /*!< 当前TSI数据处理页号 */
    uint8_t Offset_Cache;                 /*!< 当前TSI数据缓存单元, 缓存地址偏移量 */
  } TSI_CacheTypeDef;

  typedef struct
  {
    TSI_Algorithm_Delta Delta;                  /*!< TSI按键阈值 */
    TSI_Algorithm_AverageTypeDef AverageLast;   /*!< TSI均值记录 */
    TSI_Algorithm_VarianceTypeDef VarianceLast; /*!< TSI方差记录*/
  } TSI_AlgorithmTypeDef;

  typedef struct
  {
    e_TSI_Status Current;        /*!< 当前按键响应, 只会响应信号最强的通道 */
    e_TSI_Status Previous;       /*!< 上一次按键响应记录 */
    uint16_t TouchCurrent;       /*!< 当前所有通道按键状态记录 */
    TSI_Touch_Continue Continue; /*!< 每个通道在最近256次内的按下次数统计 */
  } TSI_TouchTypeDef;

  typedef struct
  {
    uint8_t point;                                     /*!< 当前Baseline指针 */
    TSI_Baseline_DataTypeDef Data[BASELINE_CAHCE_LEN]; /*!< Baseline 缓存 */
  } TSI_BaselineTypeDef;

  typedef struct
  {
    TSI_Backup_AlgBaseTypeDef AlgBase; /*!< 16组Baseline保存地址, 低功耗 */
    /*TSI_Backup_TrendTypeDef Trend;*/ /*!< 16组Trend保存地址, 低功耗 */
  } TSI_BackupTypeDef;

  typedef struct
  {
    TSI_TypeDef *Instance;           /*!< TSI硬件寄存器 */
    TSI_Filter_TypeDef Filter;       /*!< TSI滤波器配置 */
    TSI_FunctionTypeDef Fuction;     /*!< TSI运行状态 */
    TSI_CacheTypeDef Cache;          /*!< TSI数据缓存 */
    TSI_AlgorithmTypeDef Algorithm;  /*!< TSI算法数据缓存 */
    TSI_TouchTypeDef Touch;          /*!< TSI按键状态记录 */
    TSI_BaselineTypeDef Baseline;    /*!< TSI基准值缓存 */
    TSI_BackupTypeDef Backup;        /*!< TSI低功耗数据保存 */
    volatile uint32_t ChannlesValid; /*!< TSI通道使能 */
  } TSI_HandleTypeDef;

  typedef struct
  {
    TSI_Init_CbaseTypeDef Cbase;               /*!< 16通道Cbase缓存 */
    TSI_Init_CoarseTypeDef Coarse;             /*!< 16通道Coarse缓存 */
    TSI_Init_AdjustDirectionTypeDef Direction; /*!< Cbase&Coarse调整方向 */
    e_TSI_InitStatus Status;                   /*!< Cbase&Coarse调整函数状态记录 */
  } TSI_InitialTypeDef;
  /**
   * @}
   */

  /*** 全局变量声明 **************************************************************/
  /** @addtogroup TSI Exported Variables
   * @{
   */

  /**
   * @}
   */

  /*** 函数声明 ******************************************************************/
  /** @addtogroup TSI Exported Functions
   * @{
   */
  /* 寄存器操作相关 */
  extern void hal_tsi_reset(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_closeIEScan(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_closeIEAWD(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_clearIFScanFinish(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_clearIFAWD(TSI_HandleTypeDef *htsi);
  extern uint8_t hal_tsi_getIFScanFinish(void);
  extern uint8_t hal_tsi_getIFAWD(void);
  extern uint8_t hal_tsi_getIEScanFinish(TSI_HandleTypeDef *htsi);
  extern uint8_t hal_tsi_getIEAWD(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_configRegister(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_rememberConfiguration(TSI_HandleTypeDef *htsi, TSI_InitialTypeDef *initial);
  extern void hal_tsi_loadCbaseCoarse(TSI_HandleTypeDef *htsi, TSI_InitialTypeDef *initial);
  extern void hal_tsi_loadDelta(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_checkFine(TSI_HandleTypeDef *htsi, TSI_InitialTypeDef *initial);
  extern uint32_t hal_tsi_waitScanFinished(TSI_HandleTypeDef *htsi);
  extern uint32_t hal_tsi_clearScanFinishFlag(TSI_HandleTypeDef *htsi);
  /* tsi hal层结构体初始化, 模块寄存器初始化, 应用逻辑流程切换 */
  extern void hal_tsi_configBackupStruct(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_configInitStruct(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_switchFineMode(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_switchLowpower(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_switchInterrupt(TSI_HandleTypeDef *htsi);
  extern void hal_tsi_adjustRegister(TSI_HandleTypeDef *htsi);
  /* tsi hal算法相关 */
  extern inline uint8_t hal_tsi_getChannleValid(TSI_HandleTypeDef *htsi, const uint8_t channel);
  extern void hal_tsi_transferFineData(TSI_HandleTypeDef *htsi);

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

#endif /* __TSI_HAL_H */

/************************ (C) COPYRIGHT C*Core *****END OF FILE****/
