/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include <stdio.h>

#include "output.h"
#include "global_data.h"

#include "assert.h"

uint32_t assertUInt32Equal(uint32_t expected, uint32_t actual, const char* pFileName, uint32_t lineNumber)
{
    if (expected != actual)
    {
        size_t messageLength = snprintf(g_testAssertBuffer, OUTPUT_BUFFER_LENGTH, "FAILED(%lu!=%lu), %s:%lu", expected, actual, pFileName, lineNumber);
        reportTestError(g_testAssertBuffer, messageLength);
        return 1;
    }
    else
    {
        return 0;
    }
}
