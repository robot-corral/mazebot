#include "spi.h"

#include "status.h"
#include "global_data.h"
#include "interrupt_priorities.h"

#include <stm32/stm32l1xx_ll_crc.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_spi.h>
#include <stm32/stm32l1xx_ll_exti.h>
#include <stm32/stm32l1xx_ll_system.h>

static void initializeNssPinExti();
static void resetSpiDma();

static uint32_t getFillerReceiveSizeByCommand(lineSensorCommandCode_t command);
static void processCommand(lineSensorCommandCode_t command);
static void processEndOfReceivingFillerAndTransmittingResponse();

lineSensorDetailedStatus_t decodeCommand(lineSensorEncodedCommandCode_t encodedCmd, lineSensorCommandCode_t* pResultCmd)
{
    lineSensorCommandCode_t command1 = (lineSensorCommandCode_t) (encodedCmd & COMMAND_1_MASK);
    lineSensorCommandCode_t command2 = (~((lineSensorCommandCode_t) (encodedCmd >> 8)));
    if (command1 != command2)
    {
        *pResultCmd = LSC_NONE;
        return LSDS_ERR_FLAG_SPI_RX_ERROR;
    }
    if (command1 < LSC_FIRST_COMMAND || command1 > LSC_LAST_COMMAND)
    {
        *pResultCmd = LSC_NONE;
        return LSDS_ERR_FLAG_UNKNOWN_COMMAND_REQUESTED;
    }
    else
    {
        *pResultCmd = command1;
        return LSDS_OK;
    }
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

    LL_CRC_ResetCRCCalculationUnit(CRC);

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
            const lineSensorEncodedCommandCode_t encodedCommand = LL_SPI_ReceiveData16(SPI2);
            const lineSensorDetailedStatus_t status = decodeCommand(encodedCommand, &command);
            if (status == LSDS_OK)
            {
                g_spiCurrentCommand = command;
                processCommand(command);
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
        switch (g_spiCurrentCommand)
        {
            case LSC_GET_SENSOR_VALUES:
            {
                // TODO need to clear LSDS_OK_FLAG_NEW_DATA_AVAILABLE flag
                break;
            }
            case LSC_RESET:
            {
                // TODO
                // NVIC_SystemReset();
                return;
            }
        }
        g_spiState = SPI_S_IDLE;
    }
}

uint32_t getFillerReceiveSizeByCommand(lineSensorCommandCode_t command)
{
    switch (command)
    {
        case LSC_GET_SENSOR_VALUES          : return sizeof(lineSensorResponseGetSensorValues_t) / sizeof(uint16_t);
        case LSC_START_PHYSICAL_CALIBRATION : return sizeof(lineSensorResponseStartPhysicalCalibration_t) / sizeof(uint16_t);
        case LSC_GET_CALIBRATION_STATUS     : return sizeof(lineSensorResponseGetCalibrationStatus_t) / sizeof(uint16_t);
        case LSC_FINISH_CALIBRATION         : return sizeof(lineSensorResponseFinishCalibration_t) / sizeof(uint16_t);
        case LSC_GET_CALIBRATION_VALUES     : return sizeof(lineSensorResponseGetCalibrationValues_t) / sizeof(uint16_t);
        case LSC_GET_DETAILED_SENSOR_STATUS : return sizeof(lineSensorResponseGetDetailedSensorStatus_t) / sizeof(uint16_t);
        case LSC_RESET                      : return sizeof(lineSensorResponseReset_t) / sizeof(uint16_t);
        default                             : return 0;
    }
}

void processCommand(lineSensorCommandCode_t command)
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

    uint32_t txLength = 0;

    switch (command)
    {
        case LSC_GET_SENSOR_VALUES          :
        case LSC_START_PHYSICAL_CALIBRATION :
        case LSC_GET_CALIBRATION_STATUS     :
        case LSC_FINISH_CALIBRATION         :
        case LSC_GET_CALIBRATION_VALUES     :
        {
            // TODO
            break;
        }
        case LSC_GET_DETAILED_SENSOR_STATUS :
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
            break;
        }
        case LSC_RESET:
        {
            const lineSensorStatus_t status = getSensorStatus();
            g_spiTxBuffer.reset.respondingToCommandCode = LSC_RESET;
            g_spiTxBuffer.reset.currentStatus = status;
            LL_CRC_FeedData32(CRC, status | LSC_RESET);
            g_spiTxBuffer.reset.crc = LL_CRC_ReadData32(CRC);
            txLength = sizeof(lineSensorResponseReset_t) / sizeof(uint16_t);
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
                               (uint32_t) &g_spiTxBuffer,
                               LL_SPI_DMA_GetRegAddr(SPI2),
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, txLength);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);
    }
}
