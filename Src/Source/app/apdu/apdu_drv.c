/*
 * apdu_drv.c
 *
 *  Created on: 2018年8月13日
 *      Author: YangWenfeng
 */
#include <string.h>
#include "sys.h"
#include "reset_drv.h"
#include "cpm_drv.h"
#include "apdu_drv.h"
#include "delay.h"
#include "libRegOpt.h"

const UINT8 ATRString[ATR_LENGTH] = {
		0x3b,0x97,0x96,0x80,0x1f,0xc6,
		0x45,0x41,0x4C,0x34,0x2B,0x5F,0x54,0x04};

const UINT16  FiDi_TBL[15] = {
		0x31DE,0x11FE,0x12FD,0x13FC,0x14FB,0x15FA,0x16F9,0x18F7,
		0x917E,0x927D,0x937C,0x947B,0x957A,0x9679,0x9778 };
const UINT8  FD_Cycle[15] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
		0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

UINT8 adpu_status = APDU_STATE_IDLE;

UINT8 apdu_cmdbuf[260];
UINT8 apdu_rspbuf[256];
volatile UINT8 apdu_cnt      = 0;
UINT16 apdu_datalen = 0;
UINT16 resultCode   = 0;
volatile UINT8 g_selfcheck = 0;
USI_TypeDef* gUSIReg;

void usiDev_init(void)
{
	/*disable simcard reset*/
	CHIP_RESET->RCR &= ~(0x00800000);

	/*disable USI2 I/O pull-down*/
	//USI1->USIPCR = 0;
	USI->USIPCR = 0;

	/*enable USI2 I/O pull-up*/
	IOCTRL_USICR = 0x77770001;

	/*disable USI2 I/O isolation*/
//	CPM->CPM_PWRCR &= ~(0x03<<25);
	CPM_PWRCR_OptBits((0x03<<25),RESET);

	/*enable USI2 card LDO, 5v output*/
	CPM->CPM_SLPCFGR = (CPM->CPM_SLPCFGR &~ 0x0000ff00) | 0x00003300;
	//等待card vcc输出稳定
	while((CPM->CPM_PWRSR & 0x04) != 0x04);
}

/**
 * void SendATR(void)
 * send ATR
 *
 * @param	void
 * @return	void
 * @see		N/A
 */
void SendATR(void)
{
	UINT8 i = 0;

	for ( i = 0 ; i < ATR_LENGTH; i++ )
	{
		TransmitByte(ATRString[i]);
	}
}

u8 PPS_Structure(u8 pps0)
{
	UINT8 i, count = 0;

	for(i=0; i<3; i++)
	{
		if(((pps0>>i)&0x01) == 1)
		{
			count++;
		}
	}

	return (count+3);
}

/**
 * void Send_PTS_ACK(void)
 * Send PPS Ack
 *
 * @param	void
 * @return	void
 * @see		N/A
 */
void Send_PTS_ACK(void)
{
	TransmitByte(apdu_cmdbuf[0]);
	TransmitByte(apdu_cmdbuf[1]);
	TransmitByte(apdu_cmdbuf[2]);
	TransmitByte(apdu_cmdbuf[3]);
}

void Send_Default_ACK(void)
{
	TransmitByte(0xFF);
	TransmitByte(0x00);
	TransmitByte(0xFF);
	gUSIReg->USIBDR = 0x02;
}

/**
 * void PPSManager(void)
 * PPS Manager
 *
 * @param	void
 * @return	void
 * @see		N/A
 */
void PPSManager(void)
{
	UINT8  i = 0;
	UINT16 temp = 0;

	if( apdu_cmdbuf[1] != INS_PPS ){//pps cmd error
		Send_Default_ACK();
		return;
	}

	if((apdu_cmdbuf[0]^apdu_cmdbuf[1]^apdu_cmdbuf[2]) != apdu_cmdbuf[3]){//pps check byte error
		return;
	}

	temp = (((UINT16)apdu_cmdbuf[2] & 0x00ff) << 8);
	temp |= ((UINT16)apdu_cmdbuf[3] & 0x00ff);

	for ( i = 0; i < sizeof(FiDi_TBL)/2;i++ )
	{
		if ( temp == (UINT16)FiDi_TBL[i] )
		{
			Send_PTS_ACK();

			gUSIReg->USIBDR = FD_Cycle[i];

			return;
		}
	}
	if(i == sizeof(FiDi_TBL)/2)
	{
		Send_Default_ACK();
	}
}

void ATR_Handler(void)
{
	if(((gUSIReg->USICR1) & 0x10) == 0)
	{
		gUSIReg->USICR1 = 0x50;
		while(((gUSIReg->USISR) & USISR_TC_MASK) == USISR_TC_MASK);
		while(((gUSIReg->USISR) & USISR_TC_MASK) == 0);
	}
    SendATR();			// 400-40000 cycle OK
    adpu_status = APDU_STATE_IDLE;
}

void USIx_IRQHandler(void)
{
	UINT8 USI_status;

	//indicate interrupt come from which USI

	USI_status = gUSIReg->USISR;

	if(USI_status & USISR_SB_MASK)
	{
		gUSIReg->USISR = USISR_SB_MASK;
	}

	if(USI_status & USISR_ATR_MASK)//answer to reset
	{
		gUSIReg->USIBDR = 0x02;
        gUSIReg->USISR |= USISR_ATR_MASK; //clear ATR
        adpu_status = APDU_STATE_ATR;
        ATR_Handler();
        return;
    }

	//RDRF Receive Data Register Full Flag
	if(USI_status & USISR_RDRF_MASK)
	{
		while((USI_status & USISR_RDRF_MASK) == USISR_RDRF_MASK)
		{
			apdu_cmdbuf[apdu_cnt] = gUSIReg->USIRDR;
			apdu_cnt++;
			USI_status = gUSIReg->USISR;
		}

		if(adpu_status == APDU_STATE_IDLE)
		{
			if(apdu_cmdbuf[0] == 0xFF) //pps cmd
			{
				if(apdu_cnt == 4)
				{
					adpu_status = APDU_STATE_PPS;
					apdu_cnt = 0;
				}
				else if((apdu_cnt == 3) && (apdu_cmdbuf[1] == 0x00))
				{
					adpu_status = APDU_STATE_PPS;
					apdu_cnt = 0;
				}
			}
			else if(apdu_cnt == 5)
			{//apdu cmd
				EN_WTO;
				adpu_status = APDU_STATE_CMD;
			}
		}
		else if((adpu_status == APDU_STATE_DATA) &&(apdu_cnt == apdu_datalen))
		{
			EN_WTO;
			adpu_status = APDU_STATE_DATA_OK;
		}
		// PPS command
		if(adpu_status == APDU_STATE_PPS)
		{
			resultCode = SW_NULL;
			PPSManager();
			adpu_status = APDU_STATE_IDLE;
		}
		return;
	}
}

void USI1_IRQHandler(void)
{
	USIx_IRQHandler();
}


void USI2_IRQHandler(void)
{
	USIx_IRQHandler();
}
/**
 * void InitAPDU(void)
 * Init the USI registers.
 *
 * @param	void
 * @return	void
 * @s
 */
 void InitAPDU(UINT32 usi_baseAddr)
 {
 	UINT8 ret;
 	USI_TypeDef* USIreg = (USI_TypeDef*)usi_baseAddr;

 	USIreg = (USI_TypeDef*)usi_baseAddr;
	 
	gUSIReg = USIreg;		//ATR request on current USI port

 	// set USI control register1
	USIreg->USICR1 = 0x00; // DREN(Enable Delay Request), UART enable, WTEN enable

	// set USI control register2
	USIreg->USICR2 = 0xFF;

	//set WTR
	//USIWTRM = 0x12;

	USIreg->USIGTRL = 20;

	// Fsys=32MHz Fclk=2MHz F=372 D=1
	USIreg->USIBDR = 0x02;

	// set USI port control register
	//USIreg->USIPCR |= 0x20;	//open-drain mode isodata

	USIreg->USIFIFOINTCON = 0x00;

	//Clear USISR,USIRDR
	ret = USIreg->USISR;
	if(ret & USISR_ATR_MASK)
	{
		USIreg->USISR |= USISR_ATR_MASK; //clear ATR
		adpu_status = APDU_STATE_ATR;
		gUSIReg = USIreg;		//ATR request on current USI port
     	//ATR Handler in Normal
		ATR_Handler();
	}
	ret = USIreg->USIRDR;

	USIreg->USIIER = 0x24; //enalbe recv interrupt(RDIE and ATR), 0x24;
 }

UINT8 Transmitdata(UINT8 *pdata, unsigned int len)
{
	unsigned int i;

	for(i=0; i<len; i++){
		while((gUSIReg->USISR & USISR_TDRE_MASK)==0){}
		gUSIReg->USITDR = pdata[i];
		while((gUSIReg->USISR & USISR_TC_MASK)==0){//wait for send done
		}
	}

	return 0;
}

void SendPB (unsigned char val)
{
	/* this is for keep 12 etu */
	delay(0x10);

	TransmitByte(val);
}
































