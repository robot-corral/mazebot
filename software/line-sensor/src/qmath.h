#pragma once

#include <stdint.h>

static uint16_t convertValueToUQ1_15(uint16_t value, uint16_t maxValue)
{
    return value * 32768 / maxValue;
}
