#include "lock_config.h"              
#include "flash.h"
#include "audio.h"
#include "password.h"
#include <string.h>
#include "card.h"
#include "lock_record.h" 
#include "key.h"
#include "eport_drv.h"
#include "bat.h"



//产品型号;硬件版本;软件版本;发行日期;类型beta或release
//通过版本可以判断产品版本，以及产品类型


#ifdef  LIPOWER
const char ver[40]="G3MW_H1.7_S3.0_230224_release";                       //2.0带Boot的OTA版本  
#else
const char ver[40]="G3W_H1.7_S3.0_230224_release";                     //2.0带Boot的OTA版本    
#endif  

SysConfig_t SysConfig={0},SysConfigBk={0};
PeripLock_t PeripLock={0},PeripLockBk={0}; 
TamperAlarm_t TamperAlarm={0};
LowBatAlarm_t LowBatAlarm={0};
LingTime_t LingTime={0};
NetTime_t NetTime={0};
IapInfo_t IapInfo={0};
CoerceFp_t CoerceFp={0};
// DG   22    17        M5       000016    085105
//产地  年  第几周  型号（2-4）  第几台     SN码
char SN[SN_MAX_LEN]={0};  
char SnNum[7]={'7','8','6','9','3','0',0}; 
uint8_t MutiIdyItem=0;
uint8_t SnInvalid=0;
uint8_t IsNewBoot=0;

//2022-09-19
//1.H1.7第一版，修改功能

//2022-09-28
//1.修复外设锁刚锁定就解锁的BUG（计算超时时间<=改为<）。
//2.写内部FLASH死机问题。
//3.电机正反转改方向，跟第一次的样机相反
//4.G3不带人脸和红外，程序上去除人脸相关界面操作，红外暂时不管
//5.红外电源一直打开，远程开关人体感应无效

//2022-10-12
//1.新增上电关锁操作
//2.修复初次上电判断临时密码满的BUG

//2022-10-27
//1.把语音播报淡入淡出数据使用函数计算，不再使用固定数组，已节省16K程序空间
//2.修复误唤醒后进入低功耗功耗异常BUG（外部FLASH引脚未处理，LED OE引脚未处理），多接近20UA

//2022-11-09
//1.音量整体缩减为70%
//2.修复持续验证成功很多次后电机不转BUG

//2022-11-10
//1.修改指纹接口休眠程序，以适配新版指纹头，不然功耗比较高

//2022-11-18
//1.添加指纹时，增加指纹库已满提示，以前只是提示操作失败

//2022-11-29
//1.修复频繁开锁，导致不自动关锁的BUG
//2.锁体未复位不强制休眠，不然会导致锁未关而进入休眠，导致不关锁
//3.防拆开关打开才能报警

//2022-12-13
//1.超省电模式关掉PIR，关闭超声点模式打开PIR
//2.音量默认低

//2022-12-13
//1.修改BOOT测试
//2.调整BOOT程序，把BOOT升级信息放在内部FLASH，IO_Latch_Clr()放在APP区，修改cpm_handleWakeup中修改部分功能
//3.增加删除用户（用户下多种开锁方式：指纹，密码，人脸，卡片）


//2023-01-06
//1.增加兼容旧版BOOT相关程序
//2.修复可能存在的死机风险
//3.修复语音乱报的问题
//4.增加胁迫密码与胁迫指纹
//5.新版程序搭配的是新boot，但是兼容老版BOOT，不然以后升级会出问题。

//2023-02-06
//1.上电增加20ms延时，防止芯片上电sysinit时间不够（可能由于硬件LDO供电导致）
//2.屏蔽掉PIR相关设置，因为G3没有PIR，如果后面有，再考虑添加
/******************************************************************************/
/*
//更新锁体SN号
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UpdateLockSn(void)
{
		uint8_t i=0,snStart=0;
	
		SnInvalid=0;
		//读序列号数据
		IntFlashRead((uint8_t*)SN,SN_ADDR,SN_MAX_LEN);  
		//找出有效序列号位置
		for(i=31;i>0;i--)
		{
				if(SN[i]!=0)
				{
						snStart=i-5;
						break;
				}
		}
		//获取6字节序列号
		for(i=snStart;i<snStart+6;i++)
		{
				SnNum[i-snStart]=SN[i];
				if(SN[i]<'0' || SN[i]>'9')
				{
						SnInvalid=1;
				}
		}
		//验证激活码是否有效
		printf("SN: %s",SnNum);
}

/******************************************************************************/
/*
//读系统设置信息
input:   none
output   none
return   none 
*/
/******************************************************************************/
void LockReadSysConfig(void)
{
		uint8_t i=0;  
	  //读boot信息，判断Boot版本
		IntFlashRead(IapInfo.DetBuf,IAP_INFO_ADDR,sizeof(IapInfo_t));  
		IsNewBoot=(IapInfo.Det.flag==OTA_FLAG)?1:0; 
		printf("BootType:%d\n",IsNewBoot);
		//读SN
		UpdateLockSn();
		//读设备控制字
		IntFlashRead(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t));           
		if(SysConfig.Bits.Flag!=0xA)        //1010  
		{
				SysConfig.Bits.Activate=0;
				SysConfig.Bits.Language=0;
				SysConfig.Bits.CardLock=0;
				SysConfig.Bits.ChildLock=0;
				SysConfig.Bits.FaceLock=0;
				SysConfig.Bits.MutiIdy=0;
				SysConfig.Bits.FpLock=0;
				SysConfig.Bits.SupPowSave=0;
				SysConfig.Bits.TampAct=1;
				SysConfig.Bits.KeyPADLock=0;
				SysConfig.Bits.KeepOpen=0;
				SysConfig.Bits.Volume=1;              //0-0,1-5,2-10
				SysConfig.Bits.OTCode=0;
				SysConfig.Bits.PrivMode=0;
				SysConfig.Bits.PirOn=1;               //默认打开PIR
				SysConfig.Bits.DiSel=0;
				SysConfig.Bits.TimeZone=8;
				SysConfig.Bits.Rsv=0;                 //保留值0
				SysConfig.Bits.Flag=0xA;
				IntFlashWrite(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t)); 
		}	
		SysConfigBk=SysConfig;
//		if(SysConfig.Bits.Activate==0)SysConfig.Bits.PirOn=0;    //未激活状态关闭PIR
		printf("timezone:%d\n",SysConfig.Bits.TimeZone);
		//设置音量
		Audio_SetVolume(SysConfig.Bits.Volume);
		//读外设锁相关，此处没有对重新上电情况做处理，放在主循环中每秒检测中更新去了。
		IntFlashRead(PeripLock.PripBuf,SYS_PERIP_LOCK_ADDR,sizeof(PeripLock_t)); 
		if(PeripLock.Perip.Flag!=PERIPHLOCK_FLAG)  
		{
				for(i=0;i<4;i++)
				{
						PeripLock.Perip.FailTime[i]=0;
				}
				for(i=0;i<4;i++)
				{
						PeripLock.Perip.FailStart[i].day=0;
						PeripLock.Perip.FailStart[i].hour=0;
						PeripLock.Perip.FailStart[i].minute=0;
						PeripLock.Perip.FailStart[i].second=0;
						PeripLock.Perip.LockStart[i].day=0;
						PeripLock.Perip.LockStart[i].hour=0;
						PeripLock.Perip.LockStart[i].minute=0;
						PeripLock.Perip.LockStart[i].second=0;
				}
				PeripLock.Perip.Flag=PERIPHLOCK_FLAG;
				IntFlashWrite(PeripLock.PripBuf,SYS_PERIP_LOCK_ADDR,sizeof(PeripLock_t)); 
		}
		PeripLockBk=PeripLock;
		//读电池电压
		IntFlashRead(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t)); 
		if(LowBatAlarm.Bat.Flag!=0xAA)
		{
				LowBatAlarm.Bat.Flag=0xAA;
				LowBatAlarm.Bat.Value=0;
				LowBatAlarm.Bat.Day=0;
				LowBatAlarm.Bat.Hour=0;
				LowBatAlarm.Bat.Minute=0;
				LowBatAlarm.Bat.Second=0; 
				IntFlashWrite(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t));   //初始化
		}
		if(LowBatAlarm.Bat.Value>0)     //有保存电压
		{
				mytm_t tm={0};
				tm.day=LowBatAlarm.Bat.Day;
				tm.hour=LowBatAlarm.Bat.Hour;
				tm.minute=LowBatAlarm.Bat.Minute;
				tm.second=LowBatAlarm.Bat.Second;
				if(GetPeripLockTimeElapse(&tm)<=0)    //重新上电全部清0
				{
						LowBatAlarm.Bat.Value=0;
						LowBatAlarm.Bat.Day=0;
						LowBatAlarm.Bat.Hour=0;
						LowBatAlarm.Bat.Minute=0;
						LowBatAlarm.Bat.Second=0; 
				}
				BatPerCent=GetVoltagePercent(LowBatAlarm.Bat.Value);   //唤醒或上电就用以前记录的电池电压计算
		}	
		//读网络时间
		IntFlashRead(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t)); 
		if(NetTime.NT.Flag!=0xAA)          //初始化网络时间结构
		{
				NetTime.NT.Stat=0;
				NetTime.NT.Day=0;
				NetTime.NT.Hour=0;
				NetTime.NT.Minute=0;
				NetTime.NT.Second=0;
				NetTime.NT.Flag=0xAA;
				IntFlashWrite(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t)); 
		}
		else if(NetTime.NT.Stat==1)       //已经获取过时间，判断是否有重新上电
		{
				mytm_t tm={0};
				tm.day=NetTime.NT.Day;
				tm.hour=NetTime.NT.Hour;
				tm.minute=NetTime.NT.Minute;
				tm.second=NetTime.NT.Second;
				if(GetPeripLockTimeElapse(&tm)<=0)    //重新上电全部清0
				{
						NetTime.NT.Stat=0;
						NetTime.NT.Day=0;
						NetTime.NT.Hour=0;
						NetTime.NT.Minute=0;
						NetTime.NT.Second=0;
						NetTime.NT.Flag=0xAA;
						IntFlashWrite(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t));
				}
		}
		for(i=0;i<8;i++)printf("data:0x%02x ",NetTime.NTBuf[i]);
		printf("nt: day:%02d,hour:%02d,min:%02d,sec:%02d\n",NetTime.NT.Day,NetTime.NT.Hour,NetTime.NT.Minute,NetTime.NT.Second);
		//读徘徊报警
		IntFlashRead(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
		if(LingTime.LTT.Flag!=0x55)          //初始化徘徊时间
		{
				LingTime.LTT.Stat=0;
				LingTime.LTT.TrigT=10;           //默认报警触发为10S
				LingTime.LTT.Day=0;
				LingTime.LTT.Hour=0;
				LingTime.LTT.Minute=0;
				LingTime.LTT.Second=0;
				LingTime.LTT.Flag=0x55;
				IntFlashWrite(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
		}
		else if(LingTime.LTT.Stat==1)       //已经触发过徘徊报警
		{
				mytm_t tm={0};
				tm.day=LingTime.LTT.Day;
				tm.hour=LingTime.LTT.Hour;
				tm.minute=LingTime.LTT.Minute;
				tm.second=LingTime.LTT.Second;
				if(GetPeripLockTimeElapse(&tm)<=0)    //重新上电全部清0
				{
						LingTime.LTT.Stat=0;
						LingTime.LTT.Day=0;
						LingTime.LTT.Hour=0;
						LingTime.LTT.Minute=0;
						LingTime.LTT.Second=0;
						LingTime.LTT.Flag=0x55;
						IntFlashWrite(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t));
				}
		}
		//读胁迫指纹标识
		IntFlashRead(CoerceFp.CoerceFpBuf,COERCE_FP_ADDR,sizeof(CoerceFp_t)); 
		if(CoerceFp.CFP.Flag1!=0xAA && CoerceFp.CFP.Flag2!=0x55 && CoerceFp.CFP.Flag3!=0xAA)
		{
				CoerceFp.CFP.Flag1=0xAA;CoerceFp.CFP.Flag2=0x55;CoerceFp.CFP.Flag3=0xAA;
				for(i=0;i<13;i++)
				{
						CoerceFp.CFP.CoerceFp[i]=0;
				}
				IntFlashWrite(CoerceFp.CoerceFpBuf,COERCE_FP_ADDR,sizeof(CoerceFp_t)); 
		}
		//读外部FLASH用户操作记录数据包头
		ExFlashRead(UserOperaHead.OpHeadBuf,USER_RECORD_HEAD_ADDR,sizeof(UserOperaHead_t));
		if(UserOperaHead.OpHead.Flag!=0xAA55 || UserOperaHead.OpHead.WriteIndex>=USER_RECORD_MAX_NUM)
		{
				printf("user record init\n");
				UserOperaHead.OpHead.Flag=0xAA55;
				UserOperaHead.OpHead.WriteIndex=0;
				UserOperaHead.OpHead.RecordSum=0;
				ExFlashWrite(UserOperaHead.OpHeadBuf,USER_RECORD_HEAD_ADDR,sizeof(UserOperaHead_t));
		}
		printf("user record sum: 0x%02x\n",UserOperaHead.OpHead.WriteIndex);
}

/******************************************************************************/
/*
//写系统设置信息
input:   none
output   none
return   none 
*/
/******************************************************************************/
void LockWriteSysConfig(void)
{
		if(SysConfigBk.B32!=SysConfig.B32)
		{
				if(SysConfigBk.Bits.Volume!=SysConfig.Bits.Volume)
				{
						Audio_SetVolume(SysConfig.Bits.Volume);
				}
				LockModifySysConfigOpera(SysConfigBk.B32,SysConfig.B32);
				SysConfigBk.B32=SysConfig.B32;
				IntFlashWrite(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t)); 
		}
}
/******************************************************************************/
/*
//写外设锁状态信息
input:   none
output   none
return   none 
*/
/******************************************************************************/
void LockWritePerpLcokInfo(void)
{
		uint8_t i=0;
		for(i=0;i<sizeof(PeripLock_t);i++)
		{
				if(PeripLockBk.PripBuf[i]!=PeripLock.PripBuf[i])
				{
						PeripLockBk=PeripLock;
						IntFlashWrite(PeripLock.PripBuf,SYS_PERIP_LOCK_ADDR,sizeof(PeripLock_t)); 
						return;
				}
		}
}
/***************************************************************************/
/*
-----------------简易产生激活码-------------------
. 先调换SN各个字符的位置，1~3互换，2~4互换，5~6互换
. 产生一个随机整数（1~9）
. 将各个字符的值与随机数相加 
. 对10取余
. 去掉第一个数字 ，用随机数取代
. 用新的数字产生激活码

--------------------------举例-------------------------
SN： 786930
.调换位置 -> 697803
.产生随机数-> 2
.相加 -> [6+2,9+2,7+2，8+2，0+2，3+2]
.取余 -> [8，1，9，0，2，5 ]
.去掉第一个数字 ，用随机数取代 ->[2,1,9,0,2,5]
.产生激活码 219025
*/


/****************************************************************************/

/******************************************************************************/
/*
//激活码校验
input:   ActCode1--激活码6字节字符
output   none
return   1-OK 0-NOK 
*/
/******************************************************************************/
uint8_t CheckSnActiveCode(char *ActCode1) 
{
		uint8_t i=0;
		char newSn[7]={0};
		uint8_t arad=ActCode1[0];        //第一个为随机数
		
		if(SnInvalid==1)return 0;        //SN无效或未烧录SN
		
		//交换顺序
		newSn[0]=SnNum[2]-'0';
		newSn[1]=SnNum[3]-'0';
		newSn[2]=SnNum[0]-'0';
		newSn[3]=SnNum[1]-'0';
		newSn[4]=SnNum[5]-'0';
		newSn[5]=SnNum[4]-'0';
		
		for(i=0;i<6;i++)
		{
				newSn[i]+=(arad-'0');         //加随机数
				newSn[i]%=10;
				newSn[i]+='0';
		}
		newSn[0]=arad;
		
		printf("actcode: %s",newSn);
		
		
		for(i=0;i<6;i++)
		{
				if(newSn[i]!=ActCode1[i])
				{
						break;
				}
		}
		if(i<6)return 0;
		
		return 1;
}
/******************************************************************************/
/*
//未激活状态提示
input:   none
output   none
return   none 
*/
/******************************************************************************/
void SysActCheck(void)
{
		static uint8_t AlarmAct=0;
	
		if(SysConfig.Bits.Activate==0 && AlarmAct==0 && GetTimerElapse(0)>1000)
		{
				AlarmAct=1;
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"请激活产品"),UNBREAK);
		}
}

/******************************************************************************/
/*
//恢复出厂设置
input:   none
output   none
return   none 
*/
/******************************************************************************/
void FactoryReset(void)
{
		uint16_t i=0;

		//sysconfig设置
//		SysConfig.Bits.Activate=0; 保留系统激活信息，其他还原到默认设置
		SysConfig.Bits.Language=0;
		SysConfig.Bits.CardLock=0;
		SysConfig.Bits.ChildLock=0;
		SysConfig.Bits.FaceLock=0;
		SysConfig.Bits.MutiIdy=0;
		SysConfig.Bits.FpLock=0;
		SysConfig.Bits.SupPowSave=0;
		SysConfig.Bits.TampAct=1;
		SysConfig.Bits.KeyPADLock=0;
		SysConfig.Bits.KeepOpen=0;
		SysConfig.Bits.Volume=1;              //0-0,1-5,2-10
		SysConfig.Bits.OTCode=0;
		SysConfig.Bits.PrivMode=0;
		SysConfig.Bits.PirOn=1;               //默认关PIR
		SysConfig.Bits.DiSel=0;
		SysConfig.Bits.TimeZone=8;            //默认东八区
		SysConfig.Bits.Rsv=0;                 //保留值0
		SysConfig.Bits.Flag=0xA;
//		if(SysConfig.Bits.Activate==0)SysConfig.Bits.PirOn=0;    //未激活状态关闭PIR
		IntFlashWrite(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t)); 
		//清外设锁信息
		for(i=0;i<4;i++)
		{
				PeripLock.Perip.FailTime[i]=0;
		}
		for(i=0;i<4;i++)
		{
				PeripLock.Perip.FailStart[i].day=0;
				PeripLock.Perip.FailStart[i].hour=0;
				PeripLock.Perip.FailStart[i].minute=0;
				PeripLock.Perip.FailStart[i].second=0;
				PeripLock.Perip.LockStart[i].day=0;
				PeripLock.Perip.LockStart[i].hour=0;
				PeripLock.Perip.LockStart[i].minute=0;
				PeripLock.Perip.LockStart[i].second=0;
		}
		PeripLock.Perip.Flag=PERIPHLOCK_FLAG;
		IntFlashWrite(PeripLock.PripBuf,SYS_PERIP_LOCK_ADDR,sizeof(PeripLock_t)); 
		//清一次性密码
		memset((uint8_t*)&OneTimeCode,0,sizeof(OneTimeCode_t));   
		IntFlashWrite((uint8_t*)&OneTimeCode,ONETIME_CODE_ADDR,sizeof(OneTimeCode_t));	
		//清防拆信息
		TamperAlarm.Tamp.Flag=0xAA;
		TamperAlarm.Tamp.Time=10;                                    //默认10S报警 
		TamperAlarm.Tamp.Stat=EPORT_ReadGpioData(KEY_TAMPER_PIN);    //读出状态并保存
		TamperAlarm.Tamp.Day=0;
		TamperAlarm.Tamp.Hour=0;
		TamperAlarm.Tamp.Minute=0;
		TamperAlarm.Tamp.Second=0; 
		IntFlashWrite(TamperAlarm.TampBuf,TAMPER_ADDR,sizeof(TamperAlarm_t));
		//清电池电压检测信息
		LowBatAlarm.Bat.Flag=0xAA;
		LowBatAlarm.Bat.Value=0;
		LowBatAlarm.Bat.Day=0;
		LowBatAlarm.Bat.Hour=0;
		LowBatAlarm.Bat.Minute=0;
		LowBatAlarm.Bat.Second=0; 
		IntFlashWrite(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t));   //初始化
		//清网络对时相关
		NetTime.NT.Stat=0;
		NetTime.NT.Day=0;
		NetTime.NT.Hour=0;
		NetTime.NT.Minute=0;
		NetTime.NT.Second=0;
		NetTime.NT.Flag=0xAA;
		IntFlashWrite(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t)); 
		//清徘徊报警相关
		LingTime.LTT.Stat=0;
		LingTime.LTT.TrigT=10;           //默认报警触发为10S
		LingTime.LTT.Day=0;
		LingTime.LTT.Hour=0;
		LingTime.LTT.Minute=0;
		LingTime.LTT.Second=0;
		LingTime.LTT.Flag=0x55;
		IntFlashWrite(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
		//清胁迫
		CoerceFp.CFP.Flag1=0xAA;CoerceFp.CFP.Flag2=0x55;CoerceFp.CFP.Flag3=0xAA;
		for(i=0;i<13;i++)
		{
				CoerceFp.CFP.CoerceFp[i]=0;
		}
		IntFlashWrite(CoerceFp.CoerceFpBuf,COERCE_FP_ADDR,sizeof(CoerceFp_t));
		//清用户密码
		memset(Pwd.PwdGenStr[0].PwdBuf,0,PWD_MAX*sizeof(PwdGenStr_t));    //清空密码
		IntFlashWrite(Pwd.PwdGenStr[0].PwdBuf,PWD_DATA_ADDR,PWD_MAX*sizeof(PwdGenStr_t));
		//清时效密码
		memset(Pwd.PwdTimStr[0].PwdTimBuf,0,PWD_TIM_MAX*sizeof(PwdExtStr_t));
		IntFlashWrite(Pwd.PwdTimStr[0].PwdTimBuf,PWD_TIM_DATA_ADDR,PWD_TIM_MAX*sizeof(PwdExtStr_t));
		//清卡
		memset(CardUser,0,sizeof(CardUser));
		IntFlashWrite((uint8_t*)&CardUser,CARD_DATA_ADDR,sizeof(CardUser));
}




















