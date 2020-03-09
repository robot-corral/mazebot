#pragma once

#include "position_controller.h"
#include "client_communication_uart.h"

#include <stdbool.h>

#define nullptr ((void*) 0)

extern volatile uint32_t g_slowDownPulseCount;
extern volatile uint32_t g_positionControllerX;
extern volatile uint32_t g_positionControllerXMaxValue;
extern volatile uint32_t g_positionControllerXPlannedPulseCount;
extern volatile uint32_t g_positionControllerXPulseErrorCount;
extern volatile positionControllerState_t g_positionControllerXState;
extern volatile positionControllerDirection_t g_positionControllerXDirection;

#define CLIENT_UART_RX_BUFFER_LENGTH (sizeof(clientUartRequest_t))
#define CLIENT_UART_TX_BUFFER_LENGTH (sizeof(clientUartResponse_t))

extern volatile bool g_clientUartIsTransmitting;
extern volatile clientUartRequest_t g_clientUartRxBuffer;
extern volatile clientUartResponse_t g_clientUartTxBuffer;
