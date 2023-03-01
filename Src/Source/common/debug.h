#ifndef __DEBUG_H__
#define __DEBUG_H__ 

#include "type.h"
#include "sys.h"
#include "stdio.h"	  
#include "memmap.h"
#include "uart_drv.h"


#define DEBUG

/* Exported macro ------------------------------------------------------------*/
#ifdef  DEBUG
/*******************************************************************************
* Macro Name     : assert_param
* Description    : The assert_param macro is used for function's parameters check.
*                  It is used only if the library is compiled in DEBUG mode.
* Input          : - expr: If expr is false, it calls assert_failed function
*                    which reports the name of the source file and the source
*                    line number of the call that failed.
*                    If expr is true, it returns no value.
* Return         : None
*******************************************************************************/
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((UINT8 *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  extern void assert_failed(UINT8* file, UINT32 line);
#else
  #define assert_param(expr) ((void)0)
#endif /* DEBUG */
	

extern void UART_Debug_Init(UART_TypeDef *UARTx,UINT32 pclk,UINT32 bound);
#endif	   
















