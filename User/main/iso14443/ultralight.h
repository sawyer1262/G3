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
 ** 文 件 名： ultralight.h
 **
 ** 文件简述： 符合Mifare Ultralight和 Ultralight C卡操作的所有函数，常量和变量
 **
 ** 版本历史:
 ** 2019-05-30 V1.00  EH   第一个正式版本
 **
 ******************************************************************************/
#define ULC_ACK			0x0A	//Ah positive acknowledge (ACK)
#define ULC_NAK			0x02	//2h NAK for EEPROM write error
#define ULC_NAKCRC	0x01	//1h NAK for parity or CRC error
#define ULC_NAKOTH	0x00	//0h NAK for any other error

#define ULC_READ		0x30
#define ULC_WRITE		0xA2
#define ULC_COMWRITE	0xA0
#define ULC_AUTH		0x1A


sta_result_t ULC_Authentication(uint8_t *Key);
sta_result_t ULC_Read(uint8_t address, uint8_t *receBuff);
sta_result_t ULC_Write(uint8_t address, uint8_t* data);
sta_result_t ULC_CompWrite(uint8_t pageAddress, uint8_t* data);
