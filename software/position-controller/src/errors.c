#include "led.h"
#include "position_controller.h"

void NMI_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}

void HardFault_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}

void MemManage_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}

void BusFault_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}
