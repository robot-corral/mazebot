/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

static inline uint32_t getInterruptMask()
{
    uint32_t result;
    asm volatile ("MRS %0, primask" : "=r" (result) :: "memory");
    return result;
}

static inline void setInterruptMask(uint32_t priMask)
{
  asm volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}

static inline void enableInterrupts()
{
    asm volatile ("cpsie i" : : : "memory");
}

static inline void disableInterrupts()
{
    asm volatile ("cpsid i" : : : "memory");
}
