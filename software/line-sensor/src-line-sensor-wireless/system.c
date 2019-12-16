#include "system.h"

#include "error.h"

#include <utilities/otp.h>

#include <stm32wbxx_ll_bus.h>
#include <stm32wbxx_ll_rtc.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_cortex.h>
#include <stm32wbxx_ll_system.h>

static void initializeHse();
static void initializeClocks();

void initializeSystem()
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);

    if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_3)
    {
        fatalError();
    }

    initializeHse();
    initializeClocks();
}

void initializeClocks()
{
    /* HSE configuration and activation */
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1) ;

    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) ;

    /* HSI48 configuration and activation */
    LL_RCC_HSI48_Enable();
    while (LL_RCC_HSI48_IsReady() != 1) ;

    LL_RCC_LSI1_Enable();

    /* Wait till LSI is ready */
    while (LL_RCC_LSI1_IsReady() != 1) ;

    LL_RCC_HSE_EnableCSS();
    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 8, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while (LL_RCC_PLL_IsReady() != 1) ;

    /* Sysclk activation on the main PLL */
    /* Set CPU1 prescaler*/
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    /* Set CPU2 prescaler*/
    LL_C2_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) ;

    /* Set AHB SHARED prescaler*/
    LL_RCC_SetAHB4Prescaler(LL_RCC_SYSCLK_DIV_1);

    /* Set APB1 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    /* Set APB2 prescaler*/
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(64000000);
    LL_RCC_SetRNGClockSource(LL_RCC_RNG_CLKSOURCE_CLK48);
    LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_HSI48);
    LL_RCC_SetSMPSClockSource(LL_RCC_SMPS_CLKSOURCE_HSE);
    LL_RCC_SetSMPSPrescaler(LL_RCC_SMPS_DIV_0);
    LL_RCC_SetRFWKPClockSource(LL_RCC_RFWKP_CLKSOURCE_LSI);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);

    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_HSEM |
                             LL_AHB3_GRP1_PERIPH_IPCC);
}

void initializeHse()
{
    OTP_ID0_t* p_otp = (OTP_ID0_t*) OTP_Read(0);
    if (p_otp)
    {
        LL_RCC_HSE_SetCapacitorTuning(p_otp->hse_tuning);
    }
}
