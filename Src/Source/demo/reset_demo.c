// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : reset_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "debug.h"
#include "reset_drv.h"
#include "reset_demo.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                 reset_demo
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Description:
//    Demo of reset.
//
// Input:
//    None.
//
// Output:
//    None.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Reset_Demo(void)
{
	RST_STATUS status;

	status = Get_Reset_Status();
	if (RST_POR == status)
		printf("POR reset\n");

	if (RST_WATCH_DOG_TIMER == status)
		printf("Watch dog timer reset\n");

	if (RST_SOFT == status)
		printf("Soft reset\n");

	if (RST_TC_TIMER == status)
		printf("TC reset\n");
//	Soft_Reset();
	Set_POR_Reset();
}
