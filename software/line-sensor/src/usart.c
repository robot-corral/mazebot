/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "usart.h"

#include "global_data.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_usart.h>

void initializeUsart()
{
    NVIC_SetPriority(USART3_IRQn, 0);
    NVIC_EnableIRQ(USART3_IRQn);

    LL_USART_EnableIT_IDLE(USART3);

    LL_USART_SetTransferDirection(USART3, LL_USART_DIRECTION_TX_RX);
    LL_USART_ConfigCharacter(USART3, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART3, SystemCoreClock, LL_USART_OVERSAMPLING_8, 1000000);
    LL_USART_Enable(USART3);

    LL_USART_EnableDMAReq_RX(USART3);
    LL_USART_EnableDMAReq_TX(USART3);
}

void DMA1_Channel2_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC2(DMA1))
    {
        LL_DMA_ClearFlag_TC2(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE2(DMA1))
    {
        for (;;) ;
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC3(DMA1))
    {
        LL_DMA_ClearFlag_TC3(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE3(DMA1))
    {
        for (;;) ;
    }
}

void USART3_IRQHandler()
{
    if (LL_USART_IsActiveFlag_IDLE(USART3))
    {
        LL_USART_ClearFlag_IDLE(USART3);

        // upon reset board sends random byte (can be 0 can be 255).
        // need to figure out how to handle it

        uint32_t remainingBufferLength = LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_3);
        uint32_t numberOfReceivedBytes = RX_BUFFER_LENGTH - remainingBufferLength;

        // TODO process received command

        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
        LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3,
                               LL_USART_DMA_GetRegAddr(USART3),
                               (uint32_t) &g_rxBuffer[0],
                               LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, RX_BUFFER_LENGTH);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
    }
}
