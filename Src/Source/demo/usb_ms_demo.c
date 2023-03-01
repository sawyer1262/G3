// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usb_ms_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "usb_ms_demo.h"
#include "usb_drv.h"
#include "type.h"
#include "debug.h"
#include "common.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                 usb_demo
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Description:
//    Demo of usb.
//
// Input:
//    None.
//
// Output:
//    None.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void USB_MS_Demo(void)
{
	printf("USB_MS_Demo\r\n");

	Disable_Interrupts;
	USB_Init();
	Enable_Interrupts;
	while(1)
	{
//		USBDev_Polling();
	}
}
