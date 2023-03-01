#ifndef __CARDOPER_H
#define __CARDOPER_H

#include "sky1311t.h"
#include "sky1311_drv.h"

#define MIFARE1CARD		0x01		// MIFARE 1
#define TYPEACPUCARD	0x02		// Type A CPU
#define NTAGCARD        0x03        // NTAG CARD
#define IDCARD			0x55		// …Ì∑›÷§

typedef struct cardInfo
{
   uint8_t UIDSize;
   uint8_t UID[12];
   uint8_t ATQA[2];
   uint8_t SAK;
}CardBaseInfo_t;

sta_result_t TypeA_test(void);
sta_result_t TypeACardRead(CardBaseInfo_t* cardInfo, uint8_t* cardType);

#endif








