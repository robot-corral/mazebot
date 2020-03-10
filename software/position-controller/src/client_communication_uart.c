#include "client_communication_uart.h"
#include "client_communication_uart_impl.h"

#include <stm32\stm32l4xx_ll_crc.h>
#include <stm32\stm32l4xx_ll_rcc.h>

#include "led.h"
#include "global_data.h"
#include "position_controller.h"

static uint32_t calculateRequestCrc(volatile clientUartRequest_t* pRequest);
static uint32_t calculateResponseCrc(volatile clientUartResponse_t* pResponse);
static commandResultFlags_t convertStateToCommandResultFlags(positionControllerState_t state);
static commandResultFlags_t convertMoveRequestResultToCommandResultFlags(moveRequestResult_t result);

void initializeClientCommunicationUart()
{
    g_clientUartIsTransmitting = false;

#ifdef STM32L496ZG
    enableVddIo2();
#endif
    initializeUart();

    LL_CRC_SetPolynomialCoef(CRC, CRC32_POLYNOMIAL);
}

bool processCommand(volatile clientUartRequest_t* pRequest)
{
    g_clientUartTxBuffer.unpacked.header = CLIENT_UART_RESPONSE_HEADER;
    g_clientUartTxBuffer.unpacked.position = getPosition();
    g_clientUartTxBuffer.unpacked.resultFlags = getAbsolutePositionError() != 0 ? ERR_POSITION_ERROR : 0;

    if (pRequest == nullptr)
    {
        g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(getState()) | ERR_COMMUNICATION_ERROR;
        g_clientUartTxBuffer.unpacked.crc = calculateResponseCrc(&g_clientUartTxBuffer);
        return false;
    }

    const uint32_t calculatedCrc = calculateRequestCrc(pRequest);

    if (calculatedCrc != pRequest->unpacked.crc)
    {
        g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(getState()) | ERR_CRC;
        g_clientUartTxBuffer.unpacked.crc = calculateResponseCrc(&g_clientUartTxBuffer);
        return false;
    }

    if (pRequest->unpacked.header != CLIENT_UART_REQUEST_HEADER)
    {
        g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(getState()) | ERR_COMMUNICATION_ERROR;
        g_clientUartTxBuffer.unpacked.crc = calculateResponseCrc(&g_clientUartTxBuffer);
        return false;
    }

    switch (pRequest->unpacked.motorCommand)
    {
        case MCMD_EMERGENCY_STOP:
        {
            positionControllerEmergencyStop();
            g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(getState());
            break;
        }
        case MCMD_MOVE_IF_IDLE:
        {
            positionControllerState_t state;
            moveRequestResult_t result = setPosition(pRequest->unpacked.direction, pRequest->unpacked.steps, &state);
            g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(state) | convertMoveRequestResultToCommandResultFlags(result);
            break;
        }
        case MCMD_CALIBRATE:
        {
            positionControllerState_t state;
            moveRequestResult_t result = calibratePositionController(&state);
            g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(state) | convertMoveRequestResultToCommandResultFlags(result);
            break;
        }
        case MCMD_GET_STATUS:
        {
            g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(getState());
            break;
        }
        case MCMD_RESET:
        {
            positionControllerEmergencyStop();
            NVIC_SystemReset();
            break;
        }
        default:
        {
            g_clientUartTxBuffer.unpacked.resultFlags |= convertStateToCommandResultFlags(getState()) | ERR_UNKNOWN_COMMAND;
            break;
        }
    }

    g_clientUartTxBuffer.unpacked.crc = calculateResponseCrc(&g_clientUartTxBuffer);

    return true;
}

uint32_t calculateRequestCrc(volatile clientUartRequest_t* pRequest)
{
    LL_CRC_ResetCRCCalculationUnit(CRC);
    volatile uint32_t* pData = (uint32_t*) pRequest;
    LL_CRC_FeedData32(CRC, pData[0]);
    LL_CRC_FeedData32(CRC, pData[1]);
    return LL_CRC_ReadData32(CRC);
}

uint32_t calculateResponseCrc(volatile clientUartResponse_t* pResponse)
{
    LL_CRC_ResetCRCCalculationUnit(CRC);
    const volatile uint32_t * const pData = (volatile uint32_t*) pResponse;
    LL_CRC_FeedData32(CRC, pData[0]);
    LL_CRC_FeedData32(CRC, pData[1]);
    return LL_CRC_ReadData32(CRC);
}

commandResultFlags_t convertStateToCommandResultFlags(positionControllerState_t state)
{
    if (state == PCS_RESET)
    {
        return OK_RESET;
    }
    else if (state == PCS_IDLE)
    {
        return OK_IDLE;
    }
    else if (state == PCS_EMERGENCY_STOPPED)
    {
        return OK_EMERGENCY_STOP;
    }
    else
    {
        return OK_BUSY;
    }
}

commandResultFlags_t convertMoveRequestResultToCommandResultFlags(moveRequestResult_t result)
{
    if (result == MRR_OK)
    {
        return 0;
    }
    else if (result == MRR_BUSY)
    {
        return ERR_BUSY;
    }
    else if (result == MRR_INVALID_STATE)
    {
        return ERR_INVALID_STATE;
    }
    else if (result == MRR_INVALID_PARAMETER)
    {
        return ERR_INVALID_PARAMETER;
    }
    else
    {
        return ERR_UNEXPECTED;
    }
}

bool tryToProcessEmergencyStopCommand(volatile clientUartRequest_t* pRequest)
{
    if (pRequest == nullptr)
    {
        return false;
    }

    const uint32_t calculatedCrc = calculateRequestCrc(pRequest);

    if (calculatedCrc != pRequest->unpacked.crc)
    {
        return false;
    }

    if (pRequest->unpacked.header != CLIENT_UART_REQUEST_HEADER)
    {
        return false;
    }

    if (pRequest->unpacked.motorCommand == MCMD_EMERGENCY_STOP)
    {
        positionControllerEmergencyStop();
    }

    return true;
}
