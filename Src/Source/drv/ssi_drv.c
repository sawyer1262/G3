// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : ssi_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "delay.h"
#include "debug.h"
#include "common.h"
#include "ssi_drv.h"
#include "dmac_drv.h"
#include "delay.h"

volatile int dma_int =0;

/*******************************************************************************
* Function Name  : SSI_Standard_Init
* Description    : SSI Standard 初始化
* Input          : - SSIx: SSI 基地址
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_Standard_Init(SSI_TypeDef *SSIx)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    
	while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	SSIx->SSI_SSIENR = 0x00;
	SSIx->SSI_CTRLR1 = 0x00;
	SSIx->SSI_CTRLR0 =  0x07;
	SSIx->SSI_BAUDR = 0x04;
	SSIx->SSI_TXFTLR = 0x00;
	SSIx->SSI_RXFTLR = 0x00;
	SSIx->SSI_SPI_CTRLR0 = 0x8000;
	SSIx->SSI_IMR = 0x00;
	SSIx->SSI_SSIENR = 0x01;
        
    while(SSIx->SSI_SR&SR_BUSY)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	return STATUS_OK;
}	

UINT16 SSI_Wait_Eflash_Idle(SSI_TypeDef *SSIx)
{
	 unsigned char status;
     UINT16 ret = STATUS_OK;
     UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
 	 
	 do
	 {
		 ret=SSI_EFlash_Get_Status1(SSIx,&status);
         if(ret)
         {
             return ret;
         }
		 //printf("status: 0x%02x\r\n", status);
         if(!timeout)
         {
             return STATUS_TIMEOUT;
         }
         timeout --;
	 }while(status&0x01);

     return ret;
}

UINT16 SSI_Wait_Eflash_QPI_Idle(SSI_TypeDef *SSIx)
{
	 unsigned char status;
     UINT16 ret = STATUS_OK;
     UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
 	 
	 do
	 {
		 ret=SSI_EFlash_QPI_Get_Status1(SSIx,&status);
         if(ret)
         {
             return ret;
         }
		 //printf("status: 0x%02x\r\n", status);
         if(!timeout)
         {
             return STATUS_TIMEOUT;
         }
         timeout --;
	 }while(status&0x01);

     return ret;
}	

/*******************************************************************************
* Function Name  : SSI_Software_Reset
* Description    : SSI 复位SPI flash
* Input          : - SSIx: SSI 基地址
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_Software_Reset(SSI_TypeDef *SSIx)
{
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

	while(SSIx->SSI_SR&SR_BUSY)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
	
	SSIx->SSI_SSIENR = 0x00;
	SSIx->SSI_CTRLR1 = 0x00;
	SSIx->SSI_CTRLR0 = 0x00800407;
	SSIx->SSI_SPI_CTRLR0 = 0x40000202|(2<<11);
	SSIx->SSI_BAUDR =  0x4;
	SSIx->SSI_IMR = 0x00;
	SSIx->SSI_SSIENR = 0x01;
    
    SSIx->SSI_SR;
//    while(SSIx->SSI_SR&SR_BUSY){;}
//    delay(0x1000);
	
	SSIx->SSI_DR=0x66;

	__asm("nop");	__asm("nop");	__asm("nop");
    
    while((SSIx->SSI_SR&SR_TFE)==0x00)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    
    delay(0x1000);
	timeout = SSI_TIMEOUT_COUNTERS(1000);

	SSIx->SSI_DR=0x99;

	__asm("nop");	__asm("nop");	__asm("nop");
	
	while((SSIx->SSI_SR&SR_TFE)==0x00)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
		
	while(SSIx->SSI_SR&SR_BUSY)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	return STATUS_OK;
}

/*******************************************************************************
* Function Name  : SSI_QPI_Software_Reset
* Description    : SSI QPI模式复位SPI flash
* Input          : - SSIx: SSI 基地址
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QPI_Software_Reset(SSI_TypeDef *SSIx)
{
    volatile unsigned char status;
	volatile unsigned char temp;
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret;

	ret = SSI_QPI_Init(SSIx, CMD_WRITE,0,2);
	if(ret)
	{
		return ret;
	}
	
	SSIx->SSI_DR=0x66;
    
    __asm("nop");	__asm("nop");	__asm("nop");
    
    while((SSIx->SSI_SR&SR_TFE)==0x00)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

    delay(0x100);
	timeout = SSI_TIMEOUT_COUNTERS(1000);

	SSIx->SSI_DR=0x99;
        
    __asm("nop");	__asm("nop");	__asm("nop");
	
	while((SSIx->SSI_SR&SR_TFE)==0x00)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
		
	while(SSIx->SSI_SR&SR_BUSY)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
        
    delay(0x2000);

	return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_EFlash_QPI_Enter
* Description    : SSI EFlash 进入QPI模式
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Enter(SSI_TypeDef *SSIx)
{
	unsigned char status;
	volatile unsigned char temp;
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret = STATUS_OK;
	
    ret = SSI_EFlash_Prog_Status2(SSIx, 0x02);
    if(ret)
    {
        return ret;
    }
	
	SSIx->SSI_DR=QPI_ENTER_CMD;

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
		temp=SSIx->SSI_DR;
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
	}
	
	do
	{
		ret=SSI_EFlash_QPI_Get_Status1(SSIx,&status);	
        if(ret)
        {
            return ret;
        }	 
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;

	}while(status&0x01);
	
    return ret;
}

/*******************************************************************************
* Function Name  : SSI_EFlash_QPI_Exit
* Description    : SSI EFlash 退出QPI模式
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Exit(SSI_TypeDef *SSIx)
{
	unsigned char status;
	volatile unsigned char temp;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret;
	
	ret = SSI_QPI_Init(SSIx, CMD_WRITE,0,2);
	if(ret)
    {
        return ret;
    }

	SSIx->SSI_DR=QPI_EXIT_CMD;

	__asm("nop"); __asm("nop"); __asm("nop");
	
	ret = SSI_Standard_Init(SSIx);
	if(ret)
    {
        return ret;
    }

    do
	{
		ret = SSI_EFlash_Get_Status1(SSIx,&status);		 
        if(ret)
        {
            return ret;
        }
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }while(status&0x01);

    return ret;
	
}

/*******************************************************************************
* Function Name  : SSI_DUAL_Init
* Description    : SSI DUAL 初始化
* Input          : - SSIx				: SSI 基地址
* Input          : - data_len		: 需要读的数据长度
* Input          : - waitCycles	: 等待时间
*
* Output         : None
* Return         : None
******************************************************************************/
void SSI_DUAL_Init(SSI_TypeDef *SSIx, UINT32 data_len, UINT8 waitCycles)
{
	SSIx->SSI_SSIENR = 0x00;	 
	SSIx->SSI_BAUDR =  QUAD_BAUDR;
	SSIx->SSI_CTRLR1 = data_len-1;
		
	SSIx->SSI_CTRLR0 = 07;
	SSIx->SSI_SPI_CTRLR0 = 0;
		
	SSIx->SSI_CTRLR0 |= (CTRLR0_SPI_FRF_DUAL|CTRLR0_FRF_MOT|CTRLR0_DFS_VALUE(0x07)|CTRLR0_TMOD_RX_ONLY);
	SSIx->SSI_SPI_CTRLR0|= ( CTRLR0_TRANS_TYPE_TT0
                            |CTRLR0_ADDR_L_VALUE(0x06)
                            |CTRLR0_INST_L_VALUE(0x02)
                            |CTRLR0_WAIT_CYCLES_VALUE(waitCycles)
                            |CTRLR0_CLK_STRETCH_EN_MASK);
	SSIx->SSI_TXFTLR = 0x00010000;
	SSIx->SSI_RXFTLR = 0x7;	
	
	if(QUAD_BAUDR==0x02&&g_sys_clk>=80000000)//high speed
	{
	 SSIx->SSI_RX_SAMPLE_DELAY=0x00010001;
	}
	
	SSIx->SSI_IMR = 0x00;
	SSIx->SSI_SSIENR = 0x01;
}

/*******************************************************************************
* Function Name  : SSI_QUAD_Init
* Description    : SSI QUAD 初始化
* Input          : - SSIx      : SSI 基地址
*								 ：- read      ：读写标志
*								 ：- num       ：数据帧
*								 ：- waitCycles：等待时间
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QUAD_Init(SSI_TypeDef *SSIx, int read, int num, int waitCycles)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

    SSIx->SSI_SSIENR = 0x00;
    SSIx->SSI_CTRLR1 = ((num == 0)?0:(num-1));
    SSIx->SSI_BAUDR =  0x04;
    SSIx->SSI_SPI_CTRLR0 = 0x40000218|(waitCycles<<11);
        
    if(read)
    {
        SSIx->SSI_CTRLR0 = 0x00800807;
        SSIx->SSI_TXFTLR = 0x00010000;
        SSIx->SSI_RXFTLR = 0x07;
    }
    else
    {		 
        SSIx->SSI_CTRLR0 = 0x00800407;
        
        if(num == 0)
        {
            SSIx->SSI_TXFTLR = 0x00;
        }
        else
        {
            SSIx->SSI_TXFTLR = 0x00020000;
        }
    }	
    
    SSIx->SSI_IMR = 0x00;
    SSIx->SSI_SSIENR = 0x01;
    
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_QPI_Init
* Description    : SSI QUAD 初始化 (以byte为传输单位)
* Input          : - SSIx      : SSI 基地址
*								 ：- mode      ：执行模式
*								   CMD_READ		 ：发送读指令，无地址位
*								   CMD_WRITE	 ：发送写指令，无地址位不支持inst+addr模式，配置成inst+3B数据，不能配置成inst+addr(addr+data),因为硬件加密时会把address中的数加密，而inst+3B数据没有地址位，不会对3B数据加密
*								   DATA_READ	 ：发送读数据指令，有地址位
*								   DATA_WRITE	 ：发送写数据指令，有地址位
*								 ：- data_len  ：数据帧   (写最大配置为256 byte,读最大配置为64k byte)
*								 ：- waitCycles：等待时间
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QPI_Init(SSI_TypeDef *SSIx, QPI_OPT_MODE mode, UINT32 data_len, UINT8 waitCycles)
{  
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

    while(SSIx->SSI_SR&SR_BUSY)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    
	SSIx->SSI_SSIENR = 0x00;	 
	SSIx->SSI_BAUDR =  QUAD_BAUDR;
	SSIx->SSI_CTRLR1 = ((data_len == 0)?0:(data_len-1));
		
	SSIx->SSI_CTRLR0 = 07;
	SSIx->SSI_SPI_CTRLR0 = 0;
		
	if(mode == CMD_READ)
	{
		SSIx->SSI_CTRLR0 |= (CTRLR0_SPI_FRF_QUAD|CTRLR0_FRF_MOT|CTRLR0_DFS_VALUE(0x07)|CTRLR0_TMOD_RX_ONLY);
		SSIx->SSI_SPI_CTRLR0|= ( CTRLR0_TRANS_TYPE_TT2
                                |CTRLR0_ADDR_L_VALUE(0x00)
                                |CTRLR0_INST_L_VALUE(0x02)
                                |CTRLR0_WAIT_CYCLES_VALUE(waitCycles)
                                |CTRLR0_CLK_STRETCH_EN_MASK);
		SSIx->SSI_TXFTLR = 0x00010000;
	  SSIx->SSI_RXFTLR = 0x07;
	}
	else if(mode == CMD_WRITE)
	{
		SSIx->SSI_CTRLR0 |= (CTRLR0_SPI_FRF_QUAD|CTRLR0_FRF_MOT|CTRLR0_DFS_VALUE(0x07)|CTRLR0_TMOD_TX_ONLY);
		SSIx->SSI_SPI_CTRLR0|= ( CTRLR0_TRANS_TYPE_TT2
                                |CTRLR0_ADDR_L_VALUE(0x00)
                                |CTRLR0_INST_L_VALUE(0x02)
                                |CTRLR0_WAIT_CYCLES_VALUE(waitCycles)
                                |CTRLR0_CLK_STRETCH_EN_MASK);
		if(data_len == 0)
		{
			SSIx->SSI_TXFTLR = 0x00;
		}
		else
		{
			SSIx->SSI_TXFTLR = 0x00010000;
		}
		SSIx->SSI_RXFTLR = 0x00;
	}
	else if(mode == DATA_READ)
	{
		SSIx->SSI_CTRLR0 |= (CTRLR0_SPI_FRF_QUAD|CTRLR0_FRF_MOT|CTRLR0_DFS_VALUE(0x07)|CTRLR0_TMOD_RX_ONLY);
		SSIx->SSI_SPI_CTRLR0|= ( CTRLR0_TRANS_TYPE_TT2
                                |CTRLR0_ADDR_L_VALUE(0x06)
                                |CTRLR0_INST_L_VALUE(0x02)
                                |CTRLR0_WAIT_CYCLES_VALUE(waitCycles)
                                |CTRLR0_CLK_STRETCH_EN_MASK);
		SSIx->SSI_TXFTLR = 0x00010000;
	    SSIx->SSI_RXFTLR = 0x07;
	}
	else //DATA_WRITE
	{
		SSIx->SSI_CTRLR0 |= (CTRLR0_SPI_FRF_QUAD|CTRLR0_FRF_MOT|CTRLR0_DFS_VALUE(0x07)|CTRLR0_TMOD_TX_ONLY);
		SSIx->SSI_SPI_CTRLR0|= ( CTRLR0_TRANS_TYPE_TT2
                                |CTRLR0_ADDR_L_VALUE(0x06)
                                |CTRLR0_INST_L_VALUE(0x02)
                                |CTRLR0_WAIT_CYCLES_VALUE(waitCycles)
                                |CTRLR0_CLK_STRETCH_EN_MASK);
		SSIx->SSI_TXFTLR = 0x00020000;
		SSIx->SSI_RXFTLR = 0x00;
		SSIx->SSI_DMATDLR = 0x02;	
	}

	
	if(QUAD_BAUDR==0x02&&g_sys_clk>=80000000)//high speed
	{
	    SSIx->SSI_RX_SAMPLE_DELAY=0x00010001;
	}
	
	SSIx->SSI_IMR = 0x00;
	SSIx->SSI_SSIENR = 0x01;
    
    while(SSIx->SSI_SR&SR_BUSY)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	return STATUS_OK;
}

 /*******************************************************************************
* Function Name  : SSI_QUAD_Burst_Init
* Description    : SSI QUAD 初始化  (以byte为传输单位，Burst传输,传输长度需要是4的倍数）
* Input          : - SSIx      : SSI 基地址
*								 ：- read      ：读写标志 1:read  0:write
*								 ：- num       ：数据帧   (写最大配置为256 byte,读最大配置为64k byte)
*								 ：- waitCycles：等待时间
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QUAD_Burst_Init(SSI_TypeDef *SSIx, int read, int num, int waitCycles)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

    SSIx->SSI_SSIENR = 0x00;
    SSIx->SSI_CTRLR1 = ((num == 0)?0:(num-1));
    if(read)
    {
        SSIx->SSI_CTRLR0 = 0x00800807;
    }
    else
    {		 
        SSIx->SSI_CTRLR0 = 0x00800407;
    }	 
    SSIx->SSI_BAUDR =  QUAD_BAUDR;
    if(read)
    {
        SSIx->SSI_TXFTLR = 0x00010000;
        SSIx->SSI_RXFTLR = 0x07;	 
        SSIx->SSI_DMARDLR= 0x03;		
    }
    else
    {		 
        SSIx->SSI_TXFTLR = 0x00020000;
        SSIx->SSI_RXFTLR = 0x00;
    }	 
    if(QUAD_BAUDR==0x02&&g_sys_clk>=80000000)//high speed
    {
        SSIx->SSI_RX_SAMPLE_DELAY=0x00010000;
    }
    SSIx->SSI_SPI_CTRLR0 = 0x40000218|(waitCycles<<11);
    SSIx->SSI_IMR = 0x00;
    SSIx->SSI_SSIENR = 0x01;

    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_DMA_ISR
* Description    : SSI 中断响应函数
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void SSI_DMA_ISR(void)
{
	if(m_dma_control->DMA_STATTFR&0x01) dma_int=1;
	if(m_dma_control->DMA_STATTFR&0x02) dma_int=2;
}


/*******************************************************************************
* Function Name  : SSI_EFlash_Get_Status2
* Description    : 获取SSI Flash 状态2 
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Get_Status2(SSI_TypeDef *SSIx, UINT8 *FlashStatus)
{ 
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret;
    
    ret = SSI_Standard_Init(SSIx);
	if(ret)
	{
		return ret;
	}
		
	SSIx->SSI_DR=GET_SAT2_CMD;
	SSIx->SSI_DR=DUMMY_BYTE;
	
	__asm("nop"); __asm("nop"); __asm("nop");
		
//	while((SSIx->SSI_SR&SR_BUSY)!=SR_BUSY){}
	while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	while(SSIx->SSI_SR&SR_BUSY)
	{
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
	
	 while(SSIx->SSI_SR&SR_RFNE)
	 {
		*FlashStatus=SSIx->SSI_DR;
		if(0 == timeout)
		{
			return STATUS_TIMEOUT;
		}
		timeout --;
	 }	
	
	 return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_QPI_Get_Status2
* Description    : QPI 模式获取SSI Flash 状态2 
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Get_Status2(SSI_TypeDef *SSIx, UINT8 *FlashStatus)
{ 
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    UINT16 ret;
	
    ret = SSI_QPI_Init(SSIx, CMD_READ , 1, 2);
    if(ret)
    {
        return ret;
    }
		
	SSIx->SSI_DR=GET_SAT2_CMD;
	
	__asm("nop"); __asm("nop"); __asm("nop");
		
	while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
	
	 while(SSIx->SSI_SR&SR_RFNE)
	 {
			*FlashStatus=SSIx->SSI_DR;
            if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
	 }	
	
	 return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_Prog_Status2
* Description    : 设置SSI Flash 状态2 
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Prog_Status2(SSI_TypeDef *SSIx, unsigned char val)
{
	unsigned char status;
	volatile unsigned char retVal;
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret;

	ret = SSI_EFlash_Get_Status2(SSIx,&status);
	if(ret)
	{
		return ret;
	}

	if((status&0x02) == 0)
	{
        ret = SSI_EFlash_Write_Enable(SSIx);
        if(ret)
        {
            return STATUS_TIMEOUT;
        }

        SSIx->SSI_DR=PROG_STA2_CMD;
        SSIx->SSI_DR=val;

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
            retVal=SSIx->SSI_DR;
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


/*******************************************************************************
* Function Name  : SSI_EFlash_Set_Read_Para
* Description    : SSI EFlash 设置读参数
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Set_Read_Para(SSI_TypeDef *SSIx,int waitCycles)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	volatile unsigned char temp;

    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
	SSIx->SSI_SSIENR = 0x00;
	SSIx->SSI_CTRLR1 = 0x00;
	SSIx->SSI_CTRLR0 = 0x00800407;
	SSIx->SSI_SPI_CTRLR0 = 0x40000202|(waitCycles<<11);
	SSIx->SSI_BAUDR =  0x4;
	SSIx->SSI_IMR = 0x00;
	SSIx->SSI_SSIENR = 0x01;
        
    if(waitCycles == 2)
    {
        temp = 0x00;
    }
    else if(waitCycles == 4)
    {
        temp = 0x11;
    }
    else if(waitCycles == 6)
    {
        temp = 0x21;
    }
    else
    {
        temp = 0x31;
    }
    SSIx->SSI_SR;

	SSIx->SSI_DR=SET_READ_PARA_CMD;
	SSIx->SSI_DR=temp;

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
		temp=SSIx->SSI_DR;
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
	}		

    return STATUS_OK;
}		

/*******************************************************************************
* Function Name  : Read_ID_Test
* Description    : 读取SPIFlash ID  
* Input          : - SSIx: SSI 基地址
* Output         : Flash ID
* Return         :STATUS
******************************************************************************/
UINT16 Read_ID(SSI_TypeDef *SSIx, UINT8 *ID_buf)
{
	int i;
	volatile unsigned short ID;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

	SSIx->SSI_DR=READ_ID_CMD;
	SSIx->SSI_DR = 0x00;
	SSIx->SSI_DR = 0x00;
	SSIx->SSI_DR = 0x00;

	SSIx->SSI_DR=DUMMY_BYTE;
	SSIx->SSI_DR=DUMMY_BYTE;	

	__asm("nop");	__asm("nop");	__asm("nop");

	while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }
    
	while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }

	for(i=0; i<4; i++)
	{
		ID = SSIx->SSI_DR;
	}	 

	*ID_buf = SSIx->SSI_DR;
	*(ID_buf+1)= SSIx->SSI_DR;
	
	return STATUS_OK;
}		

/*******************************************************************************
* Function Name  : SSI_EFlash_Read
* Description    : 读SSI Flash 数据  
* Input          : - SSIx: SSI 基地址
*								 ：- addr：数据地址
*								 ：- buf ：数据缓存
*								 ：- num ：数据长度
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_DUAL_Read(SSI_TypeDef *SSIx, unsigned int addr, unsigned char *buf, int num)
{   
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret = STATUS_OK;
	volatile unsigned char temp;
	int i=0;
	
	SSI_DUAL_Init(SSIx,num,8);

	SSIx->SSI_DR = DUAL_READ_CMD;
	SSIx->SSI_DR = addr;
	
	__asm("nop"); __asm("nop"); __asm("nop");
	
	while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
	{
		if(!timeout)
		{
			return STATUS_TIMEOUT;
		}
		timeout --;
	}

	while( ((num>0)&&(SSIx->SSI_SR&SR_BUSY))||(SSIx->SSI_SR&SR_RFNE) )
	{
		if(SSIx->SSI_SR&SR_RFNE)
		{
            *(buf+i)=SSIx->SSI_DR;
            i++;
            num--;
		}	
		if(!timeout)
		{
			return STATUS_TIMEOUT;
		}
		timeout --;
	}	

	return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_Get_Status1
* Description    : 获取SSI Flash 状态1 
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Get_Status1(SSI_TypeDef *SSIx,UINT8 *FlashStatus)
{ 
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret;

    ret = SSI_Standard_Init(SSIx);
	if(ret)
	{
		return ret;
	}

	SSIx->SSI_DR=GET_SAT1_CMD;
	SSIx->SSI_DR=DUMMY_BYTE;	
	
	__asm("nop"); __asm("nop"); __asm("nop");

//	while((SSIx->SSI_SR&SR_BUSY)!=SR_BUSY){;}
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
		*FlashStatus=SSIx->SSI_DR;

		if(!timeout)
		{
			return STATUS_TIMEOUT;
		}
		timeout --;
	}	

	return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_Get_Status1
* Description    : 获取SSI Flash 状态1 
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Get_Status1(SSI_TypeDef *SSIx, UINT8 *FlashStatus)
{ 
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret = STATUS_OK;
	volatile unsigned char retVal;
	
	ret = SSI_QPI_Init(SSIx, CMD_READ , 1, 2);
	if(ret)
	{
		return ret;
	}

	SSIx->SSI_DR=GET_SAT1_CMD;	
	
	__asm("nop"); __asm("nop"); __asm("nop");

//	while((SSIx->SSI_SR&SR_BUSY)!=SR_BUSY){;}
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
		*FlashStatus=SSIx->SSI_DR;
		if(!timeout)
		{
			return STATUS_TIMEOUT;
		}
		timeout --;
	}	

	return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_Write_Enable
* Description    : SSI Flash写使能 
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Write_Enable(SSI_TypeDef *SSIx)
{
	UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	UINT16 ret;
	unsigned char status;
	volatile unsigned char temp;

	ret = SSI_Standard_Init(SSIx);
	if(ret)
	{
		return ret;
	}

	ret = SSI_EFlash_Get_Status1(SSIx,&status);
	if(ret)
	{
		return ret;
	}

	if((status&0x02) == 0)
	{
		SSIx->SSI_DR=WRITE_EN_CMD;
	
		__asm("nop"); __asm("nop"); __asm("nop");
		
		while((SSIx->SSI_SR&SR_TFE)==0x00)
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
			temp=SSIx->SSI_DR;//清空fifo
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

		}while(status&0x01);  
	}

    return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_QPI_Write_Enable
* Description    : SSI Flash写使能 
* Input          : - SSIx: SSI 基地址
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Write_Enable(SSI_TypeDef *SSIx)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    UINT16 ret;
	unsigned char status;
	volatile unsigned char temp;
	
    ret = SSI_EFlash_QPI_Get_Status1(SSIx,&status);
    if(ret)
    {
        return ret;
    }
	if((status&0x02) == 0)
	{
		ret = SSI_QPI_Init(SSIx, CMD_WRITE , 0, 2);
        if(ret)
        {
            return ret;
        }
		
		SSIx->SSI_DR=WRITE_EN_CMD;
	
		__asm("nop"); __asm("nop"); __asm("nop");
			
		while(SSIx->SSI_SR&SR_BUSY)
        {
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
		
		do
		{
			ret=SSI_EFlash_QPI_Get_Status1(SSIx,&status);	
            if(ret)
            {
                return ret;
            }
            if(!timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;

		}while(status&0x01);
	}

    return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_Chip_Erase
* Description    : 整片擦除SPI Flash
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Chip_Erase(SSI_TypeDef *SSIx)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(100000);
    UINT16 ret;
	unsigned char status;
	unsigned char temp;
	
	temp = temp;
	
	ret = SSI_EFlash_Write_Enable(SSIx);
    if(ret)
    {
        return ret;
    }
    
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	SSIx->SSI_DR=CHIP_ERASE_CMD;
	
	__asm("nop"); __asm("nop"); __asm("nop");
	
	while((SSIx->SSI_SR&SR_TFE)==0x00)
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
        temp=SSIx->SSI_DR;//清空fifo
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

    }while(status&0x01);

    return ret;
	 
}

/*******************************************************************************
* Function Name  : SSI_EFlash_Sector_Erase
* Description    : 扇区擦除SPI Flash，1sector = 4KB 
* Input          : - SSIx: SSI 基地址
* Input          : - addr: 擦除地址，0x1000对齐
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Sector_Erase(SSI_TypeDef *SSIx, unsigned int addr)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    UINT16 ret;
	unsigned char status;
	unsigned char temp;
	
	temp = temp;
	
	ret = SSI_EFlash_Write_Enable(SSIx);
    if(ret)
    {
        return ret;
    }
    
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(!timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }

	SSIx->SSI_DR=SECT_ERASE_CMD;
	SSIx->SSI_DR=(addr>>16)&0xff;
	SSIx->SSI_DR=(addr>>8)&0xff;
	SSIx->SSI_DR=(addr>>0)&0xff;
	
	__asm("nop"); __asm("nop"); __asm("nop");
	
	while((SSIx->SSI_SR&SR_TFE)==0x00)
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
        temp=SSIx->SSI_DR;//清空fifo
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

    }while(status&0x01);

    return ret;
	 
}

/*******************************************************************************
* Function Name  : SSI_EFlash_QPI_Sector_Erase
* Description    : SSI QPI模式扇区擦除SPI EFlash，1sector = 4KB
* Input          : - SSIx: SSI 基地址
* Input          : - addr: 擦除地址，0x1000对齐
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Sector_Erase(SSI_TypeDef *SSIx, unsigned int addr)
{
	unsigned char status;
	unsigned char temp;
    unsigned short ret = STATUS_OK;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	
	temp = temp;
	
	ret = SSI_EFlash_QPI_Write_Enable(SSIx);
    if(STATUS_OK != ret)    
    {
        return ret;
    }
	
	ret = SSI_QPI_Init(SSIx, CMD_WRITE , 3, 2);  
    if(STATUS_OK != ret)    
    {
        return ret;
    }

	SSIx->SSI_DR=SECT_ERASE_CMD;
	SSIx->SSI_DR=(addr>>16)&0xFF;
	SSIx->SSI_DR=(addr>>8)&0xFF;
	SSIx->SSI_DR=addr&0xFF;
	
	__asm("nop"); __asm("nop"); __asm("nop");
	
	while((SSIx->SSI_SR&SR_TFE)==0x00)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	 
    }
		
	while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	 
    }

	do
	{
	    ret=SSI_EFlash_QPI_Get_Status1(SSIx, &status);
        if(STATUS_OK != ret)
        {
            return ret;
        }
        //printf("status: 0x%02x\r\n", status);
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}while(status&0x01);
	
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	 
    }
	 
    return ret;
	 
}

/*******************************************************************************
* Function Name  : SSI_EFlash_Program
* Description    : SSI Flash 写数据  
* Input          : - SSIx: SSI 基地址
*								 ：- addr：数据地址
*								 ：- buf ：数据缓存
*								 ：- num ：数据长度
* Output         : None
* Return         : STAYUS
******************************************************************************/
UINT16 SSI_EFlash_Program(SSI_TypeDef *SSIx, unsigned int addr, unsigned char *buf, int num)
{
	volatile unsigned char temp;
	unsigned char status;
    unsigned short ret;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	
	ret = SSI_EFlash_Write_Enable(SSIx);	
    if(STATUS_OK != ret)
    {
        return ret;
    }	
    
    delay(100);

	SSIx->SSI_DR=PAGE_PROG_CMD;
	SSIx->SSI_DR=(addr>>16)&0xff;
	SSIx->SSI_DR=(addr>>8)&0xff;
	SSIx->SSI_DR=(addr>>0)&0xff;
	 
#ifdef INTERRUPT_TEST
	 SSIx->SSI_IMR=0x01;
#endif	 

	while(num>0)
	{	
        if(SSIx->SSI_SR&SR_TFNF)
        {
             SSIx->SSI_DR=*buf;
             buf++;
             num--; 
        }	

        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}	


	while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }
	 

	while(SSIx->SSI_SR&SR_RFNE)
	{
        temp=SSIx->SSI_DR;
        if(0 == timeout)
        {
        return STATUS_TIMEOUT;
        }
        timeout --;	
	}	
	 
	do
	{
        ret=SSI_EFlash_Get_Status1(SSIx, &status);
        if(STATUS_OK != ret)
        {
            return ret;
        }
        //printf("[status]-2: 0x%02x\r\n", status);
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	 
	}while(status&0x01);

    return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_Read
* Description    : 读SSI Flash 数据  
* Input          : - SSIx: SSI 基地址
*								 ：- addr：数据地址
*								 ：- buf ：数据缓存
*								 ：- num ：数据长度
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_Read(SSI_TypeDef *SSIx, unsigned int addr, unsigned char *buf, int num)
{   
	volatile unsigned char temp;
	int i=0,j=0;
	int txnum;
    UINT16 ret;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    
    ret = SSI_Standard_Init(SSIx);
    if(ret)
    {
        return ret;
    }

	txnum=num;
	SSIx->SSI_DR = READ_CMD;
	SSIx->SSI_DR = (addr>>16)&0xff;
	SSIx->SSI_DR = (addr>>8)&0xff;
	SSIx->SSI_DR = (addr>>0)&0xff;
	
	__asm("nop"); __asm("nop"); __asm("nop");
	
	while(num>0)
	{	 	 
		if(SSIx->SSI_SR&SR_RFNE)
		{
			if(j < 4)
			{
				temp=SSIx->SSI_DR;
				j++;
			}
			else
			{
				*(buf+i)=SSIx->SSI_DR;
				 i++;
				 num--;
			}
		}
        
        if( (SSIx->SSI_SR&SR_TFNF)&&(txnum>0) )
		{
			SSIx->SSI_DR=DUMMY_BYTE;
			txnum--;
		}

        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}

	while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }
    return ret;
}	

/*******************************************************************************
* Function Name  : SSI_EFlash_QUAD_Program
* Description    : SSI Flash QUAD 方式写数据  
* Input          : - SSIx: SSI 基地址
*								 ：- addr：数据地址
*								 ：- buf ：数据缓存
*								 ：- num ：数据长度
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QUAD_Program(SSI_TypeDef *SSIx, unsigned int addr, unsigned char *buf, int num)
{
    volatile unsigned char temp;
    unsigned char status;
    unsigned short ret;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

	ret = SSI_EFlash_Prog_Status2(SSIx, 0x02);
    if(ret)
    {
        return ret;
    }
    
    ret = SSI_EFlash_Write_Enable(SSIx); 
    if(ret)
    {
        return ret;
    }

    ret = SSI_QUAD_Init(SSIx, 0, num, 0);
    if(ret)
    {
        return ret;
    }

    SSIx->SSI_DR=QUAD_PROG_CMD;
    SSIx->SSI_DR=addr;

    while(num>0)
    {	
        if(SSIx->SSI_SR&SR_TFNF)
        {
            SSIx->SSI_DR=*buf;
            buf++;
            num--; 
        }
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;		
    }	

    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }

    while(SSIx->SSI_SR&SR_RFNE)
    {
        temp=SSIx->SSI_DR;
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }	

    do
    {
        ret=SSI_EFlash_Get_Status1(SSIx,&status);
        if(STATUS_OK != ret)
        {
            return ret;
        }
        //printf("[status]-2: 0x%02x\r\n", status);
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }while(status&0x01);	

    return ret;
}

/*******************************************************************************
* Function Name  : SSI_EFlash_QUAD_Read
* Description    : SSI Flash QUAD 方式读数据  
* Input          : - SSIx: SSI 基地址
*								 ：- addr：数据地址
*								 ：- buf ：数据缓存
*								 ：- num ：数据长度
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QUAD_Read(SSI_TypeDef *SSIx, unsigned int addr, unsigned char *buf, int num)
{
	volatile unsigned char temp;
	volatile unsigned char status;
	int i=0;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    UINT16 ret;
    
    ret = SSI_EFlash_Prog_Status2(SSIx, 0x02);
    if(ret)
    {
        return ret;
    }

	ret = SSI_QUAD_Init(SSIx, 1, num, 8);  
    if(ret)
    {
        return ret;
    }

	SSIx->SSI_DR=QUAD_READ_CMD;
	SSIx->SSI_DR=addr;
	
	__asm("nop");__asm("nop");__asm("nop");
	
//	while((SSIx->SSI_SR&SR_BUSY)!= SR_BUSY){;}
	while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }

	while( ((num>0)&&(SSIx->SSI_SR&SR_BUSY))||(SSIx->SSI_SR&SR_RFNE) )
	{
		if(SSIx->SSI_SR&SR_RFNE)
		{
				*(buf+i)=SSIx->SSI_DR;
				i++;
				num--;
		}	
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}	
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }

    return ret;
}

/*******************************************************************************
* Function Name  : SSI_EFlash_QPI_Program
* Description    : SSI Flash QUAD 方式写数据  
* Input          : - SSIx: SSI 基地址
*								 ：- addr：数据地址
*								 ：- buf ：数据缓存
*								 ：- num ：数据长度
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Program(SSI_TypeDef *SSIx, unsigned int addr, unsigned char *buf, int num)
{
	volatile unsigned char temp;
	unsigned char status;
    unsigned short ret;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
	
	ret = SSI_EFlash_QPI_Write_Enable(SSIx);
	if(ret)
    {
        return ret;
    }
    
    ret = SSI_QPI_Init(SSIx, DATA_WRITE, num, 2);
    if(ret)
    {
        return ret;
    }
	
	SSIx->SSI_DR=PAGE_PROG_CMD;
	SSIx->SSI_DR=addr;

	while(num>0)
	{	
		if(SSIx->SSI_SR&SR_TFNF)
		{
		 SSIx->SSI_DR=*buf;
		 buf++;
		 num--; 
		}	
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}	

	while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }

	do
	{
        ret=SSI_EFlash_QPI_Get_Status1(SSIx,&status);
        if(STATUS_OK != ret)
        {
            return ret;
        }
        //printf("[status]-2: 0x%02x\r\n", status);
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}while(status&0x01);

    return ret;	
}

/*******************************************************************************
* Function Name  : SSI_EFlash_QPI_Read
* Description    : SSI Flash QUAD 方式读数据  
* Input          : - SSIx: SSI 基地址
*								 ：- addr：数据地址
*								 ：- buf ：数据缓存
*								 ：- num ：数据长度
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_EFlash_QPI_Read(SSI_TypeDef *SSIx, unsigned int addr, unsigned char *buf, int num)
{
	int i=0;
    volatile unsigned char status;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);
    UINT16 ret;

    ret = SSI_EFlash_Set_Read_Para(SSIx,2);
    if(ret)
    {
        return ret;
    }
	ret = SSI_QPI_Init(SSIx, DATA_READ, num, 2);
    if(ret)
    {
        return ret;
    }

	SSIx->SSI_DR=QPI_READ_CMD;
	SSIx->SSI_DR=addr;
	
	__asm("nop"); __asm("nop"); __asm("nop");

	while((SSIx->SSI_SR&SR_TFE)!=SR_TFE)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }

	while( ((num>0)&&(SSIx->SSI_SR&SR_BUSY))||(SSIx->SSI_SR&SR_RFNE) )
	{
		if(SSIx->SSI_SR&SR_RFNE)
		{
            *(buf+i)=SSIx->SSI_DR;
            i++;
            num--; 
		}	
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}	
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }

    return ret;
}

/*******************************************************************************
* Function Name  : SSI_STD_DMA_Trig
* Description    : 启动SSI STD DMA  
* Input          : - SSIx   : SSI 基地址
*								 ：- cmd    ：操作指令
*								 ：- addr   ：地址
*								 ：- dmaConf：传输方式
* Output         : None
* Return         : None
******************************************************************************/
void SSI_STD_DMA_Trig(SSI_TypeDef *SSIx, unsigned cmd, unsigned int addr, int dmaConf)
{	 
	SSIx->SSI_DR=cmd;
	SSIx->SSI_DR=(addr>>16)&0xff;
	SSIx->SSI_DR=(addr>>8)&0xff;
	SSIx->SSI_DR=(addr>>0)&0xff;
    
    SSIx->SSI_DMACR=dmaConf;
}	

/*******************************************************************************
* Function Name  : SSI_STD_DMA_Send
* Description    : spi dma传送函数
* Input          : - SPIx: SPI 基地址
*                - dma_ch: DMA channel
*				          - psend: 发送数据地址
*                  _ addr: SPI flash地址
*				         - length： 传输数据长度
*				         - binten: 是否开启中断模式
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_STD_DMA_Send(SSI_TypeDef *SSIx,int dma_ch, UINT8* psend, UINT32 addr, UINT32 length, BOOL binten)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

	m_dma_control->DMA_CONFIG = 1;
	//Tx
	m_dma_channel[dma_ch]->DMA_SADDR = (UINT32)psend;
	m_dma_channel[dma_ch]->DMA_DADDR = (UINT32)&SSIx->SSI_DR;
	if(binten == TRUE)
	{
		m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B|SBSIZE_4|DBSIZE_4;
	}
	else
	{
		m_dma_channel[dma_ch]->DMA_CTRL = DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B|SBSIZE_4|DBSIZE_4;
	}
	m_dma_channel[dma_ch]->DMA_CTRL_HIGH = length ;  //最大长度为0x0FFF
	m_dma_channel[dma_ch]->DMA_CFG = (HS_SEL_DST_HARD);
	if(SSIx == SSI1)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = DST_PER_SPI_TX(3);
	}
	else if(SSIx == SSI2)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = DST_PER_SPI_TX(2);
	}
	else
	{
		printf("unknown SSIx\r\n");
		return STATUS_ID_ERR;
	}
	//enable dma channel
	if(binten == TRUE)
	{	
	   m_dma_control->DMA_MASKTFR = CHANNEL_UMASK(dma_ch);
	}	
	m_dma_control->DMA_CHEN = (CHANNEL_WRITE_ENABLE(dma_ch)|CHANNEL_ENABLE(dma_ch));
	//printf("1m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);
	
	SSI_STD_DMA_Trig(SSIx, PAGE_PROG_CMD, addr, DMACR_TDMAE);

	if(binten == TRUE)
	{
		while(1)
		{
			if(dma_int == CHANNEL_STAT(dma_ch) )
			{
				dma_int = 0;
				break;
			}
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
		}
	}
	else
	{
		//delay(0x1000);
		while((m_dma_control->DMA_RAWTFR & CHANNEL_STAT(dma_ch)) != CHANNEL_STAT(dma_ch))
        {
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }
        //printf("2m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);
		
		m_dma_control->DMA_CLRTFR =  m_dma_control->DMA_STATTFR;
		//printf("3m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);

	}
	
	
	while(m_dma_control->DMA_CHEN & CHANNEL_STAT(dma_ch))
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }
	m_dma_control->DMA_CHEN = 0;
	m_dma_control->DMA_CONFIG = 0;
	SSIx->SSI_DMACR = 0;

    return STATUS_OK;

}


/*******************************************************************************
* Function Name  : SSI_STD_DMA_Read
* Description    : spi dma接收函数
* Input          : - SPIx: SPI 基地址
*                - dma_ch: DMA channel
*				          - pread: 发送数据地址
*                  - addr: spi flash地址
*				        - length ： 传输数据长度
*				        - binten : 是否开启中断模式
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_STD_DMA_Read(SSI_TypeDef *SSIx,int dma_ch, UINT8* pread, UINT32 addr, UINT32 length, BOOL binten)
{
    int num;
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

    m_dma_control->DMA_CONFIG = 1;
    //Rx
    m_dma_channel[dma_ch]->DMA_SADDR = (UINT32)&SSIx->SSI_DR;
    m_dma_channel[dma_ch]->DMA_DADDR = (UINT32)pread;
    if(binten == TRUE)
    {
        m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
    }
    else
    {
        m_dma_channel[dma_ch]->DMA_CTRL = DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
    }
    m_dma_channel[dma_ch]->DMA_CTRL_HIGH = (length + 4);  //最大长度为0x0FFF
    m_dma_channel[dma_ch]->DMA_CFG = (HS_SEL_SRC_HARD);
    if(SSIx == SSI1)
    {
        m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(2);
    }
    else if(SSIx == SSI2)
    {
        m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(5);
    }
    else
    {
        printf("unknown SSIx\r\n");
        return STATUS_ID_ERR;
    }
    //enable dma channel
    if(binten == TRUE)
    {	
        m_dma_control->DMA_MASKTFR = CHANNEL_UMASK(dma_ch);
    }	
    m_dma_control->DMA_CHEN = (CHANNEL_WRITE_ENABLE(dma_ch)|CHANNEL_ENABLE(dma_ch));
    //printf("1m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);

    num=length;
    SSI_STD_DMA_Trig(SSIx, READ_CMD, addr, DMACR_RDMAE);  
    while(num)
    {	
        if( (SSIx->SSI_SR&SR_TFNF)&&(num>0) )
        {	
            SSIx->SSI_DR=DUMMY_BYTE;
            num--;	
            while(0);
        }
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }		 
    while(SSIx->SSI_SR&SR_BUSY)
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }
    if(TRUE == binten)
    {
        while(1)
        {
            if(dma_int == CHANNEL_STAT(dma_ch) )
            {
                dma_int = 0;
                break;
            }
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }			

    }	 
    else
    {			
        while((m_dma_control->DMA_RAWTFR & CHANNEL_STAT(dma_ch)) != CHANNEL_STAT(dma_ch))
        {
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }    		
        m_dma_control->DMA_CLRTFR =  m_dma_control->DMA_STATTFR;
    }	 

    while(m_dma_control->DMA_CHEN & CHANNEL_STAT(dma_ch))
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }
    m_dma_control->DMA_CHEN = 0;
    m_dma_control->DMA_CONFIG = 0;
    SSIx->SSI_DMACR = 0;

    return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_QUAD_DMA_Trig
* Description    : 启动SSI QUAD DMA  
* Input          : - SSIx   : SSI 基地址
*								 ：- cmd    ：操作指令
*								 ：- addr   ：地址
*								 ：- dmaConf：传输方式
* Output         : None
* Return         : None
******************************************************************************/
void SSI_DUAL_DMA_Trig(SSI_TypeDef *SSIx, unsigned cmd, unsigned int addr, int dmaConf)
{
	SSIx->SSI_DR=cmd;
	SSIx->SSI_DR=addr;	
    SSIx->SSI_DMACR=dmaConf;
}	


/*******************************************************************************
* Function Name  : SSI_DUAL_DMA_Read
* Description    : spi dma接收函数
* Input          : - SPIx: SPI 基地址
*                - dma_ch: DMA channel
*				          - pread: 发送数据地址
*                  - addr: spi flash地址
*				        - length ： 传输数据长度
*				        - binten : 是否开启中断模式
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_DUAL_DMA_Read(SSI_TypeDef *SSIx,int dma_ch, UINT8* pread, UINT32 addr, UINT32 length, BOOL binten)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000); 

	m_dma_control->DMA_CONFIG = 1;
	//Rx
	m_dma_channel[dma_ch]->DMA_SADDR = (UINT32)&SSIx->SSI_DR;
	m_dma_channel[dma_ch]->DMA_DADDR = (UINT32)pread;
	if(binten == TRUE)
	{
		m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
	}
	else
	{
		m_dma_channel[dma_ch]->DMA_CTRL = DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
	}
	m_dma_channel[dma_ch]->DMA_CTRL_HIGH = length ;  //最大长度为0x0FFF
	m_dma_channel[dma_ch]->DMA_CFG = (HS_SEL_SRC_HARD);
	if(SSIx == SSI1)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(2);
	}
	else if(SSIx == SSI2)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(5);
	}
	else
	{
		printf("unknown SSIx\r\n");
        return STATUS_ID_ERR;
	}
	//enable dma channel
	if(binten == TRUE)
	{	
	   m_dma_control->DMA_MASKTFR = CHANNEL_UMASK(dma_ch);
	}	
	m_dma_control->DMA_CHEN = CHANNEL_WRITE_ENABLE(dma_ch)|CHANNEL_ENABLE(dma_ch);
	//printf("1m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);
	
	SSI_DUAL_DMA_Trig(SSIx, DUAL_READ_CMD, addr, DMACR_RDMAE);
	
	if(binten == TRUE)
	{
        while(1)
        {
            if(dma_int == CHANNEL_STAT(dma_ch) )
            {
                dma_int = 0;
                break;
            }
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }
	}	 
	else
	{			
		while((m_dma_control->DMA_RAWTFR & CHANNEL_STAT(dma_ch)) != CHANNEL_STAT(dma_ch))
        {
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }  		
		m_dma_control->DMA_CLRTFR =  m_dma_control->DMA_STATTFR;
	}	 

	while(m_dma_control->DMA_CHEN & CHANNEL_STAT(dma_ch))
	{
	   if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
	}	
	m_dma_control->DMA_CHEN = 0;
	m_dma_control->DMA_CONFIG = 0;
	SSIx->SSI_DMACR = 0;

    return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_DUAL_DMA_Trig
* Description    : 启动SSI QUAD DMA  
* Input          : - SSIx   : SSI 基地址
*								 ：- cmd    ：操作指令
*								 ：- addr   ：地址
*								 ：- dmaConf：传输方式
* Output         : None
* Return         : None
******************************************************************************/
void SSI_QUAD_DMA_Trig(SSI_TypeDef *SSIx, unsigned cmd, unsigned int addr, int dmaConf)
{
	SSIx->SSI_DR=cmd;
	SSIx->SSI_DR=addr;	
	SSIx->SSI_DMATDLR = 0x02; // 控制DMA FIFO发送水平, 解决 QUAD DMA通信异常问题.
    SSIx->SSI_DMACR=dmaConf; 
}	


/*******************************************************************************
* Function Name  : SSI_QUAD_DMA_Send
* Description    : spi dma传送函数
* Input          : - SPIx: SPI 基地址
*                - dma_ch: DMA channel
*				          - psend: 发送数据地址
*                  _ addr: SPI flash地址
*				         - length： 传输数据长度
*				         - binten: 是否开启中断模式
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QUAD_DMA_Send(SSI_TypeDef *SSIx,int dma_ch, UINT8* psend, UINT32 addr, UINT32 length, BOOL binten)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

    m_dma_control->DMA_CONFIG = 1;
    //Tx
    m_dma_channel[dma_ch]->DMA_SADDR = (UINT32)psend;
    m_dma_channel[dma_ch]->DMA_DADDR = (UINT32)&(SSIx->SSI_DR);

    if(binten == TRUE)
    {
        m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B;
        //m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B|SBSIZE_4|DBSIZE_4;
    }
    else
    {
        m_dma_channel[dma_ch]->DMA_CTRL = DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B;
    }
    m_dma_channel[dma_ch]->DMA_CTRL_HIGH = length ;  //最大长度为0x0FFF
    m_dma_channel[dma_ch]->DMA_CFG = (HS_SEL_DST_HARD);
    if(SSIx == SSI1)
    {
        m_dma_channel[dma_ch]->DMA_CFG_HIGH = DST_PER_SPI_TX(3);
    }
    else if(SSIx == SSI2)
    {
        m_dma_channel[dma_ch]->DMA_CFG_HIGH = DST_PER_SPI_TX(2);
    }
    else
    {
        printf("unknown SSIx\r\n");
        return STATUS_ID_ERR;	
    }
    //enable dma channel
    if(binten == TRUE)
    {	
        m_dma_control->DMA_MASKTFR = CHANNEL_UMASK(dma_ch);
    }	

    m_dma_control->DMA_CHEN = (CHANNEL_WRITE_ENABLE(dma_ch)|CHANNEL_ENABLE(dma_ch));
    //printf("1m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);

    SSI_QUAD_DMA_Trig(SSIx, QUAD_PROG_CMD, addr, DMACR_TDMAE);
    if(binten == TRUE)
    {
        while(1)
        {
            if(dma_int == CHANNEL_STAT(dma_ch) )
            {
                dma_int = 0;
                break;
            }
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }
    }
    else
    {
        delay(0x1000);
        while((m_dma_control->DMA_RAWTFR & CHANNEL_STAT(dma_ch)) != CHANNEL_STAT(dma_ch))
        {
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }
        //printf("2m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);

        m_dma_control->DMA_CLRTFR =  m_dma_control->DMA_STATTFR;
        //printf("3m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);

    }

    while(m_dma_control->DMA_CHEN & CHANNEL_STAT(dma_ch))
    {
        if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
    }
    m_dma_control->DMA_CHEN = 0;
    m_dma_control->DMA_CONFIG = 0;	
    SSIx->SSI_DMACR = 0;

    return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_QUAD_DMA_Read
* Description    : spi dma接收函数
* Input          : - SPIx: SPI 基地址
*                - dma_ch: DMA channel
*				          - pread: 发送数据地址
*                  - addr: spi flash地址
*				        - length ： 传输数据长度
*				        - binten : 是否开启中断模式
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QUAD_DMA_Read(SSI_TypeDef *SSIx,int dma_ch, UINT8* pread, UINT32 addr, UINT32 length, BOOL binten)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

	m_dma_control->DMA_CONFIG = 1;
	//Rx
	m_dma_channel[dma_ch]->DMA_SADDR = (UINT32)&SSIx->SSI_DR;
	m_dma_channel[dma_ch]->DMA_DADDR = (UINT32)pread;
	if(binten == TRUE)
	{
		m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
	}
	else
	{
		m_dma_channel[dma_ch]->DMA_CTRL = DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
	}
	m_dma_channel[dma_ch]->DMA_CTRL_HIGH = length ;  //最大长度为0x0FFF
	m_dma_channel[dma_ch]->DMA_CFG = (HS_SEL_SRC_HARD);
	if(SSIx == SSI1)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(2);
	}
	else if(SSIx == SSI2)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(5);
	}
	else
	{
		printf("unknown SSIx\r\n");
        return STATUS_ID_ERR;	
	}
	//enable dma channel
	if(binten == TRUE)
	{	
	   m_dma_control->DMA_MASKTFR = CHANNEL_UMASK(dma_ch);
	}	
	m_dma_control->DMA_CHEN = CHANNEL_WRITE_ENABLE(dma_ch)|CHANNEL_ENABLE(dma_ch);
	//printf("1m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);
	
	SSI_QUAD_DMA_Trig(SSIx, QUAD_READ_CMD, addr, DMACR_RDMAE);
	
	if(binten == TRUE)
	{
        while(1)
        {
            if(dma_int == CHANNEL_STAT(dma_ch) )
            {
                dma_int = 0;
                break;
            }
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }
    }	 
    else
    {			
        while((m_dma_control->DMA_RAWTFR & CHANNEL_STAT(dma_ch)) != CHANNEL_STAT(dma_ch))
        {
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;	
        }   		
        m_dma_control->DMA_CLRTFR =  m_dma_control->DMA_STATTFR;
    }	 

    while(m_dma_control->DMA_CHEN & CHANNEL_STAT(dma_ch))
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;	
    }	
    m_dma_control->DMA_CHEN = 0;
    m_dma_control->DMA_CONFIG = 0;
    SSIx->SSI_DMACR = 0;

    return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_QPI_DMA_Trig
* Description    : 启动SSI QUAD DMA  
* Input          : - SSIx   : SSI 基地址
*								 ：- cmd    ：操作指令
*								 ：- addr   ：地址
*								 ：- dmaConf：传输方式
* Output         : None
* Return         : None
******************************************************************************/
void SSI_QPI_DMA_Trig(SSI_TypeDef *SSIx, unsigned cmd, unsigned int addr, int dmaConf)
{
	SSIx->SSI_DR=cmd;
	SSIx->SSI_DR=addr;	
    SSIx->SSI_DMACR=dmaConf;
}	


/*******************************************************************************
* Function Name  : SSI_QPI_DMA_Send
* Description    : spi dma传送函数
* Input          : - SPIx: SPI 基地址
*                - dma_ch: DMA channel
*				          - psend: 发送数据地址
*                  _ addr: SPI flash地址
*				         - length： 传输数据长度
*				         - binten: 是否开启中断模式
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QPI_DMA_Send(SSI_TypeDef *SSIx,int dma_ch, UINT8* psend, UINT32 addr, UINT32 length, BOOL binten)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

    m_dma_control->DMA_CONFIG = 1;
    //Tx
    m_dma_channel[dma_ch]->DMA_SADDR = (UINT32)psend;
    m_dma_channel[dma_ch]->DMA_DADDR = (UINT32)&(SSIx->SSI_DR);

    if(binten == TRUE)
    {
        m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B;
        //m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B|SBSIZE_4|DBSIZE_4;
    }
    else
    {
        m_dma_channel[dma_ch]->DMA_CTRL = DNOCHG|SIEC|M2P_DMA|DWIDTH_B|SWIDTH_B;
    }
    m_dma_channel[dma_ch]->DMA_CTRL_HIGH = length ;  //最大长度为0x0FFF
    m_dma_channel[dma_ch]->DMA_CFG = (HS_SEL_DST_HARD);
    if(SSIx == SSI1)
    {
        m_dma_channel[dma_ch]->DMA_CFG_HIGH = DST_PER_SPI_TX(3);
    }
    else if(SSIx == SSI2)
    {
        m_dma_channel[dma_ch]->DMA_CFG_HIGH = DST_PER_SPI_TX(2);
    }
    else
    {
        printf("unknown SSIx\r\n");
        return STATUS_ID_ERR;	
    }
    //enable dma channel
    if(binten == TRUE)
    {	
            m_dma_control->DMA_MASKTFR = CHANNEL_UMASK(dma_ch);
    }	
    
    m_dma_control->DMA_CHEN = (CHANNEL_WRITE_ENABLE(dma_ch)|CHANNEL_ENABLE(dma_ch));
    //printf("1m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);
    
    SSI_QPI_DMA_Trig(SSIx, PAGE_PROG_CMD, addr, DMACR_TDMAE);
    if(binten == TRUE)
    {
        while(1)
        {
            if(dma_int == CHANNEL_STAT(dma_ch) )
            {
                dma_int = 0;
                break;
            }
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
    }
    else
    {
        delay(0x1000);
        while((m_dma_control->DMA_RAWTFR & CHANNEL_STAT(dma_ch)) != CHANNEL_STAT(dma_ch))
        {
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }
        //printf("2m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);
        
        m_dma_control->DMA_CLRTFR =  m_dma_control->DMA_STATTFR;
        //printf("3m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);

    }

    while(m_dma_control->DMA_CHEN & CHANNEL_STAT(dma_ch))
    {
        if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
    }
    m_dma_control->DMA_CHEN = 0;
    m_dma_control->DMA_CONFIG = 0;	
    SSIx->SSI_DMACR = 0;

    return STATUS_OK;
}


/*******************************************************************************
* Function Name  : SSI_QPI_DMA_Read
* Description    : spi dma接收函数
* Input          : - SPIx: SPI 基地址
*                - dma_ch: DMA channel
*				          - pread: 发送数据地址
*                  - addr: spi flash地址
*				        - length ： 传输数据长度
*				        - binten : 是否开启中断模式
*
* Output         : None
* Return         : STATUS
******************************************************************************/
UINT16 SSI_QPI_DMA_Read(SSI_TypeDef *SSIx,int dma_ch, UINT8* pread, UINT32 addr, UINT32 length, BOOL binten)
{
    UINT32 timeout = SSI_TIMEOUT_COUNTERS(1000);

	m_dma_control->DMA_CONFIG = 1;
	//Rx
	m_dma_channel[dma_ch]->DMA_SADDR = (UINT32)&SSIx->SSI_DR;
	m_dma_channel[dma_ch]->DMA_DADDR = (UINT32)pread;
	if(binten == TRUE)
	{
		m_dma_channel[dma_ch]->DMA_CTRL = INTEN|DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
	}
	else
	{
		m_dma_channel[dma_ch]->DMA_CTRL = DIEC|SNOCHG|P2M_DMA|DWIDTH_B|SWIDTH_B;
	}
	m_dma_channel[dma_ch]->DMA_CTRL_HIGH = length ;  //最大长度为0x0FFF
	m_dma_channel[dma_ch]->DMA_CFG = (HS_SEL_SRC_HARD);
	if(SSIx == SSI1)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(2);
	}
	else if(SSIx == SSI2)
	{
		m_dma_channel[dma_ch]->DMA_CFG_HIGH = SRC_PER_SPI_RX(5);
	}
	else
	{
		printf("unknown SSIx\r\n");
        return STATUS_ID_ERR;	
	}
	//enable dma channel
	if(binten == TRUE)
	{	
	   m_dma_control->DMA_MASKTFR = CHANNEL_UMASK(dma_ch);
	}	
	m_dma_control->DMA_CHEN = CHANNEL_WRITE_ENABLE(dma_ch)|CHANNEL_ENABLE(dma_ch);
	//printf("1m_dma_control->DMA_RAWTFR: 0x%08x\r\n", m_dma_control->DMA_RAWTFR);
	
	SSI_QPI_DMA_Trig(SSIx, QPI_READ_CMD, addr, DMACR_RDMAE);
	
	if(binten == TRUE)
	{
			while(1)
			{
				if(dma_int == CHANNEL_STAT(dma_ch) )
				{
					dma_int = 0;
					break;
				}
                if(0 == timeout)
                {
                    return STATUS_TIMEOUT;
                }
                timeout --;
			}
	}	 
	else
	{			
		while((m_dma_control->DMA_RAWTFR & CHANNEL_STAT(dma_ch)) != CHANNEL_STAT(dma_ch))
        {
            if(0 == timeout)
            {
                return STATUS_TIMEOUT;
            }
            timeout --;
        }  		
		m_dma_control->DMA_CLRTFR =  m_dma_control->DMA_STATTFR;
	}	 

	while(m_dma_control->DMA_CHEN & CHANNEL_STAT(dma_ch))
	{
	   if(0 == timeout)
        {
            return STATUS_TIMEOUT;
        }
        timeout --;
	}	
	m_dma_control->DMA_CHEN = 0;
	m_dma_control->DMA_CONFIG = 0;
	SSIx->SSI_DMACR = 0;

    return STATUS_OK;
}


