#ifndef  __FLASH_CONFIG_H_
#define  __FLASH_CONFIG_H_


#include <stdint.h>
#include "rtc_drv.h"    



#define     SN_MAX_LEN            32                      //22-24位，预留32位
#define     PWD_MAX               100                     //最大100组密码
#define     PWD_TIM_MAX           50                      //额外密码：带时效的，包括临时密码与周期性密码
#define     OTA_FLAG              0x55aa55aa
#define     PERIPHLOCK_FLAG       OTA_FLAG

/********************************************************************************************/
//开锁类型：
typedef enum{
		BACK_OPEN=0,
		FACE_OPEN,
		FP_OPEN,
		CODE_OPEN,
		CARD_OPEN,
}LockOpenType_t;


/********************************************************************************************
*内部Flash 分配
内部flash起始|code|OTA code|用户数据|结束

//EFLASH，用户可用空间0x08000000~0x0807EFFF，共508KB，最后4KB禁止使用，支持封装：ALL
*********************************************************************************************/
#define  EFSH_START_ADDR									  (0x08000000)                                                //EFLASH起始地址 
#define  EFSH_PAGE_SIZE											(512)																												//EFLASH每页大小
#define  EFSH_SIZE 													(0x80000-0x1000)  																					//EFLASH可用空间（最后4K芯片自用）
/*********************************************************************************************/
/*内部falsh存储数据分类和对应的空间大小定义*/
#define  BOOT_SIZE                          (0x8000)                             //BOOTLOADER区域大小，暂时预留32K 
#define  APP_SIZE                           (0x2A000)                            //APP与OTA区域大小
#define  CODE_SIZE													(0x5C000)                            //1个boot区0x8000，一个app区0x2A000，一个backup区0x2A000
#define  SYS_DATA_SIZE											(0x800)                              //预留2K系统设置信息            
#define  SN_ADDR                            (0x79F00)                            //序列号存放位置偏移量
/*内部falsh存储数据分类和对应的空间地址定义*/
#define  SYS_DATA_ADDR											(0+CODE_SIZE)                         //系统设置信息
#define  SYS_CONFIG_ADDR                     SYS_DATA_ADDR                        //LockInfo_t：2字节
#define  SYS_PERIP_LOCK_ADDR                (SYS_CONFIG_ADDR+4)                   //外设锁：48字节
#define  ONETIME_CODE_ADDR                  (SYS_PERIP_LOCK_ADDR+48)              //一次性密码:12字节
#define  TAMPER_ADDR                        (ONETIME_CODE_ADDR+12)                //防拆信息：8字节
#define  BAT_CHK_ADDR                       (TAMPER_ADDR+8)                       //电池电压信息：8字节
#define  NET_TIME_ADDR                      (BAT_CHK_ADDR+8)                      //网络时间状态：8字节 
#define  LINGER_TIME_ADDR                   (NET_TIME_ADDR+8)                     //徘徊报警信息：8字节
#define  COERCE_FP_ADDR                     (LINGER_TIME_ADDR+8)                  //胁迫指纹：16字节  



#define  IAP_INFO_ADDR                      (SYS_DATA_ADDR+2000)
#define  USER_DATA_ADDR											(SYS_DATA_ADDR+SYS_DATA_SIZE)         //用户信息：目前总共：4+1600+800+4+800=3208
//密码：
#define  PWD_INFO_ADDR                       USER_DATA_ADDR                       //密码信息：有效密码与有效时效数量 4字节
#define  PWD_DATA_ADDR                      (USER_DATA_ADDR+4)                    //100组密码，每组16字节            16*100=1600字节 
#define  PWD_TIM_DATA_ADDR                  (PWD_DATA_ADDR+16*PWD_MAX)            //50组时效，每组16字节             16*50=800 
//刷卡
#define  CARD_INFO_ADDR                     (PWD_TIM_DATA_ADDR+16*PWD_TIM_MAX)    //卡信息：卡数量                   4字节
#define  CARD_DATA_ADDR                     (CARD_INFO_ADDR+4)                    //卡用户数据100*8                  800


//外部Flash 分配
//64Mbit---留最后1M为用户数据，暂定用来存储锁体操作
//包括：添加用户，删除用户，系统设置，开锁，已经后板上报的：隐私模式及童锁开启关闭
#define  USER_RECORD_MAX_NUM               500

//用户操作记录500条
#define  USER_EXT_START_ADDR               0x700000                               //
#define  USER_RECORD_HEAD_ADDR             USER_EXT_START_ADDR
#define  USER_RECORD_DAT_ADDR             (USER_RECORD_HEAD_ADDR+8)


//用户升级信息
#define  EX_IAP_INFO_ADDR                    (0x800000-0x40)        //预留外部FLASH的最后64字节为升级信息
/********************************************************************************/

        

//系统配置相关项
typedef union{
		struct{
				uint32_t Language:1;                      //中英文标识 
				uint32_t Activate:1;                      //激活状态
				uint32_t KeepOpen:1;                      //常开模式
				uint32_t ChildLock:1;                     //童锁
				uint32_t MutiIdy:1;                       //双验证                 
				uint32_t SupPowSave:1;                    //超省电模式              
				uint32_t TampAct:1;                       //防拆开关                
				uint32_t FaceLock:1;                      //人脸锁模块锁定标识
				uint32_t FpLock:1;                        //指纹模块锁定标识
				uint32_t KeyPADLock:1;                    //键盘锁定标识
				uint32_t CardLock:1;                      //刷卡锁定标识
				uint32_t Volume:2;                        //音量                    
				uint32_t OTCode:1;                        //一次性密码标志         
				uint32_t PrivMode:1;                      //隐私模式                
			  uint32_t PirOn:1;                         //pir开关                  
				uint32_t DiSel:2;                         //按键音选择
			  int32_t TimeZone:5;                       //时区:-11到+12
				uint32_t Linger:1;                        //徘徊报警 
				uint32_t Rsv:4;                           //预留4位
				uint32_t Flag:4;                          //0101：初始化标识            
		}Bits;
		uint32_t B32;
		uint16_t B16[2];
		uint8_t  B8[4];
}SysConfig_t;

//试错过多相关设置
#pragma pack(push) 	//保存对齐状态
#pragma pack(1)		//设定为1字节对齐
typedef struct {
	UINT16 day;
	UINT8  hour;
	UINT8  minute;
	UINT8  second;
}mytm_t;
#pragma pack(pop)//恢复对齐状态


#define   PERIP_FACE_INDEX        0
#define   PERIP_FP_INDEX          1
#define   PERIP_CODE_INDEX        2
#define   PERIP_CARD_INDEX        3


typedef union{
		struct{
				uint8_t FailTime[4];           //验证错误次数
				uint32_t Flag;                 //初始化标识 
				mytm_t FailStart[4];           //首次失败时间
				mytm_t LockStart[4];           //外设锁开始时间  
		}Perip;
		uint8_t PripBuf[48];
}PeripLock_t;

//徘徊报警设置
typedef union{
		struct{
				uint8_t Flag;               //初始化标识
				uint8_t Stat;               //状态：是否已经触发过徘徊报警，触发后5分钟内不再触发 
				uint8_t TrigT;              //触发时间
				uint8_t Hour;               //时
				uint8_t Minute;             //分 
				uint8_t Second;             //秒 
				uint16_t Day;               //日期
		}LTT;
		uint8_t LTBuf[8];
}LingTime_t;
//防拆相关设置
typedef union{
		struct{
				uint8_t Flag;
				uint8_t Stat;
				uint8_t Time;           
				uint8_t Hour;
				uint8_t Minute;
				uint8_t Second;
				uint16_t Day;
		}Tamp;
		uint8_t TampBuf[8];                
}TamperAlarm_t;
//电池检测相关
typedef union{
		struct{
				uint8_t Flag;                //标识
				uint8_t Hour;                //时
				uint8_t Minute;              //分
				uint8_t Second;              //秒
				uint16_t Day;                //日期
				uint16_t Value;              //电压值
		}Bat;
		uint8_t BatBuf[8];                
}LowBatAlarm_t;
//网络时间相关
typedef union{
		struct{
				uint8_t Flag;               //初始化标识
				uint8_t Hour;               //时
				uint8_t Minute;             //分 
				uint8_t Second;             //秒 
				uint16_t Day;               //日期
				uint16_t Stat;              //状态：是否成功获取过网络时间    
		}NT;
		uint8_t NTBuf[8];
}NetTime_t;

typedef union{
		struct{
				uint8_t Flag1;               //初始化标识
				uint8_t Flag2;
				uint8_t Flag3;
				uint8_t CoerceFp[13];
		}CFP;
		uint8_t CoerceFpBuf[16];
}CoerceFp_t;

//升级相关信息
typedef union{
		struct{
				uint32_t binSum;                 //升级文件总长度
				uint32_t chkSum;                 //升级文件校验
				uint32_t stat;                   //升级状态：0-无升级，1-升级中
				uint32_t pktSum;                 //当前获取的数据长度  
				uint32_t flag;                   //初始化标志 
		}Det;		
		uint8_t DetBuf[20];
}IapInfo_t;


extern uint8_t MutiIdyItem;
extern SysConfig_t SysConfig;
extern SysConfig_t SysConfigBk;
extern PeripLock_t PeripLock;
extern TamperAlarm_t TamperAlarm;
extern LowBatAlarm_t LowBatAlarm;
extern char SN[SN_MAX_LEN];
extern char SnNum[7];
extern const char ver[];
extern IapInfo_t IapInfo;
extern NetTime_t NetTime;
extern uint8_t SnInvalid;
extern uint8_t IsNewBoot;
extern LingTime_t LingTime;
extern CoerceFp_t CoerceFp;

void FactoryReset(void);
void LockReadSysConfig(void);
void LockWriteSysConfig(void);
void LockWritePerpLcokInfo(void);
uint8_t CheckSnActiveCode(char *ActCode1);
int32_t GetPeripLockTimeElapse(mytm_t *stime);
void SysActCheck(void);
void UpdateLockSn(void);






#endif




