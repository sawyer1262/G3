#include "gpio_demo.h"

void GPIO_GintDemo(void)
{
    uint8_t tmp[12];
	
    //配稚输入
    GINT0_CONFIG_OUT;
    GINT1_CONFIG_OUT;
    GINT2_CONFIG_OUT;
    GINT3_CONFIG_OUT;

    //配稚输入
    GINT4_CONFIG_IN;  
    GINT5_CONFIG_IN;  
    GINT6_CONFIG_IN; 
    GINT7_CONFIG_IN; 
    
    while(1)
    {
        //输出高
        GINT0_OUT(1);
        GINT1_OUT(1);
        GINT2_OUT(1);
        GINT3_OUT(1);

        tmp[0] = GINT4_IN; //GINT4连接GINT0
        tmp[1] = GINT5_IN; //GINT5连接GINT1
        tmp[2] = GINT6_IN; //GINT6连接GINT2
        tmp[3] = GINT7_IN; //GINT7连接GINT3

        printf("GINT0:%d.\r\n",tmp[0]);
        printf("GINT1:%d.\r\n",tmp[1]);
        printf("GINT2:%d.\r\n",tmp[2]);
        printf("GINT3:%d.\r\n\r\n",tmp[3]);

        DelayMS(500);

        //输出低
        GINT0_OUT(0);
        GINT1_OUT(0);
        GINT2_OUT(0);
        GINT3_OUT(0);

        tmp[0] = GINT4_IN; //GINT4连接GINT0
        tmp[1] = GINT5_IN; //GINT5连接GINT1
        tmp[2] = GINT6_IN; //GINT6连接GINT2
        tmp[3] = GINT7_IN; //GINT7连接GINT3

        printf("GINT0:%d.\r\n",tmp[0]);
        printf("GINT1:%d.\r\n",tmp[1]);
        printf("GINT2:%d.\r\n",tmp[2]);
        printf("GINT3:%d.\r\n\r\n",tmp[3]);

        DelayMS(500);
    }
    
}

void GPIO_UartDemo(void)
{
    uint8_t tmp[3];
    
    //配置成输出
    UART_ConfigGpio(SCI1,UART_RX, GPIO_INPUT);
    UART_ConfigGpio(SCI2,UART_RX, GPIO_INPUT);
    UART_ConfigGpio(SCI3,UART_RX, GPIO_INPUT);
    //配置成输入
	UART_ConfigGpio(SCI1,UART_TX, GPIO_OUTPUT);
	UART_ConfigGpio(SCI2,UART_TX, GPIO_OUTPUT);
	UART_ConfigGpio(SCI3,UART_TX, GPIO_OUTPUT);

    while(1)
    {
        //输出高
        GPIO_UART1_TX_OUT(1);
        GPIO_UART2_TX_OUT(1);
        GPIO_UART3_TX_OUT(1);
        
        tmp[0] = GPIO_UART1_RX_IN;  //连接UART1 TX
        tmp[1] = GPIO_UART2_RX_IN;  //连接UART2 TX
        tmp[2] = GPIO_UART3_RX_IN;  //连接UART3 TX
        
        printf("UART1 RX:%d.\r\n",tmp[0]);
        printf("UART2 RX:%d.\r\n",tmp[1]);
        printf("UART3 RX:%d.\r\n\r\n",tmp[2]);
        
        DelayMS(500);
        
        //输出低
        GPIO_UART1_TX_OUT(0);
        GPIO_UART2_TX_OUT(0);
        GPIO_UART3_TX_OUT(0);
        
        tmp[0] = GPIO_UART1_RX_IN;  //连接UART1 TX
        tmp[1] = GPIO_UART2_RX_IN;  //连接UART2 TX
        tmp[2] = GPIO_UART3_RX_IN;  //连接UART3 TX
        
        printf("UART1 RX:%d.\r\n",tmp[0]);
        printf("UART2 RX:%d.\r\n",tmp[1]);
        printf("UART3 RX:%d.\r\n\r\n",tmp[2]);
        
        DelayMS(500);

    }

}

void GPIO_SpiDemo(void)
{
    uint8_t tmp[6];
    
    //配置成输出
    SPI_ConfigGpio(SPI1, SPI_MISO, GPIO_OUTPUT);
    SPI_ConfigGpio(SPI1, SPI_MOSI, GPIO_OUTPUT);
    //配置成输入
    SPI_ConfigGpio(SPI1, SPI_SCK, GPIO_INPUT);
    SPI_ConfigGpio(SPI1, SPI_SS, GPIO_INPUT);
    
    //配置成输出
    SPI_ConfigGpio(SPI2, SPI_MISO, GPIO_OUTPUT);
    SPI_ConfigGpio(SPI2, SPI_MOSI, GPIO_OUTPUT);
    //配置成输入
    SPI_ConfigGpio(SPI2, SPI_SCK, GPIO_INPUT);
    SPI_ConfigGpio(SPI2, SPI_SS, GPIO_INPUT);
    
    //配置成输出
    SPI_ConfigGpio(SPI3, SPI_MISO, GPIO_OUTPUT);
    SPI_ConfigGpio(SPI3, SPI_MOSI, GPIO_OUTPUT);
    //配置成输入
    SPI_ConfigGpio(SPI3, SPI_SCK, GPIO_INPUT);
    SPI_ConfigGpio(SPI3, SPI_SS, GPIO_INPUT);
    
    while(1)
    {
        //输出高
        GPIO_SPI1_MISO_OUT(1);
        GPIO_SPI1_MOSI_OUT(1);   
        GPIO_SPI2_MISO_OUT(1);
        GPIO_SPI2_MOSI_OUT(1);    
        GPIO_SPI3_MISO_OUT(1);
        GPIO_SPI3_MOSI_OUT(1);
        
        tmp[0] = GPIO_SPI1_SCK_IN;  //连接SPI1 MISO
        tmp[1] = GPIO_SPI1_SS_IN;   //连接SPI1 MOMI
        tmp[2] = GPIO_SPI2_SCK_IN;  //连接SPI2 MISO
        tmp[3] = GPIO_SPI2_SS_IN;   //连接SPI2 MOMI
        tmp[4] = GPIO_SPI3_SCK_IN;  //连接SPI3 MISO
        tmp[5] = GPIO_SPI3_SS_IN;   //连接SPI3 MOMI
        
        printf("SPI1 SCK:%d.\r\n",tmp[0]);
        printf("SPI1 SS:%d.\r\n",tmp[1]);
        printf("SPI2 SCK:%d.\r\n",tmp[2]);
        printf("SPI2 SS:%d.\r\n",tmp[3]);
        printf("SPI3 SCK:%d.\r\n",tmp[4]);
        printf("SPI3 SS:%d.\r\n\r\n",tmp[5]);
        
        DelayMS(500);
        
        //输出低
        GPIO_SPI1_MISO_OUT(0);
        GPIO_SPI1_MOSI_OUT(0);   
        GPIO_SPI2_MISO_OUT(0);
        GPIO_SPI2_MOSI_OUT(0);    
        GPIO_SPI3_MISO_OUT(0);
        GPIO_SPI3_MOSI_OUT(0);
        
        tmp[0] = GPIO_SPI1_SCK_IN;
        tmp[1] = GPIO_SPI1_SS_IN;
        tmp[2] = GPIO_SPI2_SCK_IN;
        tmp[3] = GPIO_SPI2_SS_IN;
        tmp[4] = GPIO_SPI3_SCK_IN;
        tmp[5] = GPIO_SPI3_SS_IN;
        
        printf("SPI1 SCK:%d.\r\n",tmp[0]);
        printf("SPI1 SS:%d.\r\n",tmp[1]);
        printf("SPI2 SCK:%d.\r\n",tmp[2]);
        printf("SPI2 SS:%d.\r\n",tmp[3]);
        printf("SPI3 SCK:%d.\r\n",tmp[4]);
        printf("SPI3 SS:%d.\r\n\r\n",tmp[5]);
        
        DelayMS(500);
    }
}

void GPIO_I2cDemo(void)
{
    uint8_t tmp[3];
    
    //配置成输出
    I2C_ConfigGpio(I2C1, I2C_SDA, GPIO_OUTPUT);
    I2C_ConfigGpio(I2C2, I2C_SDA, GPIO_OUTPUT);
    I2C_ConfigGpio(I2C3, I2C_SDA, GPIO_OUTPUT);
    
    //配置成输入
    I2C_ConfigGpio(I2C1, I2C_SCL, GPIO_INPUT);
    I2C_ConfigGpio(I2C2, I2C_SCL, GPIO_INPUT);
    I2C_ConfigGpio(I2C3, I2C_SCL, GPIO_INPUT);
    
    tmp[0] = GPIO_I2C1_SCL_IN;
    tmp[0] = GPIO_I2C1_SCL_IN;
    tmp[0] = GPIO_I2C1_SDA_IN;
    tmp[0] = GPIO_I2C1_SDA_IN;
    
    tmp[0] = GPIO_I2C2_SCL_IN;
    tmp[0] = GPIO_I2C2_SCL_IN;
    tmp[0] = GPIO_I2C2_SDA_IN;
    tmp[0] = GPIO_I2C2_SDA_IN;
    
    tmp[0] = GPIO_I2C3_SCL_IN;
    tmp[0] = GPIO_I2C3_SCL_IN;
    tmp[0] = GPIO_I2C3_SDA_IN;
    tmp[0] = GPIO_I2C3_SDA_IN;
    
    while(1)
    {
        //输出高
        GPIO_I2C1_SDA_OUT(1);
        GPIO_I2C2_SDA_OUT(1);
        GPIO_I2C3_SDA_OUT(1);
        
        tmp[0] = GPIO_I2C1_SCL_IN;  //连接I2C1 SDA
        tmp[1] = GPIO_I2C2_SCL_IN;  //连接I2C2 SDA
        tmp[2] = GPIO_I2C3_SCL_IN;  //连接I2C2 SDA
        
        printf("I2C1 SCL:%d.\r\n",tmp[0]);
        printf("I2C2 SCL:%d.\r\n",tmp[1]);
        printf("I2C3 SCL:%d.\r\n\r\n",tmp[2]);
        
        DelayMS(500);
        
        //输出低
        GPIO_I2C1_SDA_OUT(0);
        GPIO_I2C2_SDA_OUT(0);
        GPIO_I2C3_SDA_OUT(0);
        
        tmp[0] = GPIO_I2C1_SCL_IN;
        tmp[1] = GPIO_I2C2_SCL_IN;
        tmp[2] = GPIO_I2C3_SCL_IN;
        
        printf("I2C1 SCL:%d.\r\n",tmp[0]);
        printf("I2C2 SCL:%d.\r\n",tmp[1]);
        printf("I2C3 SCL:%d.\r\n\r\n",tmp[2]);
        
        DelayMS(500);
    }
}

void GPIO_UsiDemo(void)
{
    uint8_t tmp[4];
    //配置成输出
    USI_ConfigGpio(USI1, USI_RST, GPIO_OUTPUT);
    //配置成输入
    USI_ConfigGpio(USI1, USI_CLK, GPIO_INPUT);
    USI_ConfigGpio(USI1, USI_DAT, GPIO_INPUT);

    //配置成输出
    USI_ConfigGpio(USI2, USI_RST, GPIO_OUTPUT);
    //配置成输入
    USI_ConfigGpio(USI2, USI_CLK, GPIO_INPUT);
    USI_ConfigGpio(USI2, USI_DAT, GPIO_INPUT);

    while(1)
    {
        //输出高
        GPIO_USI1_RST_OUT(1);
        GPIO_USI2_RST_OUT(1);

        tmp[0] = GPIO_USI1_CLK_IN;  //连接USI1 RST
        tmp[1] = GPIO_USI1_DAT_IN;  //连接USI1 RST
        tmp[2] = GPIO_USI2_CLK_IN;  //连接USI2 RST
        tmp[3] = GPIO_USI2_DAT_IN;  //连接USI2 RST

        printf("USI1 CLK:%d.\r\n",tmp[0]);
        printf("USI1 DAT:%d.\r\n",tmp[1]);
        printf("USI2 CLK:%d.\r\n",tmp[2]);
        printf("USI2 DAT:%d.\r\n\r\n",tmp[3]);

        DelayMS(500);

        //输出低
        GPIO_USI1_RST_OUT(0);
        GPIO_USI2_RST_OUT(0);

        tmp[0] = GPIO_USI1_CLK_IN;
        tmp[1] = GPIO_USI1_DAT_IN; 
        tmp[2] = GPIO_USI2_CLK_IN;
        tmp[3] = GPIO_USI2_DAT_IN;

        printf("USI1 CLK:%d.\r\n",tmp[0]);
        printf("USI1 DAT:%d.\r\n",tmp[1]);
        printf("USI2 CLK:%d.\r\n",tmp[2]);
        printf("USI2 DAT:%d.\r\n\r\n",tmp[3]);

        DelayMS(500);
    }
}

void GPIO_Demo(void)
{
    GPIO_GintDemo();  //配置普通GPIO功能，中断功能参考EPORT Demo 
//    GPIO_UartDemo(); //配置普通GPIO功能，不支持中断
//    GPIO_SpiDemo();  //配置普通GPIO功能，不支持中断
//    GPIO_I2cDemo();  //配置普通GPIO功能，不支持中断
//    GPIO_UsiDemo(); //ISO7816 配置普通GPIO功能，不支持中断
}
