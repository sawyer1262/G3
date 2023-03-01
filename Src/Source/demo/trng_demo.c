// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : trng_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "type.h"
#include "debug.h"
#include "trng_drv.h"

void Trng_Demo(void)
{
	UINT32 random_data[32];
	UINT32 i;
	
	TRNG_Init();
	
	Get_RamdomData(random_data, 32);
	
	for(i=0; i<32; i++)
	{
		printf("%x", random_data[i]);
	}
}
