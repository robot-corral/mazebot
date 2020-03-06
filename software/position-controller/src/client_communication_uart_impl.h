#pragma once

#include "client_communication_uart.h"

void initializeUart();
void processCommand(volatile clientUartRequest_t* pRequest);
void tryToProcessEmergencyStopCommand(volatile clientUartRequest_t* pRequest);

#ifdef STM32L496ZG
    void enableVddIo2();
#endif
