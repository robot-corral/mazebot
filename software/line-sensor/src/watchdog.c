#include "global_settings.h"

#include "watchdog.h"

#include "global_data.h"

#include <stm32/stm32l1xx_ll_rcc.h>
#include <stm32/stm32l1xx_ll_iwdg.h>
#include <stm32/stm32l1xx_ll_system.h>

void initializeAndStartWatchdog()
{
    if (LL_RCC_IsActiveFlag_IWDGRST())
    {
        LL_RCC_ClearResetFlags();
        g_watchdogResetStatus = LSS_ERR_FLAG_WATCHDOG_RESET_DETECTED;
    }

#ifdef DEBUG
    LL_DBGMCU_APB1_GRP1_FreezePeriph(LL_DBGMCU_APB1_GRP1_IWDG_STOP);
#endif

//    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
//    LL_PWR_EnableBkUpAccess();
//
//    LL_RCC_ForceBackupDomainReset();
//    LL_RCC_ReleaseBackupDomainReset();
//    LL_RCC_LSE_Enable();
//    while (LL_RCC_LSE_IsReady() != 1) ;
//
//    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);

    LL_RCC_LSI_Enable();
    while (LL_RCC_LSI_IsReady() != 1) ;

    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);

    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_4);
    // reset if resetWatchdog wasn't called in ((6 * 4) / 37000) = 0.648649 milliseconds
    LL_IWDG_SetReloadCounter(IWDG, 6);

    while (LL_IWDG_IsReady(IWDG) != 1) ;

    LL_IWDG_ReloadCounter(IWDG);
}

void resetWatchdog()
{
    LL_IWDG_ReloadCounter(IWDG);
}
