#include "usart.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_usart.h>

void initializeUsart()
{
    LL_USART_Disable(USART2);

    LL_USART_SetTransferDirection(USART2, LL_USART_DIRECTION_TX_RX);
    LL_USART_ConfigCharacter(USART2, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART2, SystemCoreClock, LL_USART_OVERSAMPLING_16, 115200);

    LL_USART_EnableDMAReq_RX(USART2);
    LL_USART_EnableDMAReq_TX(USART2);
}
