/*
 * usi_reg.h
 *
 *  Created on: 2017Äê5ÔÂ25ÈÕ
 *      Author: YangWenfeng
 */

#ifndef USI_REG_H_
#define USI_REG_H_
#include "type.h"
#include "memmap.h"

enum
{
USISR_TDRE_MASK    =   0x80,
USISR_TC_MASK      =   0x40,
USISR_RDRF_MASK    =   0x20,
USISR_OR_MASK      =   0x10,
USISR_ERROR_MASK   =   0x08,
USISR_ATR_MASK     =   0x04,
USISR_WTO_MASK     =   0x02,
USISR_SB_MASK      =   0x01,
};
#pragma anon_unions
typedef struct
{
	__IO UINT8 USIBDR;  //0x00
	__IO UINT8 USICR1;  //0x01
	__IO UINT8 USICR2;  //0x02
	__IO UINT8 USISR;   //0x03
	__IO UINT8 USIIER;  //0x04
	union
	{
		__IO UINT8 USIRDR;  //0x05
		__IO UINT8 USITDR;  //0x05
	};
	__IO UINT8 USIWTRH;  //0x06
	__IO UINT8 USIWTRM;  //0x07
	__IO UINT8 USIWTRL;  //0x08
	__IO UINT8 USIGTRH;  //0x09
	__IO UINT8 USIGTRL;  //0x0a
	__IO UINT8 USICSR;   //0x0b
	__IO UINT8 USIPCR;   //0x0c
	__IO UINT8 USIPDR;   //0x0d
	__IO UINT8 USIDDR;   //0x0e
	__IO UINT8 USIFIFOINTCON;  //0x0f
	__IO UINT8 USICRCH;   //0x10
	__IO UINT8 USICRCL;   //0x11
}USI_TypeDef;





#endif /* USI_REG_H_ */
