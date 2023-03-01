// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usbdev_scsi.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "usb_drv.h"

void USBC_IRQHandler(void)
{
#ifdef USB_CCID
	USB_CCID_ISR();
#endif

#ifdef USB_MS
	USB_MS_ISR();
	USBDev_Polling();
#endif
	
#ifdef USB_HID
	USB_HID_ISR();
	USB_HID_Polling();
#endif
	
#ifdef USB_CDC
	USB_CDC_ISR();
#endif
}

