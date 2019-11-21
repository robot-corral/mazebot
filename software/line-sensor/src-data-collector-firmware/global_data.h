#pragma once

#include <line_sensor.h>

#include <stdbool.h>

extern volatile bool g_isMotorControlBusy;

extern volatile bool g_lineSensorClientIsReceiving;
extern volatile bool g_lineSensorClientIsTransmitting;
extern volatile lineSensorRequest_t g_lineSensorClientTxBuffer;
extern volatile lineSensorResponse_t g_lineSensorClientRxBuffer;
