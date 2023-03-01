/*
 * @Description: 
 * @version: V1.3
 * @Author: wuyingfeng
 * @Email: yfwu<yingfeng.wu@china-core.com>
 * @Date: 2021-03-22 15:51:37
 * @LastEditTime: 2021-06-03 09:20:37
 */
#include "audio_hal.h"
#include "ssi_drv.h"
#include "flash.h"
#include "string.h"

#define MAX_TUNE_VOLUME 10
uint8_t g_tuneVolume = 10;

/**
 * @brief  将4字节合并为1个32bit变量
 * @param[in] {value} 4字节数组
 * @return 合并完成后的32bit变量
 */
uint32_t byte_to_int(const uint8_t value[4])
{
	uint32_t nubmer = 0;

	nubmer = (uint32_t)value[3];
	nubmer <<= 8;
	nubmer |= (uint32_t)value[2];
	nubmer <<= 8;
	nubmer |= (uint32_t)value[1];
	nubmer <<= 8;
	nubmer |= (uint32_t)value[0];

	return nubmer;
}

/* @brief  根据语言文件编号，索引文件存储地址和长度
  *   
  * @param  playNumber 语言文件编号 
  * @param  length 文件长度 
  *      
  * @return addr 文件首地址
  */
static uint32_t AudioHalFlashAddr(uint16_t playNumber, uint32_t * length)      //
{
		AudioItemInfo_t iteminfo={0};
		uint32_t addr = 0;
		AudioDecoderInit();
  /**********************************/
		AudioHalReadBytes(iteminfo.InfoBuf,sizeof(AudioItemInfo_t),(playNumber-1)*sizeof(AudioItemInfo_t)+AUDIO_FLASH_HEAD_SIZE);       
		*length=iteminfo.Details.size;
		addr=iteminfo.Details.addr;
		return addr;
}

/**
  * @brief  解压缩功能
  *   
  * @param  dst 目的地址
  * @param  src 源地址
  * @param  length 解码前/解码后长度 
  *      
  */
void AudioHalDecode(uint8_t *dst, uint8_t *src, uint32_t *length)
{
#if AUDIO_COMPRESS           //压缩文件，根据协议解压
	uint32_t i = 0,mlen = *length;
	int16_t temp;
  for (;i < mlen; i++)
  {
 		temp = (AudioDecodeProc(src[i] & 0x0F)/MAX_TUNE_VOLUME)*g_tuneVolume*7/10;
		temp += 0x8000;
    
		dst[4 * i] = (unsigned char)(temp & 0xff);
		dst[4 * i + 1] = (unsigned char)((temp >> 8) & 0xff);

		temp = (AudioDecodeProc((src[i] >> 4) & 0x0F)/MAX_TUNE_VOLUME)*g_tuneVolume*7/10; 
		temp += 0x8000;
	
		dst[4 * i + 2] = (unsigned char)(temp & 0xff);
		dst[4 * i + 3] = (unsigned char)((temp >> 8) & 0xff);
  }
	*length = mlen*4;
#else                       //非压缩文件，直接搬运   
	
  memcpy(dst,src,*length);
#endif
}

/**
  * @brief  读取任意字节数
  *   
  * @param  dst 语言文件编号 
  * @param  length 读取长度 
  * @param  addr 地址 
  *      
  */
void AudioHalReadBytes(uint8_t *dst, uint32_t length, uint32_t addr)
{
		ENTER_CRITICAL();
		ExFlashRead(dst,addr,length);                   //语音数据，从FLASH读取
		EXIT_CRITICAL();
}


/**
  * @brief  初始化语言播放HAL层
  *         
  */
void AudioHalInit(void)
{
		AudioDrvInit();
}

/**
  * @brief  释放语言播放HAL层
  *         
  */
void AudioHalFinalize(void)
{
  
}

/**
  * @brief  设置音量大小
	* @param  volume 音量参数处于0~MAX_TUNE_VOLUME之间
  *         
  */
void AudioHalSetVolume(uint8_t volume)
{
	g_tuneVolume = volume>MAX_TUNE_VOLUME?MAX_TUNE_VOLUME:volume;
}

/**
  * @brief  获取当前音量
  *         
  * @return 当前音量值
  */
uint8_t AudioHalGetCurVolume()
{
	return g_tuneVolume;
}

/**
  * @brief  获取最大音量
  *         
  * @return 最大音量值，MAX_TUNE_VOLUME
  */
uint8_t AudioHalGetMaxVolume()
{
	return MAX_TUNE_VOLUME;
}


/**
  * @brief  设置待播放语言文件
  * 
  * @param  playNumber 语言文件编号
  *         
  */
uint8_t AudioHalSetPlayNumber(uint16_t playNumber)
{
		uint32_t length = 0;
		uint32_t addr;
		
		addr = AudioHalFlashAddr(playNumber,&length);
	
	//  printf("addr,len:0x%08x 0x%08x\n",addr,length); 
	
		if(length>SINGLE_AUDIO_SIZE_MAX)return 0;               //语音文件最长为0x569e,编号142：验证错误次数过多，锁定3分钟
	
		PlayInfo.Number=playNumber; 
		PlayInfo.StartTime=GetTimerCount();
		PlayInfo.Timeout=length/5;
		AudioDrvInit();
		AudioDrvConfig(addr,length);
		return 1;
}

/**
  * @brief  播放语言
  *         
  */
_ErrorStatus AudioHalPlay(void)            //语音播放状态机处理
{
		AudioDrvHandle();
		return AudioHalGetStatus();
}

/**
  * @brief  停止语言播放
  *         
  */
void AudioHalStop(void)
{
		AudioDrvStop();
}

/**
  * @brief  获取当前播放状态
  *         
  */
_ErrorStatus AudioHalGetStatus(void)
{
  return AudioBusyStatus()?_ERROR:_SUCCESS;
}
