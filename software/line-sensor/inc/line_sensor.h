#pragma once

#include "line_sensor_defs.h"

#include <stdint.h>

typedef uint8_t sensorIndex_t;
typedef uint8_t headerPrefix_t;
typedef uint16_t lineSensorStatus_t;
typedef uint8_t lineSensorCommandCode_t;
typedef uint32_t crcValue_t;

/*******************************************************************************
 * Common data
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    headerPrefix_t          prefix;  // must be COMMAND_PREFIX
    lineSensorCommandCode_t commandCode;
} lineSensorRequestHeaderData_t;

typedef struct __attribute__((packed))
{
    headerPrefix_t     prefix;  // must be COMMAND_RESPONSE_PREFIX
    lineSensorStatus_t status;
} lineSensorResponseHeaderData_t;

typedef struct __attribute__((packed))
{
    uint16_t minSensorUnitValues[NUMBER_OF_SENSORS];
    uint16_t maxSensorUnitValues[NUMBER_OF_SENSORS];
    crcValue_t crc;
} lineSensorCalibrationData_t;

typedef struct __attribute__((packed))
{
    uint16_t sensorUnitValues[NUMBER_OF_SENSORS];
    crcValue_t crc;
} lineSensorValuesData_t;

/*******************************************************************************
 * Command responses
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    lineSensorResponseHeaderData_t header;
    crcValue_t                     crc;
} lineSensorCommandResponseStartCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorResponseHeaderData_t header;
    lineSensorCalibrationData_t    calibrationData;
    crcValue_t                     crc;
} lineSensorCommandResponseFinishCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorResponseHeaderData_t header;
    crcValue_t                     crc;
} lineSensorCommandResponseUseCalibrationData_t;

typedef struct __attribute__((packed))
{
    lineSensorResponseHeaderData_t header;
    crcValue_t                     crc;
} lineSensorCommandResponseUseHardcodedCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorResponseHeaderData_t header;
    lineSensorValuesData_t         sensorData;
    crcValue_t                     crc;
} lineSensorCommandResponseSendSensorData_t;

/*******************************************************************************
 * Commands
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    lineSensorRequestHeaderData_t header;
    crcValue_t                    crc;
} lineSensorCommandReset_t;

typedef struct __attribute__((packed))
{
    lineSensorRequestHeaderData_t header;
    crcValue_t                    crc;
    uint8_t                       spaceFiller[sizeof(lineSensorCommandResponseStartCalibration_t)];
} lineSensorCommandStartCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorRequestHeaderData_t header;
    crcValue_t                    crc;
    uint8_t                       spaceFiller[sizeof(lineSensorCommandResponseFinishCalibration_t)];
} lineSensorCommandFinishCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorRequestHeaderData_t header;
    lineSensorCalibrationData_t   calibrationData;
    crcValue_t                    crc;
    uint8_t                       spaceFiller[sizeof(lineSensorCommandResponseUseCalibrationData_t)];
} lineSensorCommandUseCalibrationData_t;

typedef struct __attribute__((packed))
{
    lineSensorRequestHeaderData_t header;
    crcValue_t                    crc;
    uint8_t                       spaceFiller[sizeof(lineSensorCommandResponseUseHardcodedCalibration_t)];
} lineSensorCommandUseHardcodedCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorRequestHeaderData_t header;
    crcValue_t                    crc;
    uint8_t                       spaceFiller[sizeof(lineSensorCommandResponseSendSensorData_t)];
} lineSensorCommandSendSensorData_t;

/*******************************************************************************
 * Commands / responses unions
 *******************************************************************************/

typedef union __attribute__((packed))
{
    lineSensorCommandReset_t                   reset;
    lineSensorCommandStartCalibration_t        startCalibration;
    lineSensorCommandFinishCalibration_t       finishCalibration;
    lineSensorCommandUseCalibrationData_t      useCalibrationData;
    lineSensorCommandUseHardcodedCalibration_t useHardcodedCalibration;
    lineSensorCommandSendSensorData_t          sendSensorData;
} lineSensorCommand_t;

typedef union __attribute__((packed))
{
    lineSensorCommandResponseStartCalibration_t        startCalibration;
    lineSensorCommandResponseFinishCalibration_t       finishCalibration;
    lineSensorCommandResponseUseCalibrationData_t      useCalibrationData;
    lineSensorCommandResponseUseHardcodedCalibration_t useHardcodedCalibration;
    lineSensorCommandResponseSendSensorData_t          sendSensorData;
} lineSensorCommandResponse_t;
