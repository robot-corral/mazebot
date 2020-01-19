/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "line_sensor.h"

#define ADC_BUFFER_1_START_IDX    1
#define ADC_BUFFER_2_START_IDX    1
#define ADC_BUFFER_2_SENSOR_INDEX ((LINE_SENSOR_NUMBER_OF_SENSORS) - 1)

typedef enum
{
    ADC_S_IDLE                = 0x00,
    ADC_S_CALIBRATION_PENDING = 0x01,
    ADC_S_CALIBRATING         = 0x02,
    ADC_S_SENSING             = 0x03,
} adcState_t;

 /*
 * initializes ADC hardware.
 *
 * should only be called once after reset.
 */
void initializeAdc();

/*
 * starts the process of querying ADC data. This call is asynchronous.
 */
void startQueryingAdc();

#define ADC_SQR_RANKS_COUNT(ranksCount) (((ranksCount) - 1) << 20)

#define ADC_SQR_RANK_1(channelIdx) (channelIdx)
#define ADC_SQR_RANK_2(channelIdx) ((channelIdx) << 5)
#define ADC_SQR_RANK_3(channelIdx) ((channelIdx) << 10)
#define ADC_SQR_RANK_4(channelIdx) ((channelIdx) << 15)
#define ADC_SQR_RANK_5(channelIdx) ((channelIdx) << 20)
#define ADC_SQR_RANK_6(channelIdx) ((channelIdx) << 25)

#define ADC_SMPR_RANK_01(samplingSpeed) (samplingSpeed)
#define ADC_SMPR_RANK_02(samplingSpeed) ((samplingSpeed) << 3)
#define ADC_SMPR_RANK_03(samplingSpeed) ((samplingSpeed) << 6)
#define ADC_SMPR_RANK_04(samplingSpeed) ((samplingSpeed) << 9)
#define ADC_SMPR_RANK_05(samplingSpeed) ((samplingSpeed) << 12)
#define ADC_SMPR_RANK_06(samplingSpeed) ((samplingSpeed) << 15)
#define ADC_SMPR_RANK_07(samplingSpeed) ((samplingSpeed) << 18)
#define ADC_SMPR_RANK_08(samplingSpeed) ((samplingSpeed) << 21)
#define ADC_SMPR_RANK_09(samplingSpeed) ((samplingSpeed) << 24)
#define ADC_SMPR_RANK_10(samplingSpeed) ((samplingSpeed) << 27)
