/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

#include "watchdog.h"

/*******************************************************************************
 * ADC data
 *******************************************************************************/

volatile adcState_t g_adcState;
volatile uint8_t g_adcCalibrationParameter;

// ADC is a 1/2 word increment so arrays must be 1/2 word aligned (2 bytes)
volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH] __attribute__((aligned(2)));
volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH] __attribute__((aligned(2)));

/*******************************************************************************
 * Calibration data
 *******************************************************************************/

volatile uint16_t g_calibrationMinValues[LINE_SENSOR_NUMBER_OF_SENSORS];
volatile uint16_t g_calibrationMaxValues[LINE_SENSOR_NUMBER_OF_SENSORS];

volatile uint32_t g_lineSensorCalibrationValuesBuffersProducerConsumerIndexes;
volatile lineSensorResponseGetCalibrationValues_t g_lineSensorCalibrationValuesBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * Sensor values
 *******************************************************************************/

volatile uint32_t g_lineSensorValuesBuffersProducerConsumerIndexes;
volatile lineSensorResponseGetSensorValues_t g_lineSensorValuesBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * SPI data
 *******************************************************************************/

volatile spiState_t g_spiState;
volatile lineSensorCommandCode_t g_spiCurrentCommand;

volatile void* g_spiTxData;
volatile uint32_t g_spiTxDataHalfWordLength;

volatile uint32_t g_spiFillerHalfWordsTransmitted;

volatile uint8_t g_spiRxBuffer[MAX_RX_SIZE_BYTES];
volatile lineSensorResponse_t g_spiTxBuffer;

/*******************************************************************************
 * Errors
 *******************************************************************************/

volatile lineSensorDetailedStatus_t g_statusDetailedInternal;
volatile lineSensorDetailedStatus_t g_statusCumulativeDetailedInternal;

void initializeGlobalData()
{
    g_statusDetailedInternal = LSDS_OK;
    g_statusCumulativeDetailedInternal = LSDS_OK;
    consumerProducerBuffer_reset(&g_lineSensorValuesBuffersProducerConsumerIndexes);
    consumerProducerBuffer_reset(&g_lineSensorCalibrationValuesBuffersProducerConsumerIndexes);
}

/*******************************************************************************
 * Task scheduler
 *******************************************************************************/

volatile taskFunction_t g_taskSchedulerCurrentTask;
