// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : xip_drv.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __XIP_DRV_H__
#define __XIP_DRV_H__

#include "ssi_drv.h"

/*
 * Note: SSI Xip, memory map(QUAD spi eflash demo for Winbond w25q128)
 */
 
//#define XIP_REMAP_DEBUG  1 
#define XIP_REMAP_ADDR 0x10000000 

#define CPM_BASE             0x40004000
#define CPM_SSICFG          (*(volatile unsigned int  *)(CPM_BASE + 0x2c)) 
	

extern UINT16 SSI_XIP_Enable(SSI_TypeDef *SSIx);
extern UINT16 SSI_XIP_Switch(SSI_TypeDef *SSIx);
extern void SSI_XIP_Data_Cache_Enable(void);
extern void SSI_XIP_Instruction_Cache_Enable(void);
#endif /* __XIP_DRV_H__ */

