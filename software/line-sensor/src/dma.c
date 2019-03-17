/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "dma.h"

#include "global_data.h"

#include <stm32/stm32l1xx_ll_adc.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_usart.h>

static void initializeAdcDma();
static void initializeUsartDma();

void initializeDma()
{
    initializeAdcDma();

#ifdef ACTIVE_COMMUNICATION_I2C
    #error not implemented
#elif defined ACTIVE_COMMUNICATION_SPI
    #error not implemented
#elif defined ACTIVE_COMMUNICATION_USART
    initializeUsartDma();
#endif
}

void initializeAdcDma()
{
    NVIC_SetPriority(DMA1_Channel1_IRQn, 2);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                                                  LL_DMA_MODE_CIRCULAR              |
                                                  LL_DMA_PERIPH_NOINCREMENT         |
                                                  LL_DMA_MEMORY_INCREMENT           |
                                                  LL_DMA_PDATAALIGN_HALFWORD        |
                                                  LL_DMA_MDATAALIGN_HALFWORD        |
                                                  LL_DMA_PRIORITY_HIGH);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
}

void initializeUsartDma()
{
    // RX

    NVIC_SetPriority(DMA1_Channel6_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel6_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_USART2_DMA_CHANNEL_RX,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_PRIORITY_HIGH |
                          LL_DMA_MODE_NORMAL |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_BYTE |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_EnableIT_TE(DMA1, LL_USART2_DMA_CHANNEL_RX);

    // TX

    NVIC_SetPriority(DMA1_Channel7_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel7_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_USART2_DMA_CHANNEL_TX,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH |
                          LL_DMA_MODE_NORMAL |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_BYTE |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_EnableIT_TC(DMA1, LL_USART2_DMA_CHANNEL_TX);
    LL_DMA_EnableIT_TE(DMA1, LL_USART2_DMA_CHANNEL_TX);
}
