/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "spi.h"
#include "exti.h"
#include "gpio.h"
#include "timers.h"
#include "sd_impl.h"
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
    initializeLineSensorR1();
    initializeSpi();
    initializeImuLsm6ds3h();
    initializeSd();
}

void shutdown()
{
    disableLeftMotor();
    disableRightMotor();
}
