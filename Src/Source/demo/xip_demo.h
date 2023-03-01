// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : xip_demo.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __XIP_DEMO_H__
#define __XIP_DEMO_H__
#include "ssi_drv.h"

extern void XIP_Demo(void);
extern void Read_ID_Test(SSI_TypeDef *SSIx);
extern UINT16 SSI_EFlash_QPI_Enter(SSI_TypeDef *SSIx);
extern UINT16 SSI_EFlash_Set_Read_Para(SSI_TypeDef *SSIx, int waitCycles);

extern void XIP_Demo(void);

#endif
