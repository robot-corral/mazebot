#include "spi.h"

#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_spi.h>

#include "global_data.h"

void initializeSpi()
{
    LL_SPI_SetBaudRatePrescaler(SPI2, LL_SPI_BAUDRATEPRESCALER_DIV128);
    LL_SPI_SetTransferDirection(SPI2,LL_SPI_FULL_DUPLEX);

    LL_SPI_SetClockPhase(SPI2, LL_SPI_PHASE_2EDGE);
    LL_SPI_SetClockPolarity(SPI2, LL_SPI_POLARITY_HIGH);

    LL_SPI_SetDataWidth(SPI2, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(SPI2, LL_SPI_NSS_HARD_OUTPUT);
    LL_SPI_SetMode(SPI1, LL_SPI_MODE_SLAVE);

    LL_SPI_EnableDMAReq_RX(SPI2);
    LL_SPI_EnableDMAReq_TX(SPI2);
}

void activateSpi()
{
    // RX

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_4,
                           LL_SPI_DMA_GetRegAddr(SPI2),
                           (uint32_t) g_rxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, RX_BUFFER_LENGTH);

    LL_SPI_Enable(SPI2);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
    // LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);
}

void DMA1_Channel4_IRQHandler(void)
{
    if(LL_DMA_IsActiveFlag_TC4(DMA1))
    {
        LL_DMA_ClearFlag_GI4(DMA1);
    }
    else if(LL_DMA_IsActiveFlag_TE4(DMA1))
    {
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC5(DMA1))
    {
        LL_DMA_ClearFlag_GI5(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE5(DMA1))
    {
    }
}
