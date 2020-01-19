#include "line_sensor_client.h"

#include "error.h"
#include "global_data.h"
#include "interrupt_priorities.h"

#include <stm32\stm32wbxx_ll_crc.h>
#include <stm32\stm32wbxx_ll_dma.h>
#include <stm32\stm32wbxx_ll_spi.h>
#include <stm32\stm32wbxx_ll_tim.h>
#include <stm32\stm32wbxx_ll_exti.h>
#include <stm32\stm32wbxx_ll_gpio.h>

#include <string.h>

#define PIN_POWER      GPIO_PIN_11
#define PIN_POWER_PORT GPIOB

#define PIN_WAKE_UP      GPIO_PIN_2
#define PIN_WAKE_UP_PORT GPIOB

#define PIN_NOT_SLAVE_SELECT      GPIO_PIN_12
#define PIN_NOT_SLAVE_SELECT_PORT GPIOB

#define PIN_SPI1_SCK       GPIO_PIN_5
#define PIN_SPI1_SCK_PORT  GPIOA
#define PIN_SPI1_MISO      GPIO_PIN_6
#define PIN_SPI1_MISO_PORT GPIOA
#define PIN_SPI1_MOSI      GPIO_PIN_7
#define PIN_SPI1_MOSI_PORT GPIOA

static void initializeCrc();
static void initializeSpi();
static void initializeSpiDma();
static void initializeControlPins();
static void initializeTriggerTimer();

static void resetSpiDevice();

static uint32_t calculateCrc(volatile void* pData, uint32_t sizeBytes);
static void checkIfSpiCommunicationWasFinished();

static void sendLineSensorCommand();

static inline void setSlaveSelected(bool isSelected)
{
    if (isSelected)
    {
        LL_GPIO_ResetOutputPin(PIN_NOT_SLAVE_SELECT_PORT, PIN_NOT_SLAVE_SELECT);
    }
    else
    {
        LL_GPIO_SetOutputPin(PIN_NOT_SLAVE_SELECT_PORT, PIN_NOT_SLAVE_SELECT);
    }
}

static inline void setSlavePower(bool isPowerUp)
{
    if (isPowerUp)
    {
        LL_GPIO_SetOutputPin(PIN_POWER_PORT, PIN_POWER);
    }
    else
    {
        LL_GPIO_ResetOutputPin(PIN_POWER_PORT, PIN_POWER);
    }
}

void initializeLineSensor()
{
    g_lcsRestart = false;
    g_lscIsActive = false;
    g_lscIsReceiving = false;
    g_lscIsTransmitting = false;

    initializeCrc();
    initializeTriggerTimer();
    initializeControlPins();
    initializeSpi();
    initializeSpiDma();

    setSlavePower(true);

    // TODO
    startQueryingLineSensor();
    // TODO
}

void startQueryingLineSensor()
{
    if (g_lscIsActive)
    {
        return;
    }

    g_lscIsActive = true;
    g_lscCurrentCommandCode = LSC_GET_SENSOR_VALUES;

    LL_TIM_EnableCounter(TIM2);
    LL_TIM_GenerateEvent_UPDATE(TIM2);
}

void stopQueryingLineSensor()
{
    g_lscIsActive = false;
    g_lscCurrentCommandCode = LSC_NONE;

    LL_TIM_DisableCounter(TIM2);
}

void sendLineSensorCommand()
{
    if (!g_lscIsActive || g_lscIsReceiving || g_lscIsTransmitting)
    {
        return;
    }

    uint32_t lscDataLength; // tx and rx data length is the same
    const lineSensorCommandCode_t lscCurrentCommandCode = g_lscCurrentCommandCode;

    if (lscCurrentCommandCode == LSC_GET_SENSOR_VALUES)
    {
        g_lscTxBuffer.getSensorValues.encodedCommandCode = encodeCommand(LSC_GET_SENSOR_VALUES);
        lscDataLength = sizeof(lineSensorRequestGetSensorValues_t) / sizeof(uint16_t);
    }
    else if (lscCurrentCommandCode == LSC_GET_DETAILED_SENSOR_STATUS)
    {
        g_lscTxBuffer.getDetailedSensorStatus.encodedCommandCode = encodeCommand(LSC_GET_DETAILED_SENSOR_STATUS);
        lscDataLength = sizeof(lineSensorRequestGetDetailedSensorStatus_t) / sizeof(uint16_t);
    }
    else
    {
        return;
    }

    g_lscIsReceiving = true;
    g_lscIsTransmitting = true;

    setSlaveSelected(true);

    // RX

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_1,
                           LL_SPI_DMA_GetRegAddr(SPI1),
                           (uint32_t) &g_lscRxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, lscDataLength);

    // TX

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_2,
                           (uint32_t) &g_lscTxBuffer,
                           LL_SPI_DMA_GetRegAddr(SPI1),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, lscDataLength);

    // enable rx 1st
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    // enable tx 2nd
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}

void initializeCrc()
{
    LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_NONE);
    LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_NONE);
    LL_CRC_SetPolynomialCoef(CRC, LL_CRC_DEFAULT_CRC32_POLY);
    LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_32B);
    LL_CRC_SetInitialData(CRC, LL_CRC_DEFAULT_CRC_INITVALUE);

    LL_CRC_SetPolynomialCoef(CRC, LINE_SENSOR_CRC_POLYNOMIAL);
    LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_32B);
}

void initializeTriggerTimer()
{
    NVIC_SetPriority(TIM2_IRQn, LINE_SENSOR_CLIENT_SCHEDULER_TIMER);
    NVIC_EnableIRQ(TIM2_IRQn);

    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_SetPrescaler(TIM2, __LL_TIM_CALC_PSC(SystemCoreClock, 1000000));
    LL_TIM_SetAutoReload(TIM2, 1000);
}

void initializeControlPins()
{
    LL_GPIO_SetPinMode(PIN_WAKE_UP_PORT, PIN_WAKE_UP, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(PIN_WAKE_UP_PORT, PIN_WAKE_UP, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinMode(PIN_POWER_PORT, PIN_POWER, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(PIN_POWER_PORT, PIN_POWER, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinMode(PIN_NOT_SLAVE_SELECT_PORT, PIN_NOT_SLAVE_SELECT, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(PIN_NOT_SLAVE_SELECT_PORT, PIN_NOT_SLAVE_SELECT, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(PIN_NOT_SLAVE_SELECT_PORT, PIN_NOT_SLAVE_SELECT, LL_GPIO_SPEED_FREQ_HIGH);
}

void initializeSpi()
{
    LL_GPIO_SetPinMode(PIN_SPI1_SCK_PORT, PIN_SPI1_SCK, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(PIN_SPI1_SCK_PORT, PIN_SPI1_SCK, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(PIN_SPI1_SCK_PORT, PIN_SPI1_SCK, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetAFPin_0_7(PIN_SPI1_SCK_PORT, PIN_SPI1_SCK, LL_GPIO_AF_5);
    LL_GPIO_SetPinSpeed(PIN_SPI1_SCK_PORT, PIN_SPI1_SCK, LL_GPIO_SPEED_FREQ_HIGH);

    LL_GPIO_SetPinMode(PIN_SPI1_MISO_PORT, PIN_SPI1_MISO, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(PIN_SPI1_MISO_PORT, PIN_SPI1_MISO, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(PIN_SPI1_MISO_PORT, PIN_SPI1_MISO, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetAFPin_0_7(PIN_SPI1_MISO_PORT, PIN_SPI1_MISO, LL_GPIO_AF_5);
    LL_GPIO_SetPinSpeed(PIN_SPI1_MISO_PORT, PIN_SPI1_MISO, LL_GPIO_SPEED_FREQ_HIGH);

    LL_GPIO_SetPinMode(PIN_SPI1_MOSI_PORT, PIN_SPI1_MOSI, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(PIN_SPI1_MOSI_PORT, PIN_SPI1_MOSI, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(PIN_SPI1_MOSI_PORT, PIN_SPI1_MOSI, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetAFPin_0_7(PIN_SPI1_MOSI_PORT, PIN_SPI1_MOSI, LL_GPIO_AF_5);
    LL_GPIO_SetPinSpeed(PIN_SPI1_MOSI_PORT, PIN_SPI1_MOSI, LL_GPIO_SPEED_FREQ_HIGH);

    // max speed for slave is 2 MBits/s
    LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV32);
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);
    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_16BIT);
    LL_SPI_SetNSSMode(SPI1, LL_SPI_NSS_SOFT);
    LL_SPI_SetTransferBitOrder(SPI1, LL_SPI_LSB_FIRST);
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
}

void initializeSpiDma()
{
    // RX

    NVIC_SetPriority(DMA1_Channel1_IRQn, LINE_SENSOR_CLIENT_SPI_RX_DMA);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_1,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_HALFWORD        |
                          LL_DMA_MDATAALIGN_HALFWORD);

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_SPI1_RX);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);

    // TX

    NVIC_SetPriority(DMA1_Channel2_IRQn, LINE_SENSOR_CLIENT_SPI_TX_DMA);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_2,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_HALFWORD        |
                          LL_DMA_MDATAALIGN_HALFWORD);

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMAMUX_REQ_SPI1_TX);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    // enabled device

    LL_SPI_Enable(SPI1);

    LL_SPI_EnableDMAReq_RX(SPI1);
    LL_SPI_EnableDMAReq_TX(SPI1);
}

void resetSpiDevice()
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

    while (LL_SPI_GetTxFIFOLevel(SPI1) != LL_SPI_TX_FIFO_EMPTY) ;
    while (LL_SPI_IsActiveFlag_BSY(SPI1)) ;

    LL_SPI_Disable(SPI1);

    while (LL_SPI_GetRxFIFOLevel(SPI1) != LL_SPI_RX_FIFO_EMPTY)
    {
        LL_SPI_ReceiveData16(SPI1);
    }

    initializeSpi();
    initializeSpiDma();

    g_lcsRestart = false;
}

/*
 * RX
 */
void DMA1_Channel1_IRQHandler()
{
    if (LL_DMA_IsActiveFlag_TE1(DMA1))
    {
        LL_DMA_ClearFlag_TE1(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
        g_lcsRestart = true;
        g_lscIsReceiving = false;
        checkIfSpiCommunicationWasFinished();
    }
    else if (LL_DMA_IsActiveFlag_TC1(DMA1))
    {
        LL_DMA_ClearFlag_TC1(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
        g_lscIsReceiving = false;
        checkIfSpiCommunicationWasFinished();
    }
}

/*
 * TX
 */
void DMA1_Channel2_IRQHandler()
{
    if (LL_DMA_IsActiveFlag_TE2(DMA1))
    {
        LL_DMA_ClearFlag_TE2(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
        g_lcsRestart = true;
        g_lscIsTransmitting = false;
        checkIfSpiCommunicationWasFinished();
    }
    else if (LL_DMA_IsActiveFlag_TC2(DMA1))
    {
        LL_DMA_ClearFlag_TC2(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
        g_lscIsTransmitting = false;
        checkIfSpiCommunicationWasFinished();
    }
}

void TIM2_IRQHandler()
{
    if (LL_TIM_IsActiveFlag_CC1(TIM2) == 1)
    {
        LL_TIM_ClearFlag_CC1(TIM2);
        sendLineSensorCommand();
    }
}

uint32_t calculateCrc(volatile void* pData, uint32_t sizeBytes)
{
    if (sizeBytes == 0 || (sizeBytes & 1) == 1)
    {
        return false;
    }

    LL_CRC_ResetCRCCalculationUnit(CRC);

    const uint32_t sizeWords = sizeBytes >> 2;
    volatile const uint32_t* const pData32 = (volatile const uint32_t* const) pData;

    for (uint32_t i = 0; i < sizeWords; ++i)
    {
        LL_CRC_FeedData32(CRC, pData32[i]);
    }

    const uint32_t sizeHaldWord = sizeBytes >> 1;
    volatile const uint16_t* const pData16 = (volatile const uint16_t* const) pData;

    if ((sizeHaldWord & 1) == 1)
    {
        LL_CRC_FeedData32(CRC, pData16[sizeHaldWord - 1]);
    }

    return LL_CRC_ReadData32(CRC);
}

void checkIfSpiCommunicationWasFinished()
{
    if (!g_lscIsReceiving && !g_lscIsTransmitting)
    {
        if (g_lcsRestart)
        {
            resetSpiDevice();
            g_lscCurrentCommandCode = LSC_GET_SENSOR_VALUES;
            return;
        }

        setSlaveSelected(false);

        const lineSensorCommandCode_t lscCurrentCommandCode = g_lscCurrentCommandCode;
        g_lscCurrentCommandCode = LSC_NONE;

        if (lscCurrentCommandCode == LSC_GET_SENSOR_VALUES)
        {
            if (calculateCrc(&g_lscRxBuffer.getSensorValues.respondingToCommandCode, LINE_SENSOR_RESPONSE_GET_SENSOR_VALUES_CRC_PROTECTED_DATA_LENGTH_BYTES) != g_lscRxBuffer.getSensorValues.crc)
            {
                // TODO send error to client
                g_lscCurrentCommandCode = LSC_GET_SENSOR_VALUES;
            }
            else if (g_lscRxBuffer.getSensorValues.currentStatus & LSS_ERROR)
            {
                // TODO send error to client
                g_lscCurrentCommandCode = LSC_GET_DETAILED_SENSOR_STATUS;
            }
            else
            {
                // TODO send sensor values to client
                g_lscCurrentCommandCode = LSC_GET_SENSOR_VALUES;
            }
        }
        else if (lscCurrentCommandCode == LSC_GET_DETAILED_SENSOR_STATUS)
        {
            if (calculateCrc(&g_lscRxBuffer.getDetailedSensorStatus.respondingToCommandCode, LINE_SENSOR_RESPONSE_GET_DETAILED_SENSOR_STATUS_CRC_PROTECTED_DATA_LENGTH_BYTES) != g_lscRxBuffer.getDetailedSensorStatus.crc)
            {
                // TODO send error to client
                g_lscCurrentCommandCode = LSC_GET_DETAILED_SENSOR_STATUS;
            }
            else
            {
                // TODO send detailed errors to client
                g_lscCurrentCommandCode = LSC_GET_SENSOR_VALUES;
            }
        }
        else
        {
            g_lscCurrentCommandCode = LSC_GET_SENSOR_VALUES;
        }
    }
}
