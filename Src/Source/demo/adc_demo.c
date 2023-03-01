// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : adc_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "type.h"
#include "debug.h"
#include "adc_drv.h"
#include "adc_demo.h"
#include <string.h>
#include "delay.h"

#define ADC_CH0_TEST
//#define ADC_CH1_TEST
//#define ADC_CH2_TEST
//#define ADC_VBAT_TEST

void ADC_Demo(void)
{
	long res = 0;
	UINT32 resVC = 0;
	INT32 i = 0;
	
	res = res;

#ifdef ADC_CH0_TEST
	printf("******************************ADC CH0 test for 64 cyclic*************************************\r\n");
	for( i=0;i<64;i++){
	//for( i=0;i<64;i){
		res = ADC_GetConversionValue(ADCCH_0);
		printf("ADC CH0 Conversion Value is %ld;\r\n",res);
		resVC = 33*res/4095;//(33/4095)*res;
		printf("ADC CH0 calculate VC manually is %d.%dV;\r\n",resVC/10,resVC%10);
		DelayMS(1000);
	}
#endif

#ifdef ADC_CH1_TEST
	printf("******************************ADC CH1 test for 64 cyclic*************************************\r\n");
	for( i=0;i<64;i++){
	//for( i=0;i<64;){
		res = ADC_GetConversionValue(ADCCH_1);
		printf("ADC CH1 Conversion Value is %ld;\r\n",res);
		resVC = 33*res/4095;
		printf("ADC CH1 calculate VC manually is %d.%dV;\r\n",resVC/10,resVC%10);
		DelayMS(1000);
	}
#endif

#ifdef ADC_CH2_TEST
	printf("******************************ADC CH2 test for 64 cyclic*************************************\r\n");
	for( i=0;i<64;i++){
	//for( i=0;i<64;){
		res = ADC_GetConversionValue(ADCCH_2);
		printf("ADC CH2 Conversion Value is %ld;\r\n",res);
		resVC = 33*res/4095;
		printf("ADC CH2 calculate VC manually is %d.%dV;\r\n",resVC/10,resVC%10);
		DelayMS(1000);
	}
#endif
	
#ifdef ADC_VBAT_TEST
	printf("******************************ADC VBAT test for 64 cyclic*************************************\r\n");
	for( i=0;i<64;i++){
//	for( i=0;i<64;){
		resVC = ADC_Get_Battery(ADCCH_0);
		printf("ADC VBAT calculate VC manually is %d.%dV;\r\n",resVC/100,resVC%100);
		DelayMS(1000);
	}
#endif

}


