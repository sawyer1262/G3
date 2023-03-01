#ifndef __LED_H_                           
#define __LED_H_   
   

#include "eport_drv.h"
#include "i2c_drv.h"
#include "spi_drv.h"
#include "lock_config.h"



#define     PIR_LED_LIGHT_MAX         65535
#define     PIR_LED_LIGHT_MIN         0


//键盘LED：0-14
#define     LED_POWER_PIN     I2C_SCL
#define     LED_SCL_PIN       EPORT_PIN11
#define     LED_SDA_PIN       EPORT_PIN10
#define     LED_LE_PIN        EPORT_PIN9
#define     LED_OE_PIN        EPORT_PIN26            //OE与POWER共用
//红外PIR: R G B
#define     PWM_R_PIN         PWM_PORT2
#define     PWM_G_PIN         PWM_PORT1
#define     PWM_B_PIN         PWM_PORT0

//菜单LED显示MASK
#define     LED_MASK_WAKE                      0x0001                   //唤醒MASK
#define     LED_MASK_NORMAL                    0x3F7F//0xFEFC                   //主界面MASK
#define     LED_MASK_CONFIG_SEL                0X037A//0x5eC0                   //主菜单选择界面
#define     LED_MASK_NETCONF                   0X0032//0x4c00                   //网络设置界面
#define     LED_MASK_SYSCONF                   0X027A//0x5e00                   //系统设置界面
                                                            
#define     LED_MASK_ADDUSER                   0X016A//0x5e80                   //添加用户界面 
#define     LED_MASK_DELUSER                   0X036A//0x5ec0                   //删除用户界面
#define     LED_MASK_MODIFY_ADMIN              0X006A//0x5e00                   //修改管理员界面 

//删除用户
#define     LED_MASK_DELUSER_ONETIMECODE       0X0802//0x4010                   //删除一次性密码界面
#define     LED_MASK_DELUSER_FACE              0X3F7E//0x7EFC                   //删除人脸界面
#define     LED_MASK_DELUSER_FP                0X3F7E//0x7EFC                   //删除指纹界面
#define     LED_MASK_DELUSER_ALL               0X0802//0x4010                   //删除所有用户界面  
#define     LED_MASK_DELUSER_CODE              0X3F7E//0x7EFC                   //删除密码界面
#define     LED_MASK_DELUSER_CARD              0X3F7E//0x7EFC                   //删除卡片界面
//添加用户
#define     LED_MASK_ADDUSER_FACE              0X0002//0x4000                   //添加人脸界面        
#define     LED_MASK_ADDUSER_CODE              0X3F7E//0x7EFC                   //添加密码界面   
#define     LED_MASK_ADDUSER_CARD              0X1102//0x4088                   //添加卡片界面
#define     LED_MASK_ADDUSER_FP                0X0002//0x4000                   //添加指纹 
//系统设置
#define     LED_MASK_SYSCONF_SECURITY          0X0032//0x4c00                   //高级安全设置界面
#define     LED_MASK_SYSCONF_VOLUME            0X0072//0x4e00                   //音量设置界面
#define     LED_MASK_SYSCONF_LANGUAGE          0X0032//0x4c00                   //语言设置界面
#define     LED_MASK_SYSCONF_POWERSAVE         0X0032//0x4c00                   //超省电模式界面
#define     LED_MASK_SYSCONF_KEEPOPEN          0X0032//0x4c00                   //常开模式界面
#define     LED_MASK_SYSCONF_PIRSET            0X0032//0x4c00                   //PIR设置使能界面

//LOGO灯状态
typedef enum{
		LOGO_OFF=0,
		LOGO_WAKEUP,
		LOGO_OPEN_SUCCESS,
		LOGO_OPEN_SUCCESS_MUTIL,                 //多重验证第一次成功  
		LOGO_OPEN_FAIL
}LogoLedStat_t;
//LOGO灯RGB三PWM通道状态
typedef struct{
	uint8_t state;
	uint8_t val;
	uint32_t sttime;
}LedChl_t;
//LED段位
typedef union{
		struct{
				uint16_t BELL:1;                       //BELL 
				uint16_t ASTERISK:1;                   //3
				uint16_t NUM7:1;                       //6
				uint16_t NUM4:1;                       //9
				uint16_t NUM1:1;                       //#
				uint16_t NUM2:1;                       //0
				uint16_t NUM3:1;                       //*
				uint16_t RSV1:1;                       //null
				uint16_t NUM5:1;                       //8
				uint16_t NUM6:1;                       //7 
				uint16_t NUM9:1;                       //4 
				uint16_t HASH:1;                       //1
				uint16_t NUM8:1;                       //5 
				uint16_t NUM0:1;                       //2
				uint16_t BAT:1;                       //LOCK
				uint16_t VIRTU:1;                      //NULL
		}bits;
		uint16_t ledval;
}LedVal_t;



extern LedVal_t LedVal;



void Led_Init(void);
void Led_DeInit(void);
void Led_WakeupDeInit(void);
void Led_UpdateVal(void);
void PirLed_SetBrightness(uint8_t chl,uint8_t percent);
uint8_t PirLed_GetBrightness(uint8_t chl);
void PirLed_Handle(void);
uint8_t PirLed_SetStat(LogoLedStat_t stat);


#endif



