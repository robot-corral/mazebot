/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

bool g_isCalibrated;

volatile uint32_t g_currentProducerConsumerIndex;

volatile uint16_t g_adcBuffer1[NUMBER_OF_SENSORS];
volatile uint16_t g_adcBuffer2[2];

volatile lineSensorData_t g_dataBuffers[NUMBER_OF_DATA_BUFFERS];

lineSensorCommandResponseFinishCalibration_t g_calibrationData;
