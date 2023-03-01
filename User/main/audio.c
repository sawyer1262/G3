#include "audio_demo.h"
#include "eport_drv.h"
#include "audio.h"
#include "audio_service.h"
#include "lpm.h"
#include "timer.h"
#include "i2c_drv.h"
#include "lock_config.h"
#include "flash.h"
#include "delay.h"
#include "dac_drv.h"
#include "led.h"
#include "keypad_menu.h"


uint16_t AudioPowerOnCount=0;


/******************************************************************************/
/*
//语音播放完成回调函数
input:   number -- 当前播放语音number
output   none
return   none
*/
/******************************************************************************/
void Audio_Donecallback(uint32_t number)
{
		if(number==0)
		{
				if(MenuNow==Menu_KeyPadNormal && SysConfig.Bits.SupPowSave==1) //主菜单
				{
						if(SysConfig.Bits.Volume!=0)SysConfig.Bits.Volume=0;
				}
				LPM_SetStopMode(LPM_AUDIO_ID,LPM_Enable);
		}
}

/******************************************************************************/
/*
//语音播放初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void Audio_Init(void)
{
		GenFadeDat();
		AudioTimerInit(mVoiceFreq);     //初始化定时器
		Audio_PowerOn();
		AudioServiceInit(Audio_Donecallback);
		AudioPowerOnPlayMute();
}
/******************************************************************************/
/*
//语音播放设置音量
input:   none
output   none
return   none
*/
/******************************************************************************/
void Audio_SetVolume(uint8_t sel)
{
		uint8_t vol=0;
		
		if(sel==0)
		{
				vol=0;
		}
		else if(sel==1)
		{
				vol=5;
		}
		else if(sel==2)
		{
				vol=10;
		}
		AudioHalSetVolume(vol);
}

#if 0
void DelayuS(vu32 delayuS)
{
	uint32_t k=0;
	while(delayuS --)
	{
			for(k=0;k<30;k++){;}
	}
}

uint8_t decDat[13312*4]={0};

void AudioTest(void)
{
		uint32_t i=0;
		uint32_t len=13312;
		int16_t dat=0;
		
		I2C_ConfigGpio(I2C3,LED_POWER_PIN,GPIO_OUTPUT);
		I2C_WriteGpioData(I2C3,LED_POWER_PIN,Bit_RESET);
	
		I2C_ConfigGpio(I2C2,AUDIO_EN_PIN,GPIO_OUTPUT);
		Audio_PowerOn();
		DAC_Init(LEFTALIGNED_12BITS, TRIGGER_SOFTWARE, DET_ON_LOW); 
		AudioDecoderInit();
		while(1)         //可以做其他事  
		{
				dat=AudioDecodeProc( SoundData[i] &0x0F );
			  Send_DAC_data((dat<<4)+0x8000); 
				DelayuS(62);		
			
				dat=AudioDecodeProc( SoundData[i] >> 4 );
			  Send_DAC_data((dat<<4)+0x8000);	
				DelayuS(62);	
				i++;

			
				if(i >= 13312)
				{
						printf("audio finish!\n");
						i = 0;
					  AudioDecoderInit();
				}
		}
}
#endif
/******************************************************************************/
/*
//语音播放除能
input:   none
output   none
return   none
*/
/******************************************************************************/
void Audio_DeInit(void)
{
		Audio_PowerOff();
}
/******************************************************************************/
/*
//语音播放打开电源
input:   none
output   none
return   none
*/
/******************************************************************************/
void Audio_PowerOn(void)
{
		if(I2C_ReadGpioData(I2C2,AUDIO_EN_PIN)==Bit_RESET)
		{
				AudioPowerOnCount=0;
				I2C_ConfigGpio(I2C2,AUDIO_EN_PIN,GPIO_OUTPUT);
				DelayMS(5); 
				I2C_WriteGpioData(I2C2,AUDIO_EN_PIN,Bit_SET);
		}
}
/******************************************************************************/
/*
//语音播放关闭电源
input:   none
output   none
return   none
*/
/******************************************************************************/
void Audio_PowerOff(void)
{
		I2C_ConfigGpio(I2C2,AUDIO_EN_PIN,GPIO_OUTPUT);
		DelayMS(5); 
		I2C_WriteGpioData(I2C2,AUDIO_EN_PIN,Bit_RESET);
		AudioPowerOnCount=0;
}

/******************************************************************************/
/*
//语音电源上电计数
input:   none
output   none
return   none
*/
/******************************************************************************/
void Audio_PowerOnCntInc(void)
{
		if(AudioPowerOnCount<150)AudioPowerOnCount++;
}
/******************************************************************************/
/*
//播放语音
input:   num -- 语音编号
         mode -- 播放模式：打断或不打断
output   none
return   none
*/
/******************************************************************************/
void AudioPlayVoice(uint16_t num,PlayMode_t mode)
{
	//	printf("audio num:0x%04x\n",num);
		if(num==0xFFFF || num==0)return;            //未找到相应语音号，退出
		LPM_SetStopMode(LPM_AUDIO_ID,LPM_Disable);
		AudioServicePlay(num,mode);                    //
}
/******************************************************************************/
/*
//语音播放状态处理
input:   none
output   none
return   none
*/
/******************************************************************************/
void Audio_RunningHandle(void)
{
		AudioServiceRouting();
}
/******************************************************************************/
/*
//语音播放字符串
input:   str -- 待播放的数组
output   none
return   none
*/
/******************************************************************************/
void AudioPlayNumString(char* str)
{
		char *ptr=str;
	
		while(*ptr)
		{
				if(*ptr>='0' && *ptr<='9')
				{
						AudioPlayVoice(*ptr-0x30+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK);
				}
				else if(*ptr>='A' && *ptr<='Z')
				{
						AudioPlayVoice(*ptr-0x41+GetVolIndex("A"),UNBREAK);
				}
//				else
//				{
//						AudioPlayVoice(GetVolIndex("出错报警"),BREAK);
//						break;
//				}
				ptr++;
		}
}























