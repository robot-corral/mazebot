#pragma once

#include "line_sensor_defs.h"

#include <stdint.h>

typedef uint8_t sensorIndex_t;
typedef uint8_t lineSensorStatus_t;
typedef uint8_t lineSensorCommandCode_t;

typedef struct
{
    lineSensorCommandCode_t commandCode;
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

typedef struct
{
    lineSensorCommandCode_t commandCode;
    lineSensorCommandDataUseCalibrationData_t calibrationData;
} lineSensorCommand_t;

typedef struct
{
    lineSensorCommandResponseHeader_t header;
    union
    {
        lineSensorCommandResponseSendSensorData_t sensorData;
        lineSensorCommandResponseFinishCalibration_t calibrationData;
    };
} lineSensorCommandResponse_t;

#define LSC_LENGTH_SEND_SENSOR_DATA sizeof(lineSensorCommandCode_t)
#define LSCR_LENGTH_SEND_SENSOR_DATA (sizeof(lineSensorCommandResponseHeader_t) + sizeof(lineSensorCommandResponseSendSensorData_t))
