/*
 * @Description: 
 * @version: V1.3
 * @Author: wuyingfeng
 * @Email: yfwu<yingfeng.wu@china-core.com>
 * @Date: 2021-03-21 18:18:18
 * @LastEditTime: 2021-04-20 12:03:30
 */
#ifndef __AUDIO_SERVICE_H_
#define __AUDIO_SERVICE_H_

#include "audio_hal.h"

typedef enum 
{
    BREAK=0,  //打断，
    UNBREAK,  //不打断，一般用于语音组合
}PlayMode_t;


typedef struct{
		uint16_t Number;
		uint32_t StartTime;
		uint32_t Timeout;
}PlayInfo_t;




typedef void* RingBufferHandle_t;
typedef void (*AudioServiceCallback_t)(uint32_t result);

extern volatile PlayInfo_t PlayInfo;
extern volatile uint16_t activeAudioNumber;

extern void AudioServicePlay(uint16_t number,PlayMode_t mode);
extern void AudioServiceRouting(void);
extern void AudioServiceSetVolume(uint8_t volume);
extern _ErrorStatus AudioServiceInit(AudioServiceCallback_t mCallback);
extern void AudioSystickHandler(void);




#endif
