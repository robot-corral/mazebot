/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"
#include "watchdog.h"
#include "global_data.h"
#include "system_clocks.h"
#include "line_sensor_defs.h"

int main()
{
    initializeGlobalData();
    initializeAndStartWatchdog();
    initializeSystemClocks();
    initializeGpio();
    initializeSpi();
    initializeDma();
    initializeAdc();
    startQueryingAdc();
    activateSpi();
    for (;;) ;
}
