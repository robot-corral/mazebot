#include "spi.h"

#include "status.h"
#include "global_data.h"

#include <stm32/stm32l1xx_ll_dma.h>
#include <stm32/stm32l1xx_ll_spi.h>

static void processReceivedHeader();
static void processReceivedCommandWithData();
static void resetSpiDmaReceiving();

void initializeSpi()
{
    LL_SPI_SetBaudRatePrescaler(SPI2, LL_SPI_BAUDRATEPRESCALER_DIV128);
    LL_SPI_SetTransferDirection(SPI2,LL_SPI_FULL_DUPLEX);

    LL_SPI_SetClockPhase(SPI2, LL_SPI_PHASE_2EDGE);
    LL_SPI_SetClockPolarity(SPI2, LL_SPI_POLARITY_HIGH);

    LL_SPI_SetDataWidth(SPI2, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(SPI2, LL_SPI_NSS_HARD_OUTPUT);
    LL_SPI_SetMode(SPI2, LL_SPI_MODE_SLAVE);
    LL_SPI_SetCRCPolynomial(SPI2, SPI_CRC_POLYNOMIAL);
    LL_SPI_EnableCRC(SPI2);

    LL_SPI_EnableDMAReq_RX(SPI2);
    LL_SPI_EnableDMAReq_TX(SPI2);

    LL_SPI_EnableIT_ERR(SPI2);
}

void resetSpiDmaReceiving()
{
    g_receivingHeader = true;
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_4,
                           LL_SPI_DMA_GetRegAddr(SPI2),
                           (uint32_t) &g_rxBuffer,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, sizeof(lineSensorRequestHeaderData_t));
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
}

void activateSpi()
{
    LL_SPI_Enable(SPI2);
    resetSpiDmaReceiving();
}

void DMA1_Channel4_IRQHandler(void)
{
    if(LL_DMA_IsActiveFlag_TE4(DMA1))
    {
        LL_DMA_ClearFlag_TE4(DMA1);
        setSensorStatusFlags(LSS_ERR_FLAG_SPI_DMA_FAILURE);
        resetSpiDmaReceiving();
    }
    else if(LL_DMA_IsActiveFlag_TC4(DMA1))
    {
        LL_DMA_ClearFlag_TC4(DMA1);
        if (g_receivingHeader)
        {
            processReceivedHeader();
        }
        else
        {
            processReceivedCommandWithData();
        }
    }
}

void SPI2_IRQHandler(void)
{
    if(LL_SPI_IsActiveFlag_CRCERR(SPI2))
    {
        LL_SPI_ClearFlag_CRCERR(SPI2);
        setSensorStatusFlags(LSS_ERR_FLAG_SPI_ERROR_CRC);
    }
    if(LL_SPI_IsActiveFlag_MODF(SPI2))
    {
        LL_SPI_ClearFlag_MODF(SPI2);
        setSensorStatusFlags(LSS_ERR_FLAG_SPI_ERROR_MODE_FAULT);
    }
    if(LL_SPI_IsActiveFlag_OVR(SPI2))
    {
        LL_SPI_ClearFlag_OVR(SPI2);
        setSensorStatusFlags(LSS_ERR_FLAG_SPI_ERROR_OVERRUN);
    }
    if(LL_SPI_IsActiveFlag_FRE(SPI2))
    {
        LL_SPI_ClearFlag_FRE(SPI2);
        setSensorStatusFlags(LSS_ERR_FLAG_SPI_ERROR_FRAME_FORMAT);
    }

    LL_SPI_Disable(SPI2);
    LL_SPI_Enable(SPI2);
    resetSpiDmaReceiving();
}

void DMA1_Channel5_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE5(DMA1))
    {
        LL_DMA_ClearFlag_TE5(DMA1);
        setSensorStatusFlags(LSS_ERR_FLAG_SPI_DMA_FAILURE);
        resetSpiDmaReceiving();
    }
    else if (LL_DMA_IsActiveFlag_TC5(DMA1))
    {
        LL_DMA_ClearFlag_TC5(DMA1);
        resetSpiDmaReceiving();
    }
}

void processReceivedHeader()
{
    if (g_rxBuffer.header.prefix != COMMAND_PREFIX)
    {
        setSensorStatusFlags(LSS_ERR_FLAG_SPI_RECEIVING_ERROR);
        resetSpiDmaReceiving();
        return;
    }

    switch (g_rxBuffer.header.commandCode)
    {
        case LSC_RESET:
        {
            // TODO
            break;
        }
        case LSC_START_CALIBRATION:
        {
            // TODO
            break;
        }
        case LSC_FINISH_CALIBRATION:
        {
            // TODO
            break;
        }
        case LSC_USE_HARDCODED_CALIBRATION_DATA:
        {
            // TODO
            break;
        }
        case LSC_SEND_SENSOR_DATA:
        {
            // TODO
            break;
        }
        case LSC_USE_CALIBRATION_DATA:
        {
            g_receivingHeader = false;
            LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
            LL_DMA_ConfigAddresses(DMA1,
                                   LL_DMA_CHANNEL_4,
                                   LL_SPI_DMA_GetRegAddr(SPI2),
                                   (uint32_t) &g_rxBuffer.useCalibrationData.calibrationData,
                                   LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
            LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, sizeof(lineSensorCalibrationData_t));
            LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
            break;
        }
        default:
        {
            g_receivingHeader = true;
            setSensorStatusFlags(LSS_ERR_FLAG_UNKNOWN_COMMAND);
            resetSpiDmaReceiving();
            return;
        }
    }
}

void processReceivedCommandWithData()
{
    // TODO
}
