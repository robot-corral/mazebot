#include "client_communication_uart.h"
#include "client_communication_uart_impl.h"

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_pwr.h>
#include <stm32\stm32l4xx_ll_rcc.h>
#include <stm32\stm32l4xx_ll_usart.h>

#include "led.h"
#include "global_data.h"
#include "interrupt_priorities.h"

void initializeUart()
{
    // usart2
    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);

    LL_USART_SetTransferDirection(USART2, LL_USART_DIRECTION_TX_RX);
    LL_USART_ConfigCharacter(USART2, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART2, SystemCoreClock, LL_USART_OVERSAMPLING_16, 230400); 
    LL_USART_ConfigMultiProcessMode(USART2);
    LL_USART_Enable(USART2);

    while (!LL_USART_IsActiveFlag_TEACK(USART2) || !LL_USART_IsActiveFlag_REACK(USART2)) ;

    // dma

    // tx

    NVIC_SetPriority(DMA1_Channel7_IRQn, IRQ_PRIORITY_CLIENT_TX);
    NVIC_EnableIRQ(DMA1_Channel7_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_7,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_BYTE            |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_7, LL_DMA_REQUEST_2);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_7);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_7);

    LL_USART_EnableDMAReq_TX(USART2);

    // rx

    NVIC_SetPriority(DMA1_Channel6_IRQn, IRQ_PRIORITY_CLIENT_RX);
    NVIC_EnableIRQ(DMA1_Channel6_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_6,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_BYTE            |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_6,
                           LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_RECEIVE),
                           (uint32_t) &g_clientUartRxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_6, CLIENT_UART_RX_BUFFER_LENGTH);
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_6, LL_DMA_REQUEST_2);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_6);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_6);

    LL_USART_EnableDMAReq_RX(USART2);

    while (LL_USART_IsActiveFlag_RXNE(USART3))
    {
        LL_USART_ReceiveData8(USART2);
    }

    // ready to receive
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_6);
}

static void resetUart()
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_6);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_7);

    LL_USART_DisableDMAReq_RX(USART2);
    LL_USART_DisableDMAReq_TX(USART2);

    LL_USART_Disable(USART2);

    initializeUart();
}

/*
 * RX
 */
void DMA1_Channel6_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE6(DMA1))
    {
        LL_DMA_ClearFlag_TE6(DMA1);
        setRxErrorLedEnabled(true);
        resetUart();
    }
    else if (LL_DMA_IsActiveFlag_TC6(DMA1))
    {
        LL_DMA_ClearFlag_TC6(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_6);
        setRxActiveLedEnabled(false);
        setRxErrorLedEnabled(false);
        processCommand(&g_clientUartRxBuffer);
        // transmit results
        LL_DMA_ConfigAddresses(DMA1,
                               LL_DMA_CHANNEL_7,
                               (uint32_t) &g_clientUartTxBuffer,
                               LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_TRANSMIT),
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_7, CLIENT_UART_TX_BUFFER_LENGTH);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_7);
        setTxActiveLedEnabled(true);
    }
}

/*
 * TX
 */
void DMA1_Channel7_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE7(DMA1))
    {
        LL_DMA_ClearFlag_TE7(DMA1);
        setTxErrorLedEnabled(true);
        resetUart();
    }
    else if (LL_DMA_IsActiveFlag_TC7(DMA1))
    {
        LL_DMA_ClearFlag_TC7(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_7);
        setTxActiveLedEnabled(false);
        setTxErrorLedEnabled(false);
        // get ready to receive next command
        LL_DMA_ConfigAddresses(DMA1,
                               LL_DMA_CHANNEL_6,
                               LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_RECEIVE),
                               (uint32_t) &g_clientUartRxBuffer,
                               LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_6, CLIENT_UART_RX_BUFFER_LENGTH);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_6);
        setRxActiveLedEnabled(true);
    }
}
