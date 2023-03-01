#include "uart_drv.h"
#include "uart_face.h" 
#include "uart.h"
#include "timer.h"
#include "i2c_drv.h"
#include <string.h>
#include "audio.h" 
#include "uart_back.h"
#include "keypad_menu.h"
#include "lpm.h"
#include "uart_fp.h"
#include "led.h"
#include "lock_config.h"
#include "rtc.h"
#include "lock_record.h"
#include "delay.h"


OrbFaceAckStr_t OrdFace;
UartTxStr_t OrbFaceUartTxQ[ORB_FACE_TX_Q_SIZE];
uint8_t OrbFaceCurIdx=0;

FaceUserStr_t FaceUser={0,{0}};

FaceIdy_t FaceIdy={FACE_IDLE,0,0}; 
FaceAdd_t FaceAdd={FACEADD_READUSER,0,0};
FaceDel_t FaceDel={FACEDEL_READUSER,0,0};
FaceReady_t FaceReady={0};
uint8_t FaceIdyId=0;
uint8_t FaceIdyLock=0;


/******************************************************************************/
/*
//人脸串口结构初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_DatStrInit(void)
{
		UartStruct[UART_FACE_INDEX].pTxStart=0;
		UartStruct[UART_FACE_INDEX].pTxEnd=0;
		UartStruct[UART_FACE_INDEX].rp=UartStruct[UART_FACE_INDEX].wp=0;
		UartStruct[UART_FACE_INDEX].RxStat=RX_IDLE;
		UartStruct[UART_FACE_INDEX].TxStat=TX_IDLE;
		UartStruct[UART_FACE_INDEX].RtxTimeout=0;
		UartStruct[UART_FACE_INDEX].TxTime=0;
		UartStruct[UART_FACE_INDEX].RxTime=0;
	
		OrbFaceCurIdx=0xFF;
	
		I2C_ConfigGpio(I2C3,FACE_VCC_PIN,GPIO_OUTPUT);
		DelayMS(5); 
		I2C_WriteGpioData(I2C3,FACE_VCC_PIN,Bit_SET);
		printf("face power on:0x%08x\n",GetTimerCount());
}

/******************************************************************************/
/*
//人脸串口发送队列复位
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_TxQueueReset(void)
{
		uint8_t i=0;
		
		for(i=0;i<ORB_FACE_TX_Q_SIZE;i++)
		{
				OrbFaceUartTxQ[i].DatStat=0;
		}
		OrbFaceCurIdx=0xFF;
}
/******************************************************************************/
/*
//人脸串口初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_Init(void)
{
    UART_InitTypeDef UART_InitStruct;
	//init uart
    UART_InitStruct.UART_BaudRate = 115200;
    UART_InitStruct.UART_Mode = UART_INT_MODE;//采用CPU中断方式
		UART_InitStruct.UART_Parity = UART_PARITY_NONE;
		UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_10BIT;
    UART_InitStruct.UART_StopBits = 1;
//  //UART_StopBits = 2
//    UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_11BIT;
//    UART_InitStruct.UART_StopBits = 2;
		UartStruct[UART_FACE_INDEX].RtxTimeout=10;
		UART_Init(UART_FACE_SCI, &UART_InitStruct);
}

/******************************************************************************/
/*
//打开人脸模块电源
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_PowerOn(void)
{
		if(I2C_ReadGpioData(I2C3,FACE_VCC_PIN)==Bit_RESET)
		{
				I2C_ConfigGpio(I2C3,FACE_VCC_PIN,GPIO_OUTPUT);
				DelayMS(5); 
				I2C_WriteGpioData(I2C3,FACE_VCC_PIN,Bit_SET);
		}
}
/******************************************************************************/
/*
//关闭人脸模块电源
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_PowerOff(void)
{
		if(I2C_ReadGpioData(I2C3,FACE_VCC_PIN)==Bit_SET)       //待测试
		{
				I2C_ConfigGpio(I2C3,FACE_VCC_PIN,GPIO_OUTPUT);
				DelayMS(5); 
				I2C_WriteGpioData(I2C3,FACE_VCC_PIN,Bit_RESET);
		}
}
/******************************************************************************/
/*
//复位人脸模块电源
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_PowerReset(void)
{
		I2C_ConfigGpio(I2C3,FACE_VCC_PIN,GPIO_OUTPUT);
		DelayMS(5); 
		I2C_WriteGpioData(I2C3,FACE_VCC_PIN,Bit_RESET);
		DelayMS(80); 
		I2C_WriteGpioData(I2C3,FACE_VCC_PIN,Bit_SET);
}
/******************************************************************************/
/*
//人脸模块除能
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_Deinit(void)
{
		//关电源
		I2C_ConfigGpio(I2C3,FACE_VCC_PIN,GPIO_OUTPUT);
		DelayMS(5); 
		I2C_WriteGpioData(I2C3,FACE_VCC_PIN,Bit_RESET);
		//IO输出低
		UART_ConfigGpio(UART_FACE_SCI,UART_RX,GPIO_OUTPUT);
		UART_ConfigGpio(UART_FACE_SCI,UART_TX,GPIO_OUTPUT);
		UART_WriteGpioData(UART_FACE_SCI,UART_RX,Bit_RESET);
		UART_WriteGpioData(UART_FACE_SCI,UART_TX,Bit_RESET);
}
/******************************************************************************/
/*
//生成校验
input:   buf -- 需要生成校验的数据
         len -- 数据长度
output   none
return   16位校验数据
*/
/******************************************************************************/
uint16_t GenFaceChksum(uint8_t* buf,uint16_t len)
{
		uint32_t dat=0x10000;
		uint16_t i=0;
		for(i=0;i<len;i++)
		{
				dat-=buf[i];
		}
		return (dat&0xffff);
}
/******************************************************************************/
/*
//人脸串口发送数据
input:   buf -- 待发送的数据
         len -- 数据长度
output   none
return   none
*/
/******************************************************************************/
void UartFace_SendNbytes(uint8_t* buf,uint16_t len)      
{
		if(UartStruct[UART_FACE_INDEX].TxStat!=TX_IDLE)return;
		
		UartStruct[UART_FACE_INDEX].TxStat=TX_RUNNING;
		UartStruct[UART_FACE_INDEX].pTxStart=buf;
		UartStruct[UART_FACE_INDEX].pTxEnd=buf+len;
		UartStruct[UART_FACE_INDEX].TxTime=GetTimerCount();
		UART_FACE_SCI->SCIDRL=*UartStruct[UART_FACE_INDEX].pTxStart++;
		UART_FACE_SCI->SCICR2|=SCICR2_TIE_MASK;  
}
/******************************************************************************/
/*
//人脸串口发送指令
input:   type -- 指令类型：见协议文档
         buf -- 指令附加数据
         len -- 指令附加数据长度
				 ack -- 发送指令是否需要应答：1-需要， 0-不需要， 2-需要立即发送的应答
         timeout -- 等待应答超时时间，单位为毫秒
output   none
return   none 
*/
/******************************************************************************/
void UartFace_SendCmdData(ObFrameType_t type,uint8_t *buf,uint8_t len,uint8_t ack,uint32_t timeout)
{
		uint8_t i=0,j=0;
		uint16_t chksum=0;
	
		for(i=0;i<ORB_FACE_TX_Q_SIZE;i++)
		{
				if(OrbFaceUartTxQ[i].DatStat==0)
				{
						OrbFaceUartTxQ[i].DatBuf[ORB_HEAD_IDX]=ORB_HEAD_8_L;
						OrbFaceUartTxQ[i].DatBuf[ORB_HEAD_IDX+1]=ORB_HEAD_8_H;
						OrbFaceUartTxQ[i].DatBuf[ORB_SRC_IDX]=HOST;
						OrbFaceUartTxQ[i].DatBuf[ORB_TYPE_IDX]=type;
						OrbFaceUartTxQ[i].DatBuf[ORB_LEN_IDX]=len;
						OrbFaceUartTxQ[i].DatBuf[ORB_LEN_IDX+1]=0x00;
						for(j=0;j<len;j++)
						{
								OrbFaceUartTxQ[i].DatBuf[ORB_DAT_IDX+j]=buf[j];
						}
						chksum=GenFaceChksum(OrbFaceUartTxQ[i].DatBuf,len+6);
						OrbFaceUartTxQ[i].DatBuf[ORB_DAT_IDX+j++]=chksum;
						OrbFaceUartTxQ[i].DatBuf[ORB_DAT_IDX+j]=chksum>>8;
						
						OrbFaceUartTxQ[i].DatBufLen=len+8;
						OrbFaceUartTxQ[i].DatStat=1;
						OrbFaceUartTxQ[i].NeedAck=ack;
						OrbFaceUartTxQ[i].Timeout=timeout;
						OrbFaceUartTxQ[i].Retry=0;
						return;
				}
		}
}
/******************************************************************************/
/*
//人脸串口数据发送处理
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_DataSendHandle(void)
{
		uint8_t i=0;
		
		if(UartStruct[UART_FACE_INDEX].TxStat!=TX_IDLE)return;
		
		for(i=0;i<ORB_FACE_TX_Q_SIZE;i++)
		{
				if(OrbFaceUartTxQ[i].DatStat==1 && OrbFaceUartTxQ[i].NeedAck==2)        
				{
						UartFace_SendNbytes(OrbFaceUartTxQ[i].DatBuf,OrbFaceUartTxQ[i].DatBufLen);
						OrbFaceUartTxQ[i].DatStat=0;
						OrbFaceCurIdx=0xFF;
						return;
				}
		}
		
		if(OrbFaceCurIdx<ORB_FACE_TX_Q_SIZE)
		{
				if(GetTimerElapse(OrbFaceUartTxQ[OrbFaceCurIdx].TxTime)<OrbFaceUartTxQ[OrbFaceCurIdx].Timeout)return;
				
				UartFace_SendNbytes(OrbFaceUartTxQ[OrbFaceCurIdx].DatBuf,OrbFaceUartTxQ[OrbFaceCurIdx].DatBufLen);
				OrbFaceUartTxQ[OrbFaceCurIdx].TxTime=GetTimerCount();
				printf("face tx retry cmd: %02x\n",OrbFaceUartTxQ[OrbFaceCurIdx].DatBuf[ORB_DAT_IDX]);
				if(++OrbFaceUartTxQ[OrbFaceCurIdx].Retry>=3)
				{
						OrbFaceUartTxQ[OrbFaceCurIdx].DatStat=0;
						OrbFaceCurIdx=0xFF;
				}
				return;
		}
		
		for(i=0;i<ORB_FACE_TX_Q_SIZE;i++)
		{
				if(OrbFaceUartTxQ[i].DatStat==1)
				{
						UartFace_SendNbytes(OrbFaceUartTxQ[i].DatBuf,OrbFaceUartTxQ[i].DatBufLen);
						if(OrbFaceUartTxQ[i].NeedAck==1)
						{
								OrbFaceUartTxQ[i].TxTime=GetTimerCount();
								OrbFaceUartTxQ[i].Retry++;
								OrbFaceCurIdx=i;
						}
						else
						{
								OrbFaceUartTxQ[i].DatStat=0;
								OrbFaceCurIdx=0xFF;
						}
						return;
				}
		}
}
/******************************************************************************/
/*
//接收到的人脸串口数据处理
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_RxDataHandle(void)
{
#ifdef UART_FACE_TEST	  
		static 
#endif
		uint8_t pBufRec[UART_RECV_MAX_LEN]={0};   
		uint8_t *pFrameHead=0;
		uint16_t i=0,datlen=0,pktchksum=0,pktLen=0;
		
		if(UartStruct[UART_FACE_INDEX].RxFrameOk==0)return;
		
		UartStruct[UART_FACE_INDEX].RxFrameOk=0;
	
		while(UartStruct[UART_FACE_INDEX].wp != UartStruct[UART_FACE_INDEX].rp)     //读出所以数据,并调整定位数据帧与帧长度
		{
				pBufRec[i++]=UartStruct[UART_FACE_INDEX].dat[UartStruct[UART_FACE_INDEX].rp++];
				if(UartStruct[UART_FACE_INDEX].rp>=UART_RECV_MAX_LEN)
				{
						UartStruct[UART_FACE_INDEX].rp=0;
				}
		}	
		datlen=i;
		//UartFace_SendNbytes(pBufRec,i);
		for(i=0;i<datlen;i++)
		{
				if(pBufRec[i]==ORB_HEAD_8_L)
				{
						if(i+1<datlen && pBufRec[i+1]==ORB_HEAD_8_H)
						{
								break;
						}
				}
		}
		datlen-=i;
#ifdef UART_FACE_TEST	
		UartFace_SendNbytes(pFrameHead,datlen);    //接收发送测试
#endif
		if(datlen<8)return;
		pFrameHead=&pBufRec[i];
		pktLen=((uint16_t)pFrameHead[5]<<8)|pFrameHead[4];
		pktchksum=((uint16_t)pFrameHead[pktLen+7]<<8)|pFrameHead[pktLen+6];
		
		if(pFrameHead[ORB_SRC_IDX]!=SLAVE)return;     
	
		if(pktchksum!=GenFaceChksum(pFrameHead,pktLen+6))return;
		
//		for(i=0;i<datlen;i++)
//		{
//				printf("rx data: 0x%02x\n",*pFrameHead++);
//		}
		
		if(OrbFaceCurIdx<ORB_FACE_TX_Q_SIZE      //有消息等待应答
			&& OrbFaceUartTxQ[OrbFaceCurIdx].DatBuf[ORB_TYPE_IDX]==pFrameHead[ORB_TYPE_IDX]  //消息类型一致
			&& OrbFaceUartTxQ[OrbFaceCurIdx].DatBuf[ORB_DAT_IDX]==pFrameHead[ORB_DAT_IDX])   //消息命令一致
		{
				OrbFaceUartTxQ[OrbFaceCurIdx].DatStat=0;
				OrbFaceCurIdx=0xFF;
				printf("ack ok: 0x%02x\n",pFrameHead[ORB_DAT_IDX]);
		}
		switch(pFrameHead[ORB_TYPE_IDX])                //包类型
		{
				case CMD:           //控制命令返回
						switch(pFrameHead[ORB_DAT_IDX])         //类型下的命令
						{
								case 0x57:               //添加人脸中间状态，不处理
//										if(OrbFaceCurIdx<ORB_FACE_TX_Q_SIZE                                                //有消息等待应答
//											&& OrbFaceUartTxQ[OrbFaceCurIdx].DatBuf[ORB_TYPE_IDX]==pFrameHead[ORB_TYPE_IDX]  //消息类型一致
//											&& OrbFaceUartTxQ[OrbFaceCurIdx].DatBuf[ORB_DAT_IDX]==0x01)                      //添加人脸，中间状态视为有应答
//										{
//												OrbFaceUartTxQ[OrbFaceCurIdx].DatStat=0;
//												OrbFaceCurIdx=0xFF;
//										}
										break;
								case 0x02:               //删除人脸
								case 0x01:               //添加人脸
									//	printf("frame 0x%02x,0x%02x",pFrameHead[ORB_DAT_IDX+1],pFrameHead[ORB_DAT_IDX+2]);
								case 0x40:               //识别人脸
										OrdFace.Ack=1;
										OrdFace.Cmd=pFrameHead[ORB_DAT_IDX];     //指令
										OrdFace.Stat=pFrameHead[ORB_DAT_IDX+1];  //状态
										OrdFace.ErrC=pFrameHead[ORB_DAT_IDX+2];  //错误码
										break;
								case 0x55:               //系统准备好，从设备上电开始，大概300MS  
										if(pFrameHead[ORB_DAT_IDX+1]==0x00)
										{
												printf("face ready!\n");
												FaceReady.Ready=1;
												FaceAdd.StTime=0;
												FaceDel.StTime=0;
												FaceIdy.StTime=0;
										}
										break;
								default:break;
						}
						break;
				case QUERY:           //查询返回
						OrdFace.Ack=1;
						OrdFace.Cmd=pFrameHead[ORB_DAT_IDX];
						switch(pFrameHead[ORB_DAT_IDX])
						{
								case 0x50:               //查询所有ID
										FaceUser.Sum=pktLen-1;                    //存储的人脸数目
										for(i=0;i<FaceUser.Sum;i++)
										{
												FaceUser.IndexTab[pFrameHead[ORB_DAT_IDX+1+i]/8]|=(1<<(pFrameHead[ORB_DAT_IDX+1+i]%8));
										}
										for(i=0;i<13;i++)
										{
												printf("query: %02x",FaceUser.IndexTab[i]);
										}
										break;
								//此类信息，可用于APP显示
								case 0x64:              //查询芯片ID
								case 0x60:              //查询硬件版本
								case 0x61:              //查询软件版本
								case 0x62:              //查询摄像头版本
								default:break;
						}
						break;
				default:break;
		}
}
/******************************************************************************/
/*
//数据发送接收超时处理
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFace_RtxTimeoutoutHandle(void)
{
		if(UartStruct[UART_FACE_INDEX].RxTime>0 && GetTimerElapse(UartStruct[UART_FACE_INDEX].RxTime)>UartStruct[UART_FACE_INDEX].RtxTimeout)
		{
				UartStruct[UART_FACE_INDEX].RxTime=0;
				UartStruct[UART_FACE_INDEX].RxStat=RX_IDLE;
				UartStruct[UART_FACE_INDEX].RxFrameOk=1;
		}
		if(UartStruct[UART_FACE_INDEX].TxTime>0 && GetTimerElapse(UartStruct[UART_FACE_INDEX].TxTime)>UartStruct[UART_FACE_INDEX].RtxTimeout)   
		{
				UartStruct[UART_FACE_INDEX].TxTime=0;
				UartStruct[UART_FACE_INDEX].TxStat=TX_IDLE;
				UART_FACE_SCI->SCICR2&=~SCICR2_TIE_MASK;  
		}
}

/******************************************************************************/
/*
//人脸模块扫描人脸
input:   none
output   none
return   none 
*/
/******************************************************************************/
void OrbFaceScanHandle(void)
{
		uint8_t temp[10]={0};
		
		if(SysConfig.Bits.FaceLock==1 || SysConfig.Bits.SupPowSave==1)
		{	
				UartFace_PowerOff();                                   //关掉电源
				return;//错误次数过多或超省电模式人脸锁定
		}
		if(LockStat.Lock==1 || FaceIdyLock==1)return;  //门已开或者虚掩，都不再检测人脸
		
		switch(FaceIdy.FsStat)
		{
				case FACE_IDLE:
						if(FaceReady.IdyReady!=1)
						{
								if(FaceIdy.StTime==0)
								{
										FaceIdy.StTime=GetTimerCount();
								}
								else if(GetTimerElapse(FaceIdy.StTime)>1000)    //超时强制退出
								{
										FaceReady.IdyReady=1;
										FaceIdy.StTime=0;
								}
								break;             //未准备好
						}
						FaceIdy.StTime=0;
						FaceIdy.IdyRetry=0;
						FaceIdy.FsStat=FACE_CHECK_SENSOR;
						break;
				case FACE_CHECK_SENSOR:
						if(FaceIdy.StTime==0)             //发送命令
						{
								temp[0]=0x50;
								UartFace_SendCmdData(QUERY,temp,1,1,1000);
								OrdFace.Ack=0;
								FaceIdy.StTime=GetTimerCount();
						}
						else if(OrdFace.Ack==1)   //接收到数据
 						{
								FaceIdy.StTime=0;
								if(OrdFace.Cmd==0x50)
								{
										printf("face sum: %02d \n",FaceUser.Sum);
										FaceIdy.IdyRetry=0;
										FaceIdy.FsStat=FACE_IDY;
								}
						}
						else if(GetTimerElapse(FaceIdy.StTime)>=3500)
						{
								FaceIdy.StTime=0;
								if(++FaceIdy.IdyRetry>=ORB_TX_FAIL_RTY)
								{
										FaceIdy.IdyRetry=0;
										FaceIdy.FsStat=FACE_IDLE;
										FaceIdyLock=1;                      //模块异常
								}
						}
						break;
				case FACE_IDY:
						if(FaceIdy.StTime==0)      
						{
								printf("face start \n");
								temp[0]=0x40;temp[1]=0x05;
								UartFace_SendCmdData(CMD,temp,2,1,8000);
								OrdFace.Ack=0;
								FaceIdy.StTime=GetTimerCount();
						}
						else if(OrdFace.Ack==1)
						{
								OrdFace.Ack=0;
								FaceIdy.StTime=0;
								if(OrdFace.Cmd==0x40 && OrdFace.Stat==0x01)     //识别成功
								{
										FaceIdyId=OrdFace.ErrC;                 //识别ID
										printf("face idy id: 0x%02x\n",FaceIdyId);
										LockUpdatePeripLockStat(PERIP_FACE_INDEX,0);
										FaceIdy.IdyRetry=0;    //复位失败次数
										FaceIdyLock=1;
										UartFace_PowerOff();        //关人脸 
										
										if(SysConfig.Bits.MutiIdy==1)                 //双重验证
										{
												if(MutiIdyItem&(~IDY_FACE))        //非人脸验证成功+人脸验证成功
												{
														MutiIdyItem=0;                 //复位双重验证 
														PirLed_SetStat(LOGO_OPEN_SUCCESS);
														AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
														UserControlLock(CTL_OPEN_LOCK,FACE_OPEN,FaceIdyId);
												}
												else
												{
														MutiIdyItem|=IDY_FACE;
														PirLed_SetStat(LOGO_OPEN_SUCCESS_MUTIL);
												}
												printf("face muti idy ok \n");
										}
										else
										{
												printf("face idy ok \n");
												AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
												PirLed_SetStat(LOGO_OPEN_SUCCESS);
												UserControlLock(CTL_OPEN_LOCK,FACE_OPEN,FaceIdyId);
										}
								}
								else if(OrdFace.Cmd==0x40)                  //识别失败，立马重新识别    
								{
										if(++FaceIdy.IdyRetry>=3)      //识别三次失败关人脸
										{
												FaceIdy.IdyRetry=0;
												FaceIdyLock=1;
												UartFace_PowerOff();        //关人脸
										}
										else if(OrdFace.ErrC==ORBBEC_ERR_NOT_IN_DB)            
										{
												if(SysConfig.Bits.Activate==0)
												{
														FaceIdyLock=1;
														AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"请激活产品"),UNBREAK);
													
														PirLed_SetStat(LOGO_OPEN_SUCCESS);
														UserControlLock(CTL_OPEN_LOCK,FACE_OPEN,0XFF);
												}
												else 
												{
														PirLed_SetStat(LOGO_OPEN_FAIL);
														AudioPlayVoice(GetVolIndex("检卡音"),BREAK);               //验证失败
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														LockUpdatePeripLockStat(PERIP_FACE_INDEX,1);
												}
										}
								}
						}
						break;
		 }
}

/******************************************************************************/
/*
//人脸模块添加人脸
input:   type -- 人脸类型：ADD_ADMIN -- 管理人脸    ADD_NORMAL -- 普通人脸
output   none
return   
				 WAIT_RESP=0,                    //等待应答
				 OP_READY,                       //模块准备号
				 OP_SUCCESS,                     //添加成功
				 OP_FULL,                        //库已满
				 OP_FAIL,                        //添加失败
				 OP_TIMEOUT                      //操作超时  
*/
/******************************************************************************/
uint8_t FaceAddHandle(uint8_t type)
{
		uint8_t temp[5]={0};
		uint8_t retDat=0xFF;	
		
		switch(FaceAdd.AddStat)
		{
				case FACEADD_READUSER:
						if(FaceReady.AddReady!=1)
						{
								if(FaceAdd.StTime==0)
								{
										FaceAdd.StTime=GetTimerCount();
								}
								else if(GetTimerElapse(FaceAdd.StTime)>1000)    //超时强制退出
								{
										FaceReady.AddReady=1;
										FaceAdd.StTime=0;
								}
								break;             //未准备好
						}
						if(FaceAdd.StTime==0)
						{
								temp[0]=0x50;
								UartFace_SendCmdData(QUERY,temp,1,1,1000);
								OrdFace.Ack=0;
								FaceAdd.StTime=GetTimerCount();
								printf("read user\n");
						}
						else if(OrdFace.Ack==1)
						{
								FaceAdd.StTime=0;
								OrdFace.Ack=0;
								if(OrdFace.Cmd==0x50)
								{
										printf("face sum: %02d \n",FaceUser.Sum);
										FaceAdd.AddStat=FACEADD_READY;
										FaceAdd.Retry=0;
								}
								else
								{
										printf("rev cmd: 0x%02x\n",OrdFace.Cmd);  
								}
						}
						else if(GetTimerElapse(FaceAdd.StTime)>3500)
						{
								FaceAdd.StTime=0;
								if(++FaceAdd.Retry>=ORB_TX_FAIL_RTY)      //3*3
								{
										FaceAdd.Retry=0;
										FaceAdd.AddStat=FACEADD_TIMEOUT;
								}
						}
						break;
				case FACEADD_RUN:
						if(FaceAdd.StTime==0)
						{
								temp[0]=0x01;
								temp[1]=GetOrbFaceFreeSlot();
								if(ADD_ADMIN==type)temp[1]=0;
								UartFace_SendCmdData(CMD,temp,2,1,20000);
								FaceAdd.StTime=GetTimerCount();
								FaceAdd.Retry=0;
								OrdFace.Ack=0;
								printf("add start");
						}
						else if(OrdFace.Ack==1)
						{
								printf("add ret:0x%02x",OrdFace.Stat);
								FaceAdd.StTime=0;
								if(OrdFace.Cmd==0x01 && OrdFace.Stat==0x01)  //添加成功
								{
										LockAddUser(PERIP_FACE_INDEX,OrdFace.ErrC);
											
										OrbFaceUpdateUser(1,OrdFace.ErrC);
										FaceAdd.AddStat=FACEADD_OK;
								}
								else if(OrdFace.Cmd==0x01)
								{
										if(++FaceAdd.Retry>=5)
										{
												FaceAdd.Retry=0;
												FaceAdd.AddStat=FACEADD_FAIL;
										}
								}
						}
						else if(GetTimerElapse(FaceAdd.StTime)>50000)
						{
								printf("add timeout");
								FaceAdd.StTime=0;
								FaceAdd.AddStat=FACEADD_FAIL;
						}
						break;
				case FACEADD_READY:
						retDat=OP_READY;
						break;
				case FACEADD_OK:
						retDat=OP_SUCCESS;
						break;
				case FACEADD_FAIL:
						retDat=OP_FAIL;
						break;
				case FACEADD_TIMEOUT:
						retDat=OP_TIMEOUT;
						break;
		}
		return retDat;
}

/******************************************************************************/
/*
//人脸模块删除人脸
input:   id -- 人脸ID
output   none
return   
				 WAIT_RESP=0,                    //等待应答
				 OP_READY,                       //模块准备号
				 OP_SUCCESS,                     //操作成功
				 OP_FULL,                        //库已满
				 OP_FAIL,                        //操作失败
				 OP_TIMEOUT                      //操作超时  
*/
/******************************************************************************/
uint8_t FaceDelHandle(uint8_t id)
{
		uint8_t temp[10]={0};
		uint8_t retDat=0xFF;
		
		switch(FaceDel.DelStat)
		{
				case FACEDEL_READUSER:
						if(FaceReady.DelReady!=1)
						{
								if(FaceDel.StTime==0)
								{
										FaceDel.StTime=GetTimerCount();
								}
								else if(GetTimerElapse(FaceDel.StTime)>1000)    //超时强制退出
								{
										FaceReady.DelReady=1;
										FaceDel.StTime=0;
								}
								break;             //未准备好
						}
						if(FaceDel.StTime==0)
						{
								temp[0]=0x50;
								UartFace_SendCmdData(QUERY,temp,1,1,1000);
								OrdFace.Ack=0;
								FaceDel.StTime=GetTimerCount();
							
								retDat=WAIT_RESP;
								printf("read user\n");
						}
						else if(OrdFace.Ack==1)
						{
								FaceDel.StTime=0;
								if(OrdFace.Cmd==0x50)
								{
										printf("face sum: %02d \n",FaceUser.Sum);
										if(id==0xFF)
										{
												FaceDel.DelStat=FACEDEL_RUN;
												FaceDel.Retry=0;
										}
										else if((FaceUser.IndexTab[id/8]&(1<<(id%8)))==0) //无效ID
										{
												FaceDel.DelStat=FACEDEL_OK;
												FaceDel.Retry=0;
										}
										else 
										{
												FaceDel.DelStat=FACEDEL_RUN;
												FaceDel.Retry=0;
										}
								}
						}
						else if(GetTimerElapse(FaceDel.StTime)>3500)      
						{
								FaceDel.StTime=0;
								retDat=WAIT_RESP;
								printf("face READ TIMEOUT \n");
								if(++FaceDel.Retry>=3)   //3*3次通讯失败，报timeout
								{
										FaceDel.Retry=0;
										FaceDel.DelStat=FACEDEL_TIMEOUT;
								}
						}
						break;
				case FACEDEL_RUN:
						if(FaceDel.StTime==0)
						{
								printf("face del id:0x%02x\n",id);
								temp[0]=0x02;temp[1]=id;
								UartFace_SendCmdData(CMD,temp,2,1,1000);
								LockDelUser(PERIP_FACE_INDEX,id);
								
								OrdFace.Ack=0;
								FaceDel.Retry=0;
								FaceDel.StTime=GetTimerCount();
								retDat=WAIT_RESP;
						}
						else if(OrdFace.Ack==1)
						{
								FaceDel.StTime=0;
								printf("face del: 0x%02x 0x%02x \n",OrdFace.Cmd,OrdFace.Stat);
								if(OrdFace.Cmd==0x02 && OrdFace.Stat==0x01)
								{
										OrbFaceUpdateUser(0,id);
										FaceDel.DelStat=FACEDEL_OK;
								}
								else if(OrdFace.Cmd==0x02)
								{
										if(++FaceDel.Retry>=5)
										{
												FaceDel.Retry=0;
												FaceDel.DelStat=FACEDEL_FAIL;
										}
								}
						}
						else if(GetTimerElapse(FaceDel.StTime)>3500)
						{
								FaceDel.StTime=0;
								FaceDel.DelStat=FACEDEL_FAIL;
						}
						break;
				case FACEDEL_OK:
						retDat=OP_SUCCESS;
						break;
				case FACEDEL_FAIL:
						retDat=OP_FAIL;
						break;
				case FACEDEL_TIMEOUT:
						retDat=OP_TIMEOUT;
						break;
		}
		return retDat;
}
/******************************************************************************/
/*
//获取可用得人脸SLOT
input:   none
output   none
return   空闲SLOT
*/
/******************************************************************************/
uint8_t GetOrbFaceFreeSlot(void)
{
		uint8_t i=0,j=0;
	
		for(i=0;i<13;i++)
		{
				for(j=0;j<8;j++)
				{
						if((FaceUser.IndexTab[i]&(1<<j))==0)
						{
								return (i*8+j);
						}
				}
		}
		return (i*8);     //13*8=104
}
/******************************************************************************/
/*
//更新人脸用户
input:   type -- 更新类型：1-添加 0-删除
         id -- 更新的ID  删除时FF为删除所有
output   none
return   none
*/
/******************************************************************************/
void OrbFaceUpdateUser(uint8_t type,uint8_t id)
{
		if(id==0 || (id>ORB_FACE_MAX && id!=0xFF))return;
		
		if(id==0xFF && type==0) //删除所有
		{
				memset(FaceUser.IndexTab,0,13);
				FaceUser.Sum=0;
				return;
		}
		else if(id==0xFF)       //出错，无添加所有操作，退出
		{
				return;
		}
		if(type)                //添加人脸
		{
				FaceUser.IndexTab[id/8]|=(1<<id%8);
				FaceUser.Sum+=1;
		}
		else if(FaceUser.Sum>0) //删除人脸
		{
				FaceUser.IndexTab[id/8]&=~(1<<id%8);
				FaceUser.Sum-=1;
		}
}












