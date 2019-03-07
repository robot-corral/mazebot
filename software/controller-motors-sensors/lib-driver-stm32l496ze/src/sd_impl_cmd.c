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
// 1) Need to go over all while loops and check if I need to add timeouts

#include "sd_impl.h"

#include "stm32/stm32l496xx.h"
#include "stm32/stm32l4xx_ll_sdmmc.h"

static uint8_t getSdResponseCommandIndex();
static uint32_t getSdResponse(uint32_t responseId);

uint8_t getSdResponseCommandIndex()
{
    // only 1st 6 bits are used
    return (uint8_t) (SDMMC1->RESPCMD);
}

uint32_t getSdResponse(uint32_t responseId)
{
    uint32_t responseAddress = (uint32_t) (&(SDMMC1->RESP1)) + responseId;
    return (*(__IO uint32_t *) responseAddress);
}

SdResult_t sdCmdWithNoResponse(uint32_t cmdIndex, uint32_t argument)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_NO | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE | cmdIndex);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CMDSENT | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_ERR_COMMAND_RESPONSE_TIMEOUT;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);
        return SDR_OK;
    }
}

SdResult_t sdCmdWithResponse1(uint32_t cmdIndex, uint32_t argument, uint32_t* pCardStatus)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_SHORT | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE | cmdIndex);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));
  
    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_ERR_COMMAND_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL);
        return SDR_ERR_COMMAND_CRC_FAIL;
    }
    else if (getSdResponseCommandIndex() != cmdIndex)
    {
        return SDR_ERR_COMMAND_RESPONSE_INDEX_FAIL;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

        const uint32_t cardStatus = getSdResponse(SDMMC_RESP1);

        if (pCardStatus)
        {
            *pCardStatus = cardStatus;
        }

        if ((cardStatus & SDMMC_OCR_ERRORBITS) == SDMMC_ALLZERO)
        {
            return SDR_OK;
        }

        return SDR_ERR_CARD_STATUS_ERROR;
    }
}

SdResult_t sdCmdWithResponse2(uint32_t cmdIndex, uint32_t argument, uint32_t* pResponseRegisterValue)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_LONG | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE | cmdIndex);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_ERR_COMMAND_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL);
        return SDR_ERR_COMMAND_CRC_FAIL;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

        pResponseRegisterValue[0] = getSdResponse(SDMMC_RESP1);
        pResponseRegisterValue[1] = getSdResponse(SDMMC_RESP2);
        pResponseRegisterValue[2] = getSdResponse(SDMMC_RESP3);
        pResponseRegisterValue[3] = getSdResponse(SDMMC_RESP4);

        return SDR_OK;
    }
}

SdResult_t sdCmdWithResponse3(uint32_t cmdIndex, uint32_t argument, uint32_t* pOcrRegister)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_SHORT | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE | cmdIndex);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_ERR_COMMAND_RESPONSE_TIMEOUT;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);
        *pOcrRegister = getSdResponse(SDMMC_RESP1);
        return SDR_OK;
    }
}

SdResult_t sdCmdWithResponse6(uint32_t cmdIndex, uint32_t argument, uint16_t* pNewCardRelativeAddress, uint32_t* pCardStatus)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_SHORT | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE | cmdIndex);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_ERR_COMMAND_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL);
        return SDR_ERR_COMMAND_CRC_FAIL;
    }
    else if (getSdResponseCommandIndex() != cmdIndex)
    {
        return SDR_ERR_COMMAND_RESPONSE_INDEX_FAIL;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

        uint32_t response1 = getSdResponse(SDMMC_RESP1);
        uint32_t cardStatus = (response1 & 0x1FFFU) |
                              (((response1 >> 13U) & 1U) << 19U) |
                              (((response1 >> 14U) & 1U) << 22U) |
                              (((response1 >> 15U) & 1U) << 23U);

        if (pCardStatus)
        {
            *pCardStatus = cardStatus;
        }

        if ((cardStatus & SDMMC_OCR_ERRORBITS) == SDMMC_ALLZERO)
        {
            *pNewCardRelativeAddress = (uint16_t) (response1 >> 16U);
            return SDR_OK;
        }
        else
        {
            return SDR_ERR_CARD_STATUS_ERROR;
        }
    }
}

SdResult_t sdCmdWithResponse7(uint32_t cmdIndex, uint32_t argument)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, SDMMC_RESPONSE_SHORT | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE | cmdIndex);

    do
    {
        // CMD timeout is 64 SDMMC_CK cycles
    } while (!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

    if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);
        return SDR_ERR_COMMAND_RESPONSE_TIMEOUT;
    }
    else if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL))
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CCRCFAIL);
        return SDR_ERR_COMMAND_CRC_FAIL;
    }
    else if (getSdResponseCommandIndex() != cmdIndex)
    {
        return SDR_ERR_COMMAND_RESPONSE_INDEX_FAIL;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);
        return SDR_OK;
    }
}
