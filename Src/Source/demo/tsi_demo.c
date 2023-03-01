// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : tsi_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "tsi_app.h"
#include "pci_demo.h"
#include "cache_drv.h"
#include "libTSI.h"

/**
 * @brief  触摸键盘扫描 获取按键值
 * @note
 *
 * @return 返回按键值，无按键动作返回0
 */
uint8_t Touch_GetKeyValue(void)
{
    app_tsi_handle(&htsi);

    if (htsi.Touch.Current < TSI_MULTI_VALID)
    {
        return htsi.Touch.Current;
    }
    else
    {
        return 0xFF;
    }
}

static uint8_t lastKey = 0xff;
static uint16_t longPressCnt = 1;
#define FFPRINTF printf //按键打印
/**
 * @brief  读取按钮动作
 * @note   短按 keyValue = CHx
 * 长按 keyValue = CHx + 0x10
 * 松手 keyValue = CHx + 0x20
 * 无按键 keyValue = 0xFF
 *
 * @return 有动作返回SUCCESS  无动作返回ERROR
 */
uint8_t TouchTask_ReadAction(void)
{
    uint8_t keyValue = 0xff;
    uint8_t keyValue2 = 0xff;

    keyValue = Touch_GetKeyValue();
    if (keyValue != 0xff) //按下了
    {
        if (keyValue != lastKey) //第一次读到按键：短按
        {
            longPressCnt = 1;
            if (lastKey != 0xff)
            {

                return keyValue2; //当上一次的按键没有松开继续按另一个键时，不发送短按消息
            }
            FFPRINTF("短按 %d\n", keyValue);
            keyValue2 = keyValue;
        }
        else if (longPressCnt) //读取到同样的按键值：长按
        {
            DelayMS(1);
            if (++longPressCnt >= 3000) //长按计时：约3s
            {
                FFPRINTF("longPressCnt %d\n", longPressCnt);
                longPressCnt = 0;
                FFPRINTF("长按 %d\n", keyValue);
                keyValue2 = keyValue + 0x10;
            }
        }
    }
    else //松手了
    {
        longPressCnt = 1; //清除长按计数
        if (lastKey != 0xff)
        {
            keyValue2 = keyValue + 0x20;
            FFPRINTF("松手 %d\n", lastKey);
        }
    }
    lastKey = keyValue;
    return keyValue2;
}

// TSI Demo需要PCI供电(纽扣电池供电)，同时打开startup_ARMCM4.s中cpm_handleWakeup
void TSI_Demo(void)
{
    UINT32 i;
    e_CPM_wakeupSource_Status rst_source = *(e_CPM_wakeupSource_Status *)CPM_WAKEUPSOURCE_REG;
    ;
    printf("\r\nReset source is:%d\r\n", rst_source);

    if ((rst_source == WAKEUP_SOURCE_POR) || (rst_source == WAKEUP_SOURCE_NONE))
    {
        Sys_Init();
        PCI_Init(EXTERNAL_CLK_SEL);
        PCI->PCI_CR &= ~(0xffu << 24); // NVSRAM don`t Clear
        TC_selectClockSource(1);

        // EPORT_Init(EPORT_PIN5, FALLING_EDGE_INT);
    }
    else
    {
        // 开启EFLASH加速模块
        DCACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff);
        ICACHE_Init(cacheOff, cacheThrough, cacheOff, cacheOff);
        PCI_InitWakeup(); // PCI reinit.
    }

    app_tsi_init(&htsi);
    printf("TSI Init Done!");
    for (i = 0; i < 10000; i++)
    {
#if 0
        if (0xFF != TouchTask_ReadAction())
        {
            i = 0; //有键按下，清零计数
        }
#else
        DelayMS(1);
        app_tsi_handle(&htsi);
        if (htsi.Touch.Current < TSI_MULTI_VALID) // 否则无按键
        {
            i = 0;
            printf("Touch Key is %d\r\n", htsi.Touch.Current);
        }
#endif
    }
    app_tsi_sleep(&htsi);
    // PCI_AsyncTC_Init(2000);
    //    printf_tsiReg();
    printf("Enter POFF1.5...\r\n");
    CPM_PowerOff_1p5();
}
