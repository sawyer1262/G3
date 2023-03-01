#ifndef __OLED_H
#define __OLED_H

#include "type.h"
#include "stdlib.h"
#include "i2c_drv.h"
#include "spi_drv.h"
#define OLED_Hardware_i2c

#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    						  
//-----------------OLED IIC端口定义----------------  					   
//#define OLED_RST_OUT	SPI_Config_Gpio(SPI2,SPI_MOSI,GPIO_OUTPUT)   //MOSI2
//#define OLED_RST_L		SPI_Write_Gpio_Data(SPI2,SPI_MOSI,Bit_RESET)
//#define OLED_RST_H		SPI_Write_Gpio_Data(SPI2,SPI_MOSI,Bit_SET)

#define OLED_SDA_OUT	I2C_ConfigGpio(I2C3,I2C_SDA,GPIO_OUTPUT)

#define OLED_SCL_OUT	I2C_ConfigGpio(I2C3,I2C_SCL,GPIO_OUTPUT)

#define OLED_SCLK_Clr()  I2C_WriteGpioData(I2C3,I2C_SCL,Bit_RESET)//CLK
#define OLED_SCLK_Set()  I2C_WriteGpioData(I2C3,I2C_SCL,Bit_SET)

#define OLED_SDIN_Clr()  I2C_WriteGpioData(I2C3,I2C_SDA,Bit_RESET)//DIN
#define OLED_SDIN_Set()  I2C_WriteGpioData(I2C3,I2C_SDA,Bit_SET)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


//OLED控制用函数
void OLED_WR_Byte(unsigned dat,unsigned cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);	   							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(UINT8 x,UINT8 y,UINT8 t);
void OLED_Fill(UINT8 x1,UINT8 y1,UINT8 x2,UINT8 y2,UINT8 dot);
void OLED_ShowChar(UINT8 x,UINT8 y,UINT8 chr,UINT8 Char_Size);
void OLED_ShowNum(UINT8 x,UINT8 y,UINT32 num,UINT8 len,UINT8 size);
void OLED_ShowString(UINT8 x,UINT8 y, UINT8 *p,UINT8 Char_Size);	 
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(UINT8 x,UINT8 y,UINT8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
void Delay_50ms(unsigned int Del_50ms);
void Delay_1ms(unsigned int Del_1ms);
void fill_picture(unsigned char fill_Data);
void Picture(void);
void IIC_Start(void);
void IIC_Stop(void);
void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Byte(unsigned char IIC_Byte);
void OLED_ShowCHinese_string(UINT8 x,UINT8 y,UINT8 *chr,UINT8 size);



#endif  
	 
