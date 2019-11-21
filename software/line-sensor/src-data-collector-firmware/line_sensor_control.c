#include "line_sensor_control.h"
#include "line_sensor_control_impl.h"

#include "led.h"
#include "global_data.h"
#include "interrupt_priorities.h"

#include <line_sensor.h>

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_spi.h>
#include <stm32\stm32l4xx_ll_gpio.h>

void initializeLineSensor()
{
    NVIC_SetPriority(SPI3_IRQn, IRQ_PRIORITY_LINE_SENSOR_SPI);
    NVIC_EnableIRQ(SPI3_IRQn);

    LL_SPI_EnableIT_ERR(SPI3);

    LL_SPI_SetBaudRatePrescaler(SPI3, LL_SPI_BAUDRATEPRESCALER_DIV256);
    LL_SPI_SetTransferDirection(SPI3, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPhase(SPI3, LL_SPI_PHASE_2EDGE);
    LL_SPI_SetClockPolarity(SPI3, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetDataWidth(SPI3, LL_SPI_DATAWIDTH_16BIT);
    LL_SPI_SetNSSMode(SPI3, LL_SPI_NSS_SOFT);
    LL_SPI_SetTransferBitOrder(SPI3, LL_SPI_LSB_FIRST);
    LL_SPI_SetStandard(SPI3, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_SetMode(SPI3, LL_SPI_MODE_MASTER);

    LL_SPI_EnableDMAReq_RX(SPI3);
    LL_SPI_EnableDMAReq_TX(SPI3);

    NVIC_SetPriority(DMA2_Channel1_IRQn, IRQ_PRIORITY_LINE_SENSOR_DMI2_CHANNEL1_RX);
    NVIC_EnableIRQ(DMA2_Channel1_IRQn);

    NVIC_SetPriority(DMA2_Channel2_IRQn, IRQ_PRIORITY_LINE_SENSOR_DMI2_CHANNEL2_TX);
    NVIC_EnableIRQ(DMA2_Channel2_IRQn);

    // RX

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_1,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_HALFWORD        |
                          LL_DMA_MDATAALIGN_HALFWORD);

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_1, LL_DMA_REQUEST_3);

    // TX

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_2,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_HALFWORD        |
                          LL_DMA_MDATAALIGN_HALFWORD);

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_2, LL_DMA_REQUEST_3);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_1);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_2);

    LL_SPI_Enable(SPI3);
}

void checkIfSpiCommunicationHasFinished()
{
    if (!g_lineSensorClientIsReceiving && !g_lineSensorClientIsTransmitting)
    {
        while (LL_SPI_GetTxFIFOLevel(SPI3) != LL_SPI_TX_FIFO_EMPTY) ;
        while (LL_SPI_IsActiveFlag_BSY(SPI3) != 0) ;
        while (LL_SPI_GetRxFIFOLevel(SPI3) != LL_SPI_RX_FIFO_EMPTY)
        {
            LL_SPI_ReceiveData16(SPI3);
        }

        setSlaveSelected(false);
    }
}

void SPI3_IRQHandler(void)
{
    if(LL_SPI_IsActiveFlag_CRCERR(SPI3))
    {
        LL_SPI_ClearFlag_CRCERR(SPI3);
        setRedLedEnabled(true);
    }
    if(LL_SPI_IsActiveFlag_MODF(SPI3))
    {
        LL_SPI_ClearFlag_MODF(SPI3);
        setRedLedEnabled(true);
    }
    if(LL_SPI_IsActiveFlag_OVR(SPI3))
    {
        LL_SPI_ClearFlag_OVR(SPI3);
        setRedLedEnabled(true);
    }
    if(LL_SPI_IsActiveFlag_FRE(SPI3))
    {
        LL_SPI_ClearFlag_FRE(SPI3);
        setRedLedEnabled(true);
    }
}

void DMA2_Channel1_IRQHandler(void)
{
    // RX

    if (LL_DMA_IsActiveFlag_TE1(DMA2))
    {
        setRedLedEnabled(true);
    }
    else if (LL_DMA_IsActiveFlag_TC1(DMA2))
    {
        LL_DMA_ClearFlag_TC1(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
        setBlueLedEnabled(false);
        g_lineSensorClientIsReceiving = false;
        checkIfSpiCommunicationHasFinished();
    }
}

void DMA2_Channel2_IRQHandler(void)
{
    // TX

    if (LL_DMA_IsActiveFlag_TE2(DMA2))
    {
        setRedLedEnabled(true);
    }
    else if (LL_DMA_IsActiveFlag_TC2(DMA2))
    {
        LL_DMA_ClearFlag_TC2(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_2);
        setGreenLedEnabled(false);
        g_lineSensorClientIsTransmitting = false;
        checkIfSpiCommunicationHasFinished();
    }
}

void setSlavePowerEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_7);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_7);
    }
}

bool isSlaveSelected()
{
    return !LL_GPIO_IsOutputPinSet(GPIOD, LL_GPIO_PIN_5);
}

void setSlaveSelected(bool isSelected)
{
    if (isSelected)
    {
        LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_5);
    }
    else
    {
        LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_5);
    }
}

void createGetSensorValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_lineSensorClientTxBuffer.getSensorValues.encodedCommandCode = encodeCommand(LSC_GET_SENSOR_VALUES);
    *pRxLength = sizeof(lineSensorResponseGetSensorValues_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestGetSensorValues_t) / sizeof(uint16_t);
}

void createStartCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength, uint8_t parameter)
{
    g_lineSensorClientTxBuffer.startCalibration.encodedCommandCode = encodeCommand(LSC_START_CALIBRATION);
    g_lineSensorClientTxBuffer.startCalibration.encodedCommandParameter = encodeCommandParameter(parameter);
    *pRxLength = sizeof(lineSensorResponseStartCalibration_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestStartCalibration_t) / sizeof(uint16_t);
}

void createGetCalibrationValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_lineSensorClientTxBuffer.getCalibrationValues.encodedCommandCode = encodeCommand(LSC_GET_CALIBRATION_VALUES);
    *pRxLength = sizeof(lineSensorResponseGetCalibrationValues_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestGetCalibrationValues_t) / sizeof(uint16_t);
}

void createFinishCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_lineSensorClientTxBuffer.finishCalibration.encodedCommandCode = encodeCommand(LSC_FINISH_CALIBRATION);
    *pRxLength = sizeof(lineSensorResponseFinishCalibration_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestFinishCalibration_t) / sizeof(uint16_t);
}

void createGetDetailedSensorStatusCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_lineSensorClientTxBuffer.getDetailedSensorStatus.encodedCommandCode = encodeCommand(LSC_GET_DETAILED_SENSOR_STATUS);
    *pRxLength = sizeof(lineSensorResponseGetDetailedSensorStatus_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestGetDetailedSensorStatus_t) / sizeof(uint16_t);
}

void createResetCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_lineSensorClientTxBuffer.reset.encodedCommandCode = encodeCommand(LSC_RESET);
    *pRxLength = sizeof(lineSensorResponseReset_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestReset_t) / sizeof(uint16_t);
}

void sendCommand()
{
    /*
     * TODO might need to fix that to avoid race conditions
     */
    if (g_lineSensorClientIsReceiving ||  g_lineSensorClientIsTransmitting || isSlaveSelected())
    {
        // to avoid jitter / double-press
        return;
    }

    g_lineSensorClientIsReceiving = true;
    g_lineSensorClientIsTransmitting = true;
    /*
     * TODO END: might need to fix that to avoid race conditions
     */

    setRedLedEnabled(false);

    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_2);

    uint16_t rxLength = 0;
    uint16_t txLength = 0;

    createGetSensorValuesCommand(&rxLength, &txLength);
    // createStartCalibrationCommand(&rxLength, &txLength, LSR_SC_P_WHITE_CALIBRATION);
    // createGetCalibrationValuesCommand(&rxLength, &txLength);
    // createFinishCalibrationCommand(&rxLength, &txLength);
    // createResetCommand(&rxLength, &txLength);
    // createGetDetailedSensorStatusCommand(&rxLength, &txLength);

    // RX

    setBlueLedEnabled(true);
    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_1,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           (uint32_t) &g_lineSensorClientRxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, rxLength);

    // TX

    setGreenLedEnabled(true);
    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_2,
                           (uint32_t) &g_lineSensorClientTxBuffer,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, txLength);

    setSlaveSelected(true);

    // enable rx 1st
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);
    // enable tx 2nd
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_2);
}
