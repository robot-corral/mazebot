/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "spi.h"
#include "exti.h"
#include "gpio.h"
#include "motor.h"
#include "usart.h"
#include "timers.h"
#include "motor_impl.h"
#include "system_clock.h"

#include <driver.h>

void initializeDriver()
{
    initializeSystemClock();
    initializeGpio();
    initializeExti();
    initializeTimers();
    initializeMotor();
    initializeUsart();
    initializeSpi();
}

void initializeDebugDriver(bool enable)
{
    initializeSystemClock();
    initializeDebugGpio(enable);
    if (enable)
    {
        initializeDebugUsart();
    }
}

void shutdown()
{
    disableLeftMotor();
    disableRightMotor();
}
