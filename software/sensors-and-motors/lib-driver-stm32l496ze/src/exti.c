/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "stm32/stm32l4xx_ll_exti.h"
#include "stm32/stm32l4xx_ll_system.h"

#include "exti.h"
#include "status.h"

void initializeExti()
{
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE12);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_12);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_12);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE11);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_11);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_11);

    NVIC_EnableIRQ(EXTI15_10_IRQn); 
    NVIC_SetPriority(EXTI15_10_IRQn, 0);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTE, LL_SYSCFG_EXTI_LINE6);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_6);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_6);
    NVIC_EnableIRQ(EXTI9_5_IRQn); 
    NVIC_SetPriority(EXTI9_5_IRQn, 2);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTF, LL_SYSCFG_EXTI_LINE0);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_0);
    NVIC_EnableIRQ(EXTI0_IRQn); 
    NVIC_SetPriority(EXTI0_IRQn, 0);
}

void EXTI0_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        // setCriticalError(ERROR_MOTOR_OVERCURRENT);
    }
}

void EXTI9_5_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_6) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_6);
        // startPause();
    }
}

void EXTI15_10_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_11) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_11);
        // setCriticalError(ERROR_MOTOR_OVERCURRENT);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_12) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12);
        // flushSd();
    }
}
