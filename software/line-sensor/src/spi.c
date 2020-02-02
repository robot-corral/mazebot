/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "spi.h"

#include "status.h"
#include "watchdog.h"
#include "global_data.h"
#include "simple_tasks.h"
#include "interrupt_priorities.h"

#include <mcu_arm.h>

#include <stm32/stm32l1xx_ll_crc.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_spi.h>
#include <stm32/stm32l1xx_ll_exti.h>
#include <stm32/stm32l1xx_ll_gpio.h>
#include <stm32/stm32l1xx_ll_system.h>

#define DECODE_MASK ((uint16_t) 0x000F)

#define FILLER_HALF_WORDS_TO_TRANSMIT 2

#define FILLER_HALF_WORD_NORMAL ((uint16_t) 0xFFFF)
#define FILLER_HALF_WORD_ERROR  ((uint16_t) 0xEEEE)

static void initializeNssPinExti();

static void spiStop();
static void spiStart();
static void spiError(lineSensorDetailedStatus_t status);

static inline void processCommand(lineSensorCommandCode_t command, lineSensorCommandParameter_t parameter);
static inline void processEndOfTransmittingResponse();
static inline void fillInCommandData(lineSensorCommandCode_t command, lineSensorCommandParameter_t parameter, volatile void** ppTxData, uint32_t* pTxDataHalfWordLength);
static inline void computeCommandCrc();

static lineSensorDetailedStatus_t decodeCommand(uint16_t encodedCommandWithParameter, lineSensorCommandCode_t* pResultCmd, lineSensorCommandParameter_t* pResultParam)
{
    const lineSensorCommandCode_t command1 = encodedCommandWithParameter & DECODE_MASK;
    encodedCommandWithParameter >>= 4;
    const lineSensorCommandCode_t command2 = ~((encodedCommandWithParameter & DECODE_MASK) | 0xF0);
    if (command1 != command2)
    {
        *pResultParam = 0;
        *pResultCmd = LSC_NONE;
        return LSDS_ERR_FLAG_SPI_RX_ERROR;
    }
    if (command1 < LSC_FIRST_COMMAND || command1 > LSC_LAST_COMMAND)
    {
        *pResultParam = 0;
        *pResultCmd = LSC_NONE;
        return LSDS_ERR_FLAG_SPI_UNKNOWN_COMMAND_REQUESTED;
    }
    if (command1 == LSC_START_CALIBRATION)
    {
        encodedCommandWithParameter >>= 4;
        const lineSensorCommandParameter_t parameter1 = encodedCommandWithParameter & DECODE_MASK;
        encodedCommandWithParameter >>= 4;
        const lineSensorCommandParameter_t parameter2 = ~((encodedCommandWithParameter & DECODE_MASK) | 0xF0);

        if (parameter1 != parameter2)
        {
            *pResultParam = 0;
            *pResultCmd = LSC_NONE;
            return LSDS_ERR_FLAG_SPI_RX_ERROR;
        }
        if (parameter1 > 3)
        {
            *pResultParam = 0;
            *pResultCmd = LSC_NONE;
            return LSDS_ERR_FLAG_SPI_INVALID_COMMAND_PARAMETER;
        }

        *pResultParam = parameter1;
    }
    else
    {
        *pResultParam = 0;
    }
    *pResultCmd = command1;
    return LSDS_OK;
}

void initializeNssPinExti()
{
    NVIC_EnableIRQ(EXTI0_IRQn); 
    NVIC_SetPriority(EXTI0_IRQn, INTERRUPT_PRIORITY_SPI2_EXTI);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTD, LL_SYSCFG_EXTI_LINE0);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_0);
}

void initializeSpi()
{
    initializeNssPinExti();

    NVIC_SetPriority(SPI2_IRQn, INTERRUPT_PRIORITY_SPI2_INTERRUPT);
    NVIC_EnableIRQ(SPI2_IRQn);

    SPI2->CR1 = SPI_CR1_BIDIMODE_BI_DIRECTIONAL_MODE_DISABLED |
                SPI_CR1_BIDIOE_BI_DIRECTIONAL_MODE_OUTPUT_DISABLED |
                SPI_CR1_CRCEN_CRC_DISABLED |
                SPI_CR1_CRCNEX_NEXT_TRANSFER_IS_DATA |
                SPI_CR1_DFF_16BIT_DATA_FRAME |
                SPI_CR1_RXONLY_FULL_DUPLEX |
                SPI_CR1_SSM_SOFTWARE_SLAVE_MANAGEMENT_DISABLED |
                SPI_CR1_SSI_INTERNAL_SLAVE_SELECT_DISABLED |
                SPI_CR1_LSBFIRST_LEAST_SIGNIFICANT_BIT_FIRST |
                SPI_CR1_SPE_DEVICE_DISABLED |
                SPI_CR1_BR_PCLK_DIV_2 |
                SPI_CR1_MSTR_SLAVE_CONFIGURATION |
                SPI_CR1_CPOL_CLOCK_POLARITY_1_WHEN_IDLE |
                SPI_CR1_CPHA_2ND_CLOCK_TRANSITION_1ST_DATA_CAPTURE;

    SPI2->CR2 = SPI_CR2_TXEIE_TX_EMPTY_INTERRUPT_DISABLED |
                SPI_CR2_RXNEIE_RX_NOT_EMPTY_INTERRUPT_DISABLED |
                SPI_CR2_ERRIE_ERROR_INTERRUPT_DISABLED |
                SPI_CR2_FRF_MOTOROLA_MODE |
                SPI_CR2_SSOE_SS_OUTPUT_IS_DISABLED_IN_MASTER_MODE |
                SPI_CR2_TXDMAEN_TX_BUFFER_DMA_ENABLED |
                SPI_CR2_RXDMAEN_RX_BUFFER_DMA_DISABLED;

    g_spiTxData = 0;
    g_spiState = SPI_S_IDLE;
    g_spiTxDataHalfWordLength = 0;
    g_spiCurrentCommand = LSC_NONE;
    g_spiFillerHalfWordsTransmitted = 0;
}

void activateSpi()
{
    LL_SPI_Enable(SPI2);
}

void spiStart()
{
    const uint32_t priMask = getInterruptMask();
    disableInterrupts();

    if (g_spiState != SPI_S_IDLE)
    {
        spiError(LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE);
    }
    else
    {
        g_spiState = SPI_S_WAITING_TO_RECEIVE_COMMAND_CODE;

        // one as we immediately make 1 filler halfword available
        g_spiFillerHalfWordsTransmitted = 1;
        LL_SPI_TransmitData16(SPI2, FILLER_HALF_WORD_NORMAL);

        SPI2->CR2 |= SPI_CR2_TXEIE_TX_EMPTY_INTERRUPT_ENABLED |
                     SPI_CR2_RXNEIE_RX_NOT_EMPTY_INTERRUPT_ENABLED;
    }

    setInterruptMask(priMask);
}

void spiStop()
{
    const uint32_t priMask = getInterruptMask();
    disableInterrupts();

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);

    LL_CRC_ResetCRCCalculationUnit(CRC);

    SPI2->CR2 &= ~(SPI_CR2_TXEIE_TX_EMPTY_INTERRUPT_ENABLED |
                   SPI_CR2_RXNEIE_RX_NOT_EMPTY_INTERRUPT_ENABLED);

    g_spiTxData = 0;
    g_spiState = SPI_S_IDLE;
    g_spiTxDataHalfWordLength = 0;
    g_spiCurrentCommand = LSC_NONE;
    g_spiFillerHalfWordsTransmitted = 0;

    setInterruptMask(priMask);
}

void spiError(lineSensorDetailedStatus_t status)
{
    const uint32_t priMask = getInterruptMask();
    disableInterrupts();

    setSensorStatusFlags(status);

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);

    LL_CRC_ResetCRCCalculationUnit(CRC);

    SPI2->CR2 &= ~(SPI_CR2_TXEIE_TX_EMPTY_INTERRUPT_ENABLED |
                   SPI_CR2_RXNEIE_RX_NOT_EMPTY_INTERRUPT_ENABLED);

    g_spiTxData = 0;
    g_spiState = SPI_S_IDLE;
    g_spiTxDataHalfWordLength = 0;
    g_spiCurrentCommand = LSC_NONE;
    g_spiFillerHalfWordsTransmitted = 0;

    setInterruptMask(priMask);
}

void SPI2_IRQHandler(void)
{
    if (LL_SPI_IsActiveFlag_RXNE(SPI2) != 0)
    {
        if (g_spiState == SPI_S_WAITING_TO_RECEIVE_COMMAND_CODE)
        {
            const uint16_t encodedCommandWithParameter = LL_SPI_ReceiveData16(SPI2);
            LL_SPI_DisableIT_RXNE(SPI2);
            lineSensorCommandCode_t command;
            lineSensorCommandParameter_t parameter;
            const lineSensorDetailedStatus_t status = decodeCommand(encodedCommandWithParameter, &command, &parameter);
            if (status == LSDS_OK)
            {
                g_spiCurrentCommand = command;
                processCommand(command, parameter);
            }
            else
            {
                g_spiCurrentCommand = LSC_NONE;
                spiError(status);
            }
        }
        else
        {
            LL_SPI_DisableIT_RXNE(SPI2);
        }
    }

    if (LL_SPI_IsActiveFlag_TXE(SPI2))
    {
        if (g_spiState == SPI_S_WAITING_TO_TRANSMIT_FILLER)
        {
            if (g_spiFillerHalfWordsTransmitted == FILLER_HALF_WORDS_TO_TRANSMIT)
            {
                g_spiState = SPI_S_TRANSMITTING_RESPONSE;
                LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);
                LL_SPI_DisableIT_TXE(SPI2);
            }
            else if (g_spiFillerHalfWordsTransmitted > FILLER_HALF_WORDS_TO_TRANSMIT)
            {
                LL_SPI_TransmitData16(SPI2, FILLER_HALF_WORD_ERROR);
                LL_SPI_DisableIT_TXE(SPI2);
                spiError(LSDS_ERR_FLAG_SPI_FAILURE_WHILE_TRANSMITTING_FILLER);
            }
            else
            {
                LL_SPI_TransmitData16(SPI2, FILLER_HALF_WORD_NORMAL);
                ++g_spiFillerHalfWordsTransmitted;
            }
        }
        else
        {
            LL_SPI_TransmitData16(SPI2, FILLER_HALF_WORD_NORMAL);
            ++g_spiFillerHalfWordsTransmitted;
        }
    }
}

void EXTI0_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        const bool nssIsHigh = LL_GPIO_IsInputPinSet(GPIOD, LL_GPIO_PIN_0);
        if (!nssIsHigh)
        {
            spiStart();
        }
        else
        {
            spiStop();
        }
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    // TX

    if (LL_DMA_IsActiveFlag_TE5(DMA1))
    {
        LL_DMA_ClearFlag_TE5(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);
        spiError(LSDS_ERR_FLAG_SPI_DMA_FAILURE);
    }
    else if (LL_DMA_IsActiveFlag_TC5(DMA1))
    {
        LL_DMA_ClearFlag_TC5(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);
        switch (g_spiState)
        {
            case SPI_S_TRANSMITTING_RESPONSE:
            {
                processEndOfTransmittingResponse();
                break;
            }
            default:
            {
                spiError(LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE);
                break;
            }
        }
    }
}

void processEndOfTransmittingResponse()
{
    switch (g_spiCurrentCommand)
    {
        case LSC_GET_SENSOR_VALUES:
        {
            // reset new data available in case master queries too fast
            // or there is a problem with ADC
            const uint8_t idx = consumerProducerBuffer_getConsumerIndex(&g_lineSensorValuesBuffersProducerConsumerIndexes);
            if (idx < DATA_BUFFER_LENGTH)
            {
                g_lineSensorValuesBuffers[idx].currentStatus &= ~LSS_OK_FLAG_NEW_DATA_AVAILABLE;
            }
            break;
        }
        case LSC_GET_CALIBRATION_VALUES:
        {
            // reset new data available in case master queries too fast
            // or there is a problem with ADC
            const uint8_t idx = consumerProducerBuffer_getConsumerIndex(&g_lineSensorCalibrationValuesBuffersProducerConsumerIndexes);
            if (idx < DATA_BUFFER_LENGTH)
            {
                g_lineSensorCalibrationValuesBuffers[idx].currentStatus &= ~LSS_OK_FLAG_NEW_DATA_AVAILABLE;
            }
            break;
        }
        case LSC_RESET:
        {
            NVIC_SystemReset();
            return;
        }
    }
    g_spiState = SPI_S_IDLE;
    resetSensorStatusFlags(LSDS_ERR_FLAG_SPI_ALL);
}

void processCommand(lineSensorCommandCode_t command, lineSensorCommandParameter_t parameter)
{
    volatile void* pTxData;
    uint32_t txDataHalfWordLength;

    // it is important to fill in data as quickly as possible
    fillInCommandData(command, parameter, &pTxData, &txDataHalfWordLength);

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_5,
                           (uint32_t) pTxData,
                           LL_SPI_DMA_GetRegAddr(SPI2),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, txDataHalfWordLength);

    g_spiTxData = pTxData;
    g_spiTxDataHalfWordLength = txDataHalfWordLength;

    if (scheduleTask(computeCommandCrc))
    {
        g_spiState = SPI_S_WAITING_TO_TRANSMIT_FILLER;
    }
    else
    {
        spiError(LSDS_ERR_FLAG_SPI_TASK_SCHEDULER_IS_BUSY);
    }
}

void fillInCommandData(lineSensorCommandCode_t command, lineSensorCommandParameter_t parameter, volatile void** ppTxData, uint32_t* pTxDataHalfWordLength)
{
    switch (command)
    {
        case LSC_GET_SENSOR_VALUES:
        {
            const uint8_t idx = consumerProducerBuffer_moveConsumerIndexToLastReadIndex(&g_lineSensorValuesBuffersProducerConsumerIndexes);
            const lineSensorStatus_t status = getSensorStatus();
            if (idx < DATA_BUFFER_LENGTH)
            {
                g_lineSensorValuesBuffers[idx].respondingToCommandCode = LSC_GET_SENSOR_VALUES;
                // or status as we don't want to lose LSS_OK_FLAG_NEW_DATA_AVAILABLE flag
                g_lineSensorValuesBuffers[idx].currentStatus |= status;
                *ppTxData = &g_lineSensorValuesBuffers[idx];
            }
            else
            {
                // no data was read
                g_spiTxBuffer.getSensorValues.respondingToCommandCode = LSC_GET_SENSOR_VALUES;
                g_spiTxBuffer.getSensorValues.currentStatus = status;
                *ppTxData = &g_spiTxBuffer;
            }
            *pTxDataHalfWordLength = sizeof(lineSensorResponseGetSensorValues_t) / sizeof(uint16_t);
            break;
        }
        case LSC_START_CALIBRATION:
        {
            g_adcCalibrationParameter = parameter;
            g_adcState = ADC_S_CALIBRATION_PENDING;
            const lineSensorStatus_t status = getSensorStatus();
            g_spiTxBuffer.startCalibration.respondingToCommandCode = LSC_START_CALIBRATION;
            g_spiTxBuffer.startCalibration.currentStatus = status;
            *ppTxData = &g_spiTxBuffer;
            *pTxDataHalfWordLength = sizeof(lineSensorResponseStartCalibration_t) / sizeof(uint16_t);
            break;
        }
        case LSC_GET_CALIBRATION_VALUES:
        {
            const uint8_t idx = consumerProducerBuffer_moveConsumerIndexToLastReadIndex(&g_lineSensorCalibrationValuesBuffersProducerConsumerIndexes);
            const lineSensorStatus_t status = getSensorStatus();
            if (idx < DATA_BUFFER_LENGTH)
            {
                g_lineSensorCalibrationValuesBuffers[idx].respondingToCommandCode = LSC_GET_CALIBRATION_VALUES;
                g_lineSensorCalibrationValuesBuffers[idx].currentStatus = status;
                *ppTxData = &g_lineSensorCalibrationValuesBuffers[idx];
            }
            else
            {
                // no data was read
                g_spiTxBuffer.getCalibrationValues.respondingToCommandCode = LSC_GET_CALIBRATION_VALUES;
                g_spiTxBuffer.getCalibrationValues.currentStatus = status;
                *ppTxData = &g_spiTxBuffer;
            }
            *pTxDataHalfWordLength = sizeof(lineSensorResponseGetCalibrationValues_t) / sizeof(uint16_t);
            break;
        }
        case LSC_FINISH_CALIBRATION:
        {
            g_adcState = ADC_S_SENSING;
            const lineSensorStatus_t status = getSensorStatus();
            g_spiTxBuffer.finishCalibration.respondingToCommandCode = LSC_FINISH_CALIBRATION;
            g_spiTxBuffer.finishCalibration.currentStatus = status;
            *ppTxData = &g_spiTxBuffer;
            *pTxDataHalfWordLength = sizeof(lineSensorResponseFinishCalibration_t) / sizeof(uint16_t);
            break;
        }
        case LSC_GET_DETAILED_SENSOR_STATUS:
        {
            const lineSensorDetailedStatus_t detailedStatus = getDetailedSensorStatus();
            const lineSensorDetailedStatus_t cumulitiveDetailedStatus = getCumulitiveDetailedSensorStatus();
            const lineSensorStatus_t status = getSensorStatusFromDetailedStatus(detailedStatus);
            g_spiTxBuffer.getDetailedSensorStatus.respondingToCommandCode = LSC_GET_DETAILED_SENSOR_STATUS;
            g_spiTxBuffer.getDetailedSensorStatus.currentStatus = status;
            g_spiTxBuffer.getDetailedSensorStatus.currentDetailedStatus = detailedStatus;
            g_spiTxBuffer.getDetailedSensorStatus.cumulativeDetailedStatusSinceLastReset = cumulitiveDetailedStatus;
            *ppTxData = &g_spiTxBuffer;
            *pTxDataHalfWordLength = sizeof(lineSensorResponseGetDetailedSensorStatus_t) / sizeof(uint16_t);
            break;
        }
        case LSC_RESET:
        {
            const lineSensorStatus_t status = getSensorStatus();
            g_spiTxBuffer.reset.respondingToCommandCode = LSC_RESET;
            g_spiTxBuffer.reset.currentStatus = status;
            *ppTxData = &g_spiTxBuffer;
            *pTxDataHalfWordLength = sizeof(lineSensorResponseReset_t) / sizeof(uint16_t);
            break;
        }
    }
}

void computeCommandCrc()
{
    // spi crc isn't used as it only supports 8/16 bits
    // it is enabled for both tx and rx and I only need it for tx

    volatile void * const pTxData = g_spiTxData;
    const uint32_t txDataHalfWordLength = g_spiTxDataHalfWordLength - 2 /* remove crc storage */;

    if (txDataHalfWordLength == 0 || pTxData == 0)
    {
        return;
    }

    const uint32_t numberOfWords = txDataHalfWordLength >> 1;
    volatile uint32_t * const pData32 = (uint32_t*) pTxData;

    for (uint32_t i = 0; i < numberOfWords; ++i)
    {
        LL_CRC_FeedData32(CRC, pData32[i]);
    }

    if (txDataHalfWordLength & 1)
    {
        volatile uint16_t * const pData16 = (uint16_t*) pTxData;
        LL_CRC_FeedData32(CRC, pData16[txDataHalfWordLength - 1]);
    }

    switch (g_spiCurrentCommand)
    {
        case LSC_GET_SENSOR_VALUES          : ((lineSensorResponseGetSensorValues_t*) g_spiTxData)->crc = LL_CRC_ReadData32(CRC); break;
        case LSC_START_CALIBRATION          : ((lineSensorResponseStartCalibration_t*) g_spiTxData)->crc = LL_CRC_ReadData32(CRC); break;
        case LSC_GET_CALIBRATION_VALUES     : ((lineSensorResponseGetCalibrationValues_t*) g_spiTxData)->crc = LL_CRC_ReadData32(CRC); break;
        case LSC_FINISH_CALIBRATION         : ((lineSensorResponseFinishCalibration_t*) g_spiTxData)->crc = LL_CRC_ReadData32(CRC); break;
        case LSC_GET_DETAILED_SENSOR_STATUS : ((lineSensorResponseGetDetailedSensorStatus_t*) g_spiTxData)->crc = LL_CRC_ReadData32(CRC); break;
        case LSC_RESET                      : ((lineSensorResponseReset_t*) g_spiTxData)->crc = LL_CRC_ReadData32(CRC); break;
    }
}
