/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

#define FLOAT_PI 3.1415926535897932384626433f
#define FLOAT_DEGREES_TO_RADIANS (FLOAT_PI * 2.0f / 180.0f)

#define MIN(A, B) \
    ((A) < (B) ? (A) : (B))

#define MAX(A, B) \
    ((A) > (B) ? (A) : (B))

static uint16_t clampU16(uint16_t value, uint16_t min, uint16_t max)
{
    value = value > max ? max : value;
    return value < min ? 0 : value - min;
}

static uint16_t convertU16ValueToUQ1_15(uint16_t value, uint16_t maxValue)
{
    return (value << 15) / maxValue;
}
