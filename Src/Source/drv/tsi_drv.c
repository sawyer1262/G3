/**
 * @file tsi_drv.c
 * @author System Application Team
 * @brief TSI模块driver层驱动
 * @version V4.0.0
 * @date    2022.03.22
 *
 * @copyright C*Core Copyright (c) 2020
 *
 */
#include "tsi_drv.h"
/**
 * @brief 设置附加电流.
 *
 * @param[in] TSIx
 * @param[in] ich [8:11]=iche_sel_add_lv,[12:15]=ichi_sel_add_lv
 */
inline void _SET_ICH_ADD_CURRENT(TSI_TypeDef *TSIx, const uint8_t ich)
{
    TSIx->TSI_ANACR &= ~(0xffu << 8);
    TSIx->TSI_ANACR |= (ich << 8);
}

/**
 * @brief 设置track通道顺序.
 *
 * @param[in] TSIx
 * @param[in]
 */
inline void _SET_TRACK_CH(TSI_TypeDef *TSIx)
{
    //开启所有通道track
    //    TSIx->TSI_NHB01 |= 0xFFFFFFFF;
    //    TSIx->TSI_NHB23 |= 0xFFFFFFFF;
    //    TSIx->TSI_NHB45 |= 0xFFFFFFFF;
    //    TSIx->TSI_NHB67 |= 0xFFFFFFFF;
    //    TSIx->TSI_NHB89 |= 0xFFFFFFFF;
    //    TSIx->TSI_NHBab |= 0xFFFFFFFF;
    //    TSIx->TSI_NHBcd |= 0xFFFFFFFF;
    //    TSIx->TSI_NHBef |= 0xFFFFFFFF;

    TSIx->TSI_NHB01 |= 0x0FFF0FFF;
    TSIx->TSI_NHB23 |= 0x0FFF0FFF;
    TSIx->TSI_NHB45 |= 0x0FFF0FFF;
    TSIx->TSI_NHB67 |= 0x0FFF0FFF;
    TSIx->TSI_NHB89 |= 0x0FFF0FFF;
    TSIx->TSI_NHBab |= 0x0FFF0FFF;
    //    TSIx->TSI_NHBcd |= 0xFFFFFFFF;
    //    TSIx->TSI_NHBef |= 0xFFFFFFFF;
    //开启相邻通道track
    // TSIx->TSI_NHB01 |= 0x00090802;
    // TSIx->TSI_NHB23 |= 0x00120030;
    // TSIx->TSI_NHB45 |= 0x0044000C;
    // TSIx->TSI_NHB67 |= 0x03000120;
    // TSIx->TSI_NHB89 |= 0x008000C0;
    // TSIx->TSI_NHBab |= 0x04011800;
    // TSIx->TSI_NHBcd |= 0x50002400;
    // TSIx->TSI_NHBef |= 0x4000A000;
}

/**
 * @brief 设置扫描周期.
 *
 * @param[in] TSIx
 * @param[in] time
 */
inline void _SET_SCANTIME(TSI_TypeDef *TSIx, const uint8_t time)
{
    TSIx->TSI_CHEN &= ~(0xffu << 24);
    TSIx->TSI_CHEN |= (time << 24);
}

/**
 * @brief 设置扫描打盹时间.
 *
 * @param[in] TSIx
 * @param[in] time
 */
inline void _SET_DISABLETIME(TSI_TypeDef *TSIx, const uint8_t time)
{
    TSIx->TSI_CHEN &= ~(0xffu << 16);
    TSIx->TSI_CHEN |= (time << 16);
}

/**
 * @brief 设置TSI通道使能.
 *
 * @param[in] TSIx
 * @param[in] channles
 */
inline void _SET_CHANNLENABLE(TSI_TypeDef *TSIx, const uint16_t channles)
{
    TSIx->TSI_CHEN &= (~0x0000FFFFu);
    TSIx->TSI_CHEN |= channles;
}

/**
 * @brief 获取TSI所有使能通道扫描完成标志.
 * 写1清零
 * @param[in] TSIx @ref TSI_TypeDef
 * @return uint8_t 扫描完成返回0x01,否则返回0.
 */
inline uint8_t _GET_IF_SCANFINISH(TSI_TypeDef *TSIx)
{
    if (TSIx->TSI_ISR & TSI_ISR_SCANENDFINISH)
    {
        return 0x01;
    }
    else
    {
        return 0x00;
    }
}

/**
 * @brief 获得SCANFINISHI中断使能标志.
 *
 * @param[in] TSIx
 * @return uint8_t 使能返回0x01,否则返回0.
 */
inline uint8_t _GET_IE_SCANFINISH(TSI_TypeDef *TSIx)
{
    if (TSIx->TSI_IER & TSI_IER_SCANFINISH_IE)
    {
        return 0x01;
    }
    else
    {
        return 0x00;
    }
}

/**
 * @brief 获取TSI低功耗触摸唤醒中断标志位.
 *
 * @param[in] TSIx
 * @return uint8_t TSI低功耗触摸唤醒返回0x01, 否则返回0.
 */
inline uint8_t _GET_IF_AWD(TSI_TypeDef *TSIx)
{
    if (TSIx->TSI_ISR & TSI_ISR_AWD)
    {
        return 0x01;
    }
    else
    {
        return 0x00;
    }
}

/**
 * @brief 获取TSI低功耗唤醒中断使能标志位.
 *
 * @param[in] TSIx
 * @return uint8_t 使能返回0x01,否则返回0.
 */
inline uint8_t _GET_IE_AWD(TSI_TypeDef *TSIx)
{
    if (TSIx->TSI_IER & TSI_IER_ANALOGWDT_IE)
    {
        return 0x01;
    }
    else
    {
        return 0x00;
    }
}

/**
 * @brief 恢复TSI寄存器到默认值.
 *
 * @param TSIx
 */
void drv_tsi_reset(TSI_TypeDef *TSIx)
{
    const uint32_t default1[7] = {0x00000340, 0x00000000, 0x50000008, 0x10108000,
                                  0x00080100, 0x00000000, 0x00080000};
    const uint32_t default2[17] = {0x00000000, 0x00000AF5, 0x10101010, 0x10101010,
                                   0x10101010, 0x10101010, 0x02C80000, 0x00000000,
                                   0x00000000, 0x40404040, 0x40404040, 0x40404040,
                                   0x40404040, 0x40404040, 0x40404040, 0x40404040,
                                   0x40404040};

    memcpy((uint32_t *)&(TSIx->TSI_ISR), default1, 7);
    memcpy((uint32_t *)&(TSIx->TSI_RESERVED), default2, 17);
}

// TODO: CCM4202S-E add
/**
 * @brief 设置延时判断的次数
 * @param[in] TSIx
 * @param[in] cnt
 */
inline void _SET_DELAY_CHECK_CNT(TSI_TypeDef *TSIx, const uint8_t cnt)
{
    TSIx->TSI_DCCR &= ~(0x7u << 8);
    TSIx->TSI_DCCR |= (cnt << 8);
}

/**
 * @brief 设置低功耗AWD延时判断时间间隔
 * @note 两次判断的间隔时间= delay_cnt_len*500us
 *
 * @param TSIx
 * @param time
 */
inline void _SET_Double_Check_DELAY_TIME(TSI_TypeDef *TSIx, const uint32_t time)
{
    TSIx->TSI_DCCR &= ~(0xFFFFu << 16);
    TSIx->TSI_DCCR |= (time << 16);
}

/**
 * @brief 设置低功耗下环境更新时间间隔
 * @note 时间长度为 cnt_1s_len*500us
 *
 * @param TSIx
 * @param time
 */
inline void _SET_Env_Update_DELAY_TIME(TSI_TypeDef *TSIx, const uint32_t time)
{
    TSIx->TSI_ENV_CR &= ~(0xFFFFu << 16);
    TSIx->TSI_ENV_CR |= (time << 16);
}

/**
 * @brief 设置硬件IIR滤波器系数N
 *
 * @param TSIx
 * @param IIR_N
 */
inline void _SET_IIR_FILTER_N(TSI_TypeDef *TSIx, const uint8_t IIR_N)
{
    TSIx->TSI_TCFLTCR &= ~(0xFFu << 8);
    TSIx->TSI_TCFLTCR |= (IIR_N << 8);
}

/**
 * @brief 设置硬件MEAN滤波器窗口
 *
 */
inline void _SET_MEAN_FILTER_WIDTH(TSI_TypeDef *TSIx, const uint8_t MEAN_WINDOWS_SIZE)
{
    TSIx->TSI_TCFLTCR &= ~TSI_TCFLTCR_MEAN_FLT_WIDTH;
    TSIx->TSI_TCFLTCR |= (MEAN_WINDOWS_SIZE << 20);
}

/**
 * @brief 设置均值load寄存器
 *
 * @param TSIx
 */
inline void _SET_LP_AVERAGE_DATA(TSI_TypeDef *TSIx)
{
    volatile uint32_t *pu32Data = &(TSIx->TSI_CH0DR);
    uint8_t temp[16];
    for (uint8_t i = 0; i < 16; i++)
    {
        temp[i] = ((pu32Data[i] & 0xff) +
                   ((pu32Data[i] >> 8) & 0xff) +
                   ((pu32Data[i] >> 16) & 0xff) +
                   ((pu32Data[i] >> 24) & 0xff)) >>
                  2;
    }
    TSIx->TSI_AVG_03 = temp[0] + (temp[1] << 8) + (temp[2] << 16) + (temp[3] << 24);
    TSIx->TSI_AVG_47 = temp[4] + (temp[5] << 8) + (temp[6] << 16) + (temp[7] << 24);
    TSIx->TSI_AVG_8b = temp[8] + (temp[9] << 8) + (temp[10] << 16) + (temp[11] << 24);
    TSIx->TSI_AVG_cf = temp[12] + (temp[13] << 8) + (temp[14] << 16) + (temp[15] << 24);

    TSIx->TSI_ENV_DATA_03 = temp[0] + (temp[1] << 8) + (temp[2] << 16) + (temp[3] << 24);
    TSIx->TSI_ENV_DATA_47 = temp[4] + (temp[5] << 8) + (temp[6] << 16) + (temp[7] << 24);
    TSIx->TSI_ENV_DATA_8b = temp[8] + (temp[9] << 8) + (temp[10] << 16) + (temp[11] << 24);
    TSIx->TSI_ENV_DATA_cf = temp[12] + (temp[13] << 8) + (temp[14] << 16) + (temp[15] << 24);

    //    TSIx->TSI_AVG_03 = 0xFFFFFFFF;
    //    TSIx->TSI_AVG_47 = 0xFFFFFFFF;
    //    TSIx->TSI_AVG_8b = 0xFFFFFFFF;
    //    TSIx->TSI_AVG_cf = 0xFFFFFFFF;

    //	  TSIx->TSI_AVG_03 = 0;
    //    TSIx->TSI_AVG_47 = 0;
    //    TSIx->TSI_AVG_8b = 0;
    //    TSIx->TSI_AVG_cf = 0;
}

/**
 * @brief 设置环境增量寄存器
 * 
 * @param TSIx 
 */
inline void _SET_LP_ENV_DEALT(TSI_TypeDef *TSIx)
{
    volatile uint32_t *pu32Data = &(TSIx->TSI_DELTA03);
    TSIx->TSI_ENV_DLT_03 = pu32Data[0];
    TSIx->TSI_ENV_DLT_47 = pu32Data[1];
    TSIx->TSI_ENV_DLT_8b = pu32Data[2];
    TSIx->TSI_ENV_DLT_cf = pu32Data[3];
}
/************************ (C) COPYRIGHT C*Core *****END OF FILE**********************/
