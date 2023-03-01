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



//��Ʒ�ͺ�;Ӳ���汾;����汾;��������;����beta��release
//ͨ���汾�����жϲ�Ʒ�汾���Լ���Ʒ����


#ifdef  LIPOWER
const char ver[40]="G3MW_H1.7_S3.0_230224_release";                       //2.0��Boot��OTA�汾  
#else
const char ver[40]="G3W_H1.7_S3.0_230224_release";                     //2.0��Boot��OTA�汾    
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
//����  ��  �ڼ���  �ͺţ�2-4��  �ڼ�̨     SN��
char SN[SN_MAX_LEN]={0};  
char SnNum[7]={'7','8','6','9','3','0',0}; 
uint8_t MutiIdyItem=0;
uint8_t SnInvalid=0;
uint8_t IsNewBoot=0;

//2022-09-19
//1.H1.7��һ�棬�޸Ĺ���

//2022-09-28
//1.�޸��������������ͽ�����BUG�����㳬ʱʱ��<=��Ϊ<����
//2.д�ڲ�FLASH�������⡣
//3.�������ת�ķ��򣬸���һ�ε������෴
//4.G3���������ͺ��⣬������ȥ��������ؽ��������������ʱ����
//5.�����Դһֱ�򿪣�Զ�̿��������Ӧ��Ч

//2022-10-12
//1.�����ϵ��������
//2.�޸������ϵ��ж���ʱ��������BUG

//2022-10-27
//1.�������������뵭������ʹ�ú������㣬����ʹ�ù̶����飬�ѽ�ʡ16K����ռ�
//2.�޸����Ѻ����͹��Ĺ����쳣BUG���ⲿFLASH����δ����LED OE����δ��������ӽ�20UA

//2022-11-09
//1.������������Ϊ70%
//2.�޸�������֤�ɹ��ܶ�κ�����תBUG

//2022-11-10
//1.�޸�ָ�ƽӿ����߳����������°�ָ��ͷ����Ȼ���ıȽϸ�

//2022-11-18
//1.���ָ��ʱ������ָ�ƿ�������ʾ����ǰֻ����ʾ����ʧ��

//2022-11-29
//1.�޸�Ƶ�����������²��Զ�������BUG
//2.����δ��λ��ǿ�����ߣ���Ȼ�ᵼ����δ�ض��������ߣ����²�����
//3.���𿪹ش򿪲��ܱ���

//2022-12-13
//1.��ʡ��ģʽ�ص�PIR���رճ�����ģʽ��PIR
//2.����Ĭ�ϵ�

//2022-12-13
//1.�޸�BOOT����
//2.����BOOT���򣬰�BOOT������Ϣ�����ڲ�FLASH��IO_Latch_Clr()����APP�����޸�cpm_handleWakeup���޸Ĳ��ֹ���
//3.����ɾ���û����û��¶��ֿ�����ʽ��ָ�ƣ����룬��������Ƭ��


//2023-01-06
//1.���Ӽ��ݾɰ�BOOT��س���
//2.�޸����ܴ��ڵ���������
//3.�޸������ұ�������
//4.����в��������в��ָ��
//5.�°������������boot�����Ǽ����ϰ�BOOT����Ȼ�Ժ�����������⡣

//2023-02-06
//1.�ϵ�����20ms��ʱ����ֹоƬ�ϵ�sysinitʱ�䲻������������Ӳ��LDO���絼�£�
//2.���ε�PIR������ã���ΪG3û��PIR����������У��ٿ������
/******************************************************************************/
/*
//��������SN��
input:   none
output   none
return   none 
*/
/******************************************************************************/
void UpdateLockSn(void)
{
		uint8_t i=0,snStart=0;
	
		SnInvalid=0;
		//�����к�����
		IntFlashRead((uint8_t*)SN,SN_ADDR,SN_MAX_LEN);  
		//�ҳ���Ч���к�λ��
		for(i=31;i>0;i--)
		{
				if(SN[i]!=0)
				{
						snStart=i-5;
						break;
				}
		}
		//��ȡ6�ֽ����к�
		for(i=snStart;i<snStart+6;i++)
		{
				SnNum[i-snStart]=SN[i];
				if(SN[i]<'0' || SN[i]>'9')
				{
						SnInvalid=1;
				}
		}
		//��֤�������Ƿ���Ч
		printf("SN: %s",SnNum);
}

/******************************************************************************/
/*
//��ϵͳ������Ϣ
input:   none
output   none
return   none 
*/
/******************************************************************************/
void LockReadSysConfig(void)
{
		uint8_t i=0;  
	  //��boot��Ϣ���ж�Boot�汾
		IntFlashRead(IapInfo.DetBuf,IAP_INFO_ADDR,sizeof(IapInfo_t));  
		IsNewBoot=(IapInfo.Det.flag==OTA_FLAG)?1:0; 
		printf("BootType:%d\n",IsNewBoot);
		//��SN
		UpdateLockSn();
		//���豸������
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
				SysConfig.Bits.PirOn=1;               //Ĭ�ϴ�PIR
				SysConfig.Bits.DiSel=0;
				SysConfig.Bits.TimeZone=8;
				SysConfig.Bits.Rsv=0;                 //����ֵ0
				SysConfig.Bits.Flag=0xA;
				IntFlashWrite(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t)); 
		}	
		SysConfigBk=SysConfig;
//		if(SysConfig.Bits.Activate==0)SysConfig.Bits.PirOn=0;    //δ����״̬�ر�PIR
		printf("timezone:%d\n",SysConfig.Bits.TimeZone);
		//��������
		Audio_SetVolume(SysConfig.Bits.Volume);
		//����������أ��˴�û�ж������ϵ����������������ѭ����ÿ�����и���ȥ�ˡ�
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
		//����ص�ѹ
		IntFlashRead(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t)); 
		if(LowBatAlarm.Bat.Flag!=0xAA)
		{
				LowBatAlarm.Bat.Flag=0xAA;
				LowBatAlarm.Bat.Value=0;
				LowBatAlarm.Bat.Day=0;
				LowBatAlarm.Bat.Hour=0;
				LowBatAlarm.Bat.Minute=0;
				LowBatAlarm.Bat.Second=0; 
				IntFlashWrite(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t));   //��ʼ��
		}
		if(LowBatAlarm.Bat.Value>0)     //�б����ѹ
		{
				mytm_t tm={0};
				tm.day=LowBatAlarm.Bat.Day;
				tm.hour=LowBatAlarm.Bat.Hour;
				tm.minute=LowBatAlarm.Bat.Minute;
				tm.second=LowBatAlarm.Bat.Second;
				if(GetPeripLockTimeElapse(&tm)<=0)    //�����ϵ�ȫ����0
				{
						LowBatAlarm.Bat.Value=0;
						LowBatAlarm.Bat.Day=0;
						LowBatAlarm.Bat.Hour=0;
						LowBatAlarm.Bat.Minute=0;
						LowBatAlarm.Bat.Second=0; 
				}
				BatPerCent=GetVoltagePercent(LowBatAlarm.Bat.Value);   //���ѻ��ϵ������ǰ��¼�ĵ�ص�ѹ����
		}	
		//������ʱ��
		IntFlashRead(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t)); 
		if(NetTime.NT.Flag!=0xAA)          //��ʼ������ʱ��ṹ
		{
				NetTime.NT.Stat=0;
				NetTime.NT.Day=0;
				NetTime.NT.Hour=0;
				NetTime.NT.Minute=0;
				NetTime.NT.Second=0;
				NetTime.NT.Flag=0xAA;
				IntFlashWrite(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t)); 
		}
		else if(NetTime.NT.Stat==1)       //�Ѿ���ȡ��ʱ�䣬�ж��Ƿ��������ϵ�
		{
				mytm_t tm={0};
				tm.day=NetTime.NT.Day;
				tm.hour=NetTime.NT.Hour;
				tm.minute=NetTime.NT.Minute;
				tm.second=NetTime.NT.Second;
				if(GetPeripLockTimeElapse(&tm)<=0)    //�����ϵ�ȫ����0
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
		//���ǻ�����
		IntFlashRead(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
		if(LingTime.LTT.Flag!=0x55)          //��ʼ���ǻ�ʱ��
		{
				LingTime.LTT.Stat=0;
				LingTime.LTT.TrigT=10;           //Ĭ�ϱ�������Ϊ10S
				LingTime.LTT.Day=0;
				LingTime.LTT.Hour=0;
				LingTime.LTT.Minute=0;
				LingTime.LTT.Second=0;
				LingTime.LTT.Flag=0x55;
				IntFlashWrite(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
		}
		else if(LingTime.LTT.Stat==1)       //�Ѿ��������ǻ�����
		{
				mytm_t tm={0};
				tm.day=LingTime.LTT.Day;
				tm.hour=LingTime.LTT.Hour;
				tm.minute=LingTime.LTT.Minute;
				tm.second=LingTime.LTT.Second;
				if(GetPeripLockTimeElapse(&tm)<=0)    //�����ϵ�ȫ����0
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
		//��в��ָ�Ʊ�ʶ
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
		//���ⲿFLASH�û�������¼���ݰ�ͷ
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
//дϵͳ������Ϣ
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
//д������״̬��Ϣ
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
-----------------���ײ���������-------------------
. �ȵ���SN�����ַ���λ�ã�1~3������2~4������5~6����
. ����һ�����������1~9��
. �������ַ���ֵ���������� 
. ��10ȡ��
. ȥ����һ������ ���������ȡ��
. ���µ����ֲ���������

--------------------------����-------------------------
SN�� 786930
.����λ�� -> 697803
.���������-> 2
.��� -> [6+2,9+2,7+2��8+2��0+2��3+2]
.ȡ�� -> [8��1��9��0��2��5 ]
.ȥ����һ������ ���������ȡ�� ->[2,1,9,0,2,5]
.���������� 219025
*/


/****************************************************************************/

/******************************************************************************/
/*
//������У��
input:   ActCode1--������6�ֽ��ַ�
output   none
return   1-OK 0-NOK 
*/
/******************************************************************************/
uint8_t CheckSnActiveCode(char *ActCode1) 
{
		uint8_t i=0;
		char newSn[7]={0};
		uint8_t arad=ActCode1[0];        //��һ��Ϊ�����
		
		if(SnInvalid==1)return 0;        //SN��Ч��δ��¼SN
		
		//����˳��
		newSn[0]=SnNum[2]-'0';
		newSn[1]=SnNum[3]-'0';
		newSn[2]=SnNum[0]-'0';
		newSn[3]=SnNum[1]-'0';
		newSn[4]=SnNum[5]-'0';
		newSn[5]=SnNum[4]-'0';
		
		for(i=0;i<6;i++)
		{
				newSn[i]+=(arad-'0');         //�������
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
//δ����״̬��ʾ
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
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"�뼤���Ʒ"),UNBREAK);
		}
}

/******************************************************************************/
/*
//�ָ���������
input:   none
output   none
return   none 
*/
/******************************************************************************/
void FactoryReset(void)
{
		uint16_t i=0;

		//sysconfig����
//		SysConfig.Bits.Activate=0; ����ϵͳ������Ϣ��������ԭ��Ĭ������
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
		SysConfig.Bits.PirOn=1;               //Ĭ�Ϲ�PIR
		SysConfig.Bits.DiSel=0;
		SysConfig.Bits.TimeZone=8;            //Ĭ�϶�����
		SysConfig.Bits.Rsv=0;                 //����ֵ0
		SysConfig.Bits.Flag=0xA;
//		if(SysConfig.Bits.Activate==0)SysConfig.Bits.PirOn=0;    //δ����״̬�ر�PIR
		IntFlashWrite(SysConfig.B8,SYS_CONFIG_ADDR,sizeof(SysConfig_t)); 
		//����������Ϣ
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
		//��һ��������
		memset((uint8_t*)&OneTimeCode,0,sizeof(OneTimeCode_t));   
		IntFlashWrite((uint8_t*)&OneTimeCode,ONETIME_CODE_ADDR,sizeof(OneTimeCode_t));	
		//�������Ϣ
		TamperAlarm.Tamp.Flag=0xAA;
		TamperAlarm.Tamp.Time=10;                                    //Ĭ��10S���� 
		TamperAlarm.Tamp.Stat=EPORT_ReadGpioData(KEY_TAMPER_PIN);    //����״̬������
		TamperAlarm.Tamp.Day=0;
		TamperAlarm.Tamp.Hour=0;
		TamperAlarm.Tamp.Minute=0;
		TamperAlarm.Tamp.Second=0; 
		IntFlashWrite(TamperAlarm.TampBuf,TAMPER_ADDR,sizeof(TamperAlarm_t));
		//���ص�ѹ�����Ϣ
		LowBatAlarm.Bat.Flag=0xAA;
		LowBatAlarm.Bat.Value=0;
		LowBatAlarm.Bat.Day=0;
		LowBatAlarm.Bat.Hour=0;
		LowBatAlarm.Bat.Minute=0;
		LowBatAlarm.Bat.Second=0; 
		IntFlashWrite(LowBatAlarm.BatBuf,BAT_CHK_ADDR,sizeof(LowBatAlarm_t));   //��ʼ��
		//�������ʱ���
		NetTime.NT.Stat=0;
		NetTime.NT.Day=0;
		NetTime.NT.Hour=0;
		NetTime.NT.Minute=0;
		NetTime.NT.Second=0;
		NetTime.NT.Flag=0xAA;
		IntFlashWrite(NetTime.NTBuf,NET_TIME_ADDR,sizeof(NetTime_t)); 
		//���ǻ��������
		LingTime.LTT.Stat=0;
		LingTime.LTT.TrigT=10;           //Ĭ�ϱ�������Ϊ10S
		LingTime.LTT.Day=0;
		LingTime.LTT.Hour=0;
		LingTime.LTT.Minute=0;
		LingTime.LTT.Second=0;
		LingTime.LTT.Flag=0x55;
		IntFlashWrite(LingTime.LTBuf,LINGER_TIME_ADDR,sizeof(LingTime_t)); 
		//��в��
		CoerceFp.CFP.Flag1=0xAA;CoerceFp.CFP.Flag2=0x55;CoerceFp.CFP.Flag3=0xAA;
		for(i=0;i<13;i++)
		{
				CoerceFp.CFP.CoerceFp[i]=0;
		}
		IntFlashWrite(CoerceFp.CoerceFpBuf,COERCE_FP_ADDR,sizeof(CoerceFp_t));
		//���û�����
		memset(Pwd.PwdGenStr[0].PwdBuf,0,PWD_MAX*sizeof(PwdGenStr_t));    //�������
		IntFlashWrite(Pwd.PwdGenStr[0].PwdBuf,PWD_DATA_ADDR,PWD_MAX*sizeof(PwdGenStr_t));
		//��ʱЧ����
		memset(Pwd.PwdTimStr[0].PwdTimBuf,0,PWD_TIM_MAX*sizeof(PwdExtStr_t));
		IntFlashWrite(Pwd.PwdTimStr[0].PwdTimBuf,PWD_TIM_DATA_ADDR,PWD_TIM_MAX*sizeof(PwdExtStr_t));
		//�忨
		memset(CardUser,0,sizeof(CardUser));
		IntFlashWrite((uint8_t*)&CardUser,CARD_DATA_ADDR,sizeof(CardUser));
}




















