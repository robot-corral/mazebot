/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include <stdio.h>

#include "rolling_values_test.h"

void printString(const char* pMessage, size_t messageLength)
{
    // TODO transmitDebugBuffer(pMessage, messageLength);
}

int main()
{
    // TODO initializeDebugDriver(true);

    test_calculateRolling16BitValueDifference();

    for (;;) ;
}
