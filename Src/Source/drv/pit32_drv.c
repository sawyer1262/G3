// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : pit32_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "debug.h"
#include "pit32_drv.h"
#include "main.h"

#ifdef BCTC_DEMO
#include "iccemv.h"
#endif

#if PIT32_ADDR == PIT1_BASE_ADDR
	#define PIT_IRQn   PIT1_IRQn
#else
	#define PIT_IRQn   PIT2_IRQn
#endif

/*******************************************************************************
* Function Name  : PIT32_ISR
* Description    : PIT32中断处理
* Input          : - PITx 指向PIT寄存器的指针
*
* Output         : None
* Return         : None
******************************************************************************/
static void PIT32_ISR(PIT32_TypeDef *PITx)
{
	UINT32 tmp1 = PITx->PCSR;

	if (tmp1 & PCSR_PIF)//pit
	{
		PITx->PCSR |= PIT_PIF;
/*************************************************************/
		//以下为7816主模式下需要的时间中断处理代码
#ifdef USI_PIT
		if((k_IcMax_TotalTimeCount>0)&&(k_total_timeover==0))
		{
			  k_Ic_TotalTimeCount++;
			 if(k_Ic_TotalTimeCount>k_IcMax_TotalTimeCount)
			 {
				k_total_timeover=1;
			 }
			 if(k_total_timeover==1)
			 {
			   k_Ic_TotalTimeCount=0;
			   PIT32_Stop(PITx);
			 }
			 else
			 {

			 }
		 }	
#endif
	/***************************************************************/
	}
}

/*******************************************************************************
* Function Name  : PIT1_IRQHandler
* Description    : PIT1中断处理
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT1_IRQHandler(void)
{
		Timer_IntCallback();
 //   printf("enter pit1 interrupt.\r\n"); 
		PIT32_ISR(PIT1);
}

/*******************************************************************************
* Function Name  : PIT2_IRQHandler
* Description    : PIT2中断处理
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT2_IRQHandler(void)
{
    printf("enter pit2 interrupt.\r\n");
	PIT32_ISR(PIT2);
}

/*******************************************************************************
* Function Name  : PIT32_Init
* Description    : PIT初始化
* Input          : - PITx 指向PIT寄存器的指针
*                  - Prescaler：PIT时钟分频值
*                  - PMRReloadCounterVal:PIT计数器的起始值
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT32_Init(PIT32_TypeDef *PITx, PIT32_CLKDIV Prescaler, UINT32 PMRReloadCounterVal,BOOL pitINT)
{
	//ModuleClk_On(MODULE_CLK_PIT1_IPG_CLK);
	if(pitINT == TRUE)
	{
        if(PITx == PIT1)
        {
            NVIC_Init(3, 3, PIT1_IRQn, 2);
        }
        else if(PITx == PIT2)
        {
            NVIC_Init(3, 3, PIT2_IRQn, 2);
        }
        else
        {}	
	}
	PITx->PCSR &= (~PCSR_EN);
	if(pitINT == TRUE)
	{
		PITx->PCSR = (Prescaler<<8)|PCSR_OVW|PCSR_PIE|PCSR_RLD|PCSR_PDBG; //中断模式
	}
	else
	{
		PITx->PCSR = (Prescaler<<8)|PCSR_OVW|PCSR_RLD;    //普通模式
	}


	PITx->PMR = PMRReloadCounterVal;
	PITx->PCSR |= PCSR_EN;
}

/*******************************************************************************
* Function Name  : PIT32_Start
* Description    : PIT启动
* Input          : - PITx 指向PIT寄存器的指针
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT32_Start(PIT32_TypeDef *PITx)
{
	PITx->PCSR |= PCSR_EN;
}

/*******************************************************************************
* Function Name  : PIT32_Stop
* Description    : PIT关闭
* Input          : - PITx 指向PIT寄存器的指针
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT32_Stop(PIT32_TypeDef *PITx)
{
	PITx->PCSR &= ~PCSR_EN;
}


/*******************************************************************************
* Function Name  : PIT32_Cmd
* Description    : PIT使能控制
* Input          : - PITx 指向PIT寄存器的指针
*                  - NewState：new state of the specified PIT.
*                              This parameter can be: ENABLE or DISABLE.
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT32_Cmd(PIT32_TypeDef *PITx, FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		/* Enable the PIT */
		PITx->PCSR |= PCSR_EN;
	}
	else
	{
		/* Disable the PIT */
		PITx->PCSR &= ~PCSR_EN;
	}
}

/*******************************************************************************
* Function Name  : PIT32_CmdPIT_ITConfig
* Description    : PIT中断使能控制
* Input          : - PITx 指向PIT寄存器的指针
*                  - NewState：new state of the specified PIT interrupts.
*                              This parameter can be: ENABLE or DISABLE.
*
* Output         : None
* Return         : None
******************************************************************************/
void PIT32_ITConfig(PIT32_TypeDef *PITx, FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		/* Enable the Interrupt source		s */
		PITx->PCSR |= PCSR_PIE;
	}
	else
	{
		/* Disable the Interrupt sources */
		PITx->PCSR &= (UINT16)~PCSR_PIE;
	}
}

/*******************************************************************************
* Function Name  : PIT32_ReadCNTR
* Description    : 读取PIT计数值
* Input          : - PITx 指向PIT寄存器的指针
*
* Output         : 计数值
* Return         : 计数值
******************************************************************************/
UINT32 PIT32_ReadCNTR(PIT32_TypeDef *PITx)
{
	return PITx->PCNTR;
}
