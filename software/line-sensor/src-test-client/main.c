#include "line_sensor.h"

#include "motor_control.h"

#include <stdbool.h>

#include <stm32\stm32l4xx_ll_bus.h>
#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_pwr.h>
#include <stm32\stm32l4xx_ll_spi.h>
#include <stm32\stm32l4xx_ll_rcc.h>
#include <stm32\stm32l4xx_ll_tim.h>
#include <stm32\stm32l4xx_ll_exti.h>
#include <stm32\stm32l4xx_ll_gpio.h>
#include <stm32\stm32l4xx_ll_utils.h>
#include <stm32\stm32l4xx_ll_cortex.h>
#include <stm32\stm32l4xx_ll_lpuart.h>
#include <stm32\stm32l4xx_ll_system.h>

lineSensorRequest_t g_txBuffer;
lineSensorResponse_t g_rxBuffer;

bool g_isReceiving = false;
bool g_isTransmitting = false;
uint32_t g_lastSendTime = 0;

static void sendCommand();

static void createGetSensorValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength);
static void createStartCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength, uint8_t parameter);
static void createGetCalibrationValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength);
static void createFinishCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength);
static void createGetDetailedSensorStatusCommand(uint16_t* pRxLength, uint16_t* pTxLength);
static void createResetCommand(uint16_t* pRxLength, uint16_t* pTxLength);

static void setSlavePowerEnabled(bool isEnabled);

static bool isSlaveSelected();
static void setSlaveSelected(bool isSelected);

void SystemClock_Config()
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR  |
                             LL_APB1_GRP1_PERIPH_TIM5 |
                             LL_APB1_GRP1_PERIPH_TIM2 |
                             LL_APB1_GRP1_PERIPH_SPI3);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2 |
                             LL_AHB1_GRP1_PERIPH_DMA1);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA |
                             LL_AHB2_GRP1_PERIPH_GPIOB |
                             LL_AHB2_GRP1_PERIPH_GPIOC |
                             LL_AHB2_GRP1_PERIPH_GPIOD |
                             LL_AHB2_GRP1_PERIPH_GPIOG);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) ;

    LL_RCC_MSI_Enable();
    while (LL_RCC_MSI_IsReady() != 1) ;

    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
    LL_RCC_MSI_SetCalibTrimming(0);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();

    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1) ;

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) ;

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_Init1msTick(80000000);
    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
    LL_SetSystemCoreClock(80000000);

    LL_TIM_SetPrescaler(TIM5, __LL_TIM_CALC_PSC(SystemCoreClock, 1000000));
    LL_TIM_SetAutoReload(TIM5, 0xFFFFFFFF); // reload 32 bit value (TIM5 is 32 bit)
    LL_TIM_EnableCounter(TIM5);
    LL_TIM_GenerateEvent_UPDATE(TIM5);

    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
}

void initializeLed()
{
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);

    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_14, LL_GPIO_MODE_OUTPUT);

    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_14);
}

void setGreenLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_7);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);
    }
}

void setBlueLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);
    }
}

void setRedLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_14);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_14);
    }
}

void initializeControlPins()
{
    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOD, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOD, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);

    setSlavePowerEnabled(true);
    setSlaveSelected(false);
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

void initializeSpi()
{
    NVIC_SetPriority(SPI3_IRQn, 0);
    NVIC_EnableIRQ(SPI3_IRQn);

    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_10, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_10, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_10, LL_GPIO_AF_6);
    LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_HIGH);

    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_11, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_11, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_11, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_11, LL_GPIO_AF_6);
    LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_11, LL_GPIO_SPEED_FREQ_HIGH);

    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_12, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_12, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_12, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_12, LL_GPIO_AF_6);
    LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_12, LL_GPIO_SPEED_FREQ_HIGH);

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

    NVIC_SetPriority(DMA2_Channel1_IRQn, 1);
    NVIC_EnableIRQ(DMA2_Channel1_IRQn);

    NVIC_SetPriority(DMA2_Channel2_IRQn, 1);
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

void enableVddIo2()
{
    if (LL_PWR_IsEnabledVddIO2())
    {
        return;
    }

    LL_PWR_EnablePVM(LL_PWR_PVM_VDDIO2_0_9V);

    while (LL_PWR_IsActiveFlag_PVMO2());

    do
    {
        LL_PWR_EnableVddIO2();
    } while (!LL_PWR_IsEnabledVddIO2());

    LL_PWR_DisablePVM(LL_PWR_PVM_VDDIO2_0_9V);
}

void initializeUsart()
{
    // this part is for NUCLEO-L496ZG

    enableVddIo2();

    LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(GPIOG, LL_GPIO_PIN_7, LL_GPIO_AF_8);
    LL_GPIO_SetPinSpeed(GPIOG, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOG, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOG, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);

    LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_8_15(GPIOG, LL_GPIO_PIN_8, LL_GPIO_AF_8);
    LL_GPIO_SetPinSpeed(GPIOG, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOG, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOG, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);

    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_HSI);

    LL_LPUART_SetTransferDirection(LPUART1, LL_LPUART_DIRECTION_TX_RX);
    LL_LPUART_ConfigCharacter(LPUART1, LL_LPUART_DATAWIDTH_8B, LL_LPUART_PARITY_NONE, LL_LPUART_STOPBITS_1);
    LL_LPUART_SetBaudRate(LPUART1, HSI_VALUE, 230400); 
    LL_LPUART_Enable(LPUART1);

    while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1));
}

void initializeButton()
{
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 2);

    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_13, LL_GPIO_PULL_NO);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_13);
}

int main()
{
    SystemClock_Config();
    initializeLed();
    initializeControlPins();
    initializeSpi();
    initializeMotorControl();
    initializeUsart();
    initializeButton();

    for (;;) ;
}

void checkIfSpiCommunicationHasFinished()
{
    if (!g_isReceiving && !g_isTransmitting)
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
        g_isReceiving = false;
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
        g_isTransmitting = false;
        checkIfSpiCommunicationHasFinished();
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

void EXTI15_10_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        sendCommand();
    }
}

void sendCommand()
{
    uint32_t currentTime = LL_TIM_GetCounter(TIM5);

    if (g_isReceiving ||  g_isTransmitting || isSlaveSelected() || g_lastSendTime + 500000 > currentTime)
    {
        // to avoid jitter / double-press
        return;
    }

    g_isReceiving = true;
    g_isTransmitting = true;
    g_lastSendTime = currentTime;

    setRedLedEnabled(false);

    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_2);

    uint16_t rxLength = 0;
    uint16_t txLength = 0;

    // createGetSensorValuesCommand(&rxLength, &txLength);
    createStartCalibrationCommand(&rxLength, &txLength, LSR_SC_P_WHITE_CALIBRATION);
    // createGetCalibrationValuesCommand(&rxLength, &txLength);
    // createFinishCalibrationCommand(&rxLength, &txLength);
    // createResetCommand(&rxLength, &txLength);
    // createGetDetailedSensorStatusCommand(&rxLength, &txLength);

    // RX

    setBlueLedEnabled(true);
    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_1,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           (uint32_t) &g_rxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, rxLength);

    // TX

    setGreenLedEnabled(true);
    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_2,
                           (uint32_t) &g_txBuffer,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, txLength);

    setSlaveSelected(true);

    // enable rx 1st
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);
    // enable tx 2nd
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_2);
}

void createGetSensorValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_txBuffer.getSensorValues.encodedCommandCode = encodeCommand(LSC_GET_SENSOR_VALUES);
    *pRxLength = sizeof(lineSensorResponseGetSensorValues_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestGetSensorValues_t) / sizeof(uint16_t);
}

void createStartCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength, uint8_t parameter)
{
    g_txBuffer.startCalibration.encodedCommandCode = encodeCommand(LSC_START_CALIBRATION);
    g_txBuffer.startCalibration.encodedCommandParameter = encodeCommandParameter(parameter);
    *pRxLength = sizeof(lineSensorResponseStartCalibration_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestStartCalibration_t) / sizeof(uint16_t);
}

void createGetCalibrationValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_txBuffer.getCalibrationValues.encodedCommandCode = encodeCommand(LSC_GET_CALIBRATION_VALUES);
    *pRxLength = sizeof(lineSensorResponseGetCalibrationValues_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestGetCalibrationValues_t) / sizeof(uint16_t);
}

void createFinishCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_txBuffer.finishCalibration.encodedCommandCode = encodeCommand(LSC_FINISH_CALIBRATION);
    *pRxLength = sizeof(lineSensorResponseFinishCalibration_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestFinishCalibration_t) / sizeof(uint16_t);
}

void createGetDetailedSensorStatusCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_txBuffer.getDetailedSensorStatus.encodedCommandCode = encodeCommand(LSC_GET_DETAILED_SENSOR_STATUS);
    *pRxLength = sizeof(lineSensorResponseGetDetailedSensorStatus_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestGetDetailedSensorStatus_t) / sizeof(uint16_t);
}

void createResetCommand(uint16_t* pRxLength, uint16_t* pTxLength)
{
    g_txBuffer.reset.encodedCommandCode = encodeCommand(LSC_RESET);
    *pRxLength = sizeof(lineSensorResponseReset_t) / sizeof(uint16_t);
    *pTxLength = sizeof(lineSensorRequestReset_t) / sizeof(uint16_t);
}
