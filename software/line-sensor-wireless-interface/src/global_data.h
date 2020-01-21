#pragma once

#include "src-stm32-nucleo/otp.h"

#include <line_sensor.h>
#include <consumer_producer_buffer.h>

/*******************************************************************************
 * Nucleo hardware configuration
 *******************************************************************************/

extern OTP_ID0_t* g_pNucleoHardwareConfiguration;

/*******************************************************************************
 * Line sensor client
 *******************************************************************************/

extern volatile bool g_lcsRestart;
extern volatile bool g_lscIsActive;
extern volatile bool g_lscIsReceiving;
extern volatile bool g_lscIsTransmitting;
extern volatile lineSensorRequest_t g_lscTxBuffer;
extern volatile lineSensorResponse_t g_lscRxBuffer;
extern volatile lineSensorCommandCode_t g_lscCurrentCommandCode;

/*******************************************************************************
 * Data queue
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    lineSensorValue_t sensorValues[LINE_SENSOR_NUMBER_OF_SENSORS];
    uint8_t numberOfCalls;
    uint8_t numberOfFailures;
    uint8_t numberOfCrcErrors;
    lineSensorStatus_t currentStatus;
    lineSensorDetailedStatus_t currentDetailedStatus;
    lineSensorDetailedStatus_t cumulativeDetailedStatusSinceLastReset;
} lineSensorData_t;

#define NUMBER_OF_LINE_SENSOR_DATA_BUFFERS DATA_BUFFER_LENGTH

extern volatile uint32_t g_lineSensorDataQueueProducerConsumerIndexes;
extern volatile lineSensorData_t g_lineSensorDataQueue[NUMBER_OF_LINE_SENSOR_DATA_BUFFERS];
