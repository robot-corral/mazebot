#pragma once

typedef enum
{
    WS_NONE                  = 0x00,
    WS_ADC                   = 0x01,
    WS_SPI                   = 0x02,

    WS_EVERYBODY_REPORTED_IN = WS_ADC | WS_SPI
} watchdogSource_t;

void initializeAndStartWatchdog();

void resetWatchdog(watchdogSource_t source);
