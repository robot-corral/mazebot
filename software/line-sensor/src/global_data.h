/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

#define NUMBER_OF_DATA_BUFFERS 3

extern bool g_isCalibrated;

extern volatile uint32_t g_currentProducerConsumerIndex;

extern volatile lineSensorData_t g_dataBuffers[NUMBER_OF_DATA_BUFFERS];

extern lineSensorCommandResponseFinishCalibration_t g_calibrationData;
