#ifndef  __KEYPAD_MENU_H_
#define  __KEYPAD_MENU_H_


#include <stdint.h>

//键盘输入数据最大长度
#define    INPUT_MAX         26
#define    KEY_INPUT_DISABLE_FACESCAN_NUM     3    
//当前运行菜单
typedef void (*MenuFun)(void);

//正常模式与配置模式
typedef enum{
		KEYPAD_NORMAL=0,              //正常模式
		KEYPAD_CONFIG                 //配置模式
}KeyMode_t;

extern MenuFun MenuNow;
extern KeyMode_t KeyPadMode;

//
uint8_t GetKeyLedBit(uint8_t key);
//默认主页菜单
void Menu_KeyPadNormal(void);                      //主菜单
//设置子菜单菜单选择
void Menu_KeyPadConfigSelect(void);                //系统设置选择
//系统设置子菜单
void Menu_KeyPadAddUsers(void);                    //添加用户 
void Menu_KeyPadDelUsers(void);                    //删除用户
void Menu_KeyPadModifyAdmUsers(void);              //修改管理员账户
void Menu_KeyPadConfNet(void);                     //网络设置
void Menu_KeyPadSysConf(void);                     //系统设置
//系统设置子菜单
void Menu_KeyPadSysConf_Security(void);            //安全模式设置
void Menu_KeyPadSysConf_Volume(void);              //音量设置 
void Menu_KeyPadSysConf_Language(void);            //语言设置
void Menu_KeyPadSysConf_PowerSave(void);           //超省电模式设置 
void Menu_KeyPadSysConf_KeepOpen(void);            //常开模式设置
void Menu_KeyPadSysConf_PirSet(void);              //PIR开关设置
//添加用户子菜单：
void Menu_KeyPadAddUsers_Face(void);               //添加人脸
void Menu_KeyPadAddUsers_Code(void);               //添加密码
void Menu_KeyPadAddUsers_Card(void);               //添加卡片  
void Menu_KeyPadAddUsers_Fp(void);                 //添加指纹
void Menu_KeyPadAddUsers_OnetimeCode(void);        //添加一次性密码 
//删除用户子菜单
void Menu_KeyPadDelUsers_Face(void);               //删除人脸
void Menu_KeyPadDelUsers_Fp(void);                 //删除指纹 
void Menu_KeyPadDelUsers_Code(void);               //删除密码 
void Menu_KeyPadDelUsers_Card(void);               //删除卡片
void Menu_KeyPadDelUsers_OnetimeCode(void);        //删除一次性密码
void Menu_KeyPadDelUsers_All(void);                //删除所有用户 
void Menu_KeyPadDelUsers_select(void);             //删除
//修改管理员账户子菜单
void Menu_KeyPadModifyAdmUsers_Face(void);         //修改管理员人脸
void Menu_KeyPadModifyAdmUsers_Fp(void);           //修改管理员指纹 
void Menu_KeyPadModifyAdmUsers_Card(void);         //修改管理员卡
void Menu_KeyPadModifyAdmUsers_Password(void);     //修改管理员密码 



#endif

