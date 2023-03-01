/*
 * @Description: 
 * @version: V1.3
 * @Author: wuyingfeng
 * @Email: yfwu<yingfeng.wu@china-core.com>
 * @Date: 2021-03-22 15:50:23
 * @LastEditTime: 2021-04-20 09:51:15
 */

#ifndef __AUDIO_DRV__
#define __AUDIO_DRV__

#include "audio_array.h"
#include "debug.h"
//#include "main.h"

//#ifdef DAC_AUDIO_DEMO
#define DAC_AUDIO_DMA_INT
//#endif

//#define AUDIO_LOG printf



void AudioDrvInit(void);
void AudioDrvStop(void);
void AudioPowerOnPlayMute(void);
void AudioDrvConfig(uint32_t addr,uint32_t length);
void AudioDrvHandle(void);

BOOL AudioBusyStatus(void);

extern void AudioISRHandler(void);

void AudioTimerInit(uint32_t rate); 

#endif 
