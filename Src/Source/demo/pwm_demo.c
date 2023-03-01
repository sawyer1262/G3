// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : pwm_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "pwm_drv.h"
#include "pwm_demo.h"
#include "type.h"
#include "debug.h"
#include "delay.h"
#include "ccm_drv.h"
#define PWM_OUTPUT

#ifdef PWM_OUTPUT

static void pwm_output_test(void);
//pwm测试
static void pwm_output_test(void)
{
	PWM_OutputInit(PWM_PORT0,59,PWM_CLK_DIV_1,65535,10000,0);		//GINT[12]
	PWM_OutputInit(PWM_PORT1,59,PWM_CLK_DIV_1,65535,20000,0);		//GINT[13]
//	PWM_OutputInit(PWM_PORT2,59,PWM_CLK_DIV_1,65535,30000,0);		//GINT[14]/TDO	//打开PWM功能，将不能仿真
//	PWM_OutputInit(PWM_PORT3,59,PWM_CLK_DIV_1,65535,40000,0);		//GINT[15]/TCKL	//打开PWM功能，将不能仿真

	printf("pwm_output_test\n");

	while(1)
	{
		;
	}
}

#else

static void pwm_input_test(void);

static void pwm_input_test(void)
{
	PWM_InputInit(PWM_PORT0, 1,PWM_CLK_DIV_1,65535);
	PWM_InputInit(PWM_PORT1, 1,PWM_CLK_DIV_1,65535);
	PWM_InputInit(PWM_PORT2, 59,PWM_CLK_DIV_1,99);
	PWM_InputInit(PWM_PORT3, 59,PWM_CLK_DIV_1,99);
	while(1)
	{
		;
		//PWM中断触发
	}
}

#endif

void PWM_Demo(void)
{
	DelayMS(2000);	//调试时建议加上延时，预防上电修改仿真接口，导致无法仿真
	CCM->PCFG3|=0x01;
	
#ifdef PWM_OUTPUT
	PWM_Output_IntEnable(PWM_PORT3);
	PWM_Output_IntEnable(PWM_PORT2);
	pwm_output_test();
#else
	pwm_input_test();
#endif
}
