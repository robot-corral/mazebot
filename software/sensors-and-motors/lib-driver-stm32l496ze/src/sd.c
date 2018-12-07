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
static SdResult_t turnOn4BitBus();

// responseId can be one of the following:
//  SDMMC_RESP1
//  SDMMC_RESP2
//  SDMMC_RESP3
//  SDMMC_RESP4
static uint32_t SD_GetResponse(uint32_t responseId);
static uint8_t SD_GetCommandResponse();

// cpsm -> command path state machine
static SdResult_t SD_CmdWithCmdError(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm);
static SdResult_t SD_CmdWithResponse1(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm);
static SdResult_t SD_CmdWithResponse3(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm);
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
    if ((sd_result = turnOn4BitBus()) != SDR_OK)
    {
        return sd_result;
    }

    return SDR_OK;
}

void initialize()
{
    LL_RCC_SetSDMMCClockSource(LL_RCC_SDMMC1_CLKSOURCE_HSI48);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SDMMC1);

    MODIFY_REG(SDMMC1->CLKCR, CLKCR_CLEAR_MASK, SDMMC_CLOCK_EDGE_RISING |
                                                SDMMC_CLOCK_BYPASS_DISABLE |
                                                SDMMC_CLOCK_POWER_SAVE_DISABLE |
                                                SDMMC_BUS_WIDE_4B |
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

    if ((sd_result = SD_CmdWithCmdError(SDMMC_CMD_GO_IDLE_STATE, 0, SDMMC_RESPONSE_NO, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE)) != SDR_OK)
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
            if ((sd_result = SD_CmdWithResponse1(SDMMC_CMD_APP_CMD, 0, SDMMC_RESPONSE_SHORT, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE)) != SDR_OK)
            {
                return sd_result;
            }
            if ((sd_result = SD_CmdWithResponse3(SDMMC_CMD_SD_APP_OP_COND, SDMMC_VOLTAGE_WINDOW_SD | SDMMC_HIGH_CAPACITY, SDMMC_RESPONSE_SHORT, SDMMC_WAIT_NO, SDMMC_CPSM_ENABLE)) != SDR_OK)
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
    return SDR_OK;
}

SdResult_t turnOn4BitBus()
{
//    45.4.9 Wide bus selection or deselection
//    Wide bus (4-bit bus width) operation mode is selected or deselected using
//    SET_BUS_WIDTH (ACMD6). The default bus width after power-up or GO_IDLE_STATE
//    (CMD0) is 1 bit. SET_BUS_WIDTH (ACMD6) is only valid in a transfer state, which means
//    that the bus width can be changed only after a card is selected by
//    SELECT/DESELECT_CARD (CMD7).
    return SDR_OK;
}

uint32_t SD_GetResponse(uint32_t responseId)
{
    uint32_t responseAddress = (uint32_t)(&(SDMMC1->RESP1)) + SDMMC_RESP1;
    return (*(__IO uint32_t *) responseAddress);
}

uint8_t SD_GetCommandResponse()
{
    return (uint8_t) (SDMMC1->RESPCMD);
}

SdResult_t SD_CmdWithCmdError(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm)
{
    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_FLAG_CTIMEOUT);

    SDMMC1->ARG = argument;
    MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, cmdIndex | response | waitForInterrupt | cpsm);

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

SdResult_t SD_CmdWithResponse1(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm)
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
    else if (SD_GetCommandResponse() != cmdIndex)
    {
        return SDR_COMMAND_CRC_FAIL;
    }

    __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);

    uint32_t cmd_response = SD_GetResponse(SDMMC_RESP1);

    if ((cmd_response & SDMMC_OCR_ERRORBITS) == SDMMC_ALLZERO)
    {
        return SDR_OK;
    }

    return SDR_ERROR;
//    else if ((cmd_response & SDMMC_OCR_ADDR_OUT_OF_RANGE) == SDMMC_OCR_ADDR_OUT_OF_RANGE)
//    {
//        return SDR_ADDRESS_OUT_OF_RANGE;
//    }
//    else if ((cmd_response & SDMMC_OCR_ADDR_MISALIGNED) == SDMMC_OCR_ADDR_MISALIGNED)
//    {
//        return SDR_ADDRESS_MISALIGNED;
//    }
//    else if ((cmd_response & SDMMC_OCR_BLOCK_LEN_ERR) == SDMMC_OCR_BLOCK_LEN_ERR)
//    {
//        return SDR_BLOCK_LENGTH_ERROR;
//    }
//    else if ((cmd_response & SDMMC_OCR_ERASE_SEQ_ERR) == SDMMC_OCR_ERASE_SEQ_ERR)
//    {
//        return SDR_ERASE_SEQUENCE_ERROR;
//    }
//    else if ((cmd_response & SDMMC_OCR_BAD_ERASE_PARAM) == SDMMC_OCR_BAD_ERASE_PARAM)
//    {
//        return SDR_BAD_ERASE_PARAMETER;
//    }
//    else if ((cmd_response & SDMMC_OCR_WRITE_PROT_VIOLATION) == SDMMC_OCR_WRITE_PROT_VIOLATION)
//    {
//        return SDR_ERROR_WRITE_PROT_VIOLATION;
//    }
//    else if ((cmd_response & SDMMC_OCR_LOCK_UNLOCK_FAILED) == SDMMC_OCR_LOCK_UNLOCK_FAILED)
//    {
//        return SDR_ERROR_LOCK_UNLOCK_FAILED;
//    }
//    else if ((cmd_response & SDMMC_OCR_COM_CRC_FAILED) == SDMMC_OCR_COM_CRC_FAILED)
//    {
//        return SDR_ERROR_COM_CRC_FAILED;
//    }
//    else if ((cmd_response & SDMMC_OCR_ILLEGAL_CMD) == SDMMC_OCR_ILLEGAL_CMD)
//    {
//        return SDR_ERROR_ILLEGAL_CMD;
//    }
//    else if ((cmd_response & SDMMC_OCR_CARD_ECC_FAILED) == SDMMC_OCR_CARD_ECC_FAILED)
//    {
//        return SDR_ERROR_CARD_ECC_FAILED;
//    }
//    else if ((cmd_response & SDMMC_OCR_CC_ERROR) == SDMMC_OCR_CC_ERROR)
//    {
//        return SDR_ERROR_CC_ERR;
//    }
//    else if ((cmd_response & SDMMC_OCR_STREAM_READ_UNDERRUN) == SDMMC_OCR_STREAM_READ_UNDERRUN)
//    {
//        return SDR_ERROR_STREAM_READ_UNDERRUN;
//    }
//    else if ((cmd_response & SDMMC_OCR_STREAM_WRITE_OVERRUN) == SDMMC_OCR_STREAM_WRITE_OVERRUN)
//    {
//        return SDR_ERROR_STREAM_WRITE_OVERRUN;
//    }
//    else if ((cmd_response & SDMMC_OCR_CID_CSD_OVERWRITE) == SDMMC_OCR_CID_CSD_OVERWRITE)
//    {
//        return SDR_ERROR_CID_CSD_OVERWRITE;
//    }
//    else if ((cmd_response & SDMMC_OCR_WP_ERASE_SKIP) == SDMMC_OCR_WP_ERASE_SKIP)
//    {
//        return SDR_ERROR_WP_ERASE_SKIP;
//    }
//    else if ((cmd_response & SDMMC_OCR_CARD_ECC_DISABLED) == SDMMC_OCR_CARD_ECC_DISABLED)
//    {
//        return SDR_ERROR_CARD_ECC_DISABLED;
//    }
//    else if ((cmd_response & SDMMC_OCR_ERASE_RESET) == SDMMC_OCR_ERASE_RESET)
//    {
//        return SDR_ERROR_ERASE_RESET;
//    }
//    else if ((cmd_response & SDMMC_OCR_AKE_SEQ_ERROR) == SDMMC_OCR_AKE_SEQ_ERROR)
//    {
//        return SDR_ERROR_AKE_SEQ_ERR;
//    }
//    else
//    {
//        return SDR_ERROR_GENERAL_UNKNOWN_ERR;
//    }
}

SdResult_t SD_CmdWithResponse3(uint32_t cmdIndex, uint32_t argument, uint32_t response, uint32_t waitForInterrupt, uint32_t cpsm)
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
        return SDMMC_ERROR_CMD_RSP_TIMEOUT;
    }
    else
    {
        __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_CMD_FLAGS);
        return SDR_OK;
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
