// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : eport_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "debug.h"
//#include "iccemv.h"
#include "eport_drv.h"
#include "ccm_drv.h"
#include "ioctrl_drv.h"

unsigned char eport_isr_mark = 0;

__weak void EPORT_GINT0_callback(void)
{
    // printf("gint0 IRQ\n");
}

__weak void EPORT_GINT1_callback(void)
{
    // printf("gint1 IRQ\n");
}

__weak void EPORT_GINT2_callback(void)
{
    // printf("gint2 IRQ\n");
}

__weak void EPORT_GINT3_callback(void)
{
    // printf("gint3 IRQ\n");
}

__weak void EPORT_GINT4_callback(void)
{
    // printf("gint4 IRQ\n");
}

__weak void EPORT_GINT5_callback(void)
{
    // printf("gint5 IRQ\n");
}
/*******************************************************************************
* Function Name  : EPORT0_x_IRQHandler
* Description    : EPORT0_x中断处理
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
static void EPORT_IRQHandler(EPORT_TypeDef *EPORTx, EPORT_PINx PINx)
{
    UINT8 status;
    status = EPORTx->EPFR;

    printf("enter eport interrupt.current status = %02x\r\n", status);

    if (status & (0x01 << PINx))
    {
        EPORTx->EPFR |= 0x01 << PINx; //clear flag by writing 1 to it.
        if (EPORTx == EPORT)
        {
            printf("Enter EPORT%d interrupt.PIN = %d\r\n", 0, PINx);
        }
        if (EPORTx == EPORT1)
        {
            printf("Enter EPORT%d interrupt.PIN = %d\r\n", 1, PINx);
        }
        if (EPORTx == EPORT2)
        {
            printf("Enter EPORT%d interrupt.PIN = %d\r\n", 2, PINx);
        }
        if (EPORTx == EPORT3)
        {
            printf("Enter EPORT%d interrupt.PIN = %d\r\n", 3, PINx);
        }
        if (EPORTx == EPORT4)
        {
            printf("Enter EPORT%d interrupt.PIN = %d\r\n", 4, PINx);
        }
    }
}

void EPORT0_0_IRQHandler(void)
{
    const uint8_t offset = 0;

    if (EPORT->EPFR & (1u << offset)) // gint0
    {
        EPORT->EPFR |= (1u << offset); //clear flag by writing 1 to it.
        EPORT_GINT0_callback();
    }
    else
    {
        EPORT_IRQHandler(EPORT2, EPORT_PIN0);
    }
}

void EPORT0_1_IRQHandler(void)
{
    const uint8_t offset = 1;

    if (EPORT->EPFR & (1u << offset)) // gint1
    {
        EPORT->EPFR |= (1u << offset); //clear flag by writing 1 to it.
        EPORT_GINT1_callback();
    }
    else
    {
        EPORT_IRQHandler(EPORT2, EPORT_PIN1);
    }
}

void EPORT0_2_IRQHandler(void)
{
    const uint8_t offset = 2;

    if (EPORT->EPFR & (1u << offset)) // gint2
    {
        EPORT->EPFR |= (1u << offset); //clear flag by writing 1 to it.
        EPORT_GINT2_callback();
    }
    else
    {
        EPORT_IRQHandler(EPORT2, EPORT_PIN2);
    }
}

void EPORT0_3_IRQHandler(void)
{
    const uint8_t offset = 3;
    if (EPORT->EPFR & (1u << offset)) // gint3
    {
        EPORT->EPFR |= (1u << offset); //clear flag by writing 1 to it.
        EPORT_GINT3_callback();
    }
    else
    {
        EPORT_IRQHandler(EPORT2, EPORT_PIN3);
    }
}

void EPORT0_4_IRQHandler(void)
{
    const uint8_t offset = 4;
    if (EPORT->EPFR & (1u << offset)) // gint4
    {
        EPORT->EPFR |= (1u << offset); //clear flag by writing 1 to it.
        EPORT_GINT4_callback();
    }
    else
    {
        EPORT_IRQHandler(EPORT2, EPORT_PIN4);
    }
}

void EPORT0_5_IRQHandler(void)
{
    const uint8_t offset = 5;
    if (EPORT->EPFR & (1u << offset)) // gint5
    {
        EPORT->EPFR |= (1u << offset); //clear flag by writing 1 to it.
        EPORT_GINT5_callback();
    }
    else
    {
        EPORT_IRQHandler(EPORT2, EPORT_PIN5);
    }
}

void EPORT0_6_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT, EPORT_PIN6);
    EPORT_IRQHandler(EPORT2, EPORT_PIN6);
}

void EPORT0_7_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT, EPORT_PIN7);
    EPORT_IRQHandler(EPORT2, EPORT_PIN7);
}

void EPORT1_0_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN0);
    EPORT_IRQHandler(EPORT3, EPORT_PIN0);
    EPORT_IRQHandler(EPORT4, EPORT_PIN0);
}

void EPORT1_1_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN1);
    EPORT_IRQHandler(EPORT3, EPORT_PIN1);
    EPORT_IRQHandler(EPORT4, EPORT_PIN1);
}

void EPORT1_2_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN2);
    EPORT_IRQHandler(EPORT3, EPORT_PIN2);
    EPORT_IRQHandler(EPORT4, EPORT_PIN2);
}

void EPORT1_3_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN3);
    EPORT_IRQHandler(EPORT3, EPORT_PIN3);
    EPORT_IRQHandler(EPORT4, EPORT_PIN3);
}

void EPORT1_4_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN4);
    EPORT_IRQHandler(EPORT3, EPORT_PIN4);
    EPORT_IRQHandler(EPORT4, EPORT_PIN4);
}

void EPORT1_5_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN5);
    EPORT_IRQHandler(EPORT3, EPORT_PIN5);
    EPORT_IRQHandler(EPORT4, EPORT_PIN5);
}

void EPORT1_6_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN6);
    EPORT_IRQHandler(EPORT3, EPORT_PIN6);
    EPORT_IRQHandler(EPORT4, EPORT_PIN6);
}

void EPORT1_7_IRQHandler(void)
{
    EPORT_IRQHandler(EPORT1, EPORT_PIN7);
    EPORT_IRQHandler(EPORT3, EPORT_PIN7);
    EPORT_IRQHandler(EPORT4, EPORT_PIN7);
}

/*******************************************************************************
* Function Name  : EPORT_ConfigGpio
* Description    : EPORT配置成GPIO用途
* Input          : - EPORT_PINx: EPORT Pin；where x can be 0~15 to select the EPORT peripheral.
*                  - GpioDir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
INT8 EPORT_ConfigGpio(EPORT_PINx GpioNo, UINT8 GpioDir)
{
    //UINT32 temp = 0x01;
    assert_param(IS_EPORT_PINx(GpioNo));
    assert_param(IS_GPIO_DIR_BIT(GpioDir));

    if (GpioNo > EPORT_PIN39)
    {
        return -1;
    }
		 if (GpioNo == EPORT_PIN8) 
    {
      *(volatile unsigned int *)(0x40001014) &= ~(1<<13);
    }

    if ((GpioNo >= EPORT_PIN9) && (GpioNo <= EPORT_PIN11))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 0);
    }

    if ((GpioNo >= EPORT_PIN14) && (GpioNo <= EPORT_PIN15))
    {
        *(volatile unsigned int *)(0x4000001c) &= ~(1 << 1);
    }

    if ((GpioNo >= EPORT_PIN38) && (GpioNo <= EPORT_PIN39))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 2);
    }

    if ((GpioNo >= EPORT_PIN6) && (GpioNo <= EPORT_PIN7))
    {
        *(volatile unsigned int *)(0x4000001c) &= ~(1 << 4);
    }

    if ((GpioNo >= EPORT_PIN32) && (GpioNo <= EPORT_PIN37))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 7);
    }

    if ((GpioNo >= EPORT_PIN26) && (GpioNo <= EPORT_PIN31))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 12);
    }

    if ((GpioNo >= EPORT_PIN22) && (GpioNo <= EPORT_PIN25))
    {
        *(volatile unsigned int *)(0x40000044) &= ~((UINT32)0xf << 28);
        *(volatile unsigned int *)(0x4000001c) |= (1 << 13);
    }

    if ((GpioNo >= EPORT_PIN18) && (GpioNo <= EPORT_PIN21))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 14);
    }

    if ((GpioNo >= EPORT_PIN16) && (GpioNo <= EPORT_PIN17))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 15);
    }

    if (GpioDir == GPIO_INPUT) //EPORT_EPDDR
    {
        if ((GpioNo >> 3) == 0)
        {
            EPORT->EPDDR &= ~(1 << (GpioNo - EPORT_PIN0));
        }
        else if ((GpioNo >> 3) == 1)
        {
            EPORT1->EPDDR &= ~(1 << (GpioNo - EPORT_PIN8));
        }
        else if ((GpioNo >> 3) == 2)
        {
            EPORT2->EPDDR &= ~(1 << (GpioNo - EPORT_PIN16));
        }
        else if ((GpioNo >> 3) == 3)
        {
            EPORT3->EPDDR &= ~(1 << (GpioNo - EPORT_PIN24));
        }
        else
        {
            EPORT4->EPDDR &= ~(1 << (GpioNo - EPORT_PIN32));
        }
    }
    else
    {
        if ((GpioNo >> 3) == 0)
        {
            EPORT->EPDDR |= (1 << (GpioNo - EPORT_PIN0));
        }
        else if ((GpioNo >> 3) == 1)
        {
            EPORT1->EPDDR |= (1 << (GpioNo - EPORT_PIN8));
        }
        else if ((GpioNo >> 3) == 2)
        {
            EPORT2->EPDDR |= (1 << (GpioNo - EPORT_PIN16));
        }
        else if ((GpioNo >> 3) == 3)
        {
            EPORT3->EPDDR |= (1 << (GpioNo - EPORT_PIN24));
        }
        else
        {
            EPORT4->EPDDR |= (1 << (GpioNo - EPORT_PIN32));
        }
    }
    return 0;
}

/*******************************************************************************
* Function Name  : EPORT_WriteGpioData
* Description    : 设置EPORT_PINx对应引脚的电平
* Input          : - EPORT_PINx: EPORT Pin；where x can be 0~15 to select the EPORT peripheral.
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : 0: 设置成功    other：设置失败
******************************************************************************/
void EPORT_WriteGpioData(EPORT_PINx GpioNo, UINT8 bitVal)
{

    assert_param(IS_EPORT_PINx(GpioNo));
    assert_param(IS_GPIO_BIT_ACTION(bitVal));

    if (GpioNo > EPORT_PIN39)
        return;
    if ((GpioNo >> 3) == 0)
    {
        EPORT->EPDDR |= (1 << (GpioNo - EPORT_PIN0));
        if (bitVal == Bit_SET)
        {
            EPORT->EPDR |= (Bit_SET << (GpioNo - EPORT_PIN0));
        }
        else
        {
            EPORT->EPDR &= ~(Bit_SET << (GpioNo - EPORT_PIN0));
        }
    }
    else if ((GpioNo >> 3) == 1)
    {
        EPORT1->EPDDR |= (1 << (GpioNo - EPORT_PIN8));
        if (bitVal == Bit_SET)
        {
            EPORT1->EPDR |= (Bit_SET << (GpioNo - EPORT_PIN8));
        }
        else
        {
            EPORT1->EPDR &= ~(Bit_SET << (GpioNo - EPORT_PIN8));
        }
    }
    else if ((GpioNo >> 3) == 2)
    {
        EPORT2->EPDDR |= (1 << (GpioNo - EPORT_PIN16));
        if (bitVal == Bit_SET)
        {
            EPORT2->EPDR |= (Bit_SET << (GpioNo - EPORT_PIN16));
        }
        else
        {
            EPORT2->EPDR &= ~(Bit_SET << (GpioNo - EPORT_PIN16));
        }
    }
    else if ((GpioNo >> 3) == 3)
    {
        EPORT3->EPDDR |= (1 << (GpioNo - EPORT_PIN24));
        if (bitVal == Bit_SET)
        {
            EPORT3->EPDR |= (Bit_SET << (GpioNo - EPORT_PIN24));
        }
        else
        {
            EPORT3->EPDR &= ~(Bit_SET << (GpioNo - EPORT_PIN24));
        }
    }
    else
    {
        EPORT4->EPDDR |= (1 << (GpioNo - EPORT_PIN32));
        if (bitVal == Bit_SET)
        {
            EPORT4->EPDR |= (Bit_SET << (GpioNo - EPORT_PIN32));
        }
        else
        {
            EPORT4->EPDR &= ~(Bit_SET << (GpioNo - EPORT_PIN32));
        }
    }
}

/*******************************************************************************
* Function Name  : EPORT_ReadGpioData
* Description    : 获取EPORT_PINx对应引脚的电平
* Input          : - EPORT_PINx: EPORT Pin；where x can be 0~15 to select the EPORT peripheral.
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平 
******************************************************************************/
INT8 EPORT_ReadGpioData(EPORT_PINx GpioNo)
{
    INT8 bitstatus = 0x00;

    assert_param(IS_EPORT_PINx(GpioNo));

    if (GpioNo > EPORT_PIN39)
        return -1;
    if ((GpioNo >> 3) == 0)
    {
        //EPORTx->EPDDR &= ~(1<<(GpioNo - EPORT_PIN0));//xukai20181209实际应用中，当配置为输出时，也有可能读取电平状态，所以这里不能设置为输入。
        bitstatus = EPORT->EPPDR;
        if (bitstatus & (Bit_SET << (GpioNo - EPORT_PIN0)))
        {
            return Bit_SET;
        }
        else
        {
            return Bit_RESET;
        }
    }
    else if ((GpioNo >> 3) == 1)
    {
        //EPORT1->EPDDR &= ~(1<<(GpioNo - EPORT_PIN8));//xukai20181209实际应用中，当配置为输出时，也有可能读取电平状态，所以这里不能设置为输入。
        bitstatus = EPORT1->EPPDR;
        if (bitstatus & (Bit_SET << (GpioNo - EPORT_PIN8)))
        {
            return Bit_SET;
        }
        else
        {
            return Bit_RESET;
        }
    }
    else if ((GpioNo >> 3) == 2)
    {
        //EPORT2->EPDDR &= ~(1<<(GpioNo - EPORT_PIN16));//xukai20181209实际应用中，当配置为输出时，也有可能读取电平状态，所以这里不能设置为输入。
        bitstatus = EPORT2->EPPDR;
        if (bitstatus & (Bit_SET << (GpioNo - EPORT_PIN16)))
        {
            return Bit_SET;
        }
        else
        {
            return Bit_RESET;
        }
    }
    else if ((GpioNo >> 3) == 3)
    {
        //EPORT3->EPDDR &= ~(1<<(GpioNo - EPORT_PIN24));//xukai20181209实际应用中，当配置为输出时，也有可能读取电平状态，所以这里不能设置为输入。
        bitstatus = EPORT3->EPPDR;
        if (bitstatus & (Bit_SET << (GpioNo - EPORT_PIN24)))
        {
            return Bit_SET;
        }
        else
        {
            return Bit_RESET;
        }
    }
    else
    {
        //EPORT4->EPDDR &= ~(1<<(GpioNo - EPORT_PIN32));//xukai20181209实际应用中，当配置为输出时，也有可能读取电平状态，所以这里不能设置为输入。
        bitstatus = EPORT4->EPPDR;
        if (bitstatus & (Bit_SET << (GpioNo - EPORT_PIN32)))
        {
            return Bit_SET;
        }
        else
        {
            return Bit_RESET;
        }
    }
}

/*******************************************************************************
* Function Name  : EPORT_ITTypeConfig
* Description    : 设置EPORT_PINx脚的中断触发方式，并使能中断
* Input          : - EPORT_PINx: EPORT Pin；where x can be 0~15 to select the EPORT peripheral.
*                  - IntMode:中断触发方式
*                            LOW_LEVEL_INT: 低电平触发
*                            HIGH_LEVEL_INT: 高电平触发
*                            RISING_EDGE_INT:上升沿触发
*                            FALLING_EDGE_INT:下降沿触发
*                            RISING_FALLING_EDGE_INT:上升沿或下降沿触发
*
* Output         : None
* Return         : None
******************************************************************************/
void EPORT_ITTypeConfig(EPORT_PINx IntNo, EPORT_INT_MODE IntMode)
{
    assert_param(IS_EPORT_PINx(IntNo));
    assert_param(IS_EPORT_INT_MODE(IntMode));

    if (IntNo > EPORT_PIN39)
        return;

    //禁止中断
    if ((IntNo >> 3) == 0)
    {
        EPORT->EPIER &= ~(0x01 << (IntNo - EPORT_PIN0));
    }
    else if ((IntNo >> 3) == 1)
    {
        EPORT1->EPIER &= ~(0x01 << (IntNo - EPORT_PIN8));
    }
    else if ((IntNo >> 3) == 2)
    {
        EPORT2->EPIER &= ~(0x01 << (IntNo - EPORT_PIN16));
    }
    else if ((IntNo >> 3) == 3)
    {
        EPORT3->EPIER &= ~(0x01 << (IntNo - EPORT_PIN24));
    }
    else
    {
        EPORT4->EPIER &= ~(0x01 << (IntNo - EPORT_PIN32));
    }

    //IO swap
    if ((IntNo >= EPORT_PIN9) && (IntNo <= EPORT_PIN11))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 0);
    }

    if ((IntNo >= EPORT_PIN14) && (IntNo <= EPORT_PIN15))
    {
        *(volatile unsigned int *)(0x4000001c) &= ~(1 << 1);
    }

    if ((IntNo >= EPORT_PIN38) && (IntNo <= EPORT_PIN39))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 2);
    }

    if ((IntNo >= EPORT_PIN6) && (IntNo <= EPORT_PIN7))
    {
        *(volatile unsigned int *)(0x4000001c) &= ~(1 << 4);
    }

    if ((IntNo >= EPORT_PIN32) && (IntNo <= EPORT_PIN37))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 7);
    }

    if ((IntNo >= EPORT_PIN26) && (IntNo <= EPORT_PIN31))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 12);
    }

    if ((IntNo >= EPORT_PIN22) && (IntNo <= EPORT_PIN25))
    {
        *(volatile unsigned int *)(0x40000044) &= ~((UINT32)0xf << 28);
        *(volatile unsigned int *)(0x4000001c) |= (1 << 13);
    }

    if ((IntNo >= EPORT_PIN18) && (IntNo <= EPORT_PIN21))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 14);
    }

    if ((IntNo >= EPORT_PIN16) && (IntNo <= EPORT_PIN17))
    {
        *(volatile unsigned int *)(0x4000001c) |= (1 << 15);
    }

    //配置输入
    if ((IntNo >> 3) == 0)
    {
        EPORT->EPDDR &= ~(0x01 << (IntNo - EPORT_PIN0)); //输入
    }
    else if ((IntNo >> 3) == 1)
    {
        EPORT1->EPDDR &= ~(0x01 << (IntNo - EPORT_PIN8)); //输入
    }
    else if ((IntNo >> 3) == 2)
    {
        EPORT2->EPDDR &= ~(0x01 << (IntNo - EPORT_PIN16)); //输入
    }
    else if ((IntNo >> 3) == 3)
    {
        EPORT3->EPDDR &= ~(0x01 << (IntNo - EPORT_PIN24)); //输入
    }
    else
    {
        EPORT4->EPDDR &= ~(0x01 << (IntNo - EPORT_PIN32)); //输入
    }

    switch (IntMode)
    {
    case LOW_LEVEL_INT: //低电平触发
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN0) * 2));
            EPORT->EPLPR &= ~(0x01 << (IntNo - EPORT_PIN0));
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN8) * 2));
            EPORT1->EPLPR &= ~(0x01 << (IntNo - EPORT_PIN8));
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN16) * 2));
            EPORT2->EPLPR &= ~(0x01 << (IntNo - EPORT_PIN16));
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN24) * 2));
            EPORT3->EPLPR &= ~(0x01 << (IntNo - EPORT_PIN24));
        }
        else
        {
            EPORT4->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN32) * 2));
            EPORT4->EPLPR &= ~(0x01 << (IntNo - EPORT_PIN32));
        }
        break;
    }
    case HIGH_LEVEL_INT: //高电平触发
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN0) * 2));
            EPORT->EPLPR |= (0x01 << (IntNo - EPORT_PIN0));
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN8) * 2));
            EPORT1->EPLPR |= (0x01 << (IntNo - EPORT_PIN8));
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN16) * 2));
            EPORT2->EPLPR |= (0x01 << (IntNo - EPORT_PIN16));
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN24) * 2));
            EPORT3->EPLPR |= (0x01 << (IntNo - EPORT_PIN24));
        }
        else
        {
            EPORT4->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN32) * 2));
            EPORT4->EPLPR |= (0x01 << (IntNo - EPORT_PIN32));
        }
        break;
    }
    case RISING_EDGE_INT: //上升沿触发
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN0) * 2));
            EPORT->EPPAR |= (0x0001 << ((IntNo - EPORT_PIN0) * 2));
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN8) * 2));
            EPORT1->EPPAR |= (0x0001 << ((IntNo - EPORT_PIN8) * 2));
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN16) * 2));
            EPORT2->EPPAR |= (0x0001 << ((IntNo - EPORT_PIN16) * 2));
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN24) * 2));
            EPORT3->EPPAR |= (0x0001 << ((IntNo - EPORT_PIN24) * 2));
        }
        else
        {
            EPORT4->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN32) * 2));
            EPORT4->EPPAR |= (0x0001 << ((IntNo - EPORT_PIN32) * 2));
        }
        break;
    }
    case FALLING_EDGE_INT: //下降沿触发
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN0) * 2));
            EPORT->EPPAR |= (0x0002 << ((IntNo - EPORT_PIN0) * 2));
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN8) * 2));
            EPORT1->EPPAR |= (0x0002 << ((IntNo - EPORT_PIN8) * 2));
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN16) * 2));
            EPORT2->EPPAR |= (0x0002 << ((IntNo - EPORT_PIN16) * 2));
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN24) * 2));
            EPORT3->EPPAR |= (0x0002 << ((IntNo - EPORT_PIN24) * 2));
        }
        else
        {
            EPORT4->EPPAR &= ~(0x0003 << ((IntNo - EPORT_PIN32) * 2));
            EPORT4->EPPAR |= (0x0002 << ((IntNo - EPORT_PIN32) * 2));
        }
        break;
    }
    case RISING_FALLING_EDGE_INT: //上升沿和下降沿都触发
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPPAR |= (0x0003 << ((IntNo - EPORT_PIN0) * 2));
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPPAR |= (0x0003 << ((IntNo - EPORT_PIN8) * 2));
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPPAR |= (0x0003 << ((IntNo - EPORT_PIN16) * 2));
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPPAR |= (0x0003 << ((IntNo - EPORT_PIN24) * 2));
        }
        else
        {
            EPORT4->EPPAR |= (0x0003 << ((IntNo - EPORT_PIN32) * 2));
        }
        break;
    }
    default:
    {
        break;
    }
    }
    //使能中断
    if ((IntNo >> 3) == 0)
    {
        EPORT->EPIER |= (0x01 << (IntNo - EPORT_PIN0));
    }
    else if ((IntNo >> 3) == 1)
    {
        EPORT1->EPIER |= (0x01 << (IntNo - EPORT_PIN8));
    }
    else if ((IntNo >> 3) == 2)
    {
        EPORT2->EPIER |= (0x01 << (IntNo - EPORT_PIN16));
    }
    else if ((IntNo >> 3) == 3)
    {
        EPORT3->EPIER |= (0x01 << (IntNo - EPORT_PIN24));
    }
    else
    {
        EPORT4->EPIER |= (0x01 << (IntNo - EPORT_PIN32));
    }
}

/*******************************************************************************
* Function Name  : EPORT_ITConfig
* Description    : EPORT中断使能控制
* Input          : - EPORT_PINx: EPORT Pin；where x can be 0~15 to select the EPORT peripheral.
*                  - NewState：new state of the specified EPORT interrupts.
*                              This parameter can be: ENABLE or DISABLE.
*
* Output         : None
* Return         : None
******************************************************************************/
void EPORT_ITConfig(EPORT_PINx IntNo, FunctionalState NewState)
{
    assert_param(IS_EPORT_PINx(IntNo));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (IntNo > EPORT_PIN39)
        return;

    if (NewState != DISABLE)
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPIER |= (0x01 << (IntNo - EPORT_PIN0)); //使能中断
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPIER |= (0x01 << (IntNo - EPORT_PIN8)); //使能中断
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPIER |= (0x01 << (IntNo - EPORT_PIN16)); //使能中断
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPIER |= (0x01 << (IntNo - EPORT_PIN24)); //使能中断
        }
        else
        {
            EPORT4->EPIER |= (0x01 << (IntNo - EPORT_PIN32)); //使能中断
        }
    }
    else
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPIER &= ~(0x01 << (IntNo - EPORT_PIN0)); //disable中断
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPIER &= ~(0x01 << (IntNo - EPORT_PIN8)); //disable中断
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPIER &= ~(0x01 << (IntNo - EPORT_PIN16)); //disable中断
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPIER &= ~(0x01 << (IntNo - EPORT_PIN24)); //disable中断
        }
        else
        {
            EPORT4->EPIER &= ~(0x01 << (IntNo - EPORT_PIN32)); //disable中断
        }
    }
}

/*******************************************************************************
* Function Name  : EPORT_PullupConfig
* Description    : EPORT上拉使能控制
* Input          : - EPORT_PINx: EPORT Pin；where x can be 0~15 to select the EPORT peripheral.
*                  - NewState：new state of the specified EPORT Pullup.
*                              This parameter can be: ENABLE or DISABLE.
*
* Output         : None
* Return         : None
******************************************************************************/
void EPORT_PullupConfig(EPORT_PINx IntNo, FunctionalState NewState)
{
    assert_param(IS_EPORT_PINx(IntNo));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPPUER |= (0x01 << (IntNo - EPORT_PIN0));
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPPUER |= (0x01 << (IntNo - EPORT_PIN8));
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPPUER |= (0x01 << (IntNo - EPORT_PIN16));
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPPUER |= (0x01 << (IntNo - EPORT_PIN24));
        }
        else
        {
            EPORT4->EPPUER |= (0x01 << (IntNo - EPORT_PIN32));
        }
    }
    else
    {
        if ((IntNo >> 3) == 0)
        {
            EPORT->EPPUER &= ~(0x01 << (IntNo - EPORT_PIN0));
        }
        else if ((IntNo >> 3) == 1)
        {
            EPORT1->EPPUER &= ~(0x01 << (IntNo - EPORT_PIN8));
        }
        else if ((IntNo >> 3) == 2)
        {
            EPORT2->EPPUER &= ~(0x01 << (IntNo - EPORT_PIN16));
        }
        else if ((IntNo >> 3) == 3)
        {
            EPORT3->EPPUER &= ~(0x01 << (IntNo - EPORT_PIN24));
        }
        else
        {
            EPORT4->EPPUER &= ~(0x01 << (IntNo - EPORT_PIN32));
        }
    }
}

/*******************************************************************************
* Function Name  : EPORT_Init
* Description    : EPORT初始化
* Input          : -PINx   中断引脚号
*                  -IntMode 中断模式
*
* Output         : None
* Return         : None
******************************************************************************/
extern void EPORT_Init(EPORT_PINx PINx, EPORT_INT_MODE IntMode)
{

    if (((PINx >> 3) == 0) || ((PINx >> 3) == 2)) //EPORT0 或 EPORT2
    {
        NVIC_Init(3, 3, (EPORT0_0_IRQn) + (PINx & 7), 2);
    }
    else
    {
        NVIC_Init(3, 3, (EPORT1_0_IRQn) + (PINx & 7), 2);
    }

    EPORT_ITTypeConfig(PINx, IntMode);
    EPORT_ITConfig(PINx, ENABLE);
}

/** 
 * 1.enable pullup
 * 2.enable pulldown, EPORT_PIN30~EPORT_PIN39 not support pulldown.
 * 3.disable pullup or pulldown
*/
uint32_t EPORT_PullConfig(const EPORT_PINx IntNo, const EPORT_PULL_MODE eport_pull_mode)
{
    const uint8_t eport_group = IntNo / 8;
    const uint8_t gint_offset = IntNo % 8;
    EPORT_TypeDef *p_Eport;
    switch (eport_group)
    {
    case 0:
        p_Eport = EPORT;
        break;
    case 1:
        p_Eport = EPORT1;
        break;
    case 2:
        p_Eport = EPORT2;
        break;
    case 3:
        p_Eport = EPORT3;
        break;
    case 4:
        p_Eport = EPORT4;
        break;
    default:
        return 0x01; // invalid group
    }

    if (IntNo > EPORT_PIN39)
    {
        return 0x01; // invalid IntNo
    }
    if (eport_pull_mode == EPORT_PULLDOWN)
    {
        p_Eport->EPPUER |= (1u << gint_offset);
        if (IntNo >= EPORT_PIN30)
        {
            return 0x02; // not supprt pulldown
        }

        if (IntNo >= EPORT_PIN22) // gint 22~29; (bit8-11)->gint26-29;(bit12-15)->gint22-25
        {
            const uint8_t gint_offset_tmp = IntNo - 22;
            if (gint_offset_tmp > 3)
            {
                IOCTRL->GINT_29_22_CONTROL_REG &= ~(1u << (8 + gint_offset_tmp - 4));
            }
            else
            {
                IOCTRL->GINT_29_22_CONTROL_REG &= ~(1u << (12 + gint_offset_tmp));
            }
        }
        else if (IntNo >= EPORT_PIN16) // gint16~21
        {
            switch (IntNo)
            {
            case EPORT_PIN16:
                IOCTRL->SPIM1_CONTROL_REG &= ~(1u << 20);
                break;
            case EPORT_PIN17:
                IOCTRL->SPIM1_CONTROL_REG &= ~(1u << 21);
                break;
            case EPORT_PIN18:
                IOCTRL->SPIM1_CONTROL_REG &= ~(1u << 18);
                break;
            case EPORT_PIN19:
                IOCTRL->SPIM1_CONTROL_REG &= ~(1u << 19);
                break;
            case EPORT_PIN20:
                IOCTRL->SPIM1_CONTROL_REG &= ~(1u << 17);
                break;
            case EPORT_PIN21:
                IOCTRL->SPIM1_CONTROL_REG &= ~(1u << 16);
                break;
            default:
                return 0x03; // parameter error
            }
        }
        else if (IntNo == EPORT_PIN11)
        {
            IOCTRL->SPI_CONTROL_REG &= ~(1u << 27);
        }
        else if (IntNo == EPORT_PIN10)
        {
            IOCTRL->SPI_CONTROL_REG &= ~(1u << 26);
        }
        else if (IntNo == EPORT_PIN9)
        {
            IOCTRL->SPI_CONTROL_REG &= ~(1u << 25);
        }
        else if (IntNo >= EPORT_PIN8)
        {
            IOCTRL->GINTH_CONTROL_REG &= ~(1u << (24 + gint_offset));
            if(IntNo == EPORT_PIN8)
            {
                IOCTRL->SPI_CONTROL_REG  &= ~(1<<16);
            }
        }
        else
        {
            IOCTRL->GINTL_CONTROL_REG &= ~(1u << (24 + gint_offset));
        }
    }
    else if (eport_pull_mode == EPORT_PULLUP)
    {
        p_Eport->EPPUER |= (1u << gint_offset);
        if (IntNo >= EPORT_PIN30)
        {
            ;
        }
        else if (IntNo >= EPORT_PIN22)
        {
            const uint8_t gint_offset_tmp = IntNo - 22;
            if (gint_offset_tmp > 3)
            {
                IOCTRL->GINT_29_22_CONTROL_REG |= (1u << (8 + gint_offset_tmp - 4));
            }
            else
            {
                IOCTRL->GINT_29_22_CONTROL_REG |= (1u << (12 + gint_offset_tmp));
            }
        }
        else if (IntNo >= EPORT_PIN16) // gint16~21
        {
            switch (IntNo)
            {
            case EPORT_PIN16:
                IOCTRL->SPIM1_CONTROL_REG |= (1u << 20);
                break;
            case EPORT_PIN17:
                IOCTRL->SPIM1_CONTROL_REG |= (1u << 21);
                break;
            case EPORT_PIN18:
                IOCTRL->SPIM1_CONTROL_REG |= (1u << 18);
                break;
            case EPORT_PIN19:
                IOCTRL->SPIM1_CONTROL_REG |= (1u << 19);
                break;
            case EPORT_PIN20:
                IOCTRL->SPIM1_CONTROL_REG |= (1u << 17);
                break;
            case EPORT_PIN21:
                IOCTRL->SPIM1_CONTROL_REG |= (1u << 16);
                break;
            default:
                return 0x03; // parameter error
            }
        }
        else if (IntNo == EPORT_PIN11)
        {
            IOCTRL->SPI_CONTROL_REG |= (1u << 27);
        }
        else if (IntNo == EPORT_PIN10)
        {
            IOCTRL->SPI_CONTROL_REG |= (1u << 26);
        }
        else if (IntNo == EPORT_PIN9)
        {
            IOCTRL->SPI_CONTROL_REG |= (1u << 25);
        }
        else if (IntNo >= EPORT_PIN8)
        {
            IOCTRL->GINTH_CONTROL_REG |= (1u << (24 + gint_offset));
            if(IntNo == EPORT_PIN8)
            {
                IOCTRL->SPI_CONTROL_REG  |= (1<<16);
            }
        }
        else
        {
            IOCTRL->GINTL_CONTROL_REG |= (1u << (24 + gint_offset));
        }
    }
    else //eport dispull
    {
        p_Eport->EPPUER &= ~(1u << gint_offset);
    }

    return 0x00; // right
}
