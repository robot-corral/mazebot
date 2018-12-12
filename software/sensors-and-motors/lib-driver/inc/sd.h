/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SDR_OK                         = 0,
    SDR_ILLEGAL_COMMAND            = 1,
    SDR_COMMAND_CRC_FAIL           = 2,
    SDR_COMMAND_RESPONSE_TIMEOUT   = 3,
    SDR_DATA_CRC_FAIL              = 4,
    SDR_DATA_RESPONSE_TIMEOUT      = 5,
    SDR_RX_OVERRUN                 = 6,
    SDR_CARD_VERSION_NOT_SUPPORTED = 7,
    SDR_POWER_DOWN                 = 8,
    SDR_RESPONSE1_ERROR            = 9,
    SDR_UNKNOWN_ERROR              = 10,
    SDR_BUSY                       = 11,
    SDR_CARD_NOT_PRESENT           = 12,
    SDR_CARD_NOT_INITIALIZED       = 13,
    SDR_INVALID_DMA_DIRECTION      = 14,
} SdResult_t;

bool isSdCardPresentAndSupported();

uint32_t getSdCardBlockSize();
uint32_t getSdCardNumberOfBlock();

SdResult_t readBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startAddress, uint32_t numberOfBlocks);
