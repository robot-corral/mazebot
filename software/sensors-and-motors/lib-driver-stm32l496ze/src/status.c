/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "stm32/stm32l4xx_ll_gpio.h"

#include "gpio.h"
#include "driver.h"
#include "status.h"

void setCriticalError(Error_t error)
{
    shutdown();

    switch (error)
    {
        case ERROR_MOTOR: LL_GPIO_SetOutputPin(LED_ERROR_0_GPIO, LED_ERROR_0_PIN); break;
        case ERROR_DMA:   LL_GPIO_SetOutputPin(LED_ERROR_1_GPIO, LED_ERROR_1_PIN); break;
    }

    for (;;) ;
}
