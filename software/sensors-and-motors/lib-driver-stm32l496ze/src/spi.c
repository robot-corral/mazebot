#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_dma.h"
#include "stm32/stm32l4xx_ll_spi.h"
#include "stm32/stm32l4xx_ll_gpio.h"

#include "status.h"
#include "timing.h"
#include "global_data.h"

#include "spi.h"

static void initializeSpi3();
static void initializeSpi3Dma();

void initializeSpi()
{
    initializeSpi3();
    initializeSpi3Dma();
}

void initializeSpi3()
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
    LL_SPI_SetBaudRatePrescaler(SPI3, LL_SPI_BAUDRATEPRESCALER_DIV8);
    LL_SPI_SetClockPhase(SPI3, LL_SPI_PHASE_2EDGE);
    LL_SPI_SetClockPolarity(SPI3, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetDataWidth(SPI3, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(SPI3, LL_SPI_NSS_SOFT);
    LL_SPI_SetRxFIFOThreshold(SPI3, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_SetMode(SPI3, LL_SPI_MODE_MASTER);
    LL_SPI_SetTransferDirection(SPI3, LL_SPI_FULL_DUPLEX);
    LL_SPI_Enable(SPI3);
}

void initializeSpi3Dma()
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    // RX

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_1,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_PRIORITY_VERYHIGH |
                          LL_DMA_MODE_NORMAL |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_BYTE |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_1, LL_DMA_REQUEST_3);

    // TX

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_2,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH |
                          LL_DMA_MODE_NORMAL |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_BYTE |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_2, LL_DMA_REQUEST_3);

    // common

    NVIC_SetPriority(DMA2_Channel1_IRQn, 1);
    NVIC_EnableIRQ(DMA2_Channel1_IRQn);

    NVIC_SetPriority(DMA2_Channel2_IRQn, 1);
    NVIC_EnableIRQ(DMA2_Channel2_IRQn);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_2);

    LL_SPI_EnableDMAReq_RX(SPI3);
    LL_SPI_EnableDMAReq_TX(SPI3);
}

void flushSpi()
{
    while (LL_SPI_GetTxFIFOLevel(SPI3) != LL_SPI_TX_FIFO_EMPTY) ;
    while (LL_SPI_IsActiveFlag_BSY(SPI3)) ;
    while (LL_SPI_GetRxFIFOLevel(SPI3) != LL_SPI_RX_FIFO_EMPTY)
    {
        LL_SPI_ReceiveData8(SPI3);
    }
}

void setSpiBuffers(volatile void* pRxBuffer, volatile void* pTxBuffer)
{
    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_1,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           (uint32_t) pRxBuffer,
                           LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_CHANNEL_1));

    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_2,
                           (uint32_t) pTxBuffer,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_CHANNEL_2));
}

void startTransmitReceiveSpi(uint32_t bufferLength, GPIO_TypeDef* pGpio, uint32_t pinMask)
{
    setOutput0High();

    g_spiDisableGpio = pGpio;
    g_spiDisablePin = pinMask;

    flushSpi();

    LL_GPIO_ResetOutputPin(pGpio, pinMask);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, bufferLength);
    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, bufferLength);

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_2);
}

bool isSpiBusy()
{
    return LL_SPI_IsActiveFlag_BSY(SPI3);
}

void DMA2_Channel1_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE1(DMA2))
    {
        LL_DMA_ClearFlag_GI1(DMA2);
        setCriticalError(ERROR_DMA);
    }
    if (LL_DMA_IsActiveFlag_TC1(DMA2))
    {
        LL_DMA_ClearFlag_GI1(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
        LL_GPIO_SetOutputPin(g_spiDisableGpio, g_spiDisablePin);
        setOutput0Low();
    }
}

void DMA2_Channel2_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC2(DMA2))
    {
        LL_DMA_ClearFlag_GI2(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_2);
    }
    if (LL_DMA_IsActiveFlag_TE2(DMA2))
    {
        LL_DMA_ClearFlag_GI2(DMA2);
        setCriticalError(ERROR_DMA);
    }
}
