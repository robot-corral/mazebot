#include "led.h"

#include <stm32\stm32l4xx_ll_gpio.h>

void setPositionControllerMovingLedEnabled(bool isEnabled)
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

void setFatalErrorLedEnabled(bool isEnabled)
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

void setEmergencyStopLedEnabled(bool isEnabled)
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

void setTxErrorLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_15);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_15);
    }
}

void setTxActiveLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_14);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_14);
    }
}

void setRxErrorLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_11);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_11);
    }
}

void setRxActiveLedEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_10);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_10);
    }
}
