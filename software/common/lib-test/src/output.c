/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "global_data.h"

#include "output.h"

void __attribute__((weak)) reportTestSuiteStart(const char* pTestSuiteName)
{
    size_t messageLength = snprintf(g_testOutputBuffer, OUTPUT_BUFFER_LENGTH, TEST_SUITE_PREFIX"%s\r\n", pTestSuiteName);
    printString(g_testOutputBuffer, messageLength);
}

void __attribute__((weak)) reportTestStart(const char* pTestSuiteName, const char* pTestName)
{
    size_t messageLength = snprintf(g_testOutputBuffer, OUTPUT_BUFFER_LENGTH, TEST_PREFIX"%s", pTestName);
    printString(g_testOutputBuffer, messageLength);
}

void __attribute__((weak)) reportTestError(const char* pMessage, size_t messageLength)
{
    messageLength = snprintf(g_testOutputBuffer, OUTPUT_BUFFER_LENGTH, "\r\n"ASSERT_PREFIX"%s", pMessage);
    printString(g_testOutputBuffer, messageLength);
}

void __attribute__((weak)) reportTestResult(const char* pTestSuiteName, const char* pTestName, uint32_t numberOfFailedAssertions)
{
    const char* pDoneString = "DONE\r\n";
    if (numberOfFailedAssertions > 0)
    {
        uint32_t messageLength = snprintf(g_testOutputBuffer, OUTPUT_BUFFER_LENGTH, "\r\n"TEST_PREFIX"%s", pDoneString);
        printString(g_testOutputBuffer, messageLength);
    }
    else
    {
        uint32_t messageLength = snprintf(g_testOutputBuffer, OUTPUT_BUFFER_LENGTH, ": %s", pDoneString);
        printString(g_testOutputBuffer, messageLength);
    }
}

void __attribute__((weak)) reportTestSuiteResult(const char* pTestSuiteName, uint32_t numberOfSuccessfulTests, uint32_t numberOfFailedTests)
{
    g_testsFailed += numberOfFailedTests;
    g_testsSucceeded += numberOfSuccessfulTests;
    const char* pEndString = TEST_SUITE_PREFIX"END\r\n";
    printString(pEndString, strlen(pEndString));
}

void __attribute__((weak)) reportTestRunSummary()
{
    size_t messageLength = snprintf(g_testOutputBuffer, OUTPUT_BUFFER_LENGTH, "\r\nTOTAL=%lu, FAILED=%lu, SUCCEEDED=%lu\r\n",
                                    g_testsFailed + g_testsSucceeded,
                                    g_testsFailed,
                                    g_testsSucceeded);
    printString(g_testOutputBuffer, messageLength);
}
