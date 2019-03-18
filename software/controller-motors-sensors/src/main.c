/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "driver.h"
#include "line_sensor.h"

#include <det_os_implementation.h>

void main()
{
    initializeDriver();
    runDetOs(calibrateLineSensor);
}
