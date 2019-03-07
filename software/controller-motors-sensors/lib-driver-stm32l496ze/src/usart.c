/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "../global_defs.h"

#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_rcc.h"
#include "stm32/stm32l4xx_ll_usart.h"

#ifdef NUCLEO_BOARD
    #include "stm32/stm32l4xx_ll_lpuart.h"

    #define LL_DEBUG_UART_ClearFlag_TC()      LL_LPUART_ClearFlag_TC(LPUART1)
    #define LL_DEBUG_UART_IsActiveFlag_TC()   LL_LPUART_IsActiveFlag_TC(LPUART1)
    #define LL_DEBUG_UART_IsActiveFlag_TXE()  LL_LPUART_IsActiveFlag_TXE(LPUART1)
    #define LL_DEBUG_UART_TransmitData8(data) LL_LPUART_TransmitData8(LPUART1, (data))
#else
    #define LL_DEBUG_UART_ClearFlag_TC()      LL_USART_ClearFlag_TC(USART1)
    #define LL_DEBUG_UART_IsActiveFlag_TC()   LL_USART_IsActiveFlag_TC(USART1)
    #define LL_DEBUG_UART_IsActiveFlag_TXE()  LL_USART_IsActiveFlag_TXE(USART1)
    #define LL_DEBUG_UART_TransmitData8(data) LL_USART_TransmitData8(USART1, (data))
#endif

#include "debug.h"

#include "usart.h"

void initializeUsart()
{
    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);

    // TODO
}

void initializeDebugUsart()
{
#ifdef NUCLEO_BOARD
    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);

    LL_LPUART_SetTransferDirection(LPUART1, LL_LPUART_DIRECTION_TX);
    LL_LPUART_ConfigCharacter(LPUART1, LL_LPUART_DATAWIDTH_8B, LL_LPUART_PARITY_NONE, LL_LPUART_STOPBITS_1);
    LL_LPUART_SetBaudRate(LPUART1, SystemCoreClock, 230400); 

    LL_LPUART_Enable(LPUART1);

    while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1)) ;
#else
    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX);
    LL_USART_ConfigCharacter(USART1, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART1, SystemCoreClock, LL_USART_OVERSAMPLING_16, 230400); 
    LL_USART_Enable(USART1);

    while (!LL_USART_IsActiveFlag_TEACK(USART1)) ;
#endif
}

void transmitDebugBuffer(const char* pBuffer, uint32_t length)
{
    if (length == 0)
    {
        return;
    }

    const uint32_t lengthMinusOne = length - 1;

    for (uint32_t i = 0; i < lengthMinusOne; ++i)
    {
        while (!LL_DEBUG_UART_IsActiveFlag_TXE()) ;
        LL_DEBUG_UART_TransmitData8(pBuffer[i]);
    }

    while (!LL_DEBUG_UART_IsActiveFlag_TXE()) ;
    LL_DEBUG_UART_ClearFlag_TC(); 
    LL_DEBUG_UART_TransmitData8(pBuffer[length - 1]);

    while (!LL_DEBUG_UART_IsActiveFlag_TC()) ;
}
