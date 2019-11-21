#include "client_communication_usart.h"

#include <stm32\stm32l4xx_ll_pwr.h>
#include <stm32\stm32l4xx_ll_rcc.h>
#include <stm32\stm32l4xx_ll_lpuart.h>

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

void initializeClientCommunicationUsart()
{
    // this part is for NUCLEO-L496ZG

    enableVddIo2();

    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_HSI);

    LL_LPUART_SetTransferDirection(LPUART1, LL_LPUART_DIRECTION_TX_RX);
    LL_LPUART_ConfigCharacter(LPUART1, LL_LPUART_DATAWIDTH_8B, LL_LPUART_PARITY_NONE, LL_LPUART_STOPBITS_1);
    LL_LPUART_SetBaudRate(LPUART1, HSI_VALUE, 230400); 
    LL_LPUART_Enable(LPUART1);

    while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1));
}
