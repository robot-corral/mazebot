/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

/*
 * Request / response in this file are from master point of view
 * (mcu which is calling line sensor)
 */

#include "line_sensor_defs.h"

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t sensorIndex_t;
// up to 15 commands are supported
typedef uint8_t lineSensorCommandCode_t;
typedef uint8_t lineSensorEncodedCommandCode_t;
// up to 15 distinct parameters or 4 bits are supported
typedef uint8_t lineSensorCommandParameter_t;
typedef uint8_t lineSensorEncodedCommandParameter_t;
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

static lineSensorEncodedCommandCode_t encodeCommand(lineSensorCommandCode_t cmd)
{
    return ((lineSensorEncodedCommandCode_t) cmd) | (((lineSensorEncodedCommandCode_t) (~cmd)) << 4);
}

static lineSensorEncodedCommandParameter_t encodeCommandParameter(lineSensorCommandParameter_t parameter)
{
    return ((lineSensorEncodedCommandParameter_t) parameter) | (((lineSensorEncodedCommandParameter_t) (~parameter)) << 4);
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
} lineSensorResponseStartCalibration_t;

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
    lineSensorCrcValue_t crc;
} lineSensorResponseFinishCalibration_t;

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
    lineSensorEncodedCommandParameter_t encodedCommandParameter;
    uint8_t filler[sizeof(lineSensorResponseGetSensorValues_t) - sizeof(lineSensorEncodedCommandCode_t) - sizeof(lineSensorEncodedCommandParameter_t)];
} lineSensorRequestGetSensorValues_t;

typedef enum
{
    LSR_SC_P_WHITE_CALIBRATION                    = 0x00,
    LSR_SC_P_BLACK_CALIBRATION                    = 0x01,
    // before starting collecting new calibration data we need to reset previous one to 0
    // this flag can be combined with the two flags above if old data isn't needed anymore
    LSR_SC_P_FLAG_RESET_PREVIOUS_CALIBRATION_DATA = 0x02,
} lineSensorRequestStartCalibrationParameters_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    lineSensorEncodedCommandParameter_t encodedCommandParameter;
    uint8_t filler[sizeof(lineSensorResponseStartCalibration_t) - sizeof(lineSensorEncodedCommandCode_t) - sizeof(lineSensorEncodedCommandParameter_t)];
} lineSensorRequestStartCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    lineSensorEncodedCommandParameter_t encodedCommandParameter;
    uint8_t filler[sizeof(lineSensorResponseGetCalibrationValues_t) - sizeof(lineSensorEncodedCommandCode_t) - sizeof(lineSensorEncodedCommandParameter_t)];
} lineSensorRequestGetCalibrationValues_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    lineSensorEncodedCommandParameter_t encodedCommandParameter;
    uint8_t filler[sizeof(lineSensorResponseFinishCalibration_t) - sizeof(lineSensorEncodedCommandCode_t) - sizeof(lineSensorEncodedCommandParameter_t)];
} lineSensorRequestFinishCalibration_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    lineSensorEncodedCommandParameter_t encodedCommandParameter;
    uint8_t filler[sizeof(lineSensorResponseGetDetailedSensorStatus_t) - sizeof(lineSensorEncodedCommandCode_t) - sizeof(lineSensorEncodedCommandParameter_t)];
} lineSensorRequestGetDetailedSensorStatus_t;

typedef struct __attribute__((packed))
{
    lineSensorEncodedCommandCode_t encodedCommandCode;
    lineSensorEncodedCommandParameter_t encodedCommandParameter;
    uint8_t filler[sizeof(lineSensorResponseReset_t) - sizeof(lineSensorEncodedCommandCode_t) - sizeof(lineSensorEncodedCommandParameter_t)];
} lineSensorRequestReset_t;

/*******************************************************************************
 * Unions
 *******************************************************************************/

typedef union __attribute__((packed))
{
    lineSensorRequestGetSensorValues_t getSensorValues;
    lineSensorRequestStartCalibration_t startCalibration;
    lineSensorRequestGetCalibrationValues_t getCalibrationValues;
    lineSensorRequestFinishCalibration_t finishCalibration;
    lineSensorRequestGetDetailedSensorStatus_t getDetailedSensorStatus;
    lineSensorRequestReset_t reset;
} lineSensorRequest_t;

typedef union __attribute__((packed))
{
    lineSensorResponseGetSensorValues_t getSensorValues;
    lineSensorResponseStartCalibration_t startCalibration;
    lineSensorResponseGetCalibrationValues_t getCalibrationValues;
    lineSensorResponseFinishCalibration_t finishCalibration;
    lineSensorResponseGetDetailedSensorStatus_t getDetailedSensorStatus;
    lineSensorResponseReset_t reset;
} lineSensorResponse_t;
