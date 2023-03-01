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
 ** 文 件 名： ntag.c
 **
 ** 文件简述： 符合NTAG2卡操作的所有函数，常量和变量
 **
 ** 版本历史:
 ** 2019-05-05 V1.00  EH   第一个正式版本
 ******************************************************************************/
#include "iso14443.h"
#include "ntag.h"
void sky1311RxCrcOn(void);
void sky1311RxCrcOff(void);
void sky1311TxCrcOn(void);
void sky1311TxCrcOff(void);


/**
 ******************************************************************************
 ** \简  述  获取NTAG21X/20X的版本号
 **
 ** \参  数  8字节版本号的内容
 ** \返回值  状态
 ******************************************************************************/
sta_result_t NTag_GetVision(uint8_t* data)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t tmpBuf[16];

    sky1311TxCrcOn();
    sky1311RxCrcOn();

    tmpBuf[0] = NTAG_GETVER;        // 0x60
    sta = ExchangeData(tmpBuf,1,data,&tmpSize);
    if(sta == Ok)
    {
        if(tmpSize < 8)
        {
            switch (data[0]&0x0f)
            {
            case NTAG_INVALIDADDR:
                sta = NTag_InvalidAddr;
                break;
            case NTAG_CHECKERROR:
                sta = NTag_CheckError;
                break;
            case NTAG_INVALIDAUTH:
                sta = NTag_InvalidAuth;
                break;
            case NTAG_WRITEERROR:
                sta = NTag_WriteError;
                break;
            default:
                sta = UnknowError;
                break;
            }
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  获取指定地址的内容
 **
 ** \参  数  指定的地址，16字节数据内容指针
 ** \返回值  状态
 ******************************************************************************/
sta_result_t NTag_Read(uint8_t addr, uint8_t* data)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t tmpBuf[4];

    sky1311TxCrcOn();
    sky1311RxCrcOn();
    tmpBuf[0] = NTAG_READ;        // 0x30
    tmpBuf[1] = addr;
    sta = ExchangeData(tmpBuf,2,data,&tmpSize);
    if(sta == Ok)
    {
        if(tmpSize < 16)
        {
            switch (data[0]&0x0f)
            {
            case NTAG_INVALIDADDR:
                sta = NTag_InvalidAddr;
                break;
            case NTAG_CHECKERROR:
                sta = NTag_CheckError;
                break;
            case NTAG_INVALIDAUTH:
                sta = NTag_InvalidAuth;
                break;
            case NTAG_WRITEERROR:
                sta = NTag_WriteError;
                break;
            default:
                sta = UnknowError;
                break;
            }
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  读取指定地址内容
 **
 ** \参  数  开始地址和结束地址，包含内容的数据指针。数据大小是n*4字节
 ** \返回值  状态
 ******************************************************************************/
sta_result_t NTag_FastRead(uint8_t starAddr,uint8_t endAddr, uint8_t* data)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t tmpBuf[4];

    sky1311TxCrcOn();
    sky1311RxCrcOn();
    tmpBuf[0] = NTAG_FASTREAD;        // 0x3A
    tmpBuf[1] = starAddr;
    tmpBuf[2] = endAddr;
    sta = ExchangeData(tmpBuf,3,data,&tmpSize);
    if(sta == Ok)
    {
        if(tmpSize != (endAddr-starAddr+1)*4)
        {
            switch (data[0]&0x0f)
            {
            case NTAG_INVALIDADDR:
                sta = NTag_InvalidAddr;
                break;
            case NTAG_CHECKERROR:
                sta = NTag_CheckError;
                break;
            case NTAG_INVALIDAUTH:
                sta = NTag_InvalidAuth;
                break;
            case NTAG_WRITEERROR:
                sta = NTag_WriteError;
                break;
            default:
                sta = UnknowError;
                break;
            }
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  NTAG写数据。一次写入4个字节数据
 **
 ** \参  数  uint8_t addr          要操作的地址
             uint8_t* data         要写入的数据指针
 ** \返回值  操作状态 Ok表示成功， 其它表示有错误
 ******************************************************************************/
sta_result_t NTag_PageWrite(uint8_t addr, uint8_t* data)
{
    sta_result_t sta;
    uint8_t tmpBuf[16];
    uint16_t tmpSize;

    sky1311TxCrcOn();
    sky1311RxCrcOff();

    tmpBuf[0] = NTAG_WRITE;       // CMD = 0xA2
    tmpBuf[1] = addr;
    tmpBuf[2] = data[0];
    tmpBuf[3] = data[1];
    tmpBuf[4] = data[2];
    tmpBuf[5] = data[3];
    sta = ExchangeData(tmpBuf,6,tmpBuf,&tmpSize);
    if(sta == Ok)
    {
        if((tmpBuf[0] &0x0f) != NTAG_ACK)
        {
            switch (tmpBuf[0]&0x0f)
            {
            case NTAG_INVALIDADDR:
                sta = NTag_InvalidAddr;
                break;
            case NTAG_CHECKERROR:
                sta = NTag_CheckError;
                break;
            case NTAG_INVALIDAUTH:
                sta = NTag_InvalidAuth;
                break;
            case NTAG_WRITEERROR:
                sta = NTag_WriteError;
                break;
            default:
                sta = UnknowError;
                break;
            }
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  读取NTAG卡的 counter 数值
 **
 ** \参  数  3字节数据内容指针
 ** \返回值  状态
 ******************************************************************************/
sta_result_t NTag_ReadCounter(uint8_t* data)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t tmpBuf[4];

    sky1311TxCrcOn();
    sky1311RxCrcOn();
    tmpBuf[0] = NTAG_READCNT;        // 0x39
    tmpBuf[1] = 0x02;
    sta = ExchangeData(tmpBuf,2,data,&tmpSize);
    if(sta == Ok)
    {
        if(tmpSize < 3)
        {
            switch (data[0]&0x0f)
            {
            case NTAG_INVALIDADDR:
                sta = NTag_InvalidAddr;
                break;
            case NTAG_CHECKERROR:
                sta = NTag_CheckError;
                break;
            case NTAG_INVALIDAUTH:
                sta = NTag_InvalidAuth;
                break;
            case NTAG_WRITEERROR:
                sta = NTag_WriteError;
                break;
            default:
                sta = UnknowError;
                break;
            }
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  NTAG卡密码验证
 **
 ** \参  数  uint8_t* password      4字节密码

 ** \返回值  操作状态 Ok表示成功， 其它表示有错误
 ******************************************************************************/
sta_result_t NTag_Authenticate(uint8_t* password)
{
    sta_result_t sta;
    uint8_t tmpBuf[16];
    uint16_t tmpSize;

    sky1311TxCrcOn();
    sky1311RxCrcOn();

    tmpBuf[0] = NTAG_PWDAUTH;       // CMD = 0x1B
    tmpBuf[1] = password[0];      //
    tmpBuf[2] = password[1];
    tmpBuf[3] = password[2];
    tmpBuf[4] = password[3];
    sta = ExchangeData(tmpBuf,5,tmpBuf,&tmpSize);
    if(sta == Ok)
    {
        if(tmpSize<2)
        {
            switch (tmpBuf[0]&0x0f)
            {
            case NTAG_INVALIDADDR:
                sta = NTag_InvalidAddr;
                break;
            case NTAG_CHECKERROR:
                sta = NTag_CheckError;
                break;
            case NTAG_INVALIDAUTH:
                sta = NTag_InvalidAuth;
                break;
            case NTAG_WRITEERROR:
                sta = NTag_WriteError;
                break;
            default:
                sta = UnknowError;
                break;
            }
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  读取NTAG卡的 ECC signature 数值
 **
 ** \参  数  4字节数据内容指针
 ** \返回值  状态
 ******************************************************************************/
sta_result_t NTag_ReadSign(uint8_t* data)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t tmpBuf[4];

    sky1311TxCrcOn();
    sky1311RxCrcOn();
    tmpBuf[0] = NTAG_READSIG;        // 0x39
    tmpBuf[1] = 0x00;
    sta = ExchangeData(tmpBuf,2,data,&tmpSize);
    if(sta == Ok)
    {
        if(tmpSize < 4)
        {
            switch (data[0]&0x0f)
            {
            case NTAG_INVALIDADDR:
                sta = NTag_InvalidAddr;
                break;
            case NTAG_CHECKERROR:
                sta = NTag_CheckError;
                break;
            case NTAG_INVALIDAUTH:
                sta = NTag_InvalidAuth;
                break;
            case NTAG_WRITEERROR:
                sta = NTag_WriteError;
                break;
            default:
                sta = UnknowError;
                break;
            }
        }
    }
    return sta;
}

