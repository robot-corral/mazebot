/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "adc.h"
#include "spi.h"
#include "line_sensor.h"
#include "consumer_producer_buffer.h"

#define NUMBER_OF_TX_DATA_BUFFERS DATA_BUFFER_LENGTH

// 1st sensor in both banks should be queried once as otherwise value which is read
// is too high
#define ADC_BUFFER_1_LENGTH 25
#define ADC_BUFFER_2_LENGTH 2

#define MAX_RX_SIZE_BYTES (sizeof(lineSensorRequest_t) - sizeof(lineSensorEncodedCommandCode_t))

void initializeGlobalData();

/*******************************************************************************
 * ADC data
 *******************************************************************************/

extern volatile adcState_t g_adcState;

extern volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH];
extern volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH];

/*******************************************************************************
 * Calibration values
 *******************************************************************************/

extern volatile uint32_t g_lineSensorCalibrationValuesBuffersProducerConsumerIndexes;
extern volatile lineSensorResponseGetCalibrationValues_t g_lineSensorCalibrationValuesBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * Sensor values
 *******************************************************************************/

extern volatile uint32_t g_lineSensorValuesBuffersProducerConsumerIndexes;
extern volatile lineSensorResponseGetSensorValues_t g_lineSensorValuesBuffers[NUMBER_OF_TX_DATA_BUFFERS];

/*******************************************************************************
 * SPI data
 *******************************************************************************/

extern volatile spiState_t g_spiState;
extern volatile lineSensorCommandCode_t g_spiCurrentCommand;

extern volatile bool g_spiReceivingFillerFinished;
extern volatile bool g_spiTransmittingResponseFinished;

extern volatile uint8_t g_spiRxBuffer[MAX_RX_SIZE_BYTES];
extern volatile lineSensorResponse_t g_spiTxBuffer;

/*******************************************************************************
 * Errors
 *******************************************************************************/

extern volatile lineSensorDetailedStatus_t g_statusDetailedInternal; // do not read directly
extern volatile lineSensorDetailedStatus_t g_statusCumulativeDetailedInternal; // do not read directly
