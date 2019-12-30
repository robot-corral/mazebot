#include "client_communication_uart.h"
#include "client_communication_uart_impl.h"

#include <stm32\stm32l4xx_ll_rcc.h>

#include "led.h"
#include "global_data.h"
#include "position_controller.h"

void initializeClientCommunicationUart()
{
#ifdef STM32L496ZG
    enableVddIo2();
#endif
    initializeUart();
    setRxActiveLedEnabled(true);
}

void processCommand(volatile clientUartRequest_t* pRequest)
{
    g_clientUartTxBuffer.header = CLIENT_UART_RESPONSE_HEADER;
    g_clientUartTxBuffer.position = getPosition();
    g_clientUartTxBuffer.resultFlags = 0;

    if (isPositionControllerBusy())
    {
        g_clientUartTxBuffer.resultFlags |= (uint8_t) ERR_BUSY;
    }
    if (isPositionControllerInEmergency())
    {
        g_clientUartTxBuffer.resultFlags |= (uint8_t) ERR_EMERGENCY_STOP;
    }

    if (pRequest == nullptr || pRequest->header != CLIENT_UART_REQUEST_HEADER)
    {
        g_clientUartTxBuffer.resultFlags = (uint8_t) ERR_TX_ERROR;
        return;
    }

    switch (pRequest->motorCommand)
    {
        case MCMD_EMERGENCY_STOP:
        {
            positionControllerEmergencyStop();
            g_clientUartTxBuffer.resultFlags |= (uint8_t) ERR_EMERGENCY_STOP;
            break;
        }
        case MCMD_MOVE_IF_IDLE:
        {
            if (!setPosition(pRequest->direction, pRequest->steps))
            {
                g_clientUartTxBuffer.resultFlags |= (uint8_t) ERR_BUSY;
            }
            break;
        }
        case MCMD_CALIBRATE:
        {
            if (!calibratePositionController())
            {
                g_clientUartTxBuffer.resultFlags |= (uint8_t) ERR_BUSY;
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
            g_clientUartTxBuffer.resultFlags |= (uint8_t) ERR_UNKNOWN_COMMAND;
            break;
        }
    }
}
