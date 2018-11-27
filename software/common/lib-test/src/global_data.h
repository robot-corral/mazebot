/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

#define OUTPUT_BUFFER_LENGTH 128

extern volatile uint32_t g_testsFailed;
extern volatile uint32_t g_testsSucceeded;

extern char g_testAssertBuffer[OUTPUT_BUFFER_LENGTH];
extern char g_testOutputBuffer[OUTPUT_BUFFER_LENGTH];
