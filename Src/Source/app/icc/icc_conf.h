/*
 * icc_conf.h
 *
 *  Created on: 2017年5月26日
 *      Author: YangWenfeng
 */

#ifndef ICC_CONF_H_
#define ICC_CONF_H_
#include "type.h"
#include "cpm_drv.h"
#include "eport_drv.h"

#define COLD_RESET		0
#define WARM_RESET	    1

/* 7816 protocol constants ---------------------------------------------------*/
#define T0_PROTOCOL        0x00  /* T0 protocol */
#define T1_PROTOCOL        0x01  /* T1 protocol */
#define DIRECT             0x3B  /* Direct bit convention */
#define INDIRECT           0x3F  /* Indirect bit convention */
#define SETUP_LENGTH       20
#define HIST_LENGTH        20
#define LCmax              258//300根据实际长度，防止数组越界, 111029

/* 7816 timeout Constants ----------------------------------------------------*/
/* Timeout constants are decided by CPU's performence , the default CPU's freq is 36MHZ*/
/*RESET Timeout is about 55 ms*/
#define SC_Receive_Timeout      0x355548

#define STATUS_SUCCESS          0x00
#define STATUS_TIMEOUT_ERROR 	0xF0

/*-------------------------- Answer to reset Commands ------------------------*/
#define SC_GET_A2R         0x12

#define OPEN_ICCARD_1V8		Card2_Power_Switch(card_1v8) 		//打开1V8
#define CLOSE_ICCARD_1V8	Card2_Power_Off()		            //关闭1V8
#define OPEN_ICCARD_3V		Card2_Power_Switch(card_3v) 		//打开3V
#define CLOSE_ICCARD_3V		Card2_Power_Off()		            //关闭3V
#define OPEN_ICCARD_5V		Card2_Power_Switch(card_5v)  		//打开5V
#define CLOSE_ICCARD_5V		Card2_Power_Off()		            //关闭5V
#define ICCARD_PRESENT    1//!EPORT_Read_Gpio_Data(EPORT_PIN7)      //插拔卡检测端口
/* Exported types ------------------------------------------------------------*/

typedef enum
{
	ERROR = 0,
	SUCCESS  = !ERROR
} ErrorStatus;


typedef enum
{
	SC_POWER_ON = 0x00,
	SC_RESET_LOW = 0x01,
	SC_RESET_HIGH = 0x02,
	SC_ACTIVE = 0x03,
	SC_ACTIVE_ON_T0 = 0x04,
	SC_ACTIVE_ON_T1 = 0x05,
	SC_POWER_OFF = 0x06
} SC_State;


/* ATR structure - Answer To Reset -------------------------------------------*/
typedef struct
{
	u8 TS;               /* Bit Convention */
	u8 T0;               /* High nibble = Number of setup byte; low nibble = Number of historical byte */
	u8 T[SETUP_LENGTH];  /* Setup array */
	u8 H[HIST_LENGTH];   /* Historical array */
	u8 Tlength;          /* Setup array dimension */
	u8 Hlength;          /* Historical array dimension */
	u8 SW1;              /* Status byte*/
	u8 SW2;              /* Status byte*/
} SC_ATR;

/* ADPU-Header command structure ---------------------------------------------*/
typedef struct
{
	u8 CLA;  /* Command class */
	u8 INS;  /* Operation code */
	u8 P1;   /* Selection Mode */
	u8 P2;   /* Selection Option */
} SC_Header;

/* ADPU-Body command structure -----------------------------------------------*/
typedef struct
{
	u8 LC;           /* Data field length */
	u8 Data[LCmax];  /* Command parameters */
	u8 LE;           /* Expected length of data to be returned */
} SC_Body;

/* ADPU Command structure ----------------------------------------------------*/
typedef struct
{
	SC_Header Header;
	SC_Body Body;
} SC_ADPU_Commands;

/* SC response structure -----------------------------------------------------*/
typedef struct
{
	u8 Data[LCmax];  /* Data returned from the card */
	u8 SW1;          /* Command Processing status */
	u8 SW2;          /* Command Processing qualification */
} SC_ADPU_Responce;

/*SC smartcard reader protocol structure--------------------------------------*/
typedef struct
{
	u8 SC_Protocol;
	u8 SC_Convention;
	u8 SC_T0CWI;
	u8 SC_T1CWI;
	u8 SC_BWI;
	u8 SC_BGT;
	u8 SC_WTX;
	u8 SC_TC1N;
	u8 SC_IFS;
	u8 SC_T1NAD;
	u8 SC_T1PCB;
	u8 SC_T1Start;
	u8 SC_T1Block;
	u8 SC_Retry;
	u32 SC_iBWT;
	u32 SC_iCWT;
	SC_State SC_CurState;
	u32 SC_workbaudrate;
} SC_READER_SET;

#endif /* ICC_CONF_H_ */
