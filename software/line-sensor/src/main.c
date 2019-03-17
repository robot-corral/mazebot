/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "usart.h"
#include "watchdog.h"
#include "global_data.h"
#include "system_clocks.h"
#include "line_sensor_defs.h"

void main()
{
    initializeGlobalData();
    initializeAndStartWatchdog();
    initializeSystemClocks();
    initializeGpio();
    initializeDma();
    initializeAdc();
#ifdef ACTIVE_COMMUNICATION_I2C
    #error not implemented
#elif defined ACTIVE_COMMUNICATION_SPI
    #error not implemented
#elif defined ACTIVE_COMMUNICATION_USART
    initializeUsart();
#endif
    startQueryingAdc();
    for (;;) ;
}
