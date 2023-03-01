// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usb_hid_isr.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "usb_reg.h"
#include "usb_const.h"
#include "usb_drv.h"

unsigned char usb_tx_int_flag = 0;

/*
Description:USB interrupt service routine
*/
void USB_HID_ISR(void)
{
	//check common interrupt
	g_msgflags |= gUSBC_ComReg->INTRUSB;

    //Check RX interrupt
	if((gUSBC_ComReg->INTRRX_L)&(1<<USB_ENDPOINT_INDEX))
 	{
		g_msgflags |= 0x10;
 	}
	//check TX interrupt
	g_msgflags1 |=gUSBC_ComReg->INTRTX_L;
}
//#pragma interrupt off

/*
Description: USB Endpoint0 Interrupt Service Routine,respond control transfer
*/
void USB_HID_EP0_ISR(void)
{
	unsigned char ucReg=0;

	//enable ep0 register map to be accessed
	gUSBC_ComReg->EINDEX = CONTROL_EP;

	ucReg=gUSBC_IdxReg->E0CSR_L;	
	//clear SentStall bit
	if(ucReg &0x04)
	{
		ucReg &=0xFB;
	}
	//clear SetupEnd bit
	if(ucReg & 0x10)
	{
		//set ServiceSetupEnd = 1;
		ucReg |=0x80;
	}
	gUSBC_IdxReg->E0CSR_L = ucReg;
	
	//if RxPktRdy=1,Data come into ep0 buf
	if(ucReg & 0x01)
	{
		USB_HID_Ep0Handler();
	}

}


/////////////////////////////////////////////////////////////////////////////////////////
unsigned short USB_HID_EP1_RX_ISR(unsigned char* pData)
{
	unsigned char  ucRegLow;
	unsigned char  number;
	number = 0;
	gUSBC_ComReg->EINDEX = USB_ENDPOINT_INDEX;
	ucRegLow  = gUSBC_IdxReg->RXCSR_L;
	if(ucRegLow & DEV_RXCSR_SENTSTALL)
	{
		ucRegLow  &= 0xBF;
		ucRegLow  |= DEV_RXCSR_CLR_DATA_TOG;
		gUSBC_IdxReg->RXCSR_L  = ucRegLow;
	}
	if(ucRegLow & DEV_RXCSR_RXPKTRDY)
  	{
  		number = usb_receive(pData);
  	}
  	return (number);
}

void USB_HID_EP1_TX_ISR(unsigned char* pData,unsigned short len)
{
	gUSBC_ComReg->EINDEX  = USB_ENDPOINT_INDEX;
	usb_send(pData,len);	
}
