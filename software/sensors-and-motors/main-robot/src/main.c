/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "imu.h"
#include "tasks.h"
#include "driver.h"
#include "math_utils.h"
#include "line_sensor.h"

uint32_t asyncTaskCallback()
{
    uint32_t imuFrequency = startQueryingImu();
    uint32_t lineSensorFrequency = startQueryingLineSensor();
    return MIN(imuFrequency, lineSensorFrequency);
}

int main()
{
    initializeDriver();
    initializeDebugDriver(true);
    startTaskScheduler();

    for (;;) ;
}
