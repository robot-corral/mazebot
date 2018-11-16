/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "clock.h"

#include "rolling_values.h"

uint32_t getDifferenceWithCurrentTime(uint32_t startTime)
{
    return calculateRolling32BitValueDifference(startTime, getCurrentTimeInMicroseconds());
}
