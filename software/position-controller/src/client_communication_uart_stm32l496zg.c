#include "client_communication_uart.h"
#include "client_communication_uart_impl.h"

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_pwr.h>
#include <stm32\stm32l4xx_ll_rcc.h>
#include <stm32\stm32l4xx_ll_lpuart.h>

#include "led.h"
#include "global_data.h"
#include "interrupt_priorities.h"

static void initializeUartImpl();
static void processReceivedBuffer(volatile clientUartRequest_t* pClientRequest);

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

void initializeUart()
{
    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_HSI);
    initializeUartImpl();
}

void initializeUartImpl()
{
    g_clientUartIsTransmitting = false;

    LL_LPUART_SetTransferDirection(LPUART1, LL_LPUART_DIRECTION_TX_RX);
    LL_LPUART_ConfigCharacter(LPUART1, LL_LPUART_DATAWIDTH_8B, LL_LPUART_PARITY_NONE, LL_LPUART_STOPBITS_1);
    LL_LPUART_SetBaudRate(LPUART1, HSI_VALUE, 230400); 
    LL_LPUART_Enable(LPUART1);

    while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1) || !LL_LPUART_IsActiveFlag_REACK(LPUART1));

    // dma

    // tx

    NVIC_SetPriority(DMA2_Channel6_IRQn, IRQ_PRIORITY_CLIENT_TX);
    NVIC_EnableIRQ(DMA2_Channel6_IRQn);

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_6,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_BYTE            |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_6, LL_DMA_REQUEST_4);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_6);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_6);

    LL_LPUART_EnableDMAReq_TX(LPUART1);

    // rx

    NVIC_SetPriority(DMA2_Channel7_IRQn, IRQ_PRIORITY_CLIENT_RX);
    NVIC_EnableIRQ(DMA2_Channel7_IRQn);

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_7,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_PRIORITY_HIGH              |
                          LL_DMA_MODE_CIRCULAR              |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_BYTE            |
                          LL_DMA_MDATAALIGN_BYTE);

    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_7,
                           LL_LPUART_DMA_GetRegAddr(LPUART1, LL_LPUART_DMA_REG_DATA_RECEIVE),
                           (uint32_t) g_clientUartRxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_7, CLIENT_UART_RX_BUFFER_LENGTH);
    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_7, LL_DMA_REQUEST_4);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_7);
    LL_DMA_EnableIT_HT(DMA2, LL_DMA_CHANNEL_7);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_7);

    LL_LPUART_EnableDMAReq_RX(LPUART1);

    while (LL_LPUART_IsActiveFlag_RXNE(LPUART1))
    {
        LL_LPUART_ReceiveData8(LPUART1);
    }

    // ready to receive
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_7);
}

static void resetUart()
{
    // make sure to transmit result 1st

    while (LL_DMA_GetDataLength(DMA2, LL_DMA_CHANNEL_6) > 0) ;

    // make sure transmit buffer is empty

    while (LL_LPUART_IsActiveFlag_TXE(LPUART1) == 0) ;

    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_6);
    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_7);

    LL_LPUART_DisableDMAReq_RX(LPUART1);
    LL_LPUART_DisableDMAReq_TX(LPUART1);

    // receive all the data before disabling

    while (LL_LPUART_IsActiveFlag_RXNE(LPUART1))
    {
        LL_LPUART_ReceiveData8(LPUART1);
    }

    LL_LPUART_Disable(LPUART1);

    // now re-initialize device

    initializeUartImpl();
}

void DMA2_Channel6_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE6(DMA2))
    {
        LL_DMA_ClearFlag_TE6(DMA2);
        resetUart();
    }
    else if (LL_DMA_IsActiveFlag_TC6(DMA2))
    {
        LL_DMA_ClearFlag_TC6(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_6);
        g_clientUartIsTransmitting = false;
    }
}

/*
 * RX
 */
void DMA2_Channel7_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE7(DMA2))
    {
        LL_DMA_ClearFlag_TE7(DMA2);
        resetUart();
    }
    else if (LL_DMA_IsActiveFlag_HT7(DMA2))
    {
        LL_DMA_ClearFlag_HT7(DMA2);
        processReceivedBuffer(&g_clientUartRxBuffer[0]);
    }
    else if (LL_DMA_IsActiveFlag_TC7(DMA2))
    {
        LL_DMA_ClearFlag_TC7(DMA2);
        processReceivedBuffer(&g_clientUartRxBuffer[1]);
    }
}

void processReceivedBuffer(volatile clientUartRequest_t* pClientRequest)
{
    if (!g_clientUartIsTransmitting)
    {
        g_clientUartIsTransmitting = true;

        const bool result = processCommand(pClientRequest);

        // transmit results
        LL_DMA_ConfigAddresses(DMA2,
                                LL_DMA_CHANNEL_6,
                                (uint32_t) &g_clientUartTxBuffer,
                                LL_LPUART_DMA_GetRegAddr(LPUART1, LL_LPUART_DMA_REG_DATA_TRANSMIT),
                                LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_6, CLIENT_UART_TX_BUFFER_LENGTH);
        LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_6);

        if (!result)
        {
            resetUart();
        }
    }
    else
    {
        if (!tryToProcessEmergencyStopCommand(pClientRequest))
        {
            resetUart();
        }
    }
}
