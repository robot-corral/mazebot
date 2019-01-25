#pragma once

#include <stdbool.h>

typedef enum
{
    CI_UNKNOWN = 0,
    CI_I2C     = 1,
    CI_SPI     = 2,
    CI_USART   = 3,
} communicationInterface_t;

/*
 * initializes all GPIO pins as fast as possible to decrease startup time.
 * should only be called once after reset.
 */
void initializeGpio(communicationInterface_t communicationInterface);

/*
 * returns communication interface type (which is selected using either jumpers or
 * by soldering on the board).
 *
 * should be called once immediately after reset, as after configuration is done
 * all unused pins will be in analog mode to conserve power.
 */
communicationInterface_t getCommunicationInterface();

/*
 * Sets pin, used for figure out executing times, to high.
 */
void setOutput0High();

/*
 * Sets pin, used for figure out executing times, to low.
 */
void setOutput0Low();
