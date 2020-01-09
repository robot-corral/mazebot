#include "uart.h"
#include "debug.h"

#include <stm32/stm32wb55xx.h>

#include "stm32wbxx_hal.h"

#include "utilities/dbg_trace.h"

void initializeDebug()
{
    #ifdef DEBUG
        HAL_DBGMCU_EnableDBGSleepMode();
        LL_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);
        LL_C2_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);
        DbgTraceInit();
    #else
        GPIO_InitTypeDef gpio_config = {0};
        gpio_config.Pull = GPIO_NOPULL;
        gpio_config.Mode = GPIO_MODE_ANALOG;
        gpio_config.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13;

        __HAL_RCC_GPIOA_CLK_ENABLE();
        HAL_GPIO_Init(GPIOA, &gpio_config);
        __HAL_RCC_GPIOA_CLK_DISABLE();

        gpio_config.Pin = GPIO_PIN_4 | GPIO_PIN_3;
        __HAL_RCC_GPIOB_CLK_ENABLE();
        HAL_GPIO_Init(GPIOB, &gpio_config);
        __HAL_RCC_GPIOB_CLK_DISABLE();

        HAL_DBGMCU_DisableDBGSleepMode();
        HAL_DBGMCU_DisableDBGStopMode();
        HAL_DBGMCU_DisableDBGStandbyMode();
    #endif
}

void DbgOutputInit()
{
    usart1Initialize();
}
