/*
 * @Description: 
 * @version: V1.3
 * @Author: wuyingfeng
 * @Email: yfwu<yingfeng.wu@china-core.com>
 * @Date: 2021-03-22 15:51:45
 * @LastEditTime: 2021-04-20 10:11:42
 */

#ifndef _AUDIO_HAL_H_
#define _AUDIO_HAL_H_

#include "audio_drv.h"

typedef enum 
{
    _ERROR = 0, 
    _SUCCESS = 1,
    COMMUNICATIOM_FAIL = 2
}_ErrorStatus;

#define TEST_AUDIO_NUMBER 0xFF

#define ENTER_CRITICAL()  __disable_irq()
#define EXIT_CRITICAL()   __enable_irq()



extern void AudioHalInit(void);
extern void AudioHalFinalize(void);

extern void AudioHalSetVolume(uint8_t volume);
extern uint8_t AudioHalGetCurVolume(void);
extern uint8_t AudioHalGetMaxVolume(void);

extern uint8_t AudioHalSetPlayNumber(uint16_t playNumber);
extern void AudioHalStop(void);

extern _ErrorStatus AudioHalPlay(void);
extern _ErrorStatus AudioHalGetStatus(void);

extern uint32_t AudioHalFlashAddr(uint16_t playNumber, uint32_t * length);
extern void AudioHalDecode(uint8_t *dst, uint8_t *src, uint32_t* length);
extern void AudioHalReadBytes(uint8_t *dst, uint32_t length, uint32_t addr);

void AudioDecoderInit(void);
int16_t AudioDecodeProc(uint8_t code);

#endif
