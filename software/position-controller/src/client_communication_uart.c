#include "client_communication_uart.h"
#include "client_communication_uart_impl.h"

#include <stm32\stm32l4xx_ll_crc.h>
#include <stm32\stm32l4xx_ll_rcc.h>

#include "led.h"
#include "global_data.h"
#include "position_controller.h"

static uint32_t calculateRequestCrc(volatile clientUartRequest_t* pRequest);
static uint32_t calculateResponseCrc(volatile clientUartResponse_t* pResponse);

void initializeClientCommunicationUart()
{
#ifdef STM32L496ZG
    enableVddIo2();
#endif
    initializeUart();
    setRxActiveLedEnabled(true);

    LL_CRC_SetPolynomialCoef(CRC, CRC32_POLYNOMIAL);
}

void processCommand(volatile clientUartRequest_t* pRequest)
{
    g_clientUartTxBuffer.unpacked.header = CLIENT_UART_RESPONSE_HEADER;
    g_clientUartTxBuffer.unpacked.position = getPosition();
    g_clientUartTxBuffer.unpacked.resultFlags = 0;

    if (isPositionControllerBusy())
    {
        g_clientUartTxBuffer.unpacked.resultFlags |= (uint16_t) ERR_BUSY;
    }
    if (isPositionControllerInEmergency())
    {
        g_clientUartTxBuffer.unpacked.resultFlags |= (uint16_t) ERR_EMERGENCY_STOP;
    }

    const uint32_t calculatedCrc = calculateRequestCrc(pRequest);

    if (calculatedCrc != pRequest->unpacked.crc)
    {
        g_clientUartTxBuffer.unpacked.resultFlags = (uint16_t) ERR_CRC;
        g_clientUartTxBuffer.unpacked.crc = calculateResponseCrc(&g_clientUartTxBuffer);
        return;
    }

    if (pRequest == nullptr || pRequest->unpacked.header != CLIENT_UART_REQUEST_HEADER)
    {
        g_clientUartTxBuffer.unpacked.resultFlags = (uint16_t) ERR_COMMUNICATION_ERROR;
        g_clientUartTxBuffer.unpacked.crc = calculateResponseCrc(&g_clientUartTxBuffer);
        return;
    }

    switch (pRequest->unpacked.motorCommand)
    {
        case MCMD_EMERGENCY_STOP:
        {
            positionControllerEmergencyStop();
            g_clientUartTxBuffer.unpacked.resultFlags |= (uint16_t) ERR_EMERGENCY_STOP;
            break;
        }
        case MCMD_MOVE_IF_IDLE:
        {
            if (!setPosition(pRequest->unpacked.direction, pRequest->unpacked.steps))
            {
                g_clientUartTxBuffer.unpacked.resultFlags |= (uint16_t) ERR_BUSY;
            }
            break;
        }
        case MCMD_CALIBRATE:
        {
            if (!calibratePositionController())
            {
                g_clientUartTxBuffer.unpacked.resultFlags |= (uint16_t) ERR_BUSY;
            }
            break;
        }
        case MCMD_GET_STATUS:
        {
            // we already set position above
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
            g_clientUartTxBuffer.unpacked.resultFlags |= (uint16_t) ERR_UNKNOWN_COMMAND;
            break;
        }
    }

    g_clientUartTxBuffer.unpacked.crc = calculateResponseCrc(&g_clientUartTxBuffer);
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
