#include "led.h"

#include <stm32/stm32wb55xx.h>

#include "stm32wbxx_hal.h"
#include "stm32wbxx_nucleo.h"

void initializeLed()
{
    BSP_LED_Init(LED_BLUE);
    BSP_LED_Init(LED_GREEN);
    BSP_LED_Init(LED_RED);

    BSP_LED_On(LED_GREEN);
}
