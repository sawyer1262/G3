/**
 * @file tsi_drv.h
 * @author System Application Team
 * @brief TSI模块driver层驱动头文件
  * @version V4.0.0
  * @date    2022.03.22
 * 
 * @copyright C*Core Copyright (c) 2020
 * 
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _TSI_DRV_H_
#define _TSI_DRV_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include "tsi_reg.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** @addtogroup Driver
  * @{
  */

    /** @addtogroup TSI
  * @{
  */

    /*** 宏定义 *******************************************************************/
    /** @addtogroup TSI Exported Macros
  * @{
  */
 
#define _CLEAR_CHANNELFINISHFLAG(TSIx) do{TSIx->TSI_ISR |= TSI_ISR_CHANNEL_FINISH;}while(0) /*!< 单通道扫描完成标志位 */
#define _CLEAR_SCANFINISHFLAG(TSIx) do{TSIx->TSI_ISR |= TSI_ISR_SCANENDFINISH;}while(0) /*!< 清除扫描完成标志位 */
#define _CLEAR_AWDFLAG(TSIx) do{TSIx->TSI_ISR |= TSI_ISR_AWD;}while(0) /*!< 清除TSI中断唤醒标志位 */
#define _CLEAR_DELAY_CHECK_FLAG(TSIx) do{TSIx->TSI_ISR |= TSI_ISR_DELAY_CHECK_FINISH;}while(0) /*!< 清除延时判断完成中断标志位 */

#define _SET_IE_SCANFINISH(TSIx) do{TSIx->TSI_IER |= TSI_IER_SCANFINISH_IE;}while(0) /*!< 使能TSI通道扫描完成中断功能 */
#define _CLEAR_IE_SCANFINISH(TSIx) do{TSIx->TSI_IER &= ~TSI_IER_SCANFINISH_IE;}while(0) /*!< 不使能TSI通道扫描完成中断功能 */
#define _SET_IE_AWD(TSIx) do{TSIx->TSI_IER |= TSI_IER_ANALOGWDT_IE;}while(0) /*!< 使能TSI低功耗触摸唤醒功能 */
#define _CLEAR_IE_AWD(TSIx) do{TSIx->TSI_IER &= ~TSI_IER_ANALOGWDT_IE;}while(0) /*!< 不使能TSI低功耗触摸唤醒功能 */
#define _SET_IE_DELAY_CHECK(TSIx) do{TSIx->TSI_IER |= TSI_IER_DELAY_CHECK_IE;}while(0) /*!< 使能TSI低功耗延时判断触摸唤醒功能 */
#define _CLEAR_IE_DELAY_CHECK(TSIx) do{TSIx->TSI_IER &= ~TSI_IER_DELAY_CHECK_IE;}while(0) /*!< 不使能TSI低功耗延时判断触摸唤醒功能 */

#define _SET_DEFAULT_CR(TSIx) do{TSIx->TSI_CR = 0x50000008;}while(0) /*!< 设置TSI_WDCFGR寄存器为默认值 */
#define _SET_TSI_EN(TSIx) do{TSIx->TSI_CR |= TSI_CR_SCAN_EN;}while(0) /*!< 使能TSI */
#define _SET_TSI_DISABLE(TSIx) do{TSIx->TSI_CR &= ~TSI_CR_SCAN_EN;}while(0) /*!< 关闭TSI */
// #define _SET_CR_DEFAULAT(TSIx) do{TSIx->TSI_CR |= TSI_CR_DEFAULT;}while(0) /*!< 设置TSI_CR寄存器为默认值 */
#define _SET_WORKMODE_SCAN(TSIx) do{TSIx->TSI_CR |= TSI_CR_SCAN_MODE;}while(0) /*!< 设置TSI扫描模式为通道循环扫描模式 */
#define _SET_WORKMODE_SINGLE(TSIx) do{TSIx->TSI_CR &= ~TSI_CR_SCAN_MODE;}while(0) /*!< 设置TSI扫描模式为单通道扫描模式 */
#define _SET_SCANALWAYS(TSIx) do{TSIx->TSI_CR |= TSI_CR_AVALID_ALWAYS;}while(0) /*!< 使能TSI连续扫描搬运模式 */
#define _SET_SCANCYCLICITY(TSIx) do{TSIx->TSI_CR &= ~TSI_CR_AVALID_ALWAYS;}while(0) /*!< 不使能TSI连续扫描搬运模式 */
#define _SET_SCAN_SEQUENCE_MODE_00001111(TSIx) do{TSIx->TSI_CR |= TSI_CR_SCAN_SEQUENCE_MODE;}while(0) /*!< 设置通道扫描模式为00001111... */
#define _SET_SCAN_SEQUENCE_MODE_01234567(TSIx) do{TSIx->TSI_CR &= ~TSI_CR_SCAN_SEQUENCE_MODE;}while(0) /*!< 设置通道扫描模式为012345678... */
#define _SET_TSI_EXP_ENABLE(TSIx) do{TSIx->TSI_CR &= ~TSI_CR_EXP_ENABLE;}while(0) /*!< 启动异常检测功能 */
#define _SET_TSI_EXP_DISABLE(TSIx) do{TSIx->TSI_CR |= TSI_CR_EXP_ENABLE;}while(0) /*!< 不启动异常检测功能 */
#define _SET_TSI_SINGLE_FLAG_ENABLE(TSIx) do{TSIx->TSI_CR &= ~TSI_CR_SCAN_SINGLE_FLAG_EN;}while(0) /*!< 启动异常检测功能 */
#define _SET_TSI_SINGLE_FLAG_DISABLE(TSIx) do{TSIx->TSI_CR |= TSI_CR_SCAN_SINGLE_FLAG_EN;}while(0) /*!< 不启动异常检测功能 */


#define _SET_TSTR_ANALOG(TSIx) do{TSIx->TSI_TSTR |= TSI_TSTR_ANALOG;}while(0) /*!< trim the tsi verf */
#define _SET_DISCARDMODE(TSIx) do{TSIx->TSI_TSTR |= TSI_TSTR_DISCARDMODE;}while(0) /*!< =1通道切换时丢弃数据 */
#define _SET_DISCARDMODE_NONE(TSIx) do{TSIx->TSI_TSTR &= ~TSI_TSTR_DISCARDMODE;}while(0) /*!< =1通道切换时丢弃数据 */
#define _SET_DISCARD_LEN(TSIx) do{TSIx->TSI_TSTR |= TSI_TSTR_DISCARDLEN;}while(0) /*!< 丢失数据的长度 */
#define _SET_DISCARD_LEN_0(TSIx) do{TSIx->TSI_TSTR &= ~TSI_TSTR_DISCARDLEN;}while(0) /*!< 丢失数据的长度 */
#define _SET_TSTR_IPS_FTRIM_FIXED(TSIx) do{TSIx->TSI_TSTR |= TSI_TSTR_IPS_FTRIM;}while(0) /*!< =1 使用随机值进行配置 */
#define _SET_TSTR_IPS_FTRIM_RANDOM(TSIx) do{TSIx->TSI_TSTR &= ~TSI_TSTR_IPS_FTRIM;}while(0) /*!< =0 使用位[23:21]固定值配置 */
#define _SET_TSTR_filter_1(TSIx) do{TSIx->TSI_TSTR &=~0x0C;}while(0) /*!< 模拟 IP 滤波配置为1 */
#define _SET_TSTR_filter_3(TSIx) do{TSIx->TSI_TSTR &=~0x0C;TSIx->TSI_TSTR |=0x04;}while(0) /*!<  模拟 IP 滤波配置为3 */
#define _SET_TSTR_filter_5(TSIx) do{TSIx->TSI_TSTR &=~0x0C;TSIx->TSI_TSTR |=0x08;}while(0) /*!<  模拟 IP 滤波配置为5 */
#define _SET_TSTR_filter_7(TSIx) do{TSIx->TSI_TSTR &=~0x0C;TSIx->TSI_TSTR |=0x0C;}while(0) /*!<  模拟 IP 滤波配置为7 */

#define _SET_DEFAULT_WDCFGR(TSIx) do{TSIx->TSI_WDCFGR = 0x00080100;}while(0) /*!< 设置TSI_WDCFGR寄存器为默认值 */
#define _SET_WDCFGR_LP(TSIx) do{TSIx->TSI_WDCFGR = 0x000002e0;}while(0) /*!< 设置TSI_WDCFGR寄存器为低功耗配置 */
#define _SET_SLEEP_STATE_CLR_EN(TSIx) do{TSIx->TSI_WDCFGR |= TSI_TWDCFGR_STATE_CLR;}while(0) /*!< 设置TSI 休眠模式下清除状态机 */
#define _SET_SLEEP_STATE_CLR_DIS(TSIx) do{TSIx->TSI_WDCFGR &= ~TSI_TWDCFGR_STATE_CLR;}while(0) /*!< 设置TSI 休眠模式下不清除状态机*/

#define _ENABLE_FINEMODE(TSIx) do{TSIx->TSI_CHCR &= ~TSI_CHCR_MODE_COARSE;}while(0) /*!< 设置工作模式为Fine模式 */
#define _ENABLE_COARSEMODE(TSIx) do{TSIx->TSI_CHCR |= TSI_CHCR_MODE_COARSE;}while(0) /*!< 设置工作模式为Coarse模式 */
#define _SET_CURRENT(TSIx) do{TSIx->TSI_CHCR |= TSI_CHCR_CURRENT_MAX;}while(0) /*!< 设置充放电电流 */

#define _CLEAR_REGISTER_CHEN(TSIx) do{TSIx->TSI_CHEN = 0;}while(0) /*!< trim the tsi verf */

//TODO:CCM4202-E ADD  
#define _SET_IIR_FILTER_ENABLE(TSIx) do{TSIx->TSI_TCFLTCR |= TSI_TCFLTCR_IIR_EN;}while(0)/*!<使能IIR滤波器>*/
#define _SET_IIR_FILTER_DISABLE(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_IIR_EN;}while(0)/*!<关闭IIR滤波器>*/
#define _SET_MEDIAN_FILTER_ENABLE(TSIx) do{TSIx->TSI_TCFLTCR |= TSI_TCFLTCR_MEDIAN_EN;}while(0)/*!<使能中值滤波器>*/
#define _SET_MEDIAN_FILTER_DISABLE(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEDIAN_EN;}while(0)/*!<关闭中值滤波器>*/
#define _SET_MEDIAN_FLT_DATA_FROM_IIR(TSIx) do{TSIx->TSI_TCFLTCR |= TSI_TCFLTCR_MEDIAN_DATA_SEL;}while(0)/*!<设置中值滤波器数据来源为IIR输出>*/
#define _SET_MEDIAN_FLT_DATA_FROM_ANA(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEDIAN_DATA_SEL;}while(0)/*!<设置中值滤波器数据来源为模拟IP输出>*/
#define _SET_MEAN_FILTER_ENABLE(TSIx) do{TSIx->TSI_TCFLTCR |= TSI_TCFLTCR_MEAN_EN;}while(0)/*!<使能均值滤波器>*/
#define _SET_MEAN_FILTER_DISABLE(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_EN;}while(0)/*!<关闭均值滤波器>*/
#define _SET_MEAN_FLT_DATA_FROM_ANA(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_DATA_SEL;}while(0)/*!<设置均值滤波器数据来源为模拟IP输出>*/
#define _SET_MEAN_FLT_DATA_FROM_IIR(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_DATA_SEL;TSIx->TSI_TCFLTCR |= 0x1U << 17;}while(0)/*!<设置均值滤波器数据来源为IIR滤波器输出>*/
#define _SET_MEAN_FLT_DATA_FROM_MEDIAN(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_DATA_SEL;TSIx->TSI_TCFLTCR |= 0x1U << 18;}while(0)/*!<设置中值滤波器数据来源为中值滤波器输出>*/
#define _SET_LP_MEAN_DATA_CLR(TSIx) do{TSIx->TSI_TCFLTCR |= TSI_TCFLTCR_MEAN_DATA_CLR_EN;}while(0)/*!<LP均值滤波器缓存清除使能>*/
#define _SET_LP_MEAN_DATA_RSV(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_DATA_CLR_EN;}while(0)/*!<关闭LP均值滤波器缓存清除使能>*/
#define _SET_MEAN_FLT_WIDTH_4(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_FLT_WIDTH;}while(0)/*!<设置均值滤波器宽度为4byte>*/
#define _SET_MEAN_FLT_WIDTH_8(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_FLT_WIDTH;TSIx->TSI_TCFLTCR |= 0x1U << 20;}while(0)/*!<设置均值滤波器宽度为8byte>*/
#define _SET_MEAN_FLT_WIDTH_16(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_FLT_WIDTH;TSIx->TSI_TCFLTCR |= 0x1U << 21;}while(0)/*!<设置均值滤波器宽度为16byte>*/
#define _SET_MEAN_FLT_WIDTH_32(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_FLT_WIDTH;TSIx->TSI_TCFLTCR |=TSI_TCFLTCR_MEAN_FLT_WIDTH;}while(0)/*!<设置均值滤波器宽度为32byte>*/
#define _SET_AWD_DATA_FROM_ANA(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_AWD_DATA_SEL;}while(0)/*!<设置AWD数据源为模拟IP>*/
#define _SET_AWD_DATA_FROM_MEAN(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_AWD_DATA_SEL;TSIx->TSI_TCFLTCR |= 0x1U << 24;}while(0)/*!<设置AWD数据源为均值滤波器>*/
#define _SET_AWD_DATA_FROM_IIR(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_AWD_DATA_SEL;TSIx->TSI_TCFLTCR |= 0x1U << 25;}while(0)/*!<设置AWD数据源为IIR滤波器>*/
#define _SET_AWD_DATA_FROM_MEDIAN(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_AWD_DATA_SEL;TSIx->TSI_TCFLTCR |=TSI_TCFLTCR_AWD_DATA_SEL;}while(0)/*!<设置AWD数据源为中值滤波器>*/
#define _MASK_DATA_FROM_ANA(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_EXEPT_DATA_SEL;}while(0)/*!<异常屏蔽数据源为模拟 IP 的输出数据>*/
#define _MASK_DATA_FROM_FILTER(TSIx) do{TSIx->TSI_TCFLTCR |= TSI_TCFLTCR_EXEPT_DATA_SEL;}while(0)/*!<异常屏蔽数据源为滤波后的数据>*/
#define _SET_COARSE_FILTER_ENABLE(TSIx) do{TSIx->TSI_TCFLTCR |= TSI_TCFLTCR_COARSE_FILTER_EN;}while(0)/*!<TSI coarse 模式下开启滤波功能>*/
#define _SET_COARSE_FILTER_DISABLE(TSIx) do{TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_COARSE_FILTER_EN;}while(0)/*!<TSI coarse 模式下关闭滤波功能>*/

#define _SET_TRACK_ENABLE(TSIx) do{TSIx->TSI_ANACR |= TSI_ANACR_TRACK_ENABLE;}while(0)/*!<使能TRACK功能>*/
#define _SET_TRACK_DISABLE(TSIx) do{TSIx->TSI_ANACR &= ~TSI_ANACR_TRACK_ENABLE;}while(0)/*!<关闭TRACK功能>*/
#define _SET_HP_CMP_ENABLE(TSIx) do{TSIx->TSI_ANACR |= TSI_ANACR_CMP_HP_MODE;}while(0)/*!<使能比较器高压模式>*/
#define _SET_HP_CMP_DISABLE(TSIx) do{TSIx->TSI_ANACR &= ~TSI_ANACR_CMP_HP_MODE;}while(0)/*!<关闭比较器高压模式>*/
#define _SET_ICHARGE_ADD_ENABLE(TSIx) do{TSIx->TSI_ANACR |= TSI_ANACR_ICHARGE_ADD_ENABLE;}while(0)/*!<使能附件电流>*/
#define _SET_ICHARGE_ADD_DISABLE(TSIx) do{TSIx->TSI_ANACR &= ~TSI_ANACR_ICHARGE_ADD_ENABLE;}while(0)/*!<关闭附加电流>*/
#define _SET_ICHE_SEL_ADD_CURRENT(TSIx) do{TSIx->TSI_ANACR |= TSI_ANACR_ICHE_SEL_ADD_MAX;}while(0) /*!< 设置附加外部充放电电流 */
#define _SET_ICHI_SEL_ADD_CURRENT(TSIx) do{TSIx->TSI_ANACR |= TSI_ANACR_ICHI_SEL_ADD_MAX;}while(0) /*!< 设置附加内部充放电电流 */

#define _SET_DELAY_CHECK_ENABLE(TSIx) do{TSIx->TSI_DCCR |= TSI_DCCR_DELAY_CHECK_ENABLE;}while(0)/*!<开启延时判断功能>*/
#define _SET_DELAY_CHECK_DISABLE(TSIx) do{TSIx->TSI_DCCR &= ~TSI_DCCR_DELAY_CHECK_ENABLE;}while(0)/*!<关闭延时判断功能>*/
#define _SET_DELAY_CHECK_SRC_FROM_REG(TSIx) do{TSIx->TSI_DCCR |= TSI_DCCR_DELAY_CHECK_SEL;}while(0)/*!<非首次判断基准值使用均值寄存器配置值或 1 秒前自动更新的均值>*/
#define _SET_DELAY_CHECK_SRC_FROM_MEAN(TSIx) do{TSIx->TSI_DCCR &= ~TSI_DCCR_DELAY_CHECK_SEL;}while(0)/*!<非首次判断基准值使用前四轮的均值>*/
#define _SET_AVG_1S_UPDATE_ENABLE(TSIx) do{TSIx->TSI_DCCR |= TSI_DCCR_AVG_1S_UPDATE_ENABLE;}while(0)/*!<均值寄存器每过 1 秒自动更新>*/
#define _SET_AVG_1S_UPDATE_DISABLE(TSIx) do{TSIx->TSI_DCCR &= ~TSI_DCCR_AVG_1S_UPDATE_ENABLE;}while(0)/*!<均值寄存器不自动更新>*/
#define _SET_FIRST_CHECK_SRC_FROM_REG(TSIx) do{TSIx->TSI_DCCR |= TSI_DCCR_REF_CHECK_SEL;}while(0)/*!<首次判断基准值使用均值寄存器配置值或 1 秒前自动更新的均值>*/
#define _SET_FIRST_CHECK_SRC_FROM_MEAN(TSIx) do{TSIx->TSI_DCCR &= ~TSI_DCCR_REF_CHECK_SEL;}while(0)/*!<首次判断基准值使用前四轮的均值>*/

#define _SET_1S_CNT_ENABLE(TSIx) do{TSIx->TSI_ENV_CR |= TSI_ENVCR_1S_CNT_ENABLE;}while(0)/*!<开启 1 秒计数功能>*/
#define _SET_1S_CNT_DISABLE(TSIx) do{TSIx->TSI_ENV_CR &= ~TSI_ENVCR_1S_CNT_ENABLE;}while(0)/*!<关闭 1 秒计数功能>*/
#define _SET_DELAY_CMP_WITH_MEAN_AND_ENV(TSIx) do{TSIx->TSI_ENV_CR |= TSI_ENVCR_ENV_CMP_ENABLE;}while(0)/*!<同时比较环境变量和均值>*/
#define _SET_DELAY_CMP_ONLY_WITH_MEAN(TSIx) do{TSIx->TSI_ENV_CR &= ~TSI_ENVCR_ENV_CMP_ENABLE;}while(0)/*!<只比较均值>*/
#define _SET_ENV_1S_UPDATE_SRC_FROM_MEAN(TSIx) do{TSIx->TSI_ENV_CR |= TSI_ENVCR_UPDATE_SRC_SEL;}while(0)/*!<环境变量 1 秒自动更新数据源为均值>*/
#define _SET_ENV_1S_UPDATE_SRC_FROM_ANA(TSIx) do{TSIx->TSI_ENV_CR &= ~TSI_ENVCR_UPDATE_SRC_SEL;}while(0)/*!<环境变量 1 秒自动更新数据源为实时采样值>*/
#define _SET_AWD_CHECK_ENV_ONLY(TSIx) do{TSIx->TSI_ENV_CR |= TSI_ENVCR_AWD_CHECK_SRC_SEL;}while(0)/*!<AWD 只判断环境变量>*/
#define _SET_AWD_CHECK_ENV_AND_MEAN(TSIx) do{TSIx->TSI_ENV_CR &= ~TSI_ENVCR_AWD_CHECK_SRC_SEL;}while(0)/*!<AWD 判断均值和环境变量>*/

#define _LOAD_AVG_DATA(TSIx) do{TSIx->TSI_AVG_LD |= TSI_AVG_LD_EN;}while(0)/*!<把均值配置寄存器中的值加载到均值更新寄存器>*/
#define _LOAD_ENV_DATA(TSIx) do{TSIx->TSI_ENV_LD |= TSI_ENV_LD_EN;}while(0)/*!<把环境变量配置寄存器中的值加载到环境变量更新寄存器>*/


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

    /**
  * @}
  */

    /*** 函数声明 ******************************************************************/
    /** @addtogroup TSI Exported Functions
  * @{
  */
    extern inline void _SET_CHANNLENABLE(TSI_TypeDef *TSIx, const uint16_t channles);
    extern inline void _SET_SCANTIME(TSI_TypeDef *TSIx, const uint8_t time);
    extern inline void _SET_DISABLETIME(TSI_TypeDef *TSIx, const uint8_t time);
    extern inline uint8_t _GET_IF_SCANFINISH(TSI_TypeDef *TSIx);
    extern inline uint8_t _GET_IE_SCANFINISH(TSI_TypeDef *TSIx);
    extern inline uint8_t _GET_IF_AWD(TSI_TypeDef *TSIx);
    extern inline uint8_t _GET_IE_AWD(TSI_TypeDef *TSIx);
    extern void drv_tsi_reset(TSI_TypeDef *TSIx);
	extern inline uint8_t _GET_IF_SINGLE_SCANFINISH(TSI_TypeDef *TSIx);

    extern inline void _SET_DELAY_CHECK_CNT(TSI_TypeDef *TSIx, const uint8_t cnt);
    extern inline void _SET_Double_Check_DELAY_TIME(TSI_TypeDef *TSIx, const uint32_t time);
    extern inline void _SET_Env_Update_DELAY_TIME(TSI_TypeDef *TSIx, const uint32_t time);
    extern inline void _SET_IIR_FILTER_N(TSI_TypeDef *TSIx, const uint8_t IIR_N);
    extern inline void _SET_ICH_ADD_CURRENT(TSI_TypeDef *TSIx, const uint8_t ich);
    extern inline void _SET_TRACK_CH(TSI_TypeDef *TSIx);
    extern inline void _SET_MEAN_FILTER_WIDTH(TSI_TypeDef *TSIx, const uint8_t MEAN_WINDOWS_SIZE);
    extern inline void _SET_LP_AVERAGE_DATA(TSI_TypeDef *TSIx);
    extern inline void _SET_LP_ENV_DEALT(TSI_TypeDef *TSIx);
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

#endif //_TSI_DRV_H_

/************************ (C) COPYRIGHT C*Core *****END OF FILE****/
