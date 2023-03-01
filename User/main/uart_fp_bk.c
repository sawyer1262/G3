#include "uart_drv.h"
#include "uart_fp.h"
#include "i2c_drv.h"
#include "uart.h"
#include "timer.h"



LdFpStr_t LdFp;
UartTxStr_t FpUartTxQ[FP_TX_Q_SIZE];
uint8_t LastCmd=0;
uint8_t FpCurIdx=0;

LdFpRegStr_t LdFpRegStr;
LdFpIdyStr_t LdFpIdyStr;


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
		
		LdFpRegStr.Stat=FP_REG_DONE;
	
		FpMcuPowerIoInit();
}

void FpMcuPowerIoInit(void)
{
		 
		I2C_ConfigGpio(I2C2,FP_TOUCHOUT_PIN,GPIO_INPUT);
		I2C_ConfigGpio(I2C2,FP_MCU_VCC_PIN,GPIO_OUTPUT);
		FpMcuPowerOn();
}

void FpMcuPowerOn(void)
{
		I2C_WriteGpioData(I2C2,FP_MCU_VCC_PIN,Bit_RESET);
}

void FpMcuPowerOff(void)
{
		I2C_WriteGpioData(I2C2,FP_MCU_VCC_PIN,Bit_SET); 
}

void UartFp_Init(void)
{
    UART_InitTypeDef UART_InitStruct;
	//init uart
    UART_InitStruct.UART_BaudRate = 57600;
    UART_InitStruct.UART_Mode = UART_INT_MODE;//采用CPU中断方式
		UART_InitStruct.UART_Parity = UART_PARITY_NONE;
		UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_10BIT;
    UART_InitStruct.UART_StopBits = 1;
//  //UART_StopBits = 2
//    UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_11BIT;
//    UART_InitStruct.UART_StopBits = 2;
		UartStruct[UART_FP_INDEX].RtxTimeout=10;
		UART_Init(UART_FP_SCI, &UART_InitStruct);
}

void UartFp_Deinit(void)
{
		
}

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

void UartFp_DataSendHandle(void)
{
		uint8_t i=0;
		
		if(UartStruct[UART_FP_INDEX].TxStat!=TX_IDLE)return;
		
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
						}
						return;
				}
		}
}

void UartFp_RxDataHandle(void)
{
		uint8_t pBufRec[UART_RECV_MAX_LEN]={0};
		
		uint8_t *pFrameHead=0;
		uint16_t i=0,datlen=0,pktchksum=0,pktLen=0;
		
		if(UartStruct[UART_FP_INDEX].RxFrameOk==0)return;
		
		UartStruct[UART_FP_INDEX].RxFrameOk=0;
	
		while(UartStruct[UART_FP_INDEX].wp != UartStruct[UART_FP_INDEX].rp)
		{
				pBufRec[i++]=UartStruct[UART_FP_INDEX].dat[UartStruct[UART_FP_INDEX].rp++];
				
//				if(bHeadMatchStat==0)
//				{
//						if(pBufRec[i]==FP_HEAD_H)
//						{
//								bHeadMatchStat=1;
//						}
//				}
//				else if(bHeadMatchStat==1)
//				{
//						if(pBufRec[i]==FP_HEAD_L)
//						{
//								bHeadMatchStat=2;
//								datLen=2;                               
//								pFrameHead=&pBufRec[i-1];
//						}
//						else 
//						{
//								bHeadMatchStat=0;
//						}
//				}
//				else
//				{
//						datLen++;
//				}
//				i++;UartStruct[UART_FP_INDEX].rp++;
				if(UartStruct[UART_FP_INDEX].rp>=UART_RECV_MAX_LEN)
				{
						UartStruct[UART_FP_INDEX].rp=0;
				}
		}
		datlen=i;
		//UartFp_SendNbytes(pBufRec,i);        //收发测试
		for(i=0;i<datlen;i++)
		{
				if(pBufRec[i]==FP_HEAD_H)
				{
						if(i+1<datlen && pBufRec[i+1]==FP_HEAD_L)
						{
								break;
						}
				}
		}
		datlen-=i;
		if(datlen<11)return;
		
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
						case PS_ValidTempleteNum:      //有效模板数 
								if(pFrameHead[FP_LEN_IDX+2]==PS_OK)
								{
										LdFp.ActFpSum=(uint16_t)pFrameHead[FP_LEN_IDX+3]<<8|pFrameHead[FP_LEN_IDX+4];
								}
								break;
						case PS_DeletChar:             //删除指纹
						case PS_Empty:                 //清空指纹库 
								if(pFrameHead[FP_LEN_IDX+2]==PS_OK)    //清除指纹，需要重新读有效模板数
								{
										Fp_ReadActFpSum();
								}
								break;
						case PS_GetImage:              //获取图像
								if(LdFpRegStr.Stat>FP_REG_DONE)
								{
										LdFpRegStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								if(LdFpIdyStr.Stat>FP_IDY_DONE)
								{
										LdFpIdyStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								break;
						case PS_GenChar:               //生成图像特征
								if(LdFpRegStr.Stat==FP_REG_GEN_CHAR_WAIT_ACK)
								{
										LdFpRegStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								if(LdFpIdyStr.Stat>FP_IDY_DONE)
								{
										LdFpIdyStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								break;
						case PS_RegModel:              //合并特征，生成模板
								if(LdFpRegStr.Stat==FP_REG_REG_MODEL_WAIT_ACK)
								{
										LdFpRegStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								break;
						case PS_StoreChar:             //储存模板
							//	if(LdFpRegStr.Stat==FP_REG_REG_MODEL_WAIT_ACK)
								{
										LdFpRegStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								break;
						case PS_Search:                //搜索指纹 
								if(LdFpIdyStr.Stat>FP_IDY_DONE)
								{
										LdFpIdyStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								break;
						case PS_Sleep:                 //休眠指令
								if(LdFpRegStr.Stat>FP_REG_DONE)
								{
										LdFpRegStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								if(LdFpIdyStr.Stat>FP_IDY_DONE)
								{
										LdFpIdyStr.Ack=0x80+pFrameHead[FP_LEN_IDX+2];   //应答
								}
								break;
						default:break; 
				}
		}
}

void UartFp_RtxTimeoutoutHandle(void)
{
		if(UartStruct[UART_FP_INDEX].RxTime>0 && GetTimerElapse(UartStruct[UART_FP_INDEX].RxTime)>UartStruct[UART_FP_INDEX].RtxTimeout)
		{
				UartStruct[UART_FP_INDEX].RxTime=0;
				UartStruct[UART_FP_INDEX].RxStat=RX_IDLE;
				UartStruct[UART_FP_INDEX].RxFrameOk=1;
		}
		if(UartStruct[UART_FP_INDEX].TxTime>0 && GetTimerElapse(UartStruct[UART_FP_INDEX].TxTime)>UartStruct[UART_FP_INDEX].RtxTimeout) 
		{
				UartStruct[UART_FP_INDEX].TxTime=0;
				UartStruct[UART_FP_INDEX].TxStat=TX_IDLE;
				UART_FP_SCI->SCICR2&=~SCICR2_TIE_MASK;  
		}
		//注册指纹
		FpRegNewFpStatHandle();
		//刷指纹
		FpIdyNewFpStatHandle();
}

//外部调用接口


void Fp_ReadActFpSum(void)
{
		uint8_t temp=PS_ValidTempleteNum;
		UartFp_SendCmdData(FP_CMD,&temp,1,1,1000); 
}

void Fp_Sleep(void)
{
		uint8_t temp=PS_Sleep;
		UartFp_SendCmdData(FP_CMD,&temp,1,1,500); 
}

void Fp_RegNewFp(void)
{
		if(LdFpRegStr.Stat!=FP_REG_DONE)return;
		
		Fp_Sleep();
		LdFpRegStr.StartTime=GetTimerCount();
		LdFpRegStr.Ack=0;
		LdFpRegStr.Stat=FP_REG_WAIT_FG_DOWN;
		LdFpRegStr.CharCount=0;
}

void Fp_IdyNewFp(void)
{
		if(LdFpIdyStr.Stat!=FP_IDY_DONE)return;
		
		Fp_Sleep();
		LdFpIdyStr.StartTime=GetTimerCount();
		LdFpIdyStr.Ack=0;
		LdFpIdyStr.Stat=FP_IDY_WAIT_FG_DOWN;
}

void FpIdyNewFpStatHandle(void)
{
		uint8_t temp[6]={0};
		switch(LdFpIdyStr.Stat)
		{
				case FP_IDY_DONE:
						if(LdFpIdyStr.StartTime>0)LdFpIdyStr.StartTime=0;
						break;
				case FP_IDY_WAIT_FG_DOWN:
						//未收到指纹模块应答，等待应答（指令应答时间大概有300ms）
						if(LdFpIdyStr.Ack==0)             
						{
								if(GetTimerElapse(LdFpIdyStr.StartTime)>2000)        //通讯错误，设备无应答退出
								{
										LdFpIdyStr.Stat=FP_IDY_DONE;
								}
						}
						if(GetTimerElapse(LdFpIdyStr.StartTime)<300)break;    //稍微延时一会：应答大概300MS，延时200MS
						
						//成功收到应答，指纹进入低功耗，等待手指按上引脚电平变高，5秒超时
						if(I2C_ReadGpioData(I2C2,FP_TOUCHOUT_PIN)==Bit_RESET)          
						{
								if(GetTimerElapse(LdFpIdyStr.StartTime)>7000)
								{
										LdFpIdyStr.Stat=FP_IDY_DONE;
								}
						}
						//引脚电平为高，检测到手指
						else
						{
								LdFpIdyStr.Stat=FP_IDY_GET_IMG;    //转到图像获取
						}
						break;
				case FP_IDY_GET_IMG:
						temp[0]=PS_GetImage;
						UartFp_SendCmdData(FP_CMD,temp,1,1,500);
						LdFpIdyStr.StartTime=GetTimerCount();
						LdFpIdyStr.Stat=FP_IDY_GET_IMG_WAIT_ACK;
						LdFpIdyStr.Ack=0;
						break;
				case FP_IDY_GET_IMG_WAIT_ACK:
						if(LdFpIdyStr.Ack!=0)
						{
								temp[0]=LdFpIdyStr.Ack&0x7F;                //获取反馈信息
								if(temp[0]==PS_OK)                   //获取图像成功
								{
										LdFpIdyStr.Stat=FP_IDY_GEN_CHAR;
								}
								if(temp[0]==PS_GET_IMG_ERR)         //获取图像出错，重新获取 
								{
										LdFpIdyStr.Stat=FP_IDY_GET_IMG;
								}
						}
						else if(GetTimerElapse(LdFpIdyStr.StartTime)>1500)   //指令超时无应答退出
						{
								LdFpIdyStr.Stat=FP_IDY_DONE;
						}
						break;
				case FP_IDY_GEN_CHAR:
						temp[0]=PS_GenChar;
						temp[1]=1;
						UartFp_SendCmdData(FP_CMD,temp,2,1,500);
						LdFpIdyStr.StartTime=GetTimerCount();
						LdFpIdyStr.Stat=FP_IDY_GEN_CHAR_WAIT_ACK;
						LdFpIdyStr.Ack=0;
						break;
				case FP_IDY_GEN_CHAR_WAIT_ACK:
						if(LdFpIdyStr.Ack!=0)
						{
								temp[0]=LdFpIdyStr.Ack&0x7F;
								if(temp[0]==PS_OK)                         //生成特征正常
								{
										LdFpIdyStr.Stat=FP_IDY_SEACH_FP;
								}
								//如下报错信息，可语音提示
								if(temp[0]==PS_FP_DISORDER)                 //图像太乱，生不成特征
								{
										
								}
								if(temp[0]==PS_LITTLE_FEATURE)              //图像正常，特征点太少
								{
										
								}
								if(temp[0]==PS_INVALID_IMAGE)              //图像缓冲区内没有有效原始图而生不成图像
								{
										
								}
								if(temp[0]==PS_RELATE)                     //当前指纹模板与之前模板之间有关联
								{
										
								}
						}
						else if(GetTimerElapse(LdFpIdyStr.StartTime)>1500)   //超时时间500ms
						{
								LdFpIdyStr.Stat=FP_IDY_DONE;
						}
						break;
				case FP_IDY_SEACH_FP:
						temp[0]=PS_Search;
						temp[1]=1;
						temp[2]=0;
						temp[3]=0;
						temp[4]=0;
						temp[5]=32;
						UartFp_SendCmdData(FP_CMD,temp,6,1,500);
						LdFpIdyStr.StartTime=GetTimerCount();
						LdFpIdyStr.Stat=FP_IDY_SEACH_FP_WAIT_ACK;
						LdFpIdyStr.Ack=0;
						break;
				case FP_IDY_SEACH_FP_WAIT_ACK:
						if(LdFpIdyStr.Ack!=0)
						{
								temp[0]=LdFpIdyStr.Ack&0x7F;
								if(temp[0]==PS_OK)                         //生成特征正常
								{
									
								}
								if(temp[0]==PS_NOT_SEARCHED)               //没有搜素到
								{
										
								}
								LdFpIdyStr.Stat=FP_IDY_DONE;
						}
						else if(GetTimerElapse(LdFpIdyStr.StartTime)>1500)   //超时时间500ms
						{
								LdFpIdyStr.Stat=FP_IDY_DONE;
						}
						break;
				default:break;
		}
}

void FpRegNewFpStatHandle(void)
{
		uint8_t temp[2]={0};
		
		switch(LdFpRegStr.Stat)
		{
				case FP_REG_DONE:          
						if(LdFpRegStr.CharCount>0)LdFpRegStr.CharCount=0;
						if(LdFpRegStr.StartTime>0)LdFpRegStr.StartTime=0;
						break;
				case FP_REG_WAIT_FG_DOWN:
						//未收到指纹模块应答，等待应答（指令应答时间大概有300ms）
						if(LdFpRegStr.Ack==0)             
						{
								if(GetTimerElapse(LdFpRegStr.StartTime)>2000)        //通讯错误，设备无应答退出
								{
										LdFpRegStr.Stat=FP_REG_DONE;
								}
						}
						if(GetTimerElapse(LdFpRegStr.StartTime)<300)break;    //稍微延时一会：应答大概300MS，延时200MS
						
						//成功收到应答，指纹进入低功耗，等待手指按上引脚电平变高，5秒超时
						if(I2C_ReadGpioData(I2C2,FP_TOUCHOUT_PIN)==Bit_RESET)          
						{
								if(GetTimerElapse(LdFpRegStr.StartTime)>7000)
								{
										LdFpRegStr.Stat=FP_REG_DONE;
								}
						}
						//引脚电平为高，检测到手指
						else
						{
								LdFpRegStr.Stat=FP_REG_GET_IMG;    //转到图像获取
						}
						break;
				case FP_REG_GET_IMG:
						temp[0]=PS_GetImage;
						UartFp_SendCmdData(FP_CMD,temp,1,1,500);
						LdFpRegStr.StartTime=GetTimerCount();
						LdFpRegStr.Stat=FP_REG_GET_IMG_WAIT_ACK;
						LdFpRegStr.Ack=0;
						break;
				case FP_REG_GET_IMG_WAIT_ACK:
						if(LdFpRegStr.Ack!=0)
						{
								temp[0]=LdFpRegStr.Ack&0x7F;                //获取反馈信息
								if(temp[0]==PS_OK)                   //获取图像成功
								{
										LdFpRegStr.Stat=FP_REG_GEN_CHAR;
								}
								if(temp[0]==PS_NO_FINGER)            //手指未按下（可能获取过程中手指离开）
								{
										LdFpRegStr.Stat=FP_REG_WAIT_FG_DOWN;
										Fp_Sleep();
										LdFpRegStr.StartTime=GetTimerCount();
										LdFpRegStr.Ack=0;
								}
								if(temp[0]==PS_GET_IMG_ERR)         //获取图像出错，重新获取 
								{
										LdFpRegStr.Stat=FP_REG_GET_IMG;
								}
						}
						else if(GetTimerElapse(LdFpRegStr.StartTime)>1500)   //指令超时无应答退出
						{
								LdFpRegStr.Stat=FP_REG_DONE;
						}
						break;
				case FP_REG_GEN_CHAR:
						temp[0]=PS_GenChar;
						temp[1]=LdFpRegStr.CharCount+1;
						UartFp_SendCmdData(FP_CMD,temp,2,1,500);
						LdFpRegStr.StartTime=GetTimerCount();
						LdFpRegStr.Stat=FP_REG_GEN_CHAR_WAIT_ACK;
						LdFpRegStr.Ack=0;
						break;
				case FP_REG_GEN_CHAR_WAIT_ACK:
						if(LdFpRegStr.Ack!=0)
						{
								temp[0]=LdFpRegStr.Ack&0x7F;
								if(temp[0]==PS_OK)                         //生成特征正常
								{
										LdFpRegStr.CharCount++;
										if(LdFpRegStr.CharCount>=FP_REG_PRESS_TIMES)     //按压次数到指定数目
										{
												LdFpRegStr.Stat=FP_REG_REG_MODEL;
										}
										else
										{
												Fp_Sleep();
												LdFpRegStr.StartTime=GetTimerCount();
												LdFpRegStr.Ack=0;
												LdFpRegStr.Stat=FP_REG_WAIT_FG_UP; 
										}
								}
								//如下报错信息，可语音提示
								if(temp[0]==PS_FP_DISORDER)                 //图像太乱，生不成特征
								{
										
								}
								if(temp[0]==PS_LITTLE_FEATURE)              //图像正常，特征点太少
								{
										
								}
								if(temp[0]==PS_INVALID_IMAGE)              //图像缓冲区内没有有效原始图而生不成图像
								{
										
								}
								if(temp[0]==PS_RELATE)                     //当前指纹模板与之前模板之间有关联
								{
										
								}
						}
						else if(GetTimerElapse(LdFpRegStr.StartTime)>1500)   //超时时间500ms
						{
								LdFpRegStr.Stat=FP_REG_DONE;
						}
						break;
				case FP_REG_WAIT_FG_UP:
						//未收到指纹模块应答，等待应答（指令应答时间大概有300ms）
						if(LdFpRegStr.Ack==0)             
						{
								if(GetTimerElapse(LdFpRegStr.StartTime)>2000)        //通讯错误，设备无应答退出
								{
										LdFpRegStr.Stat=FP_REG_DONE;
								}
						}
						if(GetTimerElapse(LdFpRegStr.StartTime)<300)break;    //稍微延时一会：应答大概300MS，延时200MS
						//收到应答并且手指未离开
						if(I2C_ReadGpioData(I2C2,FP_TOUCHOUT_PIN)==Bit_RESET)  //手指离开
						{
								LdFpRegStr.Stat=FP_REG_WAIT_FG_DOWN;
						}
						Fp_Sleep();
						LdFpRegStr.StartTime=GetTimerCount();
						LdFpRegStr.Ack=0;
						break;
				case FP_REG_REG_MODEL:
						temp[0]=PS_RegModel;
						UartFp_SendCmdData(FP_CMD,temp,1,1,500);
						LdFpRegStr.StartTime=GetTimerCount();
						LdFpRegStr.Stat=FP_REG_REG_MODEL_WAIT_ACK;
						LdFpRegStr.Ack=0;
						break;
				case FP_REG_REG_MODEL_WAIT_ACK:
						if(LdFpRegStr.Ack!=0)
						{
								temp[0]=LdFpRegStr.Ack&0x7F;
								if(temp[0]==PS_OK)                       //成功
								{
										LdFpRegStr.Stat=FP_REG_DONE;
								}
								if(temp[0]==PS_MERGE_ERR)                //合并失败，不属于同一手指   
								{
										
								}
						}
						else if(GetTimerElapse(LdFpRegStr.StartTime)>1500)   //超时时间500ms
						{
								LdFpRegStr.Stat=FP_REG_DONE;                    //生成特征失败,退出
						}
						break;
				case FP_REG_STORE_FP:
						break;
				case FP_REG_STORE_FP_WAIT_ACK:
						break;
				default:break;
		}
}













