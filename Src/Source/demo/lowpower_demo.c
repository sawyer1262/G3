/**
 * @file lowpower_demo.c
 * @author 产品应用部
 * @brief 
 * @version 1.2
 * @date 2021-03-28
 * 1. 注意，测试前要打开eport 0，1，2的中断功能，再额外打开一个中断，用来唤醒芯片的normal sleep（必须配置成为电平触发）
 * @copyright Copyright (c) 2021
 * 
 */

#include "cpm_drv.h"
#include "debug.h"
#include "lowpower_demo.h"
#include "eport_drv.h"
#include "tc_drv.h"
#include "wdt_drv.h"
#include "delay.h"
#include "cache_drv.h"
#include "trng_drv.h"
#include "delay.h"
#include "main.h"

#ifndef LOWPOWER_CONFIG
#include "tsi_app.h"
#include "pci_drv.h"
#include "c0_drv.h"

/**
 * @brief: 模拟IO测试程序，测试正常的唤醒功能
 */
void Lowpower_function_test(void)
{
    uint delayTime = 0;
    //1. 初始化
    Init_Trng();
    // GetRandomWord();

    //2. EPORT输出管脚配置
    EPORT_ConfigGpio(EPORT_PIN2, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN3, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN4, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN5, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN23, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN24, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN25, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN26, GPIO_OUTPUT);  
    EPORT_ConfigGpio(EPORT_PIN27, GPIO_OUTPUT); 
    EPORT_ConfigGpio(EPORT_PIN28, GPIO_OUTPUT); 

    //3.
    EPORT_WriteGpioData(EPORT_PIN2, Bit_SET);
    delayTime = GetRandomWord() / 1000000;
    printf("delayTime: %d\n", delayTime);
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN2, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN3, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 100000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN3, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN4, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 1000000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN4, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN5, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 100000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN5, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN23, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 1000000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN23, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN23, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 100000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN24, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN24, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 1000000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN25, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN26, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 100000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN27, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN27, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 1000000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN28, Bit_RESET);

    EPORT_WriteGpioData(EPORT_PIN28, Bit_SET);
    printf("delayTime: %d\n", delayTime);
    delayTime = GetRandomWord() / 100000;
    DelayMS(delayTime);
    EPORT_WriteGpioData(EPORT_PIN11, Bit_RESET);
}

/**
 * @brief: 唤醒前需要配置相关唤醒内容，展示整个唤醒流程, LP唤醒不需要cpm_handlwakeup
 */
void Lowpower_wakeupSource_config(void)
{
    //系统初始化, 此处仅做流程说明实际可以将时钟初始化放置在外部
    Sys_Init();

    //1. c0唤醒逻辑配置
    drv_SubC0_init(hSubC0); //设置C0唤醒、GINT0/1/4/5电平状态等

    //2. TSI触摸唤醒配置
    app_tsi_init(&htsi);
    for (uint32_t i = 0; i < 2000000; i++)
    {
        app_tsi_handle(&htsi);
        if (htsi.Touch.Current < TSI_MULTI_VALID) // 否则无按键
        {
            printf("Touch Key is %d\r\n", htsi.Touch.Current);
        }
    }

    //3. EPORT唤醒配置
    EPORT_PullConfig(EPORT_PIN2, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN2, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN3, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN3, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN4, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN4, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN5, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN5, FALLING_EDGE_INT);

    //4. 异步定时器唤醒配置
    PCI_Init(EXTERNAL_CLK_SEL);
    PCI->PCI_CR &= ~(0xffu << 24); // NVSRAM don`t Clear
    //进入休眠前需要清理对应的中断位，确保能休眠

    TC_selectClockSource(1); //休眠后选择外部时钟源
    PCI_AsyncTC_Init(2000);  //设置异步定时器时间

    //5. 休眠前配置TSI、C0进入低功耗
    drv_SubC0_sleep();    //配置C0进入低功耗
    app_tsi_sleep(&htsi); //配置TSI进入低功耗
}

/**
 * @brief: 唤醒后需要进行的初始化及相关配置
 */
void Lowpower_wakeup_config(void)
{
    //1. cache init 开启cache加速
    DCACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff);
    ICACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff);

    //2. 唤醒后需要重新初始化TSI
    app_tsi_init(&htsi);
    for (uint32_t i = 0; i < 2000000; i++)
    {
        app_tsi_handle(&htsi);
        if (htsi.Touch.Current < TSI_MULTI_VALID) // 否则无按键
        {
            printf("Touch Key is %d\r\n", htsi.Touch.Current);
        }
    }

    //3. 初始化PCI部分掉电寄存器
    PCI_InitWakeup(); // PCI reinit.
    drv_SubC0_wakeup();
}

/**
 * @brief PLP低功耗休眠示例程序.
 * 1. LP低功耗特性:
 *              CPU SRAM不掉电；SRAM可以保存数据；唤醒之后,软件从断点处开始执行.
 *              M4-42uA（实测）; TSI-2uA; PCI-1.5uA; RTC-2.5uA
 * @pre: 
 * @Question: 
 *        1. 如何判断唤醒源，需要将判断唤醒源的逻辑设计好或者预留给客户
 *        2. 进入LP模式前需要设置好唤醒相关的代码逻辑 
 * 
 */
void Lowpower_lp_demo(void)
{
    while (1)
    {
        Lowpower_wakeupSource_config();
        printf("enter LP,:CPM->CPM_PADWKINTCR = 0x%08x\n", CPM->CPM_PADWKINTCR);
        CPM_Sleep();
        printf("wakeup from LP, CPM->CPM_PADWKINTCR = 0x%08x\n", CPM->CPM_PADWKINTCR);
        Lowpower_wakeup_config();
    }
}

/**
 * @brief 
 * 
 * @return uint32_t 
 */
void Lowpower_goto_POFF1P0(void)
{
    /* 配置AsycTimer定时唤醒 */
    PCI_AsyncTC_Init(2000);

    /* 配置TSI进入低功耗 */
    app_tsi_sleep(&htsi);
    // printf_tsiReg();

    /* 配置SubC0进入低功耗 */
    drv_SubC0_sleep();

    printf("goto POFF1.0... %08x\r\n", CPM->CPM_PADWKINTCR);
    CPM_PowerOff_1();
}

/**
 * @brief 
 * 
 * @return uint32_t 
 */
void Lowpower_reinit_fromPOFF1P0(void)
{
    UART_Debug_Init(SCI1, g_ips_clk, 115200);
    // WDT_Close();
    // TC_Close();

    DCACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff); // 开启EFLASH加速模块
    ICACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff);

    PCI_InitWakeup(); // PCI reinit.
    drv_SubC0_wakeup();
    IO_Latch_Clr();
    printf("\n\n\nwakeup from POFF1.0...\n");
}

/**
 * @brief POFF1.5低功耗休眠示例程序.
 * 1. POFF1.5低功耗特性:
 *              CPU完全掉电；所有SRAM无法保存数据；唤醒之后,软件从Reset_Handler开始执行.
 *              M4-6uA; TSI-2uA; PCI-2uA
 * @pre cpm_handleWakeup() 配合cpm_handleWakeup()区分唤醒源.
 * 
 */
void Lowpower_poff1p0_demo(void)
{
    e_CPM_wakeupSource_Status rst_source = *(e_CPM_wakeupSource_Status *)CPM_WAKEUPSOURCE_REG;
    printf("\r\nReset source is:%d\r\n", rst_source);

    // if ((rst_source == WAKEUP_SOURCE_POR) || (rst_source == WAKEUP_SOURCE_NONE))
    {
        Sys_Init();

        PCI_Init(EXTERNAL_CLK_SEL);
        PCI->PCI_CR &= ~(0xffu << 24); // NVSRAM don`t Clear

        // do
        // {
        //     CPM->CPM_PADWKINTCR |= 0xFF;
        // } while (CPM->CPM_PADWKINTCR & 0xFF);

        TC_selectClockSource(1);

        drv_SubC0_init(hSubC0);
    }

    /* 配置gint0~5中断唤醒 */
    EPORT_PullConfig(EPORT_PIN2, EPORT_PULLUP); // TODO 为什么唤醒之后进入EPORT中断比较慢
    EPORT_Init(EPORT_PIN2, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN3, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN3, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN4, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN4, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN5, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN5, FALLING_EDGE_INT);

    app_tsi_init(&htsi);
    for (uint32_t i = 0; i < 2000000; i++)
    {
        app_tsi_handle(&htsi);
        if (htsi.Touch.Current < TSI_MULTI_VALID) // 否则无按键
        {
            printf("Touch Key is %d\r\n", htsi.Touch.Current);
        }
    }

    while (1)
    {
        DelayMS(1000);
        Lowpower_goto_POFF1P0();

        Lowpower_reinit_fromPOFF1P0();

        /* device功能性reinit */
        app_tsi_init(&htsi);
        for (uint32_t i = 0; i < 2000000; i++)
        {
            app_tsi_handle(&htsi);
            if (htsi.Touch.Current < TSI_MULTI_VALID) // 否则无按键
            {
                printf("Touch Key is %d\r\n", htsi.Touch.Current);
            }
        }
    }
}

/**
 * @brief POFF1.5低功耗休眠示例程序.
 * 1. POFF1.5低功耗特性:
 *              CPU完全掉电；所有SRAM无法保存数据；唤醒之后,软件从Reset_Handler开始执行.
 *              M4-6uA; TSI-2uA; PCI-2uA
 * @pre cpm_handleWakeup() 配合cpm_handleWakeup()区分唤醒源.
 * 
 */
void Lowpower_poff1p5_demo(void)
{
    e_CPM_wakeupSource_Status rst_source = *(e_CPM_wakeupSource_Status *)CPM_WAKEUPSOURCE_REG;
    printf("\r\nReset source is:%d\r\n", rst_source);

    if ((rst_source == WAKEUP_SOURCE_POR) || (rst_source == WAKEUP_SOURCE_NONE))
    {
        Sys_Init();

        PCI_Init(EXTERNAL_CLK_SEL);
        PCI->PCI_CR &= ~(0xffu << 24); // NVSRAM don`t Clear

        // do
        // {
        //     CPM->CPM_PADWKINTCR |= 0xFF;
        // } while (CPM->CPM_PADWKINTCR & 0xFF);

        TC_selectClockSource(1);

        drv_SubC0_init(hSubC0);
    }
    else
    {
        DCACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff); // 开启EFLASH加速模块
        ICACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff);

        PCI_InitWakeup(); // PCI reinit.

        drv_SubC0_wakeup();
    }

    /* 配置gint0~5中断唤醒 */
    EPORT_PullConfig(EPORT_PIN2, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN2, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN3, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN3, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN4, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN4, FALLING_EDGE_INT);
    EPORT_PullConfig(EPORT_PIN5, EPORT_PULLUP);
    EPORT_Init(EPORT_PIN5, FALLING_EDGE_INT);

    /* 配置TSI进入低功耗 */
    app_tsi_init(&htsi);
    for (uint32_t i = 0; i < 2000000; i++)
    {
        app_tsi_handle(&htsi);
        if (htsi.Touch.Current < TSI_MULTI_VALID) // 否则无按键
        {
            printf("Touch Key is %d\r\n", htsi.Touch.Current);
        }
    }
    app_tsi_sleep(&htsi);
    //    printf_tsiReg();

    /* 配置SubC0进入低功耗 */
    drv_SubC0_sleep();

    /* 配置AsycTimer定时唤醒 */
    PCI_AsyncTC_Init(2000);
    while (1)
    {
        printf("Enter POFF1.5... %08x\r\n", CPM->CPM_PADWKINTCR);
        CPM_PowerOff_1p5();
    }
}
#endif

void Lowpower_Demo(void)
{
    DelayMS(2000); //调试时建议加上延时，预防上电进入低功耗无法仿真

    EPORT_Init(EPORT_PIN5, LOW_LEVEL_INT);

    EPORT_ConfigGpio(EPORT_PIN1, GPIO_INPUT);
    EPORT_ConfigGpio(EPORT_PIN2, GPIO_INPUT);
    EPORT_ConfigGpio(EPORT_PIN3, GPIO_INPUT);
    EPORT_ConfigGpio(EPORT_PIN4, GPIO_INPUT);

    while (1)
    {
        //检测USB是否有插入
        if (get_usb_det_sta() == TRUE)
        {
            printf("USB DET STA IS H\r\n");
        }

        //eport 1 - normal sleep
        if (EPORT_ReadGpioData(EPORT_PIN1) == 0)
        {
            DelayMS(50);
            if (EPORT_ReadGpioData(EPORT_PIN1) == 0)
            {

                printf("Enter sleep...\r\n");
                CPM_Sleep();
                printf("wake up from sleep.\r\n");
            }
        }

        //eport 2 - poweroff 1
        if (EPORT_ReadGpioData(EPORT_PIN2) == 0)
        {
            DelayMS(50);
            if (EPORT_ReadGpioData(EPORT_PIN2) == 0)
            {

                printf("enter power off 1...\r\n");
                CPM_PowerOff_1();                         //POFF1，将栈和上电需要初始化的全局变量放到DRAM中，即0x1FFF8000~0x1FFFFFFF
                WDT_Close();                              //关闭CLK
                TC_Close(TC);                               //关闭TC
                UART_Debug_Init(SCI1, g_ips_clk, 115200); //初始化串口
                printf("wake up from sleep.\r\n");
            }
        }

        //eport 3 - poweroff 1.5
        if (EPORT_ReadGpioData(EPORT_PIN3) == 0)
        {
            DelayMS(50);
            if (EPORT_ReadGpioData(EPORT_PIN3) == 0)
            {

                printf("enter power off 1.5...\r\n");
                CPM_PowerOff_1p5();
            }
        }

        //eport 4 - poweroff 2
        if (EPORT_ReadGpioData(EPORT_PIN4) == 0)
        {
            DelayMS(50);
            if (EPORT_ReadGpioData(EPORT_PIN4) == 0)
            {
                printf("\n\tenter power off 2...\n");
                CPM_PowerOff_2();
            }
        }
    }
}
