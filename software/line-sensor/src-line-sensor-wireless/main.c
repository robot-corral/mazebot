#include "stm32wbxx_hal.h"

#include "utilities/otp.h"
#include "sequencer/stm32_seq.h"

static void initializeHse();

int main()
{
    HAL_Init();

    initializeHse();

//    SystemClock_Config();
//    PeriphClock_Config();
//    Init_Exti();
//    MX_GPIO_Init();
//    MX_DMA_Init();
//    MX_RF_Init();
//    MX_RTC_Init();
//    APPE_Init();

    while (1)
    {
        UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
    }
}

void initializeHse()
{
    OTP_ID0_t* p_otp = (OTP_ID0_t*) OTP_Read(0);
    if (p_otp)
    {
        LL_RCC_HSE_SetCapacitorTuning(p_otp->hse_tuning);
    }
}
