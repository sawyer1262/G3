#include "card.h"
#include "timer.h"
#include "sky1311_drv.h"
#include "sky1311t.h"
#include "iso14443.h"
#include "string.h"
#include "lpm.h"
#include "lock_config.h"
#include "audio.h"
#include "keypad_menu.h"
#include "key.h"
#include "i2c_drv.h"
#include "uart_fp.h"
#include "uart_back.h"
#include "uart.h"
#include "led.h"
#include "rtc.h"
#include "lock_record.h"


CardStat_t CdStat; 
CardBaseInfo_t CardBaseInfo;
CardInfo_t CardInfo={0};
CardUser_t CardUser[CARD_MAX_NUM];

uint8_t isCardInit=0;


/******************************************************************************/
/*
//刷卡IO初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void CardIoInit(void)
{
		if(isCardInit==1)return;
	
		CardUserInit();
		RFID_PortInit();
		
		isCardInit=1;
}
/******************************************************************************/
/*
//刷卡初始化
input:   none
output   none
return   none 
*/
/******************************************************************************/
void CardInit(void)
{
		if(isCardInit==0)CardIoInit();
		SKY1311_ENABLE();
		sky1311Init();	
		typeAOperate();
		DelayMS(1);
}
/******************************************************************************/
/*
//刷卡除能
input:   none
output   none
return   none 
*/
/******************************************************************************/
void CardDeInit(void)
{
	//	if(isCardInit==0)return;
		RFID_PortDeInit();
		isCardInit=0;
}
/******************************************************************************/
/*
//寻卡--500ms一次
input:   none
output   none
return   none 
*/
/******************************************************************************/
void CardSearchHandle(void)
{
		uint8_t tmpBuf[10]={0};
		
		if(SysConfig.Bits.CardLock==1)return;                                //人脸锁定
		if(LPM_GetStopModeValue()==0)return;                                 //需要进低功耗，就不用再检卡了 
		
	//	Audio_PowerOff();
		CardInit();
		if(piccWakeupA(CardBaseInfo.ATQA)==Ok) 
		{
				uint8_t *uid=&CardBaseInfo.UID[0];
				uint8_t sel=SEL1;
				uint8_t isOk=0;
				CardBaseInfo.UIDSize=CardBaseInfo.ATQA[0]>>6;
				CardBaseInfo.SAK=0;
				do{
						if(piccAntiA(sel,1,uid)==Ok) 
						{
								uint8_t checkSum = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
								if(checkSum == uid[4])
								{
										if(piccSelectA(sel,uid,tmpBuf)==Ok)
										{
												CardBaseInfo.SAK=tmpBuf[0];
												if(CardBaseInfo.UIDSize!=0 && uid[0] == 0x88)
												{
														for(uint8_t i=0;i<3;i++)
														{
																uid[i] = uid[i+1];
														}
														uid += 3;
														sel += 2;
												}
												else
												{
														isOk=1;	
														printf("card id: [0]=0x%02x,[1]=0x%02x,[2]=0x%02x,[3]=0x%02x \n",uid[0],uid[1],uid[2],uid[3]);
														printf("card sak: 0x%02x\n",CardBaseInfo.SAK);
														printf("card atqa: [0]=0x%02x,[1]=0x%02x\n",CardBaseInfo.ATQA[0],CardBaseInfo.ATQA[1]);
												}
										}
								}
						}
				}while((CardBaseInfo.SAK&0x04)!=0);
				
				if(isOk==1 && SysConfig.Bits.Activate==0)
				{
						AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
						AudioPlayVoice(GetVolIndex(SysConfig.Bits.Language?"Please activate the product":"请激活产品"),UNBREAK);
						PirLed_SetStat(LOGO_OPEN_SUCCESS);
						
						UserControlLock(CTL_OPEN_LOCK,CARD_OPEN,0xFF);
				}
				else if(isOk==1 && mifare1_ReadTest(CardBaseInfo.UID,NULL)==Ok)
				{
						uint8_t idyokidx=0;
						
						CardUser_t CardUserTmp={0};
						CardUserTmp.Card.Uid[0]=CardBaseInfo.UID[0];
						CardUserTmp.Card.Uid[1]=CardBaseInfo.UID[1];
						CardUserTmp.Card.Uid[2]=CardBaseInfo.UID[2];
						CardUserTmp.Card.Uid[3]=CardBaseInfo.UID[3];
						
						idyokidx=IdfCard(&CardUserTmp);
						
						if(idyokidx<CARD_MAX_NUM)      //0-99
						{
								tmpBuf[0]=0;
								LockUpdatePeripLockStat(PERIP_CARD_INDEX,0);
							
								if(SysConfig.Bits.MutiIdy==1)
								{
										if(MutiIdyItem&(~IDY_CARD))        //非密码验证成功+密码验证成功,双重验证成功
										{	
												MutiIdyItem=0;
												AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
												PirLed_SetStat(LOGO_OPEN_SUCCESS);
												UserControlLock(CTL_OPEN_LOCK,CARD_OPEN,idyokidx);
										}
										else                              //置卡验证成功标志 
										{
												MutiIdyItem|=IDY_CARD;
												PirLed_SetStat(LOGO_OPEN_SUCCESS_MUTIL);
										}
								}
								else
								{
										AudioPlayVoice(GetVolIndex("指纹音"),BREAK);						//语音播报
										PirLed_SetStat(LOGO_OPEN_SUCCESS);
										UserControlLock(CTL_OPEN_LOCK,CARD_OPEN,idyokidx);
								}
						}
						else      //GMT卡，但是未添加
						{
								AudioPlayVoice(GetVolIndex("检卡音"),BREAK);       //验证失败
								AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
								PirLed_SetStat(LOGO_OPEN_FAIL);
								LockUpdatePeripLockStat(PERIP_CARD_INDEX,1);
						}
				}
				else if(isOk==1)         //无效卡（非GMT卡）
				{
						AudioPlayVoice(GetVolIndex("检卡音"),BREAK);       //验证失败
						AudioPlayVoice(GetVolIndex("检卡音"),UNBREAK);
						PirLed_SetStat(LOGO_OPEN_FAIL);
					  LockUpdatePeripLockStat(PERIP_CARD_INDEX,1);
				}
		}
		sky1311Reset();
	//	Audio_PowerOn();
}
/******************************************************************************/
/*
//添加卡片
input:   type:添加卡类型
             00   ---- 管理员
             0-99 ---- 普通卡
output   none
return   添加卡片返回状态 
            
             FF--未检测到卡
						 ADD_CARD_SUCESS            0   --添加成功            
						 ADD_CARD_FULL              1   --卡片库已满
						 ADD_CARD_EXIST             2   --卡片已存在 
						 ADD_CARD_FAIL              3   --添加失败 
*/
/******************************************************************************/
uint8_t CardAddHandle(uint8_t type)
{
		CardBaseInfo_t CdInfo={0};
		uint8_t addok=0xFF;
		
		CardInit();
		if(piccWakeupA(CardBaseInfo.ATQA)==Ok) 
		{
				uint8_t tmpBuf[10]={0};
				uint8_t *uid=&CdInfo.UID[0];
				uint8_t sel=SEL1;
				uint8_t isOk=0;
				CdInfo.UIDSize=CdInfo.ATQA[0]>>6;
				CdInfo.SAK=0;
				do{
						if(piccAntiA(sel,1,uid)==Ok) 
						{
								uint8_t checkSum = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
								if(checkSum == uid[4])
								{
										if(piccSelectA(sel,uid,tmpBuf)==Ok)
										{
												CdInfo.SAK=tmpBuf[0];
												if(CdInfo.UIDSize!=0 && uid[0] == 0x88)
												{
														for(uint8_t i=0;i<3;i++)
														{
																uid[i] = uid[i+1];
														}
														uid += 3;
														sel += 2;
												}
												else
												{
														isOk=1;
												}
										}
								}
						}
				}while((CdInfo.SAK&0x04)!=0);
				
				if(isOk==1)
				{
						if(mifare1_ReadTest(CdInfo.UID,NULL)==Ok)
						{
								CardUser_t CardUserTmp={0};
								
								CardUserTmp.Card.Uid[0]=CdInfo.UID[0];
								CardUserTmp.Card.Uid[1]=CdInfo.UID[1];
								CardUserTmp.Card.Uid[2]=CdInfo.UID[2];
								CardUserTmp.Card.Uid[3]=CdInfo.UID[3];
								
								CardUserTmp.Card.Roll[0]=0;
								CardUserTmp.Card.Roll[1]=0;
								CardUserTmp.Card.Roll[2]=0;
								CardUserTmp.Card.Roll[3]=1;
								addok=AddCard(&CardUserTmp,type);
						}
						else 
						{
								addok=ADD_CARD_FAIL;
						}
				}
		}
		sky1311Reset();
		return addok;
}

/******************************************************************************/
/*
//读取卡用户
input:   none
output   none
return   none 
*/
/******************************************************************************/
void CardUserInit(void)
{
		uint8_t i=0;
		//检测卡用户是否已经初始化
		
		IntFlashRead(CardInfo.CardBuf,CARD_INFO_ADDR,sizeof(CardInfo_t));  
		if(CardInfo.Card.InitFlag!=0xAA)
		{
				CardInfo.Card.InitFlag=0xAA;
				CardInfo.Card.CardSum=0;
				CardInfo.Card.Rsv1=0;
				CardInfo.Card.Rsv2=0;
				IntFlashWrite(CardInfo.CardBuf,CARD_INFO_ADDR,sizeof(CardInfo_t)); 
				//写默认数据0
				CardUser_t CardUserTmp={0};
				for(i=0;i<CARD_MAX_NUM;i++)
				{
						IntFlashWrite(CardUserTmp.CardBuf,CARD_DATA_ADDR+i*sizeof(CardUser_t),sizeof(CardUser_t));  
				}
		}
		//读卡用户，并计算有效卡数量
		CardInfo.Card.CardSum=0;     //复位卡数量，重新计算
		for(i=0;i<CARD_MAX_NUM;i++)
		{
				IntFlashRead(CardUser[i].CardBuf,CARD_DATA_ADDR+i*sizeof(CardUser_t),sizeof(CardUser_t)); 
				if(!CheckCardInvalid(&CardUser[i]))   //卡有效
				{
						CardInfo.Card.CardSum++;
				}					
		}
}

/******************************************************************************/
/*
//检测卡号是否有效
input:   usr---卡用户
output   none
return   0--卡有效   1--无效卡
*/
/******************************************************************************/
uint8_t CheckCardInvalid(CardUser_t *usr)
{
		uint8_t i=0;
		for(i=0;i<8;i++)
		{
				if(usr->CardBuf[i]!=0)return 0; 
		}
		return 1;
}
/******************************************************************************/
/*
//获取可用的卡用户slot
input:   none
output   none
return   空闲slot
*/
/******************************************************************************/
uint8_t GetCardFreeSlot(void)
{
		uint8_t i=0;
	
		for(i=0;i<CARD_MAX_NUM;i++)
		{
				if(CheckCardInvalid(&CardUser[i]))
				{
						return i;
				}
		}
		return i;
}
/******************************************************************************/
/*
//添加卡片
input:   card --- 卡用户的信息
         type --- 卡类型 ：ADD_ADMIN：管理员卡  ADD_NORMAL：普通卡
output   none
return   
				 ADD_CARD_SUCESS            0       --添加成功     
				 ADD_CARD_FULL              1       --卡片库满
				 ADD_CARD_EXIST             2       --卡片已存在
				 ADD_CARD_FAIL              3	      --添加失败  
*/
/******************************************************************************/
uint8_t AddCard(CardUser_t* card,uint8_t type)
{
		uint8_t i=GetCardFreeSlot();      //
		uint8_t j=0,k=0;     

		//添加管理员卡
		if(type==ADD_ADMIN)                        //添加管理员卡，直接覆盖
		{
				CardUser[0]=*card;
				IntFlashWrite(CardUser[0].CardBuf,CARD_DATA_ADDR,sizeof(CardUser_t));  
				
				LockAddUser(PERIP_CARD_INDEX,0);	
				return ADD_CARD_SUCESS;
		}
		//卡片库已满
		if(i==CARD_MAX_NUM)return ADD_CARD_FULL;   //非管理员卡，卡已满
		//检测卡片卡号是否已存在
		for(j=0;j<CARD_MAX_NUM;j++)
		{
				for(k=0;k<4;k++)         //只匹配4字节卡号
				{
						if(CardUser[j].Card.Uid[k]!=card->Card.Uid[k])
						{
								break;           //卡号不同
						}
				}
				if(k==4)break;           //卡号已存在 
		}
		if(j<CARD_MAX_NUM)return ADD_CARD_EXIST;       //卡已存在
		//存储卡号
		CardUser[i]=*card;
		IntFlashWrite(CardUser[i].CardBuf,CARD_DATA_ADDR+i*sizeof(CardUser_t),sizeof(CardUser_t));
		
		LockAddUser(PERIP_CARD_INDEX,i);	

		return ADD_CARD_SUCESS;
}
/******************************************************************************/
/*
//删除卡片
input:   id1 --- 卡ID

output   none
return   
				 DEL_CARD_SUCCESS           0   --删除成功
         DEL_CARD_NOEXIST           1   --卡片不存在
*/
/******************************************************************************/
uint8_t DelCard(uint8_t id1)
{
		uint8_t id=id1;
		//卡号不存在
		if(id>=CARD_MAX_NUM && id!=0xFF)return DEL_CARD_NOEXIST;
		//删除所有卡
		if(id==0xFF)           //删除所有卡片
		{
				//删除
				printf("carduser size: 0x%02x\n",sizeof(CardUser));
				memset(CardUser,0,sizeof(CardUser));
				IntFlashWrite((uint8_t*)&CardUser,CARD_DATA_ADDR,sizeof(CardUser));
				LockDelUser(PERIP_CARD_INDEX,id);
				//删完重新读
				CardUserInit();
		}
		else                 //删除单张卡片
		{
				//判断要删除的卡片是否存在
				if(CheckCardInvalid(&CardUser[id]))return DEL_CARD_NOEXIST;
				//删除卡片
				memset(CardUser[id].CardBuf,0,sizeof(CardUser_t));
				IntFlashWrite(CardUser[id].CardBuf,CARD_DATA_ADDR+id*sizeof(CardUser_t),sizeof(CardUser_t));
			
				LockDelUser(PERIP_CARD_INDEX,id);
		}
		return DEL_CARD_SUCCESS;
}
/******************************************************************************/
/*
//查找指定卡在库中的ID号
input:   card --- 卡信息

output   none
return   
				 n------- 卡的ID号
*/
/******************************************************************************/
uint8_t IdfCard(CardUser_t* card)
{
		uint8_t j=0,k=0;
		for(j=0;j<CARD_MAX_NUM;j++)
		{
				for(k=0;k<4;k++)
				{
						if(CardUser[j].Card.Uid[k]!=card->Card.Uid[k])
						{
								break;  
						}
				}
				if(k==4)return j;                    //UID相等
		}
		return j;
}








