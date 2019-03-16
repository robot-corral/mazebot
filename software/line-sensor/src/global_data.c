/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

bool g_isCalibrated;

volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH];
volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH];

volatile lineSensorData_t g_dataBuffers[NUMBER_OF_DATA_BUFFERS];

lineSensorCommandResponseFinishCalibration_t g_calibrationData;

/*******************************************************************************
 * Errors
 *******************************************************************************/

uint8_t g_adcStatus;
uint8_t g_watchdogResetStatus;
