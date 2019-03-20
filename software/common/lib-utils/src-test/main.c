/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include <stdio.h>

#include <test.h>

#include "rolling_values_test.h"

int main()
{
    initializeHardware();

    test_calculateRolling16BitValueDifference();

    for (;;) ;
}
