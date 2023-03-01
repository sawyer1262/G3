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
 ** 文 件 名： SmartCard.c
 **
 ** 文件简述： 符合CPU卡操作所有函数，常量和变量
 **
 ** 版本历史:
 ** 2019-07-09 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/

#include "iso14443.h"
#include "smartcard.h"
/**
 ******************************************************************************
 ** \简  述  向CPU卡发送 I-Block 数据，并接收返回数据，发送失败重发一次
 **
 ** \参  数
 **         sendLen:发送的数据长度
 **		    sendBuff:发送的指令数据指针
 **         recLen: 接收数据长度指针
 **         recBuff: 接收数据指针
 ** \返回值  操作状态
 ******************************************************************************/
sta_result_t CPU_I_Block(uint8_t sendLen,uint8_t *sendBuff,uint16_t *recLen,uint8_t *recBuff)
{
    sta_result_t sta;
    uint8_t length;
    uint8_t i;
    setPCDTimeOut(1000);  // timeout 1000ms
    for(i=0;i<2;i++)
    {
        length=1;
        sky1311WriteFifo(&g_PCB,1);
        if (g_PCB & 0x08)//判断是否有CID字节
        {
            sky1311WriteFifo(&g_bCID,1);
            length=2;
        }
        sky1311_fifo_tx(sendBuff, sendLen+length);
        sta = sky1311_fifo_rx(recBuff, recLen);
        if(sta==Ok)
        {
            PCD_PCB();
            break;
        }
    }
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  向CPU卡发送 R-Block 数据，并接收返回数据，发送失败重发一次
 **
 ** \参  数
 **         recLen: 接收数据长度指针
 **         recBuff: 接收数据指针
 ** \返回值  操作状态
 ******************************************************************************/
sta_result_t CPU_R_Block(uint8_t *recBuff,uint16_t *recLen)
{
    sta_result_t sta;
 	uint8_t sendBuff[2];
    uint8_t i;
	setPCDTimeOut(1000);            // timeout 1000ms
    for(i=0;i<2;i++)
    {
        sky1311WriteCmd(CMD_CLR_FF);    // clear FIFO
        if (g_PCB & 0x08)              //判断是否有CID
        {
            sendBuff[0] = (g_PCB & 0x0F) | 0xA0;
            sendBuff[1] = g_bCID;
            sta = ExchangeData(sendBuff,2,recBuff, recLen);
        }
        else
        {
            sendBuff[0] = (g_PCB & 0x0F) | 0xA0;
            sta = ExchangeData(sendBuff,1,recBuff, recLen);
        }
        if (sta==Ok)
        {
            PCD_PCB();
            break;
        }
    }
	return sta;
}
/**
 ******************************************************************************
 ** \简  述  向CPU卡发送 S-Block 数据，并接收返回数据
 **
 ** \参  数
 **         PCB_byte:设置PCB字节
 **         recLen: 接收数据长度指针
 **         recBuff: 接收数据指针
 ** \返回值  操作状态
 ******************************************************************************/
uint8_t CPU_S_Block(uint8_t PCB_byte,uint16_t *recLen,uint8_t *recBuff)
{
    sta_result_t sta;
 	uint8_t sendBuff[3];
	setPCDTimeOut(1000);            // timeout 1000ms
    sky1311WriteCmd(CMD_CLR_FF);    // clear FIFO
    sendBuff[0] =0xC0|PCB_byte;
    if ((g_PCB&0x08)==0x08)     //判断是否有CID字节
    {
        if((PCB_byte&0xF0)==0xF0) //WTX
        {
		  	sendBuff[1] = g_bCID;
			sendBuff[2] = 0x01;
			sta=ExchangeData(sendBuff,3,recBuff,recLen);
		}
		else                    //DESELECT
		{
			sendBuff[1] = g_bCID;
			sta=ExchangeData(sendBuff,2,recBuff,recLen);
		}
	}
	else
	{
		if ((PCB_byte&0xF0)==0xF0) //WTX
        {
		  	sendBuff[1] = 0x01;
			sta=ExchangeData(sendBuff,2,recBuff,recLen);
		}
		else                    //DESELECT
		{
			sendBuff[1] = 0x00;
			sta=ExchangeData(sendBuff,2,recBuff,recLen);
		}
	}
	if(sta==Ok)
	{
			PCD_PCB();
	}
	return sta;
 }



