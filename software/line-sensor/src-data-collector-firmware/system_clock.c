#include "system_clock.h"

#include <stm32\stm32l4xx_ll_bus.h>
#include <stm32\stm32l4xx_ll_pwr.h>
#include <stm32\stm32l4xx_ll_rcc.h>
#include <stm32\stm32l4xx_ll_utils.h>
#include <stm32\stm32l4xx_ll_cortex.h>
#include <stm32\stm32l4xx_ll_system.h>

void initializeSystemClock()
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR  |
                             LL_APB1_GRP1_PERIPH_TIM5 |
                             LL_APB1_GRP1_PERIPH_TIM2 |
                             LL_APB1_GRP1_PERIPH_SPI3);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA |
                             LL_AHB2_GRP1_PERIPH_GPIOB |
                             LL_AHB2_GRP1_PERIPH_GPIOC |
                             LL_AHB2_GRP1_PERIPH_GPIOD |
                             LL_AHB2_GRP1_PERIPH_GPIOG);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) ;

    LL_RCC_MSI_Enable();
    while (LL_RCC_MSI_IsReady() != 1) ;

    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
    LL_RCC_MSI_SetCalibTrimming(0);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();

    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1) ;

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) ;

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_Init1msTick(80000000);
    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
    LL_SetSystemCoreClock(80000000);

    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
}
