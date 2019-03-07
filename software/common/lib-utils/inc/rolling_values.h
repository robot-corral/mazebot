/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

uint16_t calculateRolling16BitValueDifference(uint16_t previousValue, uint16_t newValue);
uint32_t calculateRolling32BitValueDifference(uint32_t previousValue, uint32_t newValue);
