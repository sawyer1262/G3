/*
 * mcc_if.h
 */

#ifndef MCC_IF_H_
#define MCC_IF_H_

//#include "type_def.h"
#include "type.h"
#include "main.h"

#ifdef MCC_DEMO
#define MCC_DMA_INT
#endif

//#define MCC_BAT_DET_EN

#define TRACK1_EN
#define TRACK3_EN


#define TRACK1_P_V_LEN		((79*7)*2)//1106
#define TRACK2_P_V_LEN		((40*5)*2)//400
#define TRACK3_P_V_LEN		((107*5)*2)//1070

#define TRACK_DATA_LEN		(256)

#define MCC_SEP_BUF_SIZE	(256)

typedef struct
{
#ifdef TRACK1_EN
    u16 track1_pt;
#endif

    u16 track2_pt;

#ifdef TRACK3_EN
    u16 track3_pt;
#endif

#ifdef MCC_BAT_DET_EN
    u16 bat_pt;
    u16 trim_pt;
#endif
} MCC_TRACK_PT;

typedef struct
{
    volatile s32 stg;
    s32 flg_p_v;

    u32 curr_index;
    u32 adc_buff_size;
    u16 *padc_buff;

    u32 idle;
    u32 idx;

    u32 a_idle;

    u16 min_sampling_num;
    u16 max_sampling_num;

    u16 a_thres;
    u16 a_peak;
    u16 a_valley;

    u16 max_val;
    u16 min_val;

    u8 c_start;
    u8 c_end;

    u8 track_num;
} MCC_VAR_TRACK;

typedef struct
{
#ifdef TRACK1_EN
    u16 adc_buff_track1[TRACK1_P_V_LEN];
    MCC_VAR_TRACK s_t1;
#endif

    u16 adc_buff_track2[TRACK2_P_V_LEN];
    MCC_VAR_TRACK s_t2;

#ifdef TRACK3_EN
    u16 adc_buff_track3[TRACK3_P_V_LEN];
    MCC_VAR_TRACK s_t3;
#endif

    u16 diff_bak[TRACK3_P_V_LEN];

    u8 track_data[TRACK_DATA_LEN];
    u8 track_bak[TRACK_DATA_LEN];
    u8 g_doubt_bits[TRACK_DATA_LEN * 2];
    u8 buffer[MCC_SEP_BUF_SIZE];
    u8 doubt[MCC_SEP_BUF_SIZE];
} MCC_VAR_S __attribute__((aligned((sizeof(u32)))));



void MccInit(u32 param);
//hardware initialize
void mcc_adc_hardware_init(MCC_TRACK_PT  *adc_buff, u32 adc_buff_size, s32 t1_chsel, s32 t3_chsel, s32 bat_chsel);
void mcc_adc_hardware_uninit(void);

//function start
void mcc_adc_start(MCC_VAR_S *pmv, u32 t1_thres, u32 t2_thres, u32 t3_thres,
                   u32 min_sampling_number_T1_T3, u32 max_sampling_number_T1_T3,
                   u32 min_sampling_number_T2, u32 max_sampling_number_T2);
//function restart
void mcc_adc_restart(void);
//function stop
void mcc_adc_stop(void);


//check if brushed
u8 mcc_adc_brushed(void);

//get track data functions
//		buf, data buffer address
//		len, pointer to length, [IN] - buffer size
//			 					[OUT] - data length
//		dir, pointer to direction info [OUT]
//			 can be NULL if not care
u8 mcc_adc_get_data_track1(s8 *buf, s32 *len, u8 *dir);
u8 mcc_adc_get_data_track2(s8 *buf, s32 *len, u8 *dir);
u8 mcc_adc_get_data_track3(s8 *buf, s32 *len, u8 *dir);

u32 mcc_adc_get_bat_val(u32 base);


#endif /* MCC_IF_H_ */
