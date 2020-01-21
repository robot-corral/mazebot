#include "system_clocks.h"
#include "error.h"

#include <stm32wbxx_ll_bus.h>
#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_cortex.h>
#include <stm32wbxx_ll_system.h>

static void initializeClocks();

void initializeSystemClock()
{
    initializeClocks();
    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
}

void initializeClocks()
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
    {
        fatalError();
    }

    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1) ;

    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) ;

    LL_PWR_EnableBkUpAccess();

    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();

    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
    LL_RCC_LSE_Enable();
    while (LL_RCC_LSE_IsReady() != 1) ;

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_C2_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE) ;

    LL_RCC_SetAHB4Prescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_Init1msTick(32000000);

    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);

    LL_SetSystemCoreClock(32000000);

    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    LL_RCC_EnableRTC();
    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSI);
    LL_RCC_SetSMPSPrescaler(LL_RCC_SMPS_DIV_1);
    LL_RCC_SetRFWKPClockSource(LL_RCC_RFWKP_CLKSOURCE_NONE);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB |
                             LL_AHB2_GRP1_PERIPH_GPIOC |
                             LL_AHB2_GRP1_PERIPH_GPIOD);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2 |
                             LL_APB1_GRP1_PERIPH_TIM2);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1 |
                             LL_AHB1_GRP1_PERIPH_CRC |
                             LL_AHB1_GRP1_PERIPH_DMAMUX1);
}
