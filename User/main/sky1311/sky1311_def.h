/******************************************************************************/
/*               (C) 斯凯瑞利（北京）科技有限公司(SKYRELAY)                   */
/*                                                                            */
/* 此代码是斯凯瑞利（北京）科技有限公司为支持客户编写的示例程序的一部分       */
/* 所有使用斯凯瑞利相关芯片的用户可以无偿使用该代码，但需保留本声明部分       */
/* 本代码仅为演示使用，不保证符合所有的应用要求，当有特定规范或需求时，       */
/* 使用者需自行确认代码是否符合要求，不合要求时自行做出相应的修改。           */
/* http://www.skyrelay-ic.com                                                 */
/******************************************************************************/
/**
 ******************************************************************************
 ** 文 件 名： sky1311_def.h
 **
 ** 文件简述： 程序中变量的基本类型、结构体和其它类型定义
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  Mark   正式版本
 ** 2020-04-27 V1.3.0 SZ     正式版本
 ******************************************************************************/
#ifndef  __SKY1311_DEF_H_
#define  __SKY1311_DEF_H_
#include <stdio.h>
#include <limits.h>

/**
 ******************************************************************************
 ** \简述：基本类型定义
 **
 ******************************************************************************/
typedef enum sta_result
{
    Ok                      = 0,  ///< 无错，操作成功
    Error                   = 1,  ///< 非特定错误
    Timeout                 = 3,  ///< 超时错误
    RxParity			    = 4,  ///< 接收奇偶校验错
    RxCheckCRC			    = 5,  ///< 接收CRC校验错
    FifoFull                = 6,  ///< FIFO满
    FifoEmpty               = 7,  ///< FIFO空
    Collision               = 8,  ///< 防冲突错误
    Framing                 = 9,  ///< 数据帧错误
    UIDFormat               = 10, ///< UID格式错误
    M1Error                 = 11, ///< 操作M1卡错误

    ErrorRequest            = 12,  ///< 执行REQA/WUPA 命令时出错
    ErrorAnticollision      = 13,  ///< 执行防冲突命令ANTICOLLISION时出错
    ErrorSelect             = 14,  ///< 执行SELECT 命令时出错
    ErrorAts                = 15,  ///< 执行ATS命令出错
    ErrorInvalidMode        = 16,  ///< Operation not allowed in current mode
    ErrorUninitialized      = 17,  ///< Module (or part of it) was not initialized properly
    ErrorBufferFull         = 18,  ///< Circular buffer can not be written because the buffer is full
    ErrorTimeout            = 19,  ///< Time Out error occurred (e.g. I2C arbitration lost, Flash time-out, etc.)
    ErrorNotReady           = 20,  ///< A requested final state is not reached
    OperationInProgress     = 21,  ///< Indicator for operation in progress
    
    NTag_InvalidAddr        = 0x30,
    NTag_CheckError         = 0x31,
    NTag_InvalidAuth        = 0x34,
    NTag_WriteError         = 0x35,

    UnknowError             = 0x7F,
    NoResponse              = 0xFF
}sta_result_t,en_result_t;


typedef enum sta_field
{
    NoObjectIn              = 0,    ///< 读卡器场内无卡或异物
    PiccIn                  = 1,    ///< 有卡在场内
    OtherObjectIn           = 2
}sta_field_t;


/**
 ******************************************************************************
 ** \简述： 配置开关定义
 **
 ******************************************************************************/
#define CHECKCARD       1       // 是否要询卡，1：是，0：不是
#define READERCARD      1       // 是否询卡读卡，1：是，0：不是
#define USEDCMD         1       // 是否使用串口命令，1：是，0：不是
/******************************************************************************/
/* Demo程序操作卡类型选择 */
#define AUTO            0       // 自动判断卡类型（不精确）
#define MIFARE1         1       // M1卡
#define SMART           2       // CPU卡
#define NTAG2           3       // NFC TAG Type2 (NTAG21x)

#define CARDTYPE        AUTO    // (依据 SAK & ATQA 判断)

/* 低功耗检卡AD有关参数 */
#define MINADVAL        0x30        // RC扫描时低于此值认为无效
#define MAXADVAL        0xFF
#define RSSI_DELTA      3           // 检卡时AD变化相对阈值
#define RSSI_ABS        20          // 检卡时AD变化绝对阈值

/* 低功耗检卡PA电压 */
#define CHK_PAVOL       PA_2P0V     // 检卡时使用的PA电压
#define RD_PAVOL        PA_2P5V     // 读卡时使用的PA电压（2.0，2.5，3.0，3.3）

/* SKY1311T 寄存器配置有关定义，许仔细配置*/
#define IXTAL           (0)         //0：外部晶振驱动电流300uA，2：晶振驱动电流500uA
#define ANA2_A          (0x00)      //读A类卡时 Analog2寄存器的配置
#define ANA2_B          (0x1C)      //读B类卡时 Analog2寄存器的配置
#define OSCFREQ         CLK_SEL3    //外部晶振27.12MHz( CLK_SEL2 = 13.56MHz)

/* 调试信息设置 */
#define SKY_DEBUG             1//0      //uart debug
#define CARD_REMOVED_CHECK    1//0        //是否检测退卡

#endif