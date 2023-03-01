#include "string.h"
#include "usb_ccid_demo.h"
#include "usb_drv.h"
#include "type.h"
#include "debug.h"
#include "common.h"
#include "usb_const.h"



ccid_bulk_in_header  bulkin;
ccid_bulk_out_header bulkout;

UINT8 bSlotStatus;
UINT8 bClkStatus;

static UINT8 const ATR_DEF[17] = {0X3B,0X6D,0X00,0X00,0X00,0X81,0X4D,0X22,0X08,0X86,0X60,0X12,0X22,0X29,0X80,0X00,0X01};
static UINT8 version[] = "C*Core USB CCID Demo V1.0";
/*************************************************
Function: USBDev_DoCCIDCmd
Description: CCID类的处理
Calls:
Called By:
Input: 无
Output: 无
Return: 无
Others: 无
*************************************************/
void usb_ccid_poll(void)
{
	UINT32 i;
	UINT8 bMessageToSend = FALSE;//Flag determining wether response or not
	UINT32 apduLen = 0;//command length
	UINT16 resLen = 0;//response apdu length
	UINT16 recvLen = 0;
	UINT8 atrLen;

	recvLen = USB_CCID_receive(USB_ENDPOINT_INDEX, (UINT8 *)&bulkout);
	if (recvLen == 0)
		return;
	
	//get Lc value from received packet
	apduLen = (bulkout.wLength[3]<<24);
	apduLen += (bulkout.wLength[2]<<16);
	apduLen += (bulkout.wLength[1]<<8);
	apduLen += bulkout.wLength[0];

	//clear bulkin
	memset((UINT8*)&bulkin, 0, sizeof(bulkin));

	//set the default value of response tpdu
	bulkin.bMessageType = 0x81;			//bMessageType
	bulkin.wLength[0] = 0x00;			//dwLength1
	bulkin.wLength[1] = 0x00;			//dwLength2
	bulkin.wLength[2] = 0x00;			//dwLength3
	bulkin.wLength[3] = 0x00;			//dwLength4
	bulkin.bSlot = bulkout.bSlot;		//bSlot
	bulkin.bSeq = bulkout.bSeq;			//bSeq
	bulkin.bStatus = 0;					//bStatus
	bulkin.bError = 0;					//bError
	bulkin.bSpecific = 0;				//Specific

	//the length to send
	bulkin.bSizeToSend = 10;			//response header length

	//the default status of slot 0,the only slot supported now
	bSlotStatus = 0;

	//default status of clock status
	bClkStatus = 0;

	//ccid command dispatch
	switch(bulkout.bMessageType)
	{
		case PC_to_RDR_IccPowerOn:  //POWERON COMMAND
		{
			//atrLen = sizeof(a_ATRData);
			bulkin.bMessageType = 0x80;
			atrLen = 17;
			memcpy(bulkin.abData,ATR_DEF,17);
			bulkin.wLength[0] = atrLen;
			bulkin.bSizeToSend += atrLen ;	//totle return length
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_IccPowerOff:  //POWEROFF COMMAND
		{
			bClkStatus = 0;
			bulkin.bStatus = bClkStatus;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_GetSlotStatus:  //SLOTSTATUS COMMAND
		{
			bulkin.bStatus = 0x01;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_XfrBlock:  //XFRBLOCK COMMAND
		{
			bulkin.bMessageType = 0x80;
			bulkin.bStatus = bSlotStatus;
			resLen = 0;
			/*处理命令*/

			switch(bulkout.APDU[1])
			{
			 case 0x5C:   //获取版本号
				  memcpy(bulkin.abData,version,sizeof(version));
				  bulkin.abData[sizeof(version)] = 0x90;
				  bulkin.abData[sizeof(version)+1] = 0x00;
				  resLen = sizeof(version)+2;
				  break;
			 default:
				 bulkin.abData[0] = 0x6D;
				 bulkin.abData[1] = 0x00;
				 resLen = 2;
			}

			bulkin.wLength[0] = resLen & 0xFF;
			bulkin.wLength[1] = resLen >> 8;
			bulkin.bSizeToSend += resLen;

			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_GetParameters:  //GETPARAMETER COMMAND
		{
			bulkin.bMessageType = 0x82;
			bulkin.wLength[0] = 0x05;
			bulkin.bSizeToSend += 5;
			bulkin.abData[0] = 0x18;
			bulkin.abData[1] = 0x00;
			bulkin.abData[2] = 0x00;
			bulkin.abData[3] = 0x00;
			bulkin.abData[4] = bClkStatus;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_ResetParameters:  //RESETPARAMETER COMMAND
		{
			bSlotStatus = 0;
			bClkStatus = 0;
			bulkin.bMessageType = 0x82;
			bulkin.wLength[0] = 0x05;
			bulkin.bStatus = bSlotStatus;
			bulkin.bSizeToSend += 5;
			bulkin.abData[0] = 0x18;
			bulkin.abData[1] = 0x00;
			bulkin.abData[2] = 0x00;
			bulkin.abData[3] = 0x00;
			bulkin.abData[4] = bClkStatus;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_SetParameters:  //SETPARAMETER COMMAND
		{
			bSlotStatus = 0;
			bulkin.bMessageType = 0x82;
			bulkin.wLength[0] = 0x05;
			bulkin.bStatus = bSlotStatus;
			bulkin.bSizeToSend += 5;
			bClkStatus = bulkout.APDU[4];
			bulkin.abData[0] = bulkout.APDU[0];
			bulkin.abData[1] = bulkout.APDU[1];
			bulkin.abData[2] = bulkout.APDU[2];
			bulkin.abData[3] = bulkout.APDU[3];
			bulkin.abData[4] = bulkout.APDU[4];
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_Escape:  //ESCAPE COMMAND
		{
			bulkin.bMessageType = 0x83;
			bulkin.wLength[0] = bulkout.wLength[0];
			bulkin.wLength[1] = bulkout.wLength[1];
			bulkin.wLength[2] = bulkout.wLength[2];
			bulkin.wLength[3] = bulkout.wLength[3];
			for(i=0;i<bulkout.wLength[0];i++)
				bulkin.abData[i] = bulkout.APDU[i];
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_IccClock:  //ICCCLOCK COMMAND
		{
			bulkin.bMessageType = 0x81;
			if(0 == bulkout.bSpecific0)
			{
				if(0 != bClkStatus)
					bClkStatus = 0;
				bulkin.bStatus = 0;
			}
			else
			{
				bClkStatus = bulkout.bSpecific0;
				bulkin.bStatus |= 0x40;
			}
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_T0APDU:  //TOAPDU COMMAND
		{
			bulkin.bMessageType = 0x81;
			bulkin.bStatus |= 0x40;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_Secure:  //SECURE COMMAND
		{
			bulkin.bMessageType = 0x80;
			bulkin.bStatus |= 0x40;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_Mechanical:  //MECHANICAL COMMAND
		{
			bulkin.bMessageType = 0x81;
			bulkin.bStatus = bSlotStatus;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_Abort:  //ABORT COMMAND
		{
			bulkin.bMessageType = 0x81;
			bulkin.bStatus = bSlotStatus;
			bMessageToSend = TRUE;
			break;
		}
		case PC_to_RDR_SetDataRateAndClockFrequency:  //SETDATARATEANDCLOCKFREQENCY COMMAND
		{
			bulkin.bMessageType = 0x84;
			bulkin.wLength[0] = 0x08;
			bulkin.bSizeToSend += 8;
			bulkin.abData[0] = 0x00;
			bulkin.abData[1] = 0x00;
			bulkin.abData[2] = 0x0f;
			bulkin.abData[3] = 0xa0;
			bulkin.abData[4] = 0x00;
			bulkin.abData[5] = 0x00;
			bulkin.abData[6] = 0x25;
			bulkin.abData[7] = 0x80;
			bMessageToSend = TRUE;
			break;
		}
		default:
		{
			bulkin.bMessageType = 0x81;
			bulkin.bStatus = 0x01;
			bMessageToSend = TRUE;
			break;
		}
	}

	if(TRUE == bMessageToSend)
	{
		USB_CCID_send(USB_ENDPOINT_INDEX, (UINT8*)&bulkin, bulkin.bSizeToSend);
	}
}


void USB_CCID_Demo(void)
{
	
	printf("USB_CDC_Demo \r\n");
	Disable_Interrupts;
	USB_CCID_Init();
	Enable_Interrupts;
	
	while(1)
	{
		usb_ccid_poll();
	}
	
}

