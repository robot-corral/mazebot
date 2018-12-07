#pragma once

typedef enum
{
    SD_DMA_RX = 1,
    SD_DMA_TX = 2,
} SdDmaDirection_t;

typedef enum
{
    SDR_OK                         = 0,
    SDR_ILLEGAL_COMMAND            = 1,
    SDR_COMMAND_CRC_FAIL           = 2,
    SDR_COMMAND_RESPONSE_TIMEOUT   = 3,
    SDR_CARD_VERSION_NOT_SUPPORTED = 4,
    SDR_POWER_DOWN                 = 5,
    SDR_RESPONSE1_ERROR            = 6,
    SDR_UNKNOWN_ERROR              = 7,
} SdResult_t;

SdResult_t initializeSd();
