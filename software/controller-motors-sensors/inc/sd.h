/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SDR_OK                              = 0,
    SDR_ERR_CARD_POWER_DOWN             = 1,
    SDR_ERR_CARD_NOT_SUPPORTED          = 2,
    SDR_ERR_ILLEGAL_COMMAND             = 3,
    SDR_ERR_CARD_STATUS_ERROR           = 4,
    SDR_ERR_COMMAND_CRC_FAIL            = 5,
    SDR_ERR_COMMAND_RESPONSE_TIMEOUT    = 6,
    SDR_ERR_COMMAND_RESPONSE_INDEX_FAIL = 5,
    SDR_ERR_DATA_CRC_FAIL               = 7,
    SDR_ERR_DATA_RESPONSE_TIMEOUT       = 8,
    SDR_ERR_RX_OVERRUN                  = 9,
    SDR_ERR_INVALID_DMA_DIRECTION       = 10,
    SDR_ERR_CARD_IS_BUSY                = 11,
    SDR_ERR_CARD_NOT_READY              = 12,
    SDR_ERR_CARD_UNPLUGGED              = 13,
} SdResult_t;

/*
 * This function must be called after SD card driver was initialized, card is detected and before it can be used.
 * 
 * @param pCardStatus optional card status which can be used to get more information in case result is SDR_ERR_CARD_STATUS_ERROR (ignored if NULL).
 * 
 * @param returns SDR_OK if initialization was successful, otherwise it can return pretty much any value from SdResult_t enum.
 */
SdResult_t initializeSdCard(uint32_t* pCardStatus);

/*
 * Should be implemented by API consumer. Function should read SD card detect pin.
 * Default implementation assumes that SD card is always present.
 */
bool __attribute__((weak)) isSdCardPresent();

/*
 * @return block size which must be used during read/write operations.
 */
uint32_t getSdCardBlockSizeInBytes();

/*
 * @return maximum block index which can be used in read/write operations (minimum block id is 0).
 */
uint32_t getSdCardMaxBlockIndex();

/*
 * Reads blocks from SD card to memory (asynchronously).
 *
 * Note: startBlockIndex + numberOfBlocks <= getSdCardNumberOfBlocks().
 *
 * @param pBlocksBuffer pointer to a buffer of size numberOfBlocks * getSdCardBlockSizeInBytes().
 * @param startBlockIndex block index starting from which data should be read.
 * @param numberOfBlocks number of blocks to be read.
 * @param pCardStatus optional card status which can be used to get more information in case result is SDR_ERR_CARD_STATUS_ERROR (ignored if NULL).
 * 
 * @param returns SDR_OK if initialization was successful, otherwise it can return pretty much any value from SdResult_t enum.
 */
SdResult_t readBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startBlockIndex, uint32_t numberOfBlocks, uint32_t* pCardStatus);

/*
 * Writes blocks from memory to SD card (asynchronously).
 *
 * Note: startBlockIndex + numberOfBlocks <= getSdCardNumberOfBlocks().
 *
 * @param pBlocksBuffer pointer to a buffer of size numberOfBlocks * getSdCardBlockSizeInBytes().
 * @param startBlockIndex block index starting to which data should be written.
 * @param numberOfBlocks number of blocks to be write.
 * @param pCardStatus optional card status which can be used to get more information in case result is SDR_ERR_CARD_STATUS_ERROR (ignored if NULL).
 * 
 * @param returns SDR_OK if initialization was successful, otherwise it can return pretty much any value from SdResult_t enum.
 */
SdResult_t writeBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startBlockIndex, uint32_t numberOfBlocks, uint32_t* pCardStatus);
