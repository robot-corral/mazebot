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
