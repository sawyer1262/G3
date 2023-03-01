#ifndef  __CARD_H_
#define  __CARD_H_

#include "Sky_typedef.h"  

#include <stdint.h>


#define       CARD_MAX_NUM               100
#define       CARD_ID_MAX                10 
#define       CARD_SEARCH_INTVAL         500               //正常寻卡间隔，单位MS
#define       CARD_ADD_INTVAL            300               //添加卡片间隔
#define       CARD_POWER_DELAY           100

#define       NOCARD                     0xFF              //无卡
#define       HASCARD                    0x55              //卡标识


#define       ADD_CARD_SUCESS            0            
#define       ADD_CARD_FULL              1
#define       ADD_CARD_EXIST             2
#define       ADD_CARD_FAIL              3

#define       DEL_CARD_SUCCESS           0
#define       DEL_CARD_NOEXIST           1

#define       IDF_CARD_SUCESS            0
#define       IDF_CARD_FAIL              1



#define       FPCARD_VCC_PIN      I2C_SDA



typedef struct
{
   uint8_t UIDSize;
   uint8_t UID[12];
   uint8_t ATQA[2];            //对A卡的应答
   uint8_t SAK;
}CardBaseInfo_t;

typedef enum{
		CARD_IDLE=0,                    //空闲
		CARD_SCAN,                      //唤醒卡片
}CardScanStat_t;

typedef struct{
		uint32_t StTime;
}CardStat_t;


typedef union{
		struct{
				uint8_t InitFlag;
				uint8_t CardSum;
				uint8_t Rsv1;
				uint8_t Rsv2;
		}Card;
		uint8_t CardBuf[4];
}CardInfo_t;

typedef union{
		struct{
				uint8_t Uid[4];
				uint8_t Roll[4];
		}Card;
		uint8_t CardBuf[8];
}CardUser_t;



extern CardStat_t CdStat;
extern CardUser_t CardUser[CARD_MAX_NUM];
extern CardInfo_t CardInfo;


void CardIoInit(void);
void CardDeInit(void);
void CardUserInit(void);

uint8_t GetCardFreeSlot(void);
uint8_t AddCard(CardUser_t* card,uint8_t type);
uint8_t IdfCard(CardUser_t* card);
uint8_t DelCard(uint8_t id);
void CardSearchHandle(void);
uint8_t CardAddHandle(uint8_t type);
uint8_t CheckCardInvalid(CardUser_t *usr);



#endif


