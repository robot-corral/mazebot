/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdbool.h>

void calibrateLineSensor();

void startQueryingLineSensor();

/*
 * Calling this method while data querying is in progress will result in undefined behavior
 */
float getLineDisplacementFromCenterlineInMeters();
