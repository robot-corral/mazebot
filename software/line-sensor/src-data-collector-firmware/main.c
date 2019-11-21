#include "line_sensor.h"

#include "gpio.h"
#include "button.h"
#include "system_clock.h"
#include "motor_control.h"
#include "line_sensor_control.h"
#include "client_communication_usart.h"

int main()
{
    initializeSystemClock();
    intializeGpio();
    initializeButton();
    initializeLineSensor();
    initializeMotorControl();
    initializeClientCommunicationUsart();

    for (;;) ;
}
