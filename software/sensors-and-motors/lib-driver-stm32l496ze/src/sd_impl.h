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

typedef enum
{
    SDS_NONE            = 0x00,
    SDS_CARD_PRESENT    = 0x01,
    SDS_INITIALIZED     = 0x02,
    SDS_HIGH_CAPACITY   = 0x04,
    SDS_ALLOWS_WRITE    = 0x08,
    SDS_BUSY            = 0x10,
} SdStatus_t;

typedef struct
{
    uint8_t status;
    uint16_t relativeAddress;
    uint32_t maxBlockAddress;
    uint32_t cid[4];
} SdCard_t;

/*
 * Call this before using any other SD functions.
 */
void initializeSd();

/*
 * Marks SD card as busy given that it has specified status flags set. If card is already busy function will fail.
 * 
 * @param pSdCard pointer to SD card state.
 * @param statusFlagsMaskWhichMustBePresent flugs which must be set for the SD card to be marked as busy (if it includes SDS_BUSY this function will always fail).
 */
SdResult_t markSdCardAsBusy(volatile SdCard_t* pSdCard, uint8_t statusFlagsMaskWhichMustBePresent);

/*
 * Clears given flag from SD card status (operation is atomic and ignores any flags other than provided ones).
 * 
 * @param pSdCard pointer to SD card state
 * @param statusFlagsMask bitwise mask of status flags to clear (see SdStatus_t).
 */
void clearSdFlag(volatile SdCard_t* pSdCard, uint8_t statusFlagsMask);
