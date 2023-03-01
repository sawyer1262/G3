/*
 * @Description: 
 * @version: 
 * @Author: wuyingfeng
 * @Email: yfwu<yingfeng.wu@china-core.com>
 * @Date: 2021-03-22 17:58:24
 * @LastEditTime: 2021-04-20 09:50:01
 */

#ifndef __AUDIO_ARRAY__
#define __AUDIO_ARRAY__

#include "type.h"  
#include "lock_config.h"

#define   AUDIO_FLASH_HEAD_SIZE     8              //语音文件头部信息    
#define   SINGLE_AUDIO_SIZE_MAX     0x569e         //语音文件最长为0x569e,编号142：验证错误次数过多，锁定3分钟


typedef union{
		struct{
				uint32_t addr;           //语音文件起始地址
				uint32_t size;           //语音文件长度
		}Details;
		uint8_t InfoBuf[8];
}AudioItemInfo_t;
/************* User Config Area ***************/
#define AUDIO_COMPRESS      1                       //1:音频需要解压，0音频不需要解压缩
#define mIVoiceBlockNum     8
#define mIVoiceBlockSize    mIVoiceBlockNum * 1024  //8K数据

#if (AUDIO_COMPRESS ==1)
#define mIVoiceBufSize      mIVoiceBlockSize/4
#else
#define mIVoiceBufSize      mIVoiceBlockSize
#endif

#define mVoiceFreq      16000                       //声音采样率为16Khz采样率，CCM4202S最高支持44.1Khz采样率
#define AUDIO_DMA_CH    0                           //音频功能使用的DMA2通道
/****************** END **********************/



extern  unsigned char fadein_data[0x2000];   
extern  unsigned char fadeout_data[0x2000];
extern uint8_t voicemutedata[];
int16_t GetVolIndex(char* in);
void GenFadeDat(void);


#endif
