/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define LINE_SENSOR_COUNT 23

/*
 * returns - frequency (Hz) which is a reciprocal of time (sec) required to complete querying line sensor
 */
uint32_t startQueryingLineSensor();

/*
 * Calling this method while data querying is in progress will result in undefined behavior
 *
 * lineSensorValues - array to store normalized [0...1] sensors values
 */
bool getLineSensorData(uint16_t lineSensorValues[LINE_SENSOR_COUNT]);