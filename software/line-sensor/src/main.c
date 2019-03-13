/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "usart.h"
#include "system_clocks.h"

void main()
{
    initializeSystemClocks();
    initializeGpio();
    initializeAdc();
#ifdef ACTIVE_COMMUNICATION_I2C
    #error not implemented
#elif defined ACTIVE_COMMUNICATION_SPI
    #error not implemented
#elif defined ACTIVE_COMMUNICATION_USART
    initializeUsart();
#endif
    initializeDma();
    startQueryingAdc();
    for (;;) ;
}
