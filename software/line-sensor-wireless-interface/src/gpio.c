#include "gpio.h"

#include "stm32wbxx_hal.h"

void initializeGpio()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}
