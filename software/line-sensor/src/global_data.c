/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

/*******************************************************************************
 * ADC data
 *******************************************************************************/

// ADC is a 1/2 word increment so arrays must be 1/2 word aligned (2 bytes)
volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH] __attribute__((aligned(2)));

volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH] __attribute__((aligned(2)));

/*******************************************************************************
 * RX/TX data
 *******************************************************************************/

volatile uint16_t g_rxBuffer[RX_BUFFER_LENGTH];

volatile uint32_t g_txDataBufferIndexes;

volatile lineSensorCommandResponse_t g_txSendSensorDataBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * Calibration data
 *******************************************************************************/

bool g_isCalibrated;

uint16_t g_calibrationDataMaxMinusMin[NUMBER_OF_SENSORS];

lineSensorCalibrationData_t g_calibrationData;

/*******************************************************************************
 * Errors
 *******************************************************************************/

uint8_t g_adcStatus;

uint8_t g_watchdogResetStatus;

uint8_t g_communicationDeviceStatus;

void initializeGlobalData()
{
    g_isCalibrated = false;

    g_adcStatus = 0;
    g_watchdogResetStatus = 0;
    g_communicationDeviceStatus = 0;

    g_txDataBufferIndexes = DATA_BUFFER_EMPTY_INDEXES;
}
