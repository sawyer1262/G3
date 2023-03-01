#ifndef  __TIMER_H_
#define  __TIMER_H_



#include <stdint.h>



extern void AudioSystickHandler(void);
extern uint8_t b20msFlag;
extern uint8_t b100msFlag;
extern uint8_t b1sFlag;


void Timer_Init(void);
void Timer_DeInit(void);
uint32_t GetTimerCount(void);
uint32_t GetTimerElapse(uint32_t start);
void Timer_IntCallback(void);
void BlockDelayMS(uint32_t ms);


#endif


