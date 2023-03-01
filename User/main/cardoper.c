#include "sky1311_drv.h"
#include "sky1311t.h"
#include "iso14443.h"


/*****************************************************************************/
/*****************************************************************************/
/***************************************************************
    M1卡读取扇区操作示例
   读取扇区数据并通过串口打印出来
   寻卡等操作时，显示操作结果
   有错误时显示错误信息
****************************************************************/
sta_result_t mifare1_ReadTest(uint8_t *UID)
{
    sta_result_t sta;
    uint8_t tmpBuf[20];
    uint8_t default_key[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t sector_num=0, block_num=0;

    /* read test Mifare 1K card (0--63 block) */
    for(sector_num=0;sector_num<16;sector_num++)// read sector0--16
    {
        sta = M1_Authentication(M1_AUTH_KEYA, default_key , UID, block_num);

        if(Ok == sta)
        {
            for(uint8_t i=0;i<4;i++)
            {
                sta = M1_Read(block_num, tmpBuf);
                if(sta==Ok)
                {
                    block_num++;

                }
            }
        }
        else
        {
            sky1311WriteReg(ADDR_M1_CTRL, 0x00);
            return sta;
        }
        sky1311WriteReg(ADDR_M1_CTRL, 0x81);    // next Authentication
    }
    //sky1311Reset();
    return Ok;
}

/***************************************************************
    M1卡写入操作示例
    M1卡写入Block4 (Sector 1) 和Block8（Sector2）操作。
   寻卡等操作时，显示操作结果
   有错误时显示错误信息
****************************************************************/
sta_result_t mifare1_WriteTest(uint8_t *UID)
{
    sta_result_t sta;
    uint8_t default_key[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t i;
    uint8_t writeData[] = "ABCDEF1234567890";
    uint8_t keyb[]={1,2,3,4,5,6};
    uint8_t keya[]={0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF };

#if DEBUG==1
    uart_puts("\r\nWrite Mifare1 test\r\n");
#endif

    sta = M1_Authentication_Auto(M1_AUTH_KEYA, default_key, UID, 7);
    if(Ok==sta)
    {
        sta = M1_PwdChange(7,keya,keyb);
        if(Ok == sta)
        {
            sky1311WriteReg(ADDR_M1_CTRL, 0x81);
        }
    }
    else
    {
        piccHaltA();
        sky1311WriteReg(ADDR_M1_CTRL, 0x00);
        return Error;

    }
    for(i=0;i<3;i++)
    {
        irq_sta = sky1311ReadReg(ADDR_M1_CTRL);

        uint8_t block_num = 4+i;
        sta = M1_Write(block_num, writeData);
#if DEBUG==1
        uart_puts("Block write ");
        uart_putHex(block_num);
#endif
        if(Ok == sta)
        {
#if DEBUG==1
            uart_puts("Success\r\n");
#endif
        }
        else{
#if DEBUG==1
            uart_puts("Failed\r\n");
#endif
        }

    }
    return Ok;
}

/***************************************************************
    CPU卡操作示例
   读取CPU卡的UID等，并使用部分APDU做测试
   寻卡等操作时，显示操作结果
   有错误时显示错误信息
****************************************************************/
sta_result_t smartTypeA_test(void)
{
    sta_result_t sta;
    uint8_t tmpBuf[100];
    uint16_t tmpSize;

#if DEBUG==1
    uart_puts("\r\nSmart Type A test\r\n");
#endif
    sta = piccATS(tmpBuf);
    if(sta == NoResponse){
        sky1311Reset();
#if DEBUG==1
        uart_puts("\r\nRATS error!\r\n");
#endif
        return ErrorAts;
    }
    else{
#if DEBUG==1
    	uart_puts("RATS : ");
    	uart_printBuffer(tmpBuf,32);
        uart_newrow();
#endif
    }
#if 0
    /* 00 a4 04 00 00 */
    tmpBuf[0] = 0x02;
    tmpBuf[1] = 0x00;
    tmpBuf[2] = 0xA4;   // command: select file
    tmpBuf[3] = 0x04;
    tmpBuf[4] = 0x00;
    tmpBuf[5] = 0x00;

    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,6,tmpBuf,&tmpSize);
    if(sta == NoResponse)
    {
        sky1311Reset();
    	return NoResponse;
    }
    else{
#if DEBUG==1
        uart_puts("Select SD:");
    	uart_printBuffer(tmpBuf,tmpSize-2);
        uart_newrow();
#endif
    }
#endif

     /* select MF file */
    tmpBuf[0] = 0x02;
    tmpBuf[1] = 0x00;
    tmpBuf[2] = 0xA4;   // command: select file
    tmpBuf[3] = 0x00;
    tmpBuf[4] = 0x00;
    tmpBuf[5] = 0x02;
    tmpBuf[6] = 0x3F;
    tmpBuf[7] = 0x00;   // MF
    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,8,tmpBuf,&tmpSize);
    if(sta == NoResponse)
    {
        sky1311Reset();
    	return NoResponse;
    }
    else{
#if DEBUG==1
        uart_puts("Select MF:");
    	uart_printBuffer(tmpBuf,tmpSize-2);
        uart_newrow();
#endif
    }
#if 0
    /*获取随机数*/
    tmpBuf[0] = 0x02;
    tmpBuf[1] = 0x00;
    tmpBuf[2] = 0x84;
    tmpBuf[3] = 0x00;
    tmpBuf[4] = 0x00;
    tmpBuf[5] = 0x08;
    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,6,tmpBuf,&tmpSize);
    if(sta == NoResponse)
    {
        sky1311Reset();
    	return NoResponse;
    }
    else{
#if DEBUG==1
        uart_puts("RM:");
    	uart_printBuffer(tmpBuf,tmpSize-2);
        uart_newrow();
#endif
    }
#endif

#if 0
    /* 读一卡通有关的信息 */
    tmpBuf[0] = 0x03;
    tmpBuf[1] = 0x00;
    tmpBuf[2] = 0xA4;
    tmpBuf[3] = 0x00;
    tmpBuf[4] = 0x00;
    tmpBuf[5] = 0x02;
    tmpBuf[6] = 0x10;
    tmpBuf[7] = 0x01;

    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,8,tmpBuf,&tmpSize);
    if(sta == NoResponse)
    {
        sky1311Reset();
    	return NoResponse;
    }
    else{
#if DEBUG==1
        uart_puts("Read Card Info:");
    	uart_printBuffer(tmpBuf,tmpSize-2);
        uart_newrow();
        uart_puts("Card No: ");
        DecNum = (uint32_t) tmpBuf[40]<<24;
        DecNum += (uint32_t) tmpBuf[39]<<16;
        DecNum += (uint32_t) tmpBuf[38]<<8;
        DecNum += (uint32_t) tmpBuf[37];

        Dec2Str(DecNum,tmpBuf,&DecLength);
        uart_sendData(tmpBuf,DecLength);
        uart_newrow();

#endif
    }
    /* 读余额 */
    tmpBuf[0] = 0x02;
    tmpBuf[1] = 0x80;
    tmpBuf[2] = 0x5C;
    tmpBuf[3] = 0x00;
    tmpBuf[4] = 0x02;
    tmpBuf[5] = 0x04;

    sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,6,tmpBuf,&tmpSize);
    if(sta == NoResponse)
    {
        sky1311Reset();
    	return NoResponse;
    }
    else{
#if DEBUG==1
        uart_puts("Read Balance:");
    	uart_printBuffer(tmpBuf,tmpSize-2);
        uart_newrow();
        uart_puts("Balance: ￥");
        DecNum = (uint32_t) (tmpBuf[1]&0x0f)<<24;
        DecNum += (uint32_t) tmpBuf[2]<<16;
        DecNum += (uint32_t) tmpBuf[3]<<8;
        DecNum += (uint32_t) tmpBuf[4];
        uint8_t decimal = DecNum%100;
        DecNum = DecNum/100;
        Dec2Str(DecNum,tmpBuf,&DecLength);
        uart_sendData(tmpBuf,DecLength);
        uart_putChar('.');
        Dec2Str(decimal,tmpBuf,&DecLength);
        uart_sendData(tmpBuf,DecLength);
        uart_newrow();

#endif
    }

    for(i=1;i<=10;i++)
    {
        /* 读取交易记录 */
        tmpBuf[0] = 0x02;
        tmpBuf[1] = 0x00;   // CLA Class
        tmpBuf[2] = 0xB2;   // INS Instruction
        tmpBuf[3] = i;//0x01;   // P1 Parameter 1
        tmpBuf[4] = 0xC4;//0xC5;   // P2 Parameter 2
        tmpBuf[5] = 0x00;   // Le

        sta = ExchangeData(TYPE_A,RATE_OFF,tmpBuf,6,tmpBuf,&tmpSize);
        if(sta == NoResponse)
        {
            return NoResponse;
        }
        else{
    #if DEBUG==1
            uart_puts("Read Record:");
            uart_printBuffer(tmpBuf,tmpSize-2);
            uart_newrow();
    #endif
        }
    }
#endif //if 0
    sky1311Reset();
    //LED_OFF();
    return Ok;
}
/******************************************************************************/
/*  Type A PICC test */
sta_result_t TypeA_test(void)
{
    sta_result_t sta;
    uint8_t SAK=0;
    uint8_t ATQA[2]={0};
    uint8_t UID[12]={0};
    uint8_t *uid;
    uint8_t sel=SEL1;
    uint8_t uidSize=0;
    uint8_t tmpBuf[10];
    uint8_t i;
    SKY1311_ENABLE(); 	    // chip enable PD2
    sky1311Init();			// init sky1311s
    //resetPicc();            // 复位卡片()
    DelayMS(10);             // 等待卡片工作
    typeAOperate();
    //sta = piccRequestA(tmpBuf);     // send REQA command to card
    sta = piccWakeupA(tmpBuf);        // send WUPA command to card
    if(Ok == sta)
    {
        LED_ON();
        ATQA[0] = tmpBuf[1];
        ATQA[1] = tmpBuf[0];
        uidSize = tmpBuf[0]>>6;
#if DEBUG==1
        uart_puts("\r\nType-A Card Test\r\n");
        uart_puts("ATQA:");
        uart_printBuffer(ATQA,2);
        uart_newrow();

#endif
    }
    else
    {
        sky1311Reset();
        return ErrorRequest;            // 这里返回，有可能是没卡或者没读到
    }
    uid = &UID[0];
    sel = SEL1;
    do{
        sta = piccAntiA(sel, 1, uid);   // 防碰撞循环，测试获得一张卡的UID，参数'1'表示遇到冲突时选择1
        if(Ok == sta)
        {
            uint8_t checkSum = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
            if(checkSum != uid[4])
            {
#if DEBUG==1
                uart_puts("\r\nAnti error!\r\n");
#endif
                sky1311Reset();
                return ErrorAnticollision;
            }

#if DEBUG==1
            //uart_puts("UID:");
            //uart_printBuffer(UID,4);
            //uart_newrow();
#endif
        }
        else
        {
            sky1311Reset();
#if DEBUG==1
            uart_puts("\r\nAnti error!\r\n");
#endif
            return ErrorAnticollision;
        }
        sta = piccSelectA(sel,uid, tmpBuf);        // Select PICC
        if(Ok == sta)
        {
            SAK = tmpBuf[0];
#if DEBUG==1
            //uart_puts("SAK:");
            //uart_printBuffer(tmpBuf,1);
#endif
        }
        else
        {
#if DEBUG==1
            uart_puts("\r\nSelect error!\r\n");
#endif
            return ErrorSelect;
        }


        if(uidSize!=0 && uid[0] == 0x88)
        {
            for(i=0;i<3;i++){
                uid[i] = uid[i+1];
            }
            uid += 3;
            sel += 2;

        }
    }while( (SAK&0x04)!=0 );

#if DEBUG==1
    uart_puts("UID:");
    uart_printBuffer(UID,10);
    uart_newrow();
    uart_puts("SAK:");
    uart_printBuffer(tmpBuf,1);

#endif
#if CARDTYPE==AUTO

    if((SAK & 0x24) == 0x20){           //UID complete, PICC compliant with ISO/IEC 14443-4
        sta = smartTypeA_test();
    }
    else if((SAK==0x08 || SAK == 0x18) && ((ATQA[1]&0xB4) == 0x04 || (ATQA[1]&0xB2) == 0x02)){// M1 card
        //sta = mifare1_WriteTest(UID);
        sta = mifare1_ReadTest(UID);
    }
#elif CARDTYPE==ONLYM1
    sta = mifare1_ReadTest(UID);
#elif CARDTYPE==ONLYSMART
    sta = smartTypeA_test();
#elif CARDTYPE==SKY1311TEST
    sta = sky1311_ReadTest();
#endif
    sky1311Reset();
    return sta;
}

/***************************************************************
    TYPE B卡操作示例 （以读身份证串号为例）
   读取TYPE B卡的UID等，并使用部分APDU做测试
   寻卡等操作时，显示操作结果
   有错误时显示错误信息
****************************************************************/
sta_result_t TypeB_test(void)
{
    sta_result_t sta;

    nAQTB_t ATQB;
    uint8_t tmpBuf[100];
    uint16_t tmpSize;
    uint8_t  attSize;

    SKY1311_ENABLE(); 	    // chip enable PD2
    //DelayMS(5);
    sky1311Init();			// init sky1311s
    //resetPicc();            // 复位卡片
    DelayMS(10);             // 等待卡片工作
    typeBOperate();

    sta = piccRequestB(REQALL,0, 0, &ATQB);
    if(Ok != sta)
    {
        sky1311Reset();
        return NoResponse;
    }
    uart_puts("\r\nType-B Card test\r\n");
    LED_ON();
    sta = piccAttrib(ATQB.PUPI, 0, 8, 0, 1, tmpBuf, 0, tmpBuf, &attSize);
    if(Ok != sta)
    {
        sky1311Reset();
        return NoResponse;
    }

    /* 读身份证ID  ISO14443-4 */
    tmpBuf[0] = 0x00;
    tmpBuf[1] = 0x36;
    tmpBuf[2] = 0x00;
    tmpBuf[3] = 0x00;
    tmpBuf[4] = 0x08;
    //sky1311_fifo_tx(TYPE_B, tmpBuf, 5);
    //sta = sky1311_fifo_rx(TYPE_B, RATE_OFF, tmpBuf, &tmpSize);
    sta = ExchangeData(TYPE_B,RATE_OFF,tmpBuf,5,tmpBuf,&tmpSize);
    if(sta == NoResponse)
    {
        sky1311Reset();
    	return NoResponse;
    }
    else{
        uart_puts("ID Card:");
        uart_printBuffer(tmpBuf,tmpSize-2);
        uart_newrow();
    }
    sky1311Reset();
    return Ok;
}
