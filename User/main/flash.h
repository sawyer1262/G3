#ifndef  __FLASH_H_
#define  __FLASH_H_



#include <stdint.h>
#include "ssi_drv.h"


#define  DP_CMD       0xB9  
#define  RDP_CMD      0xAB

#define  FLASH_SSI    SSI1
#define  RT_OK 				0
#define  RT_FAIL 			1



/***********************************************************************
*外部Flash 分配
外部flash起始|指纹数据|语音数据|字库数据|..|结束
***********************************************************************/

#define SSI_FSH_SIZE														(0x00800000) /*外部flash大小8M*/
#define SSI_FSH_BLOCK_SIZE											(0x00001000) /*外部flash块大小*/
#define SSI_FSH_START_ADDR											(0x00000000) /*外部flash起始地址*/
#define SSI_FSH_PAGE_SIZE												(0x00000100) /*外部页大小*/
#define SSI_FSH_END_ADDR											  (SSI_FSH_START_ADDR+SSI_FSH_SIZE) /*外部flash结束地址*/



uint16_t SSI_ReleaseDeepPW(SSI_TypeDef *SSIx);
uint8_t SQI_CheckProtect(SSI_TypeDef *SSIx);
uint16_t SSI_EnterDeepPW(SSI_TypeDef *SSIx);


void SSI_FLASH_Init(void);
void SSI_FLASH_DeInit(void);
void ExFlashTest(void);


void INT_EFLASH_Init(void);
void IntFlashRead(uint8_t* pDataBuf,uint32_t nOffset,uint16_t nDataNum);
uint8_t IntFlashWrite(uint8_t* pDataBuf,uint32_t nOffset,uint16_t nDataNum);
void IntFlashEraseBackup(void);
uint8_t ExFlashWrite(uint8_t *pDataBuf,uint32_t lAddress,uint32_t lLen);
uint8_t  ExFlashRead(uint8_t *pData, uint32_t nAddr, uint32_t nLen);



#endif


