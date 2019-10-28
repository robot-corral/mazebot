/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "line_sensor.h"
#include "consumer_producer_buffer.h"

#define NUMBER_OF_TX_DATA_BUFFERS DATA_BUFFER_LENGTH

// 1st sensor in both banks should be queried once as otherwise value which is read
// is too high
#define ADC_BUFFER_1_LENGTH 25
#define ADC_BUFFER_2_LENGTH 2

void initializeGlobalData();

/*******************************************************************************
 * ADC data
 *******************************************************************************/

extern volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH];

extern volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH];

/*******************************************************************************
 * RX/TX data
 *******************************************************************************/

extern bool g_receivingHeader;

extern volatile lineSensorCommand_t g_rxBuffer;

// current consumer and producer indexes
extern volatile uint32_t g_txDataBufferIndexes;

extern volatile lineSensorCommandResponseSendSensorData_t g_txSendSensorDataBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * Calibration data
 *******************************************************************************/

extern volatile bool g_isCalibrated;

extern volatile uint16_t g_calibrationDataMaxMinusMin[NUMBER_OF_SENSORS];

extern volatile lineSensorCommandResponseFinishCalibration_t g_calibrationData;

/*******************************************************************************
 * Errors
 *******************************************************************************/

extern volatile lineSensorStatus_t g_sensorStatusInternal; // do not read directly
                                                           // use getSensorStatus()
