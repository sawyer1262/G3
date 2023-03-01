// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usb_cdc_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "string.h"
#include "usb_cdc_demo.h"
#include "usb_drv.h"
#include "type.h"
#include "debug.h"
#include "common.h"
#include "usb_const.h"

UINT8  cdc_ctl_buf[512];

/*************************************************
Function: USBDev_DoCDCCmd
Description: CCID类的处理
Calls:
Called By:
Input: 无
Output: 无
Return: 无
Others: 无
*************************************************/
void USBDev_DoCDCCmd(void)
{
	UINT16 recvLen = 0;
	//UINT16 i = 0;
	UINT16 len;

#if USB_MODE_1D1
	len = 64;
#elif USB_MODE_2D0
	len = 512;
#else
	len = 64;
#endif

	memset(cdc_ctl_buf, 0x00, sizeof(cdc_ctl_buf));
	recvLen = usb_cdc_receive(INDEX_EP2, (UINT8 *)cdc_ctl_buf);
	if (recvLen == 0)
		return;
	printf("USB Recv Len = 0x%04x:\r\n", recvLen);
	//for(; i < recvLen; i++)
	// printf("%s ", cdc_ctl_buf);
	printf("\r\n");
	if(len == recvLen)
	{
		usb_cdc_send(INDEX_EP1, (UINT8*)cdc_ctl_buf, recvLen);
		usb_cdc_send_null(INDEX_EP1);

	}
	else
	{
		usb_cdc_send(INDEX_EP1, (UINT8*)cdc_ctl_buf, recvLen);
	}
	
	
}

void usb_cdc_poll(void)
{
	if( (g_uchUSBStatus & BIT1) == BIT1 )     //接收到一包数据
	{
		g_uchUSBStatus &= ~BIT1;
		printf("USBDev_DoCDCCmd\r\n");
		USBDev_DoCDCCmd();
	}
}


void USB_CDC_Demo(void)
{
	printf("USB_CDC_Demo \r\n");
	Disable_Interrupts;
	USB_CDC_Init();
	Enable_Interrupts;

	while(1)
	{
		usb_cdc_poll();
	}
}
