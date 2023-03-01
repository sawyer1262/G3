// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : tsi_reg.h
// Version      : V0.2
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef _TSI_REG_H_
#define _TSI_REG_H_

#include "type.h"
#include "memmap.h"

#define TSI ((TSI_TypeDef *)(TSI_BASE_ADDR))

typedef struct
{
	volatile unsigned int TSI_ISR;			/*!< TSI中断状态寄存器, 偏移地址: 0x0000 */
	volatile unsigned int TSI_IER;			/*!< TSI中断配置寄存器, 偏移地址: 0x0004 */
	volatile unsigned int TSI_CR;			/*!< TSI控制寄存器, 偏移地址: 0x0008 */
	volatile unsigned int TSI_TSTR;			/*!< TSI模拟特性配置寄存器, 偏移地址: 0x000c */
	volatile unsigned int TSI_WDCFGR;		/*!< TSI模拟看门狗配置寄存器, 偏移地址: 0x0010 */
	volatile unsigned int TSI_AWDTR;		/*!< TSI模拟看门狗阈值寄存器, 偏移地址: 0x0014 */
	volatile unsigned int TSI_CHCR;			/*!< TSI模式和扫描电流配置寄存器, 偏移地址: 0x0018 */
	volatile unsigned int TSI_CH0DR;		/*!< TSI-CH0数据寄存器, 偏移地址: 0x001c */
	volatile unsigned int TSI_CH1DR;		/*!< TSI-CH1数据寄存器, 偏移地址: 0x0020 */
	volatile unsigned int TSI_CH2DR;		/*!< TSI-CH2数据寄存器, 偏移地址: 0x0024 */
	volatile unsigned int TSI_CH3DR;		/*!< TSI-CH3数据寄存器, 偏移地址: 0x0028 */
	volatile unsigned int TSI_CH4DR;		/*!< TSI-CH4数据寄存器, 偏移地址: 0x002c */
	volatile unsigned int TSI_CH5DR;		/*!< TSI-CH5数据寄存器, 偏移地址: 0x0030 */
	volatile unsigned int TSI_CH6DR;		/*!< TSI-CH6数据寄存器, 偏移地址: 0x0034 */
	volatile unsigned int TSI_CH7DR;		/*!< TSI-CH7数据寄存器, 偏移地址: 0x0038 */
	volatile unsigned int TSI_CH8DR;		/*!< TSI-CH8数据寄存器, 偏移地址: 0x003c */
	volatile unsigned int TSI_CH9DR;		/*!< TSI-CH9数据寄存器, 偏移地址: 0x0040 */
	volatile unsigned int TSI_CHaDR;		/*!< TSI-CH10数据寄存器, 偏移地址: 0x0044 */
	volatile unsigned int TSI_CHbDR;		/*!< TSI-CH11数据寄存器, 偏移地址: 0x0048 */
	volatile unsigned int TSI_CHcDR;		/*!< TSI-CH12数据寄存器, 偏移地址: 0x004c */
	volatile unsigned int TSI_CHdDR;		/*!< TSI-CH13数据寄存器, 偏移地址: 0x0050 */
	volatile unsigned int TSI_CHeDR;		/*!< TSI-CH14数据寄存器, 偏移地址: 0x0054 */
	volatile unsigned int TSI_CHfDR;		/*!< TSI-CH15数据寄存器, 偏移地址: 0x0058 */
	volatile unsigned int TSI_RESERVED;		/*!< TSI 保留寄存器, 偏移地址: 0x005c */
	volatile unsigned int TSI_RESERVED1;	/*!< TSI 保留寄存器, 偏移地址: 0x0060 */
	volatile unsigned int TSI_BASE03;		/*!< TSI CH0~3 Cbase寄存器, 偏移地址: 0x0064 */
	volatile unsigned int TSI_BASE47;		/*!< TSI CH4~7 Cbase寄存器, 偏移地址: 0x0068 */
	volatile unsigned int TSI_BASE8b;		/*!< TSI CH8~b Cbase寄存器, 偏移地址: 0x006c */
	volatile unsigned int TSI_BASEcf;		/*!< TSI CHc~f Cbase寄存器, 偏移地址: 0x0070 */
	volatile unsigned int TSI_CHEN;			/*!< TSI通道使能寄存器, 偏移地址: 0x0074 */
	volatile unsigned int TSI_TCFLTCR;		/*!< TSI 滤波器控制寄存器, 偏移地址: 0x0078 */
	volatile unsigned int TSI_RESERVED3;	/*!< TSI 保留寄存器, 偏移地址: 0x007c */
	volatile unsigned int TSI_DELTA03;		/*!< TSI 按键唤醒阈值增量寄存器, 偏移地址: 0x0080 */
	volatile unsigned int TSI_DELTA47;		/*!< TSI 按键唤醒阈值增量寄存器, 偏移地址: 0x0084 */
	volatile unsigned int TSI_DELTA8b;		/*!< TSI 按键唤醒阈值增量寄存器, 偏移地址: 0x0088 */
	volatile unsigned int TSI_DELTAcf;		/*!< TSI 按键唤醒阈值增量寄存器, 偏移地址: 0x008c */
	volatile unsigned int TSI_Cid_coarse03; /*!< TSI CH0~3 基础Coarse寄存器, 偏移地址: 0x0090 */
	volatile unsigned int TSI_Cid_coarse47; /*!< TSI CH4~7 基础Coarse寄存器, 偏移地址: 0x0094 */
	volatile unsigned int TSI_Cid_coarse8b; /*!< TSI CH8~b 基础Coarse寄存器, 偏移地址: 0x0098 */
	volatile unsigned int TSI_Cid_coarsecf; /*!< TSI CHc~f 基础Coarse寄存器, 偏移地址: 0x009c */
	volatile unsigned int TSI_NHB01;		/*!< TSI CH0与CH1 相邻通道Track控制寄存器, 偏移地址: 0x00a0 */
	volatile unsigned int TSI_NHB23;		/*!< TSI CH2与CH3 相邻通道Track控制寄存器, 偏移地址: 0x00a4 */
	volatile unsigned int TSI_NHB45;		/*!< TSI CH4与CH5 相邻通道Track控制寄存器, 偏移地址: 0x00a8 */
	volatile unsigned int TSI_NHB67;		/*!< TSI CH6与CH7 相邻通道Track控制寄存器, 偏移地址: 0x00ac */
	volatile unsigned int TSI_NHB89;		/*!< TSI CH8与CH9 相邻通道Track控制寄存器, 偏移地址: 0x00b0 */
	volatile unsigned int TSI_NHBab;		/*!< TSI CHa与CHb 相邻通道Track控制寄存器, 偏移地址: 0x00b4 */
	volatile unsigned int TSI_NHBcd;		/*!< TSI CHc与CHd 相邻通道Track控制寄存器, 偏移地址: 0x00b8 */
	volatile unsigned int TSI_NHBef;		/*!< TSI CHe与CHf 相邻通道Track控制寄存器, 偏移地址: 0x00bc */
	volatile unsigned int TSI_RESERVED4;	/*!< TSI 保留寄存器, 偏移地址: 0x00c0 */
	volatile unsigned int TSI_SEED0;		/*!< TSI 模拟IP Trim SEED0 寄存器, 偏移地址: 0x00c4 */
	volatile unsigned int TSI_SEED1;		/*!< TSI 模拟IP Trim SEED0 寄存器, 偏移地址: 0x00c8 */
	volatile unsigned int TSI_SEED2;		/*!< TSI 模拟IP Trim SEED0 寄存器, 偏移地址: 0x00cc */
	volatile unsigned int TSI_UserData[8];	/*!< TSI 用户数据寄存器, 偏移地址: 0x00d0~0x00ec */
	volatile unsigned int TSI_ANACR;		/*!< TSI 模拟控制寄存器, 偏移地址: 0x00f0 */
	volatile unsigned int TSI_DCCR;			/*!< TSI 延时判断控制寄存器, 偏移地址: 0x00f4 */
	volatile unsigned int TSI_ENV_CR;		/*!< TSI 环境控制寄存器, 偏移地址: 0x00f8 */
	volatile unsigned int TSI_RESERVED5;	/*!< TSI 保留寄存器, 偏移地址: 0x00fc */
	volatile unsigned int TSI_AVG_03;		/*!< TSI CH0~3 average寄存器, 偏移地址: 0x0100 */
	volatile unsigned int TSI_AVG_47;		/*!< TSI CH4~7 average寄存器, 偏移地址: 0x0104 */
	volatile unsigned int TSI_AVG_8b;		/*!< TSI CH8~b average寄存器, 偏移地址: 0x0108 */
	volatile unsigned int TSI_AVG_cf;		/*!< TSI CHc~f average寄存器, 偏移地址: 0x010c */
	volatile unsigned int TSI_ENV_DATA_03;	/*!< TSI CH0~3 环境变量设置寄存器, 偏移地址: 0x0110 */
	volatile unsigned int TSI_ENV_DATA_47;	/*!< TSI CH4~7 环境变量设置寄存器, 偏移地址: 0x0114 */
	volatile unsigned int TSI_ENV_DATA_8b;	/*!< TSI CH8~b 环境变量设置寄存器, 偏移地址: 0x0118 */
	volatile unsigned int TSI_ENV_DATA_cf;	/*!< TSI CHc~f 环境变量设置寄存器, 偏移地址: 0x011c */
	volatile unsigned int TSI_ENV_DLT_03;	/*!< TSI CH0~3 环境增量配置寄存器, 偏移地址: 0x0120 */
	volatile unsigned int TSI_ENV_DLT_47;	/*!< TSI CH4~7 环境增量配置寄存器, 偏移地址: 0x0124 */
	volatile unsigned int TSI_ENV_DLT_8b;	/*!< TSI CH8~b 环境增量配置寄存器, 偏移地址: 0x0128 */
	volatile unsigned int TSI_ENV_DLT_cf;	/*!< TSI CHc~f 环境增量配置寄存器, 偏移地址: 0x012c */
	volatile unsigned int TSI_AVG_SR_03;	/*!< TSI CH0~3 average状态寄存器, 偏移地址: 0x0130 */
	volatile unsigned int TSI_AVG_SR_47;	/*!< TSI CH4~7 average状态寄存器, 偏移地址: 0x0134 */
	volatile unsigned int TSI_AVG_SR_8b;	/*!< TSI CH8~b average状态寄存器, 偏移地址: 0x0138 */
	volatile unsigned int TSI_AVG_SR_cf;	/*!< TSI CHc~f average状态寄存器, 偏移地址: 0x013c */
	volatile unsigned int TSI_ENV_SR_03;	/*!< TSI CH0~3 环境变量状态寄存器, 偏移地址: 0x0140 */
	volatile unsigned int TSI_ENV_SR_47;	/*!< TSI CH4~7 环境变量状态寄存器, 偏移地址: 0x0144 */
	volatile unsigned int TSI_ENV_SR_8b;	/*!< TSI CH8~b 环境变量状态寄存器, 偏移地址: 0x0148 */
	volatile unsigned int TSI_ENV_SR_cf;	/*!< TSI CHc~f 环境变量状态寄存器, 偏移地址: 0x014c */
	volatile unsigned int TSI_AVG_LD;		/*!< TSI TSI 均值加载寄存器, 偏移地址: 0x0150 */
	volatile unsigned int TSI_ENV_LD;		/*!< TSI TSI 环境变量加载寄存器, 偏移地址: 0x0154 */
} TSI_TypeDef;

/******************************************************************************/
/*                                                                            */
/*                                  TSI                                       */
/*                                                                            */
/******************************************************************************/

/*****************  Bit definition for TSI_ISR register  *****************/
// TSI_ISR, TSI interrupt and status register, 0x00
#define TSI_ISR_CHANNEL_FINISH (uint32_t)(1U << 19)		  /*!< TSI 单通道扫描完成标志位, 写1清零 */
#define TSI_ISR_SCANENDFINISH (uint32_t)(1U << 17)		  /*!< TSI所有使能通道扫描完成标志, 写1清零 */
#define TSI_ISR_SINGLE_SCANENDFINISH (uint32_t)(1U << 16) /*!< TSI通道扫描完成标志, 写1清零 */
#define TSI_ISR_AWD (uint32_t)(1U << 7)					  /*!< TSI模拟看门狗中断标志位, 写1清零 */
#define TSI_ISR_DELAY_CHECK_FINISH (uint32_t)(1U << 6)	  /*!< 延时判断完成中断标志位, 写1清零 */
/*****************  Bit definition for TSI_IER register  *****************/
// TSI_IER, TSI interrupt and status register, 0x04
#define TSI_IER_SCANFINISH_IE (uint32_t)(1U << 30)	/*!< TSI通道扫描完成中断使能寄存器 */
#define TSI_IER_ANALOGWDT_IE (uint32_t)(1U << 29)	/*!< TSI模拟看门狗中断使能寄存器 */
#define TSI_IER_DELAY_CHECK_IE (uint32_t)(1U << 24) /*!< 延时判断中断使能位 */
/*****************  Bit definition for TSI_CR register  *****************/
// TSI_CR, TSI control register, 0x08
//#define TSI_CR_DEFAULT (uint32_t)(1U << 30)
#define TSI_CR_CH_SW_TIME_MAX (uint32_t)(0xFU << 28) /*!< 通道切换时刻配置*/
#define TSI_CR_BASE_SW_MODE (uint32_t)(0x1U << 27)	 /*!< CBase 和 Cid_Coarse 切换模式*/
#define TSI_CR_EXP_ENABLE (uint32_t)(1U << 24)		 /*!< 0 = 启动异常检测功能;1 = 不启动异常检测功能 */
#define TSI_CR_SCAN_SINGLE_FLAG_EN (uint32_t)(1U << 16)
#define TSI_CR_SCAN_MODE (uint32_t)(1U << 7)		  /*!< =1通道循环扫描模式; =0单通道扫描模式 */
#define TSI_CR_SCAN_SEQUENCE_MODE (uint32_t)(1U << 5) /*!< =1 Channel switch sequence is 000011112222; =0 Channel switch sequence is 0123456.... */
#define TSI_CR_DATA_CLR (uint32_t)(1U << 4)			  /*!< 在 TSI 休眠模式下清除数据寄存器 */
#define TSI_CR_AVALID_ALWAYS (uint32_t)(1U << 3)	  /*!< TSI常开启 */
#define TSI_CR_SCAN_EN (uint32_t)(1U << 0)			  /*!< 使能TSI */
/*****************  Bit definition for TSI_TSTR register  *****************/
//  TSI_TSTR, TSI test register, 0x0c
#define TSI_TSTR_ANALOG (1U)						   /*!< TSI模拟相关trim值 */
#define TSI_TSTR_DISCARDLEN (uint32_t)(3U << 29)	   /*!< 丢失数据的长度*/
#define TSI_TSTR_DISCARDMODE (uint32_t)(1U << 31)	   /*!< =1通道切换时丢弃数据 */
#define TSI_TSTR_FTRIM_LV (uint32_t)(7U << 21)		   /*!< 模拟 IP trim 配置 */
#define TSI_TSTR_IPS_FTRIM (uint32_t)(1U << 4)		   /*!<模拟 IP trim 配置方式选择,0 = 使用位[23:21]固定值配置,1 = 使用随机值进行配置*/
#define TSI_TSTR_IPS_FILTER_SEL_LV (uint32_t)(3U << 2) /*!<模拟 IP 滤波配置*/
/*****************  Bit definition for TSI_TWDCFGR register  *****************/
//  TSI_TWDCFGR, TSI watchdog config register, 0x10
#define TSI_TWDCFGR_STATE_CLR (uint32_t)(1U << 30) /*!< TSI 休眠模式下清除状态机 */

/*****************  Bit definition for TSI_CHCR register  *****************/
// TSI_CHCR, TSI channel config register, 0x18
#define TSI_CHCR_MODE_COARSE (uint32_t)(1U << 30)
#define TSI_CHCR_CURRENT_MAX 0xF0U /*!< 设置充放电速度 */
/*****************  Bit definition for TSI_CHEN register  *****************/
// TSI_CHEN, TSI Channel Enable register, 0x74
#define TSI_CHEN_SCANTIME (uint32_t)(0xFFU << 24)	 /*!< TSI scan times */
#define TSI_CHEN_DISABLETIME (uint32_t)(0x00U << 16) /*!< TSI disable time = (disable len+1) * 500us */

// TODO:完善CCM4202-E新增寄存器
/*****************  Bit definition for TSI_TCFLTCR register  *****************/
// TSI_TCFLTCR, TSI 滤波器控制寄存器, 0x78
#define TSI_TCFLTCR_IIR_EN (uint32_t)(0x1U << 0)			/*!<IIR滤波器使能>*/
#define TSI_TCFLTCR_MEDIAN_EN (uint32_t)(0x1U << 4)			/*!<中值滤波器使能>*/
#define TSI_TCFLTCR_MEDIAN_DATA_SEL (uint32_t)(0x1U << 5)	/*!<0=模拟IP输出，1=IIR滤波器输出>*/
#define TSI_TCFLTCR_IIR_FILTER_N_MAX (uint32_t)(0xFFU << 8) /*!< IIR滤波器系数N */
#define TSI_TCFLTCR_MEAN_EN (uint32_t)(0x1U << 16)			/*!<均值滤波器使能>*/
#define TSI_TCFLTCR_MEAN_DATA_SEL (uint32_t)(0x3U << 17)	/*!< 均值滤波数据来源，00=模拟IP，01=IIR输出，10=中值输出，11=模拟IP*/
#define TSI_TCFLTCR_MEAN_DATA_CLR_EN (uint32_t)(0x1U << 19) /*!<均值滤波器缓存清除使能>*/
#define TSI_TCFLTCR_MEAN_FLT_WIDTH (uint32_t)(0x3U << 20)	/*!< 均值滤波宽度，00=4B，01=8B，10=16B，11=32B*/
#define TSI_TCFLTCR_AWD_DATA_SEL (uint32_t)(0x3U << 24)		/*!< 均值滤波宽度，00=模拟IP，01=均值输出，10=IIR输出，11=中值输出*/
#define TSI_TCFLTCR_EXEPT_DATA_SEL (uint32_t)(0x1U << 28)	/*!<0=异常屏蔽数据源为模拟IP的输出数据，1=异常屏蔽数据源为滤波后的数据>*/
#define TSI_TCFLTCR_COARSE_FILTER_EN (uint32_t)(0x1U << 31) /*!<COARSE模式下滤波器使能>*/

/*****************  Bit definition for TSI_ANACR register  *****************/
// TSI_ANACR, TSI Analog Control register, 0xF0
#define TSI_ANACR_TRACK_ENABLE (uint32_t)(0x1U << 0)	   /*!< 开启TRACK功能 */
#define TSI_ANACR_CMP_HP_MODE (uint32_t)(0x1U << 3)		   /*!< 比较器高压模式> */
#define TSI_ANACR_ICHARGE_ADD_ENABLE (uint32_t)(0x1U << 4) /*!< 电流控制使能> */
#define TSI_ANACR_ICHE_SEL_ADD_MAX (uint32_t)(0xFU << 8)   /*!<外部电流控制> */
#define TSI_ANACR_ICHI_SEL_ADD_MAX (uint32_t)(0xFU << 12)  /*!<内部电流控制> */

/*****************  Bit definition for TSI_DCCR register  *****************/
// TSI_DCCR, TSI 延时判断控制寄存器, 0xF4
#define TSI_DCCR_DELAY_CHECK_ENABLE (uint32_t)(0x1U << 0)	   /*!< 开启延时判断功能 */
#define TSI_DCCR_DELAY_CHECK_SEL (uint32_t)(0x1U << 1)		   /*!< 延时判断模式下，非首次判断时基准值选择 */
#define TSI_DCCR_AVG_1S_UPDATE_ENABLE (uint32_t)(0x1U << 4)	   /*!< 均值寄存器 1 秒自动更新使能 */
#define TSI_DCCR_REF_CHECK_SEL (uint32_t)(0x1U << 7)		   /*!< 延时判断模式下，首次判断时基准值选择 */
#define TSI_DCCR_CHECK_TIMES_MAX (uint32_t)(0x7U << 8)		   /*!<延时判断的次数> */
#define TSI_DCCR_CHECK_DELAY_CNT_MAX (uint32_t)(0xFFFFU << 16) /*!<延时判断的时间间隔> */

/*****************  Bit definition for TSI_ENV_CR register  *****************/
// TSI_ENV_CR, TSI 环境控制寄存器, 0xF8
#define TSI_ENVCR_1S_CNT_ENABLE (uint32_t)(0x1U << 0)			/*!< 开启1 秒计数功能 */
#define TSI_ENVCR_ENV_CMP_ENABLE (uint32_t)(0x1U << 1)			/*!< 延时判断模式下， 同时比较环境变量和均值 */
#define TSI_ENVCR_UPDATE_SRC_SEL (uint32_t)(0x1U << 3)			/*!< 环境变量 1 秒自动更新数据源选择 */
#define TSI_ENVCR_AWD_CHECK_SRC_SEL (uint32_t)(0x1U << 4)		/*!< AWD 判断环境变量选择 */
#define TSI_ENVCR_CHECK_DELAY_CNT_MAX (uint32_t)(0xFFFFU << 16) /*!<1 秒计时长度配置> */

/*****************  Bit definition for TSI_AVG_LD register  *****************/
// TSI_AVG_LD, TSI 环境控制寄存器, 0x150
#define TSI_AVG_LD_EN (uint32_t)(0x1U << 0) /*!< 对此位写 1，把均值配置寄存器中的值加载到均值更新寄存器。 */
/*****************  Bit definition for TSI_ENV_LD register  *****************/
// TSI_ENV_LD, TSI 环境控制寄存器, 0x154
#define TSI_ENV_LD_EN (uint32_t)(0x1U << 0) /*!< 对此位写 1，把环境变量配置寄存器中的值加载到环境变量更新寄存器。 */

#endif /*_TSI_REG_H_*/
