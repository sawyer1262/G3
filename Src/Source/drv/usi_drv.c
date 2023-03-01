// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : usi_drv.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "usi_drv.h"
#include "delay.h"
#include "cpm_drv.h"
#include "debug.h"
#include "libRegOpt.h"
#include "iccemv.h"

//#define BCTC_CONTACT_TEST	//BCTC接触测试需要打开此宏
#ifdef BCTC_CONTACT_TEST
	#define DATA_EPORT	0	//定义7816脚与EPORT的连接方式
#endif

/******************************************************************************
 *  Global variables defined
 ******************************************************************************/

USI_TypeDef * g_USIReg = USI;


/*******************************************************************************
* Function Name  : USI_ConfigGpio
* Description    : USI配置成GPIO用途
* Input          : - USIx    : USI的基地址，USI1或者USI2
*                  - USI_PINx：USI对应的PIN脚，取值USI_RST、USI_CLK、USI_DAT
*                  - GPIO_Dir：设置GPIO方向   GPIO_OUTPUT：输出  GPIO_INPUT：输入
*
* Output         : None
* Return         : None
******************************************************************************/
void USI_ConfigGpio(USI_TypeDef * USIx, USI_PIN USI_PINx, UINT8 GPIO_Dir)
{
	/* Check the parameters */
	assert_param(IS_USI_PINx(USI_PINx));
	assert_param(IS_GPIO_DIR_BIT(GPIO_Dir));
    
    if(USIx == USI2)
    {
        CHIP_RESET->RCR &= ~(0x00800000);
        USIx->USIPCR = 0;
        *(uint32_t *)0x40000004 = 0x77770001; //设置IOCTRL USI 
        CPM_PWRCR_OptBits((0x03<<25),RESET);
        CPM->CPM_SLPCFGR = (CPM->CPM_SLPCFGR & ~0x0000FF00)|0x00003300;
        while((CPM->CPM_PWRSR & 0x04) != 0x04);
    }

	USIx->USIPCR |= 0xC0;	//config gpio

	if (GPIO_Dir == GPIO_OUTPUT)
	{
		USIx->USIDDR |= (1<<USI_PINx);//output
	}
	else if (GPIO_Dir == GPIO_INPUT)
	{
		USIx->USIDDR &= (~(1<<USI_PINx));//input
	}
}


/*******************************************************************************
* Function Name  : USI_ReadGpioData
* Description    : 获取USI_PINx对应引脚的电平
* Input          : - USIx    : USI的基地址，USI1或者USI2
*                  - USI_PINx：USI对应的PIN脚，取值USI_RST、USI_CLK、USI_DAT
*
* Output         : None
* Return         : Bit_SET:高电平  Bit_RESET：低电平
******************************************************************************/
UINT8 USI_ReadGpioData(USI_TypeDef * USIx, USI_PIN USI_PINx)
{
	UINT8 bitstatus = 0x00;

	/* Check the parameters */
	assert_param(IS_USI_PINx(USI_PINx));

	bitstatus = USIx->USIPDR;
	if (bitstatus &(Bit_SET<<USI_PINx))
		bitstatus = Bit_SET;
	else
		bitstatus = Bit_RESET;

	return bitstatus;
}


/*******************************************************************************
* Function Name  : USI_WriteGpioData
* Description    : 设置USI_PINx对应引脚的电平
* Input          : - USIx    : USI的基地址，USI1或者USI2
*                  - USI_PINx：SPI对应的PIN脚，取值USI_RST、USI_CLK、USI_DAT
*                  - bitVal：设置的电平，Bit_SET：设置为高电平  Bit_RESET：设置为低电平
*
* Output         : None
* Return         : None
******************************************************************************/
void USI_WriteGpioData(USI_TypeDef * USIx, USI_PIN USI_PINx, UINT8 bitVal)
{
	/* Check the parameters */
	assert_param(IS_USI_PINx(USI_PINx));

	if (bitVal == Bit_SET)
		USIx->USIPDR |= (Bit_SET<<USI_PINx);
	else
		USIx->USIPDR &= (~(Bit_SET<<USI_PINx));
}


/*******************************************************************************
* Function Name  : USI_Init
* Description    : USI初始化
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void  USI_Init(void)
{
	//CHIP_RESET->RCR &= ~CRE;
}

/*******************************************************************************
* Function Name  : TransmitByte
* Description    : 发送一个字节数据
* Input          : 被发送的字节data
*
* Output         : None
* Return         : None
******************************************************************************/
void TransmitByte(UINT8 data)
{
    while((g_USIReg->USISR & USISR_TDRE_MASK)==0);
    g_USIReg->USITDR = data;
	
    while((g_USIReg->USISR & USISR_TC_MASK)==0);		//每个字符的第11个etu结束时完成，避免检测到error后仍然发送下个字符
}

/*******************************************************************************
* Function Name  : ReceiveByte
* Description    : 接收一个字节数据
* Input          : None
*
* Output         : None
* Return         : 返回接收到的字节
******************************************************************************/
UINT8 ReceiveByte(void)
{
    UINT8 data;

    while((g_USIReg->USISR & USISR_RDRF_MASK) != USISR_RDRF_MASK);
    data = g_USIReg->USIRDR;
    return data;
}


/*******************************************************************************
* Function Name  : USI_DeInit
* Description    : USI驱动初始化
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void USI_DeInit(void)
{
	SC_RSTSET(0); 	//RST 拉低
	DelayMS(1);

//	g_USIReg->USIPDR &= 0xfd;                   //isoclk L
//	g_USIReg->USIPCR |= 0x40;	                //clk设为GPIO模式，避免CLK停止后有几十us的高电平
	g_USIReg->USICSR |= 0x40;                   //配置时钟停止后为低电平
	DelayNETU(1);
	g_USIReg->USICSR &= 0x7F;                   //disable cclk
	g_USIReg->USICR1 = g_USIReg->USICR1 & 0xef;	//关闭7816模块

	//IO、CLK、RST都输出0，否则会通过上拉电阻把VCC拉高
	g_USIReg->USIPCR = 0xC7;		            //IO/CLK配成GPIO
	g_USIReg->USIDDR = 0x07; 	                //isoclk, isorst, isodat output
	g_USIReg->USIPDR = 0x00;		            //输出0
}

/*******************************************************************************
* Function Name  : USIMasterInit
* Description    : USI主机初始化
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void USIMasterInit(void)
{
	unsigned char ret,i;

	//先配置时钟，否则CLK会拉高一段时间后输出时钟，不符合规范
	g_USIReg->USICSR = g_ips_clk/4000000-1;  //fclk = fips/(ccps+1); fclk = 4M

	g_USIReg->USICSR |= 0x80;       //enable cclk

	g_USIReg->USIPDR &= 0xfe;       //isorst L
	g_USIReg->USIDDR = 0x03;        //isoclk, isorst output

	g_USIReg->USIBDR = 0x02;        // 372 F/D
	g_USIReg->USIIER = 0x0;         //disable interrupt

	g_USIReg->USIPCR = 0x07;		//IO/CLK配成USI模式
	g_USIReg->USIPCR |= 0x20;       //isodat is open-drain
	g_USIReg->USICR1 = 0x14;		//使能7816模块
	g_USIReg->USICR1 |= 0x80;	    //打开SB位有效标志，如果不打开则SB只有在stop模式下收到起始位才会置1

	//Clear RDR
	ret = g_USIReg->USISR;
	g_USIReg->USISR = ret; //clear ATR
	ret = g_USIReg->USIRDR;

	for(i=0;i<8;i++)		// 20131226,解决执行1CF110.02后接着执行1CF110.03获取ATR失败问题，原因是110.02多发送了N个字节，仍保存在缓冲区中
	{
		//FIFO深度为8字节，so读8字节即可。或者通过USISR判断是否有数据
		ret = g_USIReg->USIRDR;
	}
}

/*******************************************************************************
* Function Name  : USIMasterHotInit
* Description    : USI主机热启动初始化
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void USIMasterHotInit(void)
{
	UINT8 ret,i;
	
	ret = ret;

	g_USIReg->USIBDR = 0x02; // 372 F/D
	g_USIReg->USIIER = 0x0; //disable interrupt

	for(i=0;i<8;i++)		// 20131226,解决执行1CF110.02后接着执行1CF110.03获取ATR失败问题，原因是110.02多发送了N个字节，仍保存在缓冲区中
	{
		//FIFO深度为8字节，so读8字节即可。或者通过USISR判断是否有数据
		ret = g_USIReg->USIRDR;
	}
}

/*******************************************************************************
* Function Name  : USI_OpenWTC
* Description    : USI打开帧等待时间
* Input          : 等待时间etu
*
* Output         : None
* Return         : None
******************************************************************************/
void USI_OpenWTC(UINT32 etu)
{
	g_USIReg->USICR1 &= ~0x02;		// WTEN关闭，同时清除Wait TimeOut Flag

	g_USIReg->USIWTRH = (UINT8)(etu>>16);	//设置等待时间
	g_USIReg->USIWTRM = (UINT8)(etu>>8);
	g_USIReg->USIWTRL = (UINT8)etu;

	g_USIReg->USICR1 |= 0x02;		// WTEN使能
}

/*******************************************************************************
* Function Name  : Card2_Power_On
* Description    : 读卡器上电
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void Card2_Power_On(void)
{
//	UINT32 temp;
	
	CPM->CPM_SLPCFGR &= ~(0x1<<15);
//	CPM->CPM_PWRCR &= ~(0x1<<26);
	CPM_PWRCR_OptBits((0x1<<26),RESET);

#if 0
	//等待card vcc输出稳定
	while((CPM->CPM_PWRSR & 0x02) != 0x02);
#else
	DelayMS(2);
	if ((CPM->CPM_PWRSR & 0x02) != 0x02)	//卡短路等情况导致没有稳定输出VCC则下电
	{
//		CPM->CPM_PWRCR |= (0x1<<26);
		CPM_PWRCR_OptBits((0x1<<26),SET);
		CPM->CPM_SLPCFGR |= (0x1<<15);		
	}
#endif	
}

/*******************************************************************************
* Function Name  : Card2_Power_Off
* Description    : 读卡器下电
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
void Card2_Power_Off(void)
{
//	CPM->CPM_PWRCR |= (0x1<<26);
	CPM_PWRCR_OptBits((0x1<<26), SET);
	CPM->CPM_SLPCFGR |= (0x1<<15);

	//等待card ldo电源释放完毕
	DelayMS(5);
}

/*******************************************************************************
* Function Name  : Card2_Power_Switch
* Description    : 电压类型切换
* Input          : mode 电压切换模式
*
* Output         : None
* Return         : None
******************************************************************************/
void Card2_Power_Switch(UINT8 mode)
{
	Card2_Power_Off();

	CPM->CPM_SLPCFGR &= 0xffffcfff;

	CPM->CPM_SLPCFGR |= (mode<<12);

	Card2_Power_On();
}

/*******************************************************************************
* Function Name  : Card3_Power_Off
* Description    : 读卡器下电
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
//void Card3_Power_Off(void)
//{
//	CPM->CPM_PWRCR |= (0x1<<25);
//	CPM->CPM_SLPCFGR |= (0x1<<11);

//	//等待card ldo电源释放完毕
//	DelayMS(5);
//}

/*******************************************************************************
* Function Name  : Card3_Power_On
* Description    : 读卡器上电
* Input          : None
*
* Output         : None
* Return         : None
******************************************************************************/
//void Card3_Power_On(void)
//{
//	CPM->CPM_SLPCFGR &= ~(0x1<<11);
//	CPM->CPM_PWRCR &= ~(0x1<<25);

//#if 0
//	//等待card vcc输出稳定
//	while((CPM->CPM_PWRSR & 0x04) != 0x04);
//#else
//	DelayMS(2);
//	if ((CPM->CPM_PWRSR & 0x04) != 0x04)	//卡短路等情况导致没有稳定输出VCC则下电
//	{
//		CPM->CPM_PWRCR |= (0x1<<25);
//		CPM->CPM_SLPCFGR |= (0x1<<11);
//	}
//#endif	
//}

/*******************************************************************************
* Function Name  : Card3_Power_Switch
* Description    : 电压类型切换
* Input          : mode 电压切换模式
*
* Output         : None
* Return         : None
******************************************************************************/
//void Card3_Power_Switch(unsigned char mode)
//{
//	Card3_Power_Off();

//	CPM->CPM_SLPCFGR &= 0xfffffcff;

//	CPM->CPM_SLPCFGR |= (mode<<8);

//	Card3_Power_On();
//}









