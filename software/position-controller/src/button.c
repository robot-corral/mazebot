#include "button.h"

#include "position_controller.h"
#include "interrupt_priorities.h"

#include <stm32\stm32l4xx_ll_exti.h>
#include <stm32\stm32l4xx_ll_system.h>

void initializeButton()
{
    // emergency stop

    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, IRQ_PRIORITY_EMERGENCY_BUTTON);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_13);

    // max limit stop

    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_SetPriority(EXTI1_IRQn, IRQ_PRIORITY_LIMIT_SWITCH);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE1);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_1);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_1);

    // min limit stop

    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_SetPriority(EXTI3_IRQn, IRQ_PRIORITY_LIMIT_SWITCH);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTD, LL_SYSCFG_EXTI_LINE3);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_3);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_3);
}

void EXTI15_10_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) == SET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        positionControllerEmergencyStop();
    }
}

void EXTI1_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) == SET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
        positionControllerLimitStop(PCLST_MAX);
    }
}

void EXTI3_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_3) == SET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
        positionControllerLimitStop(PCLST_MIN);
    }
}
