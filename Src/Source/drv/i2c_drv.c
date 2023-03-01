// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : i2c_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "delay.h"
#include "debug.h"
#include "common.h"
#include "def.h"
#include "i2c_drv.h"

static volatile UINT8 i2c_status;
static volatile UINT8  i2c_hs_status=0xff;
static volatile UINT32 g_len_receive = 0;
static volatile UINT32 g_len_send = 0;

static UINT8 *g_data_send ;
static UINT8 *g_data_receive;

#if 1

/**
 * @brief I2C中断服务接口.
 *
 * @param[in] I2Cx 指向I2C_TypeDef结构体的指针;
 * @return NONE
 */
static void I2C_ISR(I2C_TypeDef *I2Cx)
{
	volatile UINT8 tmp =0;
	i2c_status = I2Cx->CSR;
	i2c_hs_status = I2Cx->SHSIR;

	if((i2c_hs_status & SLV_HS) != SLV_HS )
	{
		if((i2c_status & SR_AASLV) == SR_AASLV )		//The current slave device is addressed by master
		{
			if((i2c_status & SR_RC) == SR_RC) // RECEIVER interrupt
			{
				if((I2Cx->CSR & SR_TF) == SR_TF) // transfer complete
				{
					g_data_receive[g_len_receive] = I2Cx->CDR;
					g_len_receive++;
					I2Cx->CR |= CR_ACKEN;
				}
				else
				{
					I2Cx->CR &= ~CR_AMIE;
				}
			}
			else // RECEIVER interrupt
			{
				if((i2c_status & SR_TF) == SR_TF) // transfer complete
				{
					if((i2c_status & SR_DACK)==SR_DACK)
					{
						I2Cx->CDR =g_data_send[g_len_send];
						g_len_send++;
						I2Cx->CR &= ~CR_AMIE;
					}
					else
					{
						tmp = I2Cx->CDR;
						tmp ++;//add for warning
					}
				}
				else
				{
					I2Cx->CR &= ~CR_AMIE;
				}
			}
		}
		else
		{
			//asm("bkpt");
		}
	}
	else
	{
		I2Cx->SSHTR =(I2Cx->SSHTR&0xc0)|0x1;
		I2Cx->SSHTR =I2Cx->SSHTR|0x1;

		I2Cx->SHSIR |=SLV_HS;
	}
}

/**
 * @brief I2C1中断服务接口.
 *
 * @return NONE
 */
void I2C1_IRQHandler(void)
{
	I2C_ISR(I2C1);
}

/**
 * @brief I2C2中断服务接口.
 *
 * @return NONE
 */
void I2C2_IRQHandler(void)
{
	I2C_ISR(I2C2);
}

/**
 * @brief I2C3中断服务接口.
 *
 * @return NONE
 */
void I2C3_IRQHandler(void)
{
	I2C_ISR(I2C3);
}
/**
 * @brief 把寄存器恢复默认设置，防止休眠把i2c配置成gpio后，唤醒无法正常使用i2c功能
 * 
 * @param[in] 
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * @return 无
 */
static void I2C_Default(I2C_TypeDef *I2Cx)
{
	I2Cx->CR = 0x08;
	I2Cx->PCR = 0x03;
	I2Cx->PDR = 0x00;
	I2Cx->DDR = 0x00;
}

/**
 * @brief I2C配置成GPIO功能.
 *
 * @param[in] 
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - I2C对应的PIN脚，取值I2C_SCL、I2C_SDA;
 * - GPIO_Dir 设置GPIO方向
 *   -# GPIO_OUTPUT：输出
 *   -# GPIO_INPUT： 输入
 *@return STATUS
 */
void I2C_ConfigGpio(I2C_TypeDef *I2Cx, I2C_PIN I2C_PINx, uint8_t GPIO_Dir)
{
	/* Check the parameters */
	assert_param(IS_I2C_PINx(I2C_PINx));
	assert_param(IS_GPIO_DIR_BIT(GPIO_Dir));

//	I2C_Default(I2Cx); //先恢复默认配置

	if (I2Cx == I2C1)
	{
		*(volatile uint32_t*)(0x40023074) &= ~0x00000060;		//SDA、SCL和TSI CH6、CH5复用，禁止TSI CH6、CH5
	}

	I2Cx->PCR |= 0xc0;//config gpio
	if (GPIO_Dir == GPIO_OUTPUT)
	{
		I2Cx->DDR |= (1<<I2C_PINx);//output
	}
	else if (GPIO_Dir == GPIO_INPUT)
	{
		I2Cx->DDR &= (~(1<<I2C_PINx));//input
	}
}

/**
 * @brief 获取I2C_PINx对应引脚的电平.
 *
 * @param[in] 
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - I2C_PINx I2C对应的PIN脚，取值I2C_SCL、I2C_SDA;
 * @return I2C_PINx电平
 * - Bit_SET:高电平
 * - Bit_RESET：低电平
 */
uint8_t I2C_ReadGpioData(I2C_TypeDef *I2Cx, I2C_PIN I2C_PINx)
{
	UINT8 bitstatus = 0x00;

	/* Check the parameters */
    assert_param(IS_I2C(I2Cx));
	assert_param(IS_I2C_PINx(I2C_PINx));

	bitstatus = I2Cx->PDR;
	if (bitstatus &(Bit_SET<<I2C_PINx))
		bitstatus = Bit_SET;
	else
		bitstatus = Bit_RESET;

	return bitstatus;
}

/**
 * @brief 设置I2C_PINx对应引脚的电平.
 *
 * @param[in] 
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - I2C_PINx I2C对应的PIN脚，取值I2C_SCL、I2C_SDA;
 * - bitVal 设置的电平
 *   -# Bit_SET：设置为高电平
 *   -# Bit_RESET：设置为低电平
 * @return NONE
 */
void I2C_WriteGpioData(I2C_TypeDef *I2Cx, I2C_PIN I2C_PINx, uint8_t bitVal)
{
	/* Check the parameters */
    assert_param(IS_I2C(I2Cx));
	assert_param(IS_I2C_PINx(I2C_PINx));

	if (bitVal == Bit_SET)
		I2Cx->PDR |= (Bit_SET<<I2C_PINx);
	else
		I2Cx->PDR &= (~(Bit_SET<<I2C_PINx));
}

/**
 * @brief 配置I2C主从模式.
 *
 * @param[in]
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - pre 地址信息;
 * - mode ：主从模式选择
 * @return NONE
 */
static uint16_t I2C_Config(I2C_TypeDef *I2Cx, uint16_t pre, uint8_t mode)
{
    if((I2Cx != I2C1)&&(I2Cx != I2C2)&&(I2Cx != I2C3))
    {
        return STATUS_ID_ERR;
    }

	I2Cx->CR = 0x0;
	I2Cx->CR |= CR_EN;

	delay(100);

	I2Cx->CR |= CR_ACKEN;

	if(mode == I2C_MASTER_MODE)
	{
        if(pre > 0x3F)
        {
            return STATUS_ADDR_ERR;
        }
		I2Cx->CPR =(UINT8)pre;
	}
	else if(mode == I2C_SLAVE_MODE)
	{
		I2Cx->CR |=CR_IEN;
		
		if((pre & 0x0300) != 0)/*10bit地址模式*/
		{
			I2Cx->SAHR = (UINT8)(((pre>>8)<<1)|0XF0);
			I2Cx->SALR = (UINT8)pre;
		}
		else /*7bit地址模式*/
		{
			I2Cx->SAHR = (UINT8)pre;
		}
		/*配置从模式下的中断*/
        if(I2C1 == I2Cx)
        {
            NVIC_Init(3, 3, I2C_1_IRQn, 2);
        }
        else if(I2C2 == I2Cx)
        {
            NVIC_Init(3, 3, I2C_2_IRQn, 2);
        }
        else
        {
            NVIC_Init(3, 3, I2C_3_IRQn, 2);
        }
		
	}
	else
	{
		return STATUS_I2C_MODE_ERR;
	}

	return STATUS_OK;
}

/**
 * @brief I2C主设备初始化.
 *
 * @param[in] I2Cx  指向I2C_TypeDef结构体的指针;
 * @return STATUS
 */
uint16_t I2C_MasterInit(I2C_TypeDef *I2Cx)
{
	I2C_Default(I2Cx); //先恢复默认配置
	return I2C_Config(I2Cx, 0x0000,I2C_MASTER_MODE);
}

/**
 * @brief I2C从设备初始化.
 *
 * @param[in]
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - Addresser 从设备地址
 * @return STATUS
 */
uint16_t I2C_SlaveInit(I2C_TypeDef *I2Cx, UINT16 Addresser)
{
	return I2C_Config(I2Cx, Addresser, I2C_SLAVE_MODE);
}

/**
 * @brief I2C主设备发送单字节数据.
 *
 * @param[in]
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - data 待发送数据;
 * @return STATUS
 */
uint16_t I2C_MasterSend(I2C_TypeDef *I2Cx, uint8_t data)
{
    uint32_t timeout;

    timeout = I2C_TIMEOUT_COUNTERS(1000);

    if((I2Cx != I2C1)&&(I2Cx != I2C2)&&(I2Cx != I2C3))
    {
        return STATUS_ID_ERR;
    }

    I2Cx->CDR = data;
    while((I2Cx->CSR & SR_TF) != SR_TF)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    while((I2Cx->CSR & SR_DACK) != SR_DACK)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

    return STATUS_OK;
}

/**
 * @brief I2C主设备接收单字节数据.
 *
 * @param[in]
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - len_offset 待接收数据的偏移量;
 * - len 待接收数据的总长度
 * - data 接收的数据
 * @return STATUS
 */
uint16_t I2C_MasterReceive(I2C_TypeDef *I2Cx, uint16_t len_offset, uint32_t len, uint8_t *data)
{
    uint32_t timeout;

    timeout = I2C_TIMEOUT_COUNTERS(1000);

    if((I2Cx != I2C1)&&(I2Cx != I2C2)&&(I2Cx != I2C3))
    {
        return STATUS_ID_ERR;
    }

    while((I2Cx->CSR & SR_TF) != SR_TF)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    
    *data = I2Cx->CDR;

    if((len > 1) && (len_offset != (len - 2)))
        I2Cx->CR |= CR_ACKEN;		//write ACKEN to clear RC status and enable next byte from Slave
    else
        I2Cx->CR &= ~CR_ACKEN;	//no write ACKEN to stop next byte from Slave

    return STATUS_OK;
}

/**
 * @brief I2C主设备发送多字节.
 *
 * @param[in]
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - slave_addr 从设备地址;
 * - buf 发送数据缓存
 * - len 数据长度
 * @return STATUS
 */
uint16_t I2C_MasterWriteData(I2C_TypeDef *I2Cx, uint16_t slave_addr, uint8_t *buf, uint32_t len)
{
    uint8_t status = 0xff;
    uint16_t i;
    uint16_t ret = STATUS_OK;
    uint32_t timeout;

    timeout = I2C_TIMEOUT_COUNTERS(1000);

    if((I2Cx != I2C1)&&(I2Cx != I2C2)&&(I2Cx != I2C3))
    {
        return STATUS_ID_ERR;
    }

#ifndef I2C_REPEAT_START

    if(slave_addr &0x300)
    {
    	I2Cx->CDR = (UINT8)(((slave_addr>>8)<<1)|0xF0);
    	while((I2Cx->CSR & SR_BBUSY) == SR_BBUSY)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
    	I2Cx->CR |= CR_MSMOD;
    	while((I2Cx->CSR & SR_TF) != SR_TF)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
    	while((I2Cx->CSR & SR_DACK) != SR_DACK)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
    	status = I2Cx->CSR;
    	I2Cx->CDR =(UINT8)(slave_addr &0xFF);
		while((I2Cx->CSR & SR_TF) != SR_TF)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
    }
    else
    {
		//send slave addr;
		I2Cx->CDR = slave_addr & 0xfe;
		while((I2Cx->CSR & SR_BBUSY) == SR_BBUSY)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
		I2Cx->CR |= CR_MSMOD;
		while((I2Cx->CSR & SR_TF) != SR_TF)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
    }
#endif

    //send Data
    while((I2Cx->CSR & SR_DACK) != SR_DACK)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

    status = I2Cx->CSR;

    if((status & SR_ARBL) == SR_ARBL)
    {
        I2Cx->CR &= ~CR_MSMOD;
//        printf("I2C Master arbitration lost occur!\n");
    }
    else if((status & SR_AACK) == SR_AACK)
    {
        I2Cx->CR &= ~CR_MSMOD;		//clear MSMOD to send STOP
//        printf("I2C Master aack error!\n");
    }
    else
    {
		for(i = 0; i < len; i++)
		{
			ret = I2C_MasterSend(I2Cx, *( buf + i));
            if(STATUS_OK != ret)
            {
                return ret;
            }
		}

    }

    I2Cx->CR &= ~CR_MSMOD;
    while(I2Cx->CSR & SR_BBUSY)		//busy bit clear indicate STOP finish
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    return ret;
}

/**
 * @brief I2C主设备发送多字节.
 *
 * @param[in]
 * - I2Cx  指向I2C_TypeDef结构体的指针;
 * - slave_addr 从设备地址;
 * - buf 接收数据缓存
 * - len 数据长度
 * @return STATUS
 */
uint16_t I2c_MasterReadData(I2C_TypeDef *I2Cx, uint16_t slave_addr, uint8_t *buf, uint32_t len)
{
    uint8_t status;
    uint16_t ret = STATUS_OK;
    uint32_t timeout,i;

    timeout = I2C_TIMEOUT_COUNTERS(1000);

#ifndef I2C_REPEAT_START
    //slave addr
    if(slave_addr &0x300)
	{
		I2Cx->CDR = (UINT8)(((slave_addr>>8)<<1)|0xF0);
		while((I2Cx->CSR & SR_BBUSY) == SR_BBUSY)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
		I2Cx->CR |= CR_MSMOD;
		while((I2Cx->CSR & SR_TF) != SR_TF)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }

		I2Cx->CDR =(UINT8)(slave_addr &0xFF);
		while((I2Cx->CSR & SR_TF) != SR_TF)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
		while((I2Cx->CSR & SR_DACK) != SR_DACK)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }

		I2Cx->CR |= CR_REPSTA;
		I2Cx->CDR = (UINT8)(((slave_addr>>8)<<1)|0xF1);
		while((I2Cx->CSR & SR_RC) != SR_RC)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
		I2Cx->CR |= CR_ACKEN;
		while((I2Cx->CSR & SR_DACK) != SR_DACK)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
	}
    else
    {
		I2Cx->CDR = slave_addr | 0x01;
		while((I2Cx->CSR & SR_BBUSY) == SR_BBUSY)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
		I2Cx->CR |= CR_MSMOD;
		while((I2Cx->CSR & SR_RC) != SR_RC)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
		if(len > 1)
		{
			I2Cx->CR |= CR_ACKEN;
		}
		else
		{
			I2Cx->CR &= ~CR_ACKEN;
		}
		while((I2Cx->CSR & SR_DACK) != SR_DACK)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
    }
#endif

    //receive data
    while((I2Cx->CSR & SR_RC) != SR_RC)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

    status = I2Cx->CSR;

    if((status & SR_ARBL) == SR_ARBL)
    {
        I2Cx->CR &= ~CR_MSMOD;		//clear MSMOD to send STOP
//        printf("I2C Master arbitration lost occur!\n");
    }
    else if((status & SR_AACK) == SR_AACK)
    {
        I2Cx->CR &= ~CR_MSMOD;		//clear MSMOD to send STOP
//        printf("I2C Master aack error!\n");
    }
    else
    {
        for(i = 0; i < len; i++)
        {
            // *(buf + i) = I2C_master_rec(I2Cx, i, len);
            ret = I2C_MasterReceive(I2Cx, i, len, (buf + i));

            if(STATUS_OK != ret)
            {
                return ret;
            }
        }

    }

    //delayUs(10000);

    I2Cx->CR &= ~CR_MSMOD;
    while(I2Cx->CSR & SR_BBUSY)		//busy bit clear indicate STOP finish
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

    return ret;
}

/**
 * @brief I2C从设备收发多字节.
 *
 * @param[in]
 * - r_buf 接收数据缓存;
 * - r_len 接收数据长度；
 * - s_buf 发送数据缓存
 * - s_len 发送数据长度
 * @return STATUS
 */
uint16_t I2C_SlaveTransfer(uint8_t *r_buf, uint16_t r_len, uint8_t *s_buf, uint32_t s_len)
{
    
    uint32_t timeout;

    timeout = I2C_TIMEOUT_COUNTERS(3000);
	g_data_send = s_buf;
	g_data_receive = r_buf;

	while(1)
	{
		if(r_len==g_len_receive)
		{
			g_len_receive=0x0;

			break;
		}

		if(s_len==g_len_send)
		{
			g_len_send=0x0;

			break;
		}

        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;

	}

    return STATUS_OK;
}

#else


/*******************************************************************************
* Function Name  : I2C_ConfigGpio
* Description    : I2C配置成GPIO用途
* Input          : - I2C_PINx：I2C对应的PIN脚，取值I2C_SCL、I2C_SDA
*                  - GPIO_Dir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
void I2C_ConfigGpio(I2C_PIN I2C_PINx, UINT8 GPIO_Dir)
{
	/* Check the parameters */
	assert_param(IS_I2C_PINx(I2C_PINx));
	assert_param(IS_GPIO_DIR_BIT(GPIO_Dir));

	*(volatile UINT32*)(0x40023074) &= ~0x00000060;		//SDA、SCL和TSI CH6、CH5复用，禁止TSI CH6、CH5

	I2C->PCR |= 0xc0;//config gpio
	if (GPIO_Dir == GPIO_OUTPUT)
	{
		I2C->DDR |= (1<<I2C_PINx);//output
	}
	else if (GPIO_Dir == GPIO_INPUT)
	{
		I2C->DDR &= (~(1<<I2C_PINx));//input
	}
}


/*******************************************************************************
* Function Name  : I2C_ReadGpioData
* Description    : 获取I2C_PINx对应引脚的电平
* Input          : - I2C_PINx：SPI对应的PIN脚，取值I2C_SCL、I2C_SDA
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平
******************************************************************************/
UINT8 I2C_ReadGpioData(I2C_PIN I2C_PINx)
{
	UINT8 bitstatus = 0x00;

	/* Check the parameters */
	assert_param(IS_I2C_PINx(I2C_PINx));

	bitstatus = I2C->PDR;
	if (bitstatus &(Bit_SET<<I2C_PINx))
		bitstatus = Bit_SET;
	else
		bitstatus = Bit_RESET;

	return bitstatus;
}


/*******************************************************************************
* Function Name  : I2C_WriteGpioData
* Description    : 设置I2C_PINx对应引脚的电平
* Input          : - I2C_PINx：SPI对应的PIN脚，取值I2C_SCL、I2C_SDA
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : None
******************************************************************************/
void I2C_WriteGpioData(I2C_PIN I2C_PINx, UINT8 bitVal)
{
	/* Check the parameters */
	assert_param(IS_I2C_PINx(I2C_PINx));

	if (bitVal == Bit_SET)
		I2C->PDR |= (Bit_SET<<I2C_PINx);
	else
		I2C->PDR &= (~(Bit_SET<<I2C_PINx));
}

/*******************************************************************************
* Function Name  : I2C_Config
* Description    : 配置I2C主从模式
* Input          : - pre    : 地址信息
*                  - mode   ：主从模式选择
*
* Output         : None
* Return         : None
******************************************************************************/
static uint16_t I2C_Config(UINT16 pre,UINT8 mode)
{
	I2C->CR = 0x0;
	I2C->CR |= CR_EN;

	delay(100);

	I2C->CR |= CR_ACKEN;

	if(mode == I2C_MASTER_MODE)
	{
		I2C->CPR =(UINT8)pre;
	}
	else if(mode == I2C_SLAVE_MODE)
	{
		I2C->CR |=CR_IEN;
		
		//if((pre & 0x0300) != 0)/*10bit地址模式*/
		{
			I2C->SAHR = (UINT8)(((pre>>8)<<1)|0XF0);
			I2C->SALR = (UINT8)pre;
		}
//		else /*7bit地址模式*/
//		{
//			I2C->SAHR = (UINT8)pre;
//		}
		/*配置从模式下的中断*/
		NVIC_Init(3, 3, I2C_IRQn, 2);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
* Function Name  : i2c_master_init
* Description    :I2C主设备初始化
* Input          :-Addresser:主设备地址
*
* Output         : None
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
BOOL i2c_master_init(UINT8 Addresser)
{
	if(Addresser >0x3F)
	{
		return FALSE;
	}
	
	return I2C_Config((UINT16)Addresser,I2C_MASTER_MODE);
}

/*******************************************************************************
* Function Name  : i2c_slave_init
* Description    :I2C从设备初始化
* Input          :-Addresser:从设备地址
*
* Output         : None
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
BOOL i2c_slave_init(UINT16 Addresser)
{
	return I2C_Config(Addresser, I2C_SLAVE_MODE);
}

/*******************************************************************************
* Function Name  : i2c_gpio_init
* Description    :I2C GPIO Init
* Input          :- None
*
* Output         : None
* Return         : 无
******************************************************************************/
void i2c_gpio_init(void)
{
	I2C_ConfigGpio(I2C_SDA, GPIO_OUTPUT);
	I2C_ConfigGpio(I2C_SCL, GPIO_OUTPUT);
	I2C_WriteGpioData(I2C_SDA, Bit_SET);
	I2C_WriteGpioData(I2C_SCL, Bit_SET);
}

/*******************************************************************************
* Function Name  : i2c_master_send
* Description    :I2C主设备发送单字节数据
* Input          : -data     :待发送数据 
*
* Output         : None
* Return         : None
******************************************************************************/
void I2C_master_send(u8 data)
{
    I2C->CDR = data;
    while((I2C->CSR & SR_TF) != SR_TF);
    while((I2C->CSR & SR_DACK) != SR_DACK);
}

/*******************************************************************************
* Function Name  : I2C_master_rec
* Description    :I2C主设备接收单字节数据
* Input          :-i        :待接收数据的偏移量
*									-len      :待接收数据的总长度
*
* Output         : None
* Return         : UINT8     接收的数据
******************************************************************************/
u8 I2C_master_rec(u16 i, u16 len)
{
    u8 data;

    while((I2C->CSR & SR_TF) != SR_TF);

    data = I2C->CDR;

    if((len > 1) && (i != (len - 2)))
        I2C->CR |= CR_ACKEN;		//write ACKEN to clear RC status and enable next byte from Slave
    else
        I2C->CR &= ~CR_ACKEN;	//no write ACKEN to stop next byte from Slave

    return data;
}

/*******************************************************************************
* Function Name  : I2c_writedata
* Description    :I2C主设备发送多字节
* Input          : -slave_addr      :从设备地址
*                  -buf             :待发送数据
*                  -len             :数据长度
* Output         : None
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
bool I2c_writedata(u16 slave_addr, u8 *buf, u16 len)
{
    u16 i;
    u8 status = 0xff;
    bool ret = FALSE;

#ifndef I2C_REPEAT_START

    if(slave_addr &0x300)
    {
    	I2C->CDR = (UINT8)(((slave_addr>>8)<<1)|0xF0);
    	while((I2C->CSR & SR_BBUSY) == SR_BBUSY);
    	I2C->CR |= CR_MSMOD;
    	while((I2C->CSR & SR_TF) != SR_TF);
    	while((I2C->CSR & SR_DACK) != SR_DACK);
    	status = I2C->CSR;
    	I2C->CDR =(UINT8)(slave_addr &0xFF);
		while((I2C->CSR & SR_TF) != SR_TF);
    }
    else
    {
		//send slave addr;
		I2C->CDR = slave_addr & 0xfe;
		while((I2C->CSR & SR_BBUSY) == SR_BBUSY);
		I2C->CR |= CR_MSMOD;
		while((I2C->CSR & SR_TF) != SR_TF);
    }
#endif

    //send Data
    while((I2C->CSR & SR_DACK) != SR_DACK);

    status = I2C->CSR;

    if((status & SR_ARBL) == SR_ARBL)
    {
        I2C->CR &= ~CR_MSMOD;
//        printf("I2C Master arbitration lost occur!\n");
    }
    else if((status & SR_AACK) == SR_AACK)
    {
        I2C->CR &= ~CR_MSMOD;		//clear MSMOD to send STOP
//        printf("I2C Master aack error!\n");
    }
    else
    {
		for(i = 0; i < len; i++)
		{
			I2C_master_send(*(buf + i));
		}
		ret = TRUE;
    }

    I2C->CR &= ~CR_MSMOD;
    while(I2C->CSR & SR_BBUSY);		//busy bit clear indicate STOP finish

    return ret;
}

/*******************************************************************************
* Function Name  : I2c_readdata
* Description    :I2C主设备发送多字节
* Input          : -slave_addr      :从设备地址
* Output         : -buf             :待接收数据
*                  -len             :数据长度
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
bool I2c_readdata(u16 slave_addr, u8 *buf, u16 len)
{
    u16 i;
    u8 status;
    bool ret = FALSE;

#ifndef I2C_REPEAT_START
    //slave addr
    if(slave_addr &0x300)
	{
		I2C->CDR = (UINT8)(((slave_addr>>8)<<1)|0xF0);
		while((I2C->CSR & SR_BBUSY) == SR_BBUSY);
		I2C->CR |= CR_MSMOD;
		while((I2C->CSR & SR_TF) != SR_TF);

		I2C->CDR =(UINT8)(slave_addr &0xFF);
		while((I2C->CSR & SR_TF) != SR_TF);
		while((I2C->CSR & SR_DACK) != SR_DACK);

		I2C->CR |= CR_REPSTA;
		I2C->CDR = (UINT8)(((slave_addr>>8)<<1)|0xF1);
		while((I2C->CSR & SR_RC) != SR_RC);
		I2C->CR |= CR_ACKEN;
		while((I2C->CSR & SR_DACK) != SR_DACK);
	}
    else
    {
		I2C->CDR = slave_addr | 0x01;
		while((I2C->CSR & SR_BBUSY) == SR_BBUSY);
		I2C->CR |= CR_MSMOD;
		while((I2C->CSR & SR_RC) != SR_RC);
		if(len > 1)
		{
			I2C->CR |= CR_ACKEN;
		}
		else
		{
			I2C->CR &= ~CR_ACKEN;
		}
		while((I2C->CSR & SR_DACK) != SR_DACK);
    }
#endif

    //receive data
    while((I2C->CSR & SR_RC) != SR_RC);

    status = I2C->CSR;

    if((status & SR_ARBL) == SR_ARBL)
    {
        I2C->CR &= ~CR_MSMOD;		//clear MSMOD to send STOP
//        printf("I2C Master arbitration lost occur!\n");
    }
    else if((status & SR_AACK) == SR_AACK)
    {
        I2C->CR &= ~CR_MSMOD;		//clear MSMOD to send STOP
//        printf("I2C Master aack error!\n");
    }
    else
    {
        for(i = 0; i < len; i++)
        {
            *(buf + i) = I2C_master_rec(i, len);
        }

        ret = TRUE;
    }

    //delayUs(10000);

    I2C->CR &= ~CR_MSMOD;
    while(I2C->CSR & SR_BBUSY);		//busy bit clear indicate STOP finish

    return ret;
}

/*******************************************************************************
* Function Name  : I2C_ISR
* Description    :I2C从中断
* Input          : -None
* Output         : -None
*
* Return         : -None
******************************************************************************/
static void I2C_ISR(void)
{
	volatile UINT8 tmp =0;
	i2c_status = I2C->CSR;
	i2c_hs_status = I2C->SHSIR;

	if((i2c_hs_status & SLV_HS) != SLV_HS )
	{
		if((i2c_status & SR_AASLV) == SR_AASLV )		//The current slave device is addressed by master
		{
			if((i2c_status & SR_RC) == SR_RC) // RECEIVER interrupt
			{
				if((I2C->CSR & SR_TF) == SR_TF) // transfer complete
				{
					g_data_receive[g_len_receive] = I2C->CDR;
					g_len_receive++;
					I2C->CR |= CR_ACKEN;
				}
				else
				{
					I2C->CR &= ~CR_AMIE;
				}
			}
			else // RECEIVER interrupt
			{
				if((i2c_status & SR_TF) == SR_TF) // transfer complete
				{
					if((i2c_status & SR_DACK)==SR_DACK)
					{
						I2C->CDR =g_data_send[g_len_send];
						g_len_send++;
						I2C->CR &= ~CR_AMIE;
					}
					else
					{
						tmp = I2C->CDR;
						tmp ++;//add for warning
					}
				}
				else
				{
					I2C->CR &= ~CR_AMIE;
				}
			}
		}
		else
		{
			//asm("bkpt");
		}
	}
	else
	{
		I2C->SSHTR =(I2C->SSHTR&0xc0)|0x1;
		I2C->SSHTR =I2C->SSHTR|0x1;

		I2C->SHSIR |=SLV_HS;
	}
}

void I2C1_IRQHandler(void)
{
	I2C_ISR();
}

void I2C2_IRQHandler(void)
{
	I2C_ISR();
}

void I2C3_IRQHandler(void)
{
	I2C_ISR();
}

/*******************************************************************************
* Function Name  : i2c_slave_trans
* Description    :I2C从设备收发送多字节
* Input          : -s_buf           :接收数据
* 				   -s_len           :数据长度
* Output         : -r_buf           :待接收数据
*                  -r_len           :数据长度
* Return         : None
******************************************************************************/
void i2c_slave_trans(UINT8* r_buf,UINT16 r_len,UINT8* s_buf,UINT16 s_len)
{
	g_data_send = s_buf;
	g_data_receive = r_buf;

	while(1)
	{
		if(r_len==g_len_receive)
		{
			g_len_receive=0x0;

			break;
		}

		if(s_len==g_len_send)
		{
			g_len_send=0x0;

			break;
		}

	}
}

#endif
