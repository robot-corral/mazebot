#pragma once

typedef enum
{
    SPI_S_IDLE,
    SPI_S_ERROR,
    SPI_S_WAITING_TO_RECEIVE_COMMAND_CODE,
    SPI_S_RECEIVING_FILLER_AND_TRANSMITTING_RESPONSE,
} spiState_t;

void initializeSpi();

void activateSpi();
