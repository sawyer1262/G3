#ifndef  __MOTOR_H_
#define  __MOTOR_H_



#include <stdint.h>



#define     MOTOR2_PIN       EPORT_PIN23
#define     MOTOR1_PIN       EPORT_PIN24



extern uint16_t ResetMotorDecCnt;



void Motor_Init(void);
void Motor_DeInit(void);
void Motor_Open(void);
void Motor_Close(void);
void Motor_Stop(void);
void Motor_Running(void);




#endif



