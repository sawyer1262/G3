/**
 * @file c0_drv.h
 * @author liang.huajun
 * @brief 
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __C0_DRV_H__
#define __C0_DRV_H__

#include <string.h>
#include <stdio.h>
#include "c0_reg.h"
#include "c0_memmap.h"
#include "cpm_drv.h"
#include "eport_drv.h"
#include "uart_drv.h"
#include "delay.h"
#include "sys.h"
#include "def.h"

#define C0_CPM ((C0_CPM_TypeDef *)(C0_CPM_BASE_ADDR))
#define C0_WDT ((WDT_TypeDef *)(C0_WDT_BASE_ADDR))
#define C0_TC ((TC_TypeDef *)C0_TC_BASE_ADDR)
#define C0_EPORT0 ((EPORT_TypeDef *)(C0_EPORT0_BASE_ADDR))
#define C0_EPORT1 ((EPORT_TypeDef *)(C0_EPORT1_BASE_ADDR))

// <<< Use Configuration Wizard in Context Menu >>>

// <o> C0 wakeup and status save as...
#ifndef C0SRAM_ADD_BUFFER
#define C0SRAM_ADD_BUFFER 0x08100F80
#endif

// <o> config C0 gin0 function
#ifndef C0_GINT0_FUNCTION
#define C0_GINT0_FUNCTION 84
#endif
// <0x50=> SUBC0_IO_INPUT
// <0x51=> SUBC0_IO_OUTPUT_HIGH
// <0x52=> SUBC0_IO_OUTPUT_LOW
// <0x53=> SUBC0_IO_INT_RISING_EDGE
// <0x54=> SUBC0_IO_INT_FALLING_EDGE
// <0x55=> SUBC0_IO_INT_RISING_FALLING_EDGE

// <o> config C0 gin1 function
#ifndef C0_GINT1_FUNCTION
#define C0_GINT1_FUNCTION 84
#endif
// <0x50=> SUBC0_IO_INPUT
// <0x51=> SUBC0_IO_OUTPUT_HIGH
// <0x52=> SUBC0_IO_OUTPUT_LOW
// <0x53=> SUBC0_IO_INT_RISING_EDGE
// <0x54=> SUBC0_IO_INT_FALLING_EDGE
// <0x55=> SUBC0_IO_INT_RISING_FALLING_EDGE

// <o> config C0 gin4 function
#ifndef C0_GINT4_FUNCTION
#define C0_GINT4_FUNCTION 84
#endif
// <0x50=> SUBC0_IO_INPUT
// <0x51=> SUBC0_IO_OUTPUT_HIGH
// <0x52=> SUBC0_IO_OUTPUT_LOW
// <0x53=> SUBC0_IO_INT_RISING_EDGE
// <0x54=> SUBC0_IO_INT_FALLING_EDGE
// <0x55=> SUBC0_IO_INT_RISING_FALLING_EDGE

// <o> config C0 gin5 function
#ifndef C0_GINT5_FUNCTION
#define C0_GINT5_FUNCTION 84
#endif
// <0x50=> SUBC0_IO_INPUT
// <0x51=> SUBC0_IO_OUTPUT_HIGH
// <0x52=> SUBC0_IO_OUTPUT_LOW
// <0x53=> SUBC0_IO_INT_RISING_EDGE
// <0x54=> SUBC0_IO_INT_FALLING_EDGE
// <0x55=> SUBC0_IO_INT_RISING_FALLING_EDGE

// <o> config C0 wakeup0 function
#ifndef C0_WAKEUP0_FUNCTION
#define C0_WAKEUP0_FUNCTION 80
#endif
// <0x50=> SUBC0_IO_INPUT
// <0x51=> SUBC0_IO_OUTPUT_HIGH
// <0x52=> SUBC0_IO_OUTPUT_LOW

// <o> config C0 wakeup1 function
#ifndef C0_WAKEUP1_FUNCTION
#define C0_WAKEUP1_FUNCTION 80
#endif
// <0x50=> SUBC0_IO_INPUT
// <0x51=> SUBC0_IO_OUTPUT_HIGH
// <0x52=> SUBC0_IO_OUTPUT_LOW

// <o> config C0 gint0 pull
#ifndef C0_GINT0_PULL
#define C0_GINT0_PULL 64
#endif
// <0x40=> SUBC0_IO_PULLUP
// <0x41=> SUBC0_IO_PULLDOWN
// <0x42=> SUBC0_IO_DISPULL

// <o> config C0 gint1 pull
#ifndef C0_GINT1_PULL
#define C0_GINT1_PULL 64
#endif
// <0x40=> SUBC0_IO_PULLUP
// <0x41=> SUBC0_IO_PULLDOWN
// <0x42=> SUBC0_IO_DISPULL

// <o> config C0 gint4 pull
#ifndef C0_GINT4_PULL
#define C0_GINT4_PULL 64
#endif
// <0x40=> SUBC0_IO_PULLUP
// <0x41=> SUBC0_IO_PULLDOWN
// <0x42=> SUBC0_IO_DISPULL

// <o> config C0 gint5 pull
#ifndef C0_GINT5_PULL
#define C0_GINT5_PULL 64
#endif
// <0x40=> SUBC0_IO_PULLUP
// <0x41=> SUBC0_IO_PULLDOWN
// <0x42=> SUBC0_IO_DISPULL

// <o> config C0 wakeup0 pull
#ifndef C0_WAKEUP0_PULL
#define C0_WAKEUP0_PULL 64
#endif
// <0x40=> SUBC0_IO_PULLUP
// <0x41=> SUBC0_IO_PULLDOWN
// <0x42=> SUBC0_IO_DISPULL

// <o> config C0 wakeup1 pull
#ifndef C0_WAKEUP1_PULL
#define C0_WAKEUP1_PULL 64
#endif
// <0x40=> SUBC0_IO_PULLUP
// <0x41=> SUBC0_IO_PULLDOWN
// <0x42=> SUBC0_IO_DISPULL

// <<< end of configuration section >>>

typedef enum
{
    SUBC0_GINT0 = 0,
    SUBC0_GINT1,
    SUBC0_GINT2,
    SUBC0_GINT3,
    SUBC0_GINT4,
    SUBC0_GINT5,
    SUBC0_GINT6,
    SUBC0_GINT7,
    SUBC0_GINT8,
    SUBC0_GINT9,
    SUBC0_GINT10,
    SUBC0_GINT11,
    SUBC0_GINT12,
    SUBC0_GINT13,
    SUBC0_GINT14,
    SUBC0_GINT15,
    SUBC0_WAKEUP0,
    SUBC0_WAKEUP1
} SubC0_PINx;

typedef enum
{
    SUBC0_DISPULL = 0,
    SUBC0_PULLUP = 1,
    SUBC0_PULLDOWN = 2
} eSubC0_PULL;

typedef enum
{
    SUBCO_NOINIT = 0x132B0000, /*!< C0没有初始化完成  */
    SUBCO_READY = 0x162B0000,  /*!< C0已经初始化完成 */
    SUBCO_SLEEP = 0x1C2B0000   /*!< C0进入休眠 */
} e_SubC0_Mask;

typedef enum
{
    SUBC0_IO_INPUT = 0x50,            /*!< C0 IO功能配置为Input */
    SUBC0_IO_OUTPUT_HIGH,             /*!< C0 IO功能配置为output高 */
    SUBC0_IO_OUTPUT_LOW,              /*!< C0 IO功能配置为Output低 */
    SUBC0_IO_INT_RISING_EDGE,         /*!< C0 IO功能配置为上升沿触发 */
    SUBC0_IO_INT_FALLING_EDGE,        /*!< C0 IO功能配置为下降沿触发 */
    SUBC0_IO_INT_RISING_FALLING_EDGE, /*!< C0 IO功能配置为双边沿触发 */
    SUBC0_IO_INT_HIGH_LEVEl,          /*!< C0 IO功能配置为高电平触发 */
    SUBC0_IO_INT_LOW_LEVEl,           /*!< C0 IO功能配置为低电平触发 */
    SUBC0_IO_HIZ                      /*!< C0 IO功能配置为高阻态 */
} e_SubC0_IOFuction;

typedef enum
{
    SUBC0_IO_PULLUP = 0x40, /*!< C0 IO 配置为上拉 */
    SUBC0_IO_PULLDOWN,      /*!< C0 IO 配置为下拉 */
    SUBC0_IO_DISPULL        /*!< C0 IO disable上下拉 */
} e_SubC0_IOPull;

typedef struct
{
    uint8_t Function; /*!< 配置C0 IO功能  */
    uint8_t Pull;     /*!< 配置C0 IO上下拉 */
    uint8_t IE;       /*!< C0 IO中断触发标志位 */
} SubC0_IOTypeDef;

typedef struct
{
    uint32_t Mask; /*!< 判断load C0是否已经正常运行  */
    uint8_t Lock;  /*!< 裁定当前C0 SRAM是否被其他程序占用 */
    SubC0_IOTypeDef gint0;
    SubC0_IOTypeDef gint1;
    SubC0_IOTypeDef gint4;
    SubC0_IOTypeDef gint5;
    SubC0_IOTypeDef wakeup0;
    SubC0_IOTypeDef wakeup1;
} SubC0_HandleTypeDef;

extern uint32_t drv_SubC0_sendM2S(void);
extern uint32_t drv_SubC0_callback(void);
extern void c0_read4KSram(void);
extern void c0_readStruct(const uint8_t *buf, const uint16_t size);
extern void drv_SubC0_loadBin(uint8_t *bin_data, int size);
extern uint32_t drv_SubC0_waitReady(uint16_t cnt);
extern void drv_SubC0_ConfigGpio(SubC0_PINx GpioNo, uint8_t GpioDir);
extern void drv_SubC0_WriteGpioData(SubC0_PINx GpioNo, uint8_t bitVal);
extern uint8_t drv_SubC0_ReadGpioData(SubC0_PINx GpioNo);
extern void drv_SubC0_ITTypeConfig(SubC0_PINx GpioNo, uint8_t IntMode);
extern void drv_SubC0_ITConfig(SubC0_PINx GpioNo, FunctionalState NewState);
extern uint32_t drv_SubC0_ClearINTFlag(void);
extern uint32_t drv_Subc0_configIOPull(SubC0_PINx GpioNo, eSubC0_PULL Pull);
extern uint32_t drv_SubC0_SoftPOR(void);
extern uint32_t drv_SubC0_initSys(void);
extern uint32_t drv_SubC0_initStruct(SubC0_HandleTypeDef *hSubC0);
extern uint32_t drv_SubC0_init(SubC0_HandleTypeDef *hSubC0);
extern uint32_t drv_SubC0_sleep(void);
extern uint32_t drv_SubC0_wakeup(void);
extern uint32_t drv_SubC0_reset(void);
extern uint32_t drv_SubC0_getWakeupSource(volatile uint32_t *pu32_wakeupSource);

extern SubC0_HandleTypeDef *hSubC0;

#endif /* __C0_DRV_H__ */
