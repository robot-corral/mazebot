#pragma once

#include "position_controller.h"
#include "client_communication_uart.h"

#include <line_sensor.h>

#include <stdbool.h>

#define nullptr ((void*) 0)

extern volatile uint32_t g_positionControllerX;
extern volatile uint32_t g_positionControllerXMaxValue;
extern volatile uint32_t g_positionControllerXDesiredValue;
extern volatile positionControllerStatus_t g_positionControllerXStatus;
extern volatile positionControllerDirection_t g_positionControllerXDirection;

extern volatile bool g_lineSensorClientIsReceiving;
extern volatile bool g_lineSensorClientIsTransmitting;
extern volatile lineSensorRequest_t g_lineSensorClientTxBuffer;
extern volatile lineSensorResponse_t g_lineSensorClientRxBuffer;

#define CLIENT_UART_RX_BUFFER_LENGTH (sizeof(clientUartRequest_t))
#define CLIENT_UART_TX_BUFFER_LENGTH (sizeof(clientUartResponse_t))

extern volatile clientUartRequest_t g_clientUartRxBuffer;
extern volatile clientUartResponse_t g_clientUartTxBuffer;
