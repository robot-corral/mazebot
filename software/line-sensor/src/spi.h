/*******************************************************************************
 * Copyright (C) 2019 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

//
// SPI CR1 register
//

#define SPI_CR1_BIDIMODE_BI_DIRECTIONAL_MODE_DISABLED      0
#define SPI_CR1_BIDIMODE_BI_DIRECTIONAL_MODE_ENABLED       (1 << 15)

#define SPI_CR1_BIDIOE_BI_DIRECTIONAL_MODE_OUTPUT_DISABLED 0
#define SPI_CR1_BIDIOE_BI_DIRECTIONAL_MODE_OUTPUT_ENABLED  (1 << 14)

#define SPI_CR1_CRCEN_CRC_DISABLED                         0
#define SPI_CR1_CRCEN_CRC_ENABLED                          (1 << 13)

#define SPI_CR1_CRCNEX_NEXT_TRANSFER_IS_DATA               0
#define SPI_CR1_CRCNEX_NEXT_TRANSFER_IS_CRC                (1 << 12)

#define SPI_CR1_DFF_8BIT_DATA_FRAME                        0
#define SPI_CR1_DFF_16BIT_DATA_FRAME                       (1 << 11)

#define SPI_CR1_RXONLY_FULL_DUPLEX                         0
#define SPI_CR1_RXONLY_RX_ONLY                             (1 << 10)

#define SPI_CR1_SSM_SOFTWARE_SLAVE_MANAGEMENT_DISABLED     0
#define SPI_CR1_SSM_SOFTWARE_SLAVE_MANAGEMENT_ENABLED      (1 << 9)

#define SPI_CR1_SSI_INTERNAL_SLAVE_SELECT_DISABLED         0
#define SPI_CR1_SSI_INTERNAL_SLAVE_SELECT_ENABLED          (1 << 8)

#define SPI_CR1_LSBFIRST_MOST_SIGNIFICANT_BIT_FIRST        0
#define SPI_CR1_LSBFIRST_LEAST_SIGNIFICANT_BIT_FIRST       (1 << 7)

#define SPI_CR1_SPE_DEVICE_DISABLED                        0
#define SPI_CR1_SPE_DEVICE_ENABLED                         (1 << 6)

#define SPI_CR1_BR_PCLK_DIV_2                              000
#define SPI_CR1_BR_PCLK_DIV_4                              (001 << 3)
#define SPI_CR1_BR_PCLK_DIV_8                              (010 << 3)
#define SPI_CR1_BR_PCLK_DIV_16                             (011 << 3)
#define SPI_CR1_BR_PCLK_DIV_32                             (100 << 3)
#define SPI_CR1_BR_PCLK_DIV_64                             (101 << 3)
#define SPI_CR1_BR_PCLK_DIV_128                            (110 << 3)
#define SPI_CR1_BR_PCLK_DIV_256                            (111 << 3)

#define SPI_CR1_MSTR_SLAVE_CONFIGURATION                   0
#define SPI_CR1_MSTR_MASTER_CONFIGURATION                  (1 << 2)

#define SPI_CR1_CPOL_CLOCK_POLARITY_0_WHEN_IDLE            0
#define SPI_CR1_CPOL_CLOCK_POLARITY_1_WHEN_IDLE            (1 << 1)

#define SPI_CR1_CPHA_1ST_CLOCK_TRANSITION_1ST_DATA_CAPTURE 0
#define SPI_CR1_CPHA_2ND_CLOCK_TRANSITION_1ST_DATA_CAPTURE 1

//
// SPI CR2 register
//

#define SPI_CR2_TXEIE_TX_EMPTY_INTERRUPT_DISABLED         0
#define SPI_CR2_TXEIE_TX_EMPTY_INTERRUPT_ENABLED          (1 << 7)

#define SPI_CR2_RXNEIE_RX_NOT_EMPTY_INTERRUPT_DISABLED    0
#define SPI_CR2_RXNEIE_RX_NOT_EMPTY_INTERRUPT_ENABLED     (1 << 6)

#define SPI_CR2_ERRIE_ERROR_INTERRUPT_DISABLED            0
#define SPI_CR2_ERRIE_ERROR_INTERRUPT_ENABLED             (1 << 5)

#define SPI_CR2_FRF_MOTOROLA_MODE                         0
#define SPI_CR2_FRF_TEXAS_INSTRUMENTS_MODE                (1 << 4)

#define SPI_CR2_SSOE_SS_OUTPUT_IS_DISABLED_IN_MASTER_MODE 0
#define SPI_CR2_SSOE_SS_OUTPUT_IS_ENABLED_IN_MASTER_MODE  (1 << 2)

#define SPI_CR2_TXDMAEN_TX_BUFFER_DMA_DISABLED            0
#define SPI_CR2_TXDMAEN_TX_BUFFER_DMA_ENABLED             (1 << 1)

#define SPI_CR2_RXDMAEN_RX_BUFFER_DMA_DISABLED            0
#define SPI_CR2_RXDMAEN_RX_BUFFER_DMA_ENABLED             1

typedef enum
{
    SPI_S_IDLE,
    SPI_S_WAITING_TO_RECEIVE_COMMAND_CODE,
    SPI_S_WAITING_TO_TRANSMIT_FILLER,
    SPI_S_TRANSMITTING_RESPONSE,
} spiState_t;

void initializeSpi();

void activateSpi();
