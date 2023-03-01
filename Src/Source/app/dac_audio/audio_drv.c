/*
 * @Description: 
 * @version: V1.3
 * @Author: wuyingfeng
 * @Email: yfwu<yingfeng.wu@china-core.com>
 * @Date: 2021-03-22 15:50:15
 * @LastEditTime: 2021-04-20 09:50:12
 */
#include "dac_reg.h"
#include "dac_drv.h"
#include "pit32_drv.h"
#include "dmac_drv.h"
#include "delay.h"
#include "string.h"



#define MAX_LLI_NUM     mIVoiceBlockNum
static DMA_LLI  g_dma_lli_voice[MAX_LLI_NUM];//DMA链表：链表项8项，每项1K

typedef struct
{
    volatile BOOL bStart;                             //语音播放启动标志
    volatile BOOL bBusy;                              //语音播放忙标志 
		volatile uint8_t mStatus;                         //语音播放状态机 
    uint32_t wCurAddr;                       //读取语音数据的地址
    uint32_t wTatolLength;                   //语音数据总长度
    uint32_t wOutIndex;                      //解码后数据指针
    uint32_t wDataInNumMax;                  //获取的数据最大长度
    uint8_t wDataBufIn[mIVoiceBufSize];      //从FLASH中读取的原始数据，最大8K，如果是压缩数据，则位2K
		uint8_t wDataBufOut[mIVoiceBlockSize*2]; //原始数据处理后（解压，调音量）的数据缓存，大小为原始数据缓存大小的2倍，一块播放，一块写入，实现播放的时候同时可以写入，保存语音连贯性
} tAudioDrv_t;
tAudioDrv_t sVoice;

#define AUDIOTM     ((PIT32_TypeDef *)PIT2_BASE_ADDR)  //触发DMA传输数据的定时器基地址
/**
 * Mircro Opr for Audio Drv Status
 * 
*/
/**********************START************************/
#define _bit(x)                 ((uint32_t)1 << (x))
#define _bit_set(value,bit)     (value |= (1 << bit))
#define _bit_clr(value,bit)     (value &= ~(1 << bit))
#define _chk_bit(val, bit)      ((val)&(1 << bit))

#define FDIN_MASK     0
#define FDOT_MASK     1
#define ADDONE_MASK   2
#define ADDMA_MASK    3
#define READDATA_MASK 4

#define STATUS  sVoice.mStatus

#define FDIN_SET()  _bit_set(STATUS,FDIN_MASK)      //淡入完成状态位
#define FDIN_CLR()  _bit_clr(STATUS,FDIN_MASK)
#define FDIN_CHK()  _chk_bit(STATUS,FDIN_MASK)

#define FDOT_SET()  _bit_set(STATUS,FDOT_MASK)      //淡出完成状态位
#define FDOT_CLR()  _bit_clr(STATUS,FDOT_MASK)
#define FDOT_CHK()  _chk_bit(STATUS,FDOT_MASK)

#define STDO_SET()  _bit_set(STATUS,ADDONE_MASK)    //数据完成状态位
#define STDO_CLR()  _bit_clr(STATUS,ADDONE_MASK)
#define STDO_CHK()  _chk_bit(STATUS,ADDONE_MASK)

#define STDM_SET()  _bit_set(STATUS,ADDMA_MASK)    //DMA完成状态位
#define STDM_CLR()  _bit_clr(STATUS,ADDMA_MASK)
#define STDM_CHK()  _chk_bit(STATUS,ADDMA_MASK)

#define RDDATA_SET()  _bit_set(STATUS,READDATA_MASK)    //请求数据状态位
#define RDDATA_CLR()  _bit_clr(STATUS,READDATA_MASK)
#define RDDATA_CHK()  _chk_bit(STATUS,READDATA_MASK)

#define AudioTimer_Enable()    AUDIOTM->PCSR |= PCSR_EN 
#define AudioTimer_Disable()   AUDIOTM->PCSR &= ~PCSR_EN
/************************END***************************/

/**
  * @brief  初始化驱动DMA搬运音频数据的定时器
  * 
  * @param  rate 波特率
  *         
  */
void AudioTimerInit(uint32_t rate)
{
		AUDIOTM->PCSR &= (~PCSR_EN);                                               //设置时先禁止定时器
		AUDIOTM->PCSR = (0<<8)|PCSR_OVW|PCSR_RLD;                                  //立即更新与重装                                 

		AUDIOTM->PMR = (g_ips_clk/1000000*(1000000/(float)rate));                  //重装值
		AUDIOTM->PCSR = (PIT32_CLK_DIV_1<<8)|PCSR_OVW|PCSR_RLD;                    //写分频

		DAC_Init(LEFTALIGNED_12BITS, TRIGGER_PIT, DET_ON_RISING);                  //DAC设置：左对齐，数据为16位，左对齐忽略低位
}

/**
  * @brief  AudioDrvProcessDma
  *         配置数据发送
  *         
  */
void AudioDrvProcessDma(uint8_t * data, uint32_t length)
{
    uint8_t i = 0;
    uint32_t transLen = 0, lli_num;
    uint32_t wDataOutLen = length;
		uint32_t index = 0;
    uint8_t *wBufOut = data;
    
    if (data == NULL || length == 0)
        return;
		
    if (wDataOutLen > (0xFF0<<1))
    {
        transLen = (0xFF0);                            //双字节16位
        lli_num = wDataOutLen / (0xFF0<<1);            //计算传输队列数量
        if (wDataOutLen % (0xFF0<<1)) lli_num++;
        for (i = 0; i < lli_num; i++)
        {
            g_dma_lli_voice[i].src_addr = ((uint32_t)(wBufOut)+index);         //源地址，数据存储地址
            g_dma_lli_voice[i].dst_addr = 0x40021004;                          //目标地址,DAC地址

            if (i < (lli_num - 1))
            {
                g_dma_lli_voice[i].next_lli = (UINT32)&g_dma_lli_voice[i + 1]; //指向下一个
            }
            else
            {
                g_dma_lli_voice[i].next_lli = 0;                               //末尾
            }
						//源地址增加|目标地址不变|内存到外设|目标宽度16位|源宽度16位|中断使能|源端链表使能											
						g_dma_lli_voice[i].control0 = SIEC | DNOCHG | M2P_DMA | DWIDTH_HW | SWIDTH_HW | INTEN | LLP_SRC_EN;   //设置DMA传输控制
            g_dma_lli_voice[i].len = transLen;
            index += (transLen<<1);              //更新指针       
            wDataOutLen -= (transLen<<1);        //剩余数据

            if (wDataOutLen > (0xFF0<<1))
            {
                transLen = 0xFF0;
            }
            else
            {
                transLen = (wDataOutLen>>1);
            }
        }
        dma_lli_reg_init(AUDIO_DMA_CH, &g_dma_lli_voice[0]);
    } 
    else
    {
        transLen = (wDataOutLen >> 1);      //16位传输，数量除以2
        DMA_DAC_Tran(AUDIO_DMA_CH, (uint32_t)wBufOut, transLen);
    }
}


/**
  * @brief  AudioDrvSendData
  *         配置数据发送
  *         
  */
static void AudioDrvSendData(void)
{
	 AudioDrvProcessDma(sVoice.wDataBufOut+sVoice.wOutIndex, sVoice.wDataInNumMax);       //发送输出缓冲中的数据
   //重置待发送缓冲区索引地址
	 if(sVoice.wDataInNumMax <= mIVoiceBlockSize)     //sVoice.wDataInNumMax肯定是<=mIVoiceBlockSize的
   {
				sVoice.wOutIndex=(sVoice.wOutIndex+sVoice.wDataInNumMax)>mIVoiceBlockSize?0:mIVoiceBlockSize;
   }
   //待发送数据长度设置为0
   sVoice.wDataInNumMax = 0;
}

/**
  * @brief  音频功能的中断处理函数
  * 
  */
void AudioISRHandler(void)
{
    STDM_SET();
    //待发送数据若不为空，配置DMA发送
    if (sVoice.wDataInNumMax)
    {
        //配置发送下一段数据
        STDM_CLR();
        AudioDrvSendData();
    }
    //设置读取标记，需要读取新数据
    RDDATA_SET();
}

/**
  * @brief  初始化驱动DMA
  *         
  */
static void AudioDMAInit(void)
{
    DMA_REG_Init(DMA2_BASE_ADDR);
    NVIC_Init(3, 3, DMA2_IRQn, 2);
}

/**
  * @brief  根据参考点寻找语言平滑过渡的起点/终点
  * 
  * @param  i 向上平滑/向下平滑移动
  * @param  data 参考点
  *         
  */
static uint32_t FindPointInFadeData(uint8_t i, uint16_t data)
{
    uint16_t end_addr, base_data, start_addr;
    uint32_t index;

    if (i == 0)
    {
        data = (data / 0x10) * 0x10; 
        for (end_addr = 0, base_data = 0, index = 0; data > base_data; end_addr = end_addr + 2)
        {
            base_data = base_data + 0x10;
            index = index + 0x02;
        }
    }
    else
    {
        for (start_addr = 0, base_data = 0xfff0, index = 0x2000; data < base_data; start_addr = start_addr + 2)
        {
            base_data = base_data - 0x10;
            index = index - 0x02;
        }

        index = 0x2000 - index;
    }
    return index;
}

/**
  * @brief  配置用于语言平滑播放的前置/后置数据
  * 
  * @param  data 参考点
  *         
  */
static void AudioFadeConfig(uint8_t *data)
{
    uint8_t dir = 0;
    uint16_t sData = 0, tData = 0;
    uint32_t dlen,addr = 0;
	
	  sData = (DAC->DAC_DOR<< 4);                        //读取DAC输出寄存器值，DAC为12位，高12位，左移4位为16位数据
    tData = (data[1] << 8) + data[0];                  //参考值 
	
    if (sData < tData)                                 //DAC寄存器值小于给定值，淡入
    {
        dir = 0;
        addr = (uint32_t)fadein_data;
    }
    else                                              //DAC寄存器值大于给定值，淡出  
    {
        dir = 1;
        addr = (uint32_t)fadeout_data;
    }

    dlen = FindPointInFadeData(dir, sData);
    addr += dlen;
    dlen = FindPointInFadeData(dir, tData) - dlen;
	
    if (dlen)                
    {
				STDM_CLR();   //清DMA完成标志
        AudioDrvProcessDma((uint8_t*)addr,dlen);    //发淡入淡出数据        
    }
		else              //无淡入淡出需求
		{
				AudioISRHandler();
		}	
}


/**
  * @brief  读取数据函数
  * 
  * @param  pbData 读数据缓冲区
  * @param  dwAddr 待读取数据首地址
  * @param  lentgh 读取长度
  *         
  */
static void AudioReadBytes(uint8_t *pbData, uint32_t dwAddr,uint32_t lentgh)
{
    AudioHalReadBytes(pbData, lentgh, dwAddr);
}

/**
  * @brief  配置用于语言平滑播放的前置/后置数据
  * 
  * @param  length 获取数据长度
  *         
  */
static void AudioDrvGetDataOut(uint32_t length)
{
    uint32_t dlen = 0;
    uint16_t data = 0;
		
		if(sVoice.wTatolLength)                                              //语音播放数据
    {  
#if AUDIO_COMPRESS                                                       //压缩情况读出的数据要*4，也就是说缓存只能容纳1/4原始数据
        dlen = length/4;
#else
        dlen = length;
#endif	
				dlen = sVoice.wTatolLength>dlen?dlen:sVoice.wTatolLength;        //能够读取的数据长度
        AudioReadBytes(sVoice.wDataBufIn,sVoice.wCurAddr,dlen);          //读取数据到BufIn缓存  
				
				sVoice.wTatolLength -= dlen;                                     //更新语音数据长度
				sVoice.wCurAddr+= dlen;                                          //更新数据起始地址 

        AudioHalDecode(sVoice.wDataBufOut+sVoice.wOutIndex,sVoice.wDataBufIn,&dlen);   //解码数据到wDataBufOut缓存   
        sVoice.wDataInNumMax = dlen;                                    //wDataBufIn中的数据数量
    }
    else                                      //数据为发完，发淡出数据
    {		
				if(!FDOT_CHK()&& STDM_CHK())          //DMA完成，淡出未完成
				{
						data = (uint16_t)0x8000;
						AudioFadeConfig((uint8_t*)&data);
						FDOT_SET();		                    //淡出设置标志  
				}
    }
		RDDATA_CLR();                             //数据读出完成，清标志
		
		if(!FDIN_CHK())                           //如果淡入未开始
		{
				AudioFadeConfig((sVoice.wDataBufOut+sVoice.wOutIndex));       //淡入数据，以缓存中第一个数据做依据
				FDIN_SET();                          //淡入标志 
		}
}

/**
  * @brief  初始化语言播放驱动层初始化
  *         
  */
void AudioDrvInit(void)
{
#ifdef AUDIO_LOG
    AUDIO_LOG("Audio Init\n");
#endif
    memset(&sVoice,0,sizeof(sVoice));
    AudioDMAInit();
}

/**
  * @brief  停止播放
  *         
  */
void AudioDrvStop(void)
{
		AudioTimer_Disable();
		DMA_dis(AUDIO_DMA_CH);
    if(sVoice.bStart)
    {     
        //播放状态
        AudioTimer_Enable();
        //播放一段静音数据，避免噪音/爆音出现
        AudioDrvProcessDma(voicemutedata,16);
        DelayMS(3);
        AudioTimer_Disable();
        DMA_dis(AUDIO_DMA_CH);
				memset(&sVoice,0,sizeof(sVoice));
    }
}

void AudioPowerOnPlayMute(void)
{
		AudioTimer_Enable();
		//播放一段静音数据，避免噪音/爆音出现
		AudioDrvProcessDma(voicemutedata,16);
		DelayMS(3);
		AudioTimer_Disable();
		DMA_dis(AUDIO_DMA_CH);
}



/**
  * @brief  语言播放功能
  *         
  */
void AudioDrvHandle(void)
{
		if(!sVoice.bBusy) return;               //没有语音播放，退出
	
    if (sVoice.bStart == 0)                 //有语音播放，但还未开始
    {	
				sVoice.bStart = TRUE;               //启动播放
        AudioTimer_Enable();                //开启定时器
				RDDATA_SET();                       //设置读语音数据标志 
    }

    if(RDDATA_CHK())                        //读语音数据标志置位   
    {
				AudioDrvGetDataOut(mIVoiceBlockSize);  //读取语音数据
    }
		else if(STDM_CHK())                     //DMA发送完成
		{
				if(sVoice.wDataInNumMax)            //还有数据未发送完成
				{			
						AudioDrvSendData();
				}
				else if(FDOT_CHK())                      //淡出内容已完，关闭输出
				{		
						AudioDrvStop();
				}
		}
}

/**
  * @brief  当前播放状态
  * 
  * @return  
  *         
  */
BOOL AudioBusyStatus(void)
{
    return sVoice.bBusy;
}

/**
  * @brief  设置待播放语音文件
  * 
  * @param addr 文件首地址
  * @param length 文件长度
  *         
  */
void AudioDrvConfig(uint32_t addr,uint32_t length)
{
    sVoice.wCurAddr = addr;
    sVoice.wTatolLength = length;
	
		sVoice.bBusy	= TRUE;
		if (sVoice.wTatolLength == 0)
		{
				AudioDrvStop();
		}
}
