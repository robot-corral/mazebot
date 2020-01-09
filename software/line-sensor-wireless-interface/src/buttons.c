#include "buttons.h"

#include <stm32/stm32wb55xx.h>

#include "stm32wbxx_hal.h"
#include "stm32wbxx_nucleo.h"

void initializeButtons()
{
    BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
    BSP_PB_Init(BUTTON_SW2, BUTTON_MODE_EXTI);
}
