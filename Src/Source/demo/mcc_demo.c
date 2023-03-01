/*
 * mcc_demo.c
 */

#include <stddef.h>
#include <string.h>
#include "uart_drv.h"
#include "mcc_drv.h"
#include "stdlib.h"
#include "type.h"
#include "sys.h"
#include "debug.h"

//#define MCC_GET_MID_DATA
//#define MCC_OUT_MID_DATA

#define TRACE_INFO	printf

#define MCC_EVB_BOARD

void mcc_adc_track_output(s8 *buf, s32 len, s32 track_num, s32 ret)
{
    //printf("TRACK%d(%d-%d): %s\n", track_num, ret, len, buf);
    printf("TRACK%d: %s\r\n", track_num, buf);
}

void mcc_adc_data_output(u16 *p_buf, s32 cnt)
{
    s32 i;

    for(i = 0; i < cnt; i++)
    {
        printf("%d, ", p_buf[i]);
        if((i + 1) % 32 == 0)
            printf("\n");
    }
    if(i % 32 != 0) printf("\n");
}

void mcc_raw_data_output_t2(void);
void mcc_raw_data_output_t3(void);

s8 msg[36];
#ifdef MCC_EVB_BOARD
s8 track1_buf[96];
#endif
s8 track2_buf[64];
s8 track3_buf[128];



//min -25%
//max +25%
#if 1//(ADC_SMPR_INIT == 6)
//QPR=15, STCNT=32, SMP=6
#define Min_Sampling_Number_T1_T3  3    //5.6，@150cm/s  4
#define Max_Sampling_Number_T1_T3  600  //213, @4cm/s 266
#define Min_Sampling_Number_T2     5    //15.6，@150cm/s 11
#define Max_Sampling_Number_T2     1200	//800  //589, @4cm/s
#elif (ADC_SMPR_INIT == 8)
//QPR=15, STCNT=32, SMP=8
#define Min_Sampling_Number_T1_T3  3    //5.1，@150cm/s
#define Max_Sampling_Number_T1_T3  239  //192, @4cm/s
#define Min_Sampling_Number_T2     10   //14.1，@150cm/s
#define Max_Sampling_Number_T2     662  //530, @4cm/s
#elif (ADC_SMPR_INIT == 10)
//QPR=15, STCNT=32, SMP=10
#define Min_Sampling_Number_T1_T3  3    //4.6，@150cm/s
#define Max_Sampling_Number_T1_T3  218  //174, @4cm/s
#define Min_Sampling_Number_T2     9   //12.8，@150cm/s
#define Max_Sampling_Number_T2     602  //482, @4cm/s
#endif

MCC_VAR_S mv;

#define MCC_DATA_BUF_SIZE	(512)
MCC_TRACK_PT adc_buff[MCC_DATA_BUF_SIZE];

void mcc_adc_hw_init(void)
{
#ifdef MCC_EVB_BOARD
    mcc_adc_hardware_init(adc_buff, MCC_DATA_BUF_SIZE, 0x02, 0x03, 0x10);
#else
    mcc_adc_hardware_init(adc_buff, MCC_DATA_BUF_SIZE, 0x02, 0x03, 0x10);
#endif
}


void MCC_DAC_Demo(void)
{
    s32 track1_len;
    u8 track1_ret = 9;
    s32 track1_succ_cnt;
    s32 track1_succ_rate;

    s32 track2_len;
    u8 track2_ret;
    s32 track2_succ_cnt;
    s32 track2_succ_rate;

    s32 track3_len;
    u8 track3_ret;
    s32 track3_succ_cnt;
    s32 track3_succ_rate;

    s32 cnt_all;
    s32 cnt_null;
    s8 show_l;

    u32 t1_sens = 19;			//参考值：0x15~0x40
    u32 t2_sens = 25;			//参考值：0x15~0x40
    u32 t3_sens = 20;			//参考值：0x15~0x40

	  char ch_in;
	  int flg_mcc_start;
  	int flg_prt;
	  int flg_restart;

	  flg_mcc_start = 1;
		flg_restart = flg_restart;
	  flg_restart = 0;
		ch_in = ch_in;
	  ch_in = 0;

#ifdef MCC_EVB_BOARD
    MccInit((1<<17) | (1<<18) | (1<<16));
#else
    MccInit((1<<17) | (1<<18));	//track2, track3 enable
#endif

	  NVIC_Init(3, 3, DMA1_IRQn, 2);
    TRACE_INFO("MCC_DAC_Demo\r\n");

	while(1)	//demo loop
	{
		if(flg_mcc_start)
		{
		    
			  track1_succ_cnt = 0;
		    track1_succ_rate = 0;
		    track2_succ_cnt = 0;
		    track2_succ_rate = 0;
		    track3_succ_cnt = 0;
		    track3_succ_rate = 0;
		    cnt_all = 0;
		    cnt_null = 0;

			//hardware initialize
		    mcc_adc_hw_init();
		//	TRACE_INFO("mcc_adc_hw_init... DONE.\n");

		    //software start
		    mcc_adc_start(&mv, t1_sens, t2_sens, t3_sens,
						  Min_Sampling_Number_T1_T3, Max_Sampling_Number_T1_T3,
						  Min_Sampling_Number_T2,    Max_Sampling_Number_T2);
		//	TRACE_INFO("mcc_adc_start... DONE\n");
		    printf("[MagCard Test start] t1_sens=%d, t2_sens=%d, t3_sens=%d\r\n", t1_sens, t2_sens, t3_sens);

			while(1)
			{
				if(mcc_adc_brushed())	//check if brushed
				{
					TRACE_INFO("\n***brushed***\n");

		#ifdef MCC_EVB_BOARD
					memset(track1_buf, 0, 96);
					track1_len = 96;
					track1_ret = 9;
		#endif

					memset(track2_buf, 0, 64);
					track2_len = 64;
					track2_ret = 9;

					memset(track3_buf, 0, 128);
					track3_len = 128;
					track3_ret = 9;

		#ifdef MCC_EVB_BOARD
					track1_ret = mcc_adc_get_data_track1(track1_buf, &track1_len, NULL);
					
		#endif
					track2_ret = mcc_adc_get_data_track2(track2_buf, &track2_len, NULL);
//					printf("mcc_raw_data_output_t2\r\n");
//					mcc_raw_data_output_t2();
//					printf("\r\n");
					track3_ret = mcc_adc_get_data_track3(track3_buf, &track3_len, NULL);
//					printf("mcc_raw_data_output_t3\r\n");
//					mcc_raw_data_output_t3();
//					printf("\r\n");

					if((track1_ret == 9) && (track2_ret == 9) && (track3_ret == 9))
					{
						cnt_null ++;

		#ifdef DPRINTF_NULL_INFO
						printf("null\n");
						show_l = 1;
		#else
						show_l = 0;
		#endif
					}
					else
					{
						cnt_all ++;

		#ifdef MCC_EVB_BOARD
						if(track1_ret == 0)
							track1_succ_cnt ++;
						track1_succ_rate = track1_succ_cnt * 100 / cnt_all;
		#endif

						if(track2_ret == 0)
							track2_succ_cnt ++;
						track2_succ_rate = track2_succ_cnt * 100 / cnt_all;

						if(track3_ret == 0)
							track3_succ_cnt ++;
						track3_succ_rate = track3_succ_cnt * 100 / cnt_all;

						show_l = 2;
					}
					
					if(show_l)
					{
		#ifdef MCC_EVB_BOARD
						printf("track1_ret=%d \r\n", track1_ret );
						mcc_adc_track_output(track1_buf, track1_len, 1, track1_ret);
		#endif
						printf("track2_ret=%d \r\n", track2_ret );
						mcc_adc_track_output(track2_buf, track2_len, 2, track2_ret);
		#ifdef MCC_OUT_MID_DATA
						if(track2_ret)
						{
							mcc_raw_data_output_t2();
						}
		#endif
						printf("track3_ret=%d \r\n", track3_ret );
						mcc_adc_track_output(track3_buf, track3_len, 3, track3_ret);
		#ifdef MCC_OUT_MID_DATA
						if(track3_ret)
						{
							mcc_raw_data_output_t3();
						}
		#endif
						printf("all: %03d (null: %03d)\r\n", cnt_all, cnt_null);
		#ifdef MCC_EVB_BOARD
						printf("t1_succ: %03d (%02d%%)\r\n", track1_succ_cnt, track1_succ_rate);
		#endif
						printf("t2_succ: %03d (%02d%%)\r\n", track2_succ_cnt, track2_succ_rate);
						printf("t3_succ: %03d (%02d%%)\r\n", track3_succ_cnt, track3_succ_rate);
						printf("\r\n");
						printf("\r\n");
					}
					//software reset
					mcc_adc_restart();
				}
			}
		}
		else /* if(flg_mcc_start) */
		{
			if(flg_prt)
			{
				flg_prt = 0;
				printf("MCC_DAC_Demo terminated.\r\n");
				printf("Press key 'S' to start.\r\n");
			}
		}
	}
}
