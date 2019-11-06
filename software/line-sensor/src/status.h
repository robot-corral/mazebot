#pragma once

#include "line_sensor.h"

#include <stdint.h>

lineSensorDetailedStatus_t getDetailedSensorStatus();

lineSensorDetailedStatus_t getCumulitiveDetailedSensorStatus();

static lineSensorStatus_t getSensorStatusFromDetailedStatus(lineSensorDetailedStatus_t detailedStatus)
{
    return detailedStatus == LSDS_OK ? LSS_ZERO : LSS_ERROR;
}

static lineSensorStatus_t getSensorStatus()
{
    return getSensorStatusFromDetailedStatus(getDetailedSensorStatus());
}

lineSensorDetailedStatus_t setSensorStatusFlags(lineSensorDetailedStatus_t flags);

lineSensorDetailedStatus_t resetSensorStatusFlags(lineSensorDetailedStatus_t flags);
