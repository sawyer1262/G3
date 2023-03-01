// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : xip_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "delay.h"
#include "debug.h"
#include "ssi_reg.h"
#include "cache_drv.h"
#include "xip_drv.h"

/*******************************************************************************
* Function Name  : SSI_XIP_Enable
* Description    : 使能SSI XIP
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_XIP_Enable(SSI_TypeDef *SSIx)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
    }
        
    SSIx->SSI_SSIENR= 0x00;
    SSIx->XIP_INCR_INST= 0xeb;
    SSIx->XIP_WRAP_INST= 0x0c;	
    SSIx->SSI_BAUDR= 0x04;
    SSIx->XIP_CTRL= 0x28c0046a|(6<<13); //prefech	

    SSIx->SSI_CTRLR0= 0x0080001f;
    SSIx->SSI_SPI_CTRLR0= 0x6830021a|(2<<11);
    SSIx->SSI_XIP_MODE_BITS = 0x00000000;
    SSIx->SSI_SSIENR= 0x01;

    SSIx->SSI_SR;
	CPM_SSICFG|=0x09;
    
    return STATUS_OK;
} 
/*******************************************************************************
* Function Name  : SSI_XIP_Clear_CMP
* Description    : 清除SSI flash cmp标志
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_XIP_Clear_CMP(SSI_TypeDef *SSIx)
{
    unsigned char temp;
    UINT16 ret = STATUS_OK;
    
    ret = SSI_Standard_Init(SSIx);
    if(ret)
    {
        return ret;
    }

    ret=SSI_EFlash_Get_Status2(SSIx,&temp);
    if(ret)
    {
        return ret;
    }
    printf("[status2]: 0x%02x\r\n", temp);
    ret = SSI_EFlash_Write_Enable(SSIx);
    if(ret)
    {
        return ret;
    }
    delay(0x1000);
    ret = SSI_EFlash_Prog_Status2(SSIx, 0x00);
    if(ret)
    {
        return ret;
    }
    ret =SSI_EFlash_Get_Status2(SSIx,&temp);
    if(ret)
    {
        return ret;
    }
    printf("[status2]: 0x%02x\r\n", temp);
    while(1);
}
 
/*******************************************************************************
* Function Name  : SSI_XIP_Switch
* Description    : 切换SSI QPI
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_XIP_Switch(SSI_TypeDef *SSIx)
{
	UINT16 ret;
	ret = SSI_EFlash_QPI_Enter(SSIx);  
    if(ret)
    {
        return ret;
    }
    ret = SSI_EFlash_Set_Read_Para(SSIx,6);
    if(ret)
    {
        return ret;
    }
	ret = SSI_XIP_Enable(SSIx);
    if(ret)
    {
        return ret;
    }
	
//	printf("SSI-QPI switched!\r\n");
    return ret;
}	

void SSI_XIP_Off(SSI_TypeDef *SSIx)
{    
    SSIx->SSI_SR;
    CPM_SSICFG = ((CPM_SSICFG & ~0x09)|0x08);
    SSIx->SSI_SR;
}	
 
/*******************************************************************************
* Function Name  : SSI_XIP_Data_Cache_Enable
* Description    : 使能SSI XIP data cache
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : None
******************************************************************************/
void SSI_XIP_Data_Cache_Enable()
{
	DPCCRGS_H &= SPIM1_CACHEOFF;
	DPCCRGS_H |= (WRITE_THROUGH << SPIM1_CACHE_SHIFT);
	
  if(DCACHE->CACHE_CCR&ENCACHE)
	{
		 printf("XIP DATA CHACHE Failed: already enabled!\r\n");
//		 while(1)
//		 {
//		   ;
//		 }	 
	}	
	else
	{
	   DCACHE->CACHE_ACRG&=0xffff00ff;
		 DCACHE->CACHE_ACRG|=0xaa00;		
		 DCACHE->CACHE_CCR = 0x85000031;
     printf("XIP SPACE data cache enabled!\r\n");	
	}		
}	

/*******************************************************************************
* Function Name  : SSI_XIP_Instruction_Cache_Enable
* Description    : 使能SSI XIP instruction cache
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : None
******************************************************************************/
void SSI_XIP_Instruction_Cache_Enable(void)
{
	IPCCRGS_H &= SPIM1_CACHEOFF;
	IPCCRGS_H |= (WRITE_THROUGH << SPIM1_CACHE_SHIFT);
  if(ICACHE->CACHE_CCR&ENCACHE)
	{
		 printf("XIP INSTRUCTION CHACHE Failed: already enabled!\r\n");
//		 while(1)
//		 {
//		   ;
//		 }	 
	}	
	else
	{
	   ICACHE->CACHE_ACRG&=0xffff00ff;
		 ICACHE->CACHE_ACRG|=0xaa00;		
		 ICACHE->CACHE_CCR = 0x85000031;
     printf("XIP SPACE INSTRUCTION cache enabled!\r\n");	
	}		
}	

