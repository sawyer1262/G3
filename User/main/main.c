#include "sys.h"
#include "debug.h"
#include "type.h"
#include "startup_demo.h"
#include "main.h"
#include "cpm_drv.h"
#include "common.h"


/*¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª·â×°ÀàÐÍ¼°Òý½Å·ÖÅä¡ª¡ª¡ª¡ª¡ª¡ª|
Òý½Å       | QFN32 | QFN40 | QFN48 | QFN88 |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
SPI        |   2   |   1   |   3   |   3   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
I2C        |   1   |   1   |   1   |   3   |   
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
SSI        |   0   |   1   |   0   |   1   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
UART       |   1   |   1   |   3   |   3   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
ISO7816    |   2   |   0   |   0   |   1   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
USB_OTG    |   1   |   1   |   1   |   1   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
ADC        |   0   |   1   |   2   |   3   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
DAC        |   0   |   0   |   1   |   1   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
PWM(¸´ÓÃ)  |   3   |   4   |   2   |   4   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
MCC        |   0   |   0   |   0   |   0   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
TSI(¸´ÓÃ)  |   8   |   5   |  10   |  16   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
EPORT(¸´ÓÃ)|  10   |  18   |  19   |  20   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
SDIO(·À²ð) |   0   |   0   |   0   |   1   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
C0 SUB I/O |   0   |   0   |   0   |   4   |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|*/

/*¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ªUSB Ö§³ÖÇé¿ö¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
·â×°   | USB2.0 | USB1.1 |  host  | device |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
QFN32  | ²»¿ÉÒÔ |  ¿ÉÒÔ  | ²»Ö§³Ö |  Ö§³Ö  |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
QFN40  |  ¿ÉÒÔ  |  ¿ÉÒÔ  | ²»Ö§³Ö |  Ö§³Ö  |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
QFN48  | ²»¿ÉÒÔ |  ¿ÉÒÔ  | ²»Ö§³Ö |  Ö§³Ö  |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|
QFN88  | ²»¿ÉÒÔ |  ¿ÉÒÔ  | ²»Ö§³Ö |  Ö§³Ö  |
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª|*/


#include "eport_drv.h"
#include "audio.h"
#include "led.h"
#include "key.h"
#include "keypad_menu.h"
#include "uart.h"
#include "uart_fp.h"
#include "password.h"
#include "lpm.h"
#include "uart_back.h"
#include "pci_drv.h"
#include "tc_drv.h"
#include "cache_drv.h"
#include "card.h"  
#include "uart_face.h"
#include "bat.h"
#include "delay.h"
#include "rtc.h"
#include "lock_config.h"
#include "wdt_drv.h"
#include "time_cal.h"
#include "motor.h"


extern void IO_Latch_Clr(void);


void Reback_Boot(void)
{
#ifdef MY_DEBUG
		IO_Latch_Clr();
//µ÷ÊÔÊ±¿ªÆô±£ÏÕ£¬ÒÔ·ÀÐ¾Æ¬±»Ëø×¡
		EPORT_ConfigGpio(KEY_TAMPER_PIN,GPIO_INPUT);            
		EPORT_ConfigGpio(KEY_BACK_PIN,GPIO_INPUT);  
		EPORT_PullupConfig(KEY_BACK_PIN,ENABLE);            //Ê¹ÄÜÉÏÀ­ 
    if(EPORT_ReadGpioData(KEY_TAMPER_PIN) == Bit_RESET)           //·À²ðËÉ¿ª£¬ËÉ¿ªÎªµÍ
    {
				DelayMS(20);
				if(EPORT_ReadGpioData(KEY_BACK_PIN) == Bit_RESET)      //ºó°å°´¼ü°´ÏÂ 
				{
						Demo_StartupConfig(Startup_From_ROM, ENABLE);
						while(1);
				}			
    }s
#else
		DelayMS(20);
#endif
}

int main()
{
		e_CPM_wakeupSource_Status rst_source=*(e_CPM_wakeupSource_Status *)CPM_WAKEUPSOURCE_REG;         //¶ÁÈ¡»½ÐÑÔ´
		Sys_Init();                                                 //ÏµÍ³³õÊ¼»¯
		Reback_Boot();
		UART_Debug_Init(SCI1,g_ips_clk,9600);                     //µ÷ÊÔ´òÓ¡³õÊ¼»¯
		printf("wakeup REG: 0x%08x \n",CPM->CPM_PADWKINTCR);
		printf("wakeup source: 0x%02x \n",rst_source);
		switch(rst_source)
		{
//				case WAKEUP_SOURCE_WAKEUP:                //PIR»½ÐÑ
//						LedVal.ledval=LED_MASK_WAKE;
//						break;
//				case WAKEUP_SOURCE_TSI:                   //´¥Ãþ°´¼ü»½ÐÑ
//						LedVal.ledval=LED_MASK_WAKE;
//						break;
//				case WAKEUP_SOURCE_GIN3:                  //ºó°å´®¿Ú»½ÐÑ
//						LedVal.ledval=LED_MASK_WAKE;
//						break;
//				case WAKEUP_SOURCE_GIN4:                  //SET°´¼ü»½ÐÑ
//						LedVal.ledval=LED_MASK_WAKE;
//						break;
//				case WAKEUP_SOURCE_GIN5:                  //·À²ð°´¼ü»½ÐÑ
//						LedVal.ledval=LED_MASK_WAKE;
//						break;
				case WAKEUP_SOURCE_POR:                     //ÉÏµç¸´Î»£ºÁÁËùÓÐµÆ£¬·ÇÉÏµç¸´Î»Ö»ÁÁÃÅÁåÍ¼±ê
						LedVal.ledval=LED_MASK_WAKE;
						PowerOnCheckBatDelay=5000; 
						break;
				default:
						LedVal.ledval=LED_MASK_WAKE;        
						PowerOnCheckBatDelay=0;
						break;
				}  //24+12+4=40,
		Motor_Init();
		Led_Init();                         //ÏÈ³õÊ¼»¯LED
		Timer_Init();                       //¶¨Ê±Æ÷³õÊ¼»¯
		SSI_FLASH_Init();                   //FLASH³õÊ¼»¯
		RtcInit();                          //RTC³õÊ¼»¯ 
		LockReadSysConfig();                //¼ÓÔØÏµÍ³ÉèÖÃÏî 
		Audio_Init();                       //ÓïÒô³õÊ¼»¯ 
		Uart_DriverInit();                  //´®¿ÚÇý¶¯³õÊ¼»¯
		Uart_Init();                        //´®¿Ú³õÊ¼»¯
		TampSetButtonInit();                //·À²ðµÈ°´¼ü³õÊ¼»¯
		PassWordInit();                     //ÃÜÂë³õÊ¼»¯    
		Key_Init();                         //´¥Ãþ°´¼ü³õÊ¼»¯
		CardIoInit();                       //¿¨Æ¬IO³õÊ¼»¯
		WDT_Init(0xffff);                   //¿´ÃÅ¹·³õÊ¼»¯ 
		WDT_FeedDog();                      //³õÊ¼»¯Î¹¹·
		Key_Scan();                         //ÉÏµç¶ÁÒ»´Î×´Ì¬£¬·ÀÖ¹»½ÐÑµçÁ¿ËùÓÐ¼üÅÌµÆ
		MenuNow=Menu_KeyPadNormal;          //µ¼ÈëÖ÷²Ëµ¥ 
		LPM_SetStopMode(LPM_POWERON_ID,LPM_Disable);   //ÉÏµçÑÓÊ±5S
		while(1)
		{
				MenuNow();                      //ÔËÐÐ²Ù×÷²Ëµ¥  
		}
}
