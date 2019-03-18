/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *                                                                             *
 * For documenation see Simplified Specifications at https://www.sdcard.org    *
 * Relevant ones:                                                              *
 *  Part 1 Simplified - Physical Layer Simplified Specification                *
 *                                                                             *
 * Implemented version is 6.00 Aug. 29, 2018                                   *
 *******************************************************************************/

// TODO
// 1) Need to go over spec and check if I execute all commands correctly (current one is based on STM32L4 drivers)
// 2) Need to go over timeouts for block read and block write (currently they are set to max value)
// 3) Need to add SD card detect pin handling
// 4) Need to check voltage level trials retry count (timeout?)
// 5) Need to add interrupt handling for errors and DMA (proper one)

#include <stdatomic.h>

#include "stm32/stm32l496xx.h"
#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_dma.h"
#include "stm32/stm32l4xx_ll_rcc.h"
#include "stm32/stm32l4xx_ll_sdmmc.h"

#include "global_data.h"

#include "sd.h"

bool __attribute__((weak)) isSdCardPresent()
{
    return true;
}

void initializeSd()
{
    initializeSdMmc();
    initializeSdDma();
}

void initializeSdMmc()
{
    LL_RCC_SetSDMMCClockSource(LL_RCC_SDMMC1_CLKSOURCE_HSI48);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SDMMC1);

    // use default configuration for power on
    MODIFY_REG(SDMMC1->CLKCR, CLKCR_CLEAR_MASK, SDMMC_CLOCK_EDGE_RISING |
                                                SDMMC_CLOCK_BYPASS_DISABLE |
                                                SDMMC_CLOCK_POWER_SAVE_DISABLE |
                                                SDMMC_BUS_WIDE_1B |
                                                SDMMC_HARDWARE_FLOW_CONTROL_ENABLE |
                                                SDMMC_TRANSFER_CLK_DIV);

    NVIC_SetPriority(SDMMC1_IRQn, 3);
    NVIC_EnableIRQ(SDMMC1_IRQn);

    __SDMMC_DISABLE(SDMMC1);
    SDMMC1->POWER = SDMMC_POWER_PWRCTRL;
    __SDMMC_ENABLE(SDMMC1);
}

void initializeSdDma()
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_CHANNEL_4, LL_DMA_REQUEST_7);

    NVIC_SetPriority(DMA2_Channel4_IRQn, 2);
    NVIC_EnableIRQ(DMA2_Channel4_IRQn);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_4);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_4);
}

SdResult_t markSdCardAsBusy(volatile SdCard_t* pSdCard, uint8_t statusFlagsMaskWhichMustBePresent, uint8_t* newFlag)
{
    uint8_t oldStatus;

    do
    {
        oldStatus = atomic_load(&g_sdCard.statusUnion.status.flags);

        if ((oldStatus & statusFlagsMaskWhichMustBePresent) != statusFlagsMaskWhichMustBePresent)
        {
            return SDR_ERR_CARD_NOT_READY;
        }
        if ((oldStatus & SDS_BUSY) != 0)
        {
            return SDR_ERR_CARD_IS_BUSY;
        }
    } while (!atomic_compare_exchange_weak(&g_sdCard.statusUnion.status.flags, &oldStatus, oldStatus | SDS_BUSY));

    if (newFlag)
    {
        *newFlag = oldStatus | SDS_BUSY;
    }

    return SDR_OK;
}

void clearSdFlag(volatile SdCard_t* pSdCard, uint8_t statusFlagsMask)
{
    atomic_fetch_and(&pSdCard->statusUnion.status.flags, ~statusFlagsMask);
}

SdResult_t initializeSdCard(uint32_t* pCardStatus)
{
    SdResult_t sd_result;

    if ((sd_result = markSdCardAsBusy(&g_sdCard, 0, NULL)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    // in case card is unplugged and then maybe plugged back in (which is detected by interrupt with priority 0) we need to make sure we don't mark card as initialized.
    // see atomic_compare_exchange_strong below.
    uint16_t oldPackedStatus = atomic_load(&g_sdCard.statusUnion.packedStatus);

    if ((sd_result = sdCardReset()) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    if ((sd_result = sdCardOperatingConditionValidation(pCardStatus)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    uint32_t pCid[4];
    uint16_t newRelativeAddress;
    uint32_t pTempRegisterStorage[4];

    if ((sd_result = sdCardIdentification(pTempRegisterStorage, pCid, &newRelativeAddress, pCardStatus)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    uint8_t status;
    uint32_t maxBlockIndex;

    if ((sd_result = decodeSdCardSpecificData(pTempRegisterStorage, &status, &maxBlockIndex)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    if ((sd_result = sdCardLoadScrRegister(newRelativeAddress, pTempRegisterStorage, pCardStatus)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    bool is4BitBusSupported = pTempRegisterStorage[1] & (1 << 18);

    if (is4BitBusSupported && (sd_result = sdcardTurnOn4BitBus(newRelativeAddress, pCardStatus)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    g_sdCard.cid[0] = pCid[0];
    g_sdCard.cid[1] = pCid[1];
    g_sdCard.cid[2] = pCid[2];
    g_sdCard.cid[3] = pCid[3];
    g_sdCard.maxBlockIndex = maxBlockIndex;
    g_sdCard.relativeAddress = newRelativeAddress;

    SdCardStatusUnion_t newStatus;
    newStatus.packedStatus = oldPackedStatus;
    newStatus.status.flags = status | SDS_INITIALIZED;
    newStatus.status.sdCardChangeDetectionIdx = newStatus.status.sdCardChangeDetectionIdx + 1;

    if (!atomic_compare_exchange_strong(&g_sdCard.statusUnion.packedStatus, &oldPackedStatus, newStatus.packedStatus))
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return SDR_ERR_CARD_UNPLUGGED;
    }
    else
    {
        return SDR_OK;
    }
}

uint32_t getSdCardBlockSizeInBytes()
{
    return BLOCK_SIZE_BYTES;
}

uint32_t getSdCardMaxBlockIndex()
{
    return g_sdCard.maxBlockIndex;
}

SdResult_t sdCardReset()
{
    SdResult_t sd_result;

    if ((sd_result = sdCmdWithNoResponse(SDMMC_CMD_GO_IDLE_STATE, 0)) != SDR_OK)
    {
        return sd_result;
    }

    return SDR_OK;
}

SdResult_t sdCardOperatingConditionValidation(uint32_t* pCardStatus)
{
    SdResult_t sd_result;

    uint32_t response;
    uint32_t cmdArgument;
    uint32_t validVoltage = 0;

    if ((sd_result = sdCmdWithResponse7(SDMMC_CMD_HS_SEND_EXT_CSD, SDMMC_CHECK_PATTERN)) != SDR_OK)
    {
        cmdArgument = SDMMC_VOLTAGE_WINDOW_SD | SDMMC_STD_CAPACITY;
    }
    else
    {
        cmdArgument = SDMMC_VOLTAGE_WINDOW_SD | SDMMC_HIGH_CAPACITY;
    }

    while (validVoltage == 0)
    {
        if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_APP_CMD, 0, pCardStatus)) != SDR_OK)
        {
            return sd_result;
        }
        if ((sd_result = sdCmdWithResponse3(SDMMC_CMD_SD_APP_OP_COND, cmdArgument, &response)) != SDR_OK)
        {
            return sd_result;
        }
        validVoltage = (((response >> 31U) == 1U) ? 1U : 0U);
    }

    return SDR_OK;
}

SdResult_t sdCardIdentification(uint32_t* pCsd, uint32_t* pCid, uint16_t* pNewCardRelativeAddress, uint32_t* pCardStatus)
{
    if ((SDMMC1->POWER & SDMMC_POWER_PWRCTRL) == 0)
    {
        return SDR_ERR_CARD_POWER_DOWN;
    }

    SdResult_t sd_result;

    if ((sd_result = sdCmdWithResponse2(SDMMC_CMD_ALL_SEND_CID, 0, pCid)) != SDR_OK)
    {
        return sd_result;
    }

    uint16_t newCardRelativeAddress;

    if ((sd_result = sdCmdWithResponse6(SDMMC_CMD_SET_REL_ADDR, 0, &newCardRelativeAddress, pCardStatus)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (newCardRelativeAddress) << 16U);

    if ((sd_result = sdCmdWithResponse2(SDMMC_CMD_SEND_CSD, sdRelativeCardAddressCmdArg, pCsd)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_SEL_DESEL_CARD, sdRelativeCardAddressCmdArg, pCardStatus)) != SDR_OK)
    {
        return sd_result;
    }

    *pNewCardRelativeAddress = newCardRelativeAddress;

    return SDR_OK;
}

SdResult_t sdCardLoadScrRegister(uint16_t relativeAddress, uint32_t* pScr, uint32_t* pCardStatus)
{
    SdResult_t sd_result;

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_SET_BLOCKLEN, 8 /* 8 bytes, 2 x uint32_t is the length of SCR register */, pCardStatus)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (relativeAddress) << 16U);

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_APP_CMD, sdRelativeCardAddressCmdArg, pCardStatus)) != SDR_OK)
    {
        return sd_result;
    }

    SDMMC1->DTIMER = 0xFFFFFFFFU;
    SDMMC1->DLEN = 8;
    MODIFY_REG(SDMMC1->DCTRL, DCTRL_CLEAR_MASK, SDMMC_DATABLOCK_SIZE_8B |
                                                SDMMC_TRANSFER_DIR_TO_SDMMC |
                                                SDMMC_TRANSFER_MODE_BLOCK |
                                                SDMMC_DPSM_ENABLE);

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_SD_APP_SEND_SCR, 0, pCardStatus)) != SDR_OK)
    {
        return sd_result;
    }

    uint32_t index = 0;
    uint32_t scr[2] = {0, 0};

    while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
    {
        if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_RXDAVL))
        {
            scr[index++] = SDMMC1->FIFO;
        }
    }

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_DTIMEOUT);
        return SDR_ERR_DATA_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_DCRCFAIL);
        return SDR_ERR_DATA_CRC_FAIL;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_RXOVERR))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_RXOVERR);
        return SDR_ERR_RX_OVERRUN;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_DATA_FLAGS);

        pScr[0] = (((scr[1] & SDMMC_0TO7BITS) << 24)  | ((scr[1] & SDMMC_8TO15BITS) << 8) | \
                   ((scr[1] & SDMMC_16TO23BITS) >> 8) | ((scr[1] & SDMMC_24TO31BITS) >> 24));
        pScr[1] = (((scr[0] & SDMMC_0TO7BITS) << 24)  | ((scr[0] & SDMMC_8TO15BITS) << 8) | \
                   ((scr[0] & SDMMC_16TO23BITS) >> 8) | ((scr[0] & SDMMC_24TO31BITS) >> 24));

        return SDR_OK;
    }
}

SdResult_t sdcardTurnOn4BitBus(uint16_t relativeAddress, uint32_t* pCardStatus)
{
    SdResult_t sd_result;

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (relativeAddress) << 16U);

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_APP_CMD, sdRelativeCardAddressCmdArg, pCardStatus)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_APP_SD_SET_BUSWIDTH, BUS_WIDTH_4, pCardStatus)) != SDR_OK)
    {
        return sd_result;
    }

    // configuration to be used when robot is running (4 bit wide bus and power saving enabled)
    MODIFY_REG(SDMMC1->CLKCR, CLKCR_CLEAR_MASK, SDMMC_CLOCK_EDGE_RISING |
                                                SDMMC_CLOCK_BYPASS_DISABLE |
                                                SDMMC_CLOCK_POWER_SAVE_ENABLE |
                                                SDMMC_BUS_WIDE_4B |
                                                SDMMC_HARDWARE_FLOW_CONTROL_ENABLE |
                                                SDMMC_TRANSFER_CLK_DIV);

    return SDR_OK;
}

SdResult_t decodeSdCardSpecificData(uint32_t* pCsd, uint8_t* pDecodedStatus, uint32_t* pDecodedMaxBlockIndex)
{
    const uint16_t ccc = pCsd[1] >> 20U;

    if ((ccc & MINIMUM_REQUIRED_COMAND_CLASSES) != MINIMUM_REQUIRED_COMAND_CLASSES)
    {
        return SDR_ERR_CARD_NOT_SUPPORTED;
    }

    uint8_t newStatus = 0;
    uint32_t newMaxBlockIndex;

    const uint32_t read_bl_len = (pCsd[1] >> 16U) & 0xFU;
    const uint8_t version = (pCsd[0] & 0xC0000000U) >> 30U;

    if (version == 0)
    {
        const uint32_t c_size = ((pCsd[1] & 0x000003FFU) << 2U) | ((pCsd[2] & 0xC0000000U) >> 30U);
        const uint8_t c_size_mul = (pCsd[2] & 0x00038000U) >> 15U;
        newMaxBlockIndex = c_size * (1 << (c_size_mul + 2U));
    }
    else if (version == 1)
    {
        const uint32_t c_size = ((pCsd[2] >> 16U) & 0xFFFFU) | ((pCsd[1] & 0x3FU) << 16U);
        newStatus |= SDS_HIGH_CAPACITY;
        newMaxBlockIndex = c_size << 10 /* mul 1024 (to convert kilo-blocks to blocks) */;
    }
    else
    {
        return SDR_ERR_CARD_NOT_SUPPORTED;
    }

    bool isPermanentlyWriteProtected = pCsd[3] & (1 << 13);
    bool isTemporarilyWriteProtected = pCsd[3] & (1 << 12);
    if (!isPermanentlyWriteProtected && !isTemporarilyWriteProtected)
    {
        newStatus |= SDS_ALLOWS_WRITE;
    }

    *pDecodedStatus = newStatus;
    *pDecodedMaxBlockIndex = newMaxBlockIndex;

    return SDR_OK;
}

SdResult_t initializeSdDmaChannel(uint32_t direction, void* pBuffer, uint32_t bufferLengthBytes)
{
    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_4);

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_4,
                          direction |
                          LL_DMA_PRIORITY_VERYHIGH |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_WORD |
                          LL_DMA_MDATAALIGN_WORD);

    if (direction == LL_DMA_DIRECTION_PERIPH_TO_MEMORY)
    {
        LL_DMA_ConfigAddresses(DMA2,
                               LL_DMA_CHANNEL_4,
                               (uint32_t) &SDMMC1->FIFO,
                               (uint32_t) pBuffer,
                               direction);
    }
    else if (direction == LL_DMA_DIRECTION_MEMORY_TO_PERIPH)
    {
        LL_DMA_ConfigAddresses(DMA2,
                               LL_DMA_CHANNEL_4,
                               (uint32_t) pBuffer,
                               (uint32_t) &SDMMC1->FIFO,
                               direction);
    }
    else
    {
        return SDR_ERR_INVALID_DMA_DIRECTION;
    }

    // DMA reads words (4x bytes), so need to divide bufferLengthBytes by 4
    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_4, bufferLengthBytes >> 2);

    return SDR_OK;
}

SdResult_t readBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startBlockIndex, uint32_t numberOfBlocks, uint32_t* pCardStatus)
{
    if (numberOfBlocks == 0)
    {
        return SDR_OK;
    }

    uint8_t flags;
    SdResult_t sd_result;

    if ((sd_result = markSdCardAsBusy(&g_sdCard, SDS_INITIALIZED, &flags)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t numberOfBytes = numberOfBlocks * BLOCK_SIZE_BYTES;

    SDMMC1->DCTRL = 0U;

    __SDMMC_ENABLE_IT(SDMMC1, SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR);

    if ((sd_result = initializeSdDmaChannel(LL_DMA_DIRECTION_PERIPH_TO_MEMORY, pBlocksBuffer, numberOfBytes)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_4);
    __SDMMC_DMA_ENABLE(SDMMC1);

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_SET_BLOCKLEN, BLOCK_SIZE_BYTES, pCardStatus)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    SDMMC1->DTIMER = 0xFFFFFFFFU; // TODO need to calculate timeout based on amount of data sent and speed (check SD specs);
    SDMMC1->DLEN = numberOfBytes;
    MODIFY_REG(SDMMC1->DCTRL, DCTRL_CLEAR_MASK, SDMMC_DATABLOCK_SIZE_512B   |
                                                SDMMC_TRANSFER_DIR_TO_SDMMC |
                                                SDMMC_TRANSFER_MODE_BLOCK   |
                                                SDMMC_DPSM_ENABLE);

    if ((flags & SDS_HIGH_CAPACITY) == 0)
    {
        startBlockIndex *= BLOCK_SIZE_BYTES;
    }

    if (numberOfBlocks > 1)
    {
        if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_READ_MULT_BLOCK, startBlockIndex, pCardStatus)) != SDR_OK)
        {
            clearSdFlag(&g_sdCard, SDS_BUSY);
            return sd_result;
        }
    }
    else
    {
        if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_READ_SINGLE_BLOCK, startBlockIndex, pCardStatus)) != SDR_OK)
        {
            clearSdFlag(&g_sdCard, SDS_BUSY);
            return sd_result;
        }
    }

    return SDR_OK;
}

SdResult_t writeBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startBlockIndex, uint32_t numberOfBlocks, uint32_t* pCardStatus)
{
    if (numberOfBlocks == 0)
    {
        return SDR_OK;
    }

    uint8_t flags;
    SdResult_t sd_result;

    if ((sd_result = markSdCardAsBusy(&g_sdCard, SDS_INITIALIZED | SDS_ALLOWS_WRITE, &flags)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t numberOfBytes = numberOfBlocks * BLOCK_SIZE_BYTES;

    SDMMC1->DCTRL = 0U;

    __SDMMC_ENABLE_IT(SDMMC1, SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR);

    if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_SET_BLOCKLEN, BLOCK_SIZE_BYTES, pCardStatus)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    if ((flags & SDS_HIGH_CAPACITY) == 0)
    {
        startBlockIndex *= BLOCK_SIZE_BYTES;
    }

    if (numberOfBlocks > 1)
    {
        if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_WRITE_MULT_BLOCK, startBlockIndex, pCardStatus)) != SDR_OK)
        {
            clearSdFlag(&g_sdCard, SDS_BUSY);
            return sd_result;
        }
    }
    else
    {
        if ((sd_result = sdCmdWithResponse1(SDMMC_CMD_WRITE_SINGLE_BLOCK, startBlockIndex, pCardStatus)) != SDR_OK)
        {
            clearSdFlag(&g_sdCard, SDS_BUSY);
            return sd_result;
        }
    }

    __SDMMC_DMA_ENABLE(SDMMC1);

    if ((sd_result = initializeSdDmaChannel(LL_DMA_DIRECTION_MEMORY_TO_PERIPH, pBlocksBuffer, numberOfBytes)) != SDR_OK)
    {
        clearSdFlag(&g_sdCard, SDS_BUSY);
        return sd_result;
    }

    SDMMC1->DTIMER = 0xFFFFFFFFU; // TODO need to calculate timeout based on amount of data sent and speed (check SD specs);
    SDMMC1->DLEN = numberOfBytes;
    MODIFY_REG(SDMMC1->DCTRL, DCTRL_CLEAR_MASK, SDMMC_DATABLOCK_SIZE_512B  |
                                                SDMMC_TRANSFER_DIR_TO_CARD |
                                                SDMMC_TRANSFER_MODE_BLOCK  |
                                                SDMMC_DPSM_ENABLE);

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_4);

    return SDR_OK;
}

void SDMMC1_IRQHandler()
{
    // TODO
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_RXOVERR);
}

void DMA2_Channel4_IRQHandler()
{
    if (LL_DMA_IsActiveFlag_TC4(DMA2))
    {
        LL_DMA_ClearFlag_GI4(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_4);
        __SDMMC_DMA_DISABLE(SDMMC1);
        clearSdFlag(&g_sdCard, SDS_BUSY);
        // TODO enque user event
    }
    if (LL_DMA_IsActiveFlag_TE4(DMA2))
    {
        LL_DMA_ClearFlag_GI4(DMA2);
    }
}
