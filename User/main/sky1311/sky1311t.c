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
 ** 文 件 名： sky1311T.c
 **
 ** 文件简述： 操作sky1311s的所有函数，常量和变量
 **
 ** 版本历史:
 ** 2015-10-05 V1.00  EH   第一个正式版本
 ** 2017-3-25  V.10  Release    更新代码和函数结构，重新发布
 ** 2018-06-12 V1.1.2 Release   增加数据收发函数，修改部分错误，重新整理发布
 ** 2018-09-02 V1.2.0 Release   修改了多处错误，增加了数据收发函数，修改了部分函数的结构和实现方式
 ** 2018-09-19 V1.2.1 Release   检卡时关闭所以中断源，使其它中断不干扰检卡
 ** 2019-08-02 V1.3.0 Beta      修改了一些错误，精简了发送和接收函数，添加了超时函数
 ** 2020-04-27 V1.3.0 Release   超时和开关场函数修改， 增加设置水线函数
 ** 2020-11-17 V1.3.1 Release   修改bitRateSet 函数设置多倍速，参数优化
 ** 2021-08-24 V1.3.2 Release   修改接收函数，判断错误状态寄存器时判断位错，应该与0xC0
 ** 2021-08-27 V1.3.2 Release   修改sky1311TxCrcOff函数，把之前 或的(!TX_CRC_EN)去掉
          
 ******************************************************************************/
#include "sky1311_drv.h"
#include "sky1311t_reg.h"
#include "timer.h"

 

/**
 ******************************************************************************
 ** \简述： 全局变量定义
 **
 ******************************************************************************/
#if CHECKCARD==1
    uint8_t maxRCADVal=MAXADVAL;         // 检测到的最大AD值
    uint8_t freqScanNeed=TRUE;  // 系统需要RC频率扫描
#endif

uint8_t  err_sta;   // 错误状态代码
uint8_t  irq_sta;   // 中断请求状态代码

#define     FIFO_LEVEL              16
/**
 ******************************************************************************
 ** \简  述  操作M1卡时打开接收的CRC校验(sky1311s)
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void sky1311RxCrcOn(void)
{
    sky1311WriteReg(ADDR_RX_CTRL, RX_MIFARE_ON|RX_CRC_EN|RX_PARITY_EN|RX_PARITY_ODD);
}
/**
 ******************************************************************************
 ** \简  述  操作M1卡时关闭接收的CRC校验(sky1311s)
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void sky1311RxCrcOff(void)
{
    sky1311WriteReg(ADDR_RX_CTRL, RX_MIFARE_ON|RX_PARITY_EN|RX_PARITY_ODD);
}
/**
 ******************************************************************************
 ** \简  述  操作M1卡时打开发送的CRC校验(sky1311s)
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void sky1311TxCrcOn(void)
{
    sky1311WriteReg(ADDR_TX_CTRL, TX_CRC_EN|TX_PARITY_ODD|TX_POLE_HIGH);
}
/**
 ******************************************************************************
 ** \简  述  操作M1卡时关闭发送的CRC校验(sky1311s)
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void sky1311TxCrcOff(void)
{
    //sky1311WriteReg(ADDR_TX_CTRL, (!TX_CRC_EN)|TX_PARITY_ODD|TX_POLE_HIGH); //20210827
    sky1311WriteReg(ADDR_TX_CTRL, TX_PARITY_ODD|TX_POLE_HIGH);
}
/**
 ******************************************************************************
 ** \简  述  设置寄存器掩码位
 **
 ** \参  数  uint8_t regAddr 寄存器地址， uint8_t mask 要设置的"掩码"字节
 ** \返回值  none
 ******************************************************************************/
void SetBitMask(uint8_t regAddr, uint8_t mask)
{
    uint8_t tmp;
    tmp = sky1311ReadReg(regAddr);
    sky1311WriteReg(regAddr, tmp|mask);
}
/**
 ******************************************************************************
 ** \简  述  清除寄存器掩码位
 **
 ** \参  数  uint8_t regAddr 寄存器地址， uint8_t mask 要清除的"掩码"字节
 ** \返回值  none
 ******************************************************************************/
void ClearBitMask(uint8_t regAddr, uint8_t mask)
{
    uint8_t tmp;
    tmp = sky1311ReadReg(regAddr);
    sky1311WriteReg(regAddr, tmp & ~mask);
}

/**
 ******************************************************************************
 ** \简  述  输出调试参数初始化
 **
 ** \参  数  选择通过MFOUT脚输出的信号，
 **          bit2,1,0
 **             0 0 0 rx phase signal after demod, before sample
 **             0 0 1 tx bit stream
 **             0 1 0 rx bit stream
 **             0 1 1 rx data from analog
 **             1 0 0 DO from digital
 ** \返回值  none
 ******************************************************************************/
/*static void directModeInit(uint8_t mfout_sel)
{
    sky1311WriteReg(ADDR_MFOUT_SEL,  mfout_sel);
}*/

/**
 ******************************************************************************
 ** \简  述  打开PCD天线
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void pcdAntennaOn(void)
{
    SetBitMask(ADDR_ANA_CFG1, TX_EN);
}

/**
 ******************************************************************************
 ** \简  述  关闭PCD天线
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void pcdAntennaOff(void)
{
    ClearBitMask(ADDR_ANA_CFG1, TX_EN);  // Disable TX
}
/**
 ******************************************************************************
 ** \简  述  清除所有中断标记
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void irqClearAll(void)
{
    sky1311WriteReg(ADDR_IRQ_STA, 0xFF);
}
/**
 ******************************************************************************
 ** \简  述  清除指定的中断标记
 **
 ** \参  数  uint8_t irq
 ** \返回值  none
 ******************************************************************************/
void irqClear(uint8_t irq)
{
    sky1311WriteReg(ADDR_IRQ_STA,irq);
}
/**
 ******************************************************************************
 ** \简  述  设置sky1311与卡通信的倍速, ISO14443-4部分规定，多倍速的PICC
 **
 ** \参  数  设置的倍速，106，212或424
 ** \返回值  none
 ******************************************************************************/
void bitRateSet(uint16_t high_rate)
{
    sky1311WriteReg(ADDR_FSM_STATE, RATE_SEL);                           // enable high-speed mode
    if(high_rate == 212){
        sky1311WriteReg(ADDR_TX_CTRL, TX_212|TX_POLE_HIGH|TX_CRC_EN|TX_PARITY_ODD);
        sky1311WriteReg(ADDR_RATE_CTRL, RX_RATE_WID_2|RX_RATE_212|RX_RATE_PAR_ODD|RX_RATE_CRC_EN|RX_RATE_PAR_EN);
        sky1311WriteReg(ADDR_TX_PUL_WID,0x11);  //0x14                           // tx pulse width = 1.5u
        sky1311WriteReg(ADDR_RATE_FRAME_END,0x40);                       // stop counter = 0x40
        sky1311WriteReg(ADDR_RATE_THRES,0x20);                           // threshold = 0x20
    }else if(high_rate == 424){
        sky1311WriteReg(ADDR_TX_CTRL, TX_424|TX_POLE_HIGH|TX_CRC_EN|TX_PARITY_ODD);
        sky1311WriteReg(ADDR_RATE_CTRL, RX_RATE_WID_2|RX_RATE_424|RX_RATE_PAR_ODD|RX_RATE_CRC_EN|RX_RATE_PAR_EN);
        sky1311WriteReg(ADDR_TX_PUL_WID,0x07);//0x0A                           // tx pulse width = 0.8u
        sky1311WriteReg(ADDR_RATE_FRAME_END,0x20);                       // stop counter = 0x20
        sky1311WriteReg(ADDR_RATE_THRES,0x10);                           // threshold = 0x10
    }
    else if(high_rate == 848){
        sky1311WriteReg(ADDR_TX_CTRL, TX_848|TX_POLE_HIGH|TX_CRC_EN|TX_PARITY_ODD);
        sky1311WriteReg(ADDR_RATE_CTRL, RX_RATE_WID_2|RX_RATE_848|RX_RATE_PAR_ODD|RX_RATE_CRC_EN|RX_RATE_PAR_EN);
        sky1311WriteReg(ADDR_TX_PUL_WID,0x04);                           // tx pulse width = 0.4u
        sky1311WriteReg(ADDR_RATE_FRAME_END,0x10);                       // stop counter = 0x10
        sky1311WriteReg(ADDR_RATE_THRES,0x08);                           // threshold = 0x8
    }
}
/**
 ******************************************************************************
 ** \简  述  设置sky1311超时定时器
 ** \使用天线的基准etu作为时钟，128/13.56MHz 约9.44微秒，计数器24位，最多可计数0x3F FFFF
 ** \最大计时时间是 9。44微秒 * 0x3F FFFF = 39.6秒，也就是约39592毫秒
 ** \定时精度使用约1毫秒，计数值 = 1ms x 106, 定时时间从1毫秒到39568毫秒之间
 ** \参  数 ： 定时时间，单位：毫秒
 ** \返回值  none
 ******************************************************************************/
void setPCDTimeOut(uint16_t dlyMs)
{
    uint32_t dlyCount;
    uint8_t cnt;
    if(dlyMs>39568)
        dlyCount = 0x3FFFFF;
    else
     dlyCount = (uint32_t)dlyMs*106;
    cnt = (uint8_t)(dlyCount & 0xff);
    sky1311WriteReg(ADDR_TIME_OUT0, cnt);
    cnt = (uint8_t)((dlyCount>>8) & 0xff);
    sky1311WriteReg(ADDR_TIME_OUT1, cnt);
    cnt = (uint8_t)((dlyCount>>16) & 0x3f);
    sky1311WriteReg(ADDR_TIME_OUT2, 0x80 + cnt);       // time_out timer star condition = beginning of RX SOF

}

/**
 ******************************************************************************
 ** \简  述  选择type A 卡作为操作对象
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void typeAOperate(void)
{
    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_A);             // analogA select
    sky1311WriteReg(ADDR_FSM_STATE, TYPE_A_SEL);        // typeA select
    sky1311WriteReg(ADDR_CRC_CTRL, CRC_A);              // crcA enable
    sky1311WriteReg(ADDR_TX_PUL_WID,0x26);              // set to default value
    sky1311WriteReg(ADDR_M1_CTRL, 0x00);                // disable M1 operation
    sky1311WriteReg(ADDR_ANA_CFG3, 0x09);
    sky1311WriteReg(ADDR_ANA_CFG3, 0x81);
//		extern void RegReadTest(void);
//		RegReadTest();
}
/**
 ******************************************************************************
 ** \简  述  选择type B 卡作为操作对象
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void typeBOperate(void)
{
    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_B);            // analogB select
    sky1311WriteReg(ADDR_FSM_STATE, TYPE_B_SEL);       // typeB select
    sky1311WriteReg(ADDR_CRC_CTRL, CRC_B);             // crcB enable
    sky1311WriteReg(ADDR_TX_PUL_WID,0x26);              // set to default value
    sky1311WriteReg(ADDR_M1_CTRL, 0x00);                // disable M1 operation
    sky1311WriteReg(ADDR_ANA_CFG3, 0x09);
    sky1311WriteReg(ADDR_ANA_CFG3, 0x81);
}
/**
 ******************************************************************************
 ** \简  述  设置FIFO收发水线
 **  FIFO 64字节，收发数据的时候达到设定的水线就触发水线中断，方便及时收发数据
 ** \参  数  水线值， 1--63
 ** \返回值  none
 ******************************************************************************/
void setFifoWaterLevel(uint8_t waterlevel)
{
    if(waterlevel<1)  
        waterlevel = 1;
    else if(waterlevel>63) 
        waterlevel = 63;
    sky1311WriteReg(ADDR_FIFO_CTRL,waterlevel);
}
/**
 ******************************************************************************
 ** \简  述  模拟参数初始化，配置模拟参数寄存器
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
//static void analogInit(void)
//{
//    sky1311WriteReg(ADDR_ANA_CFG0, RD_PAVOL); //  7-4: RC OSC Freq
//                                              //  3-2: PA Driver
//                                              //  1: ixtal
//                                              //  0: External LDO
//    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_A);
//    sky1311WriteReg(ADDR_ANA_CFG1, 0x00);
//    delay(100);
//    sky1311WriteReg(ADDR_ANA_CFG1, 0xFC);   //sky1311WriteReg(ADDR_ANA_CFG1, 0xF4);
//                                              //  7: txen
//                                              //  6: rxen
//                                              //  5-4: clk_sel
//                                              //  00(osc off, xtal off)
//                                              //  01(osc on, xtal off)
//                                              //  10(osc off, xtal 13.56)
//                                              //  11(osc off, xtal 27.12)
//}

/**
 ******************************************************************************
 ** \简  述  初始化SKY1311T寄存器, 设置晶振驱动能力，选择时钟源，打开时钟，PA电压
 **          设置接收超时时间100ms，中断开启，默认选择A类协议，初始化各种接收条件
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
 
 void RegReadTest(void)
 {
	 uint8_t i;
//	 uint8_t reg[100]={0};
	 for(i=0;i<0x40;i++)
	 {
		 	 printf("read reg:0x%02x=0x%02x\r\n",i,sky1311ReadReg(i));
			//reg[i]=sky1311ReadReg(i);
	 }
 }
void sky1311Init(void)
{
	//	RegReadTest();  
    sky1311WriteCmd(CMD_SW_RST);                // reset status
   // analogInit();
    sky1311WriteReg(ADDR_ANA_CFG0, 0x20|RD_PAVOL|IXTAL); //  7-4: RC OSC Freq(这里不关心)
                                                //  3-2: PA Driver
                                                //  1: ixtal (default: 0)
                                                //  0: External LDO (default: 0)

    sky1311WriteReg(ADDR_ANA_CFG2, ANA2_A);
    sky1311WriteReg(ADDR_ANA_CFG1, 0x00);
    delay(1000);
  //   sky1311WriteReg(ADDR_ANA_CFG1, 0xFC);   //sky1311WriteReg(ADDR_ANA_CFG1, 0xF4);
		sky1311WriteReg(ADDR_ANA_CFG1, TX_EN|RX_EN|CLK_SEL3|0x0C);
    DelayMS(10);  //Crystal Stable time
    setPCDTimeOut(100);       // default timeout time 100 mS, stop condition = beginning of RX SOF
    sky1311WriteReg(ADDR_RX_PUL_DETA, 0x34);     // 高4位识别曼彻斯特码脉宽，越大容错能力越强
    sky1311WriteReg(ADDR_RX_PRE_PROC, 0x00);
    sky1311WriteReg(ADDR_RX_START_BIT_NUM,0);
    sky1311WriteReg(ADDR_MOD_SRC,0x02);
    sky1311WriteReg(ADDR_IRQ_EN, IRQ_M1_EN|IRQ_TOUT_EN|IRQ_TX_EN|IRQ_RX_EN|IRQ_HIGH_EN|IRQ_LOW_EN);    // enable  IRQ
    sky1311WriteReg(ADDR_FIFO_CTRL,FIFO_LEVEL); //sky1311WriteReg(ADDR_FIFO_CTRL,8);            // set water-level of FIFO
}
/**
 ******************************************************************************
 ** \简  述  SKY1311S复位，并处于disable状态
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void sky1311Reset(void)
{
    sky1311WriteReg(ADDR_ANA_CFG0, 0);
    sky1311WriteReg(ADDR_ANA_CFG1, 0x01);   // close TX, RX, OSC off
   // sky1311WriteCmd(CMD_SW_RST);            // reset state machine
		sky1311WriteCmd(CMD_IDLE); 
		SKY1311_ChipDis();
		DelayMS(10);
}

/**
 ******************************************************************************
 ** \简  述  复位PICC，关闭场5ms,再打开
 **
 ** \参  数  none
 ** \返回值  none
 ******************************************************************************/
void resetPicc(void)
{
    ClearBitMask(ADDR_ANA_CFG1,TX_EN);
    DelayMS(5);
    SetBitMask(ADDR_ANA_CFG1,TX_EN);
    DelayMS(10);

}
/**
 ******************************************************************************
 ** \简  述  数据发射函数，将数据写到FIFO中并通过射频接口发送给PICC
 **
 ** \参  数   
 **           uint8_t *txBuff,      // 数据内容
 **           uint16_t txSize       // 数据大小
 ** \返回值  none
 ******************************************************************************/
void sky1311_fifo_tx(uint8_t *txBuff, uint16_t txSize)
{
    uint16_t tx_res = txSize;
    volatile uint32_t delayCount;
    sky1311WriteCmd(CMD_IDLE);              // reset state machine to Idle mode
    sky1311WriteCmd(CMD_CLR_FF);            // clear FIFO
    irqClearAll();                          // clear all IRQ state
		
		uint32_t startWhile=GetTimerCount();
    /* when TX length<=FIFO's depth, write all data to FIFO */
    if( txSize <= 64){
        sky1311WriteFifo(txBuff, txSize);
        sky1311WriteCmd(CMD_TX_RX);             // transceive & into receive mode
    }
    /* when TX length > FIFO's depth */
    else{               // txLen > 64
        sky1311WriteFifo(txBuff, 64);       // send 64 bytes
        sky1311WriteCmd(CMD_TX_RX);
        tx_res = txSize - 64;
        while(tx_res>0){                    // send remain bytes
            if(1==SKY1311_IRQ_READ()){
                irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
                if(irq_sta & IRQ_LOW){  // FIFO low
                    if(tx_res>=(64-FIFO_LEVEL)){
                        sky1311WriteFifo(&txBuff[txSize - tx_res], 64-FIFO_LEVEL);
                        tx_res -=(64-FIFO_LEVEL);
                    }
                    else{
                        sky1311WriteFifo(&txBuff[txSize - tx_res], tx_res);
                        tx_res = 0;
                    }

                    irqClear(irq_sta);//irqClearAll();
                }
                else        // the other IRQ
                    irqClear(irq_sta);//return;
            }
						if(GetTimerElapse(startWhile)>500)return;
        }
    }
    /* wait TX finished */
		startWhile=GetTimerCount();
    while(1){
        delayCount = 0xFFFF;
        while( 0==SKY1311_IRQ_READ() && --delayCount);      // 检测IRQ引脚是否有中断
        irq_sta=sky1311ReadReg(ADDR_IRQ_STA);
        if((delayCount==0)||(irq_sta & IRQ_TOUT)){             // tiemout
            sky1311WriteCmd(CMD_IDLE);
            return;
        }
        if(irq_sta & IRQ_TX){
            irqClear(IRQ_TX);
            return;
        }
        else{
            irqClear(irq_sta);  //irqClearAll();
        }
				if(GetTimerElapse(startWhile)>500)return;
    }

}
/**
 ******************************************************************************
 ** \简  述  数据接收函数，将FIFO中数据读出来
 **
 ** \参  数   uint8_t *rxBuff,      // 数据内容
 **           uint16_t rxSize       // 数据大小
 ** \返回值  Ok,正确接收到数据 ; 其它，产生错误
 ******************************************************************************/
sta_result_t sky1311_fifo_rx( uint8_t *rxBuff, uint16_t *rxSize)
{
    uint16_t rx_buf_cnt=0;
    uint8_t  temp_len;
    volatile static uint32_t delayCount;
    /* wait RX data */
    delayCount=0xFFFF;  //如果单片机主频较快，这个值要加大，可以改成32位的数
    while( (0==SKY1311_IRQ_READ()) && (--delayCount));      // 检测IRQ引脚是否有中断
    irq_sta = sky1311ReadReg(ADDR_IRQ_STA);

    if((delayCount==0)||(irq_sta & IRQ_TOUT)){             // tiemout
        sky1311WriteCmd(CMD_IDLE);
        irqClearAll();
        return NoResponse;
    }
    err_sta = sky1311ReadReg(ADDR_ERR_STA);
		if(err_sta & 0xC0)                         // error exits
    {
        sky1311WriteCmd(CMD_CLR_FF);           // noise occur, restart the rx
        sky1311WriteCmd(CMD_RX);
        irqClearAll();
        return Error;
    }
		uint32_t startWhile=GetTimerCount();
		
    while(1)
    {
        irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
        if(irq_sta & IRQ_HIGH){        // FIFO High
            sky1311ReadFifo(&rxBuff[rx_buf_cnt], 64-FIFO_LEVEL);   // load next 64-water-level bytes into FIFO
            rx_buf_cnt += (64-FIFO_LEVEL);                         // 
            irqClear(IRQ_HIGH);
        }
        else if(irq_sta & IRQ_RX){          // Received
            temp_len = sky1311ReadReg(ADDR_FIFO_LEN);               // get FIFO length
            sky1311ReadFifo(&rxBuff[rx_buf_cnt], temp_len);        // get data ,FIFO-->rx_buf
            rx_buf_cnt += temp_len;
            *rxSize  = rx_buf_cnt;
#if SKY_DEBUG
            //temp_len= sky1311ReadReg(ADDR_RX_NUM_L);
            //printf("\nFIFO Receive Size=%d, Rx Number = %d\n",rx_buf_cnt,temp_len);
#endif
            irqClear(IRQ_RX);
            return Ok;
        }
        else if(irq_sta & IRQ_LOW){    //清除IRQ_LOW中断
            irqClear(IRQ_LOW);
        }
        else if(irq_sta & IRQ_TOUT)
        {
            sky1311WriteCmd(CMD_IDLE);
            irqClearAll();
					
            return ErrorTimeout;
        }  
				if(GetTimerElapse(startWhile)>500)return ErrorTimeout;
    }
}
/**
 ******************************************************************************
 ** \简  述  发送数据后立即转入接收数据
 **          要发送到数据保存在变量pSendDataBuf中（发送缓冲区）
 **          接收的数据保存在变量pRcvDataBuf，接收到的数量保存在变量pRcvLen中
 ** \参  数
 **          uint8_t *pSendDataBuf, 发送缓冲区
 **          uint8_t ucSendLen 发送数据的长度，
 **          uint8_t *pRcvDataBuf, 接收数据缓冲区
 **          uint16_t *pRcvLen ， 接收数据长度
 **
 ** \返回值  Ok: 接收到返回的数据， 其它：没有收到返回的数据
 ******************************************************************************/
sta_result_t ExchangeData(uint8_t *pSendDataBuf, uint16_t ucSendLen,
                          uint8_t *pRcvDataBuf,  uint16_t *pRcvLen)
{
    sta_result_t sta;
    sky1311_fifo_tx(pSendDataBuf, ucSendLen);
    sta = sky1311_fifo_rx(pRcvDataBuf, pRcvLen);
    return sta;
}

/**
 ******************************************************************************
 ** \简  述  Type A 防冲突数据发送函数，将数据写到FIFO中并通过射频接口发送给PICC
 **          发送的数据保存在全局变量tx_buf中，发送缓冲区
 **
 ** \参  数  uint8_t* txBuf           数据缓冲区
 **          uint8_t txLen          发射的数据长度（包括不完整的部分）,
 **          uint8_t lastBitNum     最后一个字节有效位数
 ** \返回值  Ok--成功发射，Timeout--超时, Error -- 其它错误
 ******************************************************************************/
sta_result_t bitCollisionTrans(uint8_t* txBuf, uint8_t txLen, uint8_t lastBitNum)
{
    volatile uint16_t delayCount;
    if(txLen>7)          // The maximum length of for transmission from PCD to PICC shall be 55 data bits
        return Error;
    sky1311WriteCmd(CMD_IDLE);              // reset state machine to Idle mode
    sky1311WriteCmd(CMD_CLR_FF);            // clear FIFO
    irqClearAll();                          // clear all IRQ state
    /* write numbers */
    sky1311WriteReg(ADDR_TX_BYTE_NUM, txLen);
    sky1311WriteReg(ADDR_TX_BIT_NUM,lastBitNum);
    sky1311WriteFifo(txBuf, txLen);         // write data to FIFO
    sky1311WriteCmd(CMD_TX_RX);             // transceive & into receive mode
    /* wait TX finished */
			//	Dprint("wait TX finished:\r\n");
		 DelayMS(3); 
		uint32_t startWhile=GetTimerCount();
    while(1){
        delayCount = 0x4FFF;                // timeout count
        while( 0==SKY1311_IRQ_READ() && --delayCount);      // 检测IRQ引脚是否有中断
        //while((0==(irq_sta=sky1311ReadReg(ADDR_IRQ_STA))) && --delayCount);   // waiting for TX STOP IRQ
        irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
        if((delayCount==0)||(irq_sta & IRQ_TOUT)){
            sky1311WriteCmd(CMD_IDLE);
            irqClearAll();
            return Timeout;
        }
        else if(irq_sta & IRQ_TX){
            irqClear(IRQ_TX);
            break;
        }
        else{
            irqClear(irq_sta);  //irqClearAll(); //20200825 
        }
				if(GetTimerElapse(startWhile)>500)return Timeout;
    }
    /* 等待接收数据完成 */
    delayCount = 0x4FFF;                            // timeout count
    while( 0==SKY1311_IRQ_READ() && --delayCount);      // 检测IRQ引脚是否有中断
    //while((0==(irq_sta=sky1311ReadReg(ADDR_IRQ_STA))) && --delayCount);     // waiting for RX STOP IRQ
    irq_sta = sky1311ReadReg(ADDR_IRQ_STA);
    if((delayCount==0)||(irq_sta & IRQ_TOUT)){
        sky1311WriteCmd(CMD_IDLE);
        irqClearAll();
        return Timeout;
    }else if(!(irq_sta & IRQ_RX)){
        irqClearAll();
        return Error;
    }
    irqClear(irq_sta);
    return Ok;
}


/**
 ******************************************************************************
 ** \简  述  扫描最优的RC频率，为低功耗询卡用，
 **          注意：需要在读卡器场内没有任何导电物质遮挡时扫描
 **
 ** \参  数  none
 ** \返回值  低8位采集到的最大AD值，
         bit8--bit11 表示幅值最大时的频率值，对应Analog0寄存器的bit7--bit4
   		 bit12 表示是否扫描到最大值，1--扫描到，0--没有。
   		 bit13--15 没有使用，设置为0
 ******************************************************************************/
#if 1       // 寻找最大AD值
uint16_t sky1311RCFreqCali(void)
{
    
    uint8_t currAdcVal=0;       // 当前AD值
    uint8_t maxAdcVal=0;        // 最大AD值
    uint8_t currRCFreqVal=0;     // 当前频率值
    uint8_t maxRCFreq = 0;    // 最大的频率值
    SKY1311_ENABLE();
    DelayMS(5);
  //  sky1311WriteReg(ADDR_ANA_CFG6, 0x00); //stop Auto CheckCard
    sky1311WriteReg(ADDR_ANA_CFG1, 0x1C); //OSC Enable
    sky1311WriteReg(ADDR_ANA_CFG2, 0xA0);
		for(currRCFreqVal=0;currRCFreqVal<16;currRCFreqVal++)   // from 0000 to 1111 scan
    {
        sky1311WriteReg(ADDR_ANA_CFG0, (currRCFreqVal << 4) | CHK_PAVOL); // 调节频点
        /*  用程序产生波形并做AD采集和转换 */
        sky1311WriteReg(ADDR_ANA_CFG4, 0x00);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x40);   // pull up RSSI_EN
        delay(200);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x60);   // pull up TX_EN
        delay(60);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x70);   // pull up ADC_EN
        delay(60);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x50);   // pull low TX_EN
        delay(150);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x00);   // pull low ADC_EN, RSSI_EN
        delay(30);
        currAdcVal = sky1311ReadReg(ADDR_ANA_CFG5); // read ADC value from analog5 register
        
        //AdcValue[currRCFreqVal] = currAdcVal;  //for debug adc value
        if(currAdcVal > maxAdcVal){			    // 当前频率的AD值若大于最大值
            maxAdcVal = currAdcVal; 			// 用当前值取代最大值
            maxRCFreq = currRCFreqVal;   	// 记下当前频率值
        }
	}
    SKY1311_DISABLE();
    if(maxAdcVal>MINADVAL){
        maxRCADVal = maxAdcVal;
        #if SKY_DEBUG==1
        printf("\n--RC frequency calibate:\n");
        printf("RC Param = %2X, AD Value=%2X\n",maxRCFreq,maxRCADVal);
        #endif
        return ((0x10 | (maxRCFreq-0)) << 8 | maxAdcVal);		// 返回成功标记和频率值
    }
    else{
        maxRCADVal = MAXADVAL;
        #if SKY_DEBUG==1
        printf("\n--RC frequency calibate: Error!\n");
        #endif
        return 0;
    }
}
#else
uint16_t sky1311RCFreqCali(void)
{
    uint8_t currRCFreqVal=0;     // 当前频率值
    uint8_t maxAdcVal=0;         // 最大AD值
    uint8_t maxRCFreqVal = 0;    // 最大的频率值
    uint8_t cryADValue;
    uint8_t rcADValue[16],decADValue[16];
    uint8_t minDecVal=0xff;

    uint8_t caliRCFreqVal;
    uint16_t RetRCFValue;             //返回RCF值

    SKY1311_ENABLE();
    DelayMS(5);
    /* 使用外部晶体获得13.56MHz下的AD值 */
    sky1311WriteReg(ADDR_ANA_CFG0, CHK_PAVOL|0x02);
    sky1311WriteReg(ADDR_ANA_CFG1, 0x3C); //OSC Enable 27.12MHz
    sky1311WriteReg(ADDR_ANA_CFG2, 0xA0);
    DelayMS(5);                             // Wait OSC established

    sky1311WriteReg(ADDR_ANA_CFG4, 0x00);
    delay(30);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x40);   // pull up RSSI_EN
    delay(30);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x60);   // pull up TX_EN
    delay(10);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x70);   // pull up ADC_EN
    delay(10);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x50);   // pull low TX_EN
    delay(20);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x00);   // pull low ADC_EN, RSSI_EN
    delay(10);
    cryADValue = sky1311ReadReg(ADDR_ANA_CFG5); // read ADC value from analog5 register

    DelayMS(10);
    /* 找到16个RC对应的AD值, 并取和外部晶体下的AD值的差值 */
    sky1311WriteReg(ADDR_ANA_CFG1, 0x1C); //OSC Enable
    sky1311WriteReg(ADDR_ANA_CFG2, 0xA0);
    for(currRCFreqVal=0;currRCFreqVal<16;currRCFreqVal++)   // from 0000 to 1111 scan
    {
        sky1311WriteReg(ADDR_ANA_CFG0, (currRCFreqVal << 4) | CHK_PAVOL);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x00);
        delay(30);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x40);   // pull up RSSI_EN
        delay(30);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x60);   // pull up TX_EN
        delay(10);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x70);   // pull up ADC_EN
        delay(10);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x50);   // pull low TX_EN
        delay(20);
        sky1311WriteReg(ADDR_ANA_CFG4, 0x00);   // pull low ADC_EN, RSSI_EN
        delay(50);
        rcADValue[currRCFreqVal] = sky1311ReadReg(ADDR_ANA_CFG5); // read ADC value from analog5 register

        if(rcADValue[currRCFreqVal]>maxAdcVal)
        {
            maxAdcVal = rcADValue[currRCFreqVal];
            maxRCFreqVal = currRCFreqVal;
        }
        // 求当前AD值和外部晶体时的AD值，差值的绝对值
        if(rcADValue[currRCFreqVal]>cryADValue)
            decADValue[currRCFreqVal] = rcADValue[currRCFreqVal] - cryADValue;
        else
            decADValue[currRCFreqVal] = cryADValue - rcADValue[currRCFreqVal];
	}
    //=========== 找到最最接近13.56MHz的 RC值,既差值最小的对应的RC值 ===========
    //for(currRCFreqVal=2;/*(currRCFreqVal<=maxRCFreqVal)&&*/(currRCFreqVal<13);currRCFreqVal++)
    for(currRCFreqVal=0;currRCFreqVal<=maxRCFreqVal;currRCFreqVal++)
    {
       if(decADValue[currRCFreqVal]<minDecVal)
       {
         minDecVal = decADValue[currRCFreqVal];
         caliRCFreqVal = currRCFreqVal;
       }
    }
    RetRCFValue = (uint16_t)(((0x10 | (caliRCFreqVal)&0xf)<<8) | rcADValue[caliRCFreqVal]);
    return  RetRCFValue ;
}

#endif
/**
 ******************************************************************************
 ** \简  述  检查是否有卡进入或者离开场(低功耗询卡)
 **
 ** \参  数  uint8_t RC频率设定值
 ** \返回值  没有动作，0  (设置低功耗询卡标记)
 **          有卡进入，1  (设置读卡标记)
 **          有卡离开，2  (设置重新校准查询卡频率标记)
 ******************************************************************************/
void checkCardInit(uint16_t rc_val)
{
    uint8_t maxRCFreq, maxADVal;

    maxRCFreq = (uint8_t)((rc_val >> 4) & 0xF0); // RC频率校准值
    maxADVal = (uint8_t)((rc_val & 0xFF));         // 对应的最大AD值

    sky1311WriteReg(ADDR_ANA_CFG1, 0x00);           // disable OSC
    sky1311WriteReg(ADDR_IRQ_EN,0);                 // disable all interrupt
    SKY1311_ENABLE();
    DelayMS(2);
    sky1311WriteReg(ADDR_ANA_CFG0, maxRCFreq | CHK_PAVOL);
    sky1311WriteReg(ADDR_ANA_CFG2, 0x10);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x00);
    sky1311WriteReg(ADDR_ANA_CFG7, ADC_SAMPLE_5US | TX_SETTLE_0US | RSSI_DELTA);
    sky1311WriteReg(ADDR_ANA_CFG8, maxADVal-RSSI_ABS);
    sky1311WriteReg(ADDR_ANA_CFG6, WKU_EN | WKU_OR | WKU_300MS | RSSI_4AVG);
    irqClearAll();
    sky1311WriteReg(ADDR_ANA_CFG1, 0x1C);
	//	sky1311WriteReg(ADDR_IRQ_EN,0); 
    SKY1311_DISABLE();
//    while(1)
//		{
//				if(SKY1311_IRQ_READ())
//				{
//						maxRCFreq=sky1311ReadReg(ADDR_ANA_CFG5);
//						maxRCFreq=sky1311ReadReg(ADDR_IRQ_STA);
//						irqClearAll();
//				}
//		}
}

void checkCardDeinit(void)
{
    SKY1311_ENABLE();
    DelayMS(5);
    sky1311WriteReg(ADDR_ANA_CFG6, 0x22);
    sky1311WriteReg(ADDR_ANA_CFG7, 0x3F);
    sky1311WriteReg(ADDR_ANA_CFG8, 0xFF);
    SKY1311_DISABLE();
}

/**
 ******************************************************************************
 ** \简  述  检查是否有卡离开场
 **
 ** \参  数  uint8_t RC频率扫描得到的最大AD值
 ** \返回值  true,卡已经离开； false， 卡没有离开
 ******************************************************************************/
UINT8 checkCardRemove(uint16_t rc_val)
{
    uint8_t maxRCFreqVal;
    uint8_t maxAdcVal;
    uint8_t currAdcVal;       // 当前AD值
    maxRCFreqVal = (uint8_t)((rc_val >> 4) & 0xF0);
    maxAdcVal = (uint8_t)(rc_val & 0xff);
    SKY1311_ENABLE();
    DelayMS(5);
    sky1311WriteReg(ADDR_ANA_CFG1, 0x1C); //OSC Enable
    sky1311WriteReg(ADDR_ANA_CFG2, 0xA0);
    sky1311WriteReg(ADDR_ANA_CFG0, maxRCFreqVal | CHK_PAVOL);

    sky1311WriteReg(ADDR_ANA_CFG4, 0x00);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x40);   // pull up RSSI_EN
    delay(30);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x60);   // pull up TX_EN
    delay(10);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x70);   // pull up ADC_EN
    delay(10);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x50);   // pull low TX_EN
    delay(20);
    sky1311WriteReg(ADDR_ANA_CFG4, 0x00);   // pull low ADC_EN, RSSI_EN
    delay(10);
    currAdcVal = sky1311ReadReg(ADDR_ANA_CFG5);
    SKY1311_DISABLE();
    #if SKY_DEBUG==1
    printf("currAdcVal=%d\r",currAdcVal);
    #endif
    if(currAdcVal > (maxAdcVal - RSSI_DELTA))
    {
        return TRUE;
    }
    else
        return FALSE;
}
