#include "line_sensor.h"

#include "gpio.h"
#include "button.h"
#include "system_clock.h"
#include "position_controller.h"
#include "line_sensor_control.h"
#include "client_communication_uart.h"

int main()
{
    initializeSystemClock();
    intializeGpio();
    initializeButton();
    initializeLineSensor();
    initializePositionController();
    initializeClientCommunicationUart();

    for (;;) ;
}
