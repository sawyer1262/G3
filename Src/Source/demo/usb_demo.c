// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usb_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "usb_demo.h"
#include "usb_drv.h"
#include "usb_ms_demo.h"
#include "usb_cdc_demo.h"
#include "usb_hid_demo.h"
#include "usb_ccid_demo.h"

void USB_Demo(void)
{
#ifdef USB_MS
	USB_MS_Demo();
#endif
	
#ifdef USB_HID
	USB_HID_Demo();
#endif
	
#ifdef USB_CDC
	USB_CDC_Demo();
#endif
	
#ifdef USB_CCID
	USB_CCID_Demo();
#endif
}
