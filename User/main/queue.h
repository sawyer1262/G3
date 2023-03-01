#ifndef  __QUEUE_H_
#define  __QUEUE_H_




#include <stdint.h>


typedef uint8_t (*QueueCCFunc)(void *, void *);


typedef struct
{
		QueueCCFunc AckCallback;                
		uint8_t ItemId;                         
		uint8_t PeripId;                        
		void *data;                            
}Queue_t;





















#endif



