/**
 * @file cpm_drv.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 2.1
 * @date 2021-03-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "debug.h"
#include "delay.h"
#include "cpm_drv.h"
#include "ccm_drv.h"
#include "pci_drv.h"
#include "tc_drv.h"
#include "eport_drv.h"
#include "iomacros.h"
#include "cache_drv.h"
#include "ioctrl_drv.h"
#include "clk_switch_drv.h"
#include "eflash_drv.h"
#include "main.h"
#include "tsi_hal.h"
#include "reset_drv.h"
#include "c0_drv.h"

#define USI1_SB_IT 1

/*******************************************************************************
* Function Name  : UsbDet_Enable
* Description    : usb det enable functon
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void UsbDet_enable(void)
{
    CPM->CPM_PADWKINTCR |= 0x00010100;
    NVIC_Init(3, 3, CPM_IRQn, 2);
}

/*******************************************************************************
* Function Name  : UsbDet_Disable
* Description    : usb det disable function
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void UsbDet_disable(void)
{
    CPM->CPM_PADWKINTCR &= 0x11101011;
}

/*******************************************************************************
* Function Name  : UsbDet_callback
* Description    : usb det callback function
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
__weak void UsbDet_callback(void)
{
    UsbDet_disable();
}

/*******************************************************************************
* Function Name  : CPM_IRQHandler
* Description    : PCI唤醒休眠中断
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_IRQHandler(void)
{
    const volatile uint32_t flag = CPM->CPM_PADWKINTCR;
    volatile uint32_t temp = CPM->CPM_PADWKINTCR;

    // UART_Debug_Init(SCI1, 60000000, 115200);
    // printf("CPM IRQ: %08x\r\n", flag);
#ifndef LOWPOWER_CONFIG
    if (flag & (1u << 7)) // SubC0
    {
        drv_SubC0_callback();
    }
#endif
    if ((flag & (1u << 11)) && // PCI AsycTimer
        // (flag & (1u << 19)) &&
        (flag & (1u << 3)))
    {
        PCI_AsycTimer_callback();
    }
    if ((flag & (1u << 10)) && // SDIO
        // (flag & (1u << 18)) &&
        (flag & (1u << 2)))
    {
        CPM->CPM_PADWKINTCR |= (1u << 2);
    }
    if ((flag & (1u << 9)) && // Wakeup-pin
        // (flag & (1u << 17)) &&
        (flag & (1u << 1)))
    {
        CPM->CPM_PADWKINTCR |= (1u << 1);
    }
    if ((flag & (1u << 8)) && // USBDET-pin
        // (flag & (1u << 16)) &&
        (flag & (1u << 0)))
    {
        CPM->CPM_PADWKINTCR |= (1u << 0);
        // printf("USB_DET IRQ\n");
        UsbDet_callback();
    }
}

static void CPM_overwrite_test_mode(unsigned int ovwr_data)
{
    CPM_Write_CoreTestKey(1);
    CPM->CPM_VCCCTMR |= ovwr_data;
    // CPM->CPM_VCCCTMR &= 0x3fffffff;
    CPM_Write_CoreTestKey(0);
}

/*******************************************************************************
* Function Name  : CPM_Sleep
* Description    : CPU进入Sleep模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_Sleep(void)
{
#if SUPPER_EDGE_WAKEUP
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc600000)) | 0x1c600000); //low power mode,eport 0~4 sleep enable,supper edge wake up
#else
    CPM->CPM_SLPCFGR &= (~0xdc600000);                                    //low power mode,eport 0~4 sleep disable,don't supper edge wake up
#endif
    CPM->CPM_SLPCFGR2 &= ~(VDD_WK_SWOFF | VDD_PD_RETENT | CPM_IPS_SLPEN);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X0028DF13;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X00060000);  //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0X00000080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0X00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0x04 << 4); //limit peri_clk_en = 30us


    //CPM->CPM_PADWKINTCR = 0x000F0FFF;
    //NVIC_Init(3, 3, CPM_IRQn, 2);
    Sys_Standby();
}

/*******************************************************************************
* Function Name  : CPM_PowerOff_1
* Description    : CPU进入PowerOff_1模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_PowerOff_1(void)
{
#if SUPPER_EDGE_WAKEUP
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40280000); //Power OFF 1 mode,eport 0 sleep enable,supper edge wake up
#else
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40000000); //Power OFF 1 mode,eport 0 sleep disable,don't supper edge wake up
#endif

    CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 & (~(VDD_WK_SWOFF | CPM_IPS_SLPEN))) | VDD_PD_RETENT);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X00001000;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X00860000);  //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0x000f0080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0x00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0X04 << 4); //limit peri_clk_en = 30us

    CPM->CPM_PADWKINTCR |= 0xFF; //wake up -pad/pad ss3/usbdet wake up enable/sddc/cpi_atimer/pci_det wake up disbale

    CPM->CPM_VCCGTRIMR |= (1 << 13);   //auto latch & auto latch clear.
    CPM->CPM_VCCGTRIMR &= ~(1u << 12); // disable auto latch clear.


    NVIC_Init(3, 3, CPM_IRQn, 2);

    Sys_Standby();
}

/*******************************************************************************
* Function Name  : CPM_PowerOff_1p5
* Description    : CPU进入PowerOff_1p5模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_PowerOff_1p5(void)
{
#if SUPPER_EDGE_WAKEUP
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40280000); //Power OFF 1.5 mode,eport 0 sleep enable,supper edge wake up
#else
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40000000); //Power OFF 1.5 mode,eport 0 sleep disable,don't supper edge wake up
#endif
    //CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 &(~VDD_WK_SWOFF))|VDD_PD_RETENT);?!!
    CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 & (~(VDD_PD_RETENT | CPM_IPS_SLPEN))) | VDD_WK_SWOFF);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X00000000;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X00860000);   //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0x000f0080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0x00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0X04 << 4); //limit peri_clk_en = 30us

    // CPM->CPM_PADWKINTCR = 0x000808FF;
    //CPM->CPM_CHIPCFGR |= (1u << 17);  // Control bit PCI_H2L_ISOEN in CPM module is select
    //CPM->CPM_CHIPCFGR &= ~(1u << 16); // PCI H2L Isolation disable

    CPM->CPM_VCCGTRIMR |= (1u << 13);  //auto latch &
    CPM->CPM_VCCGTRIMR &= ~(1u << 12); // disable auto latch clear.
		
		CPM->CPM_PADWKINTCR |= (1<<9)|(1<<17);        //开启WAKE引脚唤醒
		
		CPM->CPM_PADWKINTCR |= 0xFF;
		
    NVIC_Init(3, 3, CPM_IRQn, 2);
		
    Sys_Standby();
}

void ATE_CPM_PowerOff_1p5(void)
{
#if SUPPER_EDGE_WAKEUP
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40280000); //Power OFF 1.5 mode,eport 0 sleep enable,supper edge wake up
#else
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40000000); //Power OFF 1.5 mode,eport 0 sleep disable,don't supper edge wake up
#endif
    //CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 &(~VDD_WK_SWOFF))|VDD_PD_RETENT);?!!
    CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 & (~(VDD_PD_RETENT | CPM_IPS_SLPEN))) | VDD_WK_SWOFF);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X00000000;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X00860000);   //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0x000f0080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0x00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0X04 << 4); //limit peri_clk_en = 30us

    // CPM->CPM_PADWKINTCR = 0x000808FF;
    //CPM->CPM_CHIPCFGR |= (1u << 17);  // Control bit PCI_H2L_ISOEN in CPM module is select
    //CPM->CPM_CHIPCFGR &= ~(1u << 16); // PCI H2L Isolation disable

    CPM->CPM_VCCGTRIMR |= (1u << 13);  //auto latch &
    CPM->CPM_VCCGTRIMR &= ~(1u << 12); // disable auto latch clear.
	
	
		CPM->CPM_PADWKINTCR &= ~((1<<9)|(1<<17));
		
		CPM->CPM_PADWKINTCR |= 0xFF;
		
		NVIC_DeInit(3, 3, CPM_IRQn, 2);
		
    Sys_Standby();
}

/*******************************************************************************
* Function Name  : CPM_PowerOff_2
* Description    : CPU进入power off 2模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_PowerOff_2(void)
{
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0Xdc600000)) | 0x80000000); //Power OFF 2 mode,eport 1~4 sleep disable,don't supper edge wake up

    CPM->CPM_SLPCFGR2 &= ~(VDD_WK_SWOFF | VDD_PD_RETENT | CPM_IPS_SLPEN);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X0028df13;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X00060000);    //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0X00000080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0x00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0X04 << 4); //limit peri_clk_en = 30us

    CPM->CPM_PADWKINTCR = 0x001313FF; //wake up -pad/pad ss3/usbdet wake up enable/sddc/cpi_atimer/pci_det wake up disbale

    NVIC_Init(3, 3, CPM_IRQn, 2);

    Sys_Standby();
}

/*******************************************************************************
* Function Name  : CPM_Card_Sleep
* Description    : 卡模式下CPU进入sleep模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_Card_Sleep(void)
{
//Start bit Interrupt Enable Bit
#if USI1_SB_IT
    *(volatile UINT32 *)0x40009004 |= 0x01; //USI1 USI_SB_IT_EN
#else
    *(volatile UINT32 *)0x40015004 |= 0x01;                               //USI2 USI_SB_IT_EN
#endif

#if SUPPER_EDGE_WAKEUP
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc600000)) | 0x1c600000); //low power mode,eport 1~4 sleep enable,supper edge wake up
#else
    CPM->CPM_SLPCFGR &= (~0xdc600000);                                    //low power mode,eport 1~4 sleep disable,don't supper edge wake up
#endif
    CPM->CPM_SLPCFGR2 &= ~(VDD_WK_SWOFF | VDD_PD_RETENT | CPM_IPS_SLPEN);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X0028DF13;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X01070000);  //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0X00000080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0X00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0X04 << 4); //limit peri_clk_en = 30us

    CPM->CPM_PADWKINTCR = 0x001313FF; //wake up -pad/pad ss33/usbdet wake up enable/sddc/cpi_atimer/pci_det wake up disbale

#if USI1_SB_IT
    *(volatile UINT32 *)0x40009004 &= ~0x01; //USI1 USI_SB_IT_DIS
#else
    *(volatile UINT32 *)0x40015004 &= ~0x01;                              //USI2 USI_SB_IT_DIS
#endif

    NVIC_Init(3, 3, CPM_IRQn, 2);

    Sys_Standby();
}

/*******************************************************************************
* Function Name  : CPM_Card_PowerOff1
* Description    : 卡模式下CPU进入power off 1模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_Card_PowerOff1(void)
{
//Start bit Interrupt Enable Bit
#if USI1_SB_IT
    *(volatile UINT32 *)0x40009004 |= 0x01; //USI1 USI_SB_IT_EN
#else
    *(volatile UINT32 *)0x40015004 |= 0x01;                               //USI2 USI_SB_IT_EN
#endif

#if SUPPER_EDGE_WAKEUP
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40280000); //Power OFF 1 mode,eport 1~4 sleep enable,supper edge wake up
#else
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40000000);
    ;                                        //Power OFF 1 mode,eport 1~4 sleep disable,don't supper edge wake up
#endif

    CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 & (~(VDD_WK_SWOFF | CPM_IPS_SLPEN))) | VDD_PD_RETENT);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X00001000;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X01870000);  //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0x000f0080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0x00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0X04 << 4); //limit peri_clk_en = 30us

    CPM->CPM_PADWKINTCR = 0x001313FF; //wake up -pad/pad ss33/usbdet wake up enable/sddc/cpi_atimer/pci_det wake up disbale

    CPM->CPM_VCCGTRIMR |= (1 << 12) | (1 << 13); //auto latch & auto latch clear.

#if USI1_SB_IT
    *(volatile UINT32 *)0x40009004 &= ~0x01; //USI1 USI_SB_IT_DIS
#else
    *(volatile UINT32 *)0x40015004 &= ~0x01; //USI2 USI_SB_IT_DIS
#endif

    NVIC_Init(3, 3, CPM_IRQn, 2);

    Sys_Standby();
}

/*******************************************************************************
* Function Name  : CPM_Card_PowerOff1p5
* Description    : 卡模式下CPU进入power off 1.5模式
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_Card_PowerOff1p5(void)
{
//Start bit Interrupt Enable Bit
#if USI1_SB_IT
    *(volatile UINT32 *)0x40009004 |= 0x01; //USI1 USI_SB_IT_EN
#else
    *(volatile UINT32 *)0x40015004 |= 0x01;  //USI2 USI_SB_IT_EN
#endif

#if SUPPER_EDGE_WAKEUP
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40280000); //Power OFF 1.5 mode,eport 1~4 sleep enable,supper edge wake up
#else
    CPM->CPM_SLPCFGR = ((CPM->CPM_SLPCFGR & (~0xdc680000)) | 0x40000000);
    ;                                        //Power OFF 1.5 mode,eport 1~4 sleep disable,don't supper edge wake up
#endif
    //CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 &(~VDD_WK_SWOFF))|VDD_PD_RETENT);?!!
    CPM->CPM_SLPCFGR2 = ((CPM->CPM_SLPCFGR2 & (~(VDD_PD_RETENT | CPM_IPS_SLPEN))) | VDD_WK_SWOFF);

    IOCTRL->SWAP_CONTROL_REG |= (1 << 2) | (1 << 7); //sdclk/sddat[0:3] swap enable

    CPM->CPM_MPDSLPCR = 0X00000000;

    //CPM->CPM_VCCCTMR = 0X40000000;
    //CPM->CPM_VCCCTMR = 0X80000000;
    //CPM->CPM_VCCCTMR = 0XC0000000;
    //CPM->CPM_VCCCTMR |= (0XC0000000|0X01870000);   //overwrite OSCL/OSCH STABLE TRIM
    CPM_overwrite_test_mode(0x01860000); //overwrite OSCL STABLE/OSCH STABLE/VCC/CARDLDO TRIM.

    CPM->CPM_SLPCNTR = 0x000f0080;
    CPM->CPM_WKPCNTR = 0X00000080;
    CPM->CPM_FILTCNTR = 0x00000040;

    CPM->CPM_OSCLSTIMER = 0X00000000;
    CPM->CPM_OSCHSTIMER = 0X001001FF;

    CPM->CPM_CARDTRIMR = (CPM->CPM_CARDTRIMR | (WKP_DFILT_GTE | WKP_DFILT_EN | WKP_AFILT_BYPASS)) & ~WKP_DFILT_BYPASS; //wake-up digit filter enable, analog filter bypass.

    rETIMCFG = (rETIMCFG & 0xFFFFFF0F) | (0X04 << 4); //limit peri_clk_en = 30us
#ifdef TSI_DEMO
    CPM->CPM_PADWKINTCR = 0x000303FF; //wake up -pad/pad ss33/usbdet wake up enable/sddc/cpi_atimer/pci_det wake up disbale
#else
    CPM->CPM_PADWKINTCR = 0x001313FF;        //wake up -pad/pad ss33/usbdet wake up enable/sddc/cpi_atimer/pci_det wake up disbale
#endif

    CPM->CPM_VCCGTRIMR |= (1 << 12) | (1 << 13); //auto latch & auto latch clear.

#if USI1_SB_IT
    *(volatile UINT32 *)0x40009004 &= ~0x01; //USI1 USI_SB_IT_DIS
#else
    *(volatile UINT32 *)0x40015004 &= ~0x01; //USI2 USI_SB_IT_DIS
#endif

    NVIC_Init(3, 3, CPM_IRQn, 2);

    Sys_Standby();
}

/******************************************************************************
* Function Name  : cpm_set_sysclk_div
* Description    : 设置OSC120M分频作为系统时钟
* Input          :  div： the divide value of OSC120M_CLK
*
* Output         : None
* Return         : None
******************************************************************************/
static void cpm_set_sysclk_div(UINT32 div)
{
    CPM->CPM_SCDIVR &= ~(SYS_DIV_MASK);
    CPM->CPM_SCDIVR |= (div << SYS_DIV_SHIFT);
    CPM->CPM_CDIVUPDR |= SYSDIV_UPD;
}

/*******************************************************************************
* Function Name  : CPM_Write_CoreTestKey
* Description    : 顺序写入core test key
* Input          : KeyOpt
*                      0:清除core test key
*                      !0:设置core test key
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_Write_CoreTestKey(UINT8 KeyOpt)
{
    unsigned int tmp = (CPM->CPM_VCCCTMR & 0x3FFFFFFF);

    if (KeyOpt)
    {
        CPM->CPM_VCCCTMR = tmp;
        CPM->CPM_VCCCTMR = (tmp | 0X40000000);
        CPM->CPM_VCCCTMR = (tmp | 0X80000000);
        CPM->CPM_VCCCTMR = (tmp | 0XC0000000);
    }
    else
    {
        CPM->CPM_VCCCTMR = tmp;
    }
}

/*******************************************************************************
* Function Name  : CPM_VCC5V_Bypass
* Description    : 关闭VCC5V LDO
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_VCC5V_Bypass(void)
{
    CPM_Write_CoreTestKey(1);

    CPM->CPM_VCCCTMR |= (1 << 23);

    CPM->CPM_VCCGTRIMR &= ~((UINT32)1 << 31); //V33 switch disable

    CPM_Write_CoreTestKey(0);
}

/*******************************************************************************
* Function Name  : CPM_Vref_Trim
* Description    : trim参考电压
* Input          : trim_value：b'00 is 1.05V; b'01 is 1.1V; b'10 is 1.15V; b'11 is 1.21V。
*
* Output         : None
* Return         : None
******************************************************************************/
void CPM_Vref_Trim(unsigned int trim_value)
{
    CPM_Write_CoreTestKey(1);

    CPM->CPM_VCCCTMR |= (1 << 23);

    if (CPM_VREF_TRIM_090 == trim_value)
    {
        CPM->CPM_VCCGTRIMR = (CPM->CPM_VCCGTRIMR) | (1 << 15);
    }
    else
    {
        CPM->CPM_VCCGTRIMR = (CPM->CPM_VCCGTRIMR) & ~(1 << 15);
        CPM->CPM_VCCGTRIMR = (CPM->CPM_VCCGTRIMR & (~0x0f)) | (trim_value & 0x0f);
    }

    CPM_Write_CoreTestKey(0);
}

/*******************************************************************************
* Function Name  : CPM_SysClkSelect
* Description    : 设置sys_sel分频作为系统时钟
* Input          :  - sys_sel： 系统主时钟源SYSCLK_SEL_OSC8M, SYSCLK_SEL_OSC160, SYSCLK_SEL_USBPHY or SYSCLK_SEL_OSCEXT
*                   - div：时钟预分频(用户真正希望的分频)
*
* Output         :  - None
* Return         :  - None
******************************************************************************/
void CPM_SysClkSelect(UINT32 sys_sel, UINT32 div)
{
    EFLASH_Set_RWSC(0x07); //配置eflash rwsc 为默认值，防止频率切换时卡死

    switch (sys_sel)
    {
    case SYSCLK_OSC8M:
        CPM->CPM_OCSR |= OSC8M_EN;
        while (OSC8M_STABLE != (CPM->CPM_OCSR & OSC8M_STABLE))
            ;

        CPM->CPM_CSWCFGR &= ~(SYSCLK_SEL_MASK);
        CPM->CPM_CSWCFGR |= SYSCLK_SEL_OSC8M;
        while (CLKSEL_ST_OSC8M != (CPM->CPM_CSWCFGR & CLKSEL_ST_OSC8M))
            ;
        break;

    case SYSCLK_OSC160M:
        CPM->CPM_OCSR |= OSC160M_EN;
        while (OSC160M_STABLE != (CPM->CPM_OCSR & OSC160M_STABLE))
            ;

        CPM->CPM_CSWCFGR &= ~(SYSCLK_SEL_MASK);
        CPM->CPM_CSWCFGR |= SYSCLK_SEL_OSC160M;
        while (CLKSEL_ST_OSC160M != (CPM->CPM_CSWCFGR & CLKSEL_ST_OSC160M))
            ;
        break;

    case SYSCLK_USBPHY240M:
        CPM->CPM_OCSR |= USB_PHY240M_EN;
        while (USB_PHY240M_STABLE != (CPM->CPM_OCSR & USB_PHY240M_STABLE))
            ;

        CPM->CPM_CSWCFGR &= ~(SYSCLK_SEL_MASK);
        CPM->CPM_CSWCFGR |= SYSCLK_SEL_USBPHY240M;
        while (CLKSEL_ST_USBPHY240M != (CPM->CPM_CSWCFGR & CLKSEL_ST_USBPHY240M))
            ;
        break;

    case SYSCLK_OSCEXT:
        CPM->CPM_OCSR |= OSCEXT_EN;
        while (OSCEXT_STABLE != (CPM->CPM_OCSR & OSCEXT_STABLE))
            ;

        CPM->CPM_CSWCFGR &= ~(SYSCLK_SEL_MASK);
        CPM->CPM_CSWCFGR |= SYSCLK_SEL_OSCEXT;
        while (CLKSEL_ST_OSCEXT != (CPM->CPM_CSWCFGR & CLKSEL_ST_OSCEXT))
            ;
        break;

    default:
        break;
    }

    cpm_set_sysclk_div(div);
}

/**
 * @brief close module clock, or open module clock.
 * 
 * @param[in] module @ref e_MODULE_CLOCK
 * @param[in] control FALSE or TRUE
 * @return uint32_t 
 */
uint32_t cpm_controlModuleClock(e_MODULE_CLOCK module, bool control)
{
    const uint8_t offset = (module % 32);
    volatile uint32_t *pu32_reg;

    CPM_Write_CoreTestKey(1);
    CPM->CPM_VCCCTMR |= (1u << 13);

    if (module < 32)
        pu32_reg = &(CPM->CPM_MULTICGTCR);
    else if (module < 64)
        pu32_reg = &(CPM->CPM_SYSCGTCR);
    else if (module < 96)
        pu32_reg = &(CPM->CPM_AHB3CGTCR);
    else if (module < 128)
        pu32_reg = &(CPM->CPM_ARITHCGTCR);
    else if (module < 160)
        pu32_reg = &(CPM->CPM_IPSCGTCR);
    else
        return STATUS_ADDR_ERR;

    if (control == FALSE)
    {
        *pu32_reg &= ~(1u << offset);
    }
    else if (control == TRUE)
    {
        *pu32_reg |= (1u << offset);
    }
    else
    {
        return STATUS_ADDR_ERR;
    }

    CPM_Write_CoreTestKey(0);
    return STATUS_OK;
}

/*******************************************************************************
* Function Name  : get_usb_det_sta
* Description    : 获取USB_DET脚状态
* Input          : None
*
* Output         : None
* Return         : - TRUE：           检测到有USB插入
*                  - FALSE:    检测无USB插入
******************************************************************************/
BOOL get_usb_det_sta(void)
{
    if (CPM->CPM_PADWKINTCR & 1)
    {
        CPM->CPM_PADWKINTCR |= 1; //wirte 1  to clear USBDET_STAT
        return TRUE;
    }

    return FALSE;
}

/*******************************************************************************
* Function Name  : Clock_Out_Select
* Description    : clock out管脚输出时钟频率的选择
*
 * Input          : clk - 00 SELECT_SYSTEM_CLK，系统时钟
 *                        01 SELECT_ARITH_CLK ，算法时钟
 *                        10 SELECT_TC2       ，TC2时钟
 *                        11 SELECT_RTC32K_CLK，外部RTC32K时钟
* Output         : None
* Return         : None
******************************************************************************/
void Clock_Out_Select(unsigned char clk)
{
    if(clk>=4){
        return;
    }
    else
    {
        CPM->CPM_CSWCFGR &= ~(CLKOUT_SEL_MASK);
        CPM->CPM_CSWCFGR |= (clk << CLKOUT_SEL_SHIFT);		
    }
}

/*******************************************************************************
* Function Name  : Get_Sys_Clock
* Description    : 返回当前系统频率，单位Hz
*
* Input          : 无
* Output         : 系统频率
* Return         : None
******************************************************************************/
UINT32 Get_Sys_Clock(void)
{
    UINT8 clk_src;
    UINT32 clk_freq;

    clk_src = CPM->CPM_CSWCFGR & SYSCLK_SEL_MASK;

    switch ((clk_src & 0x3))
    {
    case SYSCLK_OSC8M:
        clk_freq = 8 * 1000 * 1000;
        break;
    case SYSCLK_OSC160M:
        if (g_trim_clk == OSC_108M_HZ)
        {
            clk_freq = OSC108M;
        }
        else if (g_trim_clk == OSC_120M_HZ)
        {
            clk_freq = OSC120M;
        }
        else if (g_trim_clk == OSC_150M_HZ)
        {
            clk_freq = OSC150M;
        }
        else
        {
            clk_freq = OSC160M;
        }
        break;
    case SYSCLK_USBPHY240M:
        clk_freq = 240 * 1000 * 1000;
        break;
    case SYSCLK_OSCEXT:
        clk_freq = 12 * 1000 * 1000;
        break;
    default:
        clk_freq = 0;
        break;
    }

    return ((UINT32)(clk_freq / (((CPM->CPM_SCDIVR & SYS_DIV_MASK) >> SYS_DIV_SHIFT) + 1)));
}

/*******************************************************************************
* Function Name  : ARITH_Clk_Op
* Description    : ARITH模块时钟操作
*
* Input          : -div_op：
*                      DIVIDER_DISABLE：禁止分频
*                      UPDATA_DIVIDER：更新分频系数
*                      GET_NOW_DIVIDER：获取当前分频系数
*                  -op_data:
*                      操作参数
* Output         : 当前分频系数
* Return         : None
******************************************************************************/
UINT32 ARITH_Clk_Op(UINT32 div_op, UINT32 op_data)
{
    UINT32 return_val;

    return_val = 0;

    switch (div_op)
    {
    case DIVIDER_DISABLE:
        CPM->CPM_CDIVENR &= ~(ARITH_DIVEN);
        break;
    case UPDATA_DIVIDER:
        CPM->CPM_CDIVENR |= ARITH_DIVEN;
        CPM->CPM_PCDIVR1 &= ~(ARITH_DIV_MASK);
        CPM->CPM_PCDIVR1 |= (op_data << ARITH_DIV_SHIFT);
        CPM->CPM_CDIVUPDR |= PERDIV_UPD | SYSDIV_UPD;
        break;
    case GET_NOW_DIVIDER:
        return_val = ((CPM->CPM_PCDIVR1 & ARITH_DIV_MASK) >> ARITH_DIV_SHIFT);
        break;
    case GET_NOW_CLKGATE:
        return_val = (CPM->CPM_ARITHCGTCR);
        break;
    case CLKGATE_RESTORE:
        CPM->CPM_ARITHCGTCR = op_data;
        break;
    default:
        break;
    }

    return return_val;
}

/*******************************************************************************
* Function Name  : IPS_Clk_Op
* Description    : IPS模块时钟操作
*
* Input          : -div_op：
*                      DIVIDER_DISABLE：禁止分频
*                      UPDATA_DIVIDER：更新分频系数
*                      GET_NOW_DIVIDER：获取当前分频系数
*                  -op_data:
*                      操作参数
* Output         : 当前分频系数
* Return         : None
******************************************************************************/
UINT32 IPS_Clk_Op(UINT32 div_op, UINT32 op_data)
{
    UINT32 return_val;

    return_val = 0;

    if (op_data == 0)
    {
        op_data = 1;
    }

    switch (div_op)
    {
    case DIVIDER_DISABLE:
        CPM->CPM_CDIVENR &= ~(IPS_DIVEN);
        break;
    case UPDATA_DIVIDER:
        CPM->CPM_CDIVENR |= IPS_DIVEN;
        CPM->CPM_PCDIVR1 &= ~(IPS_DIV_MASK);
        CPM->CPM_PCDIVR1 |= (op_data << IPS_DIV_SHIFT);
        CPM->CPM_CDIVUPDR = PERDIV_UPD;
        break;
    case GET_NOW_DIVIDER:
        return_val = ((CPM->CPM_PCDIVR1 & IPS_DIV_MASK) >> IPS_DIV_SHIFT);
        break;
    case GET_NOW_CLKGATE:
        return_val = (CPM->CPM_IPSCGTCR);
        break;
    case CLKGATE_RESTORE:
        CPM->CPM_IPSCGTCR = op_data;
        break;
    default:
        break;
    }

    return return_val;
}

/*******************************************************************************
* Function Name  : SDRAM_Clk_Op
* Description    : SDRAM模块时钟操作
*
* Input          : -div_op：
*                      DIVIDER_DISABLE：禁止分频
*                      UPDATA_DIVIDER：更新分频系数
*                      GET_NOW_DIVIDER：获取当前分频系数
*                  -op_data:
*                      操作参数
* Output         : 当前分频系数
* Return         : None
******************************************************************************/
UINT32 SDRAM_Clk_Op(UINT32 div_op, UINT32 op_data)
{
    UINT32 return_val;

    return_val = 0;

    switch (div_op)
    {
    case DIVIDER_DISABLE:
        CPM->CPM_CDIVENR &= ~(SDRAM_DIVEN);
        break;
    case UPDATA_DIVIDER:
        CPM->CPM_CDIVENR |= SDRAM_DIVEN;
        CPM->CPM_PCDIVR1 &= ~(SDRAM_DIV_MASK);
        CPM->CPM_PCDIVR1 |= (op_data << SDRAM_DIV_SHIFT);
        CPM->CPM_CDIVUPDR = PERDIV_UPD;
        break;
    case GET_NOW_DIVIDER:
        return_val = ((CPM->CPM_PCDIVR1 & SDRAM_DIV_MASK) >> SDRAM_DIV_SHIFT);
        break;
    default:
        break;
    }

    return return_val;
}

/*******************************************************************************
* Function Name  : USBC_PHY_Init
* Description    : 初始化USBC PHY时钟
*
* Input          : -OSC_Type:
*                      0:internal oscillator
*                      1:external oscillator
* Output         : None
* Return         : None
******************************************************************************/
void USBC_PHY_Init(UINT8 OSC_Type)
{
    if (1 == OSC_Type)
    {
        //Config 12MHz Clk
        CPM->CPM_OSCESTIMER = 0x3000;
        CPM->CPM_OCSR |= OSCEXT_EN; //Enable 12MHz Clock
    }

    //Enable PHY Power Switch
    CPM->CPM_CHIPCFGR |= PWRCR_PHY_PSWEN_BIT; //enable usbphy power switch

    //delay at least 10ms, sys clk is 120MHz, in release obj, one clock_cycle is 6 clock
    //so delay(x) = (6 * x / 120)us = 0.05x us, we use 11ms here.so 11ms = (6 * x / 120)=>x=220000
    DelayMS(11);

    //Enable PHY Regulator
    CCM->PHYPA &= ~0xFF; //Controlled by USBC instead of by manual

    //delay at least 10us
    delay(300);
    CPM->CPM_CHIPCFGR &= ~PWRCR_PHY_I_ISOEN_BIT; //disable input usbphy isolation
    CPM->CPM_CHIPCFGR &= ~PWRCR_PHY_O_ISOEN_BIT; //disable output usbphy isolation
    if (1 == OSC_Type)
    {
        while ((CPM->CPM_OCSR & OSCEXT_STABLE) != OSCEXT_STABLE)
            ; //wait for ext 12mhz to be stable
    }
    DelayMS(10); //delay at least 10ms
    CPM->CPM_CHIPCFGR &= ~USBPHY_CFG_SRM;
    DelayMS(1); //delay at least 1ms
    CPM->CPM_CHIPCFGR &= ~USBPHY_PLL_SRM;
    DelayMS(1);                                  //delay at least 1ms
    CPM->CPM_CHIPCFGR &= ~PWRCR_PHY_RSTMASK_BIT; //diable usbphy reset mask, release the reset signal
    DelayMS(1);                                  //delay at least 1ms
}

/*******************************************************************************
* Function Name  : Sys_Clk_Init
* Description    : 初始化系统时钟
*
* Input          : -sys_clk_trim:
*                      0:OSC_108M_HZ,
*       1:OSC_120M_HZ ,
*       2:OSC_150M_HZ,
*       3:OSC_160M_HZ
*                : -sys_clk_div
*                      0:SYS_CLK_DIV_1,
*       1:SYS_CLK_DIV_2,
*       ... ...
*                : -ips_clk_div
*                      1:IPS_CLK_DIV_2,
*       2:SYS_CLK_DIV_3,
*       ... ...
* Output         : None
* Return         : None
******************************************************************************/
void Sys_Clk_Init(int sys_clk_trim, SYS_CLK_DIV sys_clk_div, IPS_CLK_DIV ips_clk_div)
{
    g_trim_clk = sys_clk_trim;

#ifdef LOWPOWER_CONFIG
    CPM_overwrite_test_mode(0x20002a00);
#endif

    CPM_OSC_Switch(sys_clk_trim); //trim 160M时钟源

    if ((CPM->CPM_VCCGTRIMR & 0x03) != CPM_VREF_TRIM_110)
    {
        CPM_Vref_Trim(CPM_VREF_TRIM_110);
    }

    if ((sys_clk_trim == OSC_160M_HZ) && (sys_clk_div == SYS_CLK_DIV_1))
    {
        sys_clk_div = SYS_CLK_DIV_2;
    }

    if ((sys_clk_trim == OSC_150M_HZ) && (sys_clk_div == SYS_CLK_DIV_1))
    {
        CPM_Vref_Trim(CPM_VREF_TRIM_121);
    }

    //配置系统时钟
    CPM_SysClkSelect(SYSCLK_OSC160M, sys_clk_div);

    //配置IPS时钟的分频系数不可以为0
    IPS_Clk_Op(UPDATA_DIVIDER, ips_clk_div);

    // get system clock
    g_sys_clk = Get_Sys_Clock();

    //get ips clock
    g_ips_clk = g_sys_clk / (IPS_Clk_Op(GET_NOW_DIVIDER, 0) + 1);

    if (g_sys_clk <= 60000000) //系统时钟60M以下，电压trim到0.9V，可以降低功耗
    {
        CPM_Vref_Trim(CPM_VREF_TRIM_090);
    }
}

/**
 * @brief 清除默认中断唤醒源
 * @param[in] none
 * @return none
 */
void CPM_ClearPADWKINTCR(void)
{
    CPM->CPM_PADWKINTCR &= ~(0xffu << 16);
    CPM->CPM_PADWKINTCR &= ~(0xffu << 8);
} 

/**
 * @brief 配置SS3输入输出功能, 在睡眠模式下。
 * 1. cpm_configSS3_Sleep(ENABLE, DISABLE, GPIO_OUTPUT, Bit_SET, DISABLE); // enable control at cpm
 * 2. cpm_configSS3_Sleep(DISABLE, DISABLE, GPIO_OUTPUT, Bit_SET, DISABLE); // disable control at cpm
 * @param control_ss3_cpm 
 * @param ss3_wakeup 
 * @param GPIO_Dir 
 * @param bitVal 
 * @param pull 
 */
void cpm_configSS3_Sleep(FunctionalState control_ss3_cpm, FunctionalState ss3_wakeup, uint8_t GPIO_Dir, UINT8 bitVal, FunctionalState pull)
{
    if (control_ss3_cpm == ENABLE)
    {
        CPM->CPM_PADSS3CR |= (0x01u << 7);

        if (ss3_wakeup == ENABLE)
        {
            CPM->CPM_PADSS3CR |= (0x01u << 3);
        }
        else
        {
            CPM->CPM_PADSS3CR &= ~(0x01u << 3);
        }

        if (pull == ENABLE)
        {
            CPM->CPM_PADSS3CR |= 0x01;
        }
        else
        {
            CPM->CPM_PADSS3CR &= ~0x01;
        }

        if (GPIO_Dir == GPIO_OUTPUT)
        {
            CPM->CPM_PADSS3CR |= (0x01u << 1);
        }
        else
        {
            CPM->CPM_PADSS3CR &= ~(0x01u << 1);
        }

        if (bitVal == Bit_SET)
        {
            CPM->CPM_PADSS3CR |= (0x01u << 2);
        }
        else
        {
            CPM->CPM_PADSS3CR &= ~(0x01u << 2);
        }
    }
    else
    {
        CPM->CPM_PADSS3CR &= ~(0x01u << 7);
    }
}

/**
 * @brief sleep nms
 * @param[in] 
 * @return none 
 */
void TC_Sleep_DelayMS(uint16_t ms)
{
    volatile uint32_t reg_back1 = CPM->CPM_PADWKINTCR & ~0x000000ff;
    volatile uint32_t reg_back2 = CPM->CPM_WKPSCR;

    CPM->CPM_PADWKINTCR = 0x00000000;
    CPM->CPM_WKPSCR = 0x00000000;

    do
    {
        TC->TCCR |= TC_IF;
    } while (TC->TCCR & TC_IS);

    g_tc_int_flag = 0;
    TC_IntInit(TC,TC_WDP_0_5, ms * 2 - 1, TRUE);
    CPM_Sleep();
    TC_Close(TC);

    if (!g_tc_int_flag)
    {
        delay(120000 * ms); // wait ms, if TC failed
        g_tc_int_flag = 0;
    }
    else
    {
        g_tc_int_flag = 0;
    }

    CPM->CPM_PADWKINTCR = reg_back1;
    CPM->CPM_WKPSCR = reg_back2;
}

/**
 * @brief 判断IRQ使能是否触发(解决中断丢失问题)
 * 
 * @param EPORT_addr EPORT组(EPORT0~EPORT4)
 * @param port EPORT PIN口(0~7)
 * @return uint8_t 
 * @pre 1. 设置gint触发模式为上升沿触发或者下降沿触发.
 */
uint8_t cpm_getIRQ_Eport(EPORT_TypeDef *EPORT_addr, uint8_t port)
{
    uint8_t EPPA = 0;
    uint8_t enable_level = 0; //0-low 1-high
    uint8_t current_level = 0;

    if (port > 7 || EPORT_addr == NULL || (EPORT_addr->EPDDR & (0x01 << port)))
    {
        return 0;
    }

    if (EPORT_addr->EPIER & (0x01 << port))
    {
        EPPA = (EPORT_addr->EPPAR >> (port * 2)) & 0x03;

        if (EPPA == 0x00) //level-sensitive
        {
            enable_level = (EPORT_addr->EPLPR & (0x01 << port)) ? 1 : 0;
        }
        else if (EPPA == 0x01)
        {
            enable_level = 1;
        }
        else if (EPPA == 0x02)
        {
            enable_level = 0;
        }
        else
        {
            return 0;
        }

        current_level = (EPORT_addr->EPPDR & (0x01 << port)) ? 1 : 0;

        if (current_level == enable_level)
        {
            return 1;
        }
    }
    return 0;
}

#ifndef LOWPOWER_CONFIG

/**
 * @brief 唤醒源区分 & 唤醒预处理.
 * 1. 在cpm_drv.h中打开对应宏, 去控制该唤醒源判断使能关闭.
 * 2. CPM_WAKEUPSOURCE_REG: 低8bit用与存放唤醒源(RST_SOURCE_STA);
 *                          高24bit存放唤醒引脚临时锁存状态(CPM_Wakeup_Pin)
 * 3. 请保证该函数在 __main 之前被调用.
 */
void cpm_handleWakeup(void)
{
//#ifdef MY_DEBUG
//		extern void IO_Latch_Clr(void);
//		IO_Latch_Clr();
//    UART_Debug_Init(SCI1, 60000000, 9600);
//    printf("IRQ:%02x, 2:%d, 3:%d\r\n", EPORT->EPFR, cpm_getIRQ_Eport(EPORT, 2), cpm_getIRQ_Eport(EPORT, 3));
//#endif
		uint8_t count = 0;
    const volatile uint32_t CPM_PADWKINTCR = CPM->CPM_PADWKINTCR;
    volatile uint32_t *pu32_wakeupSource = (uint32_t *)CPM_WAKEUPSOURCE_REG;
    *pu32_wakeupSource &= 0xFFFFFF00;
    *pu32_wakeupSource |= (WAKEUP_SOURCE_NONE);

#if CPM_HANDLEWAKEUP_WDT
    if (CHIP_RESET->RSR & (0x01 << 4))
    {
#if CPM_HANDLEWAKEUP_WDT_LOG
        PCI_InitWakeup(); // PCI reinit.
        TC_selectClockSource(1);
        PCI_Clear_Status();
        Nvram_Init();

        PCI_NVSRAM_WtiteByte(0x4003416C, 0x55);
        PCI_NVSRAM_WtiteByte(0x4003416D, 0xaa);
        PCI_NVSRAM_WtiteByte(0x4003416E, 0x55);
        PCI_NVSRAM_WtiteByte(0x4003416F, 0xaa);
        TC_Sleep_DelayMS(10);
#endif
        Set_POR_Reset();
        return; //wdt reset
    }
#endif

    if ((CPM_PADWKINTCR & 0x00ffff00) == 0x00ff0000) // POR
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_POR;
    }
#if CPM_HANDLEWAKEUP_GINT0
    else if ((EPORT->EPFR & (1u << 0)) || cpm_getIRQ_Eport(EPORT, 0))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_GIN0;
    }
#endif
#if CPM_HANDLEWAKEUP_GINT1
    else if ((EPORT->EPFR & (1u << 1)) || cpm_getIRQ_Eport(EPORT, 1))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_GIN1;
    }
#endif
#if CPM_HANDLEWAKEUP_GINT2
    else if ((EPORT->EPFR & (1u << 2)) || cpm_getIRQ_Eport(EPORT, 2))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_GIN2;
    }
#endif
#if CPM_HANDLEWAKEUP_GINT3
    else if ((EPORT->EPFR & (1u << 3)) || cpm_getIRQ_Eport(EPORT, 3))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_GIN3;
    }
#endif
#if CPM_HANDLEWAKEUP_GINT4
    else if ((EPORT->EPFR & (1u << 4)) || cpm_getIRQ_Eport(EPORT, 4))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_GIN4;
    }
#endif
#if CPM_HANDLEWAKEUP_GINT5
    else if ((EPORT->EPFR & (1u << 5)) || cpm_getIRQ_Eport(EPORT, 5))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_GIN5;
    }
#endif
#if (CPM_HANDLEWAKEUP_C0GINT0 || \
     CPM_HANDLEWAKEUP_C0GINT1 || \
     CPM_HANDLEWAKEUP_C0GINT4 || \
     CPM_HANDLEWAKEUP_C0GINT5)
    else if (drv_SubC0_getWakeupSource(pu32_wakeupSource) == 0)
    {
        // goto main
    }
#endif
#if CPM_HANDLEWAKEUP_SS3
    else if (CPM_PADWKINTCR & (1u << 4))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_SS3;
    }
#endif
#if CPM_HANDLEWAKEUP_WAKEUP
    else if (CPM_PADWKINTCR & (1u << 1))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_WAKEUP;
    }
#endif
#if CPM_HANDLEWAKEUP_USBDET
    else if (CPM_PADWKINTCR & (1u << 0))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_USBDET;
    }
#endif
#if CPM_HANDLEWAKEUP_SDIO0
    else if (CPM_PADWKINTCR & (1u << 2))
    {
        *pu32_wakeupSource = WAKEUP_SOURCE_SDIO0;
    }
#endif
#if CPM_HANDLEWAKEUP_TSI
    else if ((TSI->TSI_ISR & (1u << 7)) && (TSI->TSI_IER & (1u << 29)))
    {
        TSI_HandleTypeDef htsi;

        DCACHE_Init((cache_com)0, (cache_com)1, (cache_com)0, (cache_com)0);
        ICACHE_Init((cache_com)0, (cache_com)1, (cache_com)0, (cache_com)0);

        hal_tsi_configBackupStruct(&htsi);

        /* clear PCI Asyc & reload PCI Asyc */
        TC_Sleep_DelayMS(2);
        PCI_InitWakeup(); // PCI reinit.
        PCI->PCI_ATIMPR = ((CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE * 2000 << 16) | 0x0103);
				
        // open NVIC for sleep
        NVIC_Init(3, 3, EPORT0_0_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_1_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_2_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_3_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_4_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_5_IRQn, 2);
        // NVIC_Init(3, 3, TSI_IRQn, 2);
        // NVIC_Init(3, 3, CPM_IRQn, 2);

        // volatile uint8_t **ppu8_Trend = &(htsi.Backup.Trend.ch0);
        volatile uint8_t *pu8_data = (uint8_t *)&(TSI->TSI_CH0DR);
        volatile uint8_t **ppu8_AlgBase = &(htsi.Backup.AlgBase.ch0);
        uint8_t delta[16] = {TSI_CH0_THIN_VAL, TSI_CH1_THIN_VAL, TSI_CH2_THIN_VAL, TSI_CH3_THIN_VAL,
                             TSI_CH4_THIN_VAL, TSI_CH5_THIN_VAL, TSI_CH6_THIN_VAL, TSI_CH7_THIN_VAL,
                             TSI_CH8_THIN_VAL, TSI_CH9_THIN_VAL, TSI_CH10_THIN_VAL, TSI_CH11_THIN_VAL,
                             TSI_CH12_THIN_VAL, TSI_CH13_THIN_VAL, TSI_CH14_THIN_VAL, TSI_CH15_THIN_VAL};
        uint8_t touch_cnt;
        uint8_t touch_channle_cnt = 0;

        /* TSI check */
        _SET_SCANALWAYS(TSI); //使能always on模式
        TC_Sleep_DelayMS(4);  // 建议先sleep一段时间, 小于场强周期

        for (uint8_t channle = 0; channle < 16; channle++)
        {
            if (TSI->TSI_CHEN & (0x01u << channle))
            {
                touch_cnt = 0;
                for (uint8_t i = 0; i < 4; i++)
                {
                    if (*pu8_data > (**ppu8_AlgBase + delta[channle] + 0))
                    {
                        touch_cnt++;
                    }
                    pu8_data++;
                }
                if (touch_cnt > 1)
                {
                    touch_channle_cnt++;
                }
            }
            else
            {
                pu8_data = pu8_data + 4;
            }
            ppu8_AlgBase++;
        }

        _SET_SCANCYCLICITY(TSI); //使能周期扫描模式

        if (touch_channle_cnt > 1)
        {
            // just for log
            // volatile uint32_t *pu32_data = &(TSI->TSI_CH0DR);
            // for (uint8_t i = 0; i < 16; i++)
            // {
            //     **ppu8_Trend = ((*pu32_data & 0xff) +
            //                     ((*pu32_data >> 8) & 0xff) +
            //                     ((*pu32_data >> 16) & 0xff) +
            //                     ((*pu32_data >> 24) & 0xff)) >>
            //                    2;
            //     pu32_data++;
            //     ppu8_Trend++;
            // }
            *pu32_wakeupSource = WAKEUP_SOURCE_TSI; // goto main
        }
        else
        {
//            NVIC_Init(3, 3, TSI_IRQn, 2);

//            CPM->CPM_PADWKINTCR |= (1u << 3);
//            NVIC->ICPR[0] = (1 << 14);              // clear cpm wake-up interrupt pending.
						count=0;
						do{
                hal_tsi_clearIFAWD(&htsi);
								if(++count>5)
								{
										Set_POR_Reset();
								}
            }while (TSI->TSI_ISR & (1u << 7));
            NVIC_Init(3, 3, TSI_IRQn, 2);
            _SET_IE_AWD(TSI);
						//asyc
            CPM->CPM_PADWKINTCR |= 0xff;
						count=0;
            while (CPM->CPM_PADWKINTCR & (1u << 3)) // PCI Asyc IRQ
            {
                TC_Sleep_DelayMS(2);

                PCI->PCI_ATIMPR = ((CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE * 2000 << 16) | 0x0103);
                CPM->CPM_PADWKINTCR |= 0xff;
                NVIC->ICPR[0] = (1 << 14); // clear cpm wake-up interrupt pending.
								
								if(++count>5)
                {
                    Set_POR_Reset();
                }
            }
						count=0;
            while (1)
            {
                CPM_PowerOff_1p5();
								if(++count>5)
								{
										Set_POR_Reset();
								}
            }
        }
    }
#endif
#if CPM_HANDLEWAKEUP_ASYCTIMER
    else
    {
        TSI_HandleTypeDef htsi;

        /* clear PCI Asyc & reload PCI Asyc */
        TC_Sleep_DelayMS(2);
        PCI_InitWakeup(); // PCI reinit.
        PCI->PCI_ATIMPR = ((CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE * 2000 << 16) | 0x0103);

        DCACHE_Init((cache_com)0, (cache_com)1, (cache_com)0, (cache_com)0);
        ICACHE_Init((cache_com)0, (cache_com)1, (cache_com)0, (cache_com)0);

        hal_tsi_configBackupStruct(&htsi);
        //        htsi.Fuction.Status = TSI_s;

        // open NVIC for sleep
        NVIC_Init(3, 3, EPORT0_0_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_1_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_2_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_3_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_4_IRQn, 2);
        NVIC_Init(3, 3, EPORT0_5_IRQn, 2);
        // NVIC_Init(3, 3, TSI_IRQn, 2);
        // NVIC_Init(3, 3, CPM_IRQn, 2);
        _SET_IE_AWD(TSI);
        // TSI
        volatile uint32_t *pu32_data = &(TSI->TSI_CH0DR);
        volatile uint8_t **pu8_AlgBase = &(htsi.Backup.AlgBase.ch0);
        volatile uint32_t **ppu8_AlgBase = (volatile uint32_t **)&(htsi.Backup.AlgBase.ch0);
        uint8_t fine_data;
        for (uint8_t channle = 0; channle < 16; channle++)
        {
            if (((channle % 4) == 0) && (channle != 0))
            {
                ppu8_AlgBase = ppu8_AlgBase + 4;
            }

            if (TSI->TSI_CHEN & (0x01u << channle))
            {
                fine_data = ((*pu32_data & 0xff) +
                             ((*pu32_data >> 8) & 0xff) +
                             ((*pu32_data >> 16) & 0xff) +
                             ((*pu32_data >> 24) & 0xff)) >>
                            2;
                if (fine_data > **pu8_AlgBase) // 向上跟随
                {
                    **ppu8_AlgBase |= ((uint8_t)(fine_data) << ((channle % 4) * 8));
                }
                else if(fine_data == **pu8_AlgBase)
                {
                    ;
                }
                else // 向下跟随
                {
                    fine_data = **pu8_AlgBase - ((**pu8_AlgBase - fine_data) >> 1);
                    **ppu8_AlgBase |= ((uint8_t)(fine_data) << ((channle % 4) * 8));
                }
            }
            pu32_data++;
            pu8_AlgBase++;
        }
        NVIC_Init(3, 3, TSI_IRQn, 2);
				//清ASYC标志
        TC_Sleep_DelayMS(2);
				CPM->CPM_PADWKINTCR |= 0xff;
//        CPM->CPM_PADWKINTCR |= (1u << 3);       // 写1清中断  
//        NVIC->ICPR[0] = (1 << 14);              // clear cpm wake-up interrupt pending.清中断挂起位
				count=0;
        while (CPM->CPM_PADWKINTCR & (1u << 3)) // PCI Asyc IRQ，如果没清掉，重试几次复位
        {
            TC_Sleep_DelayMS(2);
            PCI->PCI_ATIMPR = ((CPM_HANDLEWAKEUP_ASYCTIMER_CYCLE * 2000 << 16) | 0x0103);
            CPM->CPM_PADWKINTCR |= 0xff;
            NVIC->ICPR[0] = (1 << 14); // clear cpm wake-up interrupt pending.
					
						if(++count>5)
						{
								Set_POR_Reset();
						}
        }
        //printf("IIIRQ:%02x, 2:%d, 3:%d\r\n", EPORT->EPFR, cpm_getIRQ_Eport(EPORT, 2), cpm_getIRQ_Eport(EPORT, 3));
				count=0;
				while (1)
				{
						CPM_PowerOff_1p5();
						if(++count>5)
						{
								Set_POR_Reset();
						}
				}
    }
#endif
}

#endif

/**
 * @brief USBDET、WAKEUP引脚读电平.
 * 
 * @param GpioNo 
 * @return uint8_t 高电平返回1, 低电平返回0.
 */
uint8_t CPM_ReadGpioData(e_CPM_GPIO GpioNo)
{
    uint32_t offset = 0;
    if (GpioNo == CPM_USBDET_PIN)
    {
        offset = 24;
    }
    else if (GpioNo == CPM_WAKEUP_PIN)
    {
        offset = 25;
    }
    else
    {
        return 0xff;
    }

    if (CPM->CPM_PADWKINTCR & (1u << offset))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
