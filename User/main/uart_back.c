#include "uart_drv.h"
#include "uart_back.h"
#include "uart.h"
#include "timer.h"
#include "eport_drv.h"
#include "lpm.h"
#include "debug.h"
#include "rtc.h"
#include "audio.h"
#include "flash.h"
#include "startup_demo.h"
#include "bat.h"
#include "lock_record.h"
#include <string.h>
#include "key.h"
#include "password.h"
#include "keypad_menu.h"
#include "card.h"
#include "time_cal.h"
#include "uart_face.h"
#include "uart_fp.h"
#include "wdt_drv.h"


UartTxStr_t GmtBackUartTxQ[UART_BACK_TX_Q_SIZE];       //发送队列  
uint8_t BackCurIdx=0;
LockStat_t LockStat;

RemoteDel_t RemoteDel={0,0};
RemoteAdd_t RemoteAdd={0};
RemoteDelSelect_t RemoteDelSelect={0};
//OTA相关
OtaInfo_t OtaInfo={0};
/******************************************************************************/
/*
//后板通讯串口结构初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartBack_DatStrInit(void)
{
		UartStruct[UART_BACK_INDEX].pTxStart=0;
		UartStruct[UART_BACK_INDEX].pTxEnd=0;
		UartStruct[UART_BACK_INDEX].rp=UartStruct[UART_BACK_INDEX].wp=0;
		UartStruct[UART_BACK_INDEX].RxStat=RX_IDLE;
		UartStruct[UART_BACK_INDEX].TxStat=TX_IDLE;
		UartStruct[UART_BACK_INDEX].RtxTimeout=0;
		UartStruct[UART_BACK_INDEX].TxTime=0;
	//	UartStruct[UART_BACK_INDEX].RxTime=0;              //低功耗唤醒时，不要清0
	
		BackCurIdx=0xFF;
}

/******************************************************************************/
/*
//后板通讯串口初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartBack_Init(void)
{
    UART_InitTypeDef UART_InitStruct;
    UART_InitStruct.UART_BaudRate = 9600;
    UART_InitStruct.UART_Mode = UART_INT_MODE;//采用CPU中断方式
		UART_InitStruct.UART_Parity = UART_PARITY_NONE;
		UART_InitStruct.UART_FrameLength = UART_DATA_FRAME_LEN_10BIT;
    UART_InitStruct.UART_StopBits = 1;
		UartStruct[UART_BACK_INDEX].RtxTimeout=50;
		UART_Init(UART_BACK_SCI, &UART_InitStruct);
}

/******************************************************************************/
/*
//后板通讯UART除能
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartBack_Deinit(void)
{
		UART_ConfigGpio(UART_BACK_SCI,UART_RX,GPIO_INPUT);         //RX输入，默认上拉
	
		UART_ConfigGpio(UART_BACK_SCI,UART_TX,GPIO_OUTPUT);        //TX输出高 
		UART_WriteGpioData(UART_BACK_SCI,UART_TX,Bit_SET);
	
		EPORT_ConfigGpio(NT_BACK_PIN,GPIO_INPUT);                  //开启NT_BACK串口唤醒
		EPORT_PullupConfig(NT_BACK_PIN,ENABLE);
		EPORT_Init(NT_BACK_PIN,FALLING_EDGE_INT);
}
/******************************************************************************/
/*
//生成校验
input:   buf -- 需要生成校验的数据包
         len -- 数据包长度
output   none
return   16位校验
*/
/******************************************************************************/
uint16_t GenGMTChksum(uint8_t* buf,uint16_t len)
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
//后板串口发送数据
input:   buf -- 数据缓存
         len -- 数据长度
output   none
return   none 
*/
/******************************************************************************/
void UartBack_SendNbytes(uint8_t* buf,uint16_t len)      
{
		if(UartStruct[UART_BACK_INDEX].TxStat!=TX_IDLE)return;
		
		UartStruct[UART_BACK_INDEX].TxStat=TX_RUNNING;
		UartStruct[UART_BACK_INDEX].pTxStart=buf;
		UartStruct[UART_BACK_INDEX].pTxEnd=buf+len;
		UartStruct[UART_BACK_INDEX].TxTime=GetTimerCount();
		UART_BACK_SCI->SCIDRL=*UartStruct[UART_BACK_INDEX].pTxStart++;
		UART_BACK_SCI->SCICR2|=SCICR2_TIE_MASK;  
}

/******************************************************************************/
/*
//后板串口发送指令
input:   cmd -- 指令类型：见协议文档
         buf -- 指令附加数据
         len -- 指令附加数据长度
ack -- 发送指令是否需要应答：1-需要， 0-不需要， 2-应答包，需要立即发送
         timeout -- 等待应答超时时间，单位为毫秒
output   none
return   1 -- 添加指令成功   0 -- 发送队列已满 
*/
/******************************************************************************/
uint8_t UartBack_SendCmdData(uint8_t cmd,uint8_t *buf,uint8_t len,uint8_t ack,uint32_t timeout)
{
		uint8_t i=0,j=0;
		uint16_t chksum=0;
		
		for(j=0;j<UART_BACK_TX_Q_SIZE;j++)
		{
				if(GmtBackUartTxQ[j].DatStat==0)
				{
						GmtBackUartTxQ[j].DatStat=1;
						GmtBackUartTxQ[j].NeedAck=ack;
						GmtBackUartTxQ[j].Timeout=timeout;
						GmtBackUartTxQ[j].DatBufLen=UART_BACK_WAKE_SIZE+len+7;
						GmtBackUartTxQ[j].Retry=0;
						for(i=0;i<UART_BACK_WAKE_SIZE;i++)
						{
								GmtBackUartTxQ[j].DatBuf[i]=0;
						}
						GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE]=GMT_LOCK_HEAD_8_L;
						GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE+1]=GMT_LOCK_HEAD_8_H;  
						GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE+2]=len+1;            
						GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE+3]=0;
						GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE+4]=cmd;
						for(i=0;i<len;i++)
						{
								GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE+5+i]=buf[i];
						}
						chksum=GenGMTChksum(&GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE],len+5);
						GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE+5+i]=chksum;
						GmtBackUartTxQ[j].DatBuf[UART_BACK_WAKE_SIZE+6+i]=chksum>>8;
						
						LPM_SetStopMode(LPM_BACKLOCK_TX_ID,LPM_Disable);
						return 1;
				}
		}
		return 0;
}
/******************************************************************************/
/*
//后板串口数据发送处理
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartBack_DataSendHandle(void)
{
		uint8_t i=0;
		
		if(UartStruct[UART_BACK_INDEX].TxStat!=TX_IDLE)return;
		
		//检测是否有需要立即发送的应答
		for(i=0;i<UART_BACK_TX_Q_SIZE;i++)
		{
				if(GmtBackUartTxQ[i].DatStat==1 && GmtBackUartTxQ[i].NeedAck==2)        
				{
						UartBack_SendNbytes(GmtBackUartTxQ[i].DatBuf,GmtBackUartTxQ[i].DatBufLen);
						GmtBackUartTxQ[i].DatStat=0;
						BackCurIdx=0xFF;
						return;
				}
		}
		//检测是否有需要应答的数据还在等待应答
		if(BackCurIdx<UART_BACK_TX_Q_SIZE)
		{
				if(GetTimerElapse(GmtBackUartTxQ[BackCurIdx].TxTime)<GmtBackUartTxQ[BackCurIdx].Timeout)return;
				
				UartBack_SendNbytes(GmtBackUartTxQ[BackCurIdx].DatBuf,GmtBackUartTxQ[BackCurIdx].DatBufLen);
				GmtBackUartTxQ[BackCurIdx].TxTime=GetTimerCount();
				if(++GmtBackUartTxQ[BackCurIdx].Retry>=3)
				{
						GmtBackUartTxQ[BackCurIdx].DatStat=0;
						BackCurIdx=0xFF;
				}
				return;
		}
		//查找是否有数据需要发送
		for(i=0;i<UART_BACK_TX_Q_SIZE;i++)
		{
				if(GmtBackUartTxQ[i].DatStat==1)
				{
						UartBack_SendNbytes(GmtBackUartTxQ[i].DatBuf,GmtBackUartTxQ[i].DatBufLen);
						if(GmtBackUartTxQ[i].NeedAck==1)
						{
								GmtBackUartTxQ[i].TxTime=GetTimerCount();
								GmtBackUartTxQ[i].Retry++;
								BackCurIdx=i;
						}
						else
						{
								GmtBackUartTxQ[i].DatStat=0;
								BackCurIdx=0xFF;
						}
						return;
				}
		}
		if(i==UART_BACK_TX_Q_SIZE)   //缓存没有数据要发送
		{
				LPM_SetStopMode(LPM_BACKLOCK_TX_ID,LPM_Enable);
		}
}
/******************************************************************************/
/*
//接收到的后板数据处理
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UartBack_RxDataHandle(void)
{
		uint8_t pBufRec[UART_RECV_MAX_LEN]={0};
		uint8_t ackDat[20]={0};
		uint8_t *pDatRec=0;
		uint16_t i=0,datlen=0,chksum=0,pktLen=0;
		
		if(UartStruct[UART_BACK_INDEX].RxFrameOk==0)return;
		
		UartStruct[UART_BACK_INDEX].RxFrameOk=0;
	
		LPM_SetStopMode(LPM_BACKLOCK_RX_ID,LPM_Enable);
			
		while(UartStruct[UART_BACK_INDEX].wp != UartStruct[UART_BACK_INDEX].rp)
		{
				pBufRec[i++]=UartStruct[UART_BACK_INDEX].dat[UartStruct[UART_BACK_INDEX].rp++];
				if(UartStruct[UART_BACK_INDEX].rp>=UART_RECV_MAX_LEN)
				{
						UartStruct[UART_BACK_INDEX].rp=0;
				}
		}
		datlen=i;
	//	UartBack_SendNbytes(pBufRec,i);

		for(i=0;i<datlen;i++)
		{
				if(pBufRec[i]==GMT_LOCK_HEAD_8_L)
				{
						if(i+1<datlen && pBufRec[i+1]==GMT_LOCK_HEAD_8_H)
						{
								break;
						}
				}
		}
		datlen-=i;
		if(datlen<GMT_LOCK_FRAME_MIN)return;//最短数据：2字节头+2字节长度+1字节命令+2字节校验
		pDatRec=&pBufRec[i];
		pktLen=((uint16_t)pDatRec[3]<<8)+pDatRec[2];
		chksum=((uint16_t)pDatRec[pktLen+6]<<8)+pDatRec[pktLen+5];
		if(chksum!=GenGMTChksum(pDatRec,pktLen+5))return;   //校验不通过
		
		//UartBack_SendNbytes(pDatRec,datlen);
		if(BackCurIdx<UART_BACK_TX_Q_SIZE && GmtBackUartTxQ[BackCurIdx].NeedAck==1
				&& GmtBackUartTxQ[BackCurIdx].DatBuf[UART_BACK_WAKE_SIZE+4]+1==pDatRec[4])
		{
				GmtBackUartTxQ[BackCurIdx].DatStat=0;
				BackCurIdx=0xFF;
		}
		switch(pDatRec[4])
		{
				case GET_VER:                           //读锁前板版本与ID信息应答，供外部读取
						UartBack_SendCmdData(pDatRec[4]+1,(uint8_t*)&ver[0],strlen(ver),2,1000);
						break;
				case REQ_LOCK_STAT_ACK:                          //读锁状态应答：保存锁状态，后续考虑是否上电读锁状态
				case CTRL_LOCK_ACK:                              //控制锁开关应答：无需处理
						break;
				case LOCK_STAT_REPORT:                           //后板控制前板开锁
						switch(pDatRec[5])
						{
								case 1:                                  //只有开锁指令有效 
										UserControlLock(CTL_OPEN_LOCK,BACK_OPEN,0);
										break;
								default:
										break;
						}
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],0,2,1000);    //应答
						break;
				case REQ_TIME_ACK:                           //读锁时间应答
						if(pDatRec[5]!=0 || pDatRec[6]!=0 || pDatRec[7]!=0 || pDatRec[8]!=0)         //时间有效
						{
								uint32_t timeUTC=0;
								t_ctimes st = {0};
								tm rtc_times={0};
								
								timeUTC=(uint32_t)pDatRec[8]<<24;
								timeUTC|=(uint32_t)pDatRec[7]<<16;
								timeUTC|=(uint32_t)pDatRec[6]<<8;
								timeUTC|=pDatRec[5];
								
								timeUTC+=SysConfig.Bits.TimeZone*3600;   //把时区计算进去
								
								utc_to_mytime(timeUTC,&st,0);
								mytime_to_rtc(st,&rtc_times);
								RTC_SetTime(rtc_times);
								
								NetTime.NT.Stat=1;
								NetTime.NT.Day=rtc_times.day;
								NetTime.NT.Hour=rtc_times.hour;
								NetTime.NT.Minute=rtc_times.minute;
								NetTime.NT.Second=rtc_times.second;
								NetTime.NT.Flag=0xAA;
								IntFlashWrite(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t)); 
						}
						break;
				case USER_CHG_REPORT_ACK:                         //添加删除用户上报应答
				case CONFIG_NET_ACK:                              //网络设置应答
				case CTRL_FAST_SLEEP_ACK:                         //快速休眠应答
				case CTRL_POWERSAVE_ACK:                          //超省电模式开关应答
						break;
				case NET_STAT_REPORT:                             //上报网络状态
						AudioPlayVoice(GetVolIndex(pDatRec[5]?"连接成功":"WIFI连接失败"),UNBREAK);
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],0,2,1000);    //应答
						break;
				case CHILDLOCK_STAT_REPORT:                       //童锁功能是锁后板具有得，前板只是在开关童锁是给与提示，并保存该设置
						SysConfig.Bits.ChildLock=pDatRec[5]?1:0;
						if(SysConfig.Bits.ChildLock)
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Child lock is on":"童锁已开启"),UNBREAK);
						}
						else
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Child lock is off":"童锁已解除"),UNBREAK);
						}
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],0,2,1000);
						break;
				case PRIV_STAT_REPORT:                                                  //锁后板开启或关闭隐私模式
						SysConfig.Bits.PrivMode=pDatRec[5]?1:0;
						if(SysConfig.Bits.PrivMode)
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn on privacy mode":"开启隐私模式"),UNBREAK);
						}
						else
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn off privacy mode":"关闭隐私模式"),UNBREAK);
						}
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],0,2,1000);
						break;
				case SET_SN_NUMBER:            //烧SN号
						if(pktLen>SN_MAX_LEN)break;        //最长32位
						for(i=0;i<pktLen;i++)
						{
								SN[i]=pDatRec[5+i];            //保存SN数据
						}
						for(;i<SN_MAX_LEN;i++)
						{
								SN[i]=0;                       //多余字节清0 
						}
						SysConfig.Bits.Activate=0;         //写SN，清激活标志
						IntFlashWrite((uint8_t*)SN,SN_ADDR,SN_MAX_LEN); 
						UpdateLockSn();
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],pktLen,2,1000);
						break;
				case SET_BOOT_MODE:            //进入ROM启动模式
						if(pDatRec[5]==0x11 && pDatRec[6]==0x22)
						{
								Demo_StartupConfig(Startup_From_ROM, ENABLE);
						}
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],0,2,1000);
						break;
				case GET_SN_NUMBER:            //读SN
						UartBack_SendCmdData(pDatRec[4]+1,(uint8_t*)SN,strlen(SN),2,1000);
						break;
				case GET_OP_RECORD:            //读用户操作记录
						LockReadUserOpera(ackDat,pDatRec[5]);
						UartBack_SendCmdData(pDatRec[4]+1,ackDat,20,2,1000);
						break;
				case CLEAR_ACT_FLAG:            //清激活标志
						if(pDatRec[5]==0xa5 && pDatRec[6]==0x5a)
						{
								SysConfig.Bits.Activate=0;
								UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],2,2,1000);
						}
						break;
				case READ_CFG:                 //读锁设置  
				{
						tm RT={0};
						t_ctimes gt={0};
						RTC_GetTime(&RT);	//获取芯片RTC时间
						rtctime_to_mytime(&gt,RT);	//获取时间戳
						ackDat[0]=SysConfig.B8[0];
						ackDat[1]=SysConfig.B8[1];
						ackDat[2]=SysConfig.B8[2];
						ackDat[3]=SysConfig.B8[3];
						ackDat[4]=BatPerCent;
						ackDat[5]=(uint8_t)gt.years;
						ackDat[6]=(uint8_t)(gt.years>>8);
						ackDat[7]=gt.months;
						ackDat[8]=gt.days;
						ackDat[9]=gt.hours;
						ackDat[10]=gt.minutes;
						ackDat[11]=gt.seconds;
						ackDat[12]=gt.weeks;
						ackDat[13]=Pwd.PwdInfo.Item.PwdSum;
						ackDat[14]=Pwd.PwdInfo.Item.PwdTimSum;
						ackDat[15]=CardInfo.Card.CardSum;
						ackDat[16]=FaceUser.Sum;
						ackDat[17]=LdFp.ActFpSum;
						ackDat[18]=LockStat.Lock;
						ackDat[19]=TamperAlarm.Tamp.Time;
						UartBack_SendCmdData(pDatRec[4]+1,ackDat,20,2,1000);
						break;
				}
				case SET_CFG:
						switch(pDatRec[5])
						{
//								case 0:            //PIR开关,G3没有PIR
//										SysConfig.Bits.PirOn=pDatRec[6]?1:0;
//										if(SysConfig.Bits.PirOn==1)
//										{
//												PowerOnPir();
//												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),BREAK);       
//										}
//										else 
//										{
//												PowerOffPir();
//												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),BREAK);       
//										}
//										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Human body induction":"人体感应"),UNBREAK);     
//										break;
								case 1:            //音量设置
										if(pDatRec[6]<3)
										{
												SysConfig.Bits.Volume=pDatRec[6];
												if(pDatRec[6]==1)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Low volume":"音量低"),BREAK); 
												}
												else if(pDatRec[6]==2)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"High volume":"音量高"),BREAK); 
												}
												else
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Mute":"静音"),BREAK);
												}
												Audio_SetVolume(SysConfig.Bits.Volume);	
										}
										break;
								case 2:            //APP开启常开模式
										SysConfig.Bits.KeepOpen=pDatRec[6]?1:0;
										if(SysConfig.Bits.KeepOpen)
										{
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),UNBREAK);
										}
										else
										{
												UserControlLock(CTL_CLOSE_LOCK,0xff,0xff);    //关闭常开模式，电机复位
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);
										}
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"keep open mode":"常开模式"),UNBREAK);
										break;
								case 3:     //超省电设置
										SysConfig.Bits.SupPowSave=pDatRec[6]?1:0;
										if(SysConfig.Bits.SupPowSave)
										{
//												SysConfig.Bits.Volume=0;          //此处不设置声音，不然下面得语音播放不出来
//												Audio_SetVolume(SysConfig.Bits.Volume);
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),UNBREAK);
//												if(SysConfig.Bits.PirOn==1)
//												{
//														SysConfig.Bits.PirOn=0;
//														PowerOffPir();
//												}
										}
										else
										{
												SysConfig.Bits.Volume=1;
												Audio_SetVolume(SysConfig.Bits.Volume);
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);
//												if(SysConfig.Bits.PirOn==0)
//												{
//														SysConfig.Bits.PirOn=1;
//														PowerOnPir();
//												}
										}
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Ultra power saving mode":"超省电模式"),UNBREAK);
										break;
								case 4:          //防拆开关
										SysConfig.Bits.TampAct=pDatRec[6]?1:0;
										break;
								case 5:         //防拆报警时间   
										if(pDatRec[6]>=5 && pDatRec[6]<=35)
										{
												TamperAlarm.Tamp.Time=pDatRec[6];
												IntFlashWrite(TamperAlarm.TampBuf,TAMPER_ADDR,sizeof(TamperAlarm_t));
										}
										break;
								case 6:         //设置时区  
										if((int8_t)pDatRec[6]>=-11 && (int8_t)pDatRec[6]<=12)
										{
												SysConfig.Bits.TimeZone=(int8_t)pDatRec[6];
										}
										break;
								default:break;
						}
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],2,2,1000);
						break;
				case APP_MANAGE_ADMIN_CODE:   //修改管理员密码
						if(0!=IdyPassWord(&pDatRec[5]))     //校验旧密码
						{
								ackDat[0]=2;
								UartBack_SendCmdData(pDatRec[4]+1,ackDat,2,2,1000);      //密码验证不通过，添加失败
								break;
						}
						for(i=0;i<pktLen;i++)               //查找新密码起始地址
						{
								if(pDatRec[5+i]==0 && pDatRec[6+i]!=0)break;
						}  
						AddPassWord(ADMIN,&pDatRec[6+i],NULL,&ackDat[1]);
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
						ackDat[0]=1;                                                     //添加成功
						UartBack_SendCmdData(pDatRec[4]+1,ackDat,2,2,1000); 
						break;
				case APP_MANAGE_NORMAL_USER:
						if(pDatRec[5]==0 || pDatRec[5]>2)break;          //错误数据
						if(pDatRec[5]==1)           //添加用户
						{
								switch(pDatRec[6])
								{
//										case 0x01:          //添加人脸
//												RemoteAdd.type.face=1;
//												MenuNow=Menu_KeyPadAddUsers_Face;
//												break;
										case 0x02:          //添加指纹 
												RemoteAdd.type.fp=1;
												MenuNow=Menu_KeyPadAddUsers_Fp;
												break;
										case 0x04:          //添加卡片 
												RemoteAdd.type.card=1;
												MenuNow=Menu_KeyPadAddUsers_Card;
												break;
										case 0x03:          //添加密码 
												switch(AddPassWord(NORMAL,&pDatRec[7],NULL,&ackDat[2]))
												{
														case PWD_ADD_SUCESS:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
																break;
														case PWD_ADD_CODE_EXIST:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password already exists":"密码已存在"),UNBREAK);
																break;
														case PWD_ADD_MEM_FULL:
														case PWD_ADD_TIME_MEM_FULL:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password library is full":"密码库已满"),UNBREAK);
																break;
														case PWD_ADD_TYP_WRONG:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
																break;
												}
												break;
										case 0x05:          //添加多次密码
										{
												PwdExtStr_t ext={0};
												memcpy(&ackDat[3],&pDatRec[7],6);            //固定6字节密码   
												ackDat[9]=0;
												//类型
												ext.PwdTimeFreq.Type=PWD_TIME_FEQ;
												//时间
												ext.PwdTimeFreq.TimeOutUtc=(uint32_t)pDatRec[16]<<24;
												ext.PwdTimeFreq.TimeOutUtc|=(uint32_t)pDatRec[15]<<16;
												ext.PwdTimeFreq.TimeOutUtc|=(uint32_t)pDatRec[14]<<8;
												ext.PwdTimeFreq.TimeOutUtc|=pDatRec[13];
												
												ext.PwdTimeFreq.TimeOutUtc+=(int32_t)SysConfig.Bits.TimeZone*3600;        //+时区
												//次数
												ext.PwdTimeFreq.Times=pDatRec[17];
												switch(AddPassWord(VISITOR,&ackDat[3],&ext,&ackDat[2]))
												{
														case PWD_ADD_SUCESS:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
																break;
														case PWD_ADD_CODE_EXIST:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password already exists":"密码已存在"),UNBREAK);
																break;
														case PWD_ADD_MEM_FULL:
														case PWD_ADD_TIME_MEM_FULL:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password library is full":"密码库已满"),UNBREAK);
																break;
														case PWD_ADD_TYP_WRONG:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
																break;
												}
												break;
										}
										case 0x06:        //添加访客密码，周期性
										{
												PwdExtStr_t ext={0};
												memcpy(&ackDat[3],&pDatRec[7],6);            //固定6字节密码   
												ackDat[9]=0;
												//类型
												ext.PwdTimCyc.Type=PWD_TIME_CYC;
												//时间
												ext.PwdTimCyc.TimeOutUtc=(uint32_t)pDatRec[16]<<24;
												ext.PwdTimCyc.TimeOutUtc|=(uint32_t)pDatRec[15]<<16;
												ext.PwdTimCyc.TimeOutUtc|=(uint32_t)pDatRec[14]<<8;
												ext.PwdTimCyc.TimeOutUtc|=pDatRec[13];
												//次数
												ext.PwdTimCyc.Week=pDatRec[17];
												ext.PwdTimCyc.StartHour=pDatRec[18];
												ext.PwdTimCyc.StartMin=pDatRec[19];
												ext.PwdTimCyc.StopHour=pDatRec[20];
												ext.PwdTimCyc.StopMin=pDatRec[21];
												switch(AddPassWord(VISITOR,&ackDat[3],&ext,&ackDat[2]))
												{
														case PWD_ADD_SUCESS:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
																break;
														case PWD_ADD_CODE_EXIST:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password already exists":"密码已存在"),UNBREAK);
																break;
														case PWD_ADD_MEM_FULL:
														case PWD_ADD_TIME_MEM_FULL:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password library is full":"密码库已满"),UNBREAK);
																break;
														case PWD_ADD_TYP_WRONG:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
																break;
												}
												break;
										}
										case 7:              //添加胁迫密码
												switch(AddPassWord(COERCE,&pDatRec[7],NULL,&ackDat[2]))
												{
														case PWD_ADD_SUCESS:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
																break;
														case PWD_ADD_CODE_EXIST:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password already exists":"密码已存在"),UNBREAK);
																ackDat[2]=0xFF;
																break;
														case PWD_ADD_MEM_FULL:
														case PWD_ADD_TIME_MEM_FULL:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password library is full":"密码库已满"),UNBREAK);
																break;
														case PWD_ADD_TYP_WRONG:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
																break;
												}
												break;
										case 8:              //添加胁迫指纹  
												RemoteAdd.type.fp=2;
												MenuNow=Menu_KeyPadAddUsers_Fp;
												break;
										default:break;
								}
						}
						else                        //删除用户
						{
								switch(pDatRec[6])
								{
//										case 0x01:          //删除人脸
//												if(pDatRec[7]<100)
//												{
//														RemoteDel.type.face=1;
//														RemoteDel.id=pDatRec[7];
//														MenuNow=Menu_KeyPadDelUsers_Face;
//												}
//												break;
										case 0x02:          //删除指纹 
												if(pDatRec[7]<100)
												{
														RemoteDel.type.fp=1;
														RemoteDel.id=pDatRec[7];
														MenuNow=Menu_KeyPadDelUsers_Fp;
												}
												break;
										case 0x03:          //删除密码 
										case 0x05:          //删除多次密码
										case 0x06:          //删除访客密码 
												if(pDatRec[7]<100)
												{
														switch(DelPassWord(pDatRec[7]))
														{
																case PWD_DEL_SUCCESS:
																case PWD_DEL_CODE_NOT_EXIST:
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),UNBREAK);
																		break;
																default:break;
														}
												}
												break;
										case 0x04:          //删除卡片 
												if(pDatRec[7]<100)
												{
														switch(DelCard(pDatRec[7]))
														{
																case DEL_CARD_SUCCESS:
																case DEL_CARD_NOEXIST:
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),UNBREAK);
																		break;
																default:break;
														}
												}
												break;
										case 0xAA:          //删除指定多个用户
										{
												RemoteDelSelect.sum=pDatRec[7];          //删除用户的总数
												if(RemoteDelSelect.sum==0 || RemoteDelSelect.sum>25)
												{
														RemoteDelSelect.sum=0;
														break;    //数目超限
												}
												for(i=0;i<RemoteDelSelect.sum;i++)
												{
														RemoteDelSelect.id[i].u8[0]=pDatRec[8+i*2];     //用户类型
														RemoteDelSelect.id[i].u8[1]=pDatRec[8+i*2+1];   //用户编号
												}
												RemoteDelSelect.type.select=1;
												MenuNow=Menu_KeyPadDelUsers_select;
												break;
										}
										case 0xFF:          //删除所有 
												RemoteDel.type.all=1;
												RemoteDel.id=0;
												MenuNow=Menu_KeyPadDelUsers_All;
												break;
										default:break;
								}
								ackDat[2]=pDatRec[7];         //删除时返回ID
						}
						ackDat[0]=pDatRec[5];
						ackDat[1]=pDatRec[6];
						UartBack_SendCmdData(pDatRec[4]+1,ackDat,3,2,1000);
						break;
				case OTA_START:                //OTA启动升级
				{
						uT32_t ut32tmp={0};
						//保存升级状态
						IapInfo.Det.stat=1;
						//保存升级包总长度
						ut32tmp.u8[0]=pDatRec[5];
						ut32tmp.u8[1]=pDatRec[6];
						ut32tmp.u8[2]=pDatRec[7];
						ut32tmp.u8[3]=pDatRec[8];
						IapInfo.Det.binSum=ut32tmp.u32;
						//保存升级包校验
						ut32tmp.u8[0]=pDatRec[9];
						ut32tmp.u8[1]=pDatRec[10];
						ut32tmp.u8[2]=pDatRec[11];
						ut32tmp.u8[3]=pDatRec[12];
						IapInfo.Det.chkSum=ut32tmp.u32;
						//保存当前接收数据长度
						IapInfo.Det.pktSum=IapInfo.Det.binSum/64;
						if((IapInfo.Det.binSum%64)!=0)IapInfo.Det.pktSum+=1;
						//写状态标识
						IapInfo.Det.flag=OTA_FLAG;
						//保存状态
						if(IsNewBoot==1)    //新版BOOT
						{
								IntFlashWrite(IapInfo.DetBuf,IAP_INFO_ADDR,sizeof(IapInfo_t));
						}
						else                                   //旧版BOOT 
						{
								ExFlashWrite(IapInfo.DetBuf,EX_IAP_INFO_ADDR,sizeof(IapInfo_t));
						}
						//清备份区
						IntFlashEraseBackup();
					 //应答
						UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],8,2,1000);
						//更新状态
						LPM_SetStopMode(LPM_OTA_ID,LPM_Disable);
						OtaInfo.Running=1;
						OtaInfo.TimeStart=GetTimerCount();
						OtaInfo.TimeoutCount=0;
						OtaInfo.NextPktNum=0;
						OtaInfo.GotFrame=0;   
						break;
				}
				case OTA_GET_FAME_ACK:
				{
						uT32_t ut32tmp={0};
						if(OtaInfo.NextPktNum!=((uint16_t)pDatRec[6]<<8|pDatRec[5]))
						{
								UartBack_SendCmdData(pDatRec[4]+1,&pDatRec[5],8,2,1000);
								break;       //帧序号不对
						}
						if(OtaInfo.NextPktNum<IapInfo.Det.pktSum)
						{
								IntFlashWrite(&pDatRec[7],BOOT_SIZE+APP_SIZE+OtaInfo.NextPktNum*64,(OtaInfo.NextPktNum==IapInfo.Det.pktSum-1)?(pktLen-2):64);
								OtaInfo.GotFrame=1;
								OtaInfo.NextPktNum++;
								OtaInfo.TimeoutCount=0;
						}
						else
						{
								IapInfo.Det.stat=3;
								//保存升级包校验
								ut32tmp.u8[0]=pDatRec[7];
								ut32tmp.u8[1]=pDatRec[8];
								ut32tmp.u8[2]=pDatRec[9];
								ut32tmp.u8[3]=pDatRec[10];
								IapInfo.Det.chkSum=ut32tmp.u32;
							
								if(IsNewBoot==1)    //新版BOOT
								{
										IntFlashWrite(IapInfo.DetBuf,IAP_INFO_ADDR,sizeof(IapInfo_t));
								}
								else                                   //旧版BOOT 
								{
										ExFlashWrite(IapInfo.DetBuf,EX_IAP_INFO_ADDR,sizeof(IapInfo_t));
								}
							
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"添加成功"),BREAK);
								LPM_SetStopMode(LPM_OTA_ID,LPM_Enable);
								Set_POR_Reset();
						}
						break;
				}
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
void UartBack_RtxTimeoutoutHandle(void)
{
		//防止休眠模式下，后板数据过来还没接收完就进入了低功耗
		if(UartStruct[UART_BACK_INDEX].RxStat==RX_RUNNING && (LPM_GetStopModeValue()&LPM_BACKLOCK_RX_ID)==0)
		{
				LPM_SetStopMode(LPM_BACKLOCK_RX_ID,LPM_Disable);
		}
		//接收处理
		if(UartStruct[UART_BACK_INDEX].RxTime>0 && GetTimerElapse(UartStruct[UART_BACK_INDEX].RxTime)>UartStruct[UART_BACK_INDEX].RtxTimeout)
		{
				UartStruct[UART_BACK_INDEX].RxTime=0;
				UartStruct[UART_BACK_INDEX].RxStat=RX_IDLE;
				UartStruct[UART_BACK_INDEX].RxFrameOk=1;
		}
		//发送处理
		if(UartStruct[UART_BACK_INDEX].TxTime>0 && GetTimerElapse(UartStruct[UART_BACK_INDEX].TxTime)>UartStruct[UART_BACK_INDEX].RtxTimeout)   
		{
				UartStruct[UART_BACK_INDEX].TxTime=0;
				UartStruct[UART_BACK_INDEX].TxStat=TX_IDLE;
				UART_BACK_SCI->SCICR2&=~SCICR2_TIE_MASK;  
		}
		//OTA
		CheckFirmwareUpdate();
}


void CheckFirmwareUpdate(void)
{
		static uint32_t stime=0;
		uint8_t tmp[10]={0};
		
		if(OtaInfo.Running==0)return;
		
		if(OtaInfo.GotFrame==1)
		{
				tmp[1]=OtaInfo.NextPktNum/256;
				tmp[0]=OtaInfo.NextPktNum%256;
				UartBack_SendCmdData(OTA_GET_FAME,tmp,2,0,1000);
				stime=GetTimerCount();
				OtaInfo.GotFrame=0;
		}
		else if(GetTimerElapse(stime)>=4000)
		{
				OtaInfo.GotFrame=1;
				OtaInfo.TimeoutCount++;
				if(OtaInfo.TimeoutCount>=10)
				{
						OtaInfo.Running=0;
						OtaInfo.TimeoutCount=0;
						LPM_SetStopMode(LPM_OTA_ID,LPM_Enable);         //清状态
						IapInfo.Det.stat=0;                             //更新失败，清标志
						if(IsNewBoot==1)    //新版BOOT
						{
								IntFlashWrite(IapInfo.DetBuf,IAP_INFO_ADDR,sizeof(IapInfo_t));
						}
						else                                   //旧版BOOT 
						{
								ExFlashWrite(IapInfo.DetBuf,EX_IAP_INFO_ADDR,sizeof(IapInfo_t));
						}
				}
		}
}




