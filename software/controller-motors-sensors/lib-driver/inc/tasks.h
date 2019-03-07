/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

void startTaskScheduler();

/*
 * returns - frequency (Hz) of how often this task should be called from now on
 */
uint32_t __attribute__((weak)) asyncTaskCallback();
