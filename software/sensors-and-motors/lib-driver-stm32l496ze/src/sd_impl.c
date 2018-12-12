/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include <stdatomic.h>

#include "stm32/stm32l496xx.h"
#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_dma.h"
#include "stm32/stm32l4xx_ll_rcc.h"
#include "stm32/stm32l4xx_ll_sdmmc.h"

#include "global_data.h"

#include "sd.h"

static void initializeSdMmc();
static void initializeSdDma();

static SdResult_t powerOn();
static SdResult_t initializeSdCard(volatile SdCard_t* pSdCard);
static SdResult_t decodeSdCardV2SpecificData(uint32_t pCsd[4], volatile SdCard_t* pSdCard);
static SdResult_t loadScrRegister(uint16_t relativeAddress, uint32_t pScr[2]);
static SdResult_t turnOn4BitBus(uint16_t relativeAddress);

static uint8_t SD_GetCommandResponse();
static uint32_t SD_GetResponse(uint32_t responseId);

// cpsm -> command path state machine
static SdResult_t SD_CmdWithNoResponse(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm);
static SdResult_t SD_CmdWithResponse1(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, uint32_t* pResponse1Result);
static SdResult_t SD_CmdWithResponse2(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, volatile uint32_t pResponse1Result[4]);
static SdResult_t SD_CmdWithResponse3(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm);
static SdResult_t SD_CmdWithResponse6(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, volatile uint16_t* pResponseResult);
static SdResult_t SD_CmdWithResponse7(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm);

static SdResult_t initializeSdDmaDirection(uint32_t direction, void* pBuffer, uint32_t bufferLengthBytes);

void initializeSd()
{
    // messed up with schematics and forgot to add SD card present pin
    // will be fixed in Rev 2, for now assume card is always present
    g_sdCard.status = SDS_CARD_PRESENT;

    initializeSdMmc();
    initializeSdDma();

    if (g_sdCard.status & SDS_CARD_PRESENT)
    {
        SdResult_t sd_result;

        if ((sd_result = powerOn()) != SDR_OK)
        {
            return;
        }
        if ((sd_result = initializeSdCard(&g_sdCard)) != SDR_OK)
        {
            return;
        }

        uint32_t scr[2];

        if ((sd_result = loadScrRegister(g_sdCard.relativeAddress, scr)) != SDR_OK)
        {
            return;
        }

        bool is4BitBusSupported = scr[1] & (1 << 18);

        if (is4BitBusSupported && (sd_result = turnOn4BitBus(g_sdCard.relativeAddress)) != SDR_OK)
        {
            return;
        }

        g_sdCard.status |= SDS_INITIALIZED;
    }
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

SdResult_t powerOn()
{
    SdResult_t sd_result;

    if ((sd_result = SD_CmdWithNoResponse(SDMMC_CMD_GO_IDLE_STATE, 0, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = SD_CmdWithResponse7(SDMMC_CMD_HS_SEND_EXT_CSD, SDMMC_CHECK_PATTERN, SDMMC_RESPONSE_SHORT, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE)) != SDR_OK)
    {
        // version is less than v2.0
        return SDR_CARD_VERSION_NOT_SUPPORTED;
    }
    else
    {
        uint32_t response = 0;
        uint32_t validvoltage = 0;

        while (validvoltage == 0)
        {
            if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_APP_CMD, 0, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
            {
                return sd_result;
            }
            if ((sd_result = SD_CmdWithResponse3(SDMMC_CMD_SD_APP_OP_COND, SDMMC_VOLTAGE_WINDOW_SD | SDMMC_HIGH_CAPACITY, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE)) != SDR_OK)
            {
                return sd_result;
            }
            response = SD_GetResponse(SDMMC_RESP1);
            validvoltage = (((response >> 31U) == 1U) ? 1U : 0U);
        }
    }

    return SDR_OK;
}

SdResult_t initializeSdCard(volatile SdCard_t* pSdCard)
{
    if ((SDMMC1->POWER & SDMMC_POWER_PWRCTRL) == 0)
    {
        return SDR_POWER_DOWN;
    }

    SdResult_t sd_result;

    if ((sd_result = SD_CmdWithResponse2(SDMMC_CMD_ALL_SEND_CID, 0, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, pSdCard->cid)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = SD_CmdWithResponse6(SDMMC_CMD_SET_REL_ADDR, 0, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, &pSdCard->relativeAddress)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (pSdCard->relativeAddress) << 16U);

    uint32_t sdCsd[4];

    if ((sd_result = SD_CmdWithResponse2(SDMMC_CMD_SEND_CSD, sdRelativeCardAddressCmdArg, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, (uint32_t*) sdCsd)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_SEL_DESEL_CARD, sdRelativeCardAddressCmdArg, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
    {
        return sd_result;
    }

    return decodeSdCardV2SpecificData(sdCsd, pSdCard);
}

SdResult_t decodeSdCardV2SpecificData(uint32_t pCsd[4], volatile SdCard_t* pSdCard)
{
    const uint16_t ccc = pCsd[1] >> 20U;

    if ((ccc & MINIMUM_REQUIRED_COMAND_CLASSES) != MINIMUM_REQUIRED_COMAND_CLASSES)
    {
        return SDR_CARD_VERSION_NOT_SUPPORTED;
    }

    const uint32_t read_bl_len = (pCsd[1] >> 16U) & 0xFU;
    const uint8_t version = (pCsd[0] & 0xC0000000U) >> 30U;

    if (version == 0)
    {
        const uint32_t c_size = ((pCsd[1] & 0x000003FFU) << 2U) | ((pCsd[2] & 0xC0000000U) >> 30U);
        const uint8_t c_size_mul = (pCsd[2] & 0x00038000U) >> 15U;
        pSdCard->maxBlockAddress = c_size * (1 << (c_size_mul + 2U));
        pSdCard->status &= ~SDS_HIGH_CAPACITY;
    }
    else if (version == 1)
    {
        const uint32_t c_size = ((pCsd[2] >> 16U) & 0xFFFFU) | ((pCsd[1] & 0x3FU) << 16U);
        pSdCard->maxBlockAddress = c_size << 10;
        pSdCard->status |= SDS_HIGH_CAPACITY;
    }
    else
    {
        return SDR_CARD_VERSION_NOT_SUPPORTED;
    }

    bool isPermanentlyWriteProtected = pCsd[3] & (1 << 13);
    bool isTemporarilyWriteProtected = pCsd[3] & (1 << 12);
    if (isPermanentlyWriteProtected || isTemporarilyWriteProtected)
    {
        pSdCard->status |= SDS_READ_ONLY;
    }
    else
    {
        pSdCard->status &= ~SDS_READ_ONLY;
    }

    return SDR_OK;
}

SdResult_t loadScrRegister(uint16_t relativeAddress, uint32_t pScr[2])
{
    SdResult_t sd_result;

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_SET_BLOCKLEN, 8, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (relativeAddress) << 16U);

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_APP_CMD, sdRelativeCardAddressCmdArg, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
    {
        return sd_result;
    }

    SDMMC1->DTIMER = 0xFFFFFFFFU;
    SDMMC1->DLEN = 8;
    MODIFY_REG(SDMMC1->DCTRL, DCTRL_CLEAR_MASK, SDMMC_DATABLOCK_SIZE_8B |
                                                SDMMC_TRANSFER_DIR_TO_SDMMC |
                                                SDMMC_TRANSFER_MODE_BLOCK |
                                                SDMMC_DPSM_ENABLE);

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_SD_APP_SEND_SCR, 0, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
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
        return SDR_DATA_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_DCRCFAIL);
        return SDR_DATA_CRC_FAIL;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_RXOVERR))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_RXOVERR);
        return SDR_RX_OVERRUN;
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

SdResult_t turnOn4BitBus(uint16_t relativeAddress)
{
    SdResult_t sd_result;

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (relativeAddress) << 16U);

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_APP_CMD, sdRelativeCardAddressCmdArg, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_APP_SD_SET_BUSWIDTH, 2, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
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

uint8_t SD_GetCommandResponse()
{
    return (uint8_t)(SDMMC1->RESPCMD);
}

uint32_t SD_GetResponse(uint32_t responseId)
{
    uint32_t responseAddress = (uint32_t) (&(SDMMC1->RESP1)) + responseId;
    return (*(__IO uint32_t *) responseAddress);
}

SdResult_t SD_CmdWithNoResponse(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_NO | cmdIndex | waitForInterrupt | cpsm);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CMDSENT | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_COMMAND_RESPONSE_TIMEOUT;
    }

    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

    return SDR_OK;
}

SdResult_t SD_CmdWithResponse1(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, uint32_t* pResponse1Result)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_SHORT | cmdIndex | waitForInterrupt | cpsm);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));
  
    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_COMMAND_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL);
        return SDR_COMMAND_CRC_FAIL;
    }
    else if (SD_GetCommandResponse() != cmdIndex)
    {
        return SDR_COMMAND_CRC_FAIL;
    }

    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

    uint32_t response1 = SD_GetResponse(SDMMC_RESP1);

    if (pResponse1Result)
    {
        *pResponse1Result = response1;
    }

    if ((response1 & SDMMC_OCR_ERRORBITS) == SDMMC_ALLZERO)
    {
        return SDR_OK;
    }

    return SDR_RESPONSE1_ERROR;
}

SdResult_t SD_CmdWithResponse2(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, volatile uint32_t pResponse1Result[4])
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_LONG | cmdIndex | waitForInterrupt | cpsm);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_COMMAND_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL);
        return SDR_COMMAND_CRC_FAIL;
    }

    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

    pResponse1Result[0] = SD_GetResponse(SDMMC_RESP1);
    pResponse1Result[1] = SD_GetResponse(SDMMC_RESP2);
    pResponse1Result[2] = SD_GetResponse(SDMMC_RESP3);
    pResponse1Result[3] = SD_GetResponse(SDMMC_RESP4);

    return SDR_OK;
}

SdResult_t SD_CmdWithResponse3(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_SHORT | cmdIndex | waitForInterrupt | cpsm);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_COMMAND_RESPONSE_TIMEOUT;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);
        return SDR_OK;
    }
}

SdResult_t SD_CmdWithResponse6(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, volatile uint16_t* pResponseResult)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_SHORT | cmdIndex | waitForInterrupt | cpsm);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_COMMAND_RESPONSE_TIMEOUT;
    }

    if (SD_GetCommandResponse() != cmdIndex)
    {
        return SDR_COMMAND_CRC_FAIL;
    }

    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

    uint32_t response1 = SD_GetResponse(SDMMC_RESP1);

    if ((response1 & (SDMMC_R6_GENERAL_UNKNOWN_ERROR | SDMMC_R6_ILLEGAL_CMD | SDMMC_R6_COM_CRC_FAILED)) == SDMMC_ALLZERO)
    {
        *pResponseResult = (uint16_t) (response1 >> 16);
        return SDR_OK;
    }
    else if ((response1 & SDMMC_R6_ILLEGAL_CMD) == SDMMC_R6_ILLEGAL_CMD)
    {
        return SDR_ILLEGAL_COMMAND;
    }
    else if ((response1 & SDMMC_R6_COM_CRC_FAILED) == SDMMC_R6_COM_CRC_FAILED)
    {
        return SDR_COMMAND_CRC_FAIL;
    }
    else
    {
        return SDR_UNKNOWN_ERROR;
    }
}

SdResult_t SD_CmdWithResponse7(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, cmdIndex | response | waitForInterrupt | cpsm);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_COMMAND_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL);
        return SDR_COMMAND_CRC_FAIL;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);
        return SDR_OK;
    }
}

SdResult_t initializeSdDmaDirection(uint32_t direction, void* pBuffer, uint32_t bufferLengthBytes)
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
        return SDR_INVALID_DMA_DIRECTION;
    }

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_4, bufferLengthBytes);

    return SDR_OK;
}

SdResult_t readBlocksAsync(uint8_t* pBlocksBuffer, uint32_t startAddress, uint32_t numberOfBlocks)
{
    if (numberOfBlocks == 0)
    {
        return SDR_OK;
    }

    uint8_t oldStatus;

    do
    {
        oldStatus = atomic_load(&g_sdCard.status);

        if ((oldStatus & SDS_CARD_PRESENT) == 0)
        {
            return SDR_CARD_NOT_PRESENT;
        }
        if ((oldStatus & SDS_INITIALIZED) == 0)
        {
            return SDR_CARD_NOT_INITIALIZED;
        }
        if ((oldStatus & SDS_BUSY) != 0)
        {
            return SDR_BUSY;
        }
    } while (!atomic_compare_exchange_weak(&g_sdCard.status, &oldStatus, oldStatus | SDS_BUSY));

    const uint32_t numberOfBytes = numberOfBlocks * BLOCK_SIZE_BYTES;

    SDMMC1->DCTRL = 0U;

    __SDMMC_ENABLE_IT(SDMMC1, SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND);

    SdResult_t sd_result;

    if ((sd_result = initializeSdDmaDirection(LL_DMA_DIRECTION_PERIPH_TO_MEMORY, pBlocksBuffer, numberOfBytes >> 2)) != SDR_OK)
    {
        return sd_result;
    }

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_4);
    __SDMMC_DMA_ENABLE(SDMMC1);

    if ((g_sdCard.status & SDS_HIGH_CAPACITY) == 0)
    {
        startAddress *= BLOCK_SIZE_BYTES;
    }

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_SET_BLOCKLEN, BLOCK_SIZE_BYTES, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
    {
        return sd_result;
    }

    SDMMC1->DTIMER = 0xFFFFFFFFU;  // TODO need to calculate timeout based on amount of data sent and speed (check SD specs);
    SDMMC1->DLEN = numberOfBytes;
    MODIFY_REG(SDMMC1->DCTRL, DCTRL_CLEAR_MASK, SDMMC_DATABLOCK_SIZE_512B   |
                                                SDMMC_TRANSFER_DIR_TO_SDMMC |
                                                SDMMC_TRANSFER_MODE_BLOCK   |
                                                SDMMC_DPSM_ENABLE);

    if (numberOfBlocks > 1)
    {
        if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_READ_MULT_BLOCK, startAddress, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
        {
            return sd_result;
        }
    }
    else
    {
        if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_READ_SINGLE_BLOCK, startAddress, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
        {
            return sd_result;
        }
    }

    return SDR_OK;
}

void clearSdFlag(volatile SdCard_t* pSdCard, uint8_t statusFlagsMask)
{
    atomic_fetch_and(&pSdCard->status, ~statusFlagsMask);
}

void SDMMC1_IRQHandler()
{
    // TODO
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DATAEND);
}

void DMA2_Channel4_IRQHandler()
{
    if (LL_DMA_IsActiveFlag_TC4(DMA2))
    {
        LL_DMA_ClearFlag_GI4(DMA2);
        LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_4);
        __SDMMC_DMA_DISABLE(SDMMC1);
        clearSdFlag(&g_sdCard, SDS_BUSY);
        // TODO enque user
    }
    if (LL_DMA_IsActiveFlag_TE4(DMA2))
    {
        LL_DMA_ClearFlag_GI4(DMA2);
    }
}
