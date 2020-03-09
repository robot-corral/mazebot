#include "global_data.h"

volatile uint32_t g_slowDownPulseCount;
volatile uint32_t g_positionControllerX;
volatile uint32_t g_positionControllerXMaxValue;
volatile uint32_t g_positionControllerXPlannedPulseCount;
volatile uint32_t g_positionControllerXPulseErrorCount;
volatile positionControllerState_t g_positionControllerXState;
volatile positionControllerDirection_t g_positionControllerXDirection;

volatile bool g_clientUartIsTransmitting;
volatile clientUartRequest_t g_clientUartRxBuffer;
volatile clientUartResponse_t g_clientUartTxBuffer;
