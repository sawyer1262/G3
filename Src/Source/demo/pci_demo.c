// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : pci_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "pci_drv.h"
#include "pci_demo.h"
#include "debug.h"
#include "cpm_reg.h"

//#define PCI_SDIO_INSIDE_RESISTANCE	//静态SDIO使用内部上下拉
void PCI_Demo(void)
{
	int ret;
	
	PCI_Init(INTERNAL_CLK_SEL);
//	PCI_Init(EXTERNAL_CLK_SEL);
	

	//通过判断PCI的寄存器是否已经配置完成，来保证只配置一次PCI
//	if(PCI->PCI_TDCR != 0x84020a27)	//判断的具体值与实际配置有关
//	{
//		//TD set
//		PCI_TD_Set(PCI_TD_H_LEVEL_40, PCI_TD_L_LEVEL_1, PCI_TD_TRIM_17);
//	}

//	if(PCI->PCI_VDCR != 0x8000007f)	//判断的具体值与实际配置有关
//	{
//		//VD set
		  PCI_VD_Set(PCI_VD_H_LEVEL_1, PCI_VD_L_LEVEL_8);
//	}

//	if(PCI->PCI_SDIOCR != 0x070f0007)	//判断的具体值与实际配置有关
//	{
//		//SDIO set

  		PCI_SDIO_Init();
  		PCI_SDIO_Current_Trim(SDIO_CURRENT_LEVEL_2);
/*
      //dynamic
		  PCI_SDIO_Dynamic_Set(PCI_SDIO_DYNAMIC_CHANNEL_0
												|PCI_SDIO_DYNAMIC_CHANNEL_1
												|PCI_SDIO_DYNAMIC_CHANNEL_2
												|PCI_SDIO_DYNAMIC_CHANNEL_3);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_1, SDIO_PULL_DOWN);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_3, SDIO_PULL_DOWN);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_5, SDIO_PULL_DOWN);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_7, SDIO_PULL_DOWN);
*/

      //static
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_0, 1);
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_1, 0);
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_2, 1);
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_3, 0);
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_4, 1);
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_5, 0);
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_6, 1);
		  PCI_SDIO_Static_Set(PCI_SDIO_STATIC_CHANNEL_7, 0);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_0, SDIO_PULL_DOWN);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_1, SDIO_PULL_UP);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_2, SDIO_PULL_DOWN);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_3, SDIO_PULL_UP);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_4, SDIO_PULL_DOWN);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_5, SDIO_PULL_UP);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_6, SDIO_PULL_DOWN);
		  PCI_SDIO_PullControl(PCI_SDIO_STATIC_CHANNEL_7, SDIO_PULL_UP);
	
//	}

	//带着PCI触发标志位无法写入nvsram数据
  PCI_Clear_Status();
	Nvram_Init();
	
	if(PCI_Detect() == 0)
	{
		Nvram_Write();
	}
	else
	{
		printf("error - 1\n");
	}

	if(*(unsigned int *)SRAM_START_ADDR_M4 == 0x12345678)
	{
		printf("pass\n");
	}
	else
	{
		printf("error - 2\n");
	}
	Nvram_Read();

	while(1)
	{
		ret = PCI_Detect();

		if(ret == 1)
		{
			printf("PCI detect, nvram data clean!!!\n");
			Nvram_Read();
			break;
		}
	}
}
