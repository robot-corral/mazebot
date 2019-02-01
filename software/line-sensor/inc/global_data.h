/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

#define NUMBER_OF_SENSORS 40

extern volatile uint16_t g_adcDataBuffer[NUMBER_OF_SENSORS];

#define TX_BUFFER_LENGTH (1 + 2 * NUMBER_OF_SENSORS * sizeof(uint16_t))
#define RX_BUFFER_LENGTH (1 + 1 + 2 * NUMBER_OF_SENSORS * sizeof(uint16_t))

extern volatile uint8_t g_txBuffer[TX_BUFFER_LENGTH];
extern volatile uint8_t g_rxBuffer[RX_BUFFER_LENGTH];

extern volatile uint16_t g_minValues[NUMBER_OF_SENSORS];
extern volatile uint16_t g_maxValues[NUMBER_OF_SENSORS];
