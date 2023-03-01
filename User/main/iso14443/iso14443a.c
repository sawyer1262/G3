/******************************************************************************/
/*               (C) 斯凯瑞利（北京）科技有限公司(SKYRELAY)                   */
/*                                                                            */
/* 此代码是斯凯瑞利（北京）科技有限公司为支持客户编写的示例程序的一部分       */
/* 所有使用斯凯瑞利相关芯片的用户可以无偿使用该代码，但需保留本声明部分       */
/* 本代码仅为演示使用，不保证符合所有的应用要求，当有特定规范或需求时，       */
/* 使用者需自行确认代码是否符合要求，不合要求时自行做出相应的修改。           */
/* (V1.30)                                                                    */
/******************************************************************************/
/**
 ******************************************************************************
 ** 文 件 名： iso14443_3a.c
 **
 ** 文件简述： 符合ISO14443-3 type A的操作函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 ** 2017-3-25  V.10  Release    更新代码和函数结构，重新发布
 ** 2018-09-02 V1.2.0 Release   修改了部分函数，改正防冲突算法错误
 ** 2019-08-02 V1.3.0 Beta      优化了代码，修复部分BUG，添加部分ISO14443-4部分
 ******************************************************************************/
#include "iso14443.h"
/**
 ******************************************************************************
 ** \简  述  type A "Request" 操作,command = 26H
 **
 ** \参  数  none
 ** \返回值  sta_result_t 操作状态，Ok：成功，其它：失败
 ******************************************************************************/
sta_result_t piccRequestA(uint8_t *ATQA)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t tmpBuf[1];
    tmpBuf[0] = REQA;       // 0x26
    sky1311WriteReg(ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);      // TX odd parity, no CRC
    sky1311WriteReg(ADDR_RX_CTRL, RX_PARITY_EN|RX_PARITY_ODD);      // RX odd parity, no CRC
    sky1311WriteReg(ADDR_TX_BIT_NUM, 0x07);
    //sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x01);

    //sky1311_fifo_tx(TYPE_A, tmpBuf, 1);
    //sta = sky1311_fifo_rx(TYPE_A, RATE_OFF, ATQA, &tmpSize);
    sta = ExchangeData(tmpBuf,1,ATQA,&tmpSize);

    if(sta == Ok && tmpSize == 2)
        return Ok;
    else
        return NoResponse;
}

/**
 ******************************************************************************
 ** \简  述  type A "Wake-Up" 操作,command = 52H
 **
 ** \参  数  none
 ** \返回值  sta_result_t 操作状态，Ok：成功，其它：失败
 ******************************************************************************/
sta_result_t piccWakeupA(uint8_t *ATQA)
{
    sta_result_t sta;
    uint16_t tmpSize;
    uint8_t tmpBuf[1];

    tmpBuf[0] = WUPA;       // 0x52
    sky1311WriteReg(ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);      // TX odd parity, no CRC
    sky1311WriteReg(ADDR_RX_CTRL, RX_PARITY_EN|RX_PARITY_ODD);      // RX odd parity, no CRC
    sky1311WriteReg(ADDR_TX_BIT_NUM, 0x07);
		
    sta = ExchangeData(tmpBuf,1,ATQA,&tmpSize);
    return sta;
}
/**
 ******************************************************************************
 ** \简  述  type A卡防冲突循环
 **
 ** \参  数  SEL,  (93h, 95h, 97h)
 **          uint8_t randBit 位冲突时选择的数，0或1;
 **          uint8_t* uid 获得的UID保存区域的首地址
 ** \返回值  return Ok on success, Error on error
 ******************************************************************************/
sta_result_t piccAntiA(uint8_t SEL, uint8_t rand_bit, uint8_t *uid)
{
    uint8_t NVB= 0x20;
    uint8_t currValidBytes=0, currValidBits=0;
    uint8_t recBytes, recBits;
    uint8_t hasCollision = 0;
		uint8_t tmpSize = 0;
		uint8_t tmpBuf[12];
		uint8_t g_rx_buff[12];
    uint8_t i;
    uint8_t recBit8;

    /* config registor of sky1311 */
    sky1311WriteReg(ADDR_RX_NUM_H, COLL_EN);        // anti-collision on
    sky1311WriteReg(ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);
    sky1311WriteReg(ADDR_RX_START_BIT_NUM,0);
    /* STEP 1: send SEL + 20H to PICC */
    tmpBuf[0] = SEL;
    tmpBuf[1] = NVB;
    if(Ok != bitCollisionTrans(tmpBuf, 2 , 8)){
        sky1311WriteReg(ADDR_RX_NUM_H, 0x00);               // disable anti-collision
        return Error;
    }
    delay(5);
    do{
        /* 读出PICC返回的数据字节数和不完整的位数，将数据保存到缓冲区 */
        recBytes = sky1311ReadReg(ADDR_RX_NUM_L);
        recBit8 = sky1311ReadReg(ADDR_RX_NUM_H);
        recBits = (recBit8&0xf0)>>4;
        //recBits = ((sky1311ReadReg(ADDR_RX_NUM_H) & 0xf0) >> 4);
        tmpSize = sky1311ReadReg(ADDR_FIFO_LEN);
        sky1311ReadFifo(g_rx_buff, tmpSize);
        if((sky1311ReadReg(ADDR_RX_NUM_H) & RX_FORBID) == 0)   // no collision happens
            hasCollision = 0;
        else
            hasCollision = 1;

        /* 将收到的UID数据和之前的数据拼接合并 */
        if(currValidBits == 0){         // 当前UID全部是完整的字节，按字节拼接
            for(i=0;i<tmpSize;i++){
                uid[currValidBytes++] = g_rx_buff[i];
            }
            currValidBits = recBits;
        }
        else{                           // 当前UID最后一个字节不完整，第一个字节需要按位拼接
             if(recBytes == 0){              // 仅收到一个字节新数据（可能也不完整）
                uid[currValidBytes-1] += g_rx_buff[0]<<currValidBits;    // 按位拼接第一个字节
                currValidBits += recBits;
                if(currValidBits==8){
                    currValidBytes++;
                    currValidBits = 0;
                }
            }
            else{                           // 收到多个字节数据
                uid[currValidBytes-1] |= g_rx_buff[0];//<<currValidBits;   // 按位拼接第一个字节
                for(i=1;i<tmpSize;i++){           // 复制剩余的字节
                    uid[currValidBytes++] = g_rx_buff[i];
                }
                currValidBits = recBits;
            }
        }
        if(hasCollision){

            /* 将UID数据加上rand_bit */
            if(currValidBits==0){
                uid[currValidBytes] = rand_bit;
                currValidBits = 1;
                NVB = 0x20+((currValidBytes)<<4) |  currValidBits;
                currValidBytes++;
            }
            else if(currValidBits == 7){
                uid[currValidBytes-1] += rand_bit<<7;
                currValidBits = 0;
                NVB = 0x20+((currValidBytes)<<4);
            }
            else{       // currValidBits= 1 ... 6
                uid[currValidBytes-1] += rand_bit<<currValidBits;
                currValidBits++;
                NVB = 0x20+((currValidBytes-1)<<4) |  currValidBits;
            }

            tmpBuf[0] = SEL;
            tmpBuf[1] = NVB;
            for(uint8_t i=0;i<currValidBytes;i++){
                tmpBuf[2+i] = uid[i];
            }
            sky1311WriteReg(ADDR_RX_START_BIT_NUM,currValidBits);
            if(Ok != bitCollisionTrans(tmpBuf, currValidBytes+2 , currValidBits)){
                sky1311WriteReg(ADDR_RX_NUM_H, 0x00);// disable anti-collision
                sky1311WriteReg(ADDR_RX_START_BIT_NUM,0);
//							Dprint("piccAntiA :Error2\r\n");
                return Error;
            }
        }

    }while(hasCollision);
    sky1311WriteReg(ADDR_RX_NUM_H, 0x00); // disable anti-collision
    sky1311WriteReg(ADDR_RX_START_BIT_NUM,0);
    return Ok;
}

/**
 ******************************************************************************
 ** \简  述  type A选卡
 **
 ** \参  数  uint8_t SEL, 选择序列，93H，95H， 97H
 **          uint8_t *uid, PICC 4 bytes UID
 **          uint8_t *SAK
 ** \返回值  sta_result_t， 选卡状态值
 ******************************************************************************/
sta_result_t piccSelectA(uint8_t SEL,uint8_t *uid, uint8_t *SAK)
{
    sta_result_t sta;
		uint16_t tmpSize;
		uint8_t tmpBuf[10];

    sky1311WriteReg(ADDR_TX_CTRL, TX_CRC_EN|TX_PARITY_ODD|TX_POLE_HIGH); // TX odd parity, with CRC
    sky1311WriteReg(ADDR_RX_CTRL, RX_CRC_EN|RX_PARITY_EN|RX_PARITY_ODD);
    sky1311WriteReg(ADDR_TX_BIT_NUM, 0x08);                              // complete bytes

    tmpBuf[0] = SEL;
    tmpBuf[1] = 0x70;
    tmpBuf[2] = uid[0];
    tmpBuf[3] = uid[1];
    tmpBuf[4] = uid[2];
    tmpBuf[5] = uid[3];
    tmpBuf[6] = uid[4];

    sta = ExchangeData(tmpBuf,7,SAK,&tmpSize);
    if(sta == Ok && tmpSize == 3)
        return Ok;
    else
        return NoResponse;
}
/**
 ******************************************************************************
 ** \简  述  type A休眠,终止对卡的操作
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void piccHaltA(void)
{
    sky1311WriteReg(ADDR_FIFO, 0x50);
    sky1311WriteReg(ADDR_FIFO, 0x00);
    //sky1311WriteReg(ADDR_TX_BYTE_NUM, 0x02);
    sky1311WriteCmd(CMD_TX);                     // CMD TX
}

/****************************Part of ISO14443-4********************************/

uint8_t  g_FWI;                 //Frame Waiting time Integer
uint8_t  g_bCID;                //Card IDentifier
uint16_t g_bFSD;                //Frame Size for proximity coupling Device
uint16_t g_bFSC;                //Frame Size for proximity Card
uint8_t  g_bCIDFollowed;        //CID Followed
uint8_t  g_bNADFollowed;        //NAD followed

uint8_t  g_bBlockNum;           // block number in PCB
uint8_t  g_PCB;                 // PCB for current time
uint8_t  g_bLastTransmitPCB;    //PCB of last transmit
uint8_t  * g_pbaLastTransmitBuf;
uint16_t g_wLastTransmitLength;
const uint16_t FSI[9] = {16, 24, 32, 40, 48, 64, 96, 128, 256};
/**
 ******************************************************************************
 ** \简  述  取消选卡
 **
 ** \参  数  卡类型
 ** \返回值  sta_result_t，状态
 ******************************************************************************/
sta_result_t piccDeselectA(void)
{
    sta_result_t sta;
		uint16_t tmpSize;
		uint8_t tmpBuf[2];
		uint8_t g_rx_buff[4];

    tmpBuf[0] = 0xCA;
    tmpBuf[1] = 0x00;

	//sky1311_fifo_tx(TYPE_A, tmpBuf, 2);
	//sta = sky1311_fifo_rx(TYPE_A, RATE_OFF, g_rx_buff, &tmpSize);
    sta = ExchangeData(tmpBuf,2,g_rx_buff,&tmpSize);

    if(sta == Ok && tmpSize == 4)
        return Ok;
    else
        return NoResponse;
}

void PCD_PCB(void)
{
    g_PCB ^=1;      // change block number
}
/**
 ******************************************************************************
 ** \简  述  读取type A卡的ATS值
 **
 ** \参  数  none
 ** \返回值  sta_result_t，ATS值
 ******************************************************************************/
/*
 * request ATS from card
 * ATS = TL + T0 + TA + TB + TC + T1 ... +TK + CRC1 + CRC2
*/
sta_result_t piccRATS(uint8_t param, uint8_t *ATS, uint16_t* AtsLength)
{
		uint8_t count;
    sta_result_t sta;
    //uint16_t tmpSize;
    uint8_t tmpBuf[2];
    count = 2;
    g_FWI = 4;
    tmpBuf[0] = 0xE0;
    tmpBuf[1] = param;      //default value is 0x80

    sta = ExchangeData(tmpBuf,2,ATS,AtsLength);
    if(sta == Ok)//if(sta == Ok && (tmpSize == ATS[0] + 2))
    {
        g_bFSD = FSI[(param>>4)&0x0F];  //get FSD
        g_bCID = param & 0x0F;          //get CID

        g_bFSC = FSI[ATS[1]&0x0F];      //get FSC, FSCI = T0[b3..b0]

        if(ATS[1] & 0x10)   // indicate TA is exist
        {
            count++;
        }

        if(ATS[1] & 0x20)   // indicate TB is exist
        {
            count++;
            g_FWI = ((ATS[3] >> 4) & 0x0F);
            if(g_FWI > 14)
            {
                g_FWI = 14;
            }
        }

        if(ATS[1] & 0x40)   // indicate TC is exist
        {
            count++;
            if(ATS[count] & 0x02)
            {
                g_bCIDFollowed = 1;
            }
            else
            {
                g_bCIDFollowed = 0;
            }
            if(ATS[count] & 0x01)
            {
                g_bNADFollowed = 1;
            }
            else
            {
                g_bNADFollowed=0;
            }
        }
        else
        {
            g_bCIDFollowed = 0;
        }
        g_bBlockNum = 0;
        g_PCB = g_bLastTransmitPCB = 0x02;
/*  //delete 20201223 flow by CID or NAD error
        if(g_bCIDFollowed)
            g_PCB = g_bLastTransmitPCB |= 0x08;
        if(g_bNADFollowed)
            g_PCB = g_bLastTransmitPCB |= 0x04;
*/
        //frame_delay = 32 * (2 << g_FWI) + 10;
        //timer0_value = (uint8_t)(frame_delay & 0x0000ff);
        //timer1_value = (uint8_t)((frame_delay >> 8) & 0x0000ff);
        //timer2_value = (uint8_t)((frame_delay >> 16) & 0x00003f);
        //timer2_value = timer2_value + 0x40;
    }
    else
        return NoResponse;
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  读取PPS值
 **
 ** \参  数  设置的倍速
 ** \返回值  sta_result_t，PPS值
 ******************************************************************************/
sta_result_t piccPPS(uint16_t high_rate, uint8_t *pps)
{
    sta_result_t sta;
    uint8_t tmpBuf[3]={0xD0,0x11,0x00};
    uint16_t tmpSize;

    if(high_rate == 212)
        tmpBuf[2] = 0x05;                   // RATE = 212k
    else if(high_rate == 424)
        tmpBuf[2] = 0x0A;                   // RATE = 424k

    //sky1311_fifo_tx(TYPE_A, tmpBuf, 3);
	//sta = sky1311_fifo_rx(TYPE_A, RATE_OFF, pps, &tmpSize);
    sta = ExchangeData(tmpBuf,3,pps,&tmpSize);

    return sta;
}




