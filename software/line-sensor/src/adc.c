/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "adc.h"

#include "status.h"
#include "watchdog.h"
#include "global_data.h"
#include "consumer_producer_buffer.h"

#include <math_utils.h>

#include <stm32/stm32l1xx_ll_adc.h>
#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_dma.h>

static void configureBankA();
static void configureBankB();
static bool processAdcData();

static void startCalibration();
static bool collectCalibrationData();
static bool collectSensorValues();

void initializeAdc()
{
    g_adcState = ADC_S_SENSING;

    // ADC clock is by default with prescaler DIV1

    ADC1->CR1 = ADC_CR1_SCAN;
    ADC1->CR2 = ADC_CR2_DMA | ADC_CR2_DDS;

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

    ADC1->SMPR1 = ADC_SMPR_RANK_01(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_02(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_03(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_04(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_05(LL_ADC_SAMPLINGTIME_48CYCLES);
    ADC1->SMPR2 = ADC_SMPR_RANK_01(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_02(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_03(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_04(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_05(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_06(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_07(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_08(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_09(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_10(LL_ADC_SAMPLINGTIME_48CYCLES);
    ADC1->SMPR2 = ADC_SMPR_RANK_01(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_02(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_03(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_04(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_05(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_06(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_07(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_08(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_09(LL_ADC_SAMPLINGTIME_48CYCLES) |
                  ADC_SMPR_RANK_10(LL_ADC_SAMPLINGTIME_48CYCLES);

    COMP->CSR = COMP_CSR_FCH3 | COMP_CSR_FCH8;

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
    // DMA must be disabled before calling LL_DMA_ConfigAddresses / LL_DMA_SetDataLength
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
        LL_DMA_ClearFlag_TE1(DMA1);
        setSensorStatusFlags(LSDS_ERR_FLAG_ADC_DMA_FAILURE);
        startQueryingAdc();
    }
    else if (LL_DMA_IsActiveFlag_TC1(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TC1(DMA1);

        if (LL_ADC_GetChannelsBank(ADC1) == LL_ADC_CHANNELS_BANK_A)
        {
            configureBankB();
            // DMA must be disabled before calling LL_DMA_ConfigAddresses / LL_DMA_SetDataLength
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
            if (processAdcData())
            {
                resetSensorStatusFlags(LSDS_ERR_FLAG_ADC_ALL);
                startQueryingAdc();
                resetWatchdog(WS_ADC);
            }
            else
            {
                setSensorStatusFlags(LSDS_ERR_FLAG_ADC_DATA_BUFFER_CORRUPTED);
                // even if it breaks data which is being sent we should reset to ground zero so we can send good data next time
                consumerProducerBuffer_reset(&g_lineSensorValuesBuffersProducerConsumerIndexes);
                startQueryingAdc();
            }
        }
    }
}

static bool processAdcData()
{
    switch (g_adcState)
    {
        case ADC_S_CALIBRATION_PENDING : startCalibration(); return true;
        case ADC_S_CALIBRATING         : return collectCalibrationData();
        case ADC_S_SENSING             : return collectSensorValues();
        default                        : return true;
    }
}

void startCalibration()
{
    if (g_adcCalibrationParameter == (LSR_SC_P_FLAG_RESET_PREVIOUS_CALIBRATION_DATA | LSR_SC_P_WHITE_CALIBRATION))
    {
        for (int i = 0; i < NUMBER_OF_SENSORS; ++i)
        {
            g_calibrationMinValues[i] = 0;
        }
    }
    else if (g_adcCalibrationParameter == (LSR_SC_P_FLAG_RESET_PREVIOUS_CALIBRATION_DATA | LSR_SC_P_BLACK_CALIBRATION))
    {
        for (int i = 0; i < NUMBER_OF_SENSORS; ++i)
        {
            g_calibrationMaxValues[i] = 0;
        }
    }
    g_adcState = ADC_S_CALIBRATING;
    g_adcCalibrationParameter &= ~LSR_SC_P_FLAG_RESET_PREVIOUS_CALIBRATION_DATA;
}

bool collectCalibrationData()
{
    const uint8_t producerBufferIndex = consumerProducerBuffer_getProducerIndex(&g_lineSensorCalibrationValuesBuffersProducerConsumerIndexes);

    if (producerBufferIndex == DATA_BUFFER_LENGTH)
    {
        return false;
    }

    if (g_adcCalibrationParameter == LSR_SC_P_WHITE_CALIBRATION)
    {
        for (uint8_t i = ADC_BUFFER_1_START_IDX; i < ADC_BUFFER_1_LENGTH; ++i)
        {
            g_calibrationMinValues[i - ADC_BUFFER_1_START_IDX] = MAX(g_calibrationMinValues[i - ADC_BUFFER_1_START_IDX], g_adcBuffer1[i]);
        }

        g_calibrationMinValues[ADC_BUFFER_2_SENSOR_INDEX] = MAX(g_calibrationMinValues[ADC_BUFFER_2_SENSOR_INDEX], g_adcBuffer2[ADC_BUFFER_2_START_IDX]);
    }
    else if (g_adcCalibrationParameter == LSR_SC_P_BLACK_CALIBRATION)
    {
        for (uint8_t i = ADC_BUFFER_1_START_IDX; i < ADC_BUFFER_1_LENGTH; ++i)
        {
            g_calibrationMaxValues[i - ADC_BUFFER_1_START_IDX] = MAX(g_calibrationMaxValues[i - ADC_BUFFER_1_START_IDX], g_adcBuffer1[i]);
        }

        g_calibrationMaxValues[ADC_BUFFER_2_SENSOR_INDEX] = MAX(g_calibrationMaxValues[ADC_BUFFER_2_SENSOR_INDEX], g_adcBuffer2[ADC_BUFFER_2_START_IDX]);
    }

    volatile lineSensorResponseGetCalibrationValues_t* const calibrationData = &g_lineSensorCalibrationValuesBuffers[producerBufferIndex];

    for (uint8_t i = ADC_BUFFER_1_START_IDX; i < ADC_BUFFER_1_LENGTH; ++i)
    {
        calibrationData->minSensorCalibrationValues[i] = g_calibrationMinValues[i];
        calibrationData->maxSensorCalibrationValues[i] = g_calibrationMaxValues[i];
    }

    calibrationData->currentStatus = LSS_OK_FLAG_DATA_AVAILABLE | LSS_OK_FLAG_NEW_DATA_AVAILABLE;

    return consumerProducerBuffer_setLastReadIndex(&g_lineSensorCalibrationValuesBuffersProducerConsumerIndexes, producerBufferIndex);
}

bool collectSensorValues()
{
    const uint8_t producerBufferIndex = consumerProducerBuffer_getProducerIndex(&g_lineSensorValuesBuffersProducerConsumerIndexes);

    if (producerBufferIndex == DATA_BUFFER_LENGTH)
    {
        return false;
    }

    volatile lineSensorValue_t* const sensorData = g_lineSensorValuesBuffers[producerBufferIndex].sensorValues;

    for (uint8_t i = ADC_BUFFER_1_START_IDX; i < ADC_BUFFER_1_LENGTH; ++i)
    {
        sensorData[i - ADC_BUFFER_1_START_IDX] = g_adcBuffer1[i];
    }

    sensorData[ADC_BUFFER_2_SENSOR_INDEX] = g_adcBuffer2[ADC_BUFFER_2_START_IDX];

    g_lineSensorValuesBuffers[producerBufferIndex].currentStatus = LSS_OK_FLAG_DATA_AVAILABLE | LSS_OK_FLAG_NEW_DATA_AVAILABLE;

    return consumerProducerBuffer_setLastReadIndex(&g_lineSensorValuesBuffersProducerConsumerIndexes, producerBufferIndex);
}
