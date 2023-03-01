// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : auduo_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "audio_demo.h"

void callback_print(uint32_t number)
{
	printf("Play Done\n");
	AudioServicePlay(TEST_AUDIO_NUMBER,BREAK);
}

void Audio_Demo(void)
{
    SysTick_Config(g_sys_clk/1000);  //1ms
	SysTick_Enable(TRUE);
	
	AudioServiceInit(callback_print);
	
	AudioServicePlay(TEST_AUDIO_NUMBER,UNBREAK);
	//AudioServicePlay(TEST_AUDIO_NUMBER,UNBREAK);
		
	while(1)
	{
		//printf("run\r\n");
		AudioServiceRouting();
	}
}

