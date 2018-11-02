/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

char g_testAssertBuffer[OUTPUT_BUFFER_LENGTH];
char g_testOutputBuffer[OUTPUT_BUFFER_LENGTH];

volatile uint32_t g_testsFailed = 0;
volatile uint32_t g_testsSucceeded = 0;
