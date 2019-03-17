/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "line_sensor.h"

#define NUMBER_OF_TX_DATA_BUFFERS 3

// 1st sensor in both banks should be queried once as otherwise value which is read
// is too high
#define ADC_BUFFER_1_LENGTH 25
#define ADC_BUFFER_2_LENGTH 2

// allocate some extra characters in case we receive extra characters at the beginning of the message
#define RX_BUFFER_LENGTH sizeof(lineSensorCommand_t) + 5

void initializeGlobalData();

extern bool g_isCalibrated;

extern volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH];
extern volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH];

extern volatile uint16_t g_rxBuffer[RX_BUFFER_LENGTH];

extern volatile uint32_t g_txDataBufferIndexes;

extern volatile lineSensorCommandCode_t g_txResponseForSensorCommand;

extern volatile lineSensorCommandResponse_t g_txBuffer;
extern volatile lineSensorCommandResponse_t g_txSendSensorDataBuffers[NUMBER_OF_TX_DATA_BUFFERS];

extern uint16_t g_calibrationDataMaxMinusMin[NUMBER_OF_SENSORS];
extern lineSensorCommandResponseFinishCalibration_t g_calibrationData;

/*******************************************************************************
 * Errors
 *******************************************************************************/

extern uint8_t g_adcStatus;
extern uint8_t g_watchdogResetStatus;
extern uint8_t g_communicationDeviceStatus;
