/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define LINE_SENSOR_COUNT 23

/*
 * lineSensorValues - array to store normalized [0...1] sensors values
 */
bool getLineSensorData(uint16_t lineSensorValues[LINE_SENSOR_COUNT]);
