// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : spi_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <string.h>
#include "memmap.h"
#include "spi_demo.h"
#include "spi_drv.h"
#include "spi_reg.h"
#include "dmac_drv.h"
#include "debug.h"
#include "delay.h"
#include "ccm_drv.h"
#include "ccm_reg.h"

#define SPI_MASTER_MODE 1

void Demo_SPI_CPUNormalTransfer(SPI_TypeDef *SPIx)
{
    UINT32 len, i;
    UINT16 ret;
    UINT8 send[256] = {0};
    UINT8 receive[256];
    SPI_InitTypeDef SPI_InitStruct;

    printf("SPI CPU normal test.\r\n");

#if SPI_MASTER_MODE

    SPI_StructInit(SPI_Mode_Master, &SPI_InitStruct);
    SPI_Init(SPIx, &SPI_InitStruct);

    len = 0x100;
    memset(receive, 0x00, len);
    for (i = 0; i < len; i++)
    {
        send[i] = i;
    }
    while (1)
    {
        DelayMS(500);
        SPI_CS_L(SPIx);
        ret = SPI_MasterSendData(SPIx, send, len);
        SPI_CS_H(SPIx);
        if (STATUS_OK == ret)
        {
            printf("SPI master send success.\r\n");
            DelayMS(500);
            SPI_CS_L(SPIx);
            ret = SPI_MasterReceiveData(SPIx, receive, len);
            SPI_CS_H(SPIx);
            if (STATUS_OK == ret)
            {
                printf("SPI master receive success.\r\n");
                printf("receive = ");
                for (i = 0; i < len; i++)
                {
                    printf("%02x", receive[i]);
                }
                printf("\r\n");
            }
            else
            {
                printf("SPI master receive failed,ret = %04x.\r\n", ret);
            }
        }
        else
        {
            printf("SPI master send failed,ret = %04x.\r\n", ret);
        }
    }

#else //slave
    SPI_StructInit(SPI_Mode_Slave, &SPI_InitStruct);
    SPI_Init(SPIx, &SPI_InitStruct);

    len = 0x100;
    for (i = 0; i < len; i++)
    {
        send[i] = i;
    }
    while (1)
    {
        memset(receive, 0, len);
        ret = SPI_SlaveReceiveData(SPIx, receive, len);
        if (STATUS_OK == ret)
        {
            printf("SPI slave receive success.\r\n");
            printf("receive = ");
            for (i = 0; i < len; i++)
            {
                printf("%02x", receive[i]);
            }
            printf("\r\n");
            ret = SPI_SlaveSendData(SPIx, send, len);
            if (STATUS_OK == ret)
            {
                printf("SPI slave send success.\r\n");
            }
            else
            {
                printf("SPI slave send failed,ret = %04x.\r\n", ret);
            }
        }
        else
        {
            printf("SPI slave receive failed,ret = %04x.\r\n", ret);
        }
    }

#endif
}

void Demo_SPI_DMATransfer(SPI_TypeDef *SPIx)
{
    UINT32 len, i;
    UINT16 ret;
    UINT8 send[256] = {0};
    UINT8 receive[256];
    UINT32 dma_base_addr;
    SPI_InitTypeDef SPI_InitStruct;

    printf("SPI DMA test.\r\n");

    if ((SPIx == SPI1) || (SPIx == SPI2))
    {
        dma_base_addr = DMA1_BASE_ADDR;
#ifdef SPI_DMA_INT
        NVIC_Init(3, 3, DMA1_IRQn, 2);
#endif
    }
    else if (SPIx == SPI3)
    {
        dma_base_addr = DMA2_BASE_ADDR;
#ifdef SPI_DMA_INT
        NVIC_Init(3, 3, DMA2_IRQn, 2);
#endif
    }
    else
    {
        printf("SPI type id error.\r\n");
		while(1);
    }

    DMA_Init(dma_base_addr);

    len = 0x100;
    for (i = 0; i < len; i++)
    {
        send[i] = i;
    }
#if SPI_MASTER_MODE

    printf("SPI DMA master test.\r\n");
    SPI_StructInit(SPI_Mode_Master, &SPI_InitStruct);
    SPI_Init(SPIx, &SPI_InitStruct);

    while (1)
    {
        DelayMS(500);
        memset(receive, 0x00, len);
        SPI_CS_L(SPIx);
        SPI_EnableDMA(SPIx, TRUE);
        ret = dma_spitran(SPIx, send, receive, len, TRUE);
        SPI_EnableDMA(SPIx, FALSE);
        SPI_CS_H(SPIx);
        if (STATUS_OK == ret)
        {
            printf("SPI DMA transfer success.\r\n");
        }
        else
        {
            printf("SPI DMA transfer failed,ret = %04x.\r\n", ret);
        }

        printf("receive = ");
        for (i = 0; i < len; i++)
        {
            printf("%02x", receive[i]);
        }
        printf("\r\n");
    }
#else //slave mode

    printf("SPI DMA slave test.\r\n");
    SPI_StructInit(SPI_Mode_Slave, &SPI_InitStruct);
    SPI_Init(SPIx, &SPI_InitStruct);
    while (1)
    {
        memset(receive, 0x00, len);
        SPI_EnableDMA(SPIx, TRUE);
        ret = dma_spitran(SPIx, send, receive, len, FALSE);
        SPI_EnableDMA(SPIx, FALSE);
        if (STATUS_OK == ret)
        {
            printf("SPI DMA transfer success.\r\n");
            printf("receive = ");
            for (i = 0; i < len; i++)
            {
                printf("%02x", receive[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("SPI DMA transfer failed,ret = %04x.\r\n", ret);
        }
    }
#endif
}

void Demo_SPI_EDMATransfer(SPI_TypeDef *SPIx)
{
    UINT32 len, i;
    UINT16 ret;
    UINT8 send[256] = {0};
    UINT8 receive[256];
    SPI_InitTypeDef SPI_InitStruct;

#if SPI_MASTER_MODE

    SPI_StructInit(SPI_Mode_Master, &SPI_InitStruct);
    SPI_Init(SPIx, &SPI_InitStruct);
    len = 0x100;
    memset(receive, 0x00, len);
    for (i = 0; i < len; i++)
    {
        send[i] = i;
    }
#ifdef EDMA_INT
    NVIC_Init(3, 3, ENCR1_IRQn, 2);
#endif
    while (1)
    {

#if 1 //全双工
        DelayMS(500);
        SPI_CS_L(SPIx);
        ret = EDMA_SPITransfer(SPIx, (UINT32)send, (UINT32)receive, len);
        SPI_CS_H(SPIx);
        if (STATUS_OK == ret)
        {
            printf("SPI EDMA transfer success.\r\n");
            printf("receive = ");
            for (i = 0; i < len; i++)
            {
                printf("%02x", receive[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("SPI EDMA transfer failed,ret = %04x.\r\n", ret);
        }
#else //半双工
        DelayMS(500);
        SPI_CS_L(SPIx);
        ret = SPI_MasterSendDataByEDMA(SPIx, (UINT32)send, len);
        SPI_CS_H(SPIx);
        if (STATUS_OK == ret)
        {
            printf("SPI EDMA master send success.\r\n");
            DelayMS(500);
            memset(receive, 0x00, len);
            SPI_CS_L(SPIx);
            ret = SPI_MasterRecvDataByEDMA(SPIx, (UINT32)receive, len);
            SPI_CS_H(SPIx);
            if (STATUS_OK == ret)
            {
                printf("SPI EDMA master receive success.\r\n");
                printf("receive = ");
                for (i = 0; i < len; i++)
                {
                    printf("%02x", receive[i]);
                }
                printf("\r\n");
            }
            else
            {
                printf("SPI EDMA master receive failed,ret = %04x.\r\n", ret);
            }
        }
        else
        {
            printf("SPI EDMA master send failed,ret = %04x.\r\n", ret);
        }
#endif
    }

#else //slave
    SPI_StructInit(SPI_Mode_Slave, &SPI_InitStruct);
    SPI_Init(SPIx, &SPI_InitStruct);

    len = 0x100;
    for (i = 0; i < len; i++)
    {
        send[i] = i;
    }
#ifdef EDMA_INT
    NVIC_Init(3, 3, ENCR1_IRQn, 2);
#endif
    while (1)
    {
#if 1 //全双工
        memset(receive, 0x00, len);
        ret = EDMA_SPITransfer(SPIx, (UINT32)send, (UINT32)receive, len);
        if (STATUS_OK == ret)
        {
            printf("SPI EDMA transfer success.\r\n");
            printf("receive = ");
            for (i = 0; i < len; i++)
            {
                printf("%02x", receive[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("SPI EDMA transfer failed,ret = %04x.\r\n", ret);
        }
#else //半双工

        memset(receive, 0x00, len);
        SPIx->SPICR1 |= SPI_ENABLE;
        ret = SPI_SlaveRecvDataByEDMA(SPIx, (UINT32)receive, len);
        SPIx->SPICR1 &= ~SPI_ENABLE;
        if (STATUS_OK == ret)
        {
            printf("SPI EDMA slave receive success.\r\n");
            printf("receive = ");
            for (i = 0; i < len; i++)
            {
                printf("%02x", receive[i]);
            }
            printf("\r\n");
            SPIx->SPICR1 |= SPI_ENABLE;
            ret = SPI_SlaveSendDataByEDMA(SPIx, (UINT32)send, len);
            SPIx->SPICR1 &= ~SPI_ENABLE;
            if (STATUS_OK == ret)
            {
                printf("SPI EDMA slave send success.\r\n");
            }
            else
            {
                printf("SPI EDMA slave send failed,ret = %04x.\r\n", ret);
            }
        }
        else
        {
            printf("SPI EDMA slave receive failed,ret = %04x.\r\n", ret);
        }
#endif
    }
#endif
}

void SPI_Slave_OneLineModeTest(void)
{
    SPI_InitTypeDef SPI_InitStruct;
    unsigned char spidat[512] = {0};

    SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_RxOrTx;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Slave;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;

    SPI_Init(SPI2, &SPI_InitStruct);

    while (1)
    {
#if 1
        memset(spidat, 0x00, sizeof(spidat));
        SPI_SlaveReceiveData(SPI2, spidat, 16); //input
                                                // printf("recv dat:\r\n");
                                                // for (i = 0; i < 16; i++)
                                                // {
                                                //     printf("%02x ", spidat[i]);
                                                // }
                                                // printf("\r\n");
#else
        for (i = 0; i < 16; i++)
        {
            spidat[i] = i;
        }
#endif
        SPI_ConfigGpio(SPI2,SPI_MISO,GPIO_OUTPUT);
        SPI_SlaveSendData(SPI2, spidat, 16); //output
        SPI_ConfigGpio(SPI2,SPI_MISO,GPIO_INPUT);
    }
}

void SPI_Master_OneLineModeTest(void)
{
    SPI_InitTypeDef SPI_InitStruct;
    unsigned char spidat[512] = {0};
    unsigned char spidat1[512] = {0};
    unsigned int i;

    SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_RxOrTx;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;

    SPI_Init(SPI3, &SPI_InitStruct);
    for (i = 0; i < 16; i++)
    {
        spidat[i] = i;
    }

    while (1)
    {
#if 1
        SPI_CS_L(SPI3);
        SPI_MasterSendData(SPI3, spidat, 16); //output
        SPI_CS_H(SPI3);
#endif
        SPI_ConfigGpio(SPI3,SPI_MOSI,GPIO_INPUT);
        SPI_CS_L(SPI3);
        SPI_MasterReceiveData(SPI3, spidat1, 16); //input
        SPI_CS_H(SPI3);
        printf("Recv dat:\r\n");
        for (i = 0; i < 16; i++)
        {
            printf("%02x ", spidat1[i]);
        }
        printf("\r\n");
        
        SPI_ConfigGpio(SPI3,SPI_MOSI,GPIO_OUTPUT);

        spidat[0]++;
    }
}

void SPI_Demo(void)
{
    SPI_TypeDef *spi_type_id;

    spi_type_id = SPI1;

    //SPI CPU 查询方式，半双工
    Demo_SPI_CPUNormalTransfer(spi_type_id);

    //SPI DMA方式，全双工模式,栈及全局变量放在0x20000000之后，即IRAM：Start0x20000000,size0x38000
    //	Demo_SPI_DMATransfer(spi_type_id);

    //SPI EDMA方式,全双工和半双工，栈及全局变量放在0x20000000之后，即IRAM：Start0x20000000,size0x38000
    //	Demo_SPI_EDMATransfer(spi_type_id);

    /* SPI 单线双向通信模式 */
    // SPI_Slave_OneLineModeTest();
    // SPI_Master_OneLineModeTest();
}
