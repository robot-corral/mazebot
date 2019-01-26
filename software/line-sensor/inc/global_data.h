#pragma once

#include <stdint.h>

extern volatile uint16_t g_adcDataBuffer[40];

// 1   - cmd
// 160 - 2 * 40 * sizeof(uint16_t)
#define TX_BUFFER_LENGTH (1 + 2 * 2 * 40)
// 1   - result code
// 160 - 2 * 40 * sizeof(uint16_t)
#define RX_BUFFER_LENGTH (1 + 2 * 2 * 40)

extern volatile uint8_t g_txBuffer[TX_BUFFER_LENGTH];
extern volatile uint8_t g_rxBuffer[RX_BUFFER_LENGTH];
