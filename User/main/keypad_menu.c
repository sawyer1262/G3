#include "keypad_menu.h"
#include "key.h"
#include "led.h"
#include <string.h>
#include "audio.h"
#include "lpm.h"
#include "password.h"
#include "uart_back.h"
#include "card.h"
#include "uart_fp.h"
#include "uart_face.h"
#include <math.h>
#include "uart.h"
#include "delay.h"
#include "rtc.h"
#include "lock_record.h"


uint8_t InputBuf[INPUT_MAX];           //输入数据缓存
uint8_t InputIdx=0;                    //键盘输入Index     :注意不要越界。
KeyMode_t KeyPadMode=KEYPAD_NORMAL;    //键盘输入模式
MenuFun MenuNow;                       //当前运行菜单
uint32_t MenuNormalStart=0;            //主界面开始时间
/******************************************************************************/
/*
//主菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadNormal(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0,temp[5]={0};
		uint32_t KeyPressCount=0;
		//设置界面音量
		if(SysConfig.Bits.Volume==0 || SysConfig.Bits.SupPowSave==1)      //静音或者超省电模式
		{
				Audio_SetVolume(0); 
		}
		else
		{
				Audio_SetVolume(SysConfig.Bits.Volume); 
		}
		//复位键盘输入状态
		InputIdx=0;
		//复位人脸状态
		UartFace_PowerOff();
		//复位指纹状态
		FpScan.LdFpStat=FP_IDLE;
		FpScan.StTime=0;
		FpScan.ComRetry=0;
		//复位刷卡状态
		CdStat.StTime=0;
		//复位指纹
		FpScan.LdFpStat=FP_IDLE;
		if(FpPowerDown==1)
		{
				FpPowerDown=0;
				UartFp_DatStrInit();
				UartFp_Init();
		}
		//清0主界面强制休眠计数
		ForcePowerDownCount=0;
		MenuNormalStart=GetTimerCount();
		while(1)
		{
				if((KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb) && SysConfig.Bits.KeyPADLock!=1)
				{
						LedVal.ledval=LED_MASK_NORMAL;
						KeyBit=GetKeyLedBit(KeyStat.Numb);		
						if(KeyStat.Flag!=0)
						{
								KeyPressCount++;
						}
						switch(KeyStat.Numb)
						{
								case KEY_bell:         //门铃
										if(KeyStat.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
											  UserControlLock(CTL_DOOR_BELL,CODE_OPEN,0XFF);
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:            //*退格键
										if(KeyStat.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												InputBuf[InputIdx++]=KeyStat.Numb;
												if(InputIdx>=INPUT_MAX)InputIdx=0;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
												
												if(strstr((const char*)InputBuf,"***")!=0)
												{
														LPM_ResettopModeValue();
														UartBack_SendCmdData(CTRL_FAST_SLEEP,temp,0,1,1000);        //快速休眠指令
												}
										}
										break;
								case KEY_c:        //#确认键
										if(KeyStat.Flag!=0)
										{
												uint8_t CodeBuf[INPUT_MAX]={0};
												uint8_t PwdIdx=0;
												
												LedVal.ledval&=~(1<<KeyBit);     //按键按下，相应的指示灯熄灭
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												InputBuf[InputIdx++]=KeyStat.Numb; //保存键值
												if(InputIdx>=INPUT_MAX)InputIdx=0;
												InputBuf[InputIdx]=0;            //结束符
											
												printf("inputbuf:%s\n",InputBuf);
												
												if(strcmp((const char*)InputBuf,"400#")==0)               //播报400服务电话
												{
														AudioPlayNumString("4008538996");
														InputIdx=0;
												}
												else if(strstr((const char*)InputBuf,"*#")!=0)            //*#,不处理
												{
														if(SysConfig.Bits.Activate==0)
														{
																char *ret1=NULL,inputLen=0;
																ret1=strstr((const char*)InputBuf,"*#");
																inputLen=strlen(ret1);
																if(inputLen>=PWD_LEN_MIN+2)                         //6位密码加*#
																{
																		ret1+=2;
																		printf("retbuf:%s\n",ret1);
																		GetCodeWithoutX(ret1,(char*)CodeBuf);           //提取（激活码）
																		printf("codebuf:%s\n",CodeBuf);
																		if(CheckSnActiveCode((char*)CodeBuf))
																		{
																				SysConfig.Bits.Activate=1;
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Activation succeeded":"激活成功"),UNBREAK);
																		}
																		else
																		{
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"请激活产品"),UNBREAK);
																		}
																		InputIdx=0;
																}
																else if(strstr((const char*)InputBuf,"*#06#")!=0) 
																{
																		AudioPlayNumString(SnNum);
																		InputIdx=0;
																		if(KeyPadMode==KEYPAD_CONFIG)KeyPadMode=KEYPAD_NORMAL;
																}
																else
																{
																		if(inputLen>2)   //出去*#还有其他数据
																		{
																				printf("input len short\n");
																				InputIdx=0;
																		}
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"请激活产品"),UNBREAK);
																}
														}
														else if(KeyPadMode!=KEYPAD_CONFIG)
														{	
																if(SysConfig.Bits.Volume==0 || SysConfig.Bits.SupPowSave==1)   //静音模式，先恢复声音
																{
																		Audio_SetVolume(1); 
																}
																KeyPadMode=KEYPAD_CONFIG;
																
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please verify administrator":"请验证管理员"),UNBREAK);  
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Enter management mode":"进入管理模式"),UNBREAK);
														}
														else if(strstr((const char*)InputBuf,"*#06#")!=0) 
														{
																AudioPlayNumString(SnNum);
																InputIdx=0;
																if(KeyPadMode==KEYPAD_CONFIG)KeyPadMode=KEYPAD_NORMAL;
														}
														else    //按了*#再输入密码
														{
																char *ret=NULL,inputLen=0;
																ret=strstr((const char*)InputBuf,"*#");
																inputLen=strlen(ret);
																if(inputLen>=5)
																{
																		ret+=2;
																		printf("retbuf:%s\n",ret);
																		GetCodeWithoutX(ret,(char*)CodeBuf);
																		printf("codebuf:%s\n",CodeBuf);
																		PwdIdx=IdyPassWord(CodeBuf);
																		if(PwdIdx<PWD_MAX && Pwd.PwdGenStr[PwdIdx].Pwd.Type==ADMIN)
																		{
																				if(CheckAdminCode())
																				{
																						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Change administer password":"修改管理密码"),UNBREAK);
																						MenuNow=Menu_KeyPadModifyAdmUsers_Password;
																				}
																				else
																				{
																						MenuNow=Menu_KeyPadConfigSelect;               //切换菜单
																				}
																				LockUpdatePeripLockStat(PERIP_CODE_INDEX,0);
																		}
																		else
																		{
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Verification failed":"验证失败"),UNBREAK);
																				PirLed_SetStat(LOGO_OPEN_FAIL);
																				LockUpdatePeripLockStat(PERIP_CODE_INDEX,1);
																		}
																		InputIdx=0;
																}
														}
												}
												else                                     //验证密码 
												{
														printf("inputbuf:%s\n",InputBuf);
														GetCodeWithoutX((char*)InputBuf,(char*)CodeBuf);
														printf("codebuf:%s\n",CodeBuf);
														if(strlen((char*)CodeBuf)<6)break;       //密码太短
														InputIdx=0;
														if(SysConfig.Bits.Activate==0)
														{
																if(CheckSnActiveCode((char*)CodeBuf))
																{
																		SysConfig.Bits.Activate=1;
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Activation succeeded":"激活成功"),UNBREAK);
																}
																else
																{
																		AudioPlayVoice(GetVolIndex("指纹音"),UNBREAK);						//语音播报
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"请激活产品"),UNBREAK);
																		PirLed_SetStat(LOGO_OPEN_SUCCESS);			
																		UserControlLock(CTL_OPEN_LOCK,CODE_OPEN,0XFF);
																}
																break;  
														}
														PwdIdx=IdyPassWord(CodeBuf);
														printf("PwdIdx:0x%02x\n",PwdIdx);
														if(PwdIdx<PWD_MAX)          //正常密码
														{
																LockUpdatePeripLockStat(PERIP_CODE_INDEX,0);
													
																if(KeyPadMode==KEYPAD_CONFIG && Pwd.PwdGenStr[PwdIdx].Pwd.Type==ADMIN)
																{
																		if(CheckAdminCode())
																		{
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Change administer password":"修改管理密码"),UNBREAK);
																				MenuNow=Menu_KeyPadModifyAdmUsers_Password;
																		}
																		else
																		{
																				MenuNow=Menu_KeyPadConfigSelect;               //切换菜单
																		}
																}
																else if(KeyPadMode==KEYPAD_CONFIG)
																{
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Verification failed":"验证失败"),UNBREAK);
																		PirLed_SetStat(LOGO_OPEN_FAIL);
																		LockUpdatePeripLockStat(PERIP_CODE_INDEX,1);
																}
																else if(SysConfig.Bits.MutiIdy==1)
																{
																		if(MutiIdyItem&(~IDY_CODE))        //非密码验证成功+密码验证成功
																		{
																				MutiIdyItem=0;
																				AudioPlayVoice(GetVolIndex("指纹音"),UNBREAK);						//语音播报
																				PirLed_SetStat(LOGO_OPEN_SUCCESS);
																				UserControlLock(CTL_OPEN_LOCK,CODE_OPEN,PwdIdx);
																		}
																		else
																		{
																				MutiIdyItem|=IDY_CODE;  
																				PirLed_SetStat(LOGO_OPEN_SUCCESS_MUTIL);
																		}
																}
																else
																{
																		AudioPlayVoice(GetVolIndex("指纹音"),UNBREAK);						//语音播报
																		PirLed_SetStat(LOGO_OPEN_SUCCESS);
																		UserControlLock(CTL_OPEN_LOCK,CODE_OPEN,PwdIdx);
																}
														}
														else if(PwdIdx==PWD_IDY_ONETIME_CODE)       //一次性密码
														{
																LockUpdatePeripLockStat(PERIP_CODE_INDEX,0);
																AudioPlayVoice(GetVolIndex("指纹音"),UNBREAK);						//语音播报
																PirLed_SetStat(LOGO_OPEN_SUCCESS);
																UserControlLock(CTL_OPEN_LOCK,CODE_OPEN,PwdIdx);
														}
														else
														{
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Verification failed":"验证失败"),UNBREAK);               //验证失败
																PirLed_SetStat(LOGO_OPEN_FAIL);
																LockUpdatePeripLockStat(PERIP_CODE_INDEX,1);
														}
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:           //数字键
										if(KeyStat.Numb>='0' && KeyStat.Numb<='9')
										{
												if(KeyStat.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyStat.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										else
										{
												LedVal.ledval=0;
										}
										break;
						}
						if(KeyBk.Numb==0 && KeyStat.Flag==0)      //上电检测到误弹起操作
						{
								LedVal.ledval=LED_MASK_WAKE;
						}
						KeyBk=KeyStat;
				}
				//键盘扫描
				Key_Scan();
				//刷卡扫描
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(CdStat.StTime==0)
						{
								CardSearchHandle();
								CdStat.StTime=GetTimerCount();
						}
						else if(GetTimerElapse(CdStat.StTime)>CARD_SEARCH_INTVAL)
						{
								CdStat.StTime=0;
						}
				}
				//按键超时
				if(SysConfig.Bits.KeyPADLock!=1 && KeyStat.Flag==0 && KeyStat.STime>0)         
				{
						if(GetTimerElapse(KeyStat.STime)>KEY_INPUT_TIMEOUT)     //10s未按键退出休眠
						{
								KeyStat.STime=0;
								InputIdx=0;
								LPM_SetStopMode(LPM_KEYPAD_ID,LPM_Enable);
						} 
//3S未按键熄灭除了门铃以为的所有灯,省一点电
//						else if(GetTimerElapse(KeyStat.STime)>3000)             
//						{
//								if(LedVal.ledval!=LED_MASK_WAKE)
//								{
//										LedVal.ledval=LED_MASK_WAKE;
//								}
//						}
				}
				//指纹
				LdFpStatHandle();
				//语音
				Audio_RunningHandle();
				//串口
				Uart_RtxFrameHandle();
				//PIR LED
				PirLed_Handle();
				//键盘 LED
				Led_UpdateVal();	
				//写设置信息
				LockCheckPeripLockStat();
				LockWriteSysConfig();
				LockWritePerpLcokInfo(); 
				//对时
				SnycNetTime();
				//低功耗处理
				if(GetTimerElapse(MenuNormalStart)>5000)   //主界面至少停留5s
				{
						LPM_EnterLowPower();
				}
				//界面切换
				if(MenuNow!=Menu_KeyPadNormal && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)break;
		}
}

/******************************************************************************/
/*
//一级子菜单选择菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadConfigSelect(void)  
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
		
		InputIdx=0;
		
		UartFace_PowerOff();
		UartFace_TxQueueReset();
	
	
		if(SysConfig.Bits.Volume==0 || SysConfig.Bits.SupPowSave==1)
		{
				Audio_SetVolume(1); 
		}
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add users":"添加用户"),BREAK);       
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);   
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete users":"删除用户"),UNBREAK);      
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);         
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);            
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Modify administrator account":"修改管理员账户"),UNBREAK);    
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"three":"3"),UNBREAK);    //3
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Network settings":"网络设置"),UNBREAK);    //网络设置 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"four":"4"),UNBREAK);    //4
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"System settings":"系统设置"),UNBREAK);     //系统设置 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"five":"5"),UNBREAK);     //5
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Language settings":"语言设置"),UNBREAK);     //系统设置 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"six":"6"),UNBREAK);     //5
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"语言设置":"Language settings"),UNBREAK);     //系统设置 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"请按":"Please press"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"6":"six"),UNBREAK);     //5
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);     //退出
		
		LedVal.ledval=LED_MASK_CONFIG_SEL;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_CONFIG_SEL;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);	
						switch(KeyBk.Numb)
						{
								case KEY_1:               //添加用户
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadAddUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_2:               //删除用户
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_3:               //修改管理员账户
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadModifyAdmUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_4:               //网络管理
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadConfNet; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_5:               //系统设置 
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_6:               //语言设置 
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf_Language; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //*退出
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												if(MenuNow!=Menu_KeyPadNormal)
												{
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),UNBREAK);
														KeyPadMode=KEYPAD_NORMAL;
														MenuNow=Menu_KeyPadNormal;
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				/*******************************************************************/
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				/*******************************************************************/
				//语音播放完成,按键伴随着语音，所以直接检测语音
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				/*******************************************************************/
				if(MenuNow!=Menu_KeyPadConfigSelect && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}

/******************************************************************************/
/*
//添加用户子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadAddUsers(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add face":"添加人脸"),BREAK);      //添加人脸
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);     //1
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add fingerprint":"添加指纹"),BREAK);    //添加指纹 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);    //2
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add password":"添加密码"),UNBREAK);    //添加密码 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"three":"3"),UNBREAK);    //3
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add card":"添加卡片"),UNBREAK);     //添加卡片
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"four":"4"),UNBREAK);     //4
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add one-time password":"添加一次性密码"),UNBREAK);     //添加
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"five":"5"),UNBREAK);     //5
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_ADDUSER;
	
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
//								case KEY_1:               //增加人脸用户
//										if(KeyBk.Flag!=0)
//										{
//												AudioPlayVoice(GetVolIndex("滴"),BREAK);
//												LedVal.ledval&=~(1<<KeyBit); 
//												MenuNow=Menu_KeyPadAddUsers_Face;
//										}
//										else
//										{
//												LedVal.ledval|=(1<<KeyBit);
//										}
//										break;
								case KEY_2:               //增加指纹用户
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadAddUsers_Fp;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_3:               //增加密码用户
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadAddUsers_Code;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_4:               //增加卡用户 
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadAddUsers_Card;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_5:               //增加一次性密码
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadAddUsers_OnetimeCode;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回 
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadConfigSelect;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadAddUsers && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//添加密码子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadAddUsers_Code(void)
{
	  uint8_t codeBk[PWD_LEN_MAX]={0};                  //暂存第一次输入的密码
		
		uint8_t FirstCodeOk=0;
		uint32_t VolFinishTime=0;
		
		uint8_t freeslot=0;
		
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint8_t newStart=1;
		
		UartFace_PowerOff();
		
		InputIdx=0;
		memset(InputBuf,0,INPUT_MAX);
		
		LedVal.ledval=LED_MASK_ADDUSER_CODE;
	
		while(1)
		{
				if(newStart==1 && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)      //添加完成，继续下一个
				{
						freeslot=GetPassWordFreeSlot();
						if(freeslot==PWD_MAX)
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password library is ful":"密码库已满"),UNBREAK);
								MenuNow=Menu_KeyPadAddUsers;
						}
						else
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add password":"添加密码"),BREAK);                     //添加密码
								AudioPlayVoice(freeslot/10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK); 
								AudioPlayVoice(freeslot%10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK);
								
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);     //确认
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);     //返回
						}
						newStart=0;
				}
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_CODE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:               //返回 
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												if(InputIdx>0)
												{
														InputIdx--;
												}
												else 
												{
														MenuNow=Menu_KeyPadAddUsers; 
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_c:
										if(KeyBk.Flag!=0)
										{
												InputBuf[InputIdx]=0;
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												if(InputIdx<PWD_LEN_MIN || InputIdx>PWD_LEN_MAX-1)              //密码过长
												{
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
												}
												else if(FirstCodeOk==0)    //第一次输入密码
												{
														memcpy(codeBk,InputBuf,PWD_LEN_MAX);
														printf("first code: %s",codeBk);
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter the password again":"请再输入密码"),UNBREAK);
														FirstCodeOk=1;
												}
												else
												{
														printf("second code: %s",InputBuf);
														if(strcmp((const char*)InputBuf,(const char*)codeBk)==0)
														{
																switch(AddPassWord(NORMAL,codeBk,NULL,NULL))
																{
																		case PWD_ADD_SUCESS:
																		{
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
																				newStart=1;
																				break;
																		}
																		case PWD_ADD_CODE_EXIST:
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password already exists":"密码已存在"),UNBREAK);
																				break;
																		case PWD_ADD_MEM_FULL:
																		case PWD_ADD_TIME_MEM_FULL:
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password library is ful":"密码库已满"),UNBREAK);
																				MenuNow=Menu_KeyPadAddUsers;
																				break;
																		case PWD_ADD_TYP_WRONG:
																				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
																				break;
																}
																FirstCodeOk=0;
																memset(codeBk,0,PWD_LEN_MAX);
														}
														else
														{
																AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
																AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														}
														
												}
												memset(InputBuf,0,InputIdx);
												InputIdx=0;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										if(KeyBk.Numb>='0' && KeyBk.Numb<='9')
										{
												if(KeyBk.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyBk.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadAddUsers_Code && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//添加一次性密码子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadAddUsers_OnetimeCode(void)
{
	  uint8_t codeBk[PWD_LEN_MAX]={0};                  //暂存第一次输入的密码
		
		uint8_t FirstCodeOk=0;
		uint32_t VolFinishTime=0;
		
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		
		UartFace_PowerOff();
		
		InputIdx=0;
		memset(InputBuf,0,INPUT_MAX);
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter password":"请输入密码"),BREAK);       
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);     //确认
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);     //返回

		LedVal.ledval=LED_MASK_ADDUSER_CODE;
	
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_CODE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:               //返回 
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
																			
												if(InputIdx>0)
												{
														InputIdx--;
												}
												else 
												{
														MenuNow=Menu_KeyPadAddUsers; 
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_c:
										if(KeyBk.Flag!=0)
										{
												InputBuf[InputIdx]=0;
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												if(InputIdx>7)              //密码过长
												{
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
												}
												else if(FirstCodeOk==0)    //第一次输入密码
												{
														memcpy(codeBk,InputBuf,InputIdx);
														printf("first code: %s",codeBk);
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter the password again":"请再输入密码"),UNBREAK);
														FirstCodeOk=1;
												}
												else
												{
														printf("second code: %s",InputBuf);
														if(strcmp((const char*)InputBuf,(const char*)codeBk)==0)
														{
																if(!CheckOnetimeCodeExist(codeBk))
																{
																		memcpy(OneTimeCode.OneCode,codeBk,InputIdx);
																		RTC_GetTime((tm*)&OneTimeCode.StartTime);
																		IntFlashWrite((uint8_t*)&OneTimeCode,ONETIME_CODE_ADDR,sizeof(OneTimeCode_t));
																		SysConfig.Bits.OTCode=1;
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);       //添加
																		MenuNow=Menu_KeyPadAddUsers; //返回上层
																}
																else
																{
																		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK); 
																}
														}
														else
														{
																AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
																AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														}
												}
												memset(InputBuf,0,InputIdx);
												InputIdx=0;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										if(KeyBk.Numb>='0' && KeyBk.Numb<='9')
										{
												if(KeyBk.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyBk.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadAddUsers_OnetimeCode && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//添加卡片子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadAddUsers_Card(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint8_t freeslot=0;
		uint8_t newStart=1;
		uint32_t addcardStime=GetTimerCount();
		CardStat_t addStat={0};
		InputIdx=0;
		
		UartFace_PowerOff();

		LedVal.ledval=LED_MASK_ADDUSER_CARD;           //不显示#号键
	
		while(1)
		{
				if(newStart==1 && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						freeslot=GetCardFreeSlot();

						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add card":"添加卡片"),BREAK);       //添加卡片        
						AudioPlayVoice(freeslot/10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK); 
						AudioPlayVoice(freeslot%10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK);
						
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
						newStart=0;
				}
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_CARD; 
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:                    //返回添加用户界面
										if(KeyBk.Flag!=0)
										{
												printf("key x down\n");
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadAddUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)     //语音播报不刷卡，有干扰  
				{
						if(addStat.StTime==0)                        //添加卡片300ms扫一次
						{
								addStat.StTime=GetTimerCount();
								switch(CardAddHandle(ADD_NORMAL))
								{
										case ADD_CARD_SUCESS:
										{
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),BREAK);
												if(RemoteAdd.type.card!=1)
												{
														addcardStime=GetTimerCount();
														newStart=1;
												}
												else
												{
														newStart=0;
														MenuNow=Menu_KeyPadNormal; 
														KeyPadMode=KEYPAD_NORMAL;
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),UNBREAK);
												}
												break;
										}
										case ADD_CARD_FULL:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"card library is full":"卡片库已满"),BREAK);
												MenuNow=Menu_KeyPadAddUsers;      //返回上层
												break;
										case ADD_CARD_EXIST:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"card already exists":"卡片已存在"),BREAK); 
												break;
										case ADD_CARD_FAIL:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),BREAK);
												break;
										default:break;
								}
						}
						else if(GetTimerElapse(addStat.StTime)>CARD_ADD_INTVAL)
						{
								addStat.StTime=0;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if(addcardStime>0 && GetTimerElapse(addcardStime)>30000)
				{
						printf("add card timeout\n");  
						addcardStime=0;  
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						MenuNow=Menu_KeyPadAddUsers; 
				}
				if(MenuNow!=Menu_KeyPadAddUsers_Card && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(RemoteAdd.type.all!=1)RemoteAdd.type.all=0;
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//添加指纹子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadAddUsers_Fp(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint8_t addRet=0;
		uint8_t addfinish=0;
		uint32_t addFpStime=GetTimerCount();
		uint32_t addIntval=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
		FpAdd.LdFpAddStat=FP_ADD_READ_USER;
		FpAdd.StTime=0;
		FpAdd.GenCharCnt=0;
		FpAdd.VolStime=0;
		FpAdd.AlreadGetIMG=0;
		FpAdd.ComRetry=0;
	
		LedVal.ledval=LED_MASK_ADDUSER_FP;             //不显示#号键
	
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_FP; 
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:                    //返回添加用户界面
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadAddUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				if(addfinish==0)
				{
						if(addIntval==0 && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
						{
								addRet=FpAddHandle(RemoteAdd.type.fp==2?ADD_COERCE:ADD_NORMAL);
								if(addRet==0)
								{
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
										addfinish=0;
										if(RemoteAdd.type.fp!=1)
										{
												addFpStime=GetTimerCount();
												FpAdd.LdFpAddStat=FP_ADD_READ_USER;
												FpAdd.StTime=0;
												FpAdd.GenCharCnt=0;
												FpAdd.VolStime=0;
												FpAdd.ComRetry=0;
										}
										else
										{
												addfinish=1;
												MenuNow=Menu_KeyPadNormal; 
												KeyPadMode=KEYPAD_NORMAL;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),UNBREAK);
										}
								}
								else if(addRet==1)
								{
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
										addfinish=1;
										MenuNow=Menu_KeyPadAddUsers; 
								}
								addIntval=GetTimerCount();
						}
						else if(GetTimerElapse(addIntval)>50)         //间隔50MS，调用状态机
						{
								addIntval=0;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if(addFpStime>0 && GetTimerElapse(addFpStime)>60000)
				{
						addFpStime=0;  
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						addfinish=1;
						MenuNow=Menu_KeyPadAddUsers; 
				}
				if(MenuNow!=Menu_KeyPadAddUsers_Fp && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(RemoteAdd.type.all!=1)RemoteAdd.type.all=0;
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//添加人脸子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadAddUsers_Face(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint8_t addstart=1;
		uint8_t freeslot=0;
		uint32_t addFaceStime=GetTimerCount();
		uint8_t addintval=0;
		
		InputIdx=0;
	
		FaceAdd.StTime=0;
		FaceAdd.VolFristTime=0;
		FaceAdd.Retry=0;
		FaceAdd.AddStat=FACEADD_READUSER;
		//复位人脸模块：因为人脸模块识别时间比较长，在人脸识别过程中对他进行其他操作会出错
		FaceReady.Ready=0;
		UartFace_TxQueueReset();
		UartFace_PowerOn();
		
		LedVal.ledval=LED_MASK_ADDUSER_FACE;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_FACE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:                    //返回添加用户界面
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadAddUsers; 
												addstart=0;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				if(addstart==1)
				{
						if(addintval==0 && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
						{
								addintval=GetTimerCount();
								switch(FaceAddHandle(ADD_NORMAL))
								{
										case OP_READY:
												freeslot=GetOrbFaceFreeSlot(); 
												if(freeslot>=ORB_FACE_MAX)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Face library is full":"人脸库已满"),BREAK);
														MenuNow=Menu_KeyPadAddUsers; 
														addstart=0;
														break;
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add face":"添加人脸"),BREAK);                    //添加人脸    
												AudioPlayVoice(freeslot/10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK); 
												AudioPlayVoice(freeslot%10+GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK);
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
												FaceAdd.AddStat=FACEADD_RUN;
												break;
										case OP_SUCCESS:
										{
												//语音提示
												if((LPM_GetStopModeValue()&LPM_AUDIO_ID)!=0)break;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),BREAK);
												
												LockAddUser(PERIP_FACE_INDEX,freeslot);												
												
												//开始下一次添加
												if(RemoteAdd.type.face==0)
												{
														FaceAdd.StTime=0;
														FaceAdd.VolFristTime=0;
														FaceAdd.Retry=0;
														FaceAdd.AddStat=FACEADD_READUSER; 
														
														addFaceStime=GetTimerCount();   //添加下一个重新计时
														FaceReady.Ready=0;
												}
												else
												{
														MenuNow=Menu_KeyPadNormal; 
														addstart=0;
														KeyPadMode=KEYPAD_NORMAL;
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),UNBREAK);
												}
												break;
										}
										case OP_FAIL:
												if((LPM_GetStopModeValue()&LPM_AUDIO_ID)!=0)break;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please look at the camera":"请直视摄像头"),UNBREAK);       
												FaceAdd.AddStat=FACEADD_RUN;
												FaceAdd.StTime=0;
												break;
										case OP_TIMEOUT:
												if(RemoteAdd.type.face==0)
												{
														MenuNow=Menu_KeyPadAddUsers; 
												}
												else
												{
														MenuNow=Menu_KeyPadNormal; 
														KeyPadMode=KEYPAD_NORMAL;
												}
												addstart=0;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),UNBREAK);
												break;
										default:break;
								}
						}
						else if(GetTimerElapse(addintval)>50)
						{
								addintval=0;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if(addFaceStime>0 && GetTimerElapse(addFaceStime)>50000)
				{
						printf("add face timeout\n");  
						addFaceStime=0;  
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						MenuNow=Menu_KeyPadAddUsers; 
				}
				if(MenuNow!=Menu_KeyPadAddUsers_Face && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(RemoteAdd.type.all!=1)RemoteAdd.type.all=0;
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//修改管理员用户选择界面
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadModifyAdmUsers(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
		
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Change administer face":"修改管理人脸"),BREAK);     //修改
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);     //1

		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Change administer fingerprint":"修改管理指纹"),BREAK);     //修改
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);    //2
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Change administer password":"修改管理密码"),UNBREAK);     //修改
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"three":"3"),UNBREAK);    //3
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Change administer card":"修改管理卡片"),UNBREAK);     //修改
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"four":"4"),UNBREAK);     //4
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
	
		LedVal.ledval=LED_MASK_MODIFY_ADMIN;
	
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_MODIFY_ADMIN;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
//								case KEY_1:               //修改管理员人脸
//										if(KeyBk.Flag!=0)
//										{
//												AudioPlayVoice(GetVolIndex("滴"),BREAK);
//												LedVal.ledval&=~(1<<KeyBit); 
//												MenuNow=Menu_KeyPadModifyAdmUsers_Face;
//										}
//										else
//										{
//												LedVal.ledval|=(1<<KeyBit);
//										}
//										break;
								case KEY_2:               //修改管理员指纹
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadModifyAdmUsers_Fp;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_3:               //修改管理员密码
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadModifyAdmUsers_Password;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_4:               //修改管理员卡
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadModifyAdmUsers_Card;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回 
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit); 
												MenuNow=Menu_KeyPadConfigSelect;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadModifyAdmUsers && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//修改管理员指纹子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadModifyAdmUsers_Fp(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint8_t addRet=0;
		uint8_t addfinish=0;
		uint32_t addFpStime=GetTimerCount();
		uint32_t addIntval=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
		FpAdd.LdFpAddStat=FP_ADD_READ_USER;
		FpAdd.StTime=0;
		FpAdd.GenCharCnt=0;
		FpAdd.VolStime=0;
		FpAdd.AlreadGetIMG=0;
		FpAdd.ComRetry=0;
	
		LedVal.ledval=LED_MASK_ADDUSER_FP;             //不显示#号键
	
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_FP; 
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:                    //返回添加用户界面
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadModifyAdmUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				if(addfinish==0)                 //添加完成标志
				{
						if(addIntval==0)
						{
								addRet=FpAddHandle(ADD_ADMIN);
								if(addRet==0)
								{
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
										addfinish=1;
										MenuNow=Menu_KeyPadModifyAdmUsers; 
								}
								else if(addRet==1)
								{
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
										addfinish=1;
										MenuNow=Menu_KeyPadModifyAdmUsers; 
								}
								addIntval=GetTimerCount();
						}
						else if(GetTimerElapse(addIntval)>50)  //间隔50MS，调用状态机
						{
								addIntval=0;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if(addFpStime>0 && GetTimerElapse(addFpStime)>60000)
				{
						addFpStime=0;  
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						MenuNow=Menu_KeyPadModifyAdmUsers; 
				}
				if(MenuNow!=Menu_KeyPadModifyAdmUsers_Fp && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)break;
		}
}
/******************************************************************************/
/*
//修改管理员卡片子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadModifyAdmUsers_Card(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t addcardStime=GetTimerCount();
		CardStat_t addStat={0};
		InputIdx=0;
		
		UartFace_PowerOff();
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add card":"添加卡片"),BREAK);       //添加卡片        
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK); 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"zero":"0"),UNBREAK);
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
		
		LedVal.ledval=LED_MASK_ADDUSER_CARD;           //不显示#号键
	
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_CARD; 
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:                    //返回添加用户界面
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadModifyAdmUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)     //语音播报不刷卡，有干扰  
				{
						if(addStat.StTime==0)
						{
								addStat.StTime=GetTimerCount();
								switch(CardAddHandle(ADD_ADMIN))
								{
										case ADD_CARD_SUCESS:
										case ADD_CARD_EXIST:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),BREAK);
												MenuNow=Menu_KeyPadModifyAdmUsers;      //返回上层
												break;
										case ADD_CARD_FULL:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"card library is full":"卡片库已满"),BREAK);
												MenuNow=Menu_KeyPadModifyAdmUsers;      //返回上层
												break;
										case ADD_CARD_FAIL:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),BREAK);
												break;
										default:break;
								}
						}
						else if(GetTimerElapse(addStat.StTime)>CARD_ADD_INTVAL)
						{
								addStat.StTime=0;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if(addcardStime>0 && GetTimerElapse(addcardStime)>30000)
				{
						printf("add card timeout\n");  
						addcardStime=0;  
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						MenuNow=Menu_KeyPadModifyAdmUsers; 
				}
				if(MenuNow!=Menu_KeyPadModifyAdmUsers_Card && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)break;
		}
}
/******************************************************************************/
/*
//修改管理员密码子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadModifyAdmUsers_Password(void)
{
		uint8_t codeBk[PWD_LEN_MAX]={0};                  //暂存第一次输入的密码
		
		uint8_t FirstCodeOk=0;
		uint8_t chkok=0,i=0;
		uint32_t VolFinishTime=0;
		
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		
		InputIdx=0;
		memset(InputBuf,0,INPUT_MAX);
		
		UartFace_PowerOff();
		

		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter password":"请输入密码"),BREAK);      //输入
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
		
		LedVal.ledval=LED_MASK_ADDUSER_CODE;
	
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_CODE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:               //返回 
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit); 
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												if(InputIdx>0)
												{
														InputBuf[--InputIdx]=0;
												}
												else 
												{
														MenuNow=Menu_KeyPadModifyAdmUsers; 
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_c:
										if(KeyBk.Flag!=0)
										{
												InputBuf[InputIdx]=0;
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												if(InputIdx>PWD_LEN_MAX-1)              //密码过长
												{
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
												}
												else if(FirstCodeOk==0)    //第一次输入密码
												{
														memcpy(codeBk,InputBuf,InputIdx);
														printf("first code: %s",codeBk);
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter the password again":"请再输入密码"),UNBREAK);
														FirstCodeOk=1;
												}
												else
												{
														printf("second code: %s",InputBuf);
														for(i=0;i<strlen((char*)codeBk);i++)
														{
																if(codeBk[i]!=InputBuf[i])break;
														}
														if(i<strlen((char*)codeBk))
														{
																AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
																AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														}
														else
														{
																chkok=1;
														}
												}
												memset(InputBuf,0,INPUT_MAX);
												InputIdx=0;
												
												if(chkok==0)break;
												switch(AddPassWord(ADMIN,codeBk,NULL,NULL))
												{
														case PWD_ADD_SUCESS:
														{
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),UNBREAK);
																MenuNow=Menu_KeyPadModifyAdmUsers; //返回上层
														}
																break;
														case PWD_ADD_MEM_FULL:
														case PWD_ADD_TIME_MEM_FULL:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Password library is full":"密码库已满"),UNBREAK);
																MenuNow=Menu_KeyPadModifyAdmUsers;
																break;
														case PWD_ADD_CODE_EXIST:
														case PWD_ADD_TYP_WRONG:
																AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add failed":"添加失败"),UNBREAK);
																break;
												}
												chkok=0;
												FirstCodeOk=0;
												memset(codeBk,0,PWD_LEN_MAX);
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										if(KeyBk.Numb>='0' && KeyBk.Numb<='9')
										{
												if(KeyBk.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyBk.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								MenuNow=Menu_KeyPadModifyAdmUsers;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadModifyAdmUsers_Password && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)break;
		}
}
/******************************************************************************/
/*
//修改管理员人脸子界面
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadModifyAdmUsers_Face(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint8_t admfaceDelok=0;
		uint32_t addFaceStime=GetTimerCount();
		uint32_t faceintval=0;
		
		InputIdx=0;
		
		FaceReady.Ready=0;
		UartFace_TxQueueReset();
		UartFace_PowerOn();
	
		FaceDel.DelStat=FACEDEL_READUSER;
		FaceDel.Retry=0;
		FaceDel.StTime=0;
	
		FaceAdd.StTime=0;
		FaceAdd.VolFristTime=0;
		FaceAdd.Retry=0;
		FaceAdd.AddStat=FACEADD_READUSER;
	
		LedVal.ledval=LED_MASK_ADDUSER_FACE;
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_ADDUSER_FACE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_x:                    //返回添加用户界面
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadModifyAdmUsers; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				if(admfaceDelok==0)
				{
						if(faceintval==0)
						{
								switch(FaceDelHandle(0))
								{
										case OP_SUCCESS:
												admfaceDelok=1;
												faceintval=0;
												break;
										case OP_FAIL:
												FaceDel.DelStat=FACEDEL_RUN;
												FaceDel.StTime=0;
												FaceDel.Retry=0;
												break;
										case OP_TIMEOUT:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
												admfaceDelok=2;
												MenuNow=Menu_KeyPadModifyAdmUsers;
												break;
								}
								faceintval=GetTimerCount();
						}
						else if(GetTimerElapse(faceintval)>100)
						{
								faceintval=0;
						}
				}
				else if(admfaceDelok==1)
				{
						if(faceintval==0)
						{
								switch(FaceAddHandle(ADD_ADMIN))
								{
										case OP_READY:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please look at the camera":"请直视摄像头"),UNBREAK);  
												FaceAdd.AddStat=FACEADD_RUN;
												break;
										case OP_SUCCESS:
												//语音提示
												if((LPM_GetStopModeValue()&LPM_AUDIO_ID)!=0)break;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add succeed":"添加成功"),BREAK);
												admfaceDelok=2;
												MenuNow=Menu_KeyPadModifyAdmUsers; 
												break;
										case OP_FAIL:
												if((LPM_GetStopModeValue()&LPM_AUDIO_ID)!=0)break;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please look at the camera":"请直视摄像头"),UNBREAK);       
												FaceAdd.AddStat=FACEADD_RUN;
												break;
										case OP_TIMEOUT:
												if((LPM_GetStopModeValue()&LPM_AUDIO_ID)!=0)break;
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
												admfaceDelok=2;
												MenuNow=Menu_KeyPadModifyAdmUsers;
												break;
										default:break;
								}
								faceintval=GetTimerCount();
						}
						else if(GetTimerElapse(faceintval)>100)
						{
								faceintval=0;
						}
				}
				Key_Scan();
				Uart_RtxFrameHandle();
				Audio_RunningHandle();
				Led_UpdateVal();
				if(addFaceStime>0 && GetTimerElapse(addFaceStime)>50000)
				{
						printf("add face timeout\n");  
						addFaceStime=0;  
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						MenuNow=Menu_KeyPadModifyAdmUsers; 
				}
				if(MenuNow!=Menu_KeyPadModifyAdmUsers_Face && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)break;
		}
}
/******************************************************************************/
/*
//删除用户选择菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		UartFace_PowerOff();
	
		InputIdx=0;
	
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete face":"删除人脸"),BREAK);      //删除人脸
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);     //1

		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete fingerprint":"删除指纹"),BREAK);    //删除指纹 

		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);    //2
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete password":"删除密码"),UNBREAK);    //删除密码 
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);    //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"three":"3"),UNBREAK);    //3
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete card":"删除卡片"),UNBREAK);     //删除卡片
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"four":"4"),UNBREAK);     //4
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete one-time password":"删除一次性密码"),UNBREAK);     //删除
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"five":"5"),UNBREAK);     //5
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete all users":"删除所有用户"),UNBREAK);     //删除所有用户
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"six":"6"),UNBREAK);     //6
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
	
		LedVal.ledval=LED_MASK_DELUSER;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_DELUSER;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
//								case KEY_1:               //删除人脸用户
//										if(KeyBk.Flag!=0)
//										{
//												LedVal.ledval&=~(1<<KeyBit);
//												AudioPlayVoice(GetVolIndex("滴"),BREAK);
//												MenuNow=Menu_KeyPadDelUsers_Face; 
//										}
//										else
//										{
//												LedVal.ledval|=(1<<KeyBit);
//										}
//										break;
								case KEY_2:               //删除指纹用户
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers_Fp; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_3:               //删除密码用户
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers_Code; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_4:               //删除卡用户 
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers_Card; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_5:               //删除一次性密码
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers_OnetimeCode;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_6:               //删除所有用户
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers_All;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadConfigSelect; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadDelUsers && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//删除一次性密码子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers_OnetimeCode(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		UartFace_PowerOff();
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),BREAK);
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
		
		LedVal.ledval=LED_MASK_DELUSER_ONETIMECODE;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_DELUSER_ONETIMECODE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_c:               //确认
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),UNBREAK);
												
												memset((uint8_t*)&OneTimeCode,0xFF,sizeof(OneTimeCode_t));  
												IntFlashWrite((uint8_t*)&OneTimeCode,ONETIME_CODE_ADDR,sizeof(OneTimeCode_t));
												SysConfig.Bits.OTCode=0;
												MenuNow=Menu_KeyPadDelUsers;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadDelUsers_OnetimeCode && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//删除人脸子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers_Face(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t id=0,i=0,j=0;
		uint32_t VolFinishTime=0;
		uint8_t DelId=0xFF;
		uint32_t delintval=0;
		uint8_t remoteDel=0;
		
		InputIdx=0;
	
		if(RemoteDel.type.face==1)
		{
				DelId=RemoteDel.id;
				RemoteDel.type.face=0;	
				remoteDel=1;
		}
		else
		{
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter face user number":"请输入人脸用户编号"),BREAK);        //请输入
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
		}
	
		LedVal.ledval=LED_MASK_DELUSER_FACE;
		
		//复位人脸模块：因为人脸模块识别时间比较长，在人脸识别过程中对他进行其他操作会出错
		UartFace_TxQueueReset();
		
		FaceReady.Ready=0;

		UartFace_PowerOn();
		
		FaceDel.DelStat=FACEDEL_READUSER;
		FaceDel.StTime=0;
		FaceDel.Retry=0;
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_DELUSER_FACE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_c:               //确认
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												InputBuf[InputIdx]=0;
												printf("input str:%s \n",InputBuf);
												id=0;
												for(i=InputIdx,j=0;i>0;i--,j++)
												{
														id+=(InputBuf[j]-'0')*pow(10,i-1);
												}
												InputIdx=0;
												printf("del num:%02d \n",id);
												if(id<100)           //0-99
												{
														DelId=id;
												}
												else
												{
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
														AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										if(KeyBk.Numb>='0' && KeyBk.Numb<='9')
										{
												if(KeyBk.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyBk.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				//删人脸ID
				if(DelId!=0xFF)
				{
						if(delintval==0)
						{
								delintval=GetTimerCount();
								switch(FaceDelHandle(DelId))
								{
										case OP_SUCCESS:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),BREAK);
												DelId=0xFF;
												if(remoteDel==1)
												{
														KeyPadMode=KEYPAD_NORMAL;
														MenuNow=Menu_KeyPadNormal;
												}
												else
												{
														MenuNow=Menu_KeyPadDelUsers;
												}
												break;
										case OP_FAIL:
												FaceDel.DelStat=FACEDEL_RUN;
												FaceDel.StTime=0;
												FaceDel.Retry=0;
												break;
										case OP_TIMEOUT:
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),UNBREAK);
												DelId=0xFF;
												if(remoteDel==1)
												{
														KeyPadMode=KEYPAD_NORMAL;
														MenuNow=Menu_KeyPadNormal;
												}
												else
												{
														MenuNow=Menu_KeyPadDelUsers;
												}
												break;
								}
						}
						else if(GetTimerElapse(delintval)>50)
						{
								delintval=0;
						}
				}
				//语音播放完成后10S未操作就退出本菜单
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadDelUsers_Face && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//删除指纹子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers_Fp(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t id=0,i=0,j=0;
		uint8_t DelId=0xFF,DelRet=0;
		uint8_t remoteDel=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;

		UartFace_PowerOff();
		
		if(RemoteDel.type.fp==1)
		{
				DelId=RemoteDel.id;
				RemoteDel.type.fp=0;
				remoteDel=1;
		}
		else
		{
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter fingerprint user number":"请输入指纹用户编号"),BREAK);        //请输入
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
		}
		
		LedVal.ledval=LED_MASK_DELUSER_FP;
		
		FpDel.LdFpDelStat=FP_DEL_READ_USER;
		FpDel.StTime=0;
		FpDel.ComRetry=0;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_DELUSER_FP;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_c:               //确认
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												InputBuf[InputIdx]=0;
												printf("input str:%s \n",InputBuf);
												id=0;
												for(i=InputIdx,j=0;i>0;i--,j++)
												{
														id+=(InputBuf[j]-'0')*pow(10,i-1);
												}
												InputIdx=0;
												printf("del num:%02d \n",id);
												if(id<100)        //0-99,管理员用户0有单独界面修改
												{
														DelId=id;
												}
												else
												{
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														AudioPlayVoice(GetVolIndex("滴"),UNBREAK);
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										if(KeyBk.Numb>='0' && KeyBk.Numb<='9')
										{
												if(KeyBk.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyBk.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										break;
						}
				}
				if(DelId!=0xFF)
				{
						DelRet=FpDelHandle(DelId);
						if(DelRet==FP_SUCCESS)
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),BREAK);
							
								DelId=0xFF;
								if(remoteDel==1)
								{
										KeyPadMode=KEYPAD_NORMAL;
										MenuNow=Menu_KeyPadNormal;
								}
								else
								{
										MenuNow=Menu_KeyPadDelUsers;
								}
						}
						if(DelRet==FP_FAIL)
						{
								DelId=0xFF;
								if(remoteDel==1)
								{
										KeyPadMode=KEYPAD_NORMAL;
										MenuNow=Menu_KeyPadNormal;
								}
								else
								{
										MenuNow=Menu_KeyPadDelUsers;
								}
						}
				}
				Key_Scan();
				Uart_RtxFrameHandle();
				Audio_RunningHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadDelUsers_Fp && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//删除密码子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers_Code(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t id=0,i=0,j=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter password user number":"请输入密码用户编号"),BREAK);        //请输入
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
	
		LedVal.ledval=LED_MASK_DELUSER_CODE;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_DELUSER_CODE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_c:               //确认
										if(KeyBk.Flag!=0)
										{
												uint8_t ret=0;
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												InputBuf[InputIdx]=0;
												printf("input str:%s \n",InputBuf);
												id=0;
												for(i=InputIdx,j=0;i>0;i--,j++)
												{
														id+=(InputBuf[j]-'0')*pow(10,i-1);
												}
												InputIdx=0;
												printf("del num:%02d \n",id);
												ret=DelPassWord(id);
												if(ret==PWD_DEL_SUCCESS || PWD_DEL_CODE_NOT_EXIST)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),UNBREAK);
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit);
												MenuNow=Menu_KeyPadDelUsers;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										if(KeyBk.Numb>='0' && KeyBk.Numb<='9')
										{
												if(KeyBk.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyBk.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadDelUsers_Code && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//删除卡片子菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers_Card(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t id=0,i=0,j=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please enter card user number":"请输入卡用户编号"),BREAK);        //请输入
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
	
		LedVal.ledval=LED_MASK_DELUSER_CARD;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_DELUSER_CARD;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_c:               //确认
										if(KeyBk.Flag!=0)
										{
												uint8_t ret=0;
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit);
												InputBuf[InputIdx]=0;
												printf("input str:%s \n",InputBuf);
												id=0;
												for(i=InputIdx,j=0;i>0;i--,j++)
												{
														id+=(InputBuf[j]-'0')*pow(10,i-1);
												}
												InputIdx=0;
												printf("del num:%02d \n",id);
												ret=DelCard(id);
												
												if(ret==DEL_CARD_SUCCESS || ret==DEL_CARD_NOEXIST)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),UNBREAK);
												}
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												LedVal.ledval&=~(1<<KeyBit);
												MenuNow=Menu_KeyPadDelUsers;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										if(KeyBk.Numb>='0' && KeyBk.Numb<='9')
										{
												if(KeyBk.Flag!=0)
												{
														LedVal.ledval&=~(1<<KeyBit);
														AudioPlayVoice(GetVolIndex("滴"),BREAK);
														InputBuf[InputIdx++]=KeyBk.Numb;
														if(InputIdx>=INPUT_MAX)InputIdx=0;
												}
												else
												{
														LedVal.ledval|=(1<<KeyBit);
												}
										}
										break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadDelUsers_Card && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//删除所有用户菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers_All(void)          //删除：密码，卡片，指纹，人脸
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
		uint8_t DeFpOk=0;
		uint8_t DelConfirm=0;
		uint8_t remoteDel=0;
		InputIdx=0;
		
		if(RemoteDel.type.all==1)
		{
				DelConfirm=1;
				DelPassWord(0xFF);       //删除密码
				DelCard(0xFF);           //删除卡片
				RemoteDel.type.all=0;
				remoteDel=1;
		}
		else
		{
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Hash key to confirm":"确认请按#号键"),UNBREAK);
				AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
		}
		
		//复位人脸模块：因为人脸模块识别时间比较长，在人脸识别过程中对他进行其他操作会出错   
		FaceReady.Ready=0;
		UartFace_PowerOn();
		UartFace_TxQueueReset();
		FaceDel.DelStat=FACEDEL_READUSER;
		
		LedVal.ledval=LED_MASK_DELUSER_ALL;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_DELUSER_ALL;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_c:               //确认
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												DelConfirm=1;
												DelPassWord(0xFF);       //删除密码
												DelCard(0xFF);           //删除卡片
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadDelUsers;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:
										break;
						}
				}
				if(DelConfirm==1)
				{
						if(DeFpOk==0)
						{
								switch(FpDelHandle(0xFF))
								{
										case FP_SUCCESS:
												DeFpOk=1;
												break;
										case FP_FAIL:
												DeFpOk=2;
												break;
								}
						}
						if(DeFpOk==1 || DeFpOk==2)
						{
								if(DeFpOk==1)
								{
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),BREAK);
								}
								else
								{
										AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
								}
								if(remoteDel==1)
								{
										KeyPadMode=KEYPAD_NORMAL;
										MenuNow=Menu_KeyPadNormal;
								}
								else
								{
										MenuNow=Menu_KeyPadDelUsers;
								}
								DeFpOk=0;
								DelConfirm=0;
						}
				}
				Key_Scan();
				Uart_RtxFrameHandle();
				Audio_RunningHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadDelUsers_All && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//删除指定用户，包括，指纹，人脸，卡片和密码，每项最多5组
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadDelUsers_select(void)          
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t delStartTime=0;
		uint8_t DeSum=0;
		uint8_t idx=0;
		InputIdx=0;
		
		FaceReady.Ready=0;
		UartFace_PowerOn();
		UartFace_TxQueueReset();
		FaceDel.DelStat=FACEDEL_READUSER;
		LedVal.ledval=LED_MASK_WAKE;
		Led_UpdateVal();
		while(1)
		{
				for(;idx<RemoteDelSelect.sum;idx++)  
				{
						if(RemoteDelSelect.id[idx].u8[0]==0x02)  //指纹
						{
								if(FpDelHandle(RemoteDelSelect.id[idx].u8[1])==FP_SUCCESS)
								{
										RemoteDelSelect.id[idx].u8[0]=0xFF;    //成功，超时
										DeSum++;
								}
								if(delStartTime==0)
								{
										delStartTime=GetTimerCount();
								}							
								else if(GetTimerElapse(delStartTime)>5000)
								{
										delStartTime=0;
										RemoteDelSelect.id[idx].u8[0]=0xFF;    //成功，超时
								}
								break;
						}
						else if(RemoteDelSelect.id[idx].u8[0]==0x04)  //卡片
						{
								DelCard(RemoteDelSelect.id[idx].u8[1]);           //删除卡片
								RemoteDelSelect.id[idx].u8[0]=0xFF;
								DeSum++;
						}
						else if(RemoteDelSelect.id[idx].u8[0]==0x01)  //人脸
						{
								if(FaceDelHandle(RemoteDelSelect.id[idx].u8[1])==OP_SUCCESS)
								{
										RemoteDelSelect.id[idx].u8[0]=0xFF;
										DeSum++;
								}
								if(delStartTime==0)
								{
										delStartTime=GetTimerCount();
								}							
								else if(GetTimerElapse(delStartTime)>5000)
								{
										delStartTime=0;
										RemoteDelSelect.id[idx].u8[0]=0xFF;    //成功，超时
								}
								break;
						}
						else if(RemoteDelSelect.id[idx].u8[0]==0x03)   //密码
						{
								DelPassWord(RemoteDelSelect.id[idx].u8[1]);       //删除密码
								RemoteDelSelect.id[idx].u8[0]=0xFF;
								DeSum++;
						}
						else             //无效项，跳出下一个
						{
								continue;
						}
				}
				if(idx==RemoteDelSelect.sum)
				{
						if(DeSum>=RemoteDelSelect.sum)  //删除完成
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete succeed":"删除成功"),BREAK);
						}
						else
						{
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Operation timed out":"操作超时"),BREAK);
						}
						KeyPadMode=KEYPAD_NORMAL;
						MenuNow=Menu_KeyPadNormal;
						idx=0xFF;
				}
				Key_Scan();
				Uart_RtxFrameHandle();
				Audio_RunningHandle();
				if(MenuNow!=Menu_KeyPadDelUsers_select && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}

/******************************************************************************/
/*
//网络设置界面
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadConfNet(void) 
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Add network":"添加网络"),BREAK);     //添加网络
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);     //1
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Delete network":"删除网络"),UNBREAK);     //删除网络
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     //2
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
			
		LedVal.ledval=LED_MASK_NETCONF;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_NETCONF;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //添加网络
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												AudioPlayVoice(GetVolIndex("添加成功"),UNBREAK);
												MenuNow=Menu_KeyPadConfigSelect;
												uint8_t temp=1;
												UartBack_SendCmdData(CONFIG_NET,&temp,1,1,1000);  
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_2:               //删除网络
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												AudioPlayVoice(GetVolIndex("删除成功"),UNBREAK);
												MenuNow=Menu_KeyPadConfigSelect;
												uint8_t temp=2;
												UartBack_SendCmdData(CONFIG_NET,&temp,1,1,1000);
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadConfigSelect;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成后10S未操作就退出本菜单
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadConfNet && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//系统设置界面
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadSysConf(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Advanced security settings":"高级安全设置"),BREAK);       
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);     //1
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"volume settings":"音量设置"),UNBREAK);     //音量设置
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     //2

		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Ultra power saving mode":"超省电模式"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"three":"3"),UNBREAK);     //4
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"keep open mode":"常开模式"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"four":"4"),UNBREAK);     //4

//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Human body induction settings":"人体感应设置"),UNBREAK);     //超省电模式
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"five":"5"),UNBREAK);     //4
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Serial number query":"序列号查询"),UNBREAK);     //SN号查询
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"six":"6"),UNBREAK);     //5
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_SYSCONF;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_SYSCONF;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //高级安全模式
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf_Security;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_2:               //音量设置
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf_Volume;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_3:               //超省电模式 
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf_PowerSave;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_4:               //常开模式
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf_KeepOpen;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
//								case KEY_5:               //PIR开关
//										if(KeyBk.Flag!=0)
//										{
//												LedVal.ledval&=~(1<<KeyBit);
//												AudioPlayVoice(GetVolIndex("滴"),BREAK);
//												MenuNow=Menu_KeyPadSysConf_PirSet;
//										}
//										else
//										{
//												LedVal.ledval|=(1<<KeyBit);
//										}
//										break;
								case KEY_6:               //SN号查询
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												AudioPlayNumString(SN);
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadConfigSelect;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadSysConf && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//双验证模式设置界面
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadSysConf_Security(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn on two-factor authentication":"开启双重开锁"),BREAK);     
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);      //1
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn off two-factor authentication":"关闭双重开锁"),UNBREAK);     //关闭
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     //2
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_SYSCONF_SECURITY;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_SYSCONF_SECURITY;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //开启双重验证
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												if(SysConfig.Bits.MutiIdy!=1)
												{
														SysConfig.Bits.MutiIdy=1;
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn on two-factor authentication":"开启双重开锁"),BREAK); 
												MenuNow=Menu_KeyPadSysConf;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_2:               //关闭双重验证
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												if(SysConfig.Bits.MutiIdy!=0)
												{
														SysConfig.Bits.MutiIdy=0;
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn off two-factor authentication":"关闭双重开锁"),BREAK); 
												MenuNow=Menu_KeyPadSysConf;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
								case KEY_c:               //确认
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadSysConf_Security && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//音量设置菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadSysConf_Volume(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();

		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Low volume":"音量低"),BREAK);       //低音量
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);      //1
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"High volume":"音量高"),UNBREAK);     //高音量
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     //2
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Mute":"静音"),UNBREAK);     //静音
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"three":"3"),UNBREAK);     //3
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_SYSCONF_VOLUME;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_SYSCONF_VOLUME;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //低音量
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
											
												if(SysConfig.Bits.Volume!=1)
												{
														SysConfig.Bits.Volume=1;
														Audio_SetVolume(SysConfig.Bits.Volume);
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Low volume":"音量低"),BREAK);  
												MenuNow=Menu_KeyPadSysConf;
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_2:               //高音量
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												if(SysConfig.Bits.Volume!=2)
												{
														SysConfig.Bits.Volume=2;
														Audio_SetVolume(SysConfig.Bits.Volume);
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"High volume":"音量高"),BREAK); 
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_3:               //静音:只限于非设置界面
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioHalSetVolume(5); 
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Mute":"静音"),BREAK);  
												if(SysConfig.Bits.Volume!=0)
												{
														SysConfig.Bits.Volume=0;
												}
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK); 
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadSysConf_Volume && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)   //语音播放完成才返回
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//语言设置菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadSysConf_Language(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();

//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"中文":"Chinese"),BREAK);       
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"请按":"Please press"),UNBREAK);     
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"1":"one"),UNBREAK);      
//		
//		AudioPlayVoice(GetVolIndex("English"),UNBREAK);     
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);       
//		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     
	
		AudioPlayVoice(GetVolIndex("中文"),BREAK);       
		AudioPlayVoice(GetVolIndex("请按"),UNBREAK);     
		AudioPlayVoice(GetVolIndex("1"),UNBREAK);      
		
		AudioPlayVoice(GetVolIndex("English"),UNBREAK);     
		AudioPlayVoice(GetVolIndex("Please press"),UNBREAK);       
		AudioPlayVoice(GetVolIndex("two"),UNBREAK);  
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_SYSCONF_LANGUAGE;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_SYSCONF_LANGUAGE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //中文
								case KEY_2:               //英文
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												SysConfig.Bits.Language=((KeyBk.Numb==KEY_1)?0:1);
												MenuNow=Menu_KeyPadConfigSelect; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadConfigSelect; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadSysConf_Language && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//超省电模式设置菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadSysConf_PowerSave(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),BREAK);       //开启
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Ultra power saving mode":"超省电模式"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);      //1
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);     //关闭
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Ultra power saving mode":"超省电模式"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     //2
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_SYSCONF_POWERSAVE;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_SYSCONF_POWERSAVE;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //中文
								case KEY_2:               //英文
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												SysConfig.Bits.SupPowSave=(KeyBk.Numb==KEY_1)?1:0;
												uint8_t temp=SysConfig.Bits.SupPowSave?1:2;       //1开启，2关闭 
												UartBack_SendCmdData(CTRL_POWERSAVE,&temp,1,1,1000);        //快速休眠指令       //1开启，2关闭 
												if(SysConfig.Bits.SupPowSave)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),UNBREAK);
//														if(SysConfig.Bits.PirOn==1)
//														{
//																SysConfig.Bits.PirOn=0;
//																PowerOffPir();
//														}
												}
												else
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);
														if(SysConfig.Bits.Volume!=1)
														{
																SysConfig.Bits.Volume=1;
																Audio_SetVolume(SysConfig.Bits.Volume);
														}
														if(SysConfig.Bits.PirOn==0)
														{
																SysConfig.Bits.PirOn=1;
																SysConfig.Bits.Volume=1;
																Audio_SetVolume(SysConfig.Bits.Volume);
																PowerOnPir();
														}
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Ultra power saving mode":"超省电模式"),UNBREAK);
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadSysConf_PowerSave && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//常开模式设置菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadSysConf_KeepOpen(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),BREAK);       //开启
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"keep open mode":"常开模式"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);      //1
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);     //关闭
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"keep open mode":"常开模式"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     //2
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_SYSCONF_KEEPOPEN;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_SYSCONF_KEEPOPEN;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //中文
								case KEY_2:               //英文
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												SysConfig.Bits.KeepOpen=(KeyBk.Numb==KEY_1)?1:0;
												uint8_t temp=SysConfig.Bits.KeepOpen?1:2;                  //1开启，2关闭 
												UartBack_SendCmdData(CTRL_KEEPOPEN,&temp,1,1,1000);        //常开模式设置指令       //1开启，2关闭 
												if(SysConfig.Bits.KeepOpen)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),UNBREAK);
												}
												else
												{
														UserControlLock(CTL_CLOSE_LOCK,0xff,0xff);    //关闭常开模式，电机复位
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"keep open mode":"常开模式"),UNBREAK);
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadSysConf_KeepOpen && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//人体感应设置菜单
input:none
output:none
return:none
*/
/******************************************************************************/
void Menu_KeyPadSysConf_PirSet(void)
{
		KeyStat_t KeyBk=KeyStat;
		uint8_t KeyBit=0;
		uint32_t VolFinishTime=0;
	
		InputIdx=0;
	
		UartFace_PowerOff();
	
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),BREAK);       //开启
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Human body induction":"人体感应"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"one":"1"),UNBREAK);      //1
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);     //关闭
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Human body induction":"人体感应"),UNBREAK);     //超省电模式
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please press":"请按"),UNBREAK);     //请按
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"two":"2"),UNBREAK);     //2
		
		AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Press Asterisk key to return":"返回请按*号键"),UNBREAK);
				
		LedVal.ledval=LED_MASK_SYSCONF_PIRSET;
		
		while(1)
		{
				if(KeyBk.Flag!=KeyStat.Flag || KeyBk.Numb!=KeyStat.Numb)
				{
						LedVal.ledval=LED_MASK_SYSCONF_PIRSET;
						KeyBk=KeyStat;
						KeyBit=GetKeyLedBit(KeyBk.Numb);
						switch(KeyBk.Numb)
						{
								case KEY_1:               //中文
								case KEY_2:               //英文
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												SysConfig.Bits.PirOn=(KeyBk.Numb==KEY_1)?1:0;
												if(SysConfig.Bits.PirOn)
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn On":"打开"),UNBREAK);
														PowerOnPir();
												}
												else
												{
														AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Turn Off":"关闭"),UNBREAK);
														PowerOffPir();
												}
												AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Human body induction":"人体感应"),UNBREAK);
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								case KEY_x:               //返回  
										if(KeyBk.Flag!=0)
										{
												LedVal.ledval&=~(1<<KeyBit);
												AudioPlayVoice(GetVolIndex("滴"),BREAK);
												MenuNow=Menu_KeyPadSysConf; 
										}
										else
										{
												LedVal.ledval|=(1<<KeyBit);
										}
										break;
								default:break;
						}
				}
				Key_Scan();
				Audio_RunningHandle();
				Uart_RtxFrameHandle();
				Led_UpdateVal();
				if((LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)        //语音播放完成
				{
						if(VolFinishTime==0)
						{
								VolFinishTime=GetTimerCount();
						}							
						else if(GetTimerElapse(VolFinishTime)>10000)
						{
								KeyPadMode=KEYPAD_NORMAL;
								AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Exit management mode":"退出管理模式"),BREAK);
								MenuNow=Menu_KeyPadNormal;
						}
				}
				else
				{
						VolFinishTime=0;
				}
				if(MenuNow!=Menu_KeyPadSysConf_PirSet && (LPM_GetStopModeValue()&LPM_AUDIO_ID)==0)
				{
						if(MenuNow==Menu_KeyPadNormal)LedVal.ledval=LED_MASK_NORMAL;
						break;
				}
		}
}
/******************************************************************************/
/*
//获取按键对应的LED位置
input: key -- 按键编号
output:none
return:bitK -- led位置
*/
/******************************************************************************/
uint8_t GetKeyLedBit(uint8_t key)
{
		uint8_t bitK=0;
		switch(key)
		{
				case KEY_0:                    //5
						bitK=13;
						break;
				case KEY_1:
						bitK=4;
						break;
				case KEY_2:
						bitK=5;
						break;
				case KEY_3:
						bitK=6;
						break;
				case KEY_4:
						bitK=3;
						break;
				case KEY_5:
						bitK=8;
						break;
				case KEY_6:
						bitK=9;
						break;
				case KEY_7:  
						bitK=2;
						break;
				case KEY_8:
						bitK=12;
						break;
				case KEY_9:
						bitK=10;
						break;
				case KEY_x:
						bitK=1;
						break;
				case KEY_c:
						bitK=11;
						break;
				case KEY_bell:
						bitK=0;
						break;
		}
		return bitK;
}








