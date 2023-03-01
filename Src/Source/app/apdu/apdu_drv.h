/*
 * apdu_drv.h
 *
 *  Created on: 2018Äê8ÔÂ13ÈÕ
 *      Author: YangWenfeng
 */

#ifndef APDU_DRV_H_
#define APDU_DRV_H_

#include "usi_drv.h"

#define	ATR_LENGTH		0xe

#define	INS_PPS			  0x10

#define	EN_WTO		  (gUSIReg->USICR1) |= USISR_WTO_MASK
#define	DIS_WTO		  (gUSIReg->USICR1) &= ~USISR_WTO_MASK

#define USI_SB_IT_EN  (gUSIReg->USIIER) |= USISR_SB_MASK
#define USI_SB_IT_DIS (gUSIReg->USIIER) &= ~USISR_SB_MASK

#define IOCTRL_USICR	(*(volatile unsigned int *)(0x40000004))


// APDU status words
#define SW_SUCCESS							(UINT16)0x9000
#define SW_NULL								(UINT8)0x60
//#define SW_MEMORY_VERIFY_ERROR				(UINT16)0x6581
#define SW_FLASH_ERASE_ERROR				(UINT16)0x6581
#define SW_FLASH_PROGRAM_ERROR				(UINT16)0x6582
#define SW_WRONG_LENGTH						(UINT16)0x6700
#define SW_WRONG_DATA						(UINT16)0x6A80
#define SW_WRONG_P1P2						(UINT16)0x6A86
#define SW_INS_NOT_SUPPORTED				(UINT16)0x6D00
#define SW_UNKNOWN							(UINT16)0x6F00

#define SW_SECURITY_STATUS_NOT_SATISFIED	(UINT16)0x6982
#define SW_CONDITIONS_NOT_SATISFIED			(UINT16)0x6985
#define SW_FUNC_NOT_SUPPORTED				(UINT16)0x6A81
#define SW_CLA_NOT_SUPPORTED				(UINT16)0x6E00
#define SW_WARNING_61						(UINT16)0x6100
#define SW_WARNING_63						(UINT16)0x6300
#define SW_WARNING_67						(UINT16)0x6700
#define SW_WARNING_6C						(UINT16)0x6C00

#define SW61_GET_RESPONSE					(UINT16)0x6100

#define	APDU_CMD_LEN		5

#define	APDU_STATE_IDLE		0
#define	APDU_STATE_PPS		1
#define	APDU_STATE_CMD		2
#define	APDU_STATE_DATA		3

#define	APDU_STATE_PPS_OK	4
#define	APDU_STATE_CMD_OK	5
#define	APDU_STATE_DATA_OK	6

#define	APDU_STATE_ATR		7
//-------------------------------------------------------------

extern UINT8 adpu_status;
extern UINT8 apdu_cmdbuf[];
extern UINT8 apdu_rspbuf[];
extern volatile UINT8 apdu_cnt;
extern UINT16 apdu_datalen;
extern UINT16 resultCode;
extern volatile UINT8 g_selfcheck;

extern USI_TypeDef* gUSIReg;
extern volatile UINT32 init_test_result;

extern void usiDev_init(void);
extern void InitAPDU(UINT32 usi_baseAddr);
UINT8 Transmitdata(UINT8 *pdata, UINT32 len);
void SendATR(void);
void PPSManager(void);
UINT8 PPS_Structure(UINT8 pps0);
void ATR_Handler(void);
void SendPB (UINT8 val);
void SendStatus(void);

#endif /* APDU_H_ */
