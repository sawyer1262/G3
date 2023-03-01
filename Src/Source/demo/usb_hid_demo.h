// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usb_hid_demo.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __USB_HID_DEMO_H__
#define __USB_HID_DEMO_H__

#include "usb_reg.h"
#include "usb_const.h"

extern void USB_HID_Demo(void);

extern unsigned char hid_rx_buf[512];
extern unsigned char hid_response_buf[512];







#endif /* __USB_HID_DEMO_H__ */
