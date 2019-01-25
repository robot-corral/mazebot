#include "system_clocks.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_pwr.h>
#include <stm32/stm32l1xx_ll_rcc.h>
#include <stm32/stm32l1xx_ll_utils.h>
#include <stm32/stm32l1xx_ll_cortex.h>
#include <stm32/stm32l1xx_ll_system.h>

void initializeSystemClocks(communicationInterface_t communicationInterface)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    LL_FLASH_Enable64bitAccess();
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_HSI_Enable();

    while (LL_RCC_HSI_IsReady() != 1);

    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);
    LL_RCC_PLL_Enable();

    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_Init1msTick(32000000);
    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
    LL_SetSystemCoreClock(32000000);

    if (communicationInterface == CI_I2C)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA |
                                 LL_AHB1_GRP1_PERIPH_GPIOB |
                                 LL_AHB1_GRP1_PERIPH_GPIOC |
                                 LL_AHB1_GRP1_PERIPH_GPIOE |
                                 LL_AHB1_GRP1_PERIPH_GPIOF |
                                 LL_AHB1_GRP1_PERIPH_GPIOG |
                                 LL_AHB1_GRP1_PERIPH_DMA1 | 
                                 LL_APB1_GRP1_PERIPH_I2C1);
    }
    else if(communicationInterface == CI_SPI)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA |
                                 LL_AHB1_GRP1_PERIPH_GPIOB |
                                 LL_AHB1_GRP1_PERIPH_GPIOC |
                                 LL_AHB1_GRP1_PERIPH_GPIOD |
                                 LL_AHB1_GRP1_PERIPH_GPIOE |
                                 LL_AHB1_GRP1_PERIPH_GPIOF |
                                 LL_AHB1_GRP1_PERIPH_GPIOG |
                                 LL_AHB1_GRP1_PERIPH_DMA1 | 
                                 LL_APB1_GRP1_PERIPH_SPI2);
    }
    else if (communicationInterface == CI_USART)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA |
                                 LL_AHB1_GRP1_PERIPH_GPIOB |
                                 LL_AHB1_GRP1_PERIPH_GPIOC |
                                 LL_AHB1_GRP1_PERIPH_GPIOD |
                                 LL_AHB1_GRP1_PERIPH_GPIOE |
                                 LL_AHB1_GRP1_PERIPH_GPIOF |
                                 LL_AHB1_GRP1_PERIPH_GPIOG |
                                 LL_AHB1_GRP1_PERIPH_DMA1 | 
                                 LL_APB1_GRP1_PERIPH_USART2);
    }

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
}
