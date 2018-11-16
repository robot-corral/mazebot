/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "stm32/stm32l4xx_ll_adc.h"
#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_dma.h"
#include "stm32/stm32l4xx_ll_rcc.h"

#include "status.h"
#include "timing.h"
#include "global_data.h"
#include "line_sensor.h"

#include "line_sensor_r1.h"

static void initializeAdc1();
static void initializeAdc1Dma();

static void initializeAdc2();
static void initializeAdc2Dma();

static void initializeAdc3();
static void initializeAdc3Dma();

static void activateAdc();

static void transferComplete();

void initializeLineSensorR1()
{
    LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_SYSCLK);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC);

    initializeAdc1();
    initializeAdc1Dma();

    initializeAdc2();
    initializeAdc2Dma();

    initializeAdc3();
    initializeAdc3Dma();

    activateAdc();
}

void initializeAdc1()
{
    LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_CLOCK_ASYNC_DIV1);
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);
    LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
    LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_OVERWRITTEN);
    LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_ENABLE_8RANKS);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_7);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_6);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_4, LL_ADC_CHANNEL_5);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_5, LL_ADC_CHANNEL_4);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_6, LL_ADC_CHANNEL_3);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_7, LL_ADC_CHANNEL_2);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_8, LL_ADC_CHANNEL_1);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_1, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_2, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_3, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_4, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_5, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_6, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_7, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_REG_RANK_8, LL_ADC_SAMPLINGTIME_2CYCLES_5);
}

void initializeAdc1Dma()
{
    NVIC_SetPriority(DMA1_Channel1_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_1,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_MODE_CIRCULAR              |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_HALFWORD        |
                          LL_DMA_MDATAALIGN_HALFWORD        |
                          LL_DMA_PRIORITY_HIGH);

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMA_REQUEST_0);

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_1,
                           LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                           (uint32_t) &g_lineSensorValues[7],
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 8);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
}

void initializeAdc2()
{
    LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(ADC2), LL_ADC_CLOCK_ASYNC_DIV1);
    LL_ADC_REG_SetTriggerSource(ADC2, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetContinuousMode(ADC2, LL_ADC_REG_CONV_SINGLE);
    LL_ADC_REG_SetDMATransfer(ADC2, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
    LL_ADC_REG_SetOverrun(ADC2, LL_ADC_REG_OVR_DATA_OVERWRITTEN);
    LL_ADC_REG_SetSequencerLength(ADC2, LL_ADC_REG_SEQ_SCAN_ENABLE_7RANKS);
    LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_15);
    LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_14);
    LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_13);
    LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_4, LL_ADC_CHANNEL_12);
    LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_5, LL_ADC_CHANNEL_11);
    LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_6, LL_ADC_CHANNEL_10);
    LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_7, LL_ADC_CHANNEL_9);
    LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_REG_RANK_1, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_REG_RANK_2, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_REG_RANK_3, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_REG_RANK_4, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_REG_RANK_5, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_REG_RANK_6, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_REG_RANK_7, LL_ADC_SAMPLINGTIME_2CYCLES_5);
}

void initializeAdc2Dma()
{
    NVIC_SetPriority(DMA1_Channel2_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_2,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_MODE_CIRCULAR              |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_HALFWORD        |
                          LL_DMA_MDATAALIGN_HALFWORD        |
                          LL_DMA_PRIORITY_HIGH);

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMA_REQUEST_0);

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_2,
                           LL_ADC_DMA_GetRegAddr(ADC2, LL_ADC_DMA_REG_REGULAR_DATA),
                           (uint32_t) &g_lineSensorValues[0],
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 7);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}

void initializeAdc3()
{
    LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(ADC3), LL_ADC_CLOCK_ASYNC_DIV1);
    LL_ADC_REG_SetTriggerSource(ADC3, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetContinuousMode(ADC3, LL_ADC_REG_CONV_SINGLE);
    LL_ADC_REG_SetDMATransfer(ADC3, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
    LL_ADC_REG_SetOverrun(ADC3, LL_ADC_REG_OVR_DATA_OVERWRITTEN);
    LL_ADC_REG_SetSequencerLength(ADC3, LL_ADC_REG_SEQ_SCAN_ENABLE_8RANKS);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_13);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_12);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_11);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_4, LL_ADC_CHANNEL_10);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_5, LL_ADC_CHANNEL_9);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_6, LL_ADC_CHANNEL_8);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_7, LL_ADC_CHANNEL_7);
    LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_8, LL_ADC_CHANNEL_6);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_1, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_2, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_3, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_4, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_5, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_6, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_7, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_REG_RANK_8, LL_ADC_SAMPLINGTIME_2CYCLES_5);
}

void initializeAdc3Dma()
{
    NVIC_SetPriority(DMA1_Channel3_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_3,
                          LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                          LL_DMA_MODE_CIRCULAR              |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_HALFWORD        |
                          LL_DMA_MDATAALIGN_HALFWORD        |
                          LL_DMA_PRIORITY_HIGH);

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMA_REQUEST_0);

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_3,
                           LL_ADC_DMA_GetRegAddr(ADC3, LL_ADC_DMA_REG_REGULAR_DATA),
                           (uint32_t) &g_lineSensorValues[15],
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, 8);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
}

void activateAdc()
{
    LL_ADC_DisableDeepPowerDown(ADC1);
    LL_ADC_EnableInternalRegulator(ADC1);

    LL_ADC_DisableDeepPowerDown(ADC2);
    LL_ADC_EnableInternalRegulator(ADC2);

    LL_ADC_DisableDeepPowerDown(ADC3);
    LL_ADC_EnableInternalRegulator(ADC3);

    // Note: Variable divided by 2 to compensate partially
    //       CPU processing cycles (depends on compilation optimization)
    volatile uint32_t voltage_stabilization_wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
    while (voltage_stabilization_wait_loop_index != 0)
    {
        voltage_stabilization_wait_loop_index--;
    }

    LL_ADC_StartCalibration(ADC1, LL_ADC_SINGLE_ENDED);
    LL_ADC_StartCalibration(ADC2, LL_ADC_SINGLE_ENDED);
    LL_ADC_StartCalibration(ADC3, LL_ADC_SINGLE_ENDED);

    while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0) ;
    while (LL_ADC_IsCalibrationOnGoing(ADC2) != 0) ;
    while (LL_ADC_IsCalibrationOnGoing(ADC3) != 0) ;

    // Note: Variable divided by 2 to compensate partially
    //       CPU processing cycles (depends on compilation optimization)
    volatile uint32_t adc_enable_wait_loop_index = ((LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES * 32) >> 1);
    while (adc_enable_wait_loop_index != 0)
    {
        adc_enable_wait_loop_index--;
    }

    LL_ADC_Enable(ADC1);
    LL_ADC_Enable(ADC2);
    LL_ADC_Enable(ADC3);

    while (LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0) ;
    while (LL_ADC_IsActiveFlag_ADRDY(ADC2) == 0) ;
    while (LL_ADC_IsActiveFlag_ADRDY(ADC3) == 0) ;
}

uint32_t startQueryingLineSensor()
{
    setOutput1High();
    LL_ADC_REG_StartConversion(ADC1);
    LL_ADC_REG_StartConversion(ADC3);
    // ADC2 is last as it has 7 sensors to query not 8
    LL_ADC_REG_StartConversion(ADC2);
    return 153847; // 1 / 6.5 usec
}

void transferComplete()
{
    if (LL_ADC_REG_IsConversionOngoing(ADC1) == 0 && LL_ADC_REG_IsConversionOngoing(ADC2) == 0 && LL_ADC_REG_IsConversionOngoing(ADC3) == 0)
    {
        setOutput1Low();
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE1(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TE1(DMA1);
        setCriticalError(ERROR_DMA);
    }
    if (LL_DMA_IsActiveFlag_TC1(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TC1(DMA1);
        transferComplete();
    }
}

void DMA1_Channel2_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE2(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TE2(DMA1);
        setCriticalError(ERROR_DMA);
    }
    if (LL_DMA_IsActiveFlag_TC2(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TC2(DMA1);
        transferComplete();
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TE3(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TE3(DMA1);
        setCriticalError(ERROR_DMA);
    }
    if (LL_DMA_IsActiveFlag_TC3(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TC3(DMA1);
        transferComplete();
    }
}
