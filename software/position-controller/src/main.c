#include "gpio.h"
#include "button.h"
#include "system_clock.h"
#include "position_controller.h"
#include "client_communication_uart.h"

int main()
{
    initializeSystemClock();
    intializeGpio();
    initializeButton();
    initializePositionController();
    initializeClientCommunicationUart();

    for (;;) ;
}
