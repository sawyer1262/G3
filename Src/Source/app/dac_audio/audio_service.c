/*
 * @Description: 
 * @version: V1.3
 * @Author: wuyingfeng
 * @Email: yfwu<yingfeng.wu@china-core.com>
 * @Date: 2021-03-21 18:18:18
 * @LastEditTime: 2021-04-20 12:03:22
 */
#include "audio_service.h"
#include "stdlib.h"

#include "string.h"
#include "debug.h"
#include "timer.h"

/* 计数器 */
//static volatile int xTickCount = 0U;

//#define Calc_PastTime(currentTime, lastTime)   ( (currentTime >= lastTime) ? (currentTime - lastTime) : (UINT32_MAX - lastTime + currentTime) )

/* 计算环形缓存里面可以读的数据个数 */
#define RbCanRead(rb)  ( (rb->read <= rb->write) ? ( (rb->write - rb->read) / rb->size ) : ( rb->len - ((rb->read - rb->write) / rb->size) ) )

/* 计算环形缓存里面可以写的数据个数 */
#define RbCanWrite(rb) ( rb->len - RbCanRead(rb) )

typedef void* RingBufferHandle_t;

/* 创建环形缓存，保存待播放的语音 */
static RingBufferHandle_t rbHandle = NULL;

/* 定义缓存大小 */
#define AUDIO_RING_BUFFER_LEN  ( 40 )
#define AUDIO_RING_BUFFER_SIZE ( 2 )

static AudioServiceCallback_t pAdCallBack = NULL;
volatile uint16_t activeAudioNumber = 0;
volatile PlayInfo_t PlayInfo={0};



/* 环形缓存数据结构 */
typedef struct
{
	uint16_t len;   //缓存长度（数据个数）
	uint16_t size;  //每个数据的大小
	volatile uint8_t *write;
	volatile uint8_t *read;
	volatile uint8_t buff[];
}RingBuffer_t;



/**
  * @brief  创建环形缓存
  * @note   
  * @param  buffLength：缓存长度
  * @param  itemSize：条目大小
  *         
  * @return 返回缓存句柄，错误返回NULL
  */
RingBufferHandle_t RingBufferCreate(uint16_t buffLength, uint8_t itemSize)
{
	RingBuffer_t *pRB;
	
	printf("sizeof RingBuffer_t: %02d\n",sizeof(RingBuffer_t));
	
	if (buffLength == 0 || itemSize == 0)
	{
		return NULL;
	}
	buffLength = buffLength + 1;
	
	pRB = (RingBuffer_t*)malloc(sizeof(RingBuffer_t) + buffLength * itemSize);

	if (pRB != NULL)
	{
		pRB->len = buffLength;
		pRB->size = itemSize;
		pRB->read = pRB->buff;
		pRB->write = pRB->buff;
	}
  else
  {
      //分配失败，可以重启芯片
      //TO DO
  }
	return (RingBufferHandle_t)pRB;
}

/**
  * @brief  往环形缓存写入一个数据
  * @note   该函数允许中断调用（一般是中断收到数据，就调用该函数将数据写入缓存）
  *         
  * @param  handle：缓存句柄
  * @param  pData：数据首地址
  *         
  * @return SUCCESS   ERROR
  */
_ErrorStatus RingBufferWrite(RingBufferHandle_t handle, void *pData)
{
	RingBuffer_t *pRB = (RingBuffer_t *)handle;
	
	if (pRB == NULL || pData == NULL)
	{
		return _ERROR;
	}
	
	if (RbCanWrite(pRB) <= 1)
	{
		return _ERROR;
	}
	memcpy((void*)(pRB->write), pData, pRB->size);
	
	pRB->write += pRB->size;
	if (pRB->write == (pRB->buff + pRB->len * pRB->size))
	{
		pRB->write = pRB->buff;
	}
	return _SUCCESS;
}

/**
  * @brief  从环形缓存里面读出读取有效数据的长度
  * @note   
  * @param  handle：缓存句柄
  *         
  * @return 返回数据长度（RB_BUF里面的有效字节数）
  */
uint32_t RingBufferGetValidSize(RingBufferHandle_t handle)
{
	RingBuffer_t *pRB = (RingBuffer_t *)handle;
	
	if (pRB == NULL)
	{
		return 0;
	}
	return RbCanRead(pRB);
}

/**
  * @brief  从环形缓存里面读出一个数据
  * @note   
  * @param  handle：缓存句柄
  * @param  pData：数据首地址
  *         
  * @return SUCCESS   ERROR
  */
_ErrorStatus RingBufferRead(RingBufferHandle_t handle, void *pData)
{
	RingBuffer_t *pRB = (RingBuffer_t *)handle;
	
	if (pRB == NULL || pData == NULL)
	{	
			return _ERROR;
	}
	
	if (RbCanRead(pRB) == 0)
	{
			return _ERROR;
	}
	memcpy(pData, (void*)(pRB->read), pRB->size);
	
	pRB->read += pRB->size;
	if (pRB->read == (pRB->buff + pRB->len * pRB->size))
	{
		pRB->read = pRB->buff;
	}
	return _SUCCESS;
}

/**
  * @brief  复位环形缓存
  * @note   清空缓存里面的所有数据
  *
  * @param  handle：缓存句柄
  *         
  * @return SUCCESS   ERROR
  */
_ErrorStatus RingBufferReset(RingBufferHandle_t handle)
{
	RingBuffer_t *pRB = (RingBuffer_t *)handle;
	
	if (pRB != NULL)
	{
		pRB->read = pRB->buff;
		pRB->write = pRB->buff;
		return _SUCCESS;
	}
	return _ERROR;
}


/**
 * @brief AudioServicePlayNext
 * 		播放下一个音频
 * @retval NONE 
 */
void AudioServicePlayNext()
{
    uint16_t audioNumber;
    
    if(activeAudioNumber == 0)
    {
				if (RingBufferRead(rbHandle, &audioNumber) == _SUCCESS)
				{
						if(AudioHalSetPlayNumber(audioNumber)==1)
						{
								activeAudioNumber = audioNumber;
							//	printf("play next:%2d\n",activeAudioNumber); 
						}
				}
				else
				{
						AudioDrvStop();
				}
    }
}

/**
 * @brief AudioServiceScan
 * 		扫描当前音频服务工作状态，执行回调函数
 * @retval 当前状态 
 */



_ErrorStatus AudioServiceScan()
{
		_ErrorStatus status = AudioHalGetStatus();
	
    if(status == _SUCCESS || activeAudioNumber==0)
    {   
        activeAudioNumber = 0;
        AudioServicePlayNext();
			  if(pAdCallBack != NULL)
        {
            pAdCallBack(activeAudioNumber);
        }
    }
		else if(activeAudioNumber==PlayInfo.Number)
		{
				if(PlayInfo.StartTime>0 && GetTimerElapse(PlayInfo.StartTime)>PlayInfo.Timeout)
				{
					//	printf("audio timeout\n");
						PlayInfo.StartTime=0;
						PlayInfo.Number=0;
						PlayInfo.Timeout=0;
						activeAudioNumber = 0;
				}
		}
		return status;
}


/**
 * @brief AudioSeriviceInit
 * 		初始化音频服务
 * @retval NONE 
 */
_ErrorStatus AudioServiceInit(AudioServiceCallback_t mCallback)
{
    AudioHalInit();
    if(mCallback == NULL) return _ERROR;
    pAdCallBack = mCallback;
    rbHandle = RingBufferCreate(AUDIO_RING_BUFFER_LEN, AUDIO_RING_BUFFER_SIZE);
		return _SUCCESS;
}

/**
 * @brief AudioSerivicePlay
 * 		音频服务
 * @param[in] number
 * 		需要播放的语言文件编号
 * @param[in] mode
 * 		音频播放模式，是否需要打断当前音频，0:打断当前音频，并立即播放,1:等待播放结束后再播放
 * @retval NONE 
 */
void AudioServicePlay(uint16_t playNumber,PlayMode_t mode)
{
  uint16_t audioNumber = playNumber;
	
	if (BREAK == mode)
	{
			activeAudioNumber = 0;
			RingBufferReset(rbHandle);
			AudioHalStop();
			AudioHalInit();
	}
	RingBufferWrite(rbHandle, &audioNumber);
	AudioServicePlayNext();
}

/**
 * @brief AudioServiceSetVolume
 * 		音频服务设置音量大小功能（客户可自行增加音量++,--功能，仅作示例，不增加接口）
 * @param volume 音量参数取值范围0~最大值
 * @retval NONE 
 */
void AudioServiceSetVolume(uint8_t volume)
{
    uint8_t uMaxVolume = 0;
    
    (void)uMaxVolume;

    uMaxVolume = AudioHalGetMaxVolume();

    AudioHalSetVolume(volume);  //设置音量
}

/**
 * @brief AudioSeriviceRouting
 * 		音频服务例行进程，系统主循环调用即可
 * @retval NONE 
 */
void AudioServiceRouting()
{
		static uint32_t sevtime=0;
		AudioHalPlay();
		if(sevtime==0)
		{
				AudioServiceScan();
				sevtime=GetTimerCount();
		}
		else if(GetTimerElapse(sevtime)>50)
		{
				sevtime=0;
		}
}
