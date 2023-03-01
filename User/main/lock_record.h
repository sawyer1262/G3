#ifndef  __LOCK_RECORD_H_
#define  __LOCK_RECORD_H_


#include <stdint.h>
#include "lock_config.h"


typedef enum{
		CTL_OPEN_LOCK       = 0,
		CTL_CLOSE_LOCK,
    CTL_DOOR_BELL	
}LcokCtlType_t;


//用户操作存储
typedef enum{
			//系统设置相关
			LANG_SET         = 1,           //语言设置
			ACTIVATE_SET     = 2,           //产品激活
			KEEP_OPEN_SET    = 3,           //常开模式设置         //暂时没有语音和界面，不加
			CHILD_LOCK_SET   = 4,           //童锁设置  
			MUTILIDY_SET     = 5,           //双验证模式设置 
			POWERSAVE_SET    = 6,           //超省电模式
			TAMPER_SET       = 7,           //防拆设置 
			FACE_LOCK_CHG    = 8,           //人脸模块锁定  
			FP_LOCK_CHG      = 9,           //指纹模块锁定
			KEYPAD_LOCK_CHG  = 10,          //键盘锁定
			CARD_LOCK_CHG    = 11,          //刷卡锁定
			VOLUME_SET       = 12,          //音量设置 
			OTCODE_CHG       = 13,          //一次性密码状态
			PRIVMODE_SET     = 14,          //隐私模式设置
			PIR_SET          = 15,          //PIR开关
			//添加用户
			ADD_USER         = 16,          //添加用户
			DEL_USER         = 17,          //删除用户
			//开锁
			OPEN_LOCK        = 18,          //开锁 
			//门铃
			DOOR_BELL        = 19,          //门铃   
			//防拆触发    
			TAMP_TRIG        = 20,          //防拆触发
			//恢复出厂设置
			FACTORY_RESET    = 21,           //恢复出厂设置
			
			OPERA_RESV        
}UserOperaType_t;

typedef union{
		struct{
				uint8_t isChinese;            //中英文标识
		}LAN;
		struct{
				uint8_t isActivate;           //激活
		}ACT;
		struct{
				uint8_t isEnKeepOpen;         //常开标识 
		}KP;
		struct{
				uint8_t isEnChildLock;        //童锁开关
		}CL;
		struct{
				uint8_t isEnMutiIdy;          //双重验证 
		}MI;
		struct{
				uint8_t isEnPowSave;          //超省电模式
		}PS;
		struct{
				uint8_t isEnTamp;             //防拆开关使能状态改变   
		}TP;
		struct{
				uint8_t isTampTrig;           //防拆触发  
		}TPTRIG;
		struct{
				uint8_t isFaceLock;           //人脸识别锁定
		}FACEL;
		struct{
				uint8_t isFpLock;             //指纹模块锁定 
		}FPL;
		struct{
				uint8_t isKeypadLock;         //键盘锁定   
		}KEYL;
		struct{
				uint8_t isCardLock;           //刷卡锁定  
		}CARDL;
		struct{
				uint8_t volume;               //音量设置
		}VOLU;
		struct{
				uint8_t isBell;               //门铃触发  
		}DB;
		struct{
				uint8_t isFactoryReset;       //恢复出厂设置触发
		}FR;
		struct{
				uint8_t isEnPrivMode;         //隐私模式  
		}PRIV;
		struct{
				uint8_t isEnPir;              //pir开关 
		}PIR;
		struct{
				uint8_t isAdd;                //添加或删除
		}OTCODE;                          //一次性密码  
		struct{
				uint8_t Type;                 //类型：人脸，指纹，密码，刷卡
				uint8_t Dat[13];              //人脸ID，指纹ID，密码12位+0，刷卡4字节卡号
		}ADUSR;
		struct{
				uint8_t Type;                 //类型：人脸，指纹，密码，刷卡
				uint8_t Dat[13];              //人脸ID，指纹ID，密码12位+0，刷卡4字节卡号
		}DELUSR;
		struct{
				uint8_t Type;                 //类型：人脸，指纹，密码，刷卡
				uint8_t Dat[13];              //人脸ID，指纹ID，密码12位+0，刷卡4字节卡号
		}MODADM;
		struct{
				uint8_t Type;                 //类型：人脸，指纹，密码，刷卡
				uint8_t Dat[13];               //人脸ID，指纹ID，密码12位+0，刷卡4字节卡号
		}OPLOCK;
		uint8_t OpaDatBuf[14];            //最长14字节 
}UserOperaDat_t;


typedef union{
		struct{
				UserOperaType_t OpType;       //操作类型
				UserOperaDat_t OpDat;         //操作相关数据 
				mytm_t OpTime;                 //操作时间
		}OpDet;
		uint8_t OpDetBuf[20];  
}UserOpera_t;   

typedef union{
		struct{
				uint16_t Flag;                //初始化标志 
				uint16_t WriteIndex;          //数据写入指向
				uint32_t RecordSum;           //记录总数：允许大于500，虽然只保存500条
		}OpHead;
		uint8_t OpHeadBuf[8];
}UserOperaHead_t;



extern UserOperaHead_t UserOperaHead;


uint8_t LockWriteUserOpera(UserOpera_t *oprea);
uint8_t LockReadUserOpera(void *out,uint16_t index);
uint8_t UserControlLock(uint8_t ctrlType,uint8_t usrType,uint8_t pIdx);
uint8_t LockAddUser(uint8_t usrType,uint8_t pIdx) ;
uint8_t LockDelUser(uint8_t usrType,uint8_t pIdx);
uint8_t LockFactoryResetOpera(void);
uint8_t LockTampTrigOpera(void);
uint8_t LockModifySysConfigOpera(uint32_t oldVal,uint32_t newVal);
void WriteUserOpera(void);


#endif



