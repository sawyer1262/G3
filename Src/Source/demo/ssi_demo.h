// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : ssi_demo.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __QSPI_DEMO_H__
#define __QSPI_DEMO_H__
#include "ssi_drv.h"      

//#define SSI_STANDARD_TEST   
//#define SSI_QUAD_TEST       
//#define SSI_DMA_TEST    //栈及全局变量放在0x20000000之后，即IRAM：Start0x20000000,size0x38000
//#define SSI_INTERR_TEST   

#ifdef SSI_DEMO
#define SSI_DMA_INT
#endif

extern void Init_Buf(unsigned char *u8buf,unsigned int buflen,unsigned char inittype,unsigned char initval);
extern void SSI_QQUAD_Demo(SSI_TypeDef *SSIx);
extern void SSI_Demo(void);

#endif /* __QSPI_DEMO_H__ */

