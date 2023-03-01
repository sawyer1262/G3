#include "type.h"
#include "debug.h"
#include "wdt_drv.h"
#include "wdt_demo.h"


//看门狗WDT_Open()和WDT_Close()接口上电只允许操作其中一个接口，并且只能操作一次，再操作将无效。
//startup_ARMCM4.s中关闭看门狗之后，后续将不能再打开。
//若有重复打开或关闭看门狗的需求，请调用WDT_CloseClkGate()和WDT_OpenClkGate()接口。
void WDT_Demo(void)
{
	int i;

	WDT_Init(0x08ff);		//don't close WDT in startup_ARMCM4.s 

	for(i=0x5000; i>0; i--)
	{
		if( (i%0x400)==0x00 )
		{
			WDT_FeedDog();
			printf("-1- WDT count: 0x%04x\r\n", Get_WDTCount());
		}
		else
		{
			if( (i%150) == 0x00 )
			{
				printf("-2- WDT count: 0x%04x\r\n", Get_WDTCount());
			}
		}

	}
	
//	while(1);

	printf("NO feed ...\r\n");
	while(1)
	{
		i++;
		if( (i%0x1000) == 0x00 )
		{
			printf("WDT count: 0x%04x\r\n", Get_WDTCount());
		}
	}
}
