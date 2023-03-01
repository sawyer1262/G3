// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : adc_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <string.h>
#include "i2c_demo.h"
#include "i2c_drv.h"
#include "type.h"
#include "debug.h"
#include "delay.h"
#include "common.h"

#define I2C_MASTER_DEMO

void Demo_I2C_CPUIntTransfer(I2C_TypeDef *I2Cx)
{
    uint8_t i2c_write_buf[256];
	uint8_t i2c_read_buf[256];
    uint16_t ret_val = 0;
    uint16_t slave_addr;
	uint32_t i,test_len;

    slave_addr = 0x230;  //测试时，主从保持一致
    test_len = 256;      //测试时，主从保持一致

    for(i=0;i<test_len;i++)
    {
        i2c_write_buf[i] = i;
    }

#ifdef I2C_MASTER_DEMO

	ret_val = I2C_MasterInit(I2Cx);
    if(STATUS_OK != ret_val)
    {
        printf("I2C master init failed, ret:%04x.\n",ret_val);
        while(1);
    }
    while(1)
    {
        ret_val = I2C_MasterWriteData(I2Cx, slave_addr, i2c_write_buf, test_len);
        if(STATUS_OK != ret_val)
        {
            printf("I2C master write data failed, ret:%04x.\n",ret_val);
            DelayMS(1000);
        }
        DelayMS(200);
        memset(i2c_read_buf,0,test_len);
        ret_val = I2c_MasterReadData(I2Cx, slave_addr, i2c_read_buf, test_len);
        if(STATUS_OK != ret_val)
        {
            printf("I2C master read data failed, ret:%04x.\n",ret_val);
            DelayMS(1000);
        }
        DelayMS(200);

        for(i=0;i<0xc0;i++)
        {
            if(i2c_read_buf[i] != i2c_write_buf[i])
            {
                printf("The i2c transfer failed, addr:%d.\n",i);
                DelayMS(1000);
                break;
            }
        }

        if(STATUS_OK != ret_val)
        {
            printf("The i2c transfer failed.\n");
            DelayMS(2000);
        }
        else
        {
            printf("The i2c transfer succeeds.\n");
        }
    }

#else

	ret_val = I2C_SlaveInit(I2Cx, slave_addr);

    if(STATUS_OK != ret_val)
    {
        printf("I2C slave init failed, ret:%04x.\n",ret_val);
        while(1);
    }

	while(1)
	{
        memset(i2c_read_buf,0,test_len);
		ret_val = I2C_SlaveTransfer(i2c_read_buf, test_len, i2c_write_buf, test_len);

        if(STATUS_OK != ret_val)
        {
            printf("I2C slave transfer failed, ret:%04x.\n",ret_val);
            DelayMS(1000);
        }
        else
        {
            for(i = 0;i<test_len;i++)
            {
                printf("%02x",i2c_read_buf[i]);
            }
            printf("\r\n");
        }

    }

#endif
}

void I2C_Demo(void)
{
    I2C_TypeDef *i2c_type_id;

	i2c_type_id = I2C1;
    //i2c_type_id = I2C2;
    //i2c_type_id = I2C3;

    /* I2C CPU 中断方式传输demo */
    Demo_I2C_CPUIntTransfer(i2c_type_id);
}


