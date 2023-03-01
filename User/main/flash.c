#include "flash.h"
#include "ssi_drv.h" 
#include "delay.h"
#include "string.h"
#include "audio.h"
#include "eflash_drv.h"
#include "lock_config.h"
#include "eport_drv.h"



uint8_t isIntFlashInit=0;
uint8_t isSsiFlashInit=0;



/******************************************************************************/
/*
//内部FLASH初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void INT_EFLASH_Init(void)
{
		if(isIntFlashInit==1)return;
		uint32_t eflash_clk;
		eflash_clk = g_sys_clk;
		EFLASH_Init(eflash_clk/1000);  //设置EFLASH时钟           
		EFLASH_SetWritePermission();   //使能读写
		isIntFlashInit=1;	
}
/******************************************************************************/
/*
//内部FLASH除能
input:   none
output   none
return   none
*/
/******************************************************************************/
void INT_EFLASH_DeInit(void)
{
		EFLASH_ClrWritePermission();
		isIntFlashInit=0;
}

/******************************************************************************/
/*
//内部FLASH读数据
input:   pDataBuf -- 读数据缓存
         nOffset -- 数据地址
         nDataNum -- 数据长度
output   none
return   none
*/
/******************************************************************************/
void IntFlashRead(uint8_t* pDataBuf,uint32_t nOffset,uint16_t nDataNum)
{
		uint16_t i;
		for(i = 0; i < nDataNum; i ++)
		{
				pDataBuf[i] = (*(volatile uint8_t *)(EFSH_START_ADDR+nOffset+i));
		}
}


/******************************************************************************/
/*
//内部FLASH写数据：写数据地址要4字节对齐
input:   pDataBuf -- 数据缓存
         nOffset -- 数据地址
         nDataNum -- 数据长度
output   none
return   0-OK
*/
/******************************************************************************/
uint8_t IntFlashWrite(uint8_t* pDataBuf,uint32_t nOffset,uint16_t nDataNum)
{
		__disable_irq();
		INT_EFLASH_Init();
		EFLASH_Write(EFSH_START_ADDR+nOffset, pDataBuf,nDataNum);
		__enable_irq();
		return 0;
}

/******************************************************************************/
/*
//清除FLASH备份区
input:   none
output   none
return   none
*/
/******************************************************************************/
void IntFlashEraseBackup(void)
{
		uint32_t i=0;
	
		INT_EFLASH_Init();
		for(i=0;i<APP_SIZE;i+=EFLASH_PAGE_SIZE_BYTE)
		{
				EFLASH_PageErase(EFSH_START_ADDR+BOOT_SIZE+APP_SIZE+i*EFLASH_PAGE_SIZE_BYTE);
		}
}

/******************************************************************************/
/*
//外部FLASH初始化
input:   none
output   none
return   none
*/
/******************************************************************************/
void SSI_FLASH_Init(void)
{
//		if(isSsiFlashInit==1)return;
		
		SSI_Standard_Init(FLASH_SSI);
		SSI_ReleaseDeepPW(FLASH_SSI);
		SQI_CheckProtect(FLASH_SSI);
	  
		isSsiFlashInit=1;
}
/******************************************************************************/
/*
//外部FLASH除能
input:   none
output   none
return   none
*/
/******************************************************************************/
void SSI_FLASH_DeInit(void)
{
//		if(isSsiFlashInit==0)return;
		SSI_EnterDeepPW(FLASH_SSI);        //等后续测试，看是否需要处理相关IO
		isSsiFlashInit=0;
}
/******************************************************************************/
/*
//外部FLASH resume
input:   SSIx -- 外设编号
output   none
return   ret
*/
/******************************************************************************/
uint16_t SSI_ReleaseDeepPW(SSI_TypeDef *SSIx) 
{
		uint32_t timeout = SSI_TIMEOUT_COUNTERS(1000);
		uint16_t ret;
		ret = SSI_Standard_Init(SSIx);
		if(ret)
		{
				return ret;
		}
		SSIx->SSI_DR=RDP_CMD;
		__asm("nop"); __asm("nop"); __asm("nop");  
		while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
		{
				if(!timeout)
				{
						return STATUS_TIMEOUT;
				}
				timeout --;
		}
		while(SSIx->SSI_SR&SR_BUSY)
		{
				if(!timeout)
				{
						return STATUS_TIMEOUT;
				}
				timeout --;
		}	
		while(SSIx->SSI_SR&SR_RFNE)
		{
				uint32_t temp=SSIx->SSI_DR;
				if(!timeout)
				{
						return STATUS_TIMEOUT;
				}
				timeout --;
		}
	return ret;
}
/******************************************************************************/
/*
//外部FLASH 进入低功耗
input:   SSIx -- 外设编号
output   none
return   ret
*/
/******************************************************************************/
uint16_t SSI_EnterDeepPW(SSI_TypeDef *SSIx)
{
		uint32_t timeout = SSI_TIMEOUT_COUNTERS(1000);
		uint16_t ret;  
	
		ret = SSI_Standard_Init(SSIx);
		if(ret)
		{
				return ret;
		}
		SSIx->SSI_DR=DP_CMD;
		__asm("nop"); __asm("nop"); __asm("nop");   
		while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
		{
				if(!timeout)
				{
						return STATUS_TIMEOUT;
				}
				timeout --;
		}
		while(SSIx->SSI_SR&SR_BUSY)
		{
				if(!timeout)
				{
						return STATUS_TIMEOUT;
				}
				timeout --;
		}
		while(SSIx->SSI_SR&SR_RFNE)
		{
				uint32_t temp=SSIx->SSI_DR;
				if(!timeout)
				{
						return STATUS_TIMEOUT;
				}
				timeout --;
		}
		return ret;
}
/******************************************************************************/
/*
//外部FLASH解除写保护
input:   SSIx -- 外设编号
output   none
return   ret
*/
/******************************************************************************/
uint8_t SQI_CheckProtect(SSI_TypeDef *SSIx)
{
		uint8_t status;
		uint8_t status_tmp;
	
		uint32_t timeout = SSI_TIMEOUT_COUNTERS(1000);
		uint16_t ret;
		ret = SSI_EFlash_Get_Status1(SSIx,&status_tmp);
		if(ret)
		{
				return ret;
		}
		if((status_tmp&0x10) || (status_tmp&0x20))
		{
				ret = SSI_EFlash_Write_Enable(SSIx);
				if(ret)
				{
						return STATUS_TIMEOUT;
				}
				SSIx->SSI_DR=0x01;
				SSIx->SSI_DR = 0;
				//SSIx->SSI_DR=val;

				__asm("nop"); __asm("nop"); __asm("nop");
		
				//	while((SSIx->SSI_SR&SR_BUSY)==0x00){;}
				while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
				{
						if(!timeout)
						{
								return STATUS_TIMEOUT;
						}
						timeout --;
				}
        
				while(SSIx->SSI_SR&SR_BUSY)
				{
						if(!timeout)
						{
								return STATUS_TIMEOUT;
						}
						timeout --;
				}

				while(SSIx->SSI_SR&SR_RFNE)
				{
						status_tmp=SSIx->SSI_DR;
						if(!timeout)
						{
								return STATUS_TIMEOUT;
						}
						timeout --;
				}
				do
				{
						ret=SSI_EFlash_Get_Status1(SSIx,&status);
						if(ret)
						{
								return ret;
						}		 

						if(!timeout)
						{
								return STATUS_TIMEOUT;
						}
						timeout --;
				}while(status&1);
		}
		return ret;
}

/******************************************************************************/
/*
//外部FLASH读数据
input:   pData -- 数据缓存
         nAddr -- 地址
         nLen -- 长度
output   none
return   RT_OK -- 成功  RT_FAIL -- 失败
*/
/******************************************************************************/
uint8_t  ExFlashRead(uint8_t *pData, uint32_t nAddr, uint32_t nLen)
{
		uint16_t nRet;
		nRet = SSI_EFlash_QUAD_Read(FLASH_SSI, nAddr, pData, nLen);
		if( STATUS_OK!=nRet) return RT_FAIL;
		return RT_OK;
}

/******************************************************************************/
/*
//外部FLASH写整块数据
input:   pBuf -- 数据缓存
         lBlock -- 块号
				 lOffset -- 偏移
         nLen -- 长度
output   none
return   RT_OK -- 成功  RT_FAIL -- 失败
*/
/******************************************************************************/
uint8_t FlashWriteBlock(uint8_t *pBuf,uint32_t lBlock,uint32_t lOffset,uint32_t lLen)
{
		uint16_t nRet;
		uint32_t i,n,cAGFalg;
		uint32_t lStartAddr;
		__align(4) uint8_t TBufBolck[SSI_FSH_BLOCK_SIZE];
		__align(4) uint8_t TBufBolck1[SSI_FSH_BLOCK_SIZE];
		if((lOffset+lLen)>SSI_FSH_BLOCK_SIZE)return RT_FAIL;
	
		lStartAddr=SSI_FSH_START_ADDR+lBlock*SSI_FSH_BLOCK_SIZE;
	
		nRet = SSI_EFlash_QUAD_Read(FLASH_SSI, lStartAddr, TBufBolck, SSI_FSH_BLOCK_SIZE);
		if(STATUS_OK!=nRet)
		{
				nRet = SSI_EFlash_QUAD_Read(FLASH_SSI, lStartAddr, TBufBolck, SSI_FSH_BLOCK_SIZE);
		}
		if(STATUS_OK!=nRet)return RT_FAIL;
		cAGFalg=0;
		L_AG:
		if(cAGFalg++>4)return RT_FAIL;
		n=0;
		do{
				SSI_Standard_Init(FLASH_SSI);
				nRet = SSI_EFlash_Sector_Erase(FLASH_SSI, lStartAddr); /*擦除*/
				if(STATUS_OK==nRet)break;
				DelayMS(10);
		}while(n++<5);
		if(STATUS_OK!=nRet)return RT_FAIL;
		memcpy(&TBufBolck[lOffset],pBuf,lLen);
		for(i = 0; i < SSI_FSH_BLOCK_SIZE; i += SSI_FSH_PAGE_SIZE)
		{
				n=0;
				do{
						nRet = SSI_EFlash_QUAD_Program(FLASH_SSI, lStartAddr+i, &TBufBolck[i], SSI_FSH_PAGE_SIZE);
						if(STATUS_OK==nRet)break;
						DelayMS(10);
				}while(n++<5);
				if(STATUS_OK!=nRet)goto L_AG;
		}		
		memset(TBufBolck1,0,SSI_FSH_BLOCK_SIZE);
		nRet = SSI_EFlash_QUAD_Read(FLASH_SSI, lStartAddr, TBufBolck1, SSI_FSH_BLOCK_SIZE);
		if(STATUS_OK!=nRet) 
		{
				nRet = SSI_EFlash_QUAD_Read(FLASH_SSI, lStartAddr, TBufBolck1, SSI_FSH_BLOCK_SIZE);
		}
		if(STATUS_OK!=nRet)goto L_AG;
		for(i = 0; i < SSI_FSH_BLOCK_SIZE; i ++)
		{
				if(TBufBolck[i]!=TBufBolck1[i]) 
				goto L_AG;
		}		
		return RT_OK;
}
/******************************************************************************/
/*
//外部FLASH写数据
input:   pDataBuf -- 数据缓存
				 lAddress -- 偏移
         lLen -- 长度
output   none
return   RT_OK -- 成功  RT_FAIL -- 失败
*/
/******************************************************************************/
uint8_t ExFlashWrite(uint8_t *pDataBuf,uint32_t lAddress,uint32_t lLen)
{
		uint32_t StartAddr=0;                 //写数据起始地址
		uint32_t RemainCount=0;               //剩余要写的数据数量
		uint32_t nBlock,nBlockOffset;         //写数据的block地址与块上的偏移地址 
		uint8_t *pBuf;                        //数据地址
		uint8_t nRet;                         //返回值
	 
		if((SSI_FSH_END_ADDR-1)<lAddress+lLen)return RT_FAIL;		
		    
		StartAddr=lAddress;
		nBlock=(uint32_t)(StartAddr/SSI_FSH_BLOCK_SIZE);
	 
		pBuf=pDataBuf;
		RemainCount=lLen;
		do{
				nBlockOffset=(uint32_t)(StartAddr%SSI_FSH_BLOCK_SIZE);            
				if(nBlockOffset)                              //块不对齐，第一次写
				{
						if(SSI_FSH_BLOCK_SIZE-nBlockOffset>lLen)
						{
								return FlashWriteBlock(pBuf,nBlock,nBlockOffset,lLen);
						}
						nRet=FlashWriteBlock(pBuf,nBlock,nBlockOffset,SSI_FSH_BLOCK_SIZE-nBlockOffset);
						if(nRet!=RT_OK) return nRet;

						pBuf+=SSI_FSH_BLOCK_SIZE-nBlockOffset;
						StartAddr+=SSI_FSH_BLOCK_SIZE-nBlockOffset;
						RemainCount-=SSI_FSH_BLOCK_SIZE-nBlockOffset;
						nBlock++;
				}
				else if(RemainCount>=SSI_FSH_BLOCK_SIZE)    //块对齐后，数据超出一块数量
				{
						nRet=FlashWriteBlock(pBuf,nBlock,0,SSI_FSH_BLOCK_SIZE);
						if(nRet!=RT_OK) return nRet;
						pBuf+=SSI_FSH_BLOCK_SIZE;
						StartAddr+=SSI_FSH_BLOCK_SIZE;
						RemainCount-=SSI_FSH_BLOCK_SIZE;
						nBlock++;			  	
				}
				else                                //最后一次写数据，小于一块 
				{
						nRet=FlashWriteBlock(pBuf,nBlock,0,RemainCount);			 
						if(nRet!=RT_OK) return nRet;
						RemainCount=0;
				}
		}while(RemainCount>0); 
		return RT_OK;
}











