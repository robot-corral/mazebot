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

// allocate some extra characters in case we receive extra characters at the beginning of the message
#define RX_BUFFER_LENGTH sizeof(lineSensorCommand_t) + 5

void initializeGlobalData();

/*******************************************************************************
 * ADC data
 *******************************************************************************/

extern volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH];

extern volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH];

/*******************************************************************************
 * RX/TX data
 *******************************************************************************/

extern volatile uint16_t g_rxBuffer[RX_BUFFER_LENGTH];

// current consumer and producer indexes
extern volatile uint32_t g_txDataBufferIndexes;

extern volatile lineSensorCommandResponse_t g_txSendSensorDataBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * Calibration data
 *******************************************************************************/

extern bool g_isCalibrated;

extern uint16_t g_calibrationDataMaxMinusMin[NUMBER_OF_SENSORS];

extern lineSensorCalibrationData_t g_calibrationData;

/*******************************************************************************
 * Errors
 *******************************************************************************/

extern uint8_t g_adcStatus;

extern uint8_t g_watchdogResetStatus;

extern uint8_t g_communicationDeviceStatus;
