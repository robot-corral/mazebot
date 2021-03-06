/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_settings.h"

#include "system_clocks.h"
#include "line_sensor_defs.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_pwr.h>
#include <stm32/stm32l1xx_ll_rcc.h>
#include <stm32/stm32l1xx_ll_utils.h>
#include <stm32/stm32l1xx_ll_cortex.h>
#include <stm32/stm32l1xx_ll_system.h>

void initializeSystemClocks()
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA |
                             LL_AHB1_GRP1_PERIPH_GPIOB |
                             LL_AHB1_GRP1_PERIPH_GPIOC |
                             LL_AHB1_GRP1_PERIPH_GPIOD |
                             LL_AHB1_GRP1_PERIPH_GPIOE |
                             LL_AHB1_GRP1_PERIPH_DMA1  |
                             LL_AHB1_GRP1_PERIPH_CRC);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5 |
                             LL_APB1_GRP1_PERIPH_COMP |
                             LL_APB1_GRP1_PERIPH_SPI2 |
                             LL_APB1_GRP1_PERIPH_PWR);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG | 
                             LL_APB2_GRP1_PERIPH_ADC1);

    LL_FLASH_Enable64bitAccess();
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    while (LL_PWR_IsActiveFlag_VOSF() != 0);

#ifdef CRYSTAL_EXTERNAL_HSE
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1);
#endif

    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1);

    LL_RCC_HSI_SetCalibTrimming(16);

#ifdef CRYSTAL_EXTERNAL_HSE
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLL_MUL_4, LL_RCC_PLL_DIV_3);
#else
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);
#endif

    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_Init1msTick(32000000);
    LL_SetSystemCoreClock(32000000);
}
