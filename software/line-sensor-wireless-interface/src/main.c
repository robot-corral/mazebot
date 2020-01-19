#include "led.h"
#include "exti.h"
#include "gpio.h"
#include "debug.h"
#include "buttons.h"
#include "application.h"
#include "system_clocks.h"
#include "line_sensor_client.h"

#include "stm32wbxx_hal.h"

#include "utilities/otp.h"
#include "sequencer/stm32_seq.h"

int main()
{
    HAL_Init();

    initializeSystemClock();
    initializeExti();
    initializeGpio();
    initializeLineSensor();
    initializeLed();
    initializeButtons();
    initializeDebug();

    initializeApplication();

    while (1)
    {
        UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
    }
}
