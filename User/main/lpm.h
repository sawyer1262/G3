#ifndef  __LPM_H_
#define  __LPM_H_



#include <stdint.h>


typedef enum
{                                     
		LPM_POWERON_ID            =(1 << 0),             //上电等待
		LPM_KEYPAD_ID             =(1 << 1),             //键盘触摸
		LPM_CARD_ID               =(1 << 2),             //刷卡检测
		LPM_BACKLOCK_RX_ID        =(1 << 3),             //锁板接收后锁数据
		LPM_BACKLOCK_TX_ID        =(1 << 4),             //锁板发送数据
		LPM_BTN_PIR_ID            =(1 << 5),             //红外板LED处理
		LPM_AUDIO_ID              =(1 << 6),             //语音ID
		LPM_BTN_BACK_ID           =(1 << 7),             //后板按键
		LPM_BATCHK_ID             =(1 << 8),             //电池检测
		LPM_FP_ID                 =(1 << 9),             //指纹处理
		LPM_FACE_ID               =(1 << 10),            //人脸处理
		LPM_TAMP_ID               =(1 << 11),            //防拆报警
		LPM_OTA_ID                =(1 << 12),             //OTA升级ID   
		LPM_MOTOR_ID              =(1 << 13),             //OTA升级ID 
}LPM_Id_t;


typedef enum
{
		LPM_Enable=0,
		LPM_Disable,
}LPM_SetMode_t;



/********************************************************************************************/
/*

根据芯片手册，低功耗将使用CPM_PowerOff_1p5模式：该模式唤醒源有以下：

	1>SPI(SS3PAD)低电平
	2>USI1
  3>I2C1
	4>EPORT0:EPORT00-07
	5>TSI TOUCH
	6>RTC定时唤醒
	7>POR

锁板需要得唤醒：
	1>后板UART唤醒:必须接一个中断脚，暂定33脚-GINT3
	2>PIR唤醒:PIR-H1:暂定13脚WAKEUP
//  3>TSI TOUCH唤醒     不需要IO口  
//	4>指纹检测：TOUCHOUT：检测到手指为高电平唤醒。
	7>后板按键：KEY_BACK：进入设置模式按键：暂定34脚，低电平唤醒
	8>防拆按键：Leverage：暂定36脚
	总的唤醒IO有4个：NT_BACK,PIR-H1,KEY_BACK,LEVERAGE
	
	


低功耗处理：

正常工作情况下，所有外设均打开等待操作，休眠时具有唤醒的接口有4个:PIR-H1检测唤醒，后板通讯唤醒NT_BACK,设置按键唤醒KEY_BACK,防拆报警LEVERAGE唤醒



TODO：

周一最好能飞线测试下唤醒引脚，确认能唤醒。

J5座子反了
	
*/
/********************************************************************************************/


extern uint32_t ForcePowerDownCount;

void LPM_EnterLowPower(void);
void LPM_SetStopMode(LPM_Id_t id, LPM_SetMode_t mode);
uint32_t LPM_GetStopModeValue(void);
void LPM_ResettopModeValue(void);


#endif

