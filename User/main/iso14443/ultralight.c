/******************************************************************************/
/*               (C) 斯凯瑞利（北京）科技有限公司(SKYRELAY)                   */
/*                                                                            */
/* 此代码是斯凯瑞利（北京）科技有限公司为支持客户编写的示例程序的一部分       */
/* 所有使用斯凯瑞利相关芯片的用户可以无偿使用该代码，但需保留本声明部分       */
/* 本代码仅为演示使用，不保证符合所有的应用要求，当有特定规范或需求时，       */
/* 使用者需自行确认代码是否符合要求，不合要求时自行做出相应的修改。           */
/* (V1.10)                                                                    */
/******************************************************************************/
/**
 ******************************************************************************
 ** 文 件 名： ultralight.c
 **
 ** 文件简述： 符合Mifare Ultralight和 Ultralight C卡操作的所有函数，常量和变量
 **
 ** 版本历史:
 ** 2019-05-30 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#include "iso14443.h"
#include "ultralight.h"

/* 加密指定长度的字符串 */
void encryptKey(uint8_t *keyData, uint8_t size)
{

}

/*解密指定长度字符串*/
void discryptKey(uint8_t *keyData, uint8_t size)
{


}
/**
 ******************************************************************************
 ** \简  述  ultralight c卡的加密验证,
 **
 ** \参  数
 **          uint8_t* Key 16字节密钥
 ** \返回值  验证状态
 ******************************************************************************/
sta_result_t ULC_Authentication(uint8_t *Key)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t i;
    uint8_t tmpBuf[20];
    uint8_t RndB[8], RndA[8]={0x78,0x98,0x01,0x83,0x46,0x76,0x31,0x52};
    sky1311TxCrcOn();
    sky1311RxCrcOn();
    tmpBuf[0] = ULC_AUTH;    // 0x1A
    tmpBuf[1] = 0;
    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,2,tmpBuf,&tmpSize);
    if(sta == Ok)
	{
	    if(tmpSize<9)
        {
            sta = Error;
            return sta;
        }
        else if(tmpBuf[0] == 0xAF)
        {
            discryptKey(&tmpBuf[1],8);
            for(i=0;i<8;i++)
            RndB[i] = tmpBuf[i+1];
        }
        else
        {
            sta = Error;
            return sta;
        }
        tmpBuf[0] = 0xAF;

        for(i=0;i<8;i++)
        {
            tmpBuf[i+1] = RndA[i];
            tmpBuf[i+9] =  RndB[i];
        }
        encryptKey(&tmpBuf[1],16); // 加密RndA || RndB
        sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,17,tmpBuf,&tmpSize);
        if(sta == Ok)
        {
            if(tmpSize==11)
            {
                encryptKey(RndA,8);
                for(i=0;i<8;i++)    //比较加密后的RndA和接收到的数据
                {
                    if(RndA[i] != tmpBuf[i+1])
                        sta = Error;
                }
            }
        }
	}
	return sta;
}


/**
 ******************************************************************************
 ** \简  述  ULC卡读数据，
 **
 ** \参  数  uint8_t address	要操作的地址
 **          uint8_t *receBuff  读取的数据存储地址（16字节）
 ** \返回值  操作状态
 ******************************************************************************/
sta_result_t ULC_Read(uint8_t address, uint8_t *receBuff)
{
	sta_result_t sta;
	uint16_t tmpSize;
	uint8_t tmpBuf[2];

	sky1311TxCrcOn();
	sky1311RxCrcOn();

	tmpBuf[0] = ULC_READ;    // 0x30
	tmpBuf[1] = address;

	sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,2,receBuff,&tmpSize);
	if(sta == Ok)
    {
        if(tmpSize < 16)
        {
        	switch (receBuff[0]&0x0f)
            {
            case ULC_NAK:
                sta = NoResponse;
                break;
            case ULC_NAKCRC:
                sta = RxCheckCRC;
                break;
            case ULC_NAKOTH:
                sta = UnknowError;
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
 ** \简  述  ultralight卡写数据。一次写入1页（4个字节）数据
 **
 ** \参  数  uint8_t address        要操作的页地址
             uint8_t* data           要写入的数据
 ** \返回值  操作状态 Ok表示成功， 其它表示有错误
 ******************************************************************************/
sta_result_t ULC_Write(uint8_t address, uint8_t* data)
{
    sta_result_t sta;
    uint8_t tmpBuf[16];
    uint16_t tmpSize;

    sky1311TxCrcOn();
    sky1311RxCrcOff();

    tmpBuf[0] = ULC_WRITE;       // CMD = 0xA0
    tmpBuf[1] = address;      //
    tmpBuf[2] = data[0];
    tmpBuf[3] = data[1];
    tmpBuf[4] = data[2];
    tmpBuf[5] = data[3];
    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,6,tmpBuf,&tmpSize);
    if(sta == Ok)
    {
        if((tmpBuf[0] &0x0f) != ULC_ACK)
        {
            sta = NoResponse;
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  ultralight c 卡兼容写数据。一次写入16个字节数据，只有最后4字节有效
 **
 ** \参  数  uint8_t pageAddress     要操作的页地址（2--27）
             uint8_t* data           要写入的数据
 ** \返回值  操作状态 Ok表示成功， 其它表示有错误
 ******************************************************************************/
sta_result_t ULC_CompWrite(uint8_t pageAddress, uint8_t* data)
{
    sta_result_t sta;
    uint8_t tmpBuf[16];
    uint16_t tmpSize;

    sky1311TxCrcOn();
    sky1311RxCrcOff();

    tmpBuf[0] = ULC_COMWRITE;       // CMD = 0xA0
    tmpBuf[1] = pageAddress;       //
    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,2,tmpBuf,&tmpSize);
    if(Ok == sta)
    {
        if((tmpBuf[0] & 0x0f) != M1_ACK)
        {
            sta = Error;
        }
        else
        {
            sta = ExchangeData(TYPE_A,RATE_OFF,data,16,tmpBuf,&tmpSize);
            if(sta == Ok)
             {
                if((tmpBuf[0] &0x0f) != M1_ACK)
                    return Error;
             }
        }
    }
    return sta;
}

