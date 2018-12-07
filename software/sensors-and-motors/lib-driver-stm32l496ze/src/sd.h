#pragma once

typedef enum
{
    SD_DMA_RX = 1,
    SD_DMA_TX = 2,
} SdDmaDirection_t;

typedef enum
{
    SDR_OK                         = 0,
    SDR_COMMAND_CRC_FAIL           = 1,
    SDR_COMMAND_RESPONSE_TIMEOUT   = 2,
    SDR_CARD_VERSION_NOT_SUPPORTED = 3,
    SDR_ERROR                      = 4,
} SdResult_t;

SdResult_t initializeSd();
