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
    #define LINE_SENSOR_FILLER_HALF_WORD_LENGTH 2
    #define LINE_SENSOR_COMMAND_PROCESSING_DELAY uint16_t filler1[LINE_SENSOR_FILLER_HALF_WORD_LENGTH];
    #define LINE_SENSOR_ADDRESS_OF_SENSOR_VALUES(pData) (((uint16_t*) (pData)) + (LINE_SENSOR_FILLER_HALF_WORD_LENGTH + 1))
#else
    #define LINE_SENSOR_COMMAND_PROCESSING_DELAY ;
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

/*
 * For preformance reasons crc calculation is done as if data is an array of uint32_t with occasional 
 * remainder of uint16_t (if data length isn't divisible by 4)
 */

typedef struct __attribute__((packed))
{
    LINE_SENSOR_COMMAND_PROCESSING_DELAY;
    // crc calculation starts from the next line
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorValue_t sensorValues[LINE_SENSOR_NUMBER_OF_SENSORS];
    // crc calculation stops at the previous line
    lineSensorCrcValue_t crc;
} lineSensorResponseGetSensorValues_t;

#define LINE_SENSOR_RESPONSE_GET_SENSOR_VALUES_CRC_PROTECTED_DATA_LENGTH_BYTES (sizeof(lineSensorCommandCode_t) + sizeof(lineSensorStatus_t) + sizeof(lineSensorValue_t) * LINE_SENSOR_NUMBER_OF_SENSORS)

typedef struct __attribute__((packed))
{
    LINE_SENSOR_COMMAND_PROCESSING_DELAY;
    // crc calculation starts from the next line
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    // crc calculation stops at the previous line
    lineSensorCrcValue_t crc;
} lineSensorResponseStartCalibration_t;

#define LINE_SENSOR_RESPONSE_START_CALIBRATION_CRC_PROTECTED_DATA_LENGTH_BYTES (sizeof(lineSensorCommandCode_t) + sizeof(lineSensorStatus_t))

typedef struct __attribute__((packed))
{
    LINE_SENSOR_COMMAND_PROCESSING_DELAY;
    // crc calculation starts from the next line
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorValue_t minSensorCalibrationValues[LINE_SENSOR_NUMBER_OF_SENSORS];
    lineSensorValue_t maxSensorCalibrationValues[LINE_SENSOR_NUMBER_OF_SENSORS];
    // crc calculation stops at the previous line
    lineSensorCrcValue_t crc;
} lineSensorResponseGetCalibrationValues_t;

#define LINE_SENSOR_RESPONSE_GET_CALIBRATION_VALUES_CRC_PROTECTED_DATA_LENGTH_BYTES (sizeof(lineSensorCommandCode_t) + sizeof(lineSensorStatus_t) + 2 * sizeof(lineSensorValue_t) * LINE_SENSOR_NUMBER_OF_SENSORS)

typedef struct __attribute__((packed))
{
    LINE_SENSOR_COMMAND_PROCESSING_DELAY;
    // crc calculation starts from the next line
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    // crc calculation stops at the previous line
    lineSensorCrcValue_t crc;
} lineSensorResponseFinishCalibration_t;

#define LINE_SENSOR_RESPONSE_FINISH_CALIBRATION_CRC_PROTECTED_DATA_LENGTH_BYTES (sizeof(lineSensorCommandCode_t) + sizeof(lineSensorStatus_t))

typedef struct __attribute__((packed))
{
    LINE_SENSOR_COMMAND_PROCESSING_DELAY;
    // crc calculation starts from the next line
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    lineSensorDetailedStatus_t currentDetailedStatus;
    lineSensorDetailedStatus_t cumulativeDetailedStatusSinceLastReset;
    // crc calculation stops at the previous line
    lineSensorCrcValue_t crc;
} lineSensorResponseGetDetailedSensorStatus_t;

#define LINE_SENSOR_RESPONSE_GET_DETAILED_SENSOR_STATUS_CRC_PROTECTED_DATA_LENGTH_BYTES (sizeof(lineSensorCommandCode_t) + sizeof(lineSensorStatus_t) + sizeof(lineSensorDetailedStatus_t) + sizeof(lineSensorDetailedStatus_t))

typedef struct __attribute__((packed))
{
    LINE_SENSOR_COMMAND_PROCESSING_DELAY;
    // crc calculation starts from the next line
    lineSensorCommandCode_t respondingToCommandCode;
    lineSensorStatus_t currentStatus;
    // crc calculation stops at the previous line
    lineSensorCrcValue_t crc;
} lineSensorResponseReset_t;

#define LINE_SENSOR_RESPONSE_RESET_CRC_PROTECTED_DATA_LENGTH_BYTES (sizeof(lineSensorCommandCode_t) + sizeof(lineSensorStatus_t))

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
