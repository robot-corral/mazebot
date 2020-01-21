#include "nucleo.h"
#include "system_clocks.h"
#include "line_sensor_client.h"

int main()
{
    initializeNucleoHardwareConfig();
    initializeSystemClock();
    initializeLineSensor();

    for (;;) ;
}
