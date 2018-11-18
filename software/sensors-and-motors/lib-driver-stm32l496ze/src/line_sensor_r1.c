/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include <math.h>

#include "stm32/stm32l4xx_ll_adc.h"
#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_dma.h"
#include "stm32/stm32l4xx_ll_rcc.h"

#include "../global_defs.h"

#include "clock.h"
#include "status.h"
#include "timing.h"
#include "buttons.h"
#include "math_utils.h"
#include "global_data.h"
#include "line_sensor.h"

#include "line_sensor_r1.h"

#define MIN_SAFETY_FACTOR 2.5f

static void initializeAdc1();
static void initializeAdc1Dma();

static void initializeAdc2();
static void initializeAdc2Dma();

static void initializeAdc3();
static void initializeAdc3Dma();

static void activateAdc();

static void transferComplete();

static bool isLineSensorBusy();

void initializeLineSensorR1()
{
    g_lineDisplacementFromCenterlineInMeters = 0.0f;

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
    NVIC_SetPriority(DMA1_Channel1_IRQn, 2);
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
    NVIC_SetPriority(DMA1_Channel2_IRQn, 2);
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
    NVIC_SetPriority(DMA1_Channel3_IRQn, 2);
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

void startQueryingLineSensor()
{
    setOutput1High();
    LL_ADC_REG_StartConversion(ADC1);
    LL_ADC_REG_StartConversion(ADC3);
    // ADC2 is last as it has 7 sensors to query not 8
    LL_ADC_REG_StartConversion(ADC2);
}

void calibrateLineSensor()
{
#ifndef USE_HARDCODED_CALIBRATION_VALUES
    for (;;)
    {
        for (uint32_t i = 0; i < LINE_SENSOR_COUNT; ++i)
        {
            g_lineSensorMinValues[i] = 0;
            g_lineSensorMaxValues[i] = 0xFFFF;
        }

        setCalibrationStatus(CS_CALIBRATING_WHITE);

        waitForStartPauseButtonPress();

        uint32_t startTime = getCurrentTimeInMicroseconds();

        do
        {
            startQueryingLineSensor();
            while (isLineSensorBusy()) ;
            for (uint32_t i = 0; i < LINE_SENSOR_COUNT; ++i)
            {
                g_lineSensorMinValues[i] = MAX(g_lineSensorMinValues[i], g_lineSensorValues[i]);
            }
        } while (getDifferenceWithCurrentTime(startTime) < 2000000 /* 2 seconds */);

        setCalibrationStatus(CS_CALIBRATING_BLACK);

        waitForStartPauseButtonPress();

        startTime = getCurrentTimeInMicroseconds();

        do
        {
            startQueryingLineSensor();
            while (isLineSensorBusy()) ;
            for (uint32_t i = 0; i < LINE_SENSOR_COUNT; ++i)
            {
                g_lineSensorMaxValues[i] = MIN(g_lineSensorMaxValues[i], g_lineSensorValues[i]);
            }
        } while (getDifferenceWithCurrentTime(startTime) < 2000000 /* 2 seconds */);

        bool isValid = true;

        for (uint32_t i = 0; i < LINE_SENSOR_COUNT; ++i)
        {
            if (g_lineSensorMaxValues[i] < MIN_SAFETY_FACTOR * g_lineSensorMinValues[i])
            {
                isValid = false;
            }
        }

        if (isValid)
        {
            break;
        }
    }

    setCalibrationStatus(CS_OFF);
#else
    g_lineSensorMinValues[ 0] = 273.0f;
    g_lineSensorMinValues[ 1] = 273.0f;
    g_lineSensorMinValues[ 2] = 273.0f;
    g_lineSensorMinValues[ 3] = 273.0f;
    g_lineSensorMinValues[ 4] = 273.0f;
    g_lineSensorMinValues[ 5] = 273.0f;
    g_lineSensorMinValues[ 6] = 273.0f;
    g_lineSensorMinValues[ 7] = 273.0f;
    g_lineSensorMinValues[ 8] = 273.0f;
    g_lineSensorMinValues[ 9] = 273.0f;
    g_lineSensorMinValues[10] = 273.0f;
    g_lineSensorMinValues[11] = 273.0f;
    g_lineSensorMinValues[12] = 273.0f;
    g_lineSensorMinValues[13] = 273.0f;
    g_lineSensorMinValues[14] = 273.0f;
    g_lineSensorMinValues[15] = 273.0f;
    g_lineSensorMinValues[16] = 273.0f;
    g_lineSensorMinValues[17] = 273.0f;
    g_lineSensorMinValues[18] = 273.0f;
    g_lineSensorMinValues[19] = 273.0f;
    g_lineSensorMinValues[20] = 273.0f;
    g_lineSensorMinValues[21] = 273.0f;
    g_lineSensorMinValues[22] = 273.0f;
    g_lineSensorMinValues[23] = 273.0f;

    // edge values are less lit so they don't get oversaturated and their value is a bit higher
    g_lineSensorMaxValues[ 0] = 2700.0f;
    g_lineSensorMaxValues[ 1] = 2300.0f;
    g_lineSensorMaxValues[ 2] = 2300.0f;
    g_lineSensorMaxValues[ 3] = 2300.0f;
    g_lineSensorMaxValues[ 4] = 2300.0f;
    g_lineSensorMaxValues[ 5] = 2300.0f;
    g_lineSensorMaxValues[ 6] = 2300.0f;
    g_lineSensorMaxValues[ 7] = 2300.0f;
    g_lineSensorMaxValues[ 8] = 2300.0f;
    g_lineSensorMaxValues[ 9] = 2300.0f;
    g_lineSensorMaxValues[10] = 2300.0f;
    g_lineSensorMaxValues[11] = 2300.0f;
    g_lineSensorMaxValues[12] = 2300.0f;
    g_lineSensorMaxValues[13] = 2300.0f;
    g_lineSensorMaxValues[14] = 2300.0f;
    g_lineSensorMaxValues[15] = 2300.0f;
    g_lineSensorMaxValues[16] = 2300.0f;
    g_lineSensorMaxValues[17] = 2300.0f;
    g_lineSensorMaxValues[18] = 2300.0f;
    g_lineSensorMaxValues[19] = 2300.0f;
    g_lineSensorMaxValues[20] = 2300.0f;
    g_lineSensorMaxValues[21] = 2300.0f;
    g_lineSensorMaxValues[22] = 2300.0f;
    g_lineSensorMaxValues[23] = 2700.0f;
#endif
}

void calculateLineDisplacementFromCenterlineInMeters()
{
    // TODO
}

float getLineDisplacementFromCenterlineInMeters()
{
    return g_lineDisplacementFromCenterlineInMeters;
}

bool isLineSensorBusy()
{
    return LL_ADC_REG_IsConversionOngoing(ADC1) != 0 ||
           LL_ADC_REG_IsConversionOngoing(ADC2) != 0 ||
           LL_ADC_REG_IsConversionOngoing(ADC3) != 0;
}

void transferComplete()
{
    if (!isLineSensorBusy())
    {
        calculateLineDisplacementFromCenterlineInMeters();
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
