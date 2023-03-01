#ifndef __SKY1311_DRV_H
#define __SKY1311_DRV_H

#include "spi_drv.h"
#include "eport_drv.h"
#include "delay.h"

#include "Sky_typedef.h"
#include "Sky_userdef.h"


#define RFID_SPI 	SPI2

#define RFID_CS_PORT     
#define RFID_CS_INIT()  			  SPI_ConfigGpio(RFID_SPI, SPI_SS, GPIO_OUTPUT);SPI_WriteGpioData(RFID_SPI, SPI_SS, Bit_SET);
#define RFID_CS_H()      			  SPI_WriteGpioData(RFID_SPI, SPI_SS, Bit_SET);
#define RFID_CS_L()      			  SPI_WriteGpioData(RFID_SPI, SPI_SS, Bit_RESET);

#define RFID_CHIPEN_PORT 				EPORT_PIN25
#define RFID_CHIPEN_INIT()			EPORT_PullupConfig(RFID_CHIPEN_PORT,DISABLE); EPORT_ConfigGpio(RFID_CHIPEN_PORT,GPIO_OUTPUT); 
#define RFID_CHIPEN_H()					EPORT_WriteGpioData(RFID_CHIPEN_PORT, Bit_SET);	
#define RFID_CHIPEN_L()					EPORT_WriteGpioData(RFID_CHIPEN_PORT, Bit_RESET);																

#define	RFID_IRQ_PORT 				 	EPORT_PIN22
#define RFID_IRQ_INIT()				  EPORT_PullupConfig(RFID_IRQ_PORT,DISABLE);EPORT_ConfigGpio(RFID_IRQ_PORT,GPIO_INPUT);  
#define RFID_IRQ_STATUS()		    EPORT_ReadGpioData(RFID_IRQ_PORT)



void RFID_PortDeInit(void);
void SKY1311_ChipEn(void);
void SKY1311_ChipDis(void);
void sky1311WriteCmd(uint8_t cmd);
void sky1311WriteReg(uint8_t regAdd, uint8_t data);
uint8_t sky1311ReadReg(uint8_t regAdd);
void sky1311WriteFifo(uint8_t *data, uint8_t count);
void sky1311ReadFifo(uint8_t *data, uint8_t count);
void RFID_PortInit(void);


#define SKY1311_IRQ_READ()   RFID_IRQ_STATUS()
#define SKY1311_DISABLE()    SKY1311_ChipDis()
#define SKY1311_ENABLE()     SKY1311_ChipEn()
#define sky1311Disable()     SKY1311_DISABLE()
#define sky1311Enable()      SKY1311_ENABLE()
//void SKY1311_SYSCLK(void);


#endif

