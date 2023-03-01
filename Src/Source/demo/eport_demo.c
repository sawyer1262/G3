#include "type.h"
#include "eport_demo.h"
#include "eport_drv.h"
#include "sys.h"
#include "delay.h"

UINT8 test_case=0;

void Eport_Demo(void)
{
//	EPORT_TypeDef *eport = EPORT;
//	EPORT_TypeDef *eport = EPORT1; //EPORT1_PIN0~7->GINT8~15
	
	EPORT_PINx eport_pin = EPORT_PIN0;

	test_case = gpio_output_test;
//	test_case = low_level_test;			//每次只开启一个引脚来测试
//	test_case = high_level_test;			//每次只开启一个引脚来测试
//	test_case = raise_edge_test;
//	test_case = fall_edge_test;

	switch(test_case)
	{
		case gpio_output_test:
			EPORT_ConfigGpio(eport_pin, GPIO_OUTPUT);
			break;

		case low_level_test:
			EPORT_Init(eport_pin, LOW_LEVEL_INT);
			break;

		case high_level_test:
			EPORT_Init(eport_pin, HIGH_LEVEL_INT);
			break;

		case raise_edge_test:
			EPORT_Init(eport_pin, RISING_EDGE_INT);
			break;
		
		case fall_edge_test:
			EPORT_Init(eport_pin, FALLING_EDGE_INT);
			break;
	}

    while(1)
    {
        if(test_case == gpio_output_test)
        {
            EPORT_WriteGpioData(eport_pin, Bit_SET);
            DelayMS(500);
            EPORT_WriteGpioData(eport_pin, Bit_RESET);
            DelayMS(500);
        }
    }
}



