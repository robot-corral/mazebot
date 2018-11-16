/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "stm32/stm32l4xx_ll_gpio.h"

#include "gpio.h"

#include "timing.h"

void setOutput0High()
{
    LL_GPIO_SetOutputPin(TIMING_0_GPIO, TIMING_0_PIN);
}

void setOutput0Low()
{
    LL_GPIO_ResetOutputPin(TIMING_0_GPIO, TIMING_0_PIN);
}

void setOutput1High()
{
    LL_GPIO_SetOutputPin(TIMING_1_GPIO, TIMING_1_PIN);
}

void setOutput1Low()
{
    LL_GPIO_ResetOutputPin(TIMING_1_GPIO, TIMING_1_PIN);
}

void setOutput2High()
{
    LL_GPIO_SetOutputPin(TIMING_2_GPIO, TIMING_2_PIN);
}

void setOutput2Low()
{
    LL_GPIO_ResetOutputPin(TIMING_2_GPIO, TIMING_2_PIN);
}

void setOutput3High()
{
    LL_GPIO_SetOutputPin(TIMING_3_GPIO, TIMING_3_PIN);
}

void setOutput3Low()
{
    LL_GPIO_ResetOutputPin(TIMING_3_GPIO, TIMING_3_PIN);
}

void setOutput4High()
{
    LL_GPIO_SetOutputPin(TIMING_4_GPIO, TIMING_4_PIN);
}

void setOutput4Low()
{
    LL_GPIO_ResetOutputPin(TIMING_4_GPIO, TIMING_4_PIN);
}

void setOutput5High()
{
    LL_GPIO_SetOutputPin(TIMING_5_GPIO, TIMING_5_PIN);
}

void setOutput5Low()
{
    LL_GPIO_ResetOutputPin(TIMING_5_GPIO, TIMING_5_PIN);
}

void setOutput6High()
{
    LL_GPIO_SetOutputPin(TIMING_6_GPIO, TIMING_6_PIN);
}

void setOutput6Low()
{
    LL_GPIO_ResetOutputPin(TIMING_6_GPIO, TIMING_6_PIN);
}

void setOutput7High()
{
    LL_GPIO_SetOutputPin(TIMING_7_GPIO, TIMING_7_PIN);
}

void setOutput7Low()
{
    LL_GPIO_ResetOutputPin(TIMING_7_GPIO, TIMING_7_PIN);
}
