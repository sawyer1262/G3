#include "uart_drv.h"
#include "uart.h"
#include "uart_fp.h"
#include "uart_face.h"
#include "uart_back.h"



tUartDriver UartDrv[UART_NUM]={0};

/******************************************************************************/
/*
//串口驱动结构体初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void Uart_DriverInit(void)
{
		//后板
		UartDrv[UART_BACK_INDEX].StrInit=UartBack_DatStrInit;
		UartDrv[UART_BACK_INDEX].HardInit=UartBack_Init;
		UartDrv[UART_BACK_INDEX].HardDeinit=UartBack_Deinit;
		UartDrv[UART_BACK_INDEX].RxFrameOP=UartBack_RxDataHandle;
		UartDrv[UART_BACK_INDEX].RtxTimeoutOP=UartBack_RtxTimeoutoutHandle;
		UartDrv[UART_BACK_INDEX].SendNBytes=UartBack_SendNbytes;
		UartDrv[UART_BACK_INDEX].SendQueueOP=UartBack_DataSendHandle;
		UartDrv[UART_BACK_INDEX].StrInit();        //结构体初始化
		//人脸	  
//		UartDrv[UART_FACE_INDEX].StrInit=UartFace_DatStrInit;
//		UartDrv[UART_FACE_INDEX].HardInit=UartFace_Init;
//		UartDrv[UART_FACE_INDEX].HardDeinit=UartFace_Deinit;
//		UartDrv[UART_FACE_INDEX].RxFrameOP=UartFace_RxDataHandle;
//		UartDrv[UART_FACE_INDEX].RtxTimeoutOP=UartFace_RtxTimeoutoutHandle;    
//		UartDrv[UART_FACE_INDEX].SendNBytes=UartFace_SendNbytes;
//		UartDrv[UART_FACE_INDEX].SendQueueOP=UartFace_DataSendHandle;
//		UartDrv[UART_FACE_INDEX].StrInit();        //结构体初始化
		//指纹
		UartDrv[UART_FP_INDEX].StrInit=UartFp_DatStrInit;
		UartDrv[UART_FP_INDEX].HardInit=UartFp_Init;
		UartDrv[UART_FP_INDEX].HardDeinit=UartFp_Deinit;
		UartDrv[UART_FP_INDEX].RxFrameOP=UartFp_RxDataHandle;
		UartDrv[UART_FP_INDEX].RtxTimeoutOP=UartFp_RtxTimeoutoutHandle;
		UartDrv[UART_FP_INDEX].SendNBytes=UartFp_SendNbytes;
		UartDrv[UART_FP_INDEX].SendQueueOP=UartFp_DataSendHandle;
		UartDrv[UART_FP_INDEX].StrInit();        //结构体初始化
}
/******************************************************************************/
/*
//串口初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void Uart_Init(void)
{
		uint8_t i=0;
	
		for(i=0;i<UART_NUM;i++)
		{
				if(UartDrv[i].HardInit!=0)
				{
						UartDrv[i].HardInit();       //硬件初始化
				}
		}
}
/******************************************************************************/
/*
//串口除能
input:   none
output   none
return   none
*/
/******************************************************************************/
void Uart_Deinit(void)
{
		uint8_t i=0;
	
		for(i=0;i<UART_NUM;i++)
		{
				if(UartDrv[i].HardDeinit!=0)
				{
						UartDrv[i].HardDeinit();       //硬件初始化
				}
		}
}
/******************************************************************************/
/*
//串口驱动处理
input:   none
output   none
return   none
*/
/******************************************************************************/
void Uart_RtxFrameHandle(void)
{
		uint8_t i=0;
	
		for(i=0;i<UART_NUM;i++)
		{
				if(UartDrv[i].SendQueueOP!=0)
				{
						UartDrv[i].SendQueueOP();           //发送队列处理
				}
				if(UartDrv[i].RxFrameOP!=0)
				{
						UartDrv[i].RxFrameOP();             //接收数据帧处理
				}
				if(UartDrv[i].RtxTimeoutOP!=0)
				{
						UartDrv[i].RtxTimeoutOP();         //发送接收超时处理
				}
		}
}























