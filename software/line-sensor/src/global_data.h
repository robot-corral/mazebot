/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

#define NUMBER_OF_DATA_BUFFERS 3

// 1st sensor in both banks should be queried once as otherwise value which is read
// is too high
#define ADC_BUFFER_1_LENGTH 25
#define ADC_BUFFER_2_LENGTH 2

extern bool g_isCalibrated;

extern volatile uint32_t g_currentProducerConsumerIndex;

extern volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH];
extern volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH];

extern volatile lineSensorData_t g_dataBuffers[NUMBER_OF_DATA_BUFFERS];

extern lineSensorCommandResponseFinishCalibration_t g_calibrationData;
