#include "stm32/stm32l496xx.h"
#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_dma.h"
#include "stm32/stm32l4xx_ll_rcc.h"
#include "stm32/stm32l4xx_ll_sdmmc.h"

#include "global_data.h"

#include "sd.h"

static void initialize();
static void initializeSdDma();
static void initializeSdDmaDirection(SdDmaDirection_t direction, uint32_t bufferLength);

static SdResult_t powerOn();
static SdResult_t initializeSdCard();
static SdResult_t loadScrRegister();
static SdResult_t turnOn4BitBus();

static bool is4BitBusSupported();

static uint8_t SD_GetCommandResponse();
static uint32_t SD_GetResponse(uint32_t responseId);

// cpsm -> command path state machine
static SdResult_t SD_CmdWithNoResponse(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm);
static SdResult_t SD_CmdWithResponse1(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, uint32_t* pResponse1Result);
static SdResult_t SD_CmdWithResponse2(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, uint32_t pResponse1Result[4]);
static SdResult_t SD_CmdWithResponse3(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm);
static SdResult_t SD_CmdWithResponse6(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, uint16_t* pResponseResult);
// TODO spec doesn't have Response 7 WTF is it?
static SdResult_t SD_CmdWithResponse7(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm);

SdResult_t initializeSd()
{
    // TODO need to initialize and check SD card detect pin
    // if card is not present abort

    initialize();
    initializeSdDma();

    SdResult_t sd_result;

    if ((sd_result = powerOn()) != SDR_OK)
    {
        return sd_result;
    }
    if ((sd_result = initializeSdCard()) != SDR_OK)
    {
        return sd_result;
    }
    if ((sd_result = loadScrRegister()) != SDR_OK)
    {
        return sd_result;
    }
    if (is4BitBusSupported() && (sd_result = turnOn4BitBus()) != SDR_OK)
    {
        return sd_result;
    }

    return SDR_OK;
}

void initialize()
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

    NVIC_SetPriority(DMA2_Channel4_IRQn, 3);
    NVIC_EnableIRQ(DMA2_Channel4_IRQn);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_4);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_CHANNEL_4);

    __SDMMC_DMA_ENABLE(SDMMC1);
}

void initializeSdDmaDirection(SdDmaDirection_t direction, uint32_t bufferLength)
{
    uint32_t sdRxTxDirection;

    switch (direction)
    {
        case SD_DMA_RX :
        {
            sdRxTxDirection = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
            break;
        }
        case SD_DMA_TX :
        {
            sdRxTxDirection = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
            break;
        }
        default: return;
    }

    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_4);

    LL_DMA_ConfigTransfer(DMA2,
                          LL_DMA_CHANNEL_4,
                          sdRxTxDirection |
                          LL_DMA_PRIORITY_VERYHIGH |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_WORD |
                          LL_DMA_PDATAALIGN_WORD);

    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_CHANNEL_4,
                           (uint32_t) g_sdBuffer,
                           (uint32_t) &SDMMC1->FIFO,
                           sdRxTxDirection);

    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, bufferLength);
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

        g_isSdHighCapacity = (response & SDMMC_HIGH_CAPACITY) == SDMMC_HIGH_CAPACITY;
    }

    return SDR_OK;
}

SdResult_t initializeSdCard()
{
    if ((SDMMC1->POWER & SDMMC_POWER_PWRCTRL) == 0)
    {
        return SDR_POWER_DOWN;
    }

    SdResult_t sd_result;

    if ((sd_result = SD_CmdWithResponse2(SDMMC_CMD_ALL_SEND_CID, 0, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, (uint32_t*) g_sdCid)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = SD_CmdWithResponse6(SDMMC_CMD_SET_REL_ADDR, 0, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, (uint16_t*) &g_sdRelativeCardAddress)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (g_sdRelativeCardAddress) << 16U);

    if ((sd_result = SD_CmdWithResponse2(SDMMC_CMD_SEND_CSD, sdRelativeCardAddressCmdArg, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, (uint32_t*) g_sdCsd)) != SDR_OK)
    {
        return sd_result;
    }

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_SEL_DESEL_CARD, sdRelativeCardAddressCmdArg, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
    {
        return sd_result;
    }

    return SDR_OK;
}

SdResult_t loadScrRegister()
{
    SdResult_t sd_result;

    if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_SET_BLOCKLEN, 8, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE, NULL)) != SDR_OK)
    {
        return sd_result;
    }

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t) (g_sdRelativeCardAddress) << 16U);

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

        g_sdScr[0] = (((scr[1] & SDMMC_0TO7BITS) << 24)  | ((scr[1] & SDMMC_8TO15BITS) << 8) | \
                      ((scr[1] & SDMMC_16TO23BITS) >> 8) | ((scr[1] & SDMMC_24TO31BITS) >> 24));
        g_sdScr[1] = (((scr[0] & SDMMC_0TO7BITS) << 24)  | ((scr[0] & SDMMC_8TO15BITS) << 8) | \
                      ((scr[0] & SDMMC_16TO23BITS) >> 8) | ((scr[0] & SDMMC_24TO31BITS) >> 24));

        return SDR_OK;
    }
}

SdResult_t turnOn4BitBus()
{
    SdResult_t sd_result;

    const uint32_t sdRelativeCardAddressCmdArg = ((uint32_t)(g_sdRelativeCardAddress) << 16U);

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

bool is4BitBusSupported()
{
    return g_sdScr[1] & (1 << 18);
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

SdResult_t SD_CmdWithResponse2(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, uint32_t pResponse1Result[4])
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

SdResult_t SD_CmdWithResponse6(uint32_t cmdIndex, uint32_t argument, uint32_t waitForInterrupt, uint32_t cpsm, uint16_t* pResponseResult)
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

void SDMMC1_IRQHandler()
{
}

void DMA2_Channel4_IRQHandler()
{
}
