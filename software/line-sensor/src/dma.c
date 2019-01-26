#include "dma.h"

#include "global_data.h"

#include <stm32/stm32l1xx_ll_adc.h>
#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_usart.h>

static void initializeAdcDma();
static void initializeI2cDma();
static void initializeSpiDma();
static void initializeUsartDma();

void initializeDma(communicationInterface_t communicationInterface)
{
    initializeAdcDma();
    switch (communicationInterface)
    {
        case CI_I2C  : initializeI2cDma(); break;
        case CI_SPI  : initializeSpiDma(); break;
        case CI_USART: initializeUsartDma(); break;
    }
}

void initializeAdcDma()
{
    NVIC_SetPriority(DMA1_Channel1_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                                                  LL_DMA_MODE_CIRCULAR              |
                                                  LL_DMA_PERIPH_NOINCREMENT         |
                                                  LL_DMA_MEMORY_INCREMENT           |
                                                  LL_DMA_PDATAALIGN_HALFWORD        |
                                                  LL_DMA_MDATAALIGN_HALFWORD        |
                                                  LL_DMA_PRIORITY_HIGH);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 20);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
}

void initializeI2cDma()
{
    // rx

    NVIC_SetPriority(DMA1_Channel7_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel7_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_7, LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                                                  LL_DMA_PRIORITY_HIGH              |
                                                  LL_DMA_MODE_NORMAL                |
                                                  LL_DMA_PERIPH_NOINCREMENT         |
                                                  LL_DMA_MEMORY_INCREMENT           |
                                                  LL_DMA_PDATAALIGN_BYTE            |
                                                  LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_7);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_7);

    // tx

    NVIC_SetPriority(DMA1_Channel6_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel6_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_6, LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                                                  LL_DMA_PRIORITY_HIGH              |
                                                  LL_DMA_MODE_NORMAL                |
                                                  LL_DMA_PERIPH_NOINCREMENT         |
                                                  LL_DMA_MEMORY_INCREMENT           |
                                                  LL_DMA_PDATAALIGN_BYTE            |
                                                  LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_6);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_6);
}

void initializeSpiDma()
{
    // rx

    NVIC_SetPriority(DMA1_Channel4_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_4, LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                                                  LL_DMA_PRIORITY_HIGH |
                                                  LL_DMA_MODE_NORMAL |
                                                  LL_DMA_PERIPH_NOINCREMENT |
                                                  LL_DMA_MEMORY_INCREMENT |
                                                  LL_DMA_PDATAALIGN_BYTE |
                                                  LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_4);

    // tx

    NVIC_SetPriority(DMA1_Channel5_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_5, LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                                                  LL_DMA_PRIORITY_HIGH |
                                                  LL_DMA_MODE_NORMAL |
                                                  LL_DMA_PERIPH_NOINCREMENT |
                                                  LL_DMA_MEMORY_INCREMENT |
                                                  LL_DMA_PDATAALIGN_BYTE |
                                                  LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_5);
}

void initializeUsartDma()
{
    // rx

    NVIC_SetPriority(DMA1_Channel3_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                                                  LL_DMA_PRIORITY_HIGH |
                                                  LL_DMA_MODE_NORMAL |
                                                  LL_DMA_PERIPH_NOINCREMENT |
                                                  LL_DMA_MEMORY_INCREMENT |
                                                  LL_DMA_PDATAALIGN_BYTE |
                                                  LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3,
                           LL_USART_DMA_GetRegAddr(USART3),
                           (uint32_t) g_rxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, RX_BUFFER_LENGTH);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);

    // tx

    NVIC_SetPriority(DMA1_Channel2_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                                                  LL_DMA_PRIORITY_HIGH |
                                                  LL_DMA_MODE_NORMAL |
                                                  LL_DMA_PERIPH_NOINCREMENT |
                                                  LL_DMA_MEMORY_INCREMENT |
                                                  LL_DMA_PDATAALIGN_BYTE |
                                                  LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,
                           (uint32_t) g_txBuffer,
                           LL_USART_DMA_GetRegAddr(USART3),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, TX_BUFFER_LENGTH);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    LL_USART_EnableDMAReq_RX(USART3);
    LL_USART_EnableDMAReq_TX(USART3);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}

void DMA1_Channel2_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC2(DMA1))
    {
        LL_DMA_ClearFlag_GI2(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE2(DMA1))
    {
        for (;;) ;
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC3(DMA1))
    {
        LL_DMA_ClearFlag_GI3(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE3(DMA1))
    {
        for (;;);
    }
}