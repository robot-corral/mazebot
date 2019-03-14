/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "adc.h"

#include "qmath.h"
#include "global_data.h"
#include "buffer_index.h"
#include "system_clocks.h"

#include <stm32/stm32l1xx_ll_adc.h>
#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_dma.h>

static void configureBankA();
static void configureBankB();
static void processAdcData();

void initializeAdc()
{
    //            -----                            reserved
    //                 -                           OVRIE   - 0 (overrun interrupt enable, disabled)
    //                  --                         RES     - 0 (resolution, 12 bit)
    //                    -                        AWDEN   - 0 (analog watchdog enable on regular channels, disabled)
    //                     -                       JAWDEN  - 0 (analog watchdog enable on injected channels, disabled)
    //                      ----                   reserved
    //                          -                  PDI     - 0 (power down during the idle phase, disabled)
    //                           -                 PDD     - 0 (power down during the delay phase, disabled)
    //                            ---              DISCNUM - 0 (discontinuous mode channel count, default/ignored)
    //                               -             JDISCEN - 0 (discontinuous mode on injected channels, disabled)
    //                                -            DISCEN  - 0 (discontinuous mode on regular channels, disabled)
    //                                 -           JAUTO   - 0 (automatic injected group conversion, disabled)
    //                                  -          AWDSGL  - 0 (enable the watchdog on a single channel in scan mode, disabled)
    //                                   -         SCAN    - 1 (scan mode, enabled)
    //                                    -        JEOCIE  - 0 (interrupt enable for injected channels, disabled)
    //                                     -       AWDIE   - 0 (analog watchdog interrupt, disabled)
    //                                      -      EOCIE   - 0 (end of conversation interrupt, enabled)
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

    // the following ranks are used in bank A only and these registers never change
    // so we can set them only once

    ADC1->SQR2 = ADC_SQR_RANK_1(23) |
                 ADC_SQR_RANK_2(24) |
                 ADC_SQR_RANK_3(25) |
                 ADC_SQR_RANK_4(18) |
                 ADC_SQR_RANK_5(19) |
                 ADC_SQR_RANK_6(20);

    ADC1->SQR3 = ADC_SQR_RANK_1(7)  |
                 ADC_SQR_RANK_2(14) |
                 ADC_SQR_RANK_3(15) |
                 ADC_SQR_RANK_4(8)  |
                 ADC_SQR_RANK_5(9)  |
                 ADC_SQR_RANK_6(22);

    ADC1->SQR4 = ADC_SQR_RANK_1(1) |
                 ADC_SQR_RANK_2(2) |
                 ADC_SQR_RANK_3(3) |
                 ADC_SQR_RANK_4(4) |
                 ADC_SQR_RANK_5(5) |
                 ADC_SQR_RANK_6(6);

    // 48 improves ADC accuracy and on release this is enough for 10'000 scans per second
    // (one scan is reading all sensor values)

    ADC1->SMPR1 = ADC_SMPR_RANK_1(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_2(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_3(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_4(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_5(LL_ADC_SAMPLINGTIME_48CYCLES);
    ADC1->SMPR2 = ADC_SMPR_RANK_1(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_2(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_3(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_4(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_5(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_6(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_7(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_8(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_9(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_10(LL_ADC_SAMPLINGTIME_48CYCLES);
    ADC1->SMPR2 = ADC_SMPR_RANK_1(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_2(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_3(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_4(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_5(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_6(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_7(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_8(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_9(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_10(LL_ADC_SAMPLINGTIME_48CYCLES);

    //                -                             FCH8: Select GPIO port PB0 as fast ADC input channel CH8
    //                 -                            FCH3: Select GPIO port PA3 as fast ADC input channel CH3.
    COMP->CSR = 0b00001100000000000000000000000000;

    LL_ADC_Enable(ADC1);
}

void configureBankA()
{
    LL_ADC_SetChannelsBank(ADC1, LL_ADC_CHANNELS_BANK_A);

    ADC1->SQR1 = ADC_SQR_RANKS_COUNT(ADC_BUFFER_1_LENGTH) | ADC_SQR_RANK_1(21);

    // 1st channel in sequence has higher than normal value
    // reading the same channel twice fixes this issue
    ADC1->SQR5 = ADC_SQR_RANK_1(10) |
                 ADC_SQR_RANK_2(10) |
                 ADC_SQR_RANK_3(11) |
                 ADC_SQR_RANK_4(12) |
                 ADC_SQR_RANK_5(13) |
                 ADC_SQR_RANK_6(0);
}

void configureBankB()
{
    LL_ADC_SetChannelsBank(ADC1, LL_ADC_CHANNELS_BANK_B);

    ADC1->SQR1 = ADC_SQR_RANKS_COUNT(ADC_BUFFER_2_LENGTH);

    // 1st channel in sequence has higher than normal value
    // reading the same channel twice fixes this issue
    ADC1->SQR5 = ADC_SQR_RANK_1(0) |
                 ADC_SQR_RANK_2(0);
}

void startQueryingAdc()
{
    configureBankA();

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_1,
                           LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                           (uint32_t) g_adcBuffer1,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, ADC_BUFFER_1_LENGTH);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_ADC_REG_StartConversionSWStart(ADC1);
}

void DMA1_Channel1_IRQHandler()
{
    if (LL_DMA_IsActiveFlag_TE1(DMA1) == 1)
    {
        // TODO handle error
    }
    if (LL_DMA_IsActiveFlag_TC1(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TC1(DMA1);

        if (LL_ADC_GetChannelsBank(ADC1) == LL_ADC_CHANNELS_BANK_A)
        {
            configureBankB();
            const bufferIndex_t bufferIndex = getCurrentProducerIndex();
            LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
            LL_DMA_ConfigAddresses(DMA1,
                                   LL_DMA_CHANNEL_1,
                                   LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                                   (uint32_t) g_adcBuffer2,
                                   LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
            LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, ADC_BUFFER_2_LENGTH);
            LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
            LL_ADC_REG_StartConversionSWStart(ADC1);
        }
        else
        {
            processAdcData();
            startQueryingAdc();
        }
    }
}

static void processAdcData()
{
}
