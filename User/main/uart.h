#ifndef  __UART_H_
#define  __UART_H_


#include <stdint.h>


#define     UART_NUM            3
#define     UART_TX_MAX_LEN     100

typedef enum{
		ADD_NORMAL=0,                   //添加普通人脸
		ADD_ADMIN,                       //添加管理员
		ADD_COERCE
}AddType_t;


typedef struct sUartDriver
{
		void ( *StrInit )( void );                             //结构体初始化
    void ( *HardInit )( void );                            //硬件IO初始化
    void ( *HardDeinit )( void );                          //硬件IO低功耗处理
		void ( *SendNBytes )(uint8_t* buf,uint16_t len);       //发送指定长度数据 
		void ( *RxFrameOP )(void);                             //接收数据处理   
		void ( *RtxTimeoutOP )(void);                          //收发超时检测
		void ( *SendQueueOP )(void);                           //发送队列处理  
}tUartDriver;

typedef struct{
		uint8_t DatStat;                  //数据状态：0-无数据，1-有数据待发送
		uint8_t Retry;                    //重发次数
		uint8_t NeedAck;                  //是否需要应答
		uint16_t DatBufLen;               //发送数据长度 
		uint32_t Timeout;                 //超时时间设定值 
		uint32_t TxTime;                  //数据发送时间
		uint8_t DatBuf[UART_TX_MAX_LEN];  //发送数据缓存
}UartTxStr_t;


void Uart_Init(void);
void Uart_DriverInit(void);
void Uart_Deinit(void);
void Uart_RtxFrameHandle(void);














#endif

