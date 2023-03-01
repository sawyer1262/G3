#include "uart_drv.h"
#include "uart_fp.h"
#include "i2c_drv.h"
#include "uart.h"
#include "timer.h"
#include "debug.h"
#include "audio.h"
#include "key.h"
#include "keypad_menu.h"
#include "eport_drv.h"
#include "uart_back.h"
#include "lpm.h"
#include "delay.h"
#include "string.h"
#include "led.h"
#include "lock_config.h"
#include "rtc.h"
#include "bat.h"
#include "wdt_drv.h"
#include "lock_record.h"


LdFpStr_t LdFp;
UartTxStr_t FpUartTxQ[FP_TX_Q_SIZE];
uint8_t LastCmd=0;
uint8_t FpCurIdx=0;

uint8_t LdFpAck=0;                         //指纹模块应答表示
uint16_t idyId=0;                          //识别成功的指纹ID
FpAdd_t FpAdd={FP_ADD_GET_IMG,0,0};        //添加指纹 
FpScan_t FpScan={FP_IDLE,0,0};             //扫描指纹
FpDel_t FpDel={FP_DEL_READ_USER,0,0};      //删除指纹

uint8_t FpPowerDown=0; 
uint8_t nofingertimes=0;

/******************************************************************************/
/*
//指纹串口结构初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFp_DatStrInit(void)
{
		UartStruct[UART_FP_INDEX].pTxStart=0;
		UartStruct[UART_FP_INDEX].pTxEnd=0;
		UartStruct[UART_FP_INDEX].rp=UartStruct[UART_FP_INDEX].wp=0;
		UartStruct[UART_FP_INDEX].RxStat=RX_IDLE;
		UartStruct[UART_FP_INDEX].TxStat=TX_IDLE;
		UartStruct[UART_FP_INDEX].RtxTimeout=0;
		UartStruct[UART_FP_INDEX].TxTime=0;
		UartStruct[UART_FP_INDEX].RxTime=0;
	
		FpCurIdx=0xFF;
		
		FpMcuPowerIoInit();
}

/******************************************************************************/
/*
//指纹电源与触摸唤醒IO初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void FpMcuPowerIoInit(void)
{
		EPORT_ConfigGpio(FP_TOUCHOUT_PIN,GPIO_INPUT);
		EPORT_PullConfig(FP_TOUCHOUT_PIN,EPORT_DISPULL);        
		FpMcuPowerOn();
}
/******************************************************************************/
/*
//打开指纹电源
input:   none
output   none
return   none 
*/
/******************************************************************************/
void FpMcuPowerOn(void)
{
		I2C_ConfigGpio(I2C2,FP_MCU_VCC_PIN,GPIO_OUTPUT);
		DelayMS(5); 
		I2C_WriteGpioData(I2C2,FP_MCU_VCC_PIN,Bit_RESET);
}
/******************************************************************************/
/*
//关闭指纹电源
input:   none
output   none
return   none 
*/
/******************************************************************************/
void FpMcuPowerOff(void)
{
		I2C_ConfigGpio(I2C2,FP_MCU_VCC_PIN,GPIO_OUTPUT);
		DelayMS(5); 
		I2C_WriteGpioData(I2C2,FP_MCU_VCC_PIN,Bit_SET); 
}
/******************************************************************************/
/*
//指纹串口初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFp_Init(void)
{
    UART_InitTypeDef UART_InitStruct;
    UART_InitStruct.UART_BaudRate = 57600;
    UART_InitStruct.UART_Mode = UART_INT_MODE;//采用CPU中断方式
		UART_InitStruct.UART_Parity = UART_PARITY_NONE;
		UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_10BIT;
    UART_InitStruct.UART_StopBits = 1;
		UartStruct[UART_FP_INDEX].RtxTimeout=10;
		UART_Init(UART_FP_SCI, &UART_InitStruct);
}
/******************************************************************************/
/*
//指纹串口除能
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFp_Deinit(void)
{
		//不使能上下拉，按道理是使能下拉，但是芯片口下拉使能后输出是高，导致系统不能休眠，已反馈给原厂
		EPORT_ConfigGpio(FP_TOUCHOUT_PIN,GPIO_INPUT);
		EPORT_PullConfig(FP_TOUCHOUT_PIN,EPORT_DISPULL);    
		//RX输入，默认上拉
		UART_ConfigGpio(UART_FP_SCI,UART_RX,GPIO_OUTPUT);
		UART_WriteGpioData(UART_FP_SCI,UART_RX,Bit_RESET);
		//TX输出高
		UART_ConfigGpio(UART_FP_SCI,UART_TX,GPIO_OUTPUT);
		UART_WriteGpioData(UART_FP_SCI,UART_TX,Bit_RESET);
		//关电源
		FpMcuPowerOff();
}
/******************************************************************************/
/*
//指纹串口发送数据
input:   buf -- 待发送的数据
         len -- 数据长度
output   none
return   none
*/
/******************************************************************************/
void UartFp_SendNbytes(uint8_t* buf,uint16_t len)      
{
		if(UartStruct[UART_FP_INDEX].TxStat!=TX_IDLE)return;
		
		UartStruct[UART_FP_INDEX].TxStat=TX_RUNNING;
		UartStruct[UART_FP_INDEX].pTxStart=buf;
		UartStruct[UART_FP_INDEX].pTxEnd=buf+len;
		UartStruct[UART_FP_INDEX].TxTime=GetTimerCount();
		UART_FP_SCI->SCIDRL=*UartStruct[UART_FP_INDEX].pTxStart++;
		UART_FP_SCI->SCICR2|=SCICR2_TIE_MASK;  
}

/******************************************************************************/
/*
//指纹串口生成校验
input:   cpRecBuf -- 生成校验的数据包
output   none
return   nwCHK  -- 生成的校验
*/
/******************************************************************************/
uint16_t GenFpChkSum(uint8_t *cpRecBuf)
{
		uint16_t wLen;
		uint16_t i;
		uint16_t nwCHK = 0;

		wLen = ((uint16_t)cpRecBuf[7]<<8) + cpRecBuf[8];

		for(i = 6; i < 7+wLen;i++)            //wLen-2+3
		{
				nwCHK += cpRecBuf[i];
		}
			
		cpRecBuf[7+wLen] = nwCHK>>8;
		cpRecBuf[8+wLen] = nwCHK;

		return(nwCHK);
}
/******************************************************************************/
/*
//指纹串口发送指令
input:   type -- 指令类型：见协议文档
         buf -- 指令附加数据
         len -- 指令附加数据长度
				 ack -- 发送指令是否需要应答：1-需要， 0-不需要，  2-需要立即发送的应答数据
         timeout -- 等待应答超时时间，单位为毫秒
output   none
return   none 
*/
/******************************************************************************/
void UartFp_SendCmdData(FpPktType_t type,uint8_t *buf,uint16_t len,uint8_t ack,uint32_t timeout)
{
		uint8_t i=0,j=0;
		uint16_t chksum=0;
	
		for(i=0;i<FP_TX_Q_SIZE;i++)
		{
				if(FpUartTxQ[i].DatStat==0)
				{
						FpUartTxQ[i].DatBuf[0]=FP_HEAD_H;
						FpUartTxQ[i].DatBuf[1]=FP_HEAD_L;
						FpUartTxQ[i].DatBuf[2]=0xFF;
						FpUartTxQ[i].DatBuf[3]=0xFF;
						FpUartTxQ[i].DatBuf[4]=0xFF;
						FpUartTxQ[i].DatBuf[5]=0xFF;
						FpUartTxQ[i].DatBuf[6]=type;
						FpUartTxQ[i].DatBuf[7]=(len+2)>>8;    //len只包含数据，不包含校验
						FpUartTxQ[i].DatBuf[8]=len+2;
						for(j=0;j<len;j++)
						{
								FpUartTxQ[i].DatBuf[9+j]=buf[j];
						}
						chksum=GenFpChkSum(FpUartTxQ[i].DatBuf);
						FpUartTxQ[i].DatBuf[9+len]=chksum>>8;
						FpUartTxQ[i].DatBuf[10+len]=chksum;
						
						FpUartTxQ[i].DatBufLen=len+11;
						FpUartTxQ[i].DatStat=1;
						FpUartTxQ[i].NeedAck=ack;
						FpUartTxQ[i].Timeout=timeout;
						FpUartTxQ[i].Retry=0;
						return;
				}
		}
}
/******************************************************************************/
/*
//指纹串口数据发送处理
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFp_DataSendHandle(void)
{
		uint8_t i=0;
		
		if(UartStruct[UART_FP_INDEX].TxStat!=TX_IDLE)return;
		
		for(i=0;i<FP_TX_Q_SIZE;i++)
		{
				if(FpUartTxQ[i].DatStat==1 && FpUartTxQ[i].NeedAck==2)        
				{
						UartFp_SendNbytes(FpUartTxQ[i].DatBuf,FpUartTxQ[i].DatBufLen);
						FpUartTxQ[i].DatStat=0;
						FpCurIdx=0xFF;
						return;
				}
		}
		
		if(FpCurIdx<FP_TX_Q_SIZE)
		{
				if(GetTimerElapse(FpUartTxQ[FpCurIdx].TxTime)<FpUartTxQ[FpCurIdx].Timeout)return;
				
				UartFp_SendNbytes(FpUartTxQ[FpCurIdx].DatBuf,FpUartTxQ[FpCurIdx].DatBufLen);
				LastCmd=FpUartTxQ[FpCurIdx].DatBuf[FP_LEN_IDX+2];
				FpUartTxQ[FpCurIdx].TxTime=GetTimerCount();
				if(++FpUartTxQ[FpCurIdx].Retry>=3)
				{
						FpUartTxQ[FpCurIdx].DatStat=0;
						FpCurIdx=0xFF;
				}
				return;
		}
		
		for(i=0;i<FP_TX_Q_SIZE;i++)
		{
				if(FpUartTxQ[i].DatStat==1)
				{
						UartFp_SendNbytes(FpUartTxQ[i].DatBuf,FpUartTxQ[i].DatBufLen);
						LastCmd=FpUartTxQ[i].DatBuf[FP_LEN_IDX+2];        //保存上一条数据的CMD
						if(FpUartTxQ[i].NeedAck==1)
						{
								FpUartTxQ[i].TxTime=GetTimerCount();
								FpUartTxQ[i].Retry++;
								FpCurIdx=i;
						}
						else
						{
								FpUartTxQ[i].DatStat=0;
								FpCurIdx=0xFF;
						}
						return;
				}
		}
}
/******************************************************************************/
/*
//接收到的指纹串口数据处理
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartFp_RxDataHandle(void)
{
		uint8_t pBufRec[UART_RECV_MAX_LEN]={0};
		
		uint8_t *pFrameHead=0;
		uint16_t i=0,datLen=0,pktchksum=0,pktLen=0;
		
		if(UartStruct[UART_FP_INDEX].RxFrameOk==0)return;
		
		UartStruct[UART_FP_INDEX].RxFrameOk=0;
	
		while(UartStruct[UART_FP_INDEX].wp != UartStruct[UART_FP_INDEX].rp)
		{
				pBufRec[i++]=UartStruct[UART_FP_INDEX].dat[UartStruct[UART_FP_INDEX].rp++];
				
				if(UartStruct[UART_FP_INDEX].rp>=UART_RECV_MAX_LEN)
				{
						UartStruct[UART_FP_INDEX].rp=0;
				}
		}
		datLen=i;
		for(i=0;i<datLen;i++)
		{
				if(FP_HEAD_H==pBufRec[i])
				{
						if(i+1<datLen && FP_HEAD_L==pBufRec[i+1])break;
				}
		}
		//UartFp_SendNbytes(pBufRec,i);        //收发测试
		datLen-=i;
		
		if(datLen<11)return;
		pFrameHead=&pBufRec[i];
		
		pktLen=((uint16_t)pFrameHead[FP_LEN_IDX]<<8)|pFrameHead[FP_LEN_IDX+1];
		pktchksum=((uint16_t)pFrameHead[pktLen+FP_LEN_IDX]<<8)|pFrameHead[pktLen+FP_LEN_IDX+1];
		 
		if(pktchksum!=GenFpChkSum(pFrameHead))return;   
		
		if(pFrameHead[FP_TYPE_IDX]==FP_ACK)     //处理应答包
		{
				if(FpCurIdx<FP_TX_Q_SIZE)           
				{
						FpUartTxQ[FpCurIdx].DatStat=0;
						FpCurIdx=0xFF;
				}
				switch(LastCmd)      //指令码
				{
						case PS_ReadSysPara:            //读系统参数
								LdFpAck=FP_ACK_FLAG+pFrameHead[FP_LEN_IDX+2];
								if(pFrameHead[FP_LEN_IDX+2]==PS_OK)           //确认码OK
								{
										LdFp.SysStat=(uint16_t)pFrameHead[FP_LEN_IDX+3]<<8|pFrameHead[FP_LEN_IDX+4];
										LdFp.SenType=(uint16_t)pFrameHead[FP_LEN_IDX+5]<<8|pFrameHead[FP_LEN_IDX+6];
										LdFp.MaxFpSum=(uint16_t)pFrameHead[FP_LEN_IDX+7]<<8|pFrameHead[FP_LEN_IDX+8];
										LdFp.SecLevel=(uint16_t)pFrameHead[FP_LEN_IDX+9]<<8|pFrameHead[FP_LEN_IDX+10];
										LdFp.DevAddr=(uint32_t)pFrameHead[FP_LEN_IDX+11]<<24|(uint32_t)pFrameHead[FP_LEN_IDX+12]<<16\
										|(uint32_t)pFrameHead[FP_LEN_IDX+13]<<8|pFrameHead[FP_LEN_IDX+14];
										LdFp.PktSize=(uint16_t)pFrameHead[FP_LEN_IDX+15]<<8|pFrameHead[FP_LEN_IDX+16];
										LdFp.BaudRate=(uint16_t)pFrameHead[FP_LEN_IDX+17]<<8|pFrameHead[FP_LEN_IDX+18];
								}
								break;
						case PS_ReadIndexTable:      //有效模板数 
								LdFpAck=FP_ACK_FLAG+pFrameHead[FP_LEN_IDX+2];
								if(pFrameHead[FP_LEN_IDX+2]==PS_OK)
								{
										for(i=0;i<13;i++)
										{
												LdFp.IndexTab[i]=pFrameHead[FP_LEN_IDX+3+i];
											//	printf("fpinfo:0x%02x\n",LdFp.IndexTab[i]);
										}
								}
								break;
						case PS_DeletChar:             //删除指纹
						case PS_Empty:                 //清空指纹库 
								LdFpAck=FP_ACK_FLAG+pFrameHead[FP_LEN_IDX+2];
								break;
						case PS_Search:                //搜索指纹 
								idyId=(uint16_t)pFrameHead[FP_LEN_IDX+3]<<8|pFrameHead[FP_LEN_IDX+4];
								printf("fp idyid: 0x%04x\n",idyId);
						case PS_GetImage:              //获取图像
						case PS_GenChar:               //生成图像特征
						case PS_RegModel:              //合并特征，生成模板
						case PS_StoreChar:             //储存模板
						case PS_Sleep:                 //休眠指令
								LdFpAck=FP_ACK_FLAG+pFrameHead[FP_LEN_IDX+2];
								break;
						default:break; 
				}
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
void UartFp_RtxTimeoutoutHandle(void)
{
		if(UartStruct[UART_FP_INDEX].RxTime>0 && GetTimerElapse(UartStruct[UART_FP_INDEX].RxTime)>UartStruct[UART_FP_INDEX].RtxTimeout)
		{
				UartStruct[UART_FP_INDEX].RxStat=RX_IDLE;
				UartStruct[UART_FP_INDEX].RxTime=0;
				UartStruct[UART_FP_INDEX].RxFrameOk=1;
		}
		if(UartStruct[UART_FP_INDEX].TxTime>0 && GetTimerElapse(UartStruct[UART_FP_INDEX].TxTime)>UartStruct[UART_FP_INDEX].RtxTimeout) 
		{
				UartStruct[UART_FP_INDEX].TxStat=TX_IDLE;
				UartStruct[UART_FP_INDEX].TxTime=0;
				UART_FP_SCI->SCICR2&=~SCICR2_TIE_MASK;  
		}
}

/******************************************************************************/
/*
//指纹复位
input:   none
output   none
return   none 
*/
/******************************************************************************/


/******************************************************************************/
/*
//指纹扫描
input:   none
output   none
return   none 
*/
/******************************************************************************/
void LdFpStatHandle(void)
{
		uint8_t temp[6]={0};
		
		static uint8_t idyok=0;
		static uint8_t fpfailtimes=0;
		static LdFpPdStat_t FpPd={PD_START}; 
		static uint8_t FpError=0;
		
		
		if(SysConfig.Bits.FpLock==1)return;                                //指纹锁定
		
		
		switch(FpScan.LdFpStat)
		{
				case FP_POWER_DOWN:                  //指纹休眠 
						switch(FpPd)
						{
								case PD_START:
										if(FpScan.StTime==0)
										{
												temp[0]=PS_Sleep;
												UartFp_SendCmdData(FP_CMD,temp,1,1,500);
												FpScan.StTime=GetTimerCount();
										}
										else if(LdFpAck&FP_ACK_FLAG)
										{
												temp[0]=LdFpAck&0x7F;
												if(temp[0]==PS_OK)
												{
														FpPd=PD_WAIT_PD;
														FpScan.StTime=0;
												}
										}
										else if(GetTimerElapse(FpScan.StTime)>=2000)
										{
												FpScan.StTime=0;
												printf("FP POWER DOWN TIMEOUT RETRY\n");
												break;
										}
										break;
								case PD_WAIT_PD:
										if(EPORT_ReadGpioData(FP_TOUCHOUT_PIN)==Bit_RESET)
										{
												DelayMS(10);
												if(EPORT_ReadGpioData(FP_TOUCHOUT_PIN)==Bit_RESET)
												{
														printf("NO FIG POWER DOWN \n");
														FpPd=PD_WAIT_WK;
														UartFp_Deinit();                               //指纹IO除能
														FpPowerDown=1;
														LPM_SetStopMode(LPM_FP_ID,LPM_Enable);         //清标志 
												}
										}
										else if(FpScan.StTime==0)
										{
												FpScan.StTime=GetTimerCount();
										}
										else if(GetTimerElapse(FpScan.StTime)>=200)
										{
												printf("FP POWER DOWN FAIL\n");
												FpPd=PD_START;
												FpScan.StTime=0;
										}
										break;
								case PD_WAIT_WK:
										if(EPORT_ReadGpioData(FP_TOUCHOUT_PIN)==Bit_SET)
										{
												DelayMS(10);
												if(EPORT_ReadGpioData(FP_TOUCHOUT_PIN)==Bit_SET)
												{
														nofingertimes=0;                               //有手指按下，清0计数
														UartFp_DatStrInit();                           //重新使能指纹模块
														UartFp_Init();
														FpScan.LdFpStat=FP_IDLE;                       //重启状态机
														FpPd=PD_START;
														FpPowerDown=0;                                 //清休眠标志
												}
										}
										break;
						}
						break;
				case FP_IDLE:
						if(FpError==1)break;
						nofingertimes=0;
						LPM_SetStopMode(LPM_FP_ID,LPM_Disable);
						if(FpScan.StTime==0)             //上电延时
						{
								FpScan.StTime=GetTimerCount();
						}
						else if(GetTimerElapse(FpScan.StTime)>=FP_POWERON_DELAY)  //读指纹传感器信息
						{
								FpScan.StTime=0;
								FpScan.LdFpStat=FP_CHECK_SENSOR;
								FpScan.ComRetry=0;
						}
						break;
				case FP_CHECK_SENSOR:
						if(FpScan.StTime==0)
						{
								temp[0]=PS_ReadSysPara;
								UartFp_SendCmdData(FP_CMD,temp,1,1,300); 
								LdFpAck=0;
								FpScan.StTime=GetTimerCount();
						}
						else if(LdFpAck==FP_ACK_FLAG && LdFp.DevAddr==0xFFFFFFFF)    //成功读到指纹信息，进入休眠检查手指是否按下
						{
								printf("fp addr:0x%08x \n",LdFp.DevAddr);
								FpScan.StTime=0;
								FpScan.LdFpStat=FP_READ_IDXTABLE;
								FpScan.ComRetry=0;
						}
						else if(GetTimerElapse(FpScan.StTime)>=1500)
						{
								FpScan.StTime=0;
								if(++FpScan.ComRetry>=FP_STEP_RTY_MAX)
								{
										printf("read syspara timeout \n");
										FpScan.ComRetry=0;
										FpError=1;
										LPM_SetStopMode(LPM_FP_ID,LPM_Enable);
										FpScan.LdFpStat=FP_IDLE;
								}
						}
						break;
				case FP_READ_IDXTABLE:
						if(FpScan.StTime==0)
						{
								printf("read index \n");
								temp[0]=PS_ReadIndexTable;
								UartFp_SendCmdData(FP_CMD,temp,1,1,300); 
								LdFpAck=0;
								FpScan.StTime=GetTimerCount();
						}
						else if(LdFpAck==FP_ACK_FLAG)     
						{
								LdFp.ActFpSum=Fp_GetValidFpSum();
								printf("ActFpSum: %02d \n",LdFp.ActFpSum);
								FpScan.StTime=0;
								FpScan.LdFpStat=FP_GET_IMG;
								FpScan.ComRetry=0;
						}
						else if(GetTimerElapse(FpScan.StTime)>=1500)
						{
								FpScan.StTime=0;
								if(++FpScan.ComRetry>=FP_STEP_RTY_MAX)
								{
										printf("read index timeout \n");
										FpScan.ComRetry=0;
										FpScan.LdFpStat=FP_IDLE;
								}
						}
						break;
				case FP_GET_IMG:
						if(FpScan.StTime==0)                           //发送获取图像指令
						{
						//		printf("get img \n");
								temp[0]=PS_GetImage;
								UartFp_SendCmdData(FP_CMD,temp,1,1,500);
								LdFpAck=0;
								FpScan.StTime=GetTimerCount();
						}
						else if(LdFpAck&FP_ACK_FLAG)                  //成功收到应答
						{
								temp[0]=LdFpAck&0x7F;
								LdFpAck=0;
								FpScan.StTime=0;
								if(temp[0]==PS_OK)
								{
										nofingertimes=0;                   //检测到手指，清0无手指计数
										if(SysConfig.Bits.Activate==0)
										{
												if(idyok==0)
												{
														idyok=1;
														AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"请激活产品"),UNBREAK);
														
														PirLed_SetStat(LOGO_OPEN_SUCCESS);
														UserControlLock(CTL_OPEN_LOCK,FP_OPEN,0XFF);            //演示模式开锁
												}
										}
										else
										{
												if(idyok==0)            //识别到手指，并且未验证成功    
												{
														FpScan.LdFpStat=FP_GEN_CHAR;          //获取成功，生成特征
												}
										}
								}
								else if(temp[0]==PS_NO_FINGER )//识别成功，手指离开
								{
										idyok=0;
										if(++nofingertimes>10)           //扫描10次无手指，进入休眠
										{
												FpScan.LdFpStat=FP_POWER_DOWN;
												FpPd=PD_START;
												FpScan.StTime=0;
										}
								}
						}
						else if(GetTimerElapse(FpScan.StTime)>1500)   //应答超时，重新获取图像
						{
								FpScan.StTime=0;
						}
						break;
				case FP_GEN_CHAR:
						if(FpScan.StTime==0)                          //发送生成特征指令
						{
								printf("gen char \n");
								temp[0]=PS_GenChar;
								temp[1]=1;         
								UartFp_SendCmdData(FP_CMD,temp,2,1,500);
								LdFpAck=0;
								FpScan.StTime=GetTimerCount();
						}
						else if(LdFpAck&FP_ACK_FLAG)
						{
								temp[0]=LdFpAck&0x7F;
								LdFpAck=0;
								FpScan.StTime=0;
								if(temp[0]==PS_OK)         //获取图像成功，搜索指纹库
								{
										FpScan.LdFpStat=FP_SEARCH_LIB;
								}
						}
						else if(GetTimerElapse(FpScan.StTime)>1500)
						{
								FpScan.StTime=0;
								FpScan.LdFpStat=FP_GET_IMG;
						}
						break;
				case FP_SEARCH_LIB:
						if(FpScan.StTime==0)
						{
								printf("search lib \n");
								temp[0]=PS_Search;
								temp[1]=1;
								temp[2]=0;
								temp[3]=0;
								temp[4]=0;
								temp[5]=100;
								UartFp_SendCmdData(FP_CMD,temp,6,1,2000);
								LdFpAck=0;
								FpScan.StTime=GetTimerCount();
						}
						else if(LdFpAck&FP_ACK_FLAG)
						{
								temp[0]=LdFpAck&0x7F;
								FpScan.StTime=0;
								LdFpAck=0;
								if(temp[0]==PS_OK)        //验证成功
								{
										LockUpdatePeripLockStat(PERIP_FP_INDEX,0);
								
										fpfailtimes=0;
										if(KeyPadMode==KEYPAD_CONFIG)      //00管理指纹
										{
												if(idyId==0x0)
												{
														idyok=1;
														MenuNow=Menu_KeyPadConfigSelect;
												}
												else 
												{
														idyok=2;              //验证失败
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Verification failed":"验证失败"),BREAK);
												}
										}
										else if(SysConfig.Bits.MutiIdy==1)
										{
												idyok=1;
												if(MutiIdyItem&(~IDY_FP))        //非指纹验证成功+指纹验证成功
												{
														MutiIdyItem=0;
														AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
														PirLed_SetStat(LOGO_OPEN_SUCCESS);
														printf("idy ok: 0x%04x\n",idyId);
														UserControlLock(CTL_OPEN_LOCK,FP_OPEN,idyId); 
												}
												else
												{
														MutiIdyItem|=IDY_FP;
														PirLed_SetStat(LOGO_OPEN_SUCCESS_MUTIL);
												}
										}
										else
										{
												idyok=1;
												AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
												PirLed_SetStat(LOGO_OPEN_SUCCESS);
												printf("idy ok: 0x%04x\n",idyId);
												UserControlLock(CTL_OPEN_LOCK,FP_OPEN,idyId);
										}
								}
								else                      //验证失败
								{
										idyok=2;              //验证失败
										PirLed_SetStat(LOGO_OPEN_FAIL);
										AudioPlayVoice(GetVolIndex("检卡音"),BREAK);
										AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
										if(++fpfailtimes>=3)
										{
												fpfailtimes=0;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Verification failed":"验证失败"),UNBREAK);
												LockUpdatePeripLockStat(PERIP_FP_INDEX,1);
										}
								}
								FpScan.LdFpStat=FP_GET_IMG;
						}
						else if(GetTimerElapse(FpScan.StTime)>6500)
						{
								FpScan.StTime=0;
								FpScan.LdFpStat=FP_GET_IMG;
						}
						break;
				default:break;
		}
}
/******************************************************************************/
/*
//指纹模块添加指纹
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
uint8_t FpAddHandle(uint8_t type) 
{
		uint8_t temp[10]={0};
		uint8_t freeslot=0;
		uint8_t retData=0xFF;
		
		if(FpPowerDown==1)
		{
				FpPowerDown=0;
				nofingertimes=0;
				UartFp_DatStrInit();
				UartFp_Init();
		}
		
		switch(FpAdd.LdFpAddStat)
		{
				case FP_ADD_READ_USER:
						if(FpAdd.StTime==0)
						{
								printf("FP read index \n");
								temp[0]=PS_ReadIndexTable;
								UartFp_SendCmdData(FP_CMD,temp,1,1,500); 
								LdFpAck=0;
								FpAdd.StTime=GetTimerCount();
						}
						else if(LdFpAck==FP_ACK_FLAG)     
						{
								LdFp.ActFpSum=Fp_GetValidFpSum();
								printf("ActFpSum: %02d \n",LdFp.ActFpSum);
								FpAdd.StTime=0;
								FpAdd.LdFpAddStat=FP_ADD_GET_IMG;
								FpAdd.VolStime=0;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add fingerprint":"添加指纹"),UNBREAK); 
								freeslot=Fp_GetFpFristFreeSlot();
								if(ADD_ADMIN==type)freeslot=0;
								if(freeslot<100)
								{
										AudioPlayVoice(freeslot/10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK); 
										AudioPlayVoice(freeslot%10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK);
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);     //返回
								}
								else 
								{
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Fingerprint library is full":"指纹库已满"),UNBREAK);
										retData=FP_FAIL;
								}
						}
						else if(GetTimerElapse(FpAdd.StTime)>=1500)
						{
								FpAdd.StTime=0;
								if(++FpAdd.ComRetry>=10)
								{
										printf("fp timeout\n"); 
										FpAdd.ComRetry=0;
										retData=FP_FAIL;
								}
						}
						break;
				case FP_ADD_GET_IMG:
						if(FpAdd.StTime==0)
						{
								temp[0]=PS_GetImage;
								UartFp_SendCmdData(FP_CMD,temp,1,1,1000);
								LdFpAck=0;
								FpAdd.StTime=GetTimerCount();
						}
						else if(LdFpAck&FP_ACK_FLAG)   //等待应答
						{
								temp[0]=LdFpAck&0x7F;
								LdFpAck=0;
								FpAdd.StTime=0;
								if(FpAdd.AlreadGetIMG==0)    //未采集指纹，等待手指按下
								{
										if(temp[0]==PS_NO_FINGER)
										{
												if(FpAdd.VolStime==0)
												{ 
														FpAdd.VolStime=GetTimerCount();
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press finger on the reader":"请按手指"),UNBREAK);          //请按手指 
												}
												else if(GetTimerElapse(FpAdd.VolStime)>5000)
												{
														FpAdd.VolStime=0;
												}
										}
										else if(temp[0]==PS_OK)    
										{
												FpAdd.LdFpAddStat=FP_ADD_GEN_CHAR;  //获取成功，生成特征
										}
								}
								else                       //已采集指纹，提示松开手指
								{
										if(temp[0]==PS_OK)    
										{
//												if(FpAdd.VolStime==0)
//												{
//														FpAdd.VolStime=GetTimerCount();
//												}
//												else if(GetTimerElapse(FpAdd.VolStime)>5000)
//												{
//														FpAdd.VolStime=0;
//														//AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press your finger on the reader again":"请再按手指"),BREAK);          //2S钟提示松开手指
//												}
										}
										else if(temp[0]==PS_NO_FINGER)
										{
												FpAdd.AlreadGetIMG=0;
												FpAdd.VolStime=0;
										}
								}
						}
						else if(GetTimerElapse(FpAdd.StTime)>3000)
						{
								FpAdd.StTime=0;
								retData=FP_FAIL;
						}
						break;
				case FP_ADD_GEN_CHAR:
						if(FpAdd.StTime==0)
						{
								printf("gen char \n");
								temp[0]=PS_GenChar;
								temp[1]=FpAdd.GenCharCnt+1;          //最大12（1-12）
								UartFp_SendCmdData(FP_CMD,temp,2,1,500);
								LdFpAck=0;
								FpAdd.StTime=GetTimerCount();
						}
						else if(LdFpAck&FP_ACK_FLAG)
						{
								temp[0]=LdFpAck&0x7F;
								LdFpAck=0;
								FpAdd.StTime=0;
								if(temp[0]==PS_OK)         //获取图像成功
								{
										FpAdd.AlreadGetIMG=1;
										AudioPlayVoice(GetVolIndex("指纹音"),BREAK);
										if(++FpAdd.GenCharCnt>=FP_REG_PRESS_TIMES)
										{
												FpAdd.LdFpAddStat=FP_ADD_REG_MODE;
										}
										else 
										{
												FpAdd.VolStime=0;
												FpAdd.LdFpAddStat=FP_ADD_GET_IMG;
										}
								}
								else
								{
										FpAdd.LdFpAddStat=FP_ADD_GET_IMG;   //失败重新获取图像
								}
						}
						else if(GetTimerElapse(FpAdd.StTime)>2000)
						{
								FpAdd.StTime=0;
								retData=FP_FAIL;
						}
						break;
				case FP_ADD_REG_MODE:
						if(FpAdd.StTime==0)
						{
								printf("reg mod \n");
								temp[0]=PS_RegModel;
								UartFp_SendCmdData(FP_CMD,temp,1,1,500);
								FpAdd.StTime=GetTimerCount();
								LdFpAck=0;
						}
						else if(LdFpAck&FP_ACK_FLAG)
						{
								temp[0]=LdFpAck&0x7F;
								LdFpAck=0;
								FpAdd.StTime=0;
								if(temp[0]==PS_OK)        //生成模板成功
								{
										FpAdd.LdFpAddStat=FP_ADD_STORE_MODE;
								}
						}
						else if(GetTimerElapse(FpAdd.StTime)>2000)
						{
								FpAdd.StTime=0;
								retData=FP_FAIL;
						}
						break;
				case FP_ADD_STORE_MODE:
						if(FpAdd.StTime==0)
						{
								printf("sotre mod \n");
								temp[0]=PS_StoreChar;
								temp[1]=1;
								temp[2]=0;
								temp[3]=Fp_GetFpFristFreeSlot();
								if(ADD_ADMIN==type)temp[3]=0;
								UartFp_SendCmdData(FP_CMD,temp,4,1,500);
								LdFpAck=0;
								FpAdd.StTime=GetTimerCount();
						}
						else if(LdFpAck&FP_ACK_FLAG)
						{
								temp[0]=LdFpAck&0x7F;
								FpAdd.StTime=0;
								if(temp[0]==PS_OK)        //存储模板成功
								{
										retData=FP_SUCCESS;	
										
										LockAddUser(PERIP_FP_INDEX,(ADD_ADMIN==type)?0:Fp_GetFpFristFreeSlot());
										
										FpUpdateUser(1,ADD_ADMIN==type?0:Fp_GetFpFristFreeSlot(),type==ADD_COERCE?1:0);
										printf("sotre mod ok\n");
								}
								else                      //存储失败  
								{
										retData=FP_FAIL;	
										printf("sotre mod fail: 0x%02x \n",temp[0]);
								}
						}
						else if(GetTimerElapse(FpAdd.StTime)>2000)
						{
								FpAdd.StTime=0;
								retData=FP_FAIL;
						}
						break;
		}
		return retData;
}
/******************************************************************************/
/*
//指纹模块删除指纹
input:   id -- 指纹ID
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
uint8_t FpDelHandle(uint8_t id)
{
		uint8_t temp[10]={0};
		uint8_t retData=0xFF;
		
		if(FpPowerDown==1)
		{
				FpPowerDown=0;
				nofingertimes=0;
				UartFp_DatStrInit();
				UartFp_Init();
		}
		
		switch(FpDel.LdFpDelStat)
		{
				case FP_DEL_READ_USER:
						if(FpDel.StTime==0)
						{
								printf("read indextable \n");
								temp[0]=PS_ReadIndexTable;
								UartFp_SendCmdData(FP_CMD,temp,1,1,500); 
								LdFpAck=0;
								FpDel.StTime=GetTimerCount();
						}
						else if(LdFpAck==FP_ACK_FLAG)     
						{
								LdFp.ActFpSum=Fp_GetValidFpSum();
								printf("ActFpSum: %02d \n",LdFp.ActFpSum);
								FpDel.StTime=0;
								FpDel.LdFpDelStat=FP_DEL_RUN;
						}
						else if(GetTimerElapse(FpDel.StTime)>1500)
						{
								FpDel.StTime=0;
								if(++FpDel.ComRetry>=3)
								{
										printf("fp timeout\n"); 
										FpDel.ComRetry=0;
										retData=FP_FAIL;
								}
						}
						break;
				case FP_DEL_RUN:
						if(FpDel.StTime==0)
						{
								printf("del fp start \n");
								if(id==0xFF)
								{
										temp[0]=PS_Empty;
										UartFp_SendCmdData(FP_CMD,temp,1,1,1000); 
								}
								else
								{
										temp[0]=PS_DeletChar;
										temp[1]=0;
										temp[2]=id;
										temp[3]=0;
										temp[4]=1;
										UartFp_SendCmdData(FP_CMD,temp,5,1,1000); 
								}
								LockDelUser(PERIP_FP_INDEX,id);
								LdFpAck=0;
								FpDel.StTime=GetTimerCount();
						}
						else if(LdFpAck&FP_ACK_FLAG)
						{
								FpDel.StTime=0;
								temp[0]=LdFpAck&0x7F;
								printf("del fp ack: 0x%02x \n",temp[0]);
								if(temp[0]==PS_OK)
								{
										retData=FP_SUCCESS;
										FpUpdateUser(0,id,0);
								}
								else
								{
										retData=FP_FAIL;
								}
						}
						else if(GetTimerElapse(FpDel.StTime)>=1500)
						{
								FpDel.StTime=0;
								retData=FP_FAIL;
						}
						break;
		}
		return retData;
}
/******************************************************************************/
/*
//指纹模块休眠
input:   none
output   none
return   none
*/
/******************************************************************************/
uint8_t Fp_PowerDown(void)
{
		FpPd_t FpPd={0};
		uint8_t temp=0;
		uint8_t fails=0;

		if(FpPowerDown==1)return FpPowerDown;
		
		EPORT_ConfigGpio(FP_TOUCHOUT_PIN,GPIO_INPUT);
		EPORT_PullConfig(FP_TOUCHOUT_PIN,EPORT_DISPULL); 
			
		while(1)
		{
				WDT_FeedDog();                         //喂狗
				Uart_RtxFrameHandle();
				if(FpPd.Stime==0)
				{
						temp=PS_Sleep;
						UartFp_SendCmdData(FP_CMD,&temp,1,1,500);
						FpPd.Stime=GetTimerCount();
				}
				else if(LdFpAck&FP_ACK_FLAG)
				{
						temp=LdFpAck&0x7F;
						if(temp==PS_OK)
						{
								if(EPORT_ReadGpioData(FP_TOUCHOUT_PIN)==Bit_RESET)
								{
										DelayMS(5);
										if(EPORT_ReadGpioData(FP_TOUCHOUT_PIN)==Bit_RESET)
										{
												FpPowerDown=1;
												printf("FP POWER DOWN OK \n");
												break;
										}
								}
								else if(GetTimerElapse(FpPd.Stime)>=1500)        
								{
										FpPd.Stime=0;
										if(++fails>=3)
										{
												printf("FP POWER DOWN TIMEOUT\n");
										}
								}
						}
				}
				else if(GetTimerElapse(FpPd.Stime)>=2000)
				{
						printf("FP POWER DOWN TIMEOUT\n");
						break;
				}
		}
		return FpPowerDown;
}

/******************************************************************************/
/*
//获取有效的指纹数量
input:   none
output   none
return   vSum -- 可用指纹数量 
*/
/******************************************************************************/
uint8_t Fp_GetValidFpSum(void)
{
		uint8_t i=0,j=0;
		uint8_t vSum=0;
		for(i=0;i<13;i++)
		{
				for(j=0;j<8;j++)
				{
						if(LdFp.IndexTab[i]&(1<<j))
						{
								vSum++;
						}
				}
		}
		return vSum;
}
/******************************************************************************/
/*
//获取空闲的指纹SLOT
input:   none
output   none
return   第一个可用的SLOT
*/
/******************************************************************************/
uint8_t Fp_GetFpFristFreeSlot(void)
{
		uint8_t i=0,j=0;
	
		for(i=0;i<13;i++)
		{
				for(j=0;j<8;j++)
				{
						if((LdFp.IndexTab[i]&(1<<j))==0)
						{
								return (i*8+j);     //12*8+4=100;
						}
				}
		}
		return (i*8);     //13*8=104
}
/******************************************************************************/
/*
//更新指纹用户
input:   type -- 更新类型：1-添加 0-删除
         id -- 更新的ID  删除时FF为删除所有
output   none
return   none
*/
/******************************************************************************/
void FpUpdateUser(uint8_t type,uint8_t id,uint8_t isCoerce)
{
		if(id>=FP_MAX_NUM && id!=0xFF)return;       //0-99
		if(id==0xFF && type==0) //删除所有
		{
				LdFp.ActFpSum=0;
				memset(&LdFp.IndexTab[0],0,13);
				memset(&CoerceFp.CFP.CoerceFp[0],0,13);
				IntFlashWrite(CoerceFp.CoerceFpBuf,COERCE_FP_ADDR,sizeof(CoerceFp_t));
				return;
		}
		if(type)             //添加人脸
		{
				LdFp.IndexTab[id/8]|=(1<<(id%8));
				LdFp.ActFpSum+=1;
				if(isCoerce==1)
				{
						CoerceFp.CFP.CoerceFp[id/8]|=(1<<(id%8));
						IntFlashWrite(CoerceFp.CoerceFpBuf,COERCE_FP_ADDR,sizeof(CoerceFp_t));
				}
		}
		else                 //删除人脸
		{
				if(LdFp.ActFpSum<1)return;
				LdFp.IndexTab[id/8]&=~(1<<(id%8));
				CoerceFp.CFP.CoerceFp[id/8]&=~(1<<(id%8));
				IntFlashWrite(CoerceFp.CoerceFpBuf,COERCE_FP_ADDR,sizeof(CoerceFp_t));
				LdFp.ActFpSum-=1;
		}
}

