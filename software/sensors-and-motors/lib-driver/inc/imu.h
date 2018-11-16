/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

/*
 * returns - frequency (Hz) which is a reciprocal of time (sec) required to complete querying line sensor
 */
uint32_t startQueryingImu();

/*
 * Calling this method while data querying is in progress will result in undefined behavior
 *
 * returns - yaw angle delta in radians since last query. It can be 0 if device didn't get new data.
 *           Querying it multiple times between calls to startQueryingImu() will return the same value.
 */
float getImuYawAngleDeltaRadians();
