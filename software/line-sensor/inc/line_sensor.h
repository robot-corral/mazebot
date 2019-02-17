#pragma once

#include "line_sensor_defs.h"

#include <stdint.h>

typedef uint8_t sensorIndex_t;
typedef uint8_t lineSensorStatus_t;
typedef uint8_t lineSensorCommand_t;

typedef struct
{
    lineSensorCommand_t commandCode;
    lineSensorStatus_t  status;
} lineSensorCommandResponseHeader_t;

typedef struct
{
    uint16_t sensorUnitValues[NUMBER_OF_SENSORS];
} lineSensorCommandResponseSendSensorData_t;

typedef struct
{
    uint16_t minSensorUnitValues[NUMBER_OF_SENSORS];
    uint16_t maxSensorUnitValues[NUMBER_OF_SENSORS];
} lineSensorCommandResponseFinishCalibration_t;

typedef lineSensorCommandResponseFinishCalibration_t lineSensorCommandDataUseCalibrationData_t;
