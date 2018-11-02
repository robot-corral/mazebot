/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include <stdio.h>

#include "rolling_values_test.h"

#include "debug.h"
#include "driver.h"

void printString(const char* pMessage, size_t messageLength)
{
    transmitDebugBuffer(pMessage, messageLength);
}

int main()
{
    initializeDebugDriver(true);

    test_calculateRolling16BitValueDifference();

    for (;;) ;
}
