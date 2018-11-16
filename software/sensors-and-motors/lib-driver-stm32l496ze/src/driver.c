/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "spi.h"
#include "exti.h"
#include "gpio.h"
#include "usart.h"
#include "timers.h"
#include "system_clock.h"
#include "imu_lsm6ds3h.h"
#include "line_sensor_r1.h"
#include "motor_controller.h"
#include "motor_controller_mc33926.h"

#include <driver.h>

void initializeDriver()
{
    initializeSystemClock();
    initializeGpio();
    initializeExti();
    initializeTimers();
    initializeMotorControllerMc33926();
    initializeUsart();
    initializeLineSensorR1();
    initializeSpi();
    initializeImuLsm6ds3h();
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
