// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : uart_demo.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __UART_DEMO_H__
#define __UART_DEMO_H__

#define SCI_MODE_TEST		1
#define SCI_GINT_SWAP_EN		0


#define DMA_MAX_RECV_LEN	4095 //DMA 最大传输数据为0xFFF

#define SCI_DEMO_BUF_LENGTH		32
extern void UART_Demo(void);

#endif /* UART_DEMO_H_ */
