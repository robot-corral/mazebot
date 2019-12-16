#include "uart.h"
#include "error.h"
#include "global_data.h"

void usart1Initialize()
{
    g_uart1.Instance = USART1;
    g_uart1.Init.BaudRate = 115200;
    g_uart1.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart1.Init.StopBits = UART_STOPBITS_1;
    g_uart1.Init.Parity = UART_PARITY_NONE;
    g_uart1.Init.Mode = UART_MODE_TX_RX;
    g_uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart1.Init.OverSampling = UART_OVERSAMPLING_8;
    g_uart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    g_uart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    g_uart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&g_uart1) != HAL_OK)
    {
        fatalError();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&g_uart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        fatalError();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&g_uart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        fatalError();
    }
    if (HAL_UARTEx_DisableFifoMode(&g_uart1) != HAL_OK)
    {
        fatalError();
    }
}
