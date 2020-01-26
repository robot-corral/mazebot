#pragma once

#include <stdint.h>

void initializeClientCommunicationUart();

#define CRC32_POLYNOMIAL ((uint32_t) 0x04C11DB7)

#define CLIENT_UART_REQUEST_HEADER  ((uint16_t) 0x5441)
#define CLIENT_UART_RESPONSE_HEADER ((uint16_t) 0x5441)

typedef enum
{
    MCMD_NONE           = 0,
    MCMD_CALIBRATE      = 1,
    MCMD_EMERGENCY_STOP = 2,
    MCMD_MOVE_IF_IDLE   = 3,
    MCMD_GET_STATUS     = 4,
    MCMD_RESET          = 5,
} motorCommand_t;

typedef enum
{
    OK                      = 0x00,
    ERR_COMMUNICATION_ERROR = 0x01,
    ERR_UNKNOWN_COMMAND     = 0x02,
    ERR_BUSY                = 0x04,
    ERR_EMERGENCY_STOP      = 0x08,
    ERR_CRC                 = 0x10,
} commandResultFlags_t;

typedef struct __attribute__((packed))
{
    uint16_t header;
    uint8_t motorCommand;
    uint8_t direction;
    uint32_t steps;
    uint32_t crc;
} clientUartRequestUnpacked_t;

typedef union
{
    uint32_t packed[3];
    clientUartRequestUnpacked_t unpacked;
} clientUartRequest_t;

typedef struct __attribute__((packed))
{
    uint16_t header;
    uint8_t resultFlags;
    uint8_t filler;
    uint32_t position;
    uint32_t crc;
} clientUartResponseUnpacked_t;

typedef union
{
    uint32_t packed[3];
    clientUartResponseUnpacked_t unpacked;
} clientUartResponse_t;
