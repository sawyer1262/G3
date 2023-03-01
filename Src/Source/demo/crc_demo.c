// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : crc_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "crc_drv.h"
#include "type.h"
#include "debug.h"

static UINT8 buf2[]=
{
	0x12,0x00,0x00,0x00,0x12,0x15,0x00,0x00,0x12,0x15,0x00,0x00,0x35,0x00,0x00,0x00,
	0x12,0x15,0x35,0x24,0x12,0x15,0x35,0x24,0xc0,0x00,0x00,0x00,0x12,0x15,0x35,0x24,
	0xc0,0x89,0x00,0x00
};

void Crc_Demo(void)
{
	UINT8  crc_8  = 0;
	UINT16 crc_16 = 0;
	UINT32 crc_32 = 0;
	CRC_InitTypeDef CRC_InitStruct;

	CRC_InitStruct.CRC_DataFormat = CRC_DATA_FORMAT_BIGENDDING;
	CRC_InitStruct.CRC_Mode = CRC_MODE_CRC_8;
	CRC_InitStruct.CRC_SourceSel = CRC_SOURCE_SEL_FROM_CPU;
	CRC_Config(&CRC_InitStruct);
	CRC_SetInitValue(0xffffffff);

	crc_8 = CRC_CalcCRC8(buf2, sizeof(buf2)/sizeof(char));

	printf ("CRC result is 0x%02x\r\n", crc_8);

	CRC_InitStruct.CRC_DataFormat = CRC_DATA_FORMAT_BIGENDDING;
	CRC_InitStruct.CRC_Mode = CRC_MODE_CRC_16;
	CRC_InitStruct.CRC_SourceSel = CRC_SOURCE_SEL_FROM_CPU;

	CRC_Config(&CRC_InitStruct);
	CRC_SetInitValue(0xffffffff);

	crc_16 = CRC_CalcCRC16(buf2, sizeof(buf2)/sizeof(char));

	printf ("CRC result is 0x%04x\r\n", crc_16);

	CRC_InitStruct.CRC_DataFormat = CRC_DATA_FORMAT_LITTLEENDDING;
	CRC_InitStruct.CRC_Mode = CRC_MODE_CRC_32;
	CRC_InitStruct.CRC_SourceSel = CRC_SOURCE_SEL_FROM_CPU;

	CRC_Config(&CRC_InitStruct);
	CRC_SetInitValue(0xffffffff);
	crc_32 = CRC_CalcCRC32(buf2, sizeof(buf2)/sizeof(char));
	printf ("\r\nCRC little endding result is 0x%08x\r\n", crc_32);


	CRC_InitStruct.CRC_DataFormat = CRC_DATA_FORMAT_BIGENDDING;
	CRC_InitStruct.CRC_Mode = CRC_MODE_CRC_32;
	CRC_InitStruct.CRC_SourceSel = CRC_SOURCE_SEL_FROM_CPU;

	CRC_Config(&CRC_InitStruct);
	CRC_SetInitValue(0xffffffff);
	crc_32 = CRC_CalcCRC32(buf2, sizeof(buf2)/sizeof(char));
	printf ("\r\nCRC big endding result is 0x%08x\r\n", crc_32);

}
