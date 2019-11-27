#include "button.h"

#include "position_controller.h"
#include "interrupt_priorities.h"

#include <stm32\stm32l4xx_ll_exti.h>
#include <stm32\stm32l4xx_ll_system.h>

void initializeButton()
{
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, IRQ_PRIORITY_EMERGENCY_BUTTON);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_13);
}

void EXTI15_10_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        positionControllerEmergencyStop();
    }
}
