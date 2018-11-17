/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "imu.h"
#include "tasks.h"
#include "driver.h"
#include "line_sensor.h"

uint32_t asyncTaskCallback()
{
    startQueryingImu();
    startQueryingLineSensor();
    return 40000; // 1 / 25 usec
}

int main()
{
    initializeDriver();
    initializeDebugDriver(true);
    startTaskScheduler();

    for (;;) ;
}
