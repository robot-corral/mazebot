/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

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

#define ADC_SMPR_RANK_1(samplingSpeed) (samplingSpeed)
#define ADC_SMPR_RANK_2(samplingSpeed) ((samplingSpeed) << 3)
#define ADC_SMPR_RANK_3(samplingSpeed) ((samplingSpeed) << 6)
#define ADC_SMPR_RANK_4(samplingSpeed) ((samplingSpeed) << 9)
#define ADC_SMPR_RANK_5(samplingSpeed) ((samplingSpeed) << 12)
#define ADC_SMPR_RANK_6(samplingSpeed) ((samplingSpeed) << 15)
#define ADC_SMPR_RANK_7(samplingSpeed) ((samplingSpeed) << 18)
#define ADC_SMPR_RANK_8(samplingSpeed) ((samplingSpeed) << 21)
#define ADC_SMPR_RANK_9(samplingSpeed) ((samplingSpeed) << 24)
#define ADC_SMPR_RANK_10(samplingSpeed) ((samplingSpeed) << 27)
