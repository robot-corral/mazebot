/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "adc.h"

#include "global_data.h"

#include <stm32/stm32l1xx_ll_adc.h>
#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_dma.h>

static void configureBankA();
static void configureBankB();

// ADC query takes ~55 usec

void initializeAdc()
{
    //            -----                            reserved
    //                 -                           OVRIE   - 0 (overrun interrupt enable, disabled)
    //                  --                         RES     - 0 (resolution, 12 bit)
    //                    -                        AWDEN   - 0 (analog watchdog enable on regular channels, disabled)
    //                     -                       JAWDEN  - 0 (analog watchdog enable on injected channels, disabled)
    //                      ----                   reserved
    //                          -                  PDI     - 1 (power down during the idle phase, enabled)
    //                           -                 PDD     - 0 (power down during the delay phase, disabled)
    //                            ---              DISCNUM - 0 (discontinuous mode channel count, default/ignored)
    //                               -             JDISCEN - 0 (discontinuous mode on injected channels, disabled)
    //                                -            DISCEN  - 0 (discontinuous mode on regular channels, disabled)
    //                                 -           JAUTO   - 0 (automatic injected group conversion, disabled)
    //                                  -          AWDSGL  - 0 (enable the watchdog on a single channel in scan mode, disabled)
    //                                   -         SCAN    - 1 (scan mode, enabled)
    //                                    -        JEOCIE  - 0 (interrupt enable for injected channels, disabled)
    //                                     -       AWDIE   - 0 (analog watchdog interrupt, disabled)
    //                                      -      EOCIE   - 0 (end of conversation interrupt, disabled)
    //                                       ----- AWDCH   - 0 (analog watchdog channel selector, default/ignored)
    ADC1->CR1 = 0b00000000000000100000000100000000;

    //            -                                reserved
    //             -                               SWSTART  - 0 (start conversion of regular channels, default/ignored)
    //              --                             EXTEN    - 0 (external trigger enable for regular channels, disabled)
    //                ----                         EXTSEL   - 0 (external event select for regular group, default/ignored)
    //                    -                        reserved
    //                     -                       JSWSTART - 0 (start conversion of injected channels, default/ignored)
    //                      --                     JEXTEN   - 0 (external trigger enable for injected channels, disabled)
    //                        ----                 JEXTSEL  - 0 (external event select for injected group, default/ignored)
    //                            ----             reserved
    //                                -            ALIGN    - 0 (data alignment, right alignment)
    //                                 -           EOCS     - 0 (end of conversion selection, the eoc bit is set at the end of each sequence of regular conversions)
    //                                  -          DDS      - 1 (dma disable selection, enabled)
    //                                   -         DMA      - 1 (direct memory access mode, enabled)
    //                                    -        reserved
    //                                     ---     DELS     - 0 (delay selection, no delay)
    //                                        -    reserved
    //                                         -   ADC_CFG  - 0 (adc configuration, bank A is selected on startup)
    //                                          -  CONT     - 0 (continuous conversion, disabled)
    //                                           - ADON     - 0 (a/d converter on / off, disabled)
    ADC1->CR2 = 0b00000000000000000000001100000000;

    //             -------                          reserved
    //                    -----                     L       - 10011 (regular channel sequence length, 20 - 1 -> 19)
    //                         -----                rank 28 -     0 (default/ignored)
    //                              -----           rank 27 -     0 (default/ignored)
    //                                   -----      rank 26 -     0 (default/ignored)
    //                                        ----- rank 25 -     0 (default/ignored)
    ADC1->SQR1 = 0b00000001001100000000000000000000;

    // SMPR0, SMPR1, SMPR2, SMPR3 are all set for 4 cycles which is a reset state so no need to do anything.

    LL_ADC_Enable(ADC1);
}

void configureBankA()
{
    LL_ADC_SetChannelsBank(ADC1, LL_ADC_CHANNELS_BANK_A);

    //             --                               reserved
    //               -----                          rank 24 -     0 (default/ignored)
    //                    -----                     rank 23 -     0 (default/ignored)
    //                         -----                rank 22 -     0 (default/ignored) 
    //                              -----           rank 21 -     0 (default/ignored) 
    //                                   -----      rank 20 - 01011 (channel 11)
    //                                        ----- rank 19 - 01100 (channel 12)
    ADC1->SQR2 = 0b00000000000000000000000101101100;
    //             --                               reserved
    //               -----                          rank 18 - 01101 (channel 13)
    //                    -----                     rank 17 - 00000 (channel  0)
    //                         -----                rank 16 - 00001 (channel  1)
    //                              -----           rank 15 - 00010 (channel  2)
    //                                   -----      rank 14 - 00011 (channel  3)
    //                                        ----- rank 13 - 00100 (channel  4)
    ADC1->SQR3 = 0b00011010000000001000100001100100;
    //             --                               reserved
    //               -----                          rank 12 - 00101 (channel  5)
    //                    -----                     rank 11 - 00110 (channel  6)
    //                         -----                rank 10 - 00111 (channel  7)
    //                              -----           rank  9 - 01110 (channel 14)
    //                                   -----      rank  8 - 01111 (channel 15)
    //                                        ----- rank  7 - 01000 (channel  8)
    ADC1->SQR4 = 0b00001010011000111011100111101000;
    //             --                               reserved
    //               -----                          rank  6 - 01001 (channel  9)
    //                    -----                     rank  5 - 10110 (channel 22)
    //                         -----                rank  4 - 10111 (channel 23)
    //                              -----           rank  3 - 11000 (channel 24)
    //                                   -----      rank  2 - 11001 (channel 25)
    //                                        ----- rank  1 - 01010 (channel 10)
    ADC1->SQR5 = 0b00010011011010111110001100101010;
}

void configureBankB()
{
    LL_ADC_SetChannelsBank(ADC1, LL_ADC_CHANNELS_BANK_B);

    //             --                               reserved
    //               -----                          rank 24 -     0 (default/ignored)
    //                    -----                     rank 23 -     0 (default/ignored)
    //                         -----                rank 22 -     0 (default/ignored) 
    //                              -----           rank 21 -     0 (default/ignored) 
    //                                   -----      rank 20 - 00000 (channel  0b)
    //                                        ----- rank 19 - 00001 (channel  1b)
    ADC1->SQR2 = 0b00000000000000000000000000000001;
    //             --                               reserved
    //               -----                          rank 18 - 00010 (channel  2b)
    //                    -----                     rank 17 - 00011 (channel  3b)
    //                         -----                rank 16 - 00110 (channel  6b)
    //                              -----           rank 15 - 00111 (channel  7b)
    //                                   -----      rank 14 - 01000 (channel  8b)
    //                                        ----- rank 13 - 01001 (channel  9b)
    ADC1->SQR3 = 0b00000100001100110001110100001001;
    //             --                               reserved
    //               -----                          rank 12 - 10010 (channel 18b)
    //                    -----                     rank 11 - 10011 (channel 19b)
    //                         -----                rank 10 - 10100 (channel 20b)
    //                              -----           rank  9 - 10101 (channel 21b)
    //                                   -----      rank  8 - 01010 (channel 10b)
    //                                        ----- rank  7 - 01011 (channel 11b)
    ADC1->SQR4 = 0b00100101001110100101010101001011;
    //             --                               reserved
    //               -----                          rank  6 - 01100 (channel 12b)
    //                    -----                     rank  5 - 11011 (channel 27b)
    //                         -----                rank  4 - 11100 (channel 28b)
    //                              -----           rank  3 - 11101 (channel 29b)
    //                                   -----      rank  2 - 11110 (channel 30b)
    //                                        ----- rank  1 - 11111 (channel 31b)
    ADC1->SQR5 = 0b00011001101111100111011111011111;
}

void startQueryingAdc()
{
    configureBankA();
    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_1,
                           LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                           (uint32_t) &g_adcDataBuffer[0],
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_ADC_REG_StartConversionSWStart(ADC1);
}

void DMA1_Channel1_IRQHandler()
{
    if (LL_DMA_IsActiveFlag_TC1(DMA1) == 1)
    {
        if (LL_ADC_GetChannelsBank(ADC1) == LL_ADC_CHANNELS_BANK_A)
        {
            configureBankB();
            LL_DMA_ConfigAddresses(DMA1,
                                   LL_DMA_CHANNEL_1,
                                   LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                                   (uint32_t) &g_adcDataBuffer[20],
                                   LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
            LL_ADC_REG_StartConversionSWStart(ADC1);
        }
        else
        {
            for (uint32_t i = 0; i < NUMBER_OF_SENSORS; ++i)
            {
                g_dataBuffer
            }
            // TODO fix range
            // TODO convert to Q1.15
            // TODO swap ready buffer
        }
        LL_DMA_ClearFlag_TC1(DMA1);
    }
    if (LL_DMA_IsActiveFlag_TE1(DMA1) == 1)
    {
        for (;;);
    }
}
