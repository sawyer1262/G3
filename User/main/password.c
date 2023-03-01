#include "password.h"
#include "flash.h"  
#include "lock_config.h"
#include <string.h>
#include "lock_record.h"
#include "rtc.h"
#include "time_cal.h"


PwdStr_t Pwd={0};                            //密码
OneTimeCode_t OneTimeCode={0};               //一次性密码


/******************************************************************************/
/*
//初始化密码数据
input:   none
output   none
return   none
*/
/******************************************************************************/
void PassWordInit(void)
{
		uint8_t i=0;
		PwdGenStr_t PwdTmp={0};
		PwdExtStr_t PwdTimTmp={0};
		//读一次性密码
		if(SysConfig.Bits.OTCode==1)
		{
				IntFlashRead((uint8_t*)&OneTimeCode,ONETIME_CODE_ADDR,sizeof(OneTimeCode_t));
				if(!CheckOneTimeCodeValid(&OneTimeCode))   //密码无效
				{
						SysConfig.Bits.OTCode=0;
				}
		}
		//读密码，并判断密码有效性
		Pwd.PwdInfo.Item.PwdSum=0;
		for(i=0;i<PWD_MAX;i++)
		{
				IntFlashRead(&PwdTmp.PwdBuf[0],PWD_DATA_ADDR+i*(PWD_LEN_MAX+3),PWD_LEN_MAX+3);
				if(PwdTmp.Pwd.Type<NORMAL || PwdTmp.Pwd.Type>VISITOR)          //检测密码类型是否有效
				{
						PwdTmp.Pwd.Type=INVALID;
				}
				else if(CheckPwdVail(PwdTmp.Pwd.Buf,PWD_LEN_MAX)<PWD_LEN_MIN)  //检测密码是否有效
				{
						PwdTmp.Pwd.Type=INVALID;
				}
				else
				{
						Pwd.PwdInfo.Item.PwdSum++;
						printf("code: %s \n",PwdTmp.Pwd.Buf);
				}
				Pwd.PwdGenStr[i]=PwdTmp;
		}
		if(Pwd.PwdInfo.Item.PwdSum==0)          //没有密码，设置默认管理员密码
		{
				Pwd.PwdGenStr[0].Pwd.Type=ADMIN;
				Pwd.PwdGenStr[0].Pwd.Idx=0;
				for(i=0;i<6;i++)
				{
						Pwd.PwdGenStr[0].Pwd.Buf[i]=i+0x31;      //默认管理员密码'1'-'6'
				}
				for(i=i;i<PWD_LEN_MAX+1;i++)
				{
						Pwd.PwdGenStr[0].Pwd.Buf[i]=0;
				}
				IntFlashWrite(&Pwd.PwdGenStr[0].PwdBuf[0],PWD_DATA_ADDR,PWD_LEN_MAX+3);
		}
		for(i=0;i<PWD_TIM_MAX;i++)
		{
				IntFlashRead(&PwdTimTmp.PwdTimBuf[0],PWD_TIM_DATA_ADDR+i*PWD_TIM_LEN_MAX,PWD_TIM_LEN_MAX);
				
				if(PwdTimTmp.PwdTimBuf[0]!=INACTIVE)     //标志有效
				{
						if(!CheckPwdTimVail(&PwdTimTmp))    //判断内容， 
						{
								memset(PwdTimTmp.PwdTimBuf,0,sizeof(PwdExtStr_t)); //内容无效，清0
								IntFlashWrite(&PwdTimTmp.PwdTimBuf[0],PWD_TIM_DATA_ADDR,PWD_TIM_LEN_MAX);
						}
						else 
						{
								Pwd.PwdInfo.Item.PwdTimSum++;
						}
				}
				Pwd.PwdTimStr[i]=PwdTimTmp;
		}
		printf("admin code: %s \n",Pwd.PwdGenStr[0].Pwd.Buf);
		printf("pwdsum: %d \n",Pwd.PwdInfo.Item.PwdSum);
		printf("pwdtimsum: %d \n",Pwd.PwdInfo.Item.PwdTimSum);
}

/******************************************************************************/
/*
//获取可用得密码SLOT
input:   none
output   none
return   SLOT--可用得SLOT
*/
/******************************************************************************/
uint8_t GetPassWordFreeSlot(void)
{
		uint8_t i=0;
	
		for(i=0;i<PWD_MAX;i++)
		{
				if(Pwd.PwdGenStr[i].Pwd.Type==INVALID)
				{
						return i;
				}
		}
		return i;
}
/******************************************************************************/
/*
//获取空闲得密码有效时间SLOT
input:   none
output   none
return   空闲得有效时间SLOT
*/
/******************************************************************************/
uint8_t GetPassWordTimFreeSlot(void)
{
		uint8_t i=0;
	
		for(i=0;i<PWD_TIM_MAX;i++)
		{
				if(Pwd.PwdTimStr[i].PwdTimBuf[0]==INACTIVE)
				{
						return i;
				}
		}
		return i;
}
/******************************************************************************/
/*
//添加密码
input:   type -- 用户类型：ADMIN-管理员 其他-其他用户
         code -- 添加的密码
         ext  -- 额外数据；有效时间等信息
output   slot---添加成功的密码编号
return   
				PWD_ADD_SUCESS            0x00         //添加成功
				PWD_ADD_TYP_WRONG         0x01         //类型错误
				PWD_ADD_MEM_FULL          0x02         //密码库已满
				PWD_ADD_TIME_MEM_FULL     0x03         //带时效密码库已满
				PWD_ADD_CODE_EXIST        0x04         //密码已存在   
*/
/******************************************************************************/
uint8_t AddPassWord(uint8_t type,uint8_t *code,PwdExtStr_t *ext,uint8_t *slot)
{
		uint8_t i=0,fristfreeslot=0xFF;
		uint8_t fristtimfreeslot=0;
		uint8_t codevalid[PWD_LEN_MAX]={0};
		
		if(slot!=0)*slot=0xFF;
	
		if(type>VISITOR)return PWD_ADD_TYP_WRONG;                                     //类型不对
		
		while(*code!=0)
		{
				codevalid[i++]=*code++;                  
		}
		if(i<PWD_LEN_MIN || i>PWD_LEN_MAX-1)return PWD_ADD_TYP_WRONG;           //<6 >12，密码6-12位
		
		if(type==ADMIN)
		{
				Pwd.PwdGenStr[0].Pwd.Type=ADMIN;
				Pwd.PwdGenStr[0].Pwd.Idx=0;
				for(i=0;i<PWD_LEN_MAX;i++)
				{
						Pwd.PwdGenStr[0].Pwd.Buf[i]=codevalid[i];      
				}
				IntFlashWrite(Pwd.PwdGenStr[0].PwdBuf,PWD_DATA_ADDR,PWD_LEN_MAX+3);
				
				LockAddUser(PERIP_CODE_INDEX,0);	
				if(slot!=0)*slot=0;
				return PWD_ADD_SUCESS;
		}
		
		if(Pwd.PwdInfo.Item.PwdSum>=PWD_MAX)return PWD_ADD_MEM_FULL;                               //密码已满
	
		if(type==VISITOR && Pwd.PwdInfo.Item.PwdTimSum>=PWD_TIM_MAX)return PWD_ADD_TIME_MEM_FULL;  //时效密码已满
		
		//查找密码是否已存在
		for(i=0;i<PWD_MAX;i++)
		{
				if(Pwd.PwdGenStr[i].Pwd.Type==INVALID)
				{
						if(fristfreeslot==0xFF)fristfreeslot=i;      //保存第一个空闲得SLOT，用于存放新添加得密码
						continue;
				}
				if(strstr((char*)Pwd.PwdGenStr[i].Pwd.Buf,(char*)codevalid)!=0)   //判断新添加得密码，是否已经存在
				{
						if(slot!=0)*slot=i;                 //输出已存在得密码编号
						return PWD_ADD_CODE_EXIST;          //密码已存在
				}
		}
		if(fristfreeslot==0xFF)return PWD_ADD_MEM_FULL;     //无空闲SLOT，返回密码库满
		//添加密码
		Pwd.PwdGenStr[fristfreeslot].Pwd.Type=type;         //密码类型
		for(i=0;i<PWD_LEN_MAX;i++)
		{
				Pwd.PwdGenStr[fristfreeslot].Pwd.Buf[i]=codevalid[i];
		}
		//添加密码时效，并写入FLASH
		if(type==VISITOR && ext!=NULL)                          //访客或临时密码      
		{
				fristtimfreeslot=GetPassWordTimFreeSlot();
				if(fristtimfreeslot==PWD_TIM_MAX)return PWD_ADD_TIME_MEM_FULL;

				Pwd.PwdGenStr[fristfreeslot].Pwd.Idx=fristtimfreeslot;    //指向密码时效
				memcpy(Pwd.PwdTimStr[fristtimfreeslot].PwdTimBuf,ext->PwdTimBuf,sizeof(PwdExtStr_t));
				IntFlashWrite(Pwd.PwdTimStr[fristtimfreeslot].PwdTimBuf,PWD_TIM_DATA_ADDR+fristtimfreeslot*sizeof(PwdExtStr_t),sizeof(PwdExtStr_t));   //保存时效
				Pwd.PwdInfo.Item.PwdTimSum++;
		}
		//写密码到FLASH
		IntFlashWrite(Pwd.PwdGenStr[fristfreeslot].PwdBuf,PWD_DATA_ADDR+fristfreeslot*sizeof(PwdGenStr_t),sizeof(PwdGenStr_t));                       //保存密码
		LockAddUser(PERIP_CODE_INDEX,fristfreeslot);	
		Pwd.PwdInfo.Item.PwdSum++;
		if(slot!=0)*slot=fristfreeslot;
		return PWD_ADD_SUCESS;                                 //添加成功
}

/******************************************************************************/
/*
//删除密码
input:   slot  -- 密码ID
output   none
return   
	       PWD_DEL_SUCCESS           0x00         //删除密码成功
         PWD_DEL_CODE_NOT_EXIST    0x01         //密码不存在
*/
/******************************************************************************/
uint8_t DelPassWord(uint8_t slot)
{
		uint8_t delIdx=slot;
	
		if(delIdx>=100 && delIdx!=0xFF)return PWD_DEL_CODE_NOT_EXIST;    //错误得删除序号
		
		if(delIdx==0xFF)
		{
				//清密码，不清管理员
				memset(Pwd.PwdGenStr[1].PwdBuf,0,(PWD_MAX-1)*sizeof(PwdGenStr_t));    //清空密码,不清管理员
				IntFlashWrite(Pwd.PwdGenStr[0].PwdBuf,PWD_DATA_ADDR,PWD_MAX*sizeof(PwdGenStr_t));
				//清时效
				memset(Pwd.PwdTimStr[0].PwdTimBuf,0,PWD_TIM_MAX*sizeof(PwdExtStr_t));
				IntFlashWrite(Pwd.PwdTimStr[0].PwdTimBuf,PWD_TIM_DATA_ADDR,PWD_TIM_MAX*sizeof(PwdExtStr_t));
				//写记录
				LockDelUser(PERIP_CODE_INDEX,delIdx);				
				//重新初始化密码
				PassWordInit();                                  //重新读数据
				return PWD_DEL_SUCCESS;
		}
		if(Pwd.PwdGenStr[delIdx].Pwd.Type==INVALID)return PWD_DEL_CODE_NOT_EXIST;     //密码本来无效
		//清除时效段
		if(Pwd.PwdGenStr[delIdx].Pwd.Type==VISITOR && Pwd.PwdTimStr[Pwd.PwdGenStr[delIdx].Pwd.Idx].PwdTimBuf[0]!=INACTIVE)     //访客清时间
		{
				memset(Pwd.PwdTimStr[Pwd.PwdGenStr[delIdx].Pwd.Idx].PwdTimBuf,0,sizeof(PwdExtStr_t));
				IntFlashWrite(Pwd.PwdTimStr[Pwd.PwdGenStr[delIdx].Pwd.Idx].PwdTimBuf,PWD_TIM_DATA_ADDR+Pwd.PwdGenStr[delIdx].Pwd.Idx*sizeof(PwdExtStr_t),sizeof(PwdExtStr_t));
				printf("delet ok\n");
				Pwd.PwdInfo.Item.PwdTimSum--;
		}
		//清密码段
		memset(Pwd.PwdGenStr[delIdx].PwdBuf,0,sizeof(PwdGenStr_t)); 
		Pwd.PwdGenStr[delIdx].Pwd.Idx=0;
		Pwd.PwdGenStr[delIdx].Pwd.Type=INVALID;
		IntFlashWrite(Pwd.PwdGenStr[delIdx].PwdBuf,PWD_DATA_ADDR+delIdx*sizeof(PwdGenStr_t),sizeof(PwdGenStr_t)); 
		//写操作记录
		LockDelUser(PERIP_CODE_INDEX,delIdx);
		//更新密码信息
		Pwd.PwdInfo.Item.PwdSum--;
		return PWD_DEL_SUCCESS;
}
/******************************************************************************/
/*
//验证密码
input:   code  -- 待验证的密码
output   out   -- 密码时效等信息
return   
         0-99                                   //密码在库中的ID
	       PWD_IDY_ONETIME_CODE      0xFE         //一次性密码
         PWD_IDY_CODE_NOT_EXIST    0xFF         //密码不存在
*/
/******************************************************************************/
uint8_t IdyPassWord(uint8_t *code)
{
		uint8_t i=0,len=0;
	
		uint8_t tmpCode[27]={0};                    //最大26位输入密码+结束符
		//获取密码输入信息
		while(*code!=0)
		{
				tmpCode[i++]=*code++;                   //暂存密码
		}                                    
		printf("CODE LEN: %d \n",i);
		len=i;    //输入有效密码长度
		if(len<PWD_LEN_MIN)return PWD_IDY_CODE_NOT_EXIST;   //长度小于最小长度
		//验证是否是一次性密码
		if(len==PWD_LEN_MIN && SysConfig.Bits.OTCode==1)    //存在一次性密码，一次性密码6位
		{
				for(i=0;i<PWD_LEN_MIN;i++)
				{
						if(tmpCode[i]!=OneTimeCode.OneCode[i])break;
				}
				if(i==PWD_LEN_MIN)    //一次性密码验证一次即删除
				{
						memset((uint8_t*)&OneTimeCode,0,sizeof(OneTimeCode_t));
						SysConfig.Bits.OTCode=0;
						IntFlashWrite((uint8_t*)&OneTimeCode,ONETIME_CODE_ADDR,sizeof(OneTimeCode_t));
						return PWD_IDY_ONETIME_CODE;
				}
		}
		//验证普通密码，支持虚位密码
		for(i=0;i<PWD_MAX;i++)
		{
				if(Pwd.PwdGenStr[i].Pwd.Type==INVALID)continue;         //无效密码退出
				if(strstr((char*)tmpCode,(char*)Pwd.PwdGenStr[i].Pwd.Buf)!=0)break;
		}
		printf("CODE index: %d \n",i);
		if(i==PWD_MAX)return PWD_IDY_CODE_NOT_EXIST;           //密码不存在
		//密码库存在输入密码，判断是否是临时密码，是临时密码则验证时效
		if(Pwd.PwdGenStr[i].Pwd.Type==VISITOR)
		{
				uint16_t tmpst=0,tmpsp=0,tmpnow=0;
				t_ctimes gt={0};
				tm rtc_times={0}; 
				RTC_GetTime(&rtc_times);	//获取芯片RTC时间
				rtctime_to_mytime(&gt,rtc_times);	//获取时间戳
				
				if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimBuf[0]==PWD_TIME_CYC)        //周期有效密码
				{
						if((1<<gt.weeks)&Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimCyc.Week)  //在周期里
						{
								tmpnow=gt.hours*60+gt.minutes;
								tmpst=(uint16_t)Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimCyc.StartHour*60+Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimCyc.StartMin;
								tmpsp=(uint16_t)Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimCyc.StopHour*60+Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimCyc.StopMin;
								if(tmpnow>=tmpst && tmpnow<=tmpsp)
								{
										return i;          //在有效期内
								}
								else
								{
										return PWD_IDY_CODE_NOT_EXIST; 
								}
						}
						else 
						{
								return PWD_IDY_CODE_NOT_EXIST; 
						}
				}
				else if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimBuf[0]==PWD_TIME_FEQ)   //有效时间内，限次数密码
				{
						if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimeFreq.Times>0)
						{
								Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimeFreq.Times--;
								IntFlashWrite(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimBuf,PWD_TIM_DATA_ADDR+Pwd.PwdGenStr[i].Pwd.Idx*sizeof(PwdExtStr_t),sizeof(PwdExtStr_t));   //更新次数，并写入FLASH
								printf("times left:%d\n",Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimeFreq.Times);
								if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimeFreq.Times==0)   //次数用完
								{         
										DelPassWord(i);              //删除密码
								}
								return i;
						}
						else
						{
								return PWD_IDY_CODE_NOT_EXIST;
						}
				}
				else
				{
						return PWD_IDY_CODE_NOT_EXIST;    //基本不存在这种情况
				}
		}
		//for(j=0;j<13;j++)printf("code: 0x%02x ",Pwd.PwdStr[i].Pwd.Buf[j]);
		return i;
}

/******************************************************************************/
/*
//验证密码有效性
input:   pPwd  -- 待验证的密码
output   nLen  -- 密码长度
return   j     -- 有效密码的位数
*/
/******************************************************************************/
uint8_t CheckPwdVail(uint8_t* pPwd,uint8_t nLen)
{
		uint8_t j;
	
		for(j=0;j<nLen;j++)
		{
				if(pPwd[j]<'0' || pPwd[j]>'9')break;
		}
		return j;
}
/******************************************************************************/
/*
//验证时效密码的时效设置是否有效
input:   time  -- 临时密码结构
output   none
return   0 -- 无效   1 -- 有效
*/
/******************************************************************************/
uint8_t CheckPwdTimVail(PwdExtStr_t *time)
{
		if(time->PwdTimBuf[0]==PWD_TIME_CYC)      //访客密码
		{
				if(time->PwdTimCyc.Week==0 || time->PwdTimCyc.Week>0x7f)return 0;
				else if(time->PwdTimCyc.StartHour>24)return 0;
				else if(time->PwdTimCyc.StartMin>59)return 0;
				else if(time->PwdTimCyc.StopHour>24)return 0;
				else if(time->PwdTimCyc.StopMin>59)return 0;
				else if(time->PwdTimCyc.TimeOutUtc<1589212800)return 0;    //2020年5月12
				return 1;
		}
		else if(time->PwdTimBuf[0]==PWD_TIME_FEQ)     //临时密码 
		{
				if(time->PwdTimeFreq.Times==0)return 0;
				else if(time->PwdTimeFreq.TimeOutUtc<1589212800)return 0;    //2020年5月12
				
				return 1;
		}
		return 0;
}
/******************************************************************************/
/*
//删除所有密码
input:   none
output   none
return   none
*/
/******************************************************************************/
void DelAllPassword(void)
{
		//删除密码区域
		memset(Pwd.PwdGenStr,0,PWD_MAX*sizeof(PwdGenStr_t));
		//删除时效区域
		memset(Pwd.PwdTimStr,0,PWD_TIM_MAX*sizeof(PwdExtStr_t));
		//写入FLASH
		IntFlashWrite((uint8_t*)&Pwd.PwdGenStr,PWD_DATA_ADDR,PWD_MAX*sizeof(PwdGenStr_t));
		IntFlashWrite((uint8_t*)&Pwd.PwdTimStr,PWD_TIM_DATA_ADDR,PWD_TIM_MAX*sizeof(PwdExtStr_t));
		//重新读出密码数据
		PassWordInit();
}
/******************************************************************************/
/*
//检测时效密码是否有效
input:   none
output   none
return   0 -- 无效   1 -- 有效
*/
/******************************************************************************/
uint8_t CheckOneTimeCodeValid(OneTimeCode_t* onecode)  
{
		uint8_t i=0;
	
		for(i=0;i<6;i++)
		{
				if(onecode->OneCode[i]<'0' || onecode->OneCode[i]>'9')
				{
						return 0;
				}
		}
		if(onecode->StartTime.hour>23 || onecode->StartTime.minute>59 || onecode->StartTime.second>59)
		{
				return 0;
		}
		return 1;
}
/******************************************************************************/
/*
//获取键盘输入的有效密码：去除*#
input:   in -- 键盘输入的字符
         out -- 去除*#后剩余的字符
output   none
return   none
*/
/******************************************************************************/
uint8_t GetCodeWithoutX(char *in,char *out)
{
		uint8_t i=0,j=0;
		
		for(i=0;i<strlen(in);i++)
		{
				if(in[i]=='*')
				{
						if(j>0)j--;     
				}
				else if(in[i]=='#')
				{
						j=0;
				}
				else 
				{
						out[j++]=in[i];    
				}
		}
		return 0;
}
/******************************************************************************/
/*
//检查管理员密码是不是默认的密码123456
input:   none
output   none
return   0 -- 管理员密码不是1，2，3，4，5，6
         1 -- 管理员密码是1，2，3，4，5，6
*/
/******************************************************************************/
uint8_t CheckAdminCode(void)
{
		uint8_t i=0;
		uint8_t defaultCode[PWD_LEN_MAX]={'1','2','3','4','5','6',0,0,0,0,0,0,0};
		for(i=0;i<PWD_LEN_MAX;i++)
		{
				if(Pwd.PwdGenStr[0].Pwd.Buf[i]!=defaultCode[i])break;
		}
		if(i<PWD_LEN_MAX)return 0;
		
		return 1;
}
/******************************************************************************/
/*
//验证一次性密码是否存在
input:   code -- 输入的一次性密码
output   none
return   1 -- 密码已存在   0 -- 密码可用
*/
/******************************************************************************/
uint8_t CheckOnetimeCodeExist(uint8_t *code)
{
		uint8_t i=0,j=0,len1=0,len2=0;
	
		len2=strlen((char*)code);
	
		for(i=0;i<PWD_MAX;i++)
		{
				if(Pwd.PwdGenStr[i].Pwd.Type==INVALID)continue;
				
				len1=strlen((char*)Pwd.PwdGenStr[i].Pwd.Buf);
				
				if(len1==len2)
				{
						for(j=0;j<len1;j++)
						{
								if(Pwd.PwdGenStr[i].Pwd.Buf[j]!=code[j])break;
						}
						if(j==len1)return 1;
				}
		}
		return 0;
}

/******************************************************************************/
/*
//判断临时密码是否失效

input:   stime--指定时间
output   none
return   时间差：
            -1 ----- 当前时间小于指定时间
            >0 ----- 时间差
*/
/******************************************************************************/
void CheckTempCodeValid(void)
{
		uint8_t i=0;
		uint32_t timestampnow=0;
		t_ctimes gt={0};
		tm rtc_times={0}; 
		
		if(NetTime.NT.Stat==0)return;   //未获取到时间不检测
		
		//时间转换未时间戳
		RTC_GetTime(&rtc_times);	//获取芯片RTC时间
		rtctime_to_mytime(&gt,rtc_times);	//获取时间戳
		timestampnow=mytime_to_utc(&gt,0);
		//判断临时密码是否失效
	  for(i=0;i<PWD_MAX;i++)
		{
				if(Pwd.PwdGenStr[i].Pwd.Type==VISITOR)        //临时密码
				{
						if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimBuf[0]==PWD_TIME_CYC)              //周期密码
						{
								if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimCyc.TimeOutUtc<timestampnow)   //失效时间到
								{
										DelPassWord(i);
								}
						}
						else if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimBuf[0]==PWD_TIME_FEQ)
						{
								if(Pwd.PwdTimStr[Pwd.PwdGenStr[i].Pwd.Idx].PwdTimeFreq.TimeOutUtc<timestampnow)   //失效时间到
								{
										DelPassWord(i);
								}
						}
				}
		}
}


