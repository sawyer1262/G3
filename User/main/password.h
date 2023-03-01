#ifndef  __PASSWORD_H_
#define  __PASSWORD_H_


#include <stdint.h>
#include "lock_config.h"
#include "rtc_drv.h"


//密码：100组密码（含胁迫密码）

#define      PWD_LEN_MAX               13
#define      PWD_LEN_MIN               6 
#define      PWD_TIM_LEN_MAX           16

//添加密码返回
#define      PWD_ADD_SUCESS            0x00         //添加成功
#define      PWD_ADD_TYP_WRONG         0x01         //类型错误
#define      PWD_ADD_MEM_FULL          0x02         //密码库已满
#define      PWD_ADD_TIME_MEM_FULL     0x03         //带时效密码库已满
#define      PWD_ADD_CODE_EXIST        0x04         //密码已存在
//删除密码返回 
#define      PWD_DEL_SUCCESS           0x00         //删除密码成功
#define      PWD_DEL_CODE_NOT_EXIST    0x01         //密码不存在
//识别密码返回
#define      PWD_IDY_ONETIME_CODE      0xFE         //一次性密码
#define      PWD_IDY_CODE_NOT_EXIST    0xFF         //密码不存在 
//0-99                                              //返回识别成功的密码编号

typedef enum{
		INVALID,                     //无效密码
		NORMAL,                      //普通密码
		ADMIN,                       //管理员密码
		COERCE,                      //胁迫密码              
		VISITOR,                     //访客密码 
}PwdType_t;

typedef enum{
		INACTIVE,                     //无效数据
		PWD_TIME_CYC,                 //访客密码（周期密码）
		PWD_TIME_FEQ                  //多次密码
}PwdTimType_t;


#pragma pack(push) 	//保存对齐状态
#pragma pack(1)		//设定为1字节对齐

typedef struct{
		mytm_t StartTime;                 //一次性密码开始时间
		uint8_t OneCode[7];               //密码
}OneTimeCode_t;


//通用密码结构
typedef union{
		struct{
				uint8_t Type;                //密码类型，普通密码与胁迫密码
				uint8_t Idx;                 //如果是时效密码，idx表明在时效结构中的位置 
				uint8_t Buf[PWD_LEN_MAX+1];  //密码内容：6-12位，结束0
		}Pwd;
		uint8_t PwdBuf[PWD_LEN_MAX+3];   //密码：为了凑4字节对齐
}PwdGenStr_t;

//时效结构
typedef union{
		struct{
				uint8_t Type;                //PWD_TIME_CYC  
				uint8_t Week;
				uint8_t StartHour;
				uint8_t StartMin;
				uint8_t StopHour;
				uint8_t StopMin;
				uint32_t TimeOutUtc;         //失效时间戳
		}PwdTimCyc;
		struct{
				uint8_t Type;                //PWD_TIME_FEQ
				uint8_t Times;
				uint32_t TimeOutUtc;         //失效时间戳
		}PwdTimeFreq;
		uint8_t PwdTimBuf[PWD_TIM_LEN_MAX];
}PwdExtStr_t;
#pragma pack(pop)//恢复对齐状态

typedef union{
		struct{
				uint16_t PwdSum;
				uint16_t PwdTimSum;
		}Item;
		uint8_t ItemBuf[4];
}PwdInfo_t;


typedef struct{
		PwdInfo_t PwdInfo;                   //密码信息
		PwdGenStr_t PwdGenStr[PWD_MAX];      //密码集合    
		PwdExtStr_t PwdTimStr[PWD_TIM_MAX];  //密码时效集合
}PwdStr_t;



extern PwdStr_t Pwd;
extern OneTimeCode_t OneTimeCode;


void PassWordInit(void);
uint8_t CheckPwdVail(uint8_t* pPwd,uint8_t nLen);
uint8_t CheckPwdTimVail(PwdExtStr_t *time);
uint8_t GetPassWordFreeSlot(void);
uint8_t IdyPassWord(uint8_t *code);
uint8_t AddPassWord(uint8_t type,uint8_t *code,PwdExtStr_t *ext,uint8_t *slot);
uint8_t DelPassWord(uint8_t slot);
void DelAllPassword(void);
uint8_t CheckOneTimeCodeValid(OneTimeCode_t* onecode);
uint8_t GetCodeWithoutX(char *in,char *out);
uint8_t CheckAdminCode(void);
uint8_t CheckOnetimeCodeExist(uint8_t *code);
void CheckTempCodeValid(void);


#endif


