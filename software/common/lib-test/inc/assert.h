/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ASSERT_IS_TRUE(actualValue) \
    __test_assertions_failed += assertUInt32Equal(true, actualValue, __FILE__, __LINE__);

#define ASSERT_IS_FALSE(actualValue) \
    __test_assertions_failed += assertUInt32Equal(false, actualValue, __FILE__, __LINE__);

#define ASSERT_UINT32_EQUAL(expectedValue, actualValue) \
    __test_assertions_failed += assertUInt32Equal(expectedValue, actualValue, __FILE__, __LINE__);

uint32_t assertUInt32Equal(uint32_t expected, uint32_t actual, const char* pFileName, uint32_t lineNumber);
