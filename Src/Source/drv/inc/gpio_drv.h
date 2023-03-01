// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : gpio_drv.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef GPIO_DRV_H_
#define GPIO_DRV_H_

#include "debug.h"
#include "delay.h"
#include "eport_drv.h"
#include "uart_drv.h"
#include "spi_drv.h"
#include "i2c_drv.h"
#include "usi_drv.h"
#include "ioctrl_drv.h"
/************************************gint define************************************************/
//EPORT0
#define GINT0_CONFIG_OUT     do{EPORT->EPDDR |= 0x01;}while(0)
#define GINT1_CONFIG_OUT     do{EPORT->EPDDR |= 0x02;}while(0)
#define GINT2_CONFIG_OUT     do{EPORT->EPDDR |= 0x04;}while(0)
#define GINT3_CONFIG_OUT     do{EPORT->EPDDR |= 0x08;}while(0)
#define GINT4_CONFIG_OUT     do{EPORT->EPDDR |= 0x10;}while(0)
#define GINT5_CONFIG_OUT     do{EPORT->EPDDR |= 0x20;}while(0)
#define GINT6_CONFIG_OUT     do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xEF; \
    EPORT->EPDDR |= 0x40;}while(0)

#define GINT7_CONFIG_OUT     do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xEF; \
    EPORT->EPDDR |= 0x80;}while(0)

#define GINT0_CONFIG_IN      do{EPORT->EPDDR &= 0xFE;}while(0)
#define GINT1_CONFIG_IN      do{EPORT->EPDDR &= 0xFD;}while(0)
#define GINT2_CONFIG_IN      do{EPORT->EPDDR &= 0xFB;}while(0)
#define GINT3_CONFIG_IN      do{EPORT->EPDDR &= 0xF7;}while(0)
#define GINT4_CONFIG_IN      do{EPORT->EPDDR &= 0xEF;}while(0)
#define GINT5_CONFIG_IN      do{EPORT->EPDDR &= 0xDF;}while(0)
#define GINT6_CONFIG_IN      do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xEF; \
    EPORT->EPDDR &= 0xBF;}while(0)

#define GINT7_CONFIG_IN      do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xEF; \
    EPORT->EPDDR &= 0x7F;}while(0)

#define EPORT0_OUT(val)      do{EPORT->EPDR = val;}while(0)
#define GINT0_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x01):(EPORT->EPDR &= 0xFE);}while(0)
#define GINT1_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x02):(EPORT->EPDR &= 0xFD);}while(0)
#define GINT2_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x04):(EPORT->EPDR &= 0xFB);}while(0)
#define GINT3_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x08):(EPORT->EPDR &= 0xF7);}while(0)
#define GINT4_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x10):(EPORT->EPDR &= 0xEF);}while(0)
#define GINT5_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x20):(EPORT->EPDR &= 0xDF);}while(0)
#define GINT6_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x40):(EPORT->EPDR &= 0xBF);}while(0)
#define GINT7_OUT(val)       do{(val&0x01)?(EPORT->EPDR |= 0x80):(EPORT->EPDR &= 0x7F);}while(0)

#define EPORT0_IN            (EPORT->EPPDR)
#define GINT0_IN             (EPORT->EPPDR&0x01)
#define GINT1_IN             ((EPORT->EPPDR&0x02)>>1)
#define GINT2_IN             ((EPORT->EPPDR&0x04)>>2)
#define GINT3_IN             ((EPORT->EPPDR&0x08)>>3)
#define GINT4_IN             ((EPORT->EPPDR&0x10)>>4)
#define GINT5_IN             ((EPORT->EPPDR&0x20)>>5)
#define GINT6_IN             ((EPORT->EPPDR&0x40)>>6)
#define GINT7_IN             ((EPORT->EPPDR&0x80)>>7)

//EPORT1
#define GINT8_CONFIG_OUT     do{\
	*(volatile unsigned int *)(0x40001014) &= ~(1<<13);\
EPORT1->EPDDR |= 0x01;}while(0)
#define GINT9_CONFIG_OUT     do{ \
    *(volatile unsigned int *)(0x4000001c) |= 0x01; \
    EPORT1->EPDDR |= 0x02;}while(0)

#define GINT10_CONFIG_OUT    do{ \
    *(volatile unsigned int *)(0x4000001c) |= 0x01; \
    EPORT1->EPDDR |= 0x04;}while(0)

#define GINT11_CONFIG_OUT    do{ \
    *(volatile unsigned int *)(0x4000001c) |= 0x01; \
    EPORT1->EPDDR |= 0x08;}while(0)

#define GINT12_CONFIG_OUT    do{EPORT1->EPDDR |= 0x10;}while(0)
#define GINT13_CONFIG_OUT    do{EPORT1->EPDDR |= 0x20;}while(0)
#define GINT14_CONFIG_OUT    do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xFD; \
    EPORT1->EPDDR |= 0x40;}while(0)

#define GINT15_CONFIG_OUT    do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xFD; \
    EPORT1->EPDDR |= 0x80;}while(0)

#define GINT8_CONFIG_IN       do{\
	*(volatile unsigned int *)(0x40001014) &= ~(1<<13);\
EPORT1->EPDDR &= 0xFE;}while(0)
#define GINT9_CONFIG_IN       do{ \
    *(volatile unsigned int *)(0x4000001c) |= 0x01; \
    EPORT1->EPDDR &= 0xFD;}while(0)

#define GINT10_CONFIG_IN      do{ \
    *(volatile unsigned int *)(0x4000001c) |= 0x01; \
    EPORT1->EPDDR &= 0xFB;}while(0)

#define GINT11_CONFIG_IN      do{ \
    *(volatile unsigned int *)(0x4000001c) |= 0x01; \
    EPORT1->EPDDR &= 0xF7;}while(0)

#define GINT12_CONFIG_IN      do{EPORT1->EPDDR &= 0xEF;}while(0)
#define GINT13_CONFIG_IN      do{EPORT1->EPDDR &= 0xDF;}while(0)
#define GINT14_CONFIG_IN      do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xFD; \
    EPORT1->EPDDR &= 0xBF;}while(0)

#define GINT15_CONFIG_IN      do{ \
    *(volatile unsigned int *)(0x4000001c) &= 0xFD; \
    EPORT1->EPDDR &= 0x7F;}while(0)


#define EPORT1_OUT(val)       do{EPORT1->EPDR = val;}while(0)
#define GINT8_OUT(val)        do{(val&0x01)?(EPORT1->EPDR |= 0x01):(EPORT1->EPDR &= 0xFE);}while(0)
#define GINT9_OUT(val)        do{(val&0x01)?(EPORT1->EPDR |= 0x02):(EPORT1->EPDR &= 0xFD);}while(0)
#define GINT10_OUT(val)       do{(val&0x01)?(EPORT1->EPDR |= 0x04):(EPORT1->EPDR &= 0xFB);}while(0)
#define GINT11_OUT(val)       do{(val&0x01)?(EPORT1->EPDR |= 0x08):(EPORT1->EPDR &= 0xF7);}while(0)
#define GINT12_OUT(val)       do{(val&0x01)?(EPORT1->EPDR |= 0x10):(EPORT1->EPDR &= 0xEF);}while(0)
#define GINT13_OUT(val)       do{(val&0x01)?(EPORT1->EPDR |= 0x20):(EPORT1->EPDR &= 0xDF);}while(0)
#define GINT14_OUT(val)       do{(val&0x01)?(EPORT1->EPDR |= 0x40):(EPORT1->EPDR &= 0xBF);}while(0)
#define GINT15_OUT(val)       do{(val&0x01)?(EPORT1->EPDR |= 0x80):(EPORT1->EPDR &= 0x7F);}while(0)

#define EPORT1_IN            (EPORT1->EPPDR)
#define GINT8_IN             (EPORT1->EPPDR&0x01)
#define GINT9_IN             ((EPORT1->EPPDR&0x02)>>1)
#define GINT10_IN            ((EPORT1->EPPDR&0x04)>>2)
#define GINT11_IN            ((EPORT1->EPPDR&0x08)>>3)
#define GINT12_IN            ((EPORT1->EPPDR&0x10)>>4)
#define GINT13_IN            ((EPORT1->EPPDR&0x20)>>5)
#define GINT14_IN            ((EPORT1->EPPDR&0x40)>>6)
#define GINT15_IN            ((EPORT1->EPPDR&0x80)>>7)

//EPORT2
#define GINT16_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<15);EPORT2->EPDDR |= 0x01;}while(0)
#define GINT17_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<15);EPORT2->EPDDR |= 0x02;}while(0)
#define GINT18_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR |= 0x04;}while(0)
#define GINT19_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR |= 0x08;}while(0)
#define GINT20_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR |= 0x10;}while(0)
#define GINT21_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR |= 0x20;}while(0)
#define GINT22_CONFIG_OUT     do{\
	 *(volatile unsigned int *)(0x40000044) &= ~((UINT32)0xf << 28);\
IOCTRL->SWAP_CONTROL_REG |= (1<<13);\
EPORT2->EPDDR |= 0x40;}while(0)
#define GINT23_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<13);EPORT2->EPDDR |= 0x80;}while(0)

#define GINT16_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<15);EPORT2->EPDDR &= 0xFE;}while(0)
#define GINT17_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<15);EPORT2->EPDDR &= 0xFD;}while(0)
#define GINT18_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR &= 0xFB;}while(0)
#define GINT19_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR &= 0xF7;}while(0)
#define GINT20_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR &= 0xEF;}while(0)
#define GINT21_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<14);EPORT2->EPDDR &= 0xDF;}while(0)
#define GINT22_CONFIG_IN      do{*(volatile unsigned int *)(0x40000044) &= ~((UINT32)0xf << 28);IOCTRL->SWAP_CONTROL_REG |= (1<<13);EPORT2->EPDDR &= 0xBF;}while(0)
#define GINT23_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<13);EPORT2->EPDDR &= 0x7F;}while(0)

#define EPORT2_OUT(val)       do{EPORT2->EPDR = val)
#define GINT16_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x01):(EPORT2->EPDR &= 0xFE);}while(0)
#define GINT17_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x02):(EPORT2->EPDR &= 0xFD);}while(0)
#define GINT18_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x04):(EPORT2->EPDR &= 0xFB);}while(0)
#define GINT19_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x08):(EPORT2->EPDR &= 0xF7);}while(0)
#define GINT20_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x10):(EPORT2->EPDR &= 0xEF);}while(0)
#define GINT21_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x20):(EPORT2->EPDR &= 0xDF);}while(0)
#define GINT22_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x40):(EPORT2->EPDR &= 0xBF);}while(0)
#define GINT23_OUT(val)       do{(val&0x01)?(EPORT2->EPDR |= 0x80):(EPORT2->EPDR &= 0x7F);}while(0)

#define EPORT2_IN            (EPORT2->EPPDR)
#define GINT16_IN            (EPORT2->EPPDR&0x01)
#define GINT17_IN            ((EPORT2->EPPDR&0x02)>>1)
#define GINT18_IN            ((EPORT2->EPPDR&0x04)>>2)
#define GINT19_IN            ((EPORT2->EPPDR&0x08)>>3)
#define GINT20_IN            ((EPORT2->EPPDR&0x10)>>4)
#define GINT21_IN            ((EPORT2->EPPDR&0x20)>>5)
#define GINT22_IN            ((EPORT2->EPPDR&0x40)>>6)
#define GINT23_IN            ((EPORT2->EPPDR&0x80)>>7)

//EPORT3
#define GINT24_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<13);EPORT3->EPDDR |= 0x01;}while(0)
#define GINT25_CONFIG_OUT     do{*(volatile unsigned int *)(0x40000044) &= ~((UINT32)0xf << 28);IOCTRL->SWAP_CONTROL_REG |= (1<<13);EPORT3->EPDDR |= 0x02;}while(0)
#define GINT26_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR |= 0x04;}while(0)
#define GINT27_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR |= 0x08;}while(0)
#define GINT28_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR |= 0x10;}while(0)
#define GINT29_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR |= 0x20;}while(0)
#define GINT30_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR |= 0x40;}while(0)
#define GINT31_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR |= 0x80;}while(0)

#define GINT24_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<13);EPORT3->EPDDR &= 0xFE;}while(0)
#define GINT25_CONFIG_IN      do{*(volatile unsigned int *)(0x40000044) &= ~((UINT32)0xf << 28);IOCTRL->SWAP_CONTROL_REG |= (1<<13);EPORT3->EPDDR &= 0xFD;}while(0)
#define GINT26_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR &= 0xFB;}while(0)
#define GINT27_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR &= 0xF7;}while(0)
#define GINT28_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR &= 0xEF;}while(0)
#define GINT29_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR &= 0xDF;}while(0)
#define GINT30_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR &= 0xBF;}while(0)
#define GINT31_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<12);EPORT3->EPDDR &= 0x7F;}while(0)

#define EPORT3_OUT(val)       do{EPORT3->EPDR = val;}while(0)
#define GINT24_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x01):(EPORT3->EPDR &= 0xFE);}while(0)
#define GINT25_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x02):(EPORT3->EPDR &= 0xFD);}while(0)
#define GINT26_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x04):(EPORT3->EPDR &= 0xFB);}while(0)
#define GINT27_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x08):(EPORT3->EPDR &= 0xF7);}while(0)
#define GINT28_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x10):(EPORT3->EPDR &= 0xEF);}while(0)
#define GINT29_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x20):(EPORT3->EPDR &= 0xDF);}while(0)
#define GINT30_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x40):(EPORT3->EPDR &= 0xBF);}while(0)
#define GINT31_OUT(val)       do{(val&0x01)?(EPORT3->EPDR |= 0x80):(EPORT3->EPDR &= 0x7F);}while(0)

#define EPORT3_IN            (EPORT3->EPPDR)
#define GINT24_IN            (EPORT3->EPPDR&0x01)
#define GINT25_IN            ((EPORT3->EPPDR&0x02)>>1)
#define GINT26_IN            ((EPORT3->EPPDR&0x04)>>2)
#define GINT27_IN            ((EPORT3->EPPDR&0x08)>>3)
#define GINT28_IN            ((EPORT3->EPPDR&0x10)>>4)
#define GINT29_IN            ((EPORT3->EPPDR&0x20)>>5)
#define GINT30_IN            ((EPORT3->EPPDR&0x40)>>6)
#define GINT31_IN            ((EPORT3->EPPDR&0x80)>>7)

//EPORT4
#define GINT32_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR |= 0x01;}while(0)
#define GINT33_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR |= 0x02;}while(0)
#define GINT34_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR |= 0x04;}while(0)
#define GINT35_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR |= 0x08;}while(0)
#define GINT36_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR |= 0x10;}while(0)
#define GINT37_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR |= 0x20;}while(0)
#define GINT38_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<2);EPORT4->EPDDR |= 0x40;}while(0)
#define GINT39_CONFIG_OUT     do{IOCTRL->SWAP_CONTROL_REG |= (1<<2);EPORT4->EPDDR |= 0x80;}while(0)

#define GINT32_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR &= 0xFE;}while(0)
#define GINT33_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR &= 0xFD;}while(0)
#define GINT34_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR &= 0xFB;}while(0)
#define GINT35_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR &= 0xF7;}while(0)
#define GINT36_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR &= 0xEF;}while(0)
#define GINT37_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<7);EPORT4->EPDDR &= 0xDF;}while(0)
#define GINT38_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<2);EPORT4->EPDDR &= 0xBF;}while(0)
#define GINT39_CONFIG_IN      do{IOCTRL->SWAP_CONTROL_REG |= (1<<2);EPORT4->EPDDR &= 0x7F;}while(0)


#define EPORT4_OUT(val)       do{EPORT4->EPDR = val)while(0)
#define GINT32_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x01):(EPORT4->EPDR &= 0xFE);}while(0)
#define GINT33_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x02):(EPORT4->EPDR &= 0xFD);}while(0)
#define GINT34_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x04):(EPORT4->EPDR &= 0xFB);}while(0)
#define GINT35_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x08):(EPORT4->EPDR &= 0xF7);}while(0)
#define GINT36_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x10):(EPORT4->EPDR &= 0xEF);}while(0)
#define GINT37_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x20):(EPORT4->EPDR &= 0xDF);}while(0)
#define GINT38_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x40):(EPORT4->EPDR &= 0xBF);}while(0)
#define GINT39_OUT(val)       do{(val&0x01)?(EPORT4->EPDR |= 0x80):(EPORT4->EPDR &= 0x7F);}while(0)

#define EPORT4_IN            (EPORT4->EPPDR)
#define GINT32_IN            (EPORT4->EPPDR&0x01)
#define GINT33_IN            ((EPORT4->EPPDR&0x02)>>1)
#define GINT34_IN            ((EPORT4->EPPDR&0x04)>>2)
#define GINT35_IN            ((EPORT4->EPPDR&0x08)>>3)
#define GINT36_IN            ((EPORT4->EPPDR&0x10)>>4)
#define GINT37_IN            ((EPORT4->EPPDR&0x20)>>5)
#define GINT38_IN            ((EPORT4->EPPDR&0x40)>>6)
#define GINT39_IN            ((EPORT4->EPPDR&0x80)>>7)

/************************************uart define************************************************/
#define GPIO_UART1_RX_OUT(val)      do{(val&0x01)?(SCI1->SCIPORT |= 0x01):(SCI1->SCIPORT &= 0xFE);}while(0)
#define GPIO_UART1_TX_OUT(val)      do{(val&0x01)?(SCI1->SCIPORT |= 0x02):(SCI1->SCIPORT &= 0xFD);}while(0)
#define GPIO_UART1_RX_IN            (SCI1->SCIPORT&0x01)
#define GPIO_UART1_TX_IN            ((SCI1->SCIPORT&0x02)>>1)

#define GPIO_UART2_RX_OUT(val)      do{(val&0x01)?(SCI2->SCIPORT |= 0x01):(SCI2->SCIPORT &= 0xFE);}while(0)
#define GPIO_UART2_TX_OUT(val)      do{(val&0x01)?(SCI2->SCIPORT |= 0x02):(SCI2->SCIPORT &= 0xFD);}while(0)
#define GPIO_UART2_RX_IN            (SCI2->SCIPORT&0x01)
#define GPIO_UART2_TX_IN            ((SCI2->SCIPORT&0x02)>>1)

#define GPIO_UART3_RX_OUT(val)      do{(val&0x01)?(SCI3->SCIPORT |= 0x01):(SCI3->SCIPORT &= 0xFE);}while(0)
#define GPIO_UART3_TX_OUT(val)      do{(val&0x01)?(SCI3->SCIPORT |= 0x02):(SCI3->SCIPORT &= 0xFD);}while(0)
#define GPIO_UART3_RX_IN            (SCI3->SCIPORT&0x01)
#define GPIO_UART3_TX_IN            ((SCI3->SCIPORT&0x02)>>1)

/************************************spi define************************************************/
#define GPIO_SPI1_MISO_OUT(val)      do{(val&0x01)?(SPI1->SPIPORT |= 0x01):(SPI1->SPIPORT &= 0xFE);}while(0)
#define GPIO_SPI1_MOSI_OUT(val)      do{(val&0x01)?(SPI1->SPIPORT |= 0x02):(SPI1->SPIPORT &= 0xFD);}while(0)
#define GPIO_SPI1_SCK_OUT(val)       do{(val&0x01)?(SPI1->SPIPORT |= 0x04):(SPI1->SPIPORT &= 0xFB);}while(0)
#define GPIO_SPI1_SS_OUT(val)        do{(val&0x01)?(SPI1->SPIPORT |= 0x08):(SPI1->SPIPORT &= 0xF7);}while(0)
#define GPIO_SPI1_MISO_IN            (SPI1->SPIPORT&0x01)
#define GPIO_SPI1_MOSI_IN            ((SPI1->SPIPORT&0x02)>>1)
#define GPIO_SPI1_SCK_IN             ((SPI1->SPIPORT&0x04)>>2)
#define GPIO_SPI1_SS_IN              ((SPI1->SPIPORT&0x08)>>3)

#define GPIO_SPI2_MISO_OUT(val)      do{(val&0x01)?(SPI2->SPIPORT |= 0x01):(SPI2->SPIPORT &= 0xFE);}while(0)
#define GPIO_SPI2_MOSI_OUT(val)      do{(val&0x01)?(SPI2->SPIPORT |= 0x02):(SPI2->SPIPORT &= 0xFD);}while(0)
#define GPIO_SPI2_SCK_OUT(val)       do{(val&0x01)?(SPI2->SPIPORT |= 0x04):(SPI2->SPIPORT &= 0xFB);}while(0)
#define GPIO_SPI2_SS_OUT(val)        do{(val&0x01)?(SPI2->SPIPORT |= 0x08):(SPI2->SPIPORT &= 0xF7);}while(0)
#define GPIO_SPI2_MISO_IN            (SPI2->SPIPORT&0x01)
#define GPIO_SPI2_MOSI_IN            ((SPI2->SPIPORT&0x02)>>1)
#define GPIO_SPI2_SCK_IN             ((SPI2->SPIPORT&0x04)>>2)
#define GPIO_SPI2_SS_IN              ((SPI2->SPIPORT&0x08)>>3)

#define GPIO_SPI3_MISO_OUT(val)      do{(val&0x01)?(SPI3->SPIPORT |= 0x01):(SPI3->SPIPORT &= 0xFE);}while(0)
#define GPIO_SPI3_MOSI_OUT(val)      do{(val&0x01)?(SPI3->SPIPORT |= 0x02):(SPI3->SPIPORT &= 0xFD);}while(0)
#define GPIO_SPI3_SCK_OUT(val)       do{(val&0x01)?(SPI3->SPIPORT |= 0x04):(SPI3->SPIPORT &= 0xFB);}while(0)
#define GPIO_SPI3_SS_OUT(val)        do{(val&0x01)?(SPI3->SPIPORT |= 0x08):(SPI3->SPIPORT &= 0xF7);}while(0)
#define GPIO_SPI3_MISO_IN            (SPI3->SPIPORT&0x01)
#define GPIO_SPI3_MOSI_IN            ((SPI3->SPIPORT&0x02)>>1)
#define GPIO_SPI3_SCK_IN             ((SPI3->SPIPORT&0x04)>>2)
#define GPIO_SPI3_SS_IN              ((SPI3->SPIPORT&0x08)>>3)

/************************************i2c define************************************************/
#define GPIO_I2C1_SCL_OUT(val)      do{(val&0x01)?(I2C1->PDR |= 0x01):(I2C1->PDR &= 0xFE);}while(0)
#define GPIO_I2C1_SDA_OUT(val)      do{(val&0x01)?(I2C1->PDR |= 0x02):(I2C1->PDR &= 0xFD);}while(0)
#define GPIO_I2C1_SCL_IN            (I2C1->PDR&0x01)
#define GPIO_I2C1_SDA_IN            ((I2C1->PDR&0x02)>>1)

#define GPIO_I2C2_SCL_OUT(val)      do{(val&0x01)?(I2C2->PDR |= 0x01):(I2C2->PDR &= 0xFE);}while(0)
#define GPIO_I2C2_SDA_OUT(val)      do{(val&0x01)?(I2C2->PDR |= 0x02):(I2C2->PDR &= 0xFD);}while(0)
#define GPIO_I2C2_SCL_IN            (I2C2->PDR&0x01)
#define GPIO_I2C2_SDA_IN            ((I2C2->PDR&0x02)>>1)

#define GPIO_I2C3_SCL_OUT(val)      do{(val&0x01)?(I2C3->PDR |= 0x01):(I2C3->PDR &= 0xFE);}while(0)
#define GPIO_I2C3_SDA_OUT(val)      do{(val&0x01)?(I2C3->PDR |= 0x02):(I2C3->PDR &= 0xFD);}while(0)
#define GPIO_I2C3_SCL_IN            (I2C3->PDR&0x01)
#define GPIO_I2C3_SDA_IN            ((I2C3->PDR&0x02)>>1)

/************************************usi(iso7816) define************************************************/
#define USI1         ((USI_TypeDef *)USI1_BASE_ADDR)
#define USI2         ((USI_TypeDef *)USI2_BASE_ADDR)

#define GPIO_USI1_RST_OUT(val)      do{(val&0x01)?(USI1->USIPDR |= 0x01):(USI1->USIPDR &= 0xFE);}while(0)
#define GPIO_USI1_CLK_OUT(val)      do{(val&0x01)?(USI1->USIPDR |= 0x02):(USI1->USIPDR &= 0xFD);}while(0)
#define GPIO_USI1_DAT_OUT(val)      do{(val&0x01)?(USI1->USIPDR |= 0x04):(USI1->USIPDR &= 0xFB);}while(0)
#define GPIO_USI1_RST_IN            (USI1->USIPDR&0x01)
#define GPIO_USI1_CLK_IN            ((USI1->USIPDR&0x02)>>1)
#define GPIO_USI1_DAT_IN            ((USI1->USIPDR&0x04)>>2)

#define GPIO_USI2_RST_OUT(val)      do{(val&0x01)?(USI2->USIPDR |= 0x01):(USI2->USIPDR &= 0xFE);}while(0)
#define GPIO_USI2_CLK_OUT(val)      do{(val&0x01)?(USI2->USIPDR |= 0x02):(USI2->USIPDR &= 0xFD);}while(0)
#define GPIO_USI2_DAT_OUT(val)      do{(val&0x01)?(USI2->USIPDR |= 0x04):(USI2->USIPDR &= 0xFB);}while(0)
#define GPIO_USI2_RST_IN            (USI2->USIPDR&0x01)
#define GPIO_USI2_CLK_IN            ((USI2->USIPDR&0x02)>>1)
#define GPIO_USI2_DAT_IN            ((USI2->USIPDR&0x04)>>2)

#endif /* GPIO_DRV_H_ */


