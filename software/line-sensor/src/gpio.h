/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdbool.h>

/*
 * initializes all GPIO pins as fast as possible to decrease startup time.
 * should only be called once after reset.
 */
void initializeGpio();
