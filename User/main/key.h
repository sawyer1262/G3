#ifndef  __KEY_H_
#define  __KEY_H_


#include <stdint.h>


#define      KEY_INPUT_TIMEOUT          10000
#define      SET_FACTORY_RESET_TIME     250                 //250*20=5000
#define      TAMP_ALARM_TIME            10000

#define      KEY_TAMPER_PIN        EPORT_PIN5      //防拆
#define      KEY_BACK_PIN          EPORT_PIN4      //后板按键 
#define      PIR_POWER_PIN         EPORT_PIN31


#define      KEY_IDX_SET           0
#define      KEX_IDX_TAMP          1
#define      KEX_IDX_PIR           2




typedef enum{
		KEY_0=0x30,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_x=0x2a,       //*
		KEY_c=0x23,       //#
		KEY_bell='B',      //门铃
}KeyTouch_t;


typedef struct{
		uint8_t  Flag;                //按下或弹起
		uint8_t  Numb;                //按键号
		uint32_t STime;               //开始时间
}KeyStat_t;


extern KeyStat_t KeyStat;
extern uint32_t TampStartTime;
extern uint8_t LockAlarm;
extern uint8_t isPirOn;
extern uint32_t lingerstart;

void Key_Init(void);
void Key_Scan(void);
void Key_DeInit(void);
void TampSetButtonInit(void);
void TampSetButton_Scan(void);
void TampSetButtonDeInit(void);


void PowerOnPir(void);
void PowerOffPir(void);
void PirLingerCheck(void);

#endif

