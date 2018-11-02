/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "inc/rolling_values.h"

uint16_t calculateRolling16BitValueDifference(uint16_t previousValue, uint16_t newValue)
{
    if (previousValue <= newValue)
    {
        return newValue - previousValue;
    }
    else
    {
        return 0xFFFF - previousValue + 1 + newValue;
    }
}
