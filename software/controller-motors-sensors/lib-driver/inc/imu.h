/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

void startQueryingImu();

/*
 * Calling this method while data querying is in progress will result in undefined behavior
 *
 * returns - yaw angle delta in radians / second since last query. It can be 0 if device didn't get new data.
 *           Querying it multiple times between calls to startQueryingImu() will return the same value.
 */
float getImuYawAngleDeltaRadiansPerSecond();
