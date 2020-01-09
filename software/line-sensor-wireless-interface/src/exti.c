#include "exti.h"

#include "stm32wbxx_hal.h"

void initializeExti()
{
    LL_EXTI_DisableIT_0_31(~0);
    LL_EXTI_DisableIT_32_63((~0) & (~(LL_EXTI_LINE_36 | LL_EXTI_LINE_38)));
}
