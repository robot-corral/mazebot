/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "imu.h"
#include "driver.h"
#include "status.h"
#include "line_sensor.h"

void main()
{
    initializeDriver();
    initializeDebugDriver(true);

    calibrateLineSensor();
    setRunningStatus(RS_PAUSED);

    for (;;) ;
}
