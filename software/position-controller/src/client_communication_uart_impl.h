#pragma once

#include <stdbool.h>

#include "client_communication_uart.h"

void initializeUart();
bool processCommand(volatile clientUartRequest_t* pRequest);
bool tryToProcessEmergencyStopCommand(volatile clientUartRequest_t* pRequest);

#ifdef STM32L496ZG
    void enableVddIo2();
#endif
