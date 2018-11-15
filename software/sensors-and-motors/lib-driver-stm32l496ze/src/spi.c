#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_dma.h"
#include "stm32/stm32l4xx_ll_spi.h"
#include "stm32/stm32l4xx_ll_gpio.h"
#include "stm32/stm32l4xx_ll_utils.h"

#include "status.h"

#include "spi.h"

typedef struct
{
    uint8_t status;
    int16_t temperature;
    int16_t gyro_pitch;
    int16_t gyro_roll;
    int16_t gyro_yaw;
} __attribute__((__packed__)) ImuSensorData_t;

typedef struct
{
    uint8_t dummy;
    ImuSensorData_t data;
} __attribute__((__packed__)) ImuRawSensorData_t;

#define IMU_SPI_TX_BUFFER_LENGTH 11

static void initializeSpi3();
static void initializeSpi3Dma();
static void activateImu();

volatile ImuRawSensorData_t g_imuRxBuffer;

volatile uint8_t g_imuTxBuffer[IMU_SPI_TX_BUFFER_LENGTH];

void initializeSpi()
{
    initializeSpi3();
    initializeSpi3Dma();
    activateImu();
}

void initializeSpi3()
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
    LL_SPI_SetBaudRatePrescaler(SPI3, LL_SPI_BAUDRATEPRESCALER_DIV8);
    LL_SPI_SetClockPhase(SPI3, LL_SPI_PHASE_2EDGE);
    LL_SPI_SetClockPolarity(SPI3, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetDataWidth(SPI3, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(SPI3, LL_SPI_NSS_SOFT);
    LL_SPI_SetRxFIFOThreshold(SPI3, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_SetMode(SPI3, LL_SPI_MODE_MASTER);
}

void initializeSpi3Dma()
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    // RX

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_1,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_PRIORITY_VERYHIGH |
                          LL_DMA_MODE_NORMAL |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_BYTE |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_1,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           (uint32_t) &g_imuRxBuffer,
                           LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_CHANNEL_1));

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, sizeof(ImuRawSensorData_t));

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_1, LL_DMA_REQUEST_3);

    // TX

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_2,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH |
                          LL_DMA_MODE_NORMAL |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_BYTE |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_2,
                           (uint32_t) &g_imuTxBuffer,
                           LL_SPI_DMA_GetRegAddr(SPI3),
                           LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_CHANNEL_2));

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, IMU_SPI_TX_BUFFER_LENGTH);

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_2, LL_DMA_REQUEST_3);

    // common

    NVIC_SetPriority(DMA2_Channel1_IRQn, 1);
    NVIC_EnableIRQ(DMA2_Channel1_IRQn);

    NVIC_SetPriority(DMA2_Channel2_IRQn, 1);
    NVIC_EnableIRQ(DMA2_Channel2_IRQn);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_2);

    LL_SPI_EnableDMAReq_RX(SPI3);
    LL_SPI_EnableDMAReq_TX(SPI3);
}

void DMA2_Channel1_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC1(DMA2))
    {
        LL_DMA_ClearFlag_GI1(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
        LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_12);
    }
    else if (LL_DMA_IsActiveFlag_TE1(DMA2))
    {
        LL_DMA_ClearFlag_GI1(DMA2);
        setCriticalError(ERROR_DMA);
    }
}

void DMA2_Channel2_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC2(DMA2))
    {
        LL_DMA_ClearFlag_GI2(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_2);
    }
    else if (LL_DMA_IsActiveFlag_TE2(DMA2))
    {
        LL_DMA_ClearFlag_GI2(DMA2);
        setCriticalError(ERROR_DMA);
    }
}

void flushSpi()
{
    while (LL_SPI_GetTxFIFOLevel(SPI3) != LL_SPI_TX_FIFO_EMPTY) ;
    while (LL_SPI_IsActiveFlag_BSY(SPI3)) ;
    while (LL_SPI_GetRxFIFOLevel(SPI3) != LL_SPI_RX_FIFO_EMPTY)
    {
        LL_SPI_ReceiveData8(SPI3);
    }
}

void sendConfigToImu(uint8_t txDataLength)
{
    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_12);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, txDataLength);
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_2);

    while (LL_SPI_IsActiveFlag_BSY(SPI3)) ;

    LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_12);
}

void startReadingImuData()
{
    flushSpi();

    g_imuTxBuffer[0] = 0b10011110;   // STATUS_REG(1Eh): address
    g_imuTxBuffer[1] = 0b00000000;   // STATUS_REG(1Eh): read placeholder
    g_imuTxBuffer[2] = 0b00000000;   // OUT_TEMP_L(20h): read placeholder
    g_imuTxBuffer[3] = 0b00000000;   // OUT_TEMP_h(21h): read placeholder
    g_imuTxBuffer[4] = 0b00000000;   // OUTX_L_G  (22h): read placeholder
    g_imuTxBuffer[5] = 0b00000000;   // OUTX_H_G  (23h): read placeholder
    g_imuTxBuffer[6] = 0b00000000;   // OUTY_L_G  (24h): read placeholder
    g_imuTxBuffer[7] = 0b00000000;   // OUTY_H_G  (25h): read placeholder
    g_imuTxBuffer[8] = 0b00000000;   // OUTZ_L_G  (26h): read placeholder
    g_imuTxBuffer[9] = 0b00000000;   // OUTZ_H_G  (27h): read placeholder

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, 10);
    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, 10);

    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_12);

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_2);
}

void activateImu()
{
    LL_SPI_SetTransferDirection(SPI3, LL_SPI_HALF_DUPLEX_TX);

    LL_SPI_Enable(SPI3);

    LL_mDelay(200); // sleep for 200 milliseonds to wait for device powerup

    g_imuTxBuffer[0]  = 0b00010000; // CTRL1_XL (10h): address
    g_imuTxBuffer[1]  = 0b10000011; // CTRL1_XL (10h): value: +/- 2g, 1.66 kHz, 50Hz filter bandwidth
    g_imuTxBuffer[2]  = 0b10000100; // CTRL2_G  (11h): value: 500 dps, 1.66 kHz
    g_imuTxBuffer[3]  = 0b01000110; // CTRL3_C  (12h): value: BDU=1, IF_INC=1, BLE=1
    g_imuTxBuffer[4]  = 0b10000000; // CTRL4_C  (13h): value: XL_BW_SCAL_ODR=1
    g_imuTxBuffer[5]  = 0b00000000; // CTRL5_C  (14h): value:
    g_imuTxBuffer[6]  = 0b00000000; // CTRL6_C  (15h): value:
    g_imuTxBuffer[7]  = 0b00000000; // CTRL7_G  (16h): value:
    g_imuTxBuffer[8]  = 0b11100001; // CTRL8_XL (17h): value: LPF2_XL_EN=1,LOW_PASS_ON_6D=1, LP Filter cutoff frequency=ODR_XL/400
    g_imuTxBuffer[9]  = 0b00111000; // CTRL9_XL (18h): value: enable = x,y,z
    g_imuTxBuffer[10] = 0b00111100; // CTRL10_C (19h): value: enable = yaw, pitch, roll

    sendConfigToImu(3); // power on
    LL_mDelay(800); // wait for gyroscope / accelerator to stabilize
    sendConfigToImu(11); // configure
    flushSpi();

    LL_SPI_Disable(SPI3);
    LL_SPI_SetTransferDirection(SPI3, LL_SPI_FULL_DUPLEX);
    LL_SPI_Enable(SPI3);
}
