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

volatile lineSensorCommand_t g_rxBuffer;

volatile uint32_t g_txDataBufferIndexes;

volatile lineSensorCommandResponseSendSensorData_t g_txSendSensorDataBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * Calibration data
 *******************************************************************************/

volatile bool g_isCalibrated;

volatile uint16_t g_calibrationDataMaxMinusMin[NUMBER_OF_SENSORS];

volatile lineSensorCommandResponseFinishCalibration_t g_calibrationData;

/*******************************************************************************
 * Errors
 *******************************************************************************/

volatile lineSensorStatus_t g_sensorStatus;

void initializeGlobalData()
{
    g_isCalibrated = false;
    g_sensorStatus = LSS_ZERO;
    g_txDataBufferIndexes = DATA_BUFFER_EMPTY_INDEXES;
}
