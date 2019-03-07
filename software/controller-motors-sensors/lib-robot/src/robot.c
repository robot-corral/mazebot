/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "imu.h"
#include "tasks.h"
#include "driver.h"
#include "status.h"
#include "line_sensor.h"

#include "global_data.h"

#include "robot.h"

uint32_t asyncTaskCallback()
{
    const bool oldTaskQuerySensors = g_taskQuerySensors;
    g_taskQuerySensors = !g_taskQuerySensors;

    if (oldTaskQuerySensors)
    {
        startQueryingImu();
        startQueryingLineSensor();
        return 40000; // 1 / 25 usec
    }
    else
    {
        return 1667; // 1'600 Hz minus 40'000 Hz
    }
}

void robotMainLoop()
{
    initializeDriver();
    initializeDebugDriver(true);

    calibrateLineSensor();
    setRunningStatus(RS_PAUSED);

    g_taskQuerySensors = true;

    startTaskScheduler();

    for (;;) ;
}
