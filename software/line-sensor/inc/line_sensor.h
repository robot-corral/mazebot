#pragma once

#include "line_sensor_defs.h"

#include <stdint.h>

typedef uint8_t sensorIndex_t;
typedef uint16_t headerPrefix_t;
typedef uint16_t lineSensorStatus_t;
typedef uint8_t lineSensorCommandCode_t;

/*******************************************************************************
 * Common data
 *******************************************************************************/

typedef struct
{
    uint16_t minSensorUnitValues[NUMBER_OF_SENSORS];
    uint16_t maxSensorUnitValues[NUMBER_OF_SENSORS];
} lineSensorCalibrationData_t;

/*******************************************************************************
 * Commands
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    // must be COMMAND_PREFIX
    headerPrefix_t          prefix;
    lineSensorCommandCode_t commandCode;
} lineSensorCommandHeader_t;

typedef struct __attribute__((packed))
{
    lineSensorCommandHeader_t header;
    lineSensorCalibrationData_t calibrationData;
} lineSensorCommand_t;

/*******************************************************************************
 * Command responses
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    // must be COMMAND_RESPONSE_PREFIX
    headerPrefix_t     prefix;
    lineSensorStatus_t status;
} lineSensorCommandResponseHeader_t;

typedef struct
{
    uint16_t sensorUnitValues[NUMBER_OF_SENSORS];
} lineSensorCommandResponseSendSensorData_t;

typedef struct __attribute__((packed))
{
    lineSensorCommandResponseHeader_t header;
    union
    {
        lineSensorCommandResponseSendSensorData_t sensorData;
        lineSensorCalibrationData_t calibrationData;
    };
} lineSensorCommandResponse_t;
