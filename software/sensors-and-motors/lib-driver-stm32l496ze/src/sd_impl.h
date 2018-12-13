/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "sd.h"

// will use 512 across all cards (SD, HC, XC)
#define BLOCK_SIZE_BYTES 512

// minimum required command classes = 0, 2, 4, 5
#define MINIMUM_REQUIRED_COMAND_CLASSES 0b110101U

// see ACMD6 command description in the spec
#define BUS_WIDTH_1 0b00U
#define BUS_WIDTH_4 0b10U

typedef enum
{
    SDS_NONE            = 0x00,
    SDS_INITIALIZED     = 0x01,
    SDS_HIGH_CAPACITY   = 0x02,
    SDS_ALLOWS_WRITE    = 0x04,
    SDS_BUSY            = 0x08,
} SdStatus_t;

typedef struct
{
    uint8_t flags;
    uint8_t sdCardChangeDetectionIdx;
} __attribute__((__packed__)) SdCardStatus_t;

typedef union
{
    uint16_t packedStatus;
    SdCardStatus_t status;
} SdCardStatusUnion_t;

typedef struct
{
    SdCardStatusUnion_t statusUnion;
    uint16_t relativeAddress;
    uint32_t maxBlockIndex;
    uint32_t cid[4];
} SdCard_t;

/*
 * Call ONCE this before using any other SD functions.
 */
void initializeSd();

/*
 * Initializes SDMMC1 STM32L4 hardware.
 */
void initializeSdMmc();

/*
 * Initializes SDMMC1 related DMA.
 */
void initializeSdDma();

/*
 * Marks SD card as busy given that it has specified status flags set. If card is already busy function will fail.
 * 
 * @param pSdCard pointer to SD card state.
 * @param statusFlagsMaskWhichMustBePresent flags (see SdStatus_t) which must be set for the SD card to be marked as busy (if it includes SDS_BUSY this function will always fail).
 * @param newFlag optional pointer which will have new flags if function returns SDR_OK (ignored if it is NULL).
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_CARD_IS_BUSY if card is already busy.
 * @return SDR_ERR_CARD_NOT_READY if card doesn't have required status flags.
 */
SdResult_t markSdCardAsBusy(volatile SdCard_t* pSdCard, uint8_t statusFlagsMaskWhichMustBePresent, uint8_t* newFlag);

/*
 * Clears given flag from SD card status (operation is atomic and ignores any flags other than provided ones).
 * 
 * @param pSdCard pointer to SD card state.
 * @param statusFlagsMask bitwise mask of status flags to clear (see SdStatus_t).
 */
void clearSdFlag(volatile SdCard_t* pSdCard, uint8_t statusFlagsMask);

/*
 * Resets SD card so we can start initialization process.
 * 
 * @return SDR_OK if function succeeded otherwise error code.
 */
SdResult_t sdCardReset();

/*
 * Validates SD card operation conditions (operating voltage).
 * 
 * @param pCardStatus optional pointer to SD card status (see spec for definitions). This value should only be accessed
 *          if function returns SDR_OK or SDR_ERR_CARD_STATUS_ERROR. If NULL is passed it will be ignored.
 *
 * @return SDR_OK if function succeeded otherwise error code.
 */
SdResult_t sdCardOperatingConditionValidation(uint32_t* pCardStatus);

/*
 * Performs card identification. Gets card identification (CID), card specific data (CSD), new card relative address.
 * 
 * @param pCsd pointer to 4 item array which will hold CSD (only if return value is SDR_OK).
 * @param pCid pointer to 4 item array which will hold CID (only if return value is SDR_OK).
 * @param pNewCardRelativeAddress will return new card relative address  (only if return value is SDR_OK).
 * @param pCardStatus optional pointer to SD card status (see spec for definitions). This value should only be accessed
 *          if function returns SDR_OK or SDR_ERR_CARD_STATUS_ERROR. If NULL is passed it will be ignored.
 *
 * @return SDR_OK if function succeeded otherwise error code.
 */
SdResult_t sdCardIdentification(uint32_t* pCsd, uint32_t* pCid, uint16_t* pNewCardRelativeAddress, uint32_t* pCardStatus);

/*
 * Loads SD card configuration register (SCR).
 * 
 * @param relativeAddress card relative address.
 * @param pScr pointer to 2 item array which will hold SCR (only if return value is SDR_OK).
 * @param pCardStatus optional pointer to SD card status (see spec for definitions). This value should only be accessed
 *          if function returns SDR_OK or SDR_ERR_CARD_STATUS_ERROR. If NULL is passed it will be ignored.
 *
 * @return SDR_OK if function succeeded otherwise error code.
 */
SdResult_t sdCardLoadScrRegister(uint16_t relativeAddress, uint32_t* pScr, uint32_t* pCardStatus);

/*
 * Turns on 4-bit wide bus for SD card. Should only be called if card supports this mode (see spec for details).
 * 
 * @param relativeAddress card relative address.
 * @param pCardStatus optional pointer to SD card status (see spec for definitions). This value should only be accessed
 *          if function returns SDR_OK or SDR_ERR_CARD_STATUS_ERROR. If NULL is passed it will be ignored.
 * 
 * @return SDR_OK if function succeeded otherwise error code.
 */
SdResult_t sdcardTurnOn4BitBus(uint16_t relativeAddress, uint32_t* pCardStatus);

/*
 * Decodes CSD register.
 * 
 * @param pCsd 4 byte array which holds CSD register. Index 0 holds bits [127:96], index 1 holds bits [95:64], etc.
 * @param pDecodedStatus will return card status (SDS_HIGH_CAPACITY/SDS_ALLOWS_WRITE).
 * @param pDecodedMaxBlockIndex will return maximum block index which can be used in read/write.
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_CARD_NOT_SUPPORTED if card isn't supported (unknown CSD register structure).
 */
SdResult_t decodeSdCardSpecificData(uint32_t* pCsd, uint8_t* pDecodedStatus, uint32_t* pDecodedMaxBlockIndex);

/*
 * Configures DMA for RX or TX.
 * 
 * @param direction is either:
 *          LL_DMA_DIRECTION_PERIPH_TO_MEMORY - RX
 *          LL_DMA_DIRECTION_MEMORY_TO_PERIPH - TX
 * @param pBuffer is buffer which either goinr to receive data (RX) or provide data (TX). Buffer must be of length bufferLengthBytes.
 * @param bufferLengthBytes length of the buffer (must be a multiple of 4 as SDMMC1 DMA works with WORDs).
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_INVALID_DMA_DIRECTION if invalid DMA direction was passed as a 1st argument.
 */
SdResult_t initializeSdDmaChannel(uint32_t direction, void* pBuffer, uint32_t bufferLengthBytes);

/*
 * Sends SD command which expect no response. See spec for commands list and appropriate arguments.
 * 
 * @param cmdIndex command index.
 * @param argument command argument.
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_COMMAND_RESPONSE_TIMEOUT if SD command timedout.
 */
SdResult_t sdCmdWithNoResponse(uint32_t cmdIndex, uint32_t argument);

/*
 * Sends SD command which expect R1 response. See spec for commands list and appropriate arguments.
 * 
 * @param cmdIndex command index.
 * @param argument command argument.
 * @param pCardStatus optional pointer to SD card status (see spec for definitions). This value should only be accessed
 *          if function returns SDR_OK or SDR_ERR_CARD_STATUS_ERROR. If NULL is passed it will be ignored.
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_COMMAND_CRC_FAIL command CRC check failed.
 * @return SDR_ERR_CARD_STATUS_ERROR SD card has some errors after command execution.
 * @return SDR_ERR_COMMAND_RESPONSE_TIMEOUT if SD command timedout.
 * @return SDR_ERR_COMMAND_RESPONSE_INDEX_FAIL response has command index different from the one which was sent.
 */
SdResult_t sdCmdWithResponse1(uint32_t cmdIndex, uint32_t argument, uint32_t* pCardStatus);

/*
 * Sends SD command which expect R2 response. See spec for commands list and appropriate arguments.
 * 
 * @param cmdIndex command index.
 * @param argument command argument.
 * @param pResponseRegisterValue uint32_t[4] array. See specs for response value based on command index. This value should
 *          only be used if function returned SDR_OK.
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_COMMAND_CRC_FAIL command CRC check failed.
 * @return SDR_ERR_COMMAND_RESPONSE_TIMEOUT if SD command timedout.
 */
SdResult_t sdCmdWithResponse2(uint32_t cmdIndex, uint32_t argument, uint32_t* pResponseRegisterValue);

/*
 * Sends SD command which expect R3 response. See spec for commands list and appropriate arguments.
 * 
 * @param cmdIndex command index.
 * @param argument command argument.
 * @param pOcrRegister pointer to uint32_t which will have SD card OCR register value. This value should only be used if
 *          function returned SDR_OK.
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_COMMAND_RESPONSE_TIMEOUT if SD command timedout.
 */
SdResult_t sdCmdWithResponse3(uint32_t cmdIndex, uint32_t argument, uint32_t* pOcrRegister);

/*
 * Sends SD command which expect R6 response. See spec for commands list and appropriate arguments.
 * 
 * @param cmdIndex command index.
 * @param argument command argument.
 * @param pNewCardRelativeAddress pointer to new SD card relative address. This value should only be used if function
 *          returned SDR_OK.
 * @param pCardStatus optional pointer to SD card status (see spec for definitions). This value should only be accessed
 *          if function returns SDR_OK or SDR_ERR_CARD_STATUS_ERROR. If NULL is passed it will be ignored.
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_COMMAND_CRC_FAIL command CRC check failed.
 * @return SDR_ERR_CARD_STATUS_ERROR SD card has some errors after command execution.
 * @return SDR_ERR_COMMAND_RESPONSE_TIMEOUT if SD command timedout.
 * @return SDR_ERR_COMMAND_RESPONSE_INDEX_FAIL response has command index different from the one which was sent.
 */
SdResult_t sdCmdWithResponse6(uint32_t cmdIndex, uint32_t argument, uint16_t* pNewCardRelativeAddress, uint32_t* pCardStatus);

/*
 * Sends SD command which expect R7 response. See spec for commands list and appropriate arguments.
 * 
 * @param cmdIndex command index.
 * @param argument command argument.
 * 
 * @return SDR_OK if function succeeded.
 * @return SDR_ERR_COMMAND_CRC_FAIL command CRC check failed.
 * @return SDR_ERR_COMMAND_RESPONSE_TIMEOUT if SD command timedout.
 * @return SDR_ERR_COMMAND_RESPONSE_INDEX_FAIL response has command index different from the one which was sent.
 */
SdResult_t sdCmdWithResponse7(uint32_t cmdIndex, uint32_t argument);
