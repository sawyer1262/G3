/******************************************************************************/
/*               (C) 斯凯瑞利（北京）科技有限公司(SKYRELAY)                   */
/*                                                                            */
/* 此代码是斯凯瑞利（北京）科技有限公司为支持客户编写的示例程序的一部分       */
/* 所有使用斯凯瑞利相关芯片的用户可以无偿使用该代码，但需保留本声明部分       */
/* 本代码仅为演示使用，不保证符合所有的应用要求，当有特定规范或需求时，       */
/* 使用者需自行确认代码是否符合要求，不合要求时自行做出相应的修改。           */
/* (V1.00)                                                                    */
/******************************************************************************/
/**
 ******************************************************************************
 ** 文 件 名： iso14443.h
 **
 ** 文件简述： 与ISO14443以及M1卡有关的操作函数，变量以及常量定义
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  Mark   正式版本
 ** 2018-09-02 V1.2.0 Mark   正式版本
 ******************************************************************************/
#ifndef __ISO14443_H_
#define __ISO14443_H_

#include "sky1311t.h"
#include "sky1311_drv.h"

 /**
 ******************************************************************************
 ** \简述：全局预定义
 **
 ******************************************************************************/
#define     SEL1            0x93
#define     SEL2            0x95
#define     SEL3            0x97

#define     REQA            0x26
#define     WUPA            0x52
#define     SELECT          0x90
#define     HALTA           0x50
#define     PATS            0xE0
#define     PPS             0xD0

#define	    APF_CODE	    0x05		// REQB命令帧前缀字节APf
#define	    APN_CODE	    0x05		// REQB命令帧前缀字节APn
#define	    APC_CODE	    0x1D		// ATTRIB命令帧前缀字节APC
#define	    HALTB_CODE	    0x50		// 挂起命令

#define     REQIDEL		    0x00		// IDLE
#define     REQALL		    0x08		// ALL

#define     M1_AUTH         0x60
#define     M1_AUTH_KEYA    0x60
#define     M1_AUTH_KEYB    0x61
#define     M1_READ         0x30
#define     M1_WRITE        0xA0
#define     M1_INCREMENT    0xC1
#define     M1_DECREMENT    0xC0
#define     M1_RESTORE      0xC2
#define     M1_TRANSFER     0xB0
#define     M1_ACK          0x0A



//I-block (00xxxxxx)b (not (00xxx101)b)
//R-block (10xxxxxx)b (not (1001xxxx)b)
//S-block (11xxxxxx)b (not (1110xxxx)b and not (1101xxxx)b)

/*****************************************************************************/
/*                      以下是ISO14443-4内容  （备用）                        */
/*****************************************************************************/

#define TCL_I_BLOCK               0x02    //I-Block                             000x xx1x
#define TCL_I_BLOCK_RFU           0x02    //I-BLOCK_RFU                         00xx xx1x
#define TCL_R_BLOCK               0xA2    //R-Block                             1010 x01x
#define TCL_R_BLOCK_RFU           0xA0    //R-Block_RFU                         1010 x0xx
#define TCL_R_BLOCK_ACK           0xA2    //R-Block-ACK                         1010 x01x
#define TCL_R_BLOCK_ACK_RFU       0xA0    //R-Block-ACK_RFU                     1010 x0xx
#define TCL_R_BLOCK_NAK           0xB2    //R-Block-NAK                         1011 x01x
#define TCL_R_BLOCK_NAK_RFU       0xB0    //R-Block-NAK_RFU                     1011 x0xx
#define TCL_S_BLOCK_DES           0xC2    //S-Block                             1100 x010
#define TCL_S_BLOCK_DES_RFU       0xC0    //S-Block_RFU                         1100 x0xx
#define TCL_S_BLOCK_WTX           0xF2    //S-Block                             1111 x010
#define TCL_S_BLOCK_WTX_RFU       0xF0    //S-Block_RFU                         1111 x0xx
#define TCL_PPSS_BLOCK            0xD0    //PPS S-Block                         1101 xxxx
#define POWER_LEVEL_INDICATION    0x00    //CID byte                            xx00 0000
//#define TCL_S_BLOCK               0xC2    //S-Block                             11xx x010

#define TCL_M_I_BLOCK             0xE2    //I-BLOCK MASK                        1110 0010
#define TCL_M_I_BLOCK_RFU         0xC2    //I-BLOCK_RFU MASK                    1100 0010
#define TCL_M_R_BLOCK             0xE6    //R-BLOCK MASK                        1110 0110
#define TCL_M_R_BLOCK_RFU         0xE4    //R-BLOCK_RFU MASK                    1110 0100
//#define TCL_M_S_BLOCK             0xF6    //S-BLOCK MASK                        1111 0111
#define TCL_M_S_BLOCK             0xF7    //S-BLOCK MASK                        1111 0111
#define TCL_M_S_BLOCK_RFU         0xF4    //S-BLOCK_RFU MASK                    1111 0100
#define TCL_M_CHAINING            0x10    //CHAIN MASK                          0001 0000
#define TCL_M_CID                 0x08    //CID following                       0000 1000
#define TCL_M_NAD                 0x04    //NAD following                       0000 0100
#define TCL_M_BLK_NUM             0x01    //BLOCK NUM MASK                      0000 0001
#define TCL_M_NAK                 0x10    //R-NAK                               0001 0000
#define TCL_M_POWER_LEVEL         0x00    //Power Level Indication              0000 0000
#define TCL_M_WTXM                0x3F    //Waiting Time eXtension Multipier    0011 1111

/**
 ******************************************************************************
 ** \简述： 数据类型定义
 **
 ******************************************************************************/

////============= 全局变量和函数定义 ================================================================
typedef struct
{
	uint8_t PUPI[4];					// 伪唯一PICC标识符
	uint8_t AppDat[4];					// 应用数据
	uint8_t ProtInf[3];					// 参数信息
}nAQTB_t;

/******************************************************************************
** \简述： 全局变量定义
**
******************************************************************************/
extern uint8_t  g_FWI;                 //Frame Waiting time Integer
extern uint8_t  g_bCID;                //Card IDentifier
extern uint16_t g_bFSD;                //Frame Size for proximity coupling Device
extern uint16_t g_bFSC;                //Frame Size for proximity Card
extern uint8_t  g_bCIDFollowed;        //CID Followed
extern uint8_t  g_bNADFollowed;        //NAD followed

extern uint8_t  g_bBlockNum;           // block number in PCB
extern uint8_t  g_PCB;                 // PCB for current time
extern uint8_t  g_bLastTransmitPCB;    //PCB of last transmit
extern uint8_t  * g_pbaLastTransmitBuf;
extern uint16_t g_wLastTransmitLength;
/**
******************************************************************************
** \简述： 函数声明
**
******************************************************************************/
sta_result_t M1_Authentication(uint8_t auth_mode,uint8_t *m1Key,uint8_t *uid, uint8_t blockAddr);
sta_result_t M1_Write(uint8_t blockAddr, uint8_t* data);
sta_result_t M1_Read(uint8_t blockAddr, uint8_t *blockBuff);
sta_result_t M1_Increment(uint8_t blockAddr, uint32_t value);
sta_result_t M1_Decrement(uint8_t blockAddr, uint32_t value);
sta_result_t M1_Transfer(uint8_t blockAddr);
sta_result_t M1_Restore(uint8_t blockAddr);

sta_result_t piccRequestA(uint8_t *ATQA);
sta_result_t piccWakeupA(uint8_t *ATQA);
sta_result_t piccAntiA(uint8_t SEL, uint8_t rand_bit, uint8_t *uid);
sta_result_t piccSelectA(uint8_t SEL,uint8_t *uid, uint8_t *SAK);
void piccHaltA(void);
sta_result_t piccDeselectA(void);

sta_result_t piccRequestB(uint8_t ucReqCode, uint8_t ucAFI, uint8_t N, nAQTB_t *pATQB);
sta_result_t piccSlotMarker(uint8_t N, nAQTB_t *pATQB);
sta_result_t piccAttrib(uint8_t *pPUPI, uint8_t ucDSI_DRI,uint8_t MAX_FSDI,
				 uint8_t ucCID, uint8_t ucProType, uint8_t *pHigherLayerINF, uint8_t ucINFLen,
				 uint8_t *pAATTRIB, uint8_t *pRLen);
sta_result_t piccHaltB(uint8_t *pPUPI);
sta_result_t piccDeselectB(uint8_t *param);

//sta_result_t piccATS(uint8_t *ATS);
sta_result_t piccRATS(uint8_t param, uint8_t *ATS, uint16_t* AtsLength);
sta_result_t piccPPS(uint16_t high_rate, uint8_t *pps);

uint8_t ISO14443_3_Deal_Flow(uint8_t *rxBuff, uint8_t *rxSize);
sta_result_t ISO14443_Transcive(uint8_t *txBuf, uint16_t txSize, uint8_t *rxBuf, uint16_t *rxSize);
void PCD_PCB(void);

uint8_t mifare1_ReadTest(uint8_t *UID,uint8_t *roll);
sta_result_t mifare1_WriteTest(uint8_t *UID,uint8_t sectorN);

#endif
