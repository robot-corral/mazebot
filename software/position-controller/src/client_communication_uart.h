#pragma once

#include <stdint.h>

// one step is 0.025 mm (roughly)

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
    /*
     * ok value should be compared using equality sign after masking off all the error flags, e.g.:
     * if ((flags & 0x3) == okValue) { ... }
     */

    OK_RESET                = 0x0000,
    OK_IDLE                 = 0x0001,
    /*
     * this tells client that position controller is busy but it didn't interfere with executing requested command
     */
    OK_BUSY                 = 0x0002,
    OK_EMERGENCY_STOP       = 0x0003,

    /*
     * errors are flags and multiple can be set at the same time as well as they can go with ok values.
     */

    ERR_CRC                 = 0x0004,
    /*
     * busy error means, requested command cannot be executed as position controller is busy
     */
    ERR_BUSY                = 0x0008,
    ERR_COMMUNICATION_ERROR = 0x0010,
    /*
     * invalid state means that requested command cannot be executed when position controller is
     * in the state it is at the moment command execution was requested
     */
    ERR_INVALID_STATE       = 0x0020,
    ERR_INVALID_PARAMETER   = 0x0040,
    ERR_UNKNOWN_COMMAND     = 0x0080,
    ERR_UNEXPECTED          = 0x0100,
    /*
     * desired and actual movement had an error
     */
    ERR_POSITION_ERROR      = 0x0200,
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
    uint16_t resultFlags;
    /*
     * actual position (can be different from desired if there was an error, see ERR_POSITION_ERROR)
     */
    uint32_t position;
    uint32_t crc;
} clientUartResponseUnpacked_t;

typedef union
{
    uint32_t packed[3];
    clientUartResponseUnpacked_t unpacked;
} clientUartResponse_t;
