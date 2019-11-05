#pragma once

#include "line_sensor.h"

#include <stdint.h>

lineSensorDetailedStatus_t getDetailedSensorStatus();

lineSensorDetailedStatus_t getCumulitiveDetailedSensorStatus();

lineSensorStatus_t getSensorStatusFromDetailedStatus(lineSensorDetailedStatus_t detailedStatus);

static lineSensorStatus_t getSensorStatus()
{
    return getSensorStatusFromDetailedStatus(getDetailedSensorStatus());
}

lineSensorDetailedStatus_t setSensorStatusFlags(lineSensorDetailedStatus_t flags);

lineSensorDetailedStatus_t resetSensorStatusFlags(lineSensorDetailedStatus_t flags);
