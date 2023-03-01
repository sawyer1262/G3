// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usb_hid_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "string.h"
#include "usb_hid_demo.h"
#include "usb_drv.h"
#include "type.h"
#include "debug.h"
#include "common.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                 USB_HID_Demo
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Description:
//    Demo of usb hid dev.
//
// Input:
//    None.
//
// Output:
//    None.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void USB_HID_Demo(void)
{
	printf("usb hid demo\r\n");
	Disable_Interrupts;
	USB_HID_Init();
	Enable_Interrupts;

	while(1)
	{
//		USB_HID_Polling();
	}
}
