#pragma once

#include "line_sensor.h"

#include <stdint.h>

lineSensorStatus_t getSensorStatus();

lineSensorDetailedStatus_t getDetailedSensorStatus();

lineSensorDetailedStatus_t getCumulitiveDetailedSensorStatus();

lineSensorDetailedStatus_t setSensorStatusFlags(lineSensorDetailedStatus_t flags);

lineSensorDetailedStatus_t resetSensorStatusFlags(lineSensorDetailedStatus_t flags);
