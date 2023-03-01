// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : c0_reg.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#ifndef __C0_REG_H__
#define __C0_REG_H__

#include "type.h"

typedef struct
{
	__IO unsigned int  CPM_SLPCFGR;//0x00
	__IO unsigned int  CPM_SLPCR;//0x04
	__IO unsigned int  CPM_SCDIVR;//0x08
	__IO unsigned int  CPM_PCDIVR;//0x0c

	__IO unsigned int  CPM_OCSR;//0x10
	__IO unsigned int  CPM_CSWCFGR;//0x14
	__IO unsigned int  CPM_CSWUPDR;//0x18
       unsigned int  RESERVED1;//0x1c

	__IO unsigned int  CPM_CGTCR;//0x20
	__IO unsigned int  CPM_SYSCGTCR;//0x24
	     unsigned int  RESERVED2[2];//0x28
	
	__IO unsigned int  CPM_PWRCR;//0x30
	__IO unsigned int  CPM_SLPCNTR;//0x34
	__IO unsigned int  CPM_WKPCNTR;//0x38
	__IO unsigned int  CPM_SRSTMASKR;//0x3c

	__IO unsigned int  CPM_VCCQGTRIMR;//0x40
	__IO unsigned int  CPM_VCCQLTRIMR;//0x44
	__IO unsigned int  CPM_VCCQVTRIMR;//0x48
	__IO unsigned int  CPM_VCCQCTMR;//0x4c

	     unsigned int  RESERVED3[4];//0x50
			 
	__IO unsigned int  CPM_O8MTRIMR;//0x60
	     unsigned int  RESERVED4[2];//0x64
	__IO unsigned int  CPM_WKPFILTCR;//0x6c

	__IO unsigned int  CPM_OSCLSTIMER;//0x70
	     unsigned int  RESERVED5[2];//0x74
	__IO unsigned int  CPM_PWRSR;//0x7c
	     unsigned int  RESERVED6[11];//0x80
	__IO unsigned int  CPM_PWRINDICR;//0xac

}C0_CPM_TypeDef;

#endif /* __C0_REG_H__ */
