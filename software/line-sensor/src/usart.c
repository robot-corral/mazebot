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
    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn);

    LL_USART_EnableIT_IDLE(USART2);
    LL_USART_EnableIT_ERROR(USART2);

    LL_USART_SetTransferDirection(USART2, LL_USART_DIRECTION_TX_RX);
    LL_USART_ConfigCharacter(USART2, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART2, SystemCoreClock, LL_USART_OVERSAMPLING_8, 1000000);
    LL_USART_Enable(USART2);

    LL_USART_EnableDMAReq_RX(USART2);
    LL_USART_EnableDMAReq_TX(USART2);
}

void DMA1_Channel6_IRQHandler(void)
{
    // USART2_RX

    if (LL_DMA_IsActiveFlag_TC2(DMA1))
    {
        LL_DMA_ClearFlag_TC2(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE2(DMA1))
    {
        // TODO handle error
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    // USART_TX

    if (LL_DMA_IsActiveFlag_TC3(DMA1))
    {
        LL_DMA_ClearFlag_TC3(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE3(DMA1))
    {
        // TODO handle error
    }
}

void USART2_IRQHandler()
{
    if (LL_USART_IsActiveFlag_IDLE(USART2))
    {
        LL_USART_ClearFlag_IDLE(USART2);
    }
    // TODO handle error
}
