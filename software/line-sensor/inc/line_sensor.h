#pragma once

/*
 * Request / response in this file are from master point of view
 * (mcu which is calling line sensor)
 */

#include "line_sensor_defs.h"

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t sensorIndex_t;
typedef uint8_t lineSensorCommandCode_t;
typedef uint16_t lineSensorEncodedCommandCode_t;
typedef uint8_t lineSensorStatus_t;
typedef uint32_t lineSensorDetailedStatus_t;
typedef uint32_t lineSensorCrcValue_t;
typedef uint16_t lineSensorValue_t;

#ifndef SLAVE
    #define COMMAND_PROCESSING_DELAY uint8_t filler1[4];
#else
    #define COMMAND_PROCESSING_DELAY ;
#endif

/*******************************************************************************
 * Common data
 *******************************************************************************/

#define COMMAND_1_MASK ((uint16_t) 0b0000000011111111)

static lineSensorEncodedCommandCode_t encodeCommand(lineSensorCommandCode_t cmd)
{
    return ((lineSensorEncodedCommandCode_t) cmd) | (((lineSensorEncodedCommandCode_t) (~cmd)) << 8);
}

/*******************************************************************************
 * Responses
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    COMMAND_PROCESSING_DELAY;
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorValue_t sensorValues[NUMBER_OF_SENSORS];
    lineSensorCrcValue_t crc;
} lineSensorResponseGetSensorValues_t;

typedef struct __attribute__((packed))
{
    COMMAND_PROCESSING_DELAY;
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorCrcValue_t crc;
} lineSensorResponseStartPhysicalCalibration_t;

typedef struct __attribute__((packed))
{
    COMMAND_PROCESSING_DELAY;
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorCrcValue_t crc;
} lineSensorResponseGetCalibrationStatus_t;

typedef struct __attribute__((packed))
{
    COMMAND_PROCESSING_DELAY;
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorCrcValue_t crc;
} lineSensorResponseFinishCalibration_t;

typedef struct __attribute__((packed))
{
    COMMAND_PROCESSING_DELAY;
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorValue_t minSensorCalibrationValues[NUMBER_OF_SENSORS];
    lineSensorValue_t maxSensorCalibrationValues[NUMBER_OF_SENSORS];
    lineSensorCrcValue_t crc;
} lineSensorResponseGetCalibrationValues_t;

typedef struct __attribute__((packed))
{
    COMMAND_PROCESSING_DELAY;
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorDetailedStatus_t currentDetailedStatus;
    lineSensorDetailedStatus_t cumulativeDetailedStatusSinceLastReset;
    lineSensorCrcValue_t crc;
} lineSensorResponseGetDetailedSensorStatus_t;

typedef struct __attribute__((packed))
{
    COMMAND_PROCESSING_DELAY;
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorCrcValue_t crc;
} lineSensorResponseReset_t;

/*******************************************************************************
 * Requests
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    uint8_t filler[sizeof(lineSensorResponseGetSensorValues_t)];
} lineSensorRequestGetSensorValues_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    uint8_t filler[sizeof(lineSensorResponseStartPhysicalCalibration_t)];
} lineSensorRequestStartPhysicalCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    uint8_t filler[sizeof(lineSensorResponseGetCalibrationStatus_t)];
} lineSensorRequestGetCalibrationStatus_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    uint8_t filler[sizeof(lineSensorResponseFinishCalibration_t)];
} lineSensorRequestFinishCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    uint8_t filler[sizeof(lineSensorResponseGetCalibrationValues_t)];
} lineSensorRequestGetCalibrationValues_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    uint8_t filler[sizeof(lineSensorResponseGetDetailedSensorStatus_t)];
} lineSensorRequestGetDetailedSensorStatus_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    uint8_t filler[sizeof(lineSensorResponseReset_t)];
} lineSensorRequestReset_t;

/*******************************************************************************
 * Unions
 *******************************************************************************/

typedef union __attribute__((packed))
{
    lineSensorRequestGetSensorValues_t getSensorValues;
    lineSensorRequestStartPhysicalCalibration_t startPhysicalCalibration;
    lineSensorRequestGetCalibrationStatus_t getCalibrationStatus;
    lineSensorRequestFinishCalibration_t finishCalibration;
    lineSensorRequestGetCalibrationValues_t getCalibrationValues;
    lineSensorRequestGetDetailedSensorStatus_t getDetailedSensorStatus;
    lineSensorRequestReset_t reset;
} lineSensorRequest_t;

typedef union __attribute__((packed))
{
    lineSensorResponseGetSensorValues_t getSensorValues;
    lineSensorResponseStartPhysicalCalibration_t startPhysicalCalibration;
    lineSensorResponseGetCalibrationStatus_t getCalibrationStatus;
    lineSensorResponseFinishCalibration_t finishCalibration;
    lineSensorResponseGetCalibrationValues_t getCalibrationValues;
    lineSensorResponseGetDetailedSensorStatus_t getDetailedSensorStatus;
    lineSensorResponseReset_t reset;
} lineSensorResponse_t;
