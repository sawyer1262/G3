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
 ** 文 件 名： ntag.h
 **
 ** 文件简述： 与NFC NTAG Type2卡有关的操作函数，变量以及常量定义
 **
 ** 版本历史:
 ** 2019-05-05 V1.00  Mark   正式版本
 ******************************************************************************/
#ifndef __NTAG2_H_
#define __NTAG2_H_


#include "sky1311t.h"
#include "sky1311_drv.h"


// command for NTAG card
#define     NTAG_GETVER     0x60
#define     NTAG_READ       0x30
#define     NTAG_FASTREAD   0x3A
#define     NTAG_WRITE      0xA2
#define     NTAG_COMPWRITE  0xA0
#define     NTAG_READCNT    0x39
#define     NTAG_PWDAUTH    0x1B
#define     NTAG_READSIG    0x3C

#define     NTAG_ACK            0x0A
#define     NTAG_INVALIDADDR    0x00
#define     NTAG_CHECKERROR     0x01
#define     NTAG_INVALIDAUTH    0x04
#define     NTAG_WRITEERROR     0x05

/**
 ******************************************************************************
 ** \简述： 函数声明
 **
 ******************************************************************************/
sta_result_t NTag_GetVision(uint8_t* data);
sta_result_t NTag_Read(uint8_t addr, uint8_t* data);
sta_result_t NTag_FastRead(uint8_t starAddr,uint8_t endAddr, uint8_t* data);
sta_result_t NTag_PageWrite(uint8_t addr, uint8_t* data);
sta_result_t NTag_ReadCounter(uint8_t* data);
sta_result_t NTag_Authenticate(uint8_t* password);
sta_result_t NTag_ReadSign(uint8_t* data);

#endif
