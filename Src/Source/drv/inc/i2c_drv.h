// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : i2c_drv.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef I2C_DRV_H_
#define I2C_DRV_H_

#include "type.h"
#include "memmap.h"
#include "i2c_reg.h"


#define I2C_BASE_ADDR             (I2C1_BASE_ADDR)
// #define I2C_BASE_ADDR             (I2C2_BASE_ADDR)
// #define I2C_BASE_ADDR             (I2C3_BASE_ADDR)

#define I2C                       ((I2C_TypeDef *)I2C1_BASE_ADDR)

#define I2C1                       ((I2C_TypeDef *)I2C1_BASE_ADDR)
#define I2C2                       ((I2C_TypeDef *)I2C2_BASE_ADDR)
#define I2C3                       ((I2C_TypeDef *)I2C3_BASE_ADDR)

#if I2C_BASE_ADDR == I2C1_BASE_ADDR
	#define I2C_IRQn   I2C_1_IRQn

#elif I2C_BASE_ADDR == I2C2_BASE_ADDR
	#define I2C_IRQn   I2C_2_IRQn
#else
	#define I2C_IRQn   I2C_3_IRQn
#endif

#define I2C_TIMEOUT_COUNTERS(x)          (x*10000) 

typedef enum
{
    I2C_SCL = 0,
    I2C_SDA,

}I2C_PIN;

#define I2C_MASTER_MODE		1
#define I2C_SLAVE_MODE		0


#define IS_I2C_PINx(pin) (((pin) == I2C_SCL) || \
                          ((pin) == I2C_SDA))

#define IS_I2C(i2c) (((i2c) == I2C1) || \
                     ((i2c) == I2C2) || \
                     ((i2c) == I2C3))
#if 1

extern void I2C1_IRQHandler(void);

extern void I2C2_IRQHandler(void);

extern void I2C3_IRQHandler(void);

extern void I2C_Default(I2C_TypeDef *I2Cx);

extern void I2C_ConfigGpio(I2C_TypeDef *I2Cx, I2C_PIN I2C_PINx, uint8_t GPIO_Dir);

extern uint8_t I2C_ReadGpioData(I2C_TypeDef *I2Cx, I2C_PIN I2C_PINx);

extern void I2C_WriteGpioData(I2C_TypeDef *I2Cx, I2C_PIN I2C_PINx, uint8_t bitVal);

extern uint16_t I2C_MasterInit(I2C_TypeDef *I2Cx);

extern uint16_t I2C_SlaveInit(I2C_TypeDef *I2Cx, UINT16 Addresser);

extern uint16_t I2C_MasterSend(I2C_TypeDef *I2Cx, uint8_t data);

extern uint16_t I2C_MasterReceive(I2C_TypeDef *I2Cx, uint16_t len_offset, uint32_t len, uint8_t *data);

extern uint16_t I2C_MasterWriteData(I2C_TypeDef *I2Cx, uint16_t slave_addr, uint8_t *buf, uint32_t len);

extern uint16_t I2c_MasterReadData(I2C_TypeDef *I2Cx, uint16_t slave_addr, uint8_t *buf, uint32_t len);

extern uint16_t I2C_SlaveTransfer(uint8_t *r_buf, uint16_t r_len, uint8_t *s_buf, uint32_t s_len);

#else
/*******************************************************************************
* Function Name  : I2C_ConfigGpio
* Description    : I2C配置成GPIO用途
* Input          : - I2C_PINx：SPI对应的PIN脚，取值I2C_SCL、I2C_SDA
*                  - GPIO_Dir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
extern void I2C_ConfigGpio(I2C_PIN I2C_PINx, UINT8 GPIO_Dir);

/*******************************************************************************
* Function Name  : I2C_ReadGpioData
* Description    : 获取I2C_PINx对应引脚的电平
* Input          : - I2C_PINx：SPI对应的PIN脚，取值I2C_SCL、I2C_SDA
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平
******************************************************************************/
extern UINT8 I2C_ReadGpioData(I2C_PIN I2C_PINx);


/*******************************************************************************
* Function Name  : I2C_WriteGpioData
* Description    : 设置I2C_PINx对应引脚的电平
* Input          : - I2C_PINx：SPI对应的PIN脚，取值I2C_SCL、I2C_SDA
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : None
******************************************************************************/
extern void I2C_WriteGpioData(I2C_PIN I2C_PINx, UINT8 bitVal);

/*******************************************************************************
* Function Name  : i2c_master_init
* Description    :I2C主设备初始化
* Input          :-Addresser:主设备地址
*
* Output         : None
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
extern BOOL i2c_master_init(UINT8 Addresser);

/*******************************************************************************
* Function Name  : i2c_slave_init
* Description    :I2C从设备初始化
* Input          :-Addresser:从设备地址
*
* Output         : None
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
extern BOOL i2c_slave_init(UINT16 Addresser);

/*******************************************************************************
* Function Name  : i2c_gpio_init
* Description    :I2C GPIO Init
* Input          :- None
*
* Output         : None
* Return         : 无
******************************************************************************/
extern void i2c_gpio_init(void);

/*******************************************************************************
* Function Name  : I2C_master_send
* Description    :I2C主设备发送单字节数据
* Input          : -data     :待发送数据 
*
* Output         : None
* Return         : None
******************************************************************************/
extern void I2C_master_send(u8 data);

/*******************************************************************************
* Function Name  : I2C_master_rec
* Description    :I2C主设备接收单字节数据
* Input          :-i        :待接收数据的偏移量
*									-len      :待接收数据的总长度
*
* Output         : None
* Return         : UINT8     接收的数据
******************************************************************************/
extern u8 I2C_master_rec(u16 i, u16 len);

/*******************************************************************************
* Function Name  : I2c_writedata
* Description    :I2C主设备发送多字节
* Input          : -slave_addr      :从设备地址
*                  -buf             :待发送数据
*                  -len             :数据长度
* Output         : None
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
extern bool I2c_writedata(u16 slave_addr, u8 *buf, u16 len);

/*******************************************************************************
* Function Name  : I2c_readdata
* Description    :I2C主设备发送多字节
* Input          :-slave_addr       :从设备地址
* Output         : -buf             :待接收数据
*                  -len             :数据长度
* Return         : TRUE 执行成功  FALSE 执行失败
******************************************************************************/
extern bool I2c_readdata(u16 slave_addr, u8 *buf, u16 len);

/*******************************************************************************
* Function Name  : i2c_slave_trans
* Description    :I2C从设备收发送多字节
* Input          : -s_buf           :接收数据
* 				   -s_len           :数据长度
* Output         : -r_buf           :待接收数据
*                  -r_len           :数据长度
* Return         : None
******************************************************************************/
extern void i2c_slave_trans(UINT8* r_buf,UINT16 r_len,UINT8* s_buf,UINT16 s_len);

#endif

#endif /* I2C_DRV_H_ */
