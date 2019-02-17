#pragma once

#include "line_sensor.h"

typedef uint8_t bufferIndex_t;

typedef struct
{
    lineSensorCommandResponseHeader_t header;
    union
    {
        lineSensorCommandResponseSendSensorData_t sensorData;
        lineSensorCommandResponseFinishCalibration_t calibrationData;
    } data;
} lineSensorData_t;
