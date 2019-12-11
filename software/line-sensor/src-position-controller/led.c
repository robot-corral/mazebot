#include "led.h"

#include <stm32\stm32l4xx_ll_gpio.h>

void setGreenLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_7);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);
    }
}

void setBlueLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);
    }
}

void setRedLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_14);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_14);
    }
}
