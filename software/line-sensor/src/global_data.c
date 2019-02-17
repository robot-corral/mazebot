/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

bool g_isCalibrated;

volatile uint32_t g_currentProducerConsumerIndex;

volatile lineSensorData_t g_dataBuffers[NUMBER_OF_DATA_BUFFERS];

lineSensorCommandResponseFinishCalibration_t g_calibrationData;
