#include "debug.h"
#include "get_sn_drv.h"
#include "get_sn_demo.h"

void GetSN_Demo(void)
{
	unsigned char databuf[8];
	int i=0;

	get_serial_number(databuf);

	printf("sn: ");
	for(i=0;i<8;i++)
	{
		printf("0x%02x ",databuf[i]);
	}
	while(1);
}

