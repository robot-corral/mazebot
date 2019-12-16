#include "application.h"
#include "global_data.h"

#include "application/app_conf.h"

#include "application/hw_if.h"
#include "tiny_lpm/stm32_lpm.h"

static void initializeDebug();

void initializeApplication()
{
    UTIL_LPM_Init();

    HW_TS_Init(hw_ts_InitMode_Full, &g_rtc);

    UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);

    // TODO
    // appe_Tl_Init();	/* Initialize all transport layers */
}
