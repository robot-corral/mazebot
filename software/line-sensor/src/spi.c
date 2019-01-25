#include "spi.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_spi.h>

void initializeSpi()
{
    LL_SPI_Disable(SPI2);
    LL_SPI_SetBaudRatePrescaler(SPI2, LL_SPI_BAUDRATEPRESCALER_DIV2);
    LL_SPI_SetTransferDirection(SPI2, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPhase(SPI2, LL_SPI_PHASE_2EDGE);
    LL_SPI_SetClockPolarity(SPI2, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetDataWidth(SPI2, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(SPI2, LL_SPI_NSS_SOFT);

    LL_SPI_EnableDMAReq_RX(SPI2);
    LL_SPI_EnableDMAReq_TX(SPI2);
}
