/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "usart.h"

#include "dma.h"
#include "global_data.h"
#include "interrupt_priorities.h"
#include "consumer_producer_buffer.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_usart.h>

static void resetUsartRxBuffer();

static void handleCommand();

static void registerErrorsAndResetUsart(lineSensorStatus_t usartErrors);

static void respondTransmissionError();
static void respondSendSensorDataCommand();
static void respondToCommand(const lineSensorCommand_t* pCommand, uint32_t numberOfReceivedBytes);

void initializeUsart()
{
    NVIC_SetPriority(USART2_IRQn, INTERRUPT_PRIORITY_COMMUNICATION);
    NVIC_EnableIRQ(USART2_IRQn);

    LL_USART_EnableIT_IDLE(USART2);
    LL_USART_EnableIT_ERROR(USART2);

    LL_USART_SetTransferDirection(USART2, LL_USART_DIRECTION_TX_RX);
    LL_USART_ConfigCharacter(USART2, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART2, SystemCoreClock, LL_USART_OVERSAMPLING_16, USART_BAUDRATE);

    resetUsartRxBuffer();

    LL_USART_EnableDMAReq_RX(USART2);
    LL_USART_EnableDMAReq_TX(USART2);

    // USART should be enabled after everything is configured otherwise extra garbage can be transmitted (usually 0xFF)

    LL_USART_Enable(USART2);
}

void DMA1_Channel6_IRQHandler(void)
{
    // USART2_RX

    if (LL_DMA_IsActiveFlag_TE6(DMA1))
    {
        LL_DMA_ClearFlag_TE6(DMA1);
        registerErrorsAndResetUsart(LSS_ERR_FLAG_USART_DMA_FAILURE);
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    // USART_TX

    if (LL_DMA_IsActiveFlag_TC7(DMA1))
    {
        LL_DMA_ClearFlag_TC7(DMA1);
        if (g_txResponseForSensorCommand)
        {
            // we successfully rx/tx command/response so we can clear all errors
            g_communicationDeviceStatus = 0;
        }
        if (g_txResponseForSensorCommand = LSC_SEND_SENSOR_DATA)
        {
            const uint8_t consumerIndex = consumerProducerBufferGetConsumerIndex(&g_txDataBufferIndexes);
            if (consumerIndex < NUMBER_OF_TX_DATA_BUFFERS)
            {
                // as we already sent this data, clear new data flag
                g_txSendSensorDataBuffers[consumerIndex].header.status &= ~LSS_OK_FLAG_NEW_DATA_AVAILABLE;
            }
        }
        g_txResponseForSensorCommand = 0;
        LL_DMA_DisableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
    }
    if (LL_DMA_IsActiveFlag_TE7(DMA1))
    {
        LL_DMA_ClearFlag_TE7(DMA1);
        registerErrorsAndResetUsart(LSS_ERR_FLAG_USART_DMA_FAILURE);
    }
}

void USART2_IRQHandler()
{
    lineSensorStatus_t errors = 0;

    if (LL_USART_IsActiveFlag_FE(USART2))
    {
        LL_USART_ClearFlag_FE(USART2);
        errors = LSS_ERR_FLAG_USART_FRAMING_ERROR;
    }
    if (LL_USART_IsActiveFlag_NE(USART2))
    {
        LL_USART_ClearFlag_NE(USART2);
        errors |= LSS_ERR_FLAG_USART_NOISE_ERROR;
    }
    if (LL_USART_IsActiveFlag_ORE(USART2))
    {
        LL_USART_ClearFlag_ORE(USART2);
        errors |= LSS_ERR_FLAG_USART_OVERRUN_ERROR;
    }

    if (errors == 0)
    {
        if (LL_USART_IsActiveFlag_IDLE(USART2))
        {
            LL_USART_ClearFlag_IDLE(USART2);
            handleCommand();
        }
    }
    else
    {
        registerErrorsAndResetUsart(errors);
    }
}

static void resetUsartRxBuffer()
{
    LL_DMA_DisableChannel(DMA1, LL_USART2_DMA_CHANNEL_RX);
    LL_DMA_ConfigAddresses(DMA1,
                           LL_USART2_DMA_CHANNEL_RX,
                           LL_USART_DMA_GetRegAddr(USART2),
                           (uint32_t) g_rxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_RX, RX_BUFFER_LENGTH);
    LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_RX);
}

static void handleCommand()
{
    // only respond to commands if we are not transmitting
    if (!LL_DMA_IsEnabledChannel(DMA1, LL_USART2_DMA_CHANNEL_TX))
    {
        const uint32_t remainingBufferLength = LL_DMA_GetDataLength(DMA1, LL_USART2_DMA_CHANNEL_RX);
        const uint32_t numberOfReceivedBytes = RX_BUFFER_LENGTH - remainingBufferLength;

        if (numberOfReceivedBytes == LSC_LENGTH_SIMPLE_COMMAND)
        {
            const lineSensorCommand_t* const pCommand = (lineSensorCommand_t*) g_rxBuffer;

            if (pCommand->header.prefix != COMMAND_PREFIX)
            {
                respondTransmissionError();
            }
            else
            {
                respondToCommand(pCommand, numberOfReceivedBytes);
            }
        }
        else if (numberOfReceivedBytes > LSC_LENGTH_SIMPLE_COMMAND)
        {
            uint8_t commandStartIndex = 0;
            const lineSensorCommand_t* pCommand = 0;

            for (; commandStartIndex < numberOfReceivedBytes - LSC_LENGTH_SIMPLE_COMMAND; ++commandStartIndex)
            {
                const lineSensorCommand_t* const pTempCommand = (lineSensorCommand_t*) &g_rxBuffer[commandStartIndex];

                if (pTempCommand->header.prefix == COMMAND_PREFIX)
                {
                    pCommand = pTempCommand;
                    break;
                }
            }

            if (pCommand)
            {
                respondToCommand(pCommand, numberOfReceivedBytes + commandStartIndex);
            }
            else
            {
                respondTransmissionError();
            }
        }
        else
        {
            respondTransmissionError();
        }
    }

    resetUsartRxBuffer();
}

static void registerErrorsAndResetUsart(lineSensorStatus_t usartErrors)
{
    g_communicationDeviceStatus |= usartErrors;
    LL_DMA_DisableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
    resetUsartRxBuffer();
}

static void respondToCommand(const lineSensorCommand_t* pCommand, uint32_t numberOfReceivedBytes)
{
    const lineSensorCommandCode_t commandCode = pCommand->header.commandCode;

    g_txResponseForSensorCommand = commandCode;

    if (commandCode == LSC_SEND_SENSOR_DATA)
    {
        respondSendSensorDataCommand();
    }
    else if (commandCode == LSC_RESET)
    {
        NVIC_SystemReset();
    }
    else
    {
        // unknown command
        g_txResponseForSensorCommand = 0;
        // unknown command code
        respondTransmissionError();
    }
}

static void respondTransmissionError()
{
    g_txBuffer.header.status = g_adcStatus | g_watchdogResetStatus | g_communicationDeviceStatus | LSS_ERR_FLAG_TRANSMISSION_ERROR;

    LL_DMA_ConfigAddresses(DMA1,
                            LL_USART2_DMA_CHANNEL_TX,
                            (uint32_t) &g_txBuffer,
                            LL_USART_DMA_GetRegAddr(USART2),
                            LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_TX, LSCR_LENGTH_SIMPLE_COMMAND);
    LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
}

static void respondSendSensorDataCommand()
{
    const uint8_t consumerIndex = consumerProducerBufferMoveConsumerIndexToLastReadIndex(&g_txDataBufferIndexes);

    if (consumerIndex < NUMBER_OF_TX_DATA_BUFFERS)
    {
        g_txSendSensorDataBuffers[consumerIndex].header.status |= g_adcStatus | g_watchdogResetStatus | g_communicationDeviceStatus;

        LL_DMA_ConfigAddresses(DMA1,
                               LL_USART2_DMA_CHANNEL_TX,
                               (uint32_t) &g_txSendSensorDataBuffers[consumerIndex],
                               LL_USART_DMA_GetRegAddr(USART2),
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_TX, LSCR_LENGTH_SEND_SENSOR_DATA);
        LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
    }
    else
    {
        // still no data from ADC

        g_txBuffer.header.status = g_adcStatus | g_watchdogResetStatus | g_communicationDeviceStatus | LSS_ERR_FLAG_NOT_READY;

        LL_DMA_ConfigAddresses(DMA1,
                               LL_USART2_DMA_CHANNEL_TX,
                               (uint32_t) &g_txBuffer,
                               LL_USART_DMA_GetRegAddr(USART2),
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_USART2_DMA_CHANNEL_TX, LSCR_LENGTH_SEND_SENSOR_DATA);
        LL_DMA_EnableChannel(DMA1, LL_USART2_DMA_CHANNEL_TX);
    }
}
