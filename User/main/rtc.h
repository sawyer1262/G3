#ifndef  __RTC_H_
#define  __RTC_H_



#include "lock_config.h"


void RtcInit(void);
void RtcHandle(void);





int32_t GetPeripLockTimeElapse(mytm_t *stime);
void LockCheckPeripLockStat(void);
void LockUpdatePeripLockStat(uint8_t idx,uint8_t stat);
void SnycNetTime(void);






#endif


