/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef ASSERT_PREFIX
    #define ASSERT_PREFIX "    "
#endif

#ifndef TEST_SUITE_PREFIX
    #define TEST_SUITE_PREFIX
#endif

#ifndef TEST_PREFIX
    #define TEST_PREFIX "  "
#endif

// you can implement any of these methods in your code to customize their behavior

void reportTestSuiteStart(const char* pTestSuiteName);

void reportTestStart(const char* pTestSuiteName, const char* pTestName);

void reportTestError(const char* pMessage, size_t messageLength);

void reportTestResult(const char* pTestSuiteName, const char* pTestName, uint32_t numberOfFailedAssertions);

void reportTestSuiteResult(const char* pTestSuiteName, uint32_t numberOfSuccessfulTests, uint32_t numberOfFailedTests);

void reportTestRunSummary();

void printString(const char* pMessage, size_t messageLength);
