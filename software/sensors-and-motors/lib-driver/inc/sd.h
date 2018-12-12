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
    SDR_CARD_NOT_READY             = 12,
    SDR_INVALID_DMA_DIRECTION      = 13,
} SdResult_t;

/*
 * @return true if SD card is present and is usable (status can change at any moment though).
 */
bool isSdCardPresentAndInitialized();

/*
 * @return block size which must be used during read/write/erase operations.
 */
uint32_t getSdCardBlockSizeInBytes();

/*
 * @return maximum block id which can be used in read/write/erase operations (minimum block id is 0).
 */
uint32_t getSdCardNumberOfBlocks();

/*
 * Reads blocks from SD card to memory (asynchronously).
 *
 * Note: startBlockIndex + numberOfBlocks <= getSdCardNumberOfBlocks().
 *
 * @param pBlocksBuffer pointer to a buffer of size numberOfBlocks * getSdCardBlockSizeInBytes().
 * @param startBlockIndex block index starting from which data should be read.
 * @param numberOfBlocks number of blocks to be read.
 */
SdResult_t readBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startBlockIndex, uint32_t numberOfBlocks);

/*
 * Writes blocks from memory to SD card (asynchronously).
 *
 * Note: startBlockIndex + numberOfBlocks <= getSdCardNumberOfBlocks().
 *
 * @param pBlocksBuffer pointer to a buffer of size numberOfBlocks * getSdCardBlockSizeInBytes().
 * @param startBlockIndex block index starting to which data should be written.
 * @param numberOfBlocks number of blocks to be write.
 */
SdResult_t writeBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startBlockIndex, uint32_t numberOfBlocks);
