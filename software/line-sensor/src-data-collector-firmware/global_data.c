#include "global_data.h"

volatile bool g_isMotorControlBusy;

volatile bool g_lineSensorClientIsReceiving;
volatile bool g_lineSensorClientIsTransmitting;
volatile lineSensorRequest_t g_lineSensorClientTxBuffer;
volatile lineSensorResponse_t g_lineSensorClientRxBuffer;
