/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "usart.h"

#include "dma.h"
#include "global_data.h"
#include "buffer_index.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_usart.h>

static void handleCommand();

void initializeUsart()
{
    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn);

    LL_USART_EnableIT_IDLE(USART2);
    LL_USART_EnableIT_ERROR(USART2);

    LL_USART_SetTransferDirection(USART2, LL_USART_DIRECTION_TX_RX);
    LL_USART_ConfigCharacter(USART2, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART2, SystemCoreClock, LL_USART_OVERSAMPLING_16, USART_BAUDRATE);

    LL_DMA_ConfigAddresses(DMA1,
                           LL_USART2_DMA_CHANNEL_RX,
                           LL_USART_DMA_GetRegAddr(USART2),
                           (uint32_t) &g_rxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_RX, sizeof(lineSensorCommand_t));
    LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_RX);

    LL_USART_EnableDMAReq_RX(USART2);
    LL_USART_EnableDMAReq_TX(USART2);

    LL_USART_Enable(USART2);
}

void DMA1_Channel6_IRQHandler(void)
{
    // USART2_RX

    if (LL_DMA_IsActiveFlag_TE6(DMA1))
    {
        // TODO handle error
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    // USART_TX

    if (LL_DMA_IsActiveFlag_TC7(DMA1))
    {
        LL_DMA_ClearFlag_TC7(DMA1);
        // TODO clear LSS_FLAG_NEW_DATA_AVAILABLE flag on consumer index
        LL_DMA_DisableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
    }
    if (LL_DMA_IsActiveFlag_TE7(DMA1))
    {
        // TODO handle error
    }
}

void USART2_IRQHandler()
{
    if (LL_USART_IsActiveFlag_IDLE(USART2))
    {
        LL_USART_ClearFlag_IDLE(USART2);
        handleCommand();
    }
    if (LL_USART_IsActiveFlag_FE(USART2))
    {
        // TODO handle error
    }
    if (LL_USART_IsActiveFlag_NE(USART2))
    {
        // TODO handle error
    }
    if (LL_USART_IsActiveFlag_ORE(USART2))
    {
        // TODO handle error
    }
}

static void handleCommand()
{
    const uint8_t commandCode = g_rxBuffer.commandCode;
    // reset command in case this function is called again by accident
    g_rxBuffer.commandCode = 0;

    if (commandCode == LSC_SEND_SENSOR_DATA)
    {
        if (!LL_DMA_IsEnabledChannel(DMA1, LL_USART2_DMA_CHANNEL_TX))
        {
            const uint8_t consumerIndex = moveConsumerIndexToLastRead();

            if (consumerIndex == NUMBER_OF_TX_DATA_BUFFERS)
            {
                g_txBuffer.header.status = g_adcStatus | g_watchdogResetStatus | LSS_ERR_NOT_READY;
                g_txBuffer.header.commandCode = LSC_SEND_SENSOR_DATA;

                LL_DMA_ConfigAddresses(DMA1,
                                       LL_USART2_DMA_CHANNEL_TX,
                                       (uint32_t) &g_txBuffer,
                                       LL_USART_DMA_GetRegAddr(USART2),
                                       LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
                LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_TX, LSCR_LENGTH_SEND_SENSOR_DATA);
                LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
            }
            else
            {
                g_txSendSensorDataBuffers[consumerIndex].header.status |= g_adcStatus | g_watchdogResetStatus;
                g_txSendSensorDataBuffers[consumerIndex].header.commandCode = LSC_SEND_SENSOR_DATA;

                LL_DMA_ConfigAddresses(DMA1,
                                       LL_USART2_DMA_CHANNEL_TX,
                                       (uint32_t) &g_txSendSensorDataBuffers[consumerIndex],
                                       LL_USART_DMA_GetRegAddr(USART2),
                                       LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
                LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_TX, LSCR_LENGTH_SEND_SENSOR_DATA);
                LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
            }
        }
    }

    LL_DMA_DisableChannel(DMA1, LL_USART2_DMA_CHANNEL_RX);
    LL_DMA_ConfigAddresses(DMA1,
                           LL_USART2_DMA_CHANNEL_RX,
                           LL_USART_DMA_GetRegAddr(USART2),
                           (uint32_t) &g_rxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_RX, sizeof(lineSensorCommand_t));
    LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_RX);
}
