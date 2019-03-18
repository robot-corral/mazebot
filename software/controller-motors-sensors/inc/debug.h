/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

/*
 * Transmits buffer data to debug output and returns after transmission completed.
 */
void transmitDebugBuffer(const char* pBuffer, uint32_t length);
