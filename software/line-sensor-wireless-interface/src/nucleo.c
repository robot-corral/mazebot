#include "nucleo.h"

#include "global_data.h"

#include "src-stm32-nucleo\otp.h"

#include <stm32wbxx_ll_rcc.h>

void initializeNucleoHardwareConfig()
{
    g_pNucleoHardwareConfiguration = (OTP_ID0_t*) OTP_Read(0);

    if (g_pNucleoHardwareConfiguration)
    {
        LL_RCC_HSE_SetCapacitorTuning(g_pNucleoHardwareConfiguration->hse_tuning);
    }
}
