// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : cpm_drv.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef __CPM_DRV_H__
#define __CPM_DRV_H__

#include "type.h"
#include "memmap.h"
#include "cpm_reg.h"
#include "eport_reg.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h> config cpm_handleWakeup()

// <o> remember wakeup source to register
// <i> CPM->CPM_MRMCR
#ifndef CPM_WAKEUPSOURCE_REG
#define CPM_WAKEUPSOURCE_REG 0x400040A4
#endif
// <q> enable WDT reset check
#ifndef CPM_HANDLEWAKEUP_WDT
#define CPM_HANDLEWAKEUP_WDT 1
#endif
// <q> enable log for WDT event
// <i> log to NVSRANM(0x4003416C~0x4003416F), trim value = 0xaa55aa55
// <i> @pre pci, tc
#ifndef CPM_HANDLEWAKEUP_WDT_LOG
#define CPM_HANDLEWAKEUP_WDT_LOG 0
#endif
// <q> enable Gint0 wakeup check
#ifndef CPM_HANDLEWAKEUP_GINT0
#define CPM_HANDLEWAKEUP_GINT0 0
#endif
// <q> enable Gint1 wakeup check
#ifndef CPM_HANDLEWAKEUP_GINT1
#define CPM_HANDLEWAKEUP_GINT1 0
#endif
// <q> enable Gint2 wakeup check
#ifndef CPM_HANDLEWAKEUP_GINT2
#define CPM_HANDLEWAKEUP_GINT2 0
#endif
// <q> enable Gint3 wakeup check
#ifndef CPM_HANDLEWAKEUP_GINT3
#define CPM_HANDLEWAKEUP_GINT3 1
#endif
// <q> enable Gint4 wakeup check
#ifndef CPM_HANDLEWAKEUP_GINT4
#define CPM_HANDLEWAKEUP_GINT4 1
#endif
// <q> enable Gint5 wakeup check
#ifndef CPM_HANDLEWAKEUP_GINT5
#define CPM_HANDLEWAKEUP_GINT5 1
#endif
// <q> enable SubC0 Gint0 wakeup check
#ifndef CPM_HANDLEWAKEUP_C0GINT0
#define CPM_HANDLEWAKEUP_C0GINT0 0
#endif
// <q> enable SubC0 Gint1 wakeup check
#ifndef CPM_HANDLEWAKEUP_C0GINT1
#define CPM_HANDLEWAKEUP_C0GINT1 0
#endif
// <q> enable SubC0 Gint4 wakeup check
#ifndef CPM_HANDLEWAKEUP_C0GINT4
#define CPM_HANDLEWAKEUP_C0GINT4 0
#endif
// <q> enable SubC0 Gint5 wakeup check
#ifndef CPM_HANDLEWAKEUP_C0GINT5
#define CPM_HANDLEWAKEUP_C0GINT5 0
#endif
// <q> enable ASYCTIMER wakeup check
#ifndef CPM_HANDLEWAKEUP_ASYCTIMER
#define CPM_HANDLEWAKEUP_ASYCTIMER 1
#endif
// <o> enable ASYCTIMER wakeup check
// <i> AsycTimer Cycle is *** second
#ifndef CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE
#define CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE 5
#endif
// <q> enable TSI wakeup check
// <i> @pre must open PCI
#ifndef CPM_HANDLEWAKEUP_TSI
#define CPM_HANDLEWAKEUP_TSI 1
#endif
// <q> enable SS3 pin wakeup check
// <i> SS3 pin, Trigger mode: Low level
#ifndef CPM_HANDLEWAKEUP_SS3
#define CPM_HANDLEWAKEUP_SS3 0
#endif
// <q> enable WAKEUP pin wakeup check
// <i> WAKEUP pin, Trigger mode: High level
#ifndef CPM_HANDLEWAKEUP_WAKEUP
#define CPM_HANDLEWAKEUP_WAKEUP 1
#endif
// <q> enable USBDET pin wakeup check
// <i> USBDET pin, Trigger mode: High level
#ifndef CPM_HANDLEWAKEUP_USBDET
#define CPM_HANDLEWAKEUP_USBDET 0
#endif
// <q> enable SDIO0 pin wakeup check
// <i>
#ifndef CPM_HANDLEWAKEUP_SDIO0
#define CPM_HANDLEWAKEUP_SDIO0 0
#endif
// </h>

// <<< end of configuration section >>>

typedef enum
{
    CPM_USBDET_PIN = 0,
    CPM_WAKEUP_PIN
} e_CPM_GPIO;

typedef enum
{
    /* System define */
    WAKEUP_SOURCE_GIN0 = 0,
    WAKEUP_SOURCE_GIN1,
    WAKEUP_SOURCE_GIN2,
    WAKEUP_SOURCE_GIN3,
    WAKEUP_SOURCE_GIN4,
    WAKEUP_SOURCE_GIN5,
    WAKEUP_SOURCE_C0GIN0,
    WAKEUP_SOURCE_C0GIN1,
    WAKEUP_SOURCE_C0GIN4,  // 8
    WAKEUP_SOURCE_C0GIN5,  // 9
    WAKEUP_SOURCE_TSI,     // 10
    WAKEUP_SOURCE_ASYNCTC, // 11
    /* User define */
    WAKEUP_SOURCE_SS3,    /*!< SS3 pin, Trigger mode: Low level */
    WAKEUP_SOURCE_WAKEUP, /*!< WAKEUP pin, Trigger mode: High level */
    WAKEUP_SOURCE_USBDET, /*!< USBDET pin, Trigger mode: High level */
    WAKEUP_SOURCE_SDIO0,  /*!< SDIO0 pin */
    /* Maximum number of wakeup sources */
    WAKEUP_SOURCE_POR = 0xFE, /*!< don't modify this line! */
    WAKEUP_SOURCE_NONE = 0xFF /*!< don't modify this line! */
} e_CPM_wakeupSource_Status;

//系统时钟源
#define OSC108M 108 * 1000 * 1000
#define OSC120M 120 * 1000 * 1000
#define OSC150M 150 * 1000 * 1000
#define OSC160M 160 * 1000 * 1000 //必须二分频使用

typedef enum //系统时钟最高256分频
{
    SYS_CLK_DIV_1 = 0,
    SYS_CLK_DIV_2,
    SYS_CLK_DIV_3,
    SYS_CLK_DIV_4,
    SYS_CLK_DIV_5,
    SYS_CLK_DIV_6,
    SYS_CLK_DIV_7,
    SYS_CLK_DIV_8,
    SYS_CLK_DIV_9,
    SYS_CLK_DIV_10,
    SYS_CLK_DIV_11,
    SYS_CLK_DIV_12,
    SYS_CLK_DIV_13,
    SYS_CLK_DIV_14,
    SYS_CLK_DIV_15,
    SYS_CLK_DIV_16
} SYS_CLK_DIV;

typedef enum //IPS 时钟最高16分频
{
    //IPS_CLK_DIV_1 = 0,
    IPS_CLK_DIV_2 = 1,
    IPS_CLK_DIV_3,
    IPS_CLK_DIV_4,
    IPS_CLK_DIV_5,
    IPS_CLK_DIV_6,
    IPS_CLK_DIV_7,
    IPS_CLK_DIV_8,
    IPS_CLK_DIV_9,
    IPS_CLK_DIV_10,
    IPS_CLK_DIV_11,
    IPS_CLK_DIV_12,
    IPS_CLK_DIV_13,
    IPS_CLK_DIV_14,
    IPS_CLK_DIV_15,
    IPS_CLK_DIV_16
} IPS_CLK_DIV;

//打开需要trim的时钟

#define CPM ((CPM_TypeDef *)(CPM_BASE_ADDR))

#define EFLASH_SYS_CLK_20M 20000000
#define EFLASH_SYS_CLK_30M 30000000
#define EFLASH_SYS_CLK_40M 40000000
#define EFLASH_SYS_CLK_60M 60000000
#define EFLASH_SYS_CLK_80M 80000000
#define EFLASH_SYS_CLK_120M 120000000

#define SUPPER_EDGE_WAKEUP 1

#define CPM_VREF_TRIM_090 0x10 //0.90V
#define CPM_VREF_TRIM_105 0x00 //1.05V
#define CPM_VREF_TRIM_110 0x01 //1.10V
#define CPM_VREF_TRIM_115 0x02 //1.15V
#define CPM_VREF_TRIM_121 0x03 //1.21V

#define VDD_PD_RETENT (1 << 0)
#define VDD_WK_SWOFF (1 << 5)
#define CPM_IPS_SLPEN (1 << 9)
#define WKP_AFILT_BYPASS (1 << 15)
#define WKP_DFILT_GTE (1 << 28)
#define WKP_DFILT_BYPASS (1 << 29)
#define WKP_DFILT_EN (1 << 30)

typedef enum _SYSCLK_SEL_
{
    SYSCLK_OSC8M = 0,
    SYSCLK_OSC160M,
    SYSCLK_USBPHY240M,
    SYSCLK_OSCEXT
} SysClkSelect;

typedef enum
{
    /* MULTICGTCR, 0x003C */
    MODULE_CLK_RESERVED0 = 0, /*!< =0 */
    MODULE_CLK_EFMBUS,
    MODULE_CLK_RESERVED1,
    MODULE_CLK_RESERVED2,
    MODULE_CLK_RESERVED3,
    MODULE_CLK_MCC,
    MODULE_CLK_MCCADR,
    MODULE_CLK_ADC,
    MODULE_CLK_RESERVED4,
    MODULE_CLK_MESH,
    MODULE_CLK_TC,
    MODULE_CLK_RESERVED5,
    MODULE_CLK_RESERVED6,
    MODULE_CLK_RESERVED7,
    MODULE_CLK_RESERVED8,
    MODULE_CLK_CLKOUT,
    MODULE_CLK_KEYCTRL,
    MODULE_CLK_RESERVED9,
    MODULE_CLK_EFMIPS,
    MODULE_CLK_CPMIPS,
    MODULE_CLK_EPORT,
    MODULE_CLK_EPORT1,
    MODULE_CLK_EPORT2,
    MODULE_CLK_EPORT3,
    MODULE_CLK_EPORT4,
    MODULE_CLK_RESERVED10,
    MODULE_CLK_TRACE,
    MODULE_CLK_RESERVED11,
    MODULE_CLK_RESERVED12,
    MODULE_CLK_RESERVED13,
    MODULE_CLK_RESERVED14,
    MODULE_CLK_RESERVED15,

    /* SYSCGTCR, 0x0040 */
    MODULE_CLK_RESERVED16, /*!< =32 */
    MODULE_CLK_DMAC1,
    MODULE_CLK_DMAC2,
    MODULE_CLK_RESERVED17,
    MODULE_CLK_CRC0,
    MODULE_CLK_CRC1,
    MODULE_CLK_RESERVED18,
    MODULE_CLK_RESERVED19,
    MODULE_CLK_RESERVED20,
    MODULE_CLK_RESERVED21,
    MODULE_CLK_RESERVED22,
    MODULE_CLK_AHB2MUX,
    MODULE_CLK_SRAMD,
    MODULE_CLK_SRAM0,
    MODULE_CLK_SRAM1,
    MODULE_CLK_SRAM2,
    MODULE_CLK_SRAM3,
    MODULE_CLK_SSI4,
    MODULE_CLK_SSI5,
    MODULE_CLK_ROM,
    MODULE_CLK_RESERVED23,
    MODULE_CLK_RESERVED24,
    MODULE_CLK_M2SBUSM,
    MODULE_CLK_RESERVED25,
    MODULE_CLK_RESERVED26,
    MODULE_CLK_RESERVED27,
    MODULE_CLK_RESERVED28,
    MODULE_CLK_RESERVED29,
    MODULE_CLK_RESERVED30,
    MODULE_CLK_RESERVED31,
    MODULE_CLK_RESERVED32,
    MODULE_CLK_RESERVED33,

    /* AHB3CGTCR, 0x0044 */
    MODULE_CLK_RESERVED34, /*!< =64 */
    MODULE_CLK_RESERVED35,
    MODULE_CLK_RESERVED36,
    MODULE_CLK_USBC,
    MODULE_CLK_RESERVED37,
    MODULE_CLK_AHB3MUX,
    MODULE_CLK_RESERVED38,
    MODULE_CLK_RESERVED39,
    MODULE_CLK_RESERVED40,
    MODULE_CLK_RESERVED41,
    MODULE_CLK_RESERVED42,
    MODULE_CLK_RESERVED43,
    MODULE_CLK_RESERVED44,
    MODULE_CLK_RESERVED45,
    MODULE_CLK_RESERVED46,
    MODULE_CLK_RESERVED47,
    MODULE_CLK_RESERVED48,
    MODULE_CLK_RESERVED49,
    MODULE_CLK_RESERVED50,
    MODULE_CLK_RESERVED51,
    MODULE_CLK_RESERVED52,
    MODULE_CLK_RESERVED53,
    MODULE_CLK_RESERVED54,
    MODULE_CLK_RESERVED55,
    MODULE_CLK_RESERVED56,
    MODULE_CLK_RESERVED57,
    MODULE_CLK_RESERVED58,
    MODULE_CLK_RESERVED59,
    MODULE_CLK_RESERVED60,
    MODULE_CLK_RESERVED61,
    MODULE_CLK_RESERVED62,
    MODULE_CLK_RESERVED63,

    /* ARITHCGTCR, 0x0048 */
    MODULE_CLK_RESERVED64, /*!< =96 */
    MODULE_CLK_AES,
    MODULE_CLK_SM4,
    MODULE_CLK_RESERVED65,
    MODULE_CLK_RF, /*!< RF for AES & SM4 */
    MODULE_CLK_CRYPTO,
    MODULE_CLK_SHA,
    MODULE_CLK_EDMAC0,
    MODULE_CLK_DES,
    MODULE_CLK_ZUC,
    MODULE_CLK_AHB2MLB,
    MODULE_CLK_AHB2IPS2,
    MODULE_CLK_RESERVED66,
    MODULE_CLK_RESERVED67,
    MODULE_CLK_RESERVED68,
    MODULE_CLK_RESERVED69,
    MODULE_CLK_RESERVED70,
    MODULE_CLK_RESERVED71,
    MODULE_CLK_RESERVED72,
    MODULE_CLK_RESERVED73,
    MODULE_CLK_RESERVED74,
    MODULE_CLK_RESERVED75,
    MODULE_CLK_RESERVED76,
    MODULE_CLK_RESERVED77,
    MODULE_CLK_RESERVED78,
    MODULE_CLK_RESERVED79,
    MODULE_CLK_RESERVED80,
    MODULE_CLK_RESERVED81,
    MODULE_CLK_RESERVED82,
    MODULE_CLK_RESERVED83,
    MODULE_CLK_RESERVED84,
    MODULE_CLK_RESERVED85,

    /* IPSCGTCR, 0x004C */
    MODULE_CLK_IOCTRL, /*!< =128 */
    MODULE_CLK_WDT,
    MODULE_CLK_RTC,
    MODULE_CLK_PIT1,
    MODULE_CLK_PIT2,
    MODULE_CLK_USI1,
    MODULE_CLK_EDMAC1,
    MODULE_CLK_SPI1,
    MODULE_CLK_SPI2,
    MODULE_CLK_SPI3,
    MODULE_CLK_SCI1,
    MODULE_CLK_SCI2,
    MODULE_CLK_USI2,
    MODULE_CLK_RESERVED86,
    MODULE_CLK_I2C1,
    MODULE_CLK_PWM0,
    MODULE_CLK_I2C2,
    MODULE_CLK_I2C3,
    MODULE_CLK_SCI3,
    MODULE_CLK_RESERVED87,
    MODULE_CLK_QADC,
    MODULE_CLK_DAC,
    MODULE_CLK_MCC2,
    MODULE_CLK_TSI,
    MODULE_CLK_LD,
    MODULE_CLK_TRNG,
    MODULE_CLK_PGD,
    MODULE_CLK_SECDET,
    MODULE_CLK_PCI,
    MODULE_CLK_PMURTC,
    MODULE_CLK_AHB2IPS,
    MODULE_CLK_CCM /*!< CCM & RESET */
} e_MODULE_CLOCK;

typedef enum _DIVIDER_TYPE_
{
    SYS_DIVIDER = 0,
    TRACE_DIVIDER,
    CLKOUT_DIVIDER,
    IPS_DIVIDER,
    AHB3_DIVIDER,
    ARITH_DIVIDER,
    SDRAM_DIVIDER,
    SDRAM_SM_DIVIER,
    SDRAM2LCD_DIVIDER,
    MCC_DIVIDER,
    MCCADR_DIVIDER,
    ADC_DIVIDER,
    CLCD_DIVIDER,
    MESH_DIVIDER,
    TC_DIVIDER,
    DMA2D_SRAM_DIVIDER,
    MIPI_SAMPLE_DIVIDER,
    DCMI_PIX_DIVIDER,
    DCMI_SENSOR_DIVIDER
} CLK_DIVIDER;

typedef enum _CLK_DIVIDER_OPERATION_
{
    DIVIDER_DISABLE = 0,
    UPDATA_DIVIDER,
    GET_NOW_DIVIDER,
    GET_NOW_CLKGATE,
    CLKGATE_RESTORE
} CLK_DIV_OP;

/*******************************************************************************
* Function Name  : CPM_Write_CoreTestKey
* Description    : 顺序写入core test key
* Input          : KeyOpt
*                      0:清除core test key
*                      !0:设置core test key
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_Write_CoreTestKey(UINT8 KeyOpt);

/*******************************************************************************
* Function Name  : CPM_VCC5V_Bypass
* Description    : 关闭VCC5V LDO
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_VCC5V_Bypass(void);

/*******************************************************************************
* Function Name  : CPM_Vref_Trim
* Description    : trim参考电压
* Input          : trim_value：0x10 is 0.90V; b'00 is 1.05V; b'01 is 1.1V; b'10 is 1.15V; b'11 is 1.21V。
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_Vref_Trim(unsigned int trim_value);

/*******************************************************************************
* Function Name  : CPM_Sleep
* Description    : CPU进入Sleep模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_Sleep(void);

/*******************************************************************************
* Function Name  : CPM_PowerOff_1
* Description    : CPU进入PowerOff_1模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_PowerOff_1(void);

/*******************************************************************************
* Function Name  : CPM_PowerOff_1p5
* Description    : CPU进入PowerOff_1p5模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_PowerOff_1p5(void);

/*******************************************************************************
* Function Name  : CPM_PowerOff_2
* Description    : CPU进入power off 2模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_PowerOff_2(void);

/*******************************************************************************
* Function Name  : CPM_Card_Sleep
* Description    : 卡模式下CPU进入Sleep模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_Card_Sleep(void);

/*******************************************************************************
* Function Name  : CPM_Card_PowerOff1
* Description    : 卡模式下CPU进入power off 1模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_Card_PowerOff1(void);

/*******************************************************************************
* Function Name  : CPM_Card_PowerOff1p5
* Description    : 卡模式下CPU进入power off 1.5模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_Card_PowerOff1p5(void);

/*******************************************************************************
* Function Name  : CPM_SysClkSelect
* Description    : 设置sys_sel分频作为系统时钟
* Input          :  - sys_sel： 系统主时钟源SYSCLK_SEL_OSC1M, SYSCLK_SEL_OSC160M, SYSCLK_SEL_USBPHY or SYSCLK_SEL_OSCEXT
*                   - div：时钟预分频
*
* Output         : None
* Return         : None
******************************************************************************/
extern void CPM_SysClkSelect(UINT32 sys_sel, UINT32 div);

/*******************************************************************************
* Function Name  : get_usb_det_sta
* Description    : 获取USB_DET脚状态
* Input          : None
*
* Output         : None
* Return         : - TRUE：           检测到有USB插入
*                  - FALSE:    检测无USB插入
******************************************************************************/
extern BOOL get_usb_det_sta(void);

/*******************************************************************************
* Function Name  : Clock_Out_Select
* Description    : clock out管脚输出时钟频率的选择
*
* Input          : clk - SELECT_SYSTEM_CLK，系统时钟
*                        SELECT_ARITH_CLK ，算法时钟
*                        SELECT_EFLASH_CLK，Eflash时钟
*                        SELECT_RTC32K_CLK，外部RTC32K时钟
* Output         : None
* Return         : None
******************************************************************************/
extern void Clock_Out_Select(unsigned char clk);
/*******************************************************************************
* Function Name  : Get_Sys_Clock
* Description    : 返回当前系统频率，单位Hz
*
* Input          : 无
* Output         : 系统频率
* Return         : None
******************************************************************************/
extern UINT32 Get_Sys_Clock(void);

/*******************************************************************************
* Function Name  : ARITH_Clk_Op
* Description    : ARITH模块时钟操作
*
* Input          : -div_op：
*                      DIVIDER_DISABLE：禁止分频
*                      UPDATA_DIVIDER：更新分频系数
*                      GET_NOW_DIVIDER：获取当前分频系数
*                  -op_data:
*                      操作参数
* Output         : 当前分频系数
* Return         : None
******************************************************************************/
extern UINT32 ARITH_Clk_Op(UINT32 div_op, UINT32 op_data);

/*******************************************************************************
* Function Name  : SDRAM_Clk_Op
* Description    : SDRAM模块时钟操作
*
* Input          : -div_op：
*                      DIVIDER_DISABLE：禁止分频
*                      UPDATA_DIVIDER：更新分频系数
*                      GET_NOW_DIVIDER：获取当前分频系数
*                  -op_data:
*                      操作参数
* Output         : 当前分频系数
* Return         : None
******************************************************************************/
extern UINT32 SDRAM_Clk_Op(UINT32 div_op, UINT32 op_data);

/*******************************************************************************
* Function Name  : IPS_Clk_Op
* Description    : IPS模块时钟操作
*
* Input          : -div_op：
*                      DIVIDER_DISABLE：禁止分频
*                      UPDATA_DIVIDER：更新分频系数
*                      GET_NOW_DIVIDER：获取当前分频系数
*                  -op_data:
*                      操作参数
* Output         : 当前分频系数
* Return         : None
******************************************************************************/
extern UINT32 IPS_Clk_Op(UINT32 div_op, UINT32 op_data);

/*******************************************************************************
* Function Name  : USBC_PHY_Init
* Description    : 初始化USBC PHY时钟
*
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
extern void USBC_PHY_Init(UINT8 osc_type);

/*******************************************************************************
* Function Name  : Sys_Clk_Init
* Description    : 初始化系统时钟
*
* Input          : -sys_clk_trim:
*                      0:OSC_108M_HZ,
*       1:OSC_120M_HZ ,
*       2:OSC_150M_HZ,
*       3:OSC_160M_HZ
*                : -sys_clk_div
*                      0:SYS_CLK_DIV_1,
*       1:SYS_CLK_DIV_2,
*       ... ...
*                : -ips_clk_div
*                      1:IPS_CLK_DIV_2,
*       2:SYS_CLK_DIV_3,
*       ... ...
* Output         : None
* Return         : None
******************************************************************************/
extern void Sys_Clk_Init(int sys_clk_trim, SYS_CLK_DIV sys_clk_div, IPS_CLK_DIV ips_clk_div);
extern void CPM_ClearPADWKINTCR(void);
extern void TC_Sleep_DelayMS(uint16_t ms);
extern uint8_t cpm_getIRQ_Eport(EPORT_TypeDef *EPORT_addr, uint8_t port);
extern void cpm_handleWakeup(void);
extern uint8_t CPM_ReadGpioData(e_CPM_GPIO GpioNo);
extern void UsbDet_disable(void);
extern void UsbDet_enable(void);
extern uint32_t cpm_controlModuleClock(e_MODULE_CLOCK module, bool control);

extern void ATE_CPM_PowerOff_1p5(void);

#endif /* __CPM_DRV_H__ */
