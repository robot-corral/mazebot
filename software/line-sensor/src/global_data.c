/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

bool g_isCalibrated;

// ADC is a 1/2 word increment so arrays must be 1/2 word aligned (2 bytes)
volatile uint16_t g_adcBuffer1[ADC_BUFFER_1_LENGTH] __attribute__((aligned(2)));
volatile uint16_t g_adcBuffer2[ADC_BUFFER_2_LENGTH] __attribute__((aligned(2)));

volatile uint16_t g_rxBuffer[RX_BUFFER_LENGTH];

volatile uint32_t g_txDataBufferIndexes;

volatile lineSensorCommandCode_t g_txResponseForSensorCommand;

volatile lineSensorCommandResponse_t g_txBuffer;
volatile lineSensorCommandResponse_t g_txSendSensorDataBuffers[NUMBER_OF_TX_DATA_BUFFERS];

uint16_t g_calibrationDataMaxMinusMin[NUMBER_OF_SENSORS];
lineSensorCommandResponseFinishCalibration_t g_calibrationData;

/*******************************************************************************
 * Errors
 *******************************************************************************/

uint8_t g_adcStatus;
uint8_t g_watchdogResetStatus;
uint8_t g_communicationDeviceStatus;

void initializeGlobalData()
{
    g_txBuffer.header.prefix = COMMAND_RESPONSE_PREFIX;
    for (uint8_t i = 0; i < NUMBER_OF_TX_DATA_BUFFERS; ++i)
    {
        g_txSendSensorDataBuffers[i].header.prefix = COMMAND_RESPONSE_PREFIX;
    }
}
