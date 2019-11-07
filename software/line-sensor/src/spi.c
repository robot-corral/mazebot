#include "spi.h"

#include "status.h"
#include "watchdog.h"
#include "global_data.h"
#include "interrupt_priorities.h"

#include <stm32/stm32l1xx_ll_crc.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_spi.h>
#include <stm32/stm32l1xx_ll_exti.h>
#include <stm32/stm32l1xx_ll_system.h>

#define DECODE_MASK 0x000F

static void initializeNssPinExti();
static void resetSpiDma();

static uint32_t getFillerReceiveSizeByCommand(lineSensorCommandCode_t command);
static void processCommand(lineSensorCommandCode_t command, lineSensorCommandParameter_t parameter);
static void processEndOfReceivingFillerAndTransmittingResponse();

static lineSensorDetailedStatus_t decodeCommand(uint16_t encodedCommandWithParameter, lineSensorCommandCode_t* pResultCmd, lineSensorCommandParameter_t* pResultParam)
{
    const lineSensorCommandCode_t command1 = encodedCommandWithParameter & DECODE_MASK;
    encodedCommandWithParameter >>= 4;
    const lineSensorCommandCode_t command2 = encodedCommandWithParameter & DECODE_MASK;
    encodedCommandWithParameter >>= 4;
    const lineSensorCommandParameter_t parameter1 = encodedCommandWithParameter & DECODE_MASK;
    encodedCommandWithParameter >>= 4;
    const lineSensorCommandParameter_t parameter2 = encodedCommandWithParameter & DECODE_MASK;
    if (command1 != command2 || parameter1 != parameter2)
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
        if (parameter1 > 3)
        {
            *pResultParam = 0;
            *pResultCmd = LSC_NONE;
            return LSDS_ERR_FLAG_SPI_INVALID_COMMAND_PARAMETER;
        }
    }
    *pResultCmd = command1;
    *pResultParam = parameter1;
    return LSDS_OK;
}

void initializeNssPinExti()
{
    NVIC_EnableIRQ(EXTI0_IRQn); 
    NVIC_SetPriority(EXTI0_IRQn, INTERRUPT_PRIORITY_SPI2_EXTI);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTD, LL_SYSCFG_EXTI_LINE0);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);
}

void initializeSpi()
{
    initializeNssPinExti();

    NVIC_SetPriority(SPI2_IRQn, INTERRUPT_PRIORITY_SPI2_INTERRUPT);
    NVIC_EnableIRQ(SPI2_IRQn);

    SPI2->CR1 = SPI_CR1_DFF      |
                SPI_CR1_LSBFIRST |
                SPI_CR1_CPOL     |
                SPI_CR1_CPHA;

    SPI2->CR2 = SPI_CR2_ERRIE   |
                SPI_CR2_RXDMAEN |
                SPI_CR2_TXDMAEN;
}

void resetSpiDma()
{
    g_spiCurrentCommand = LSC_NONE;
    g_spiState = SPI_S_WAITING_TO_RECEIVE_COMMAND_CODE;

    g_spiReceivingFillerFinished = false;
    g_spiTransmittingResponseFinished = false;

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);

    while (LL_SPI_IsActiveFlag_TXE(SPI2) != 1) ;
    while (LL_SPI_IsActiveFlag_RXNE(SPI2) != 0)
    {
        LL_SPI_ReceiveData16(SPI2);
    }
    while (LL_SPI_IsActiveFlag_BSY(SPI2) != 0) ;

    LL_SPI_EnableIT_RXNE(SPI2);
}

void activateSpi()
{
    LL_SPI_Enable(SPI2);
    resetSpiDma();
}

void SPI2_IRQHandler(void)
{
    if(LL_SPI_IsActiveFlag_MODF(SPI2))
    {
        LL_SPI_ClearFlag_MODF(SPI2);
        setSensorStatusFlags(LSDS_ERR_FLAG_SPI_ERROR_MODE_FAULT);
        g_spiState = SPI_S_ERROR;
    }
    if(LL_SPI_IsActiveFlag_OVR(SPI2))
    {
        LL_SPI_ClearFlag_OVR(SPI2);
        setSensorStatusFlags(LSDS_ERR_FLAG_SPI_ERROR_OVERRUN);
        g_spiState = SPI_S_ERROR;
    }
    if(LL_SPI_IsActiveFlag_FRE(SPI2))
    {
        LL_SPI_ClearFlag_FRE(SPI2);
        setSensorStatusFlags(LSDS_ERR_FLAG_SPI_ERROR_FRAME_FORMAT);
        g_spiState = SPI_S_ERROR;
    }
    if (g_spiState == SPI_S_WAITING_TO_RECEIVE_COMMAND_CODE)
    {
        if (LL_SPI_IsActiveFlag_RXNE(SPI2) != 0)
        {
            LL_SPI_DisableIT_RXNE(SPI2);
            lineSensorCommandCode_t command;
            lineSensorCommandParameter_t parameter;
            const uint16_t encodedCommandWithParameter = LL_SPI_ReceiveData16(SPI2);
            const lineSensorDetailedStatus_t status = decodeCommand(encodedCommandWithParameter, &command, &parameter);
            if (status == LSDS_OK)
            {
                g_spiCurrentCommand = command;
                processCommand(command, parameter);
            }
            else
            {
                g_spiCurrentCommand = LSC_NONE;
                setSensorStatusFlags(status);
                g_spiState = SPI_S_ERROR;
            }
        }
    }
    else
    {
        LL_SPI_DisableIT_RXNE(SPI2);
    }
}

void EXTI0_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        resetSpiDma();
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    // TX

    if (LL_DMA_IsActiveFlag_TE5(DMA1))
    {
        LL_DMA_ClearFlag_TE5(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);
        setSensorStatusFlags(LSDS_ERR_FLAG_SPI_DMA_FAILURE);
        g_spiState = SPI_S_ERROR;
    }
    else if (LL_DMA_IsActiveFlag_TC5(DMA1))
    {
        LL_DMA_ClearFlag_TC5(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);
        switch (g_spiState)
        {
            case SPI_S_RECEIVING_FILLER_AND_TRANSMITTING_RESPONSE:
            {
                g_spiTransmittingResponseFinished = true;
                processEndOfReceivingFillerAndTransmittingResponse();
                break;
            }
            default:
            {
                setSensorStatusFlags(LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE);
                g_spiState = SPI_S_ERROR;
                break;
            }
        }
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    // RX

    if(LL_DMA_IsActiveFlag_TE4(DMA1))
    {
        LL_DMA_ClearFlag_TE4(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
        setSensorStatusFlags(LSDS_ERR_FLAG_SPI_DMA_FAILURE);
        g_spiState = SPI_S_ERROR;
    }
    else if(LL_DMA_IsActiveFlag_TC4(DMA1))
    {
        LL_DMA_ClearFlag_TC4(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
        switch (g_spiState)
        {
            case SPI_S_RECEIVING_FILLER_AND_TRANSMITTING_RESPONSE:
            {
                g_spiReceivingFillerFinished = true;
                processEndOfReceivingFillerAndTransmittingResponse();
                break;
            }
            default:
            {
                setSensorStatusFlags(LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE);
                g_spiState = SPI_S_ERROR;
                break;
            }
        }
    }
}

void processEndOfReceivingFillerAndTransmittingResponse()
{
    if (g_spiReceivingFillerFinished && g_spiTransmittingResponseFinished)
    {
        resetWatchdog(WS_SPI);
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
}

uint32_t getFillerReceiveSizeByCommand(lineSensorCommandCode_t command)
{
    switch (command)
    {
        case LSC_GET_SENSOR_VALUES          : return sizeof(lineSensorResponseGetSensorValues_t) / sizeof(uint16_t);
        case LSC_START_CALIBRATION          : return sizeof(lineSensorResponseStartCalibration_t) / sizeof(uint16_t);
        case LSC_GET_CALIBRATION_VALUES     : return sizeof(lineSensorResponseGetCalibrationValues_t) / sizeof(uint16_t);
        case LSC_FINISH_CALIBRATION         : return sizeof(lineSensorResponseFinishCalibration_t) / sizeof(uint16_t);
        case LSC_GET_DETAILED_SENSOR_STATUS : return sizeof(lineSensorResponseGetDetailedSensorStatus_t) / sizeof(uint16_t);
        case LSC_RESET                      : return sizeof(lineSensorResponseReset_t) / sizeof(uint16_t);
        default                             : return 0;
    }
}

void processCommand(lineSensorCommandCode_t command, lineSensorCommandParameter_t parameter)
{
    g_spiState = SPI_S_RECEIVING_FILLER_AND_TRANSMITTING_RESPONSE;

    const uint32_t spiCurrentCommandFillerReceiveSize = getFillerReceiveSizeByCommand(command);

    if (spiCurrentCommandFillerReceiveSize == 0)
    {
        setSensorStatusFlags(LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE);
        g_spiState = SPI_S_ERROR;
        return;
    }

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_4,
                           LL_SPI_DMA_GetRegAddr(SPI2),
                           (uint32_t) &g_spiRxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, spiCurrentCommandFillerReceiveSize);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);

    volatile void* pData = 0;
    uint32_t txLength = 0;

    LL_CRC_ResetCRCCalculationUnit(CRC);

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
                // make sure to calculate crc using or'ed status
                LL_CRC_FeedData32(CRC, (g_lineSensorValuesBuffers[idx].currentStatus << 8) | LSC_GET_SENSOR_VALUES);
                for (sensorIndex_t sensorIdx = 0; sensorIdx < NUMBER_OF_SENSORS; ++sensorIdx)
                {
                    LL_CRC_FeedData32(CRC, g_lineSensorValuesBuffers[idx].sensorValues[sensorIdx]);
                }
                g_lineSensorValuesBuffers[idx].crc = LL_CRC_ReadData32(CRC);
                pData = &g_lineSensorValuesBuffers[idx];
            }
            else
            {
                // no data was read
                g_spiTxBuffer.getSensorValues.respondingToCommandCode = LSC_GET_SENSOR_VALUES;
                // no new data is available
                g_spiTxBuffer.getSensorValues.currentStatus = status;
                LL_CRC_FeedData32(CRC, (status << 8) | LSC_GET_SENSOR_VALUES);
                g_spiTxBuffer.getSensorValues.crc = LL_CRC_ReadData32(CRC);
                pData = &g_spiTxBuffer;
            }
            txLength = sizeof(lineSensorResponseGetCalibrationValues_t) / sizeof(uint16_t);
            break;
        }
        case LSC_START_CALIBRATION:
        {
            g_adcCalibrationParameter = parameter;
            g_adcState = ADC_S_CALIBRATION_PENDING;
            const lineSensorStatus_t status = getSensorStatus();
            g_spiTxBuffer.startCalibration.respondingToCommandCode = LSC_START_CALIBRATION;
            g_spiTxBuffer.startCalibration.currentStatus = status;
            LL_CRC_FeedData32(CRC, (status << 8) | LSC_START_CALIBRATION);
            g_spiTxBuffer.startCalibration.crc = LL_CRC_ReadData32(CRC);
            txLength = sizeof(lineSensorResponseStartCalibration_t) / sizeof(uint16_t);
            pData = &g_spiTxBuffer;
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
                LL_CRC_FeedData32(CRC, (status << 8) | LSC_GET_CALIBRATION_VALUES);
                for (sensorIndex_t sensorIdx = 0; sensorIdx < NUMBER_OF_SENSORS; ++sensorIdx)
                {
                    LL_CRC_FeedData32(CRC, (g_lineSensorCalibrationValuesBuffers[idx].minSensorCalibrationValues[sensorIdx] << 16) |
                                            g_lineSensorCalibrationValuesBuffers[idx].maxSensorCalibrationValues[sensorIdx]);
                }
                g_lineSensorCalibrationValuesBuffers[idx].crc = LL_CRC_ReadData32(CRC);
                pData = &g_lineSensorCalibrationValuesBuffers[idx];
            }
            else
            {
                // no data was read
                g_spiTxBuffer.getCalibrationValues.respondingToCommandCode = LSC_GET_CALIBRATION_VALUES;
                g_spiTxBuffer.getCalibrationValues.currentStatus = status;
                LL_CRC_FeedData32(CRC, (status << 8) | LSC_GET_CALIBRATION_VALUES);
                g_spiTxBuffer.getCalibrationValues.crc = LL_CRC_ReadData32(CRC);
                pData = &g_spiTxBuffer;
            }
            txLength = sizeof(lineSensorResponseGetCalibrationValues_t) / sizeof(uint16_t);
            break;
        }
        case LSC_FINISH_CALIBRATION:
        {
            g_adcState = ADC_S_SENSING;
            const lineSensorStatus_t status = getSensorStatus();
            g_spiTxBuffer.finishCalibration.respondingToCommandCode = LSC_FINISH_CALIBRATION;
            g_spiTxBuffer.finishCalibration.currentStatus = status;
            LL_CRC_FeedData32(CRC, (status << 8) | LSC_FINISH_CALIBRATION);
            g_spiTxBuffer.finishCalibration.crc = LL_CRC_ReadData32(CRC);
            txLength = sizeof(lineSensorResponseFinishCalibration_t) / sizeof(uint16_t);
            pData = &g_spiTxBuffer;
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
            LL_CRC_FeedData32(CRC, (status << 8) | LSC_GET_DETAILED_SENSOR_STATUS);
            LL_CRC_FeedData32(CRC, detailedStatus);
            LL_CRC_FeedData32(CRC, cumulitiveDetailedStatus);
            g_spiTxBuffer.getDetailedSensorStatus.crc = LL_CRC_ReadData32(CRC);
            txLength = sizeof(lineSensorResponseGetDetailedSensorStatus_t) / sizeof(uint16_t);
            pData = &g_spiTxBuffer;
            break;
        }
        case LSC_RESET:
        {
            const lineSensorStatus_t status = getSensorStatus();
            g_spiTxBuffer.reset.respondingToCommandCode = LSC_RESET;
            g_spiTxBuffer.reset.currentStatus = status;
            LL_CRC_FeedData32(CRC, (status << 8) | LSC_RESET);
            g_spiTxBuffer.reset.crc = LL_CRC_ReadData32(CRC);
            txLength = sizeof(lineSensorResponseReset_t) / sizeof(uint16_t);
            pData = &g_spiTxBuffer;
            break;
        }
    }

    if (spiCurrentCommandFillerReceiveSize == 0)
    {
        setSensorStatusFlags(LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE);
        g_spiState = SPI_S_ERROR;
    }
    else
    {
        LL_DMA_ConfigAddresses(DMA1,
                               LL_DMA_CHANNEL_5,
                               (uint32_t) pData,
                               LL_SPI_DMA_GetRegAddr(SPI2),
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, txLength);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);
    }
}
