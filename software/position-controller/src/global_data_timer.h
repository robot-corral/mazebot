#pragma once

#include <stdint.h>

#define DMA_TIMER_STEPS_COUNT       385

#define DMA_TIMER_MIN_FREQUENCY_ARR 399999
#define DMA_TIMER_MIN_FREQUENCY_CC  199999

#define DMA_TIMER_MAX_FREQUENCY_ARR 26665

extern const uint32_t g_dmaTimerDataIncreasing[DMA_TIMER_STEPS_COUNT * 3];
extern const uint32_t g_dmaTimerDataDecreasing[DMA_TIMER_STEPS_COUNT * 3];
