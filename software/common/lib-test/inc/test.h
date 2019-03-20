/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

#include "assert.h"
#include "../src/output.h"

#define START_TEST_SUITE() \
    const char* __test_suite_name = __func__; \
    char* __test_name = 0; \
    uint32_t __tests_failed = 0; \
    uint32_t __tests_succeeded = 0; \
    uint32_t __test_assertions_failed = 0; \
    reportTestSuiteStart(__test_suite_name);

#define START_TEST(test_name) \
    __test_name = #test_name; \
    __test_assertions_failed = 0; \
    reportTestStart(__test_suite_name, __test_name); \
    {

#define END_TEST() \
    } \
    if (__test_assertions_failed > 0) \
    { \
        ++__tests_failed; \
    } else { \
        ++__tests_succeeded;\
    } \
    reportTestResult(__test_suite_name, __test_name, __test_assertions_failed);

#define END_TEST_SUITE() \
    reportTestSuiteResult(__test_suite_name, __tests_succeeded, __tests_failed);

void initializeHardware();
