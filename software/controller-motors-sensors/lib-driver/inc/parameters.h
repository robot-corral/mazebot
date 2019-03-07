/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#define MAX_MOTOR_POWER   1000

#define RADIUS_METERS     (25.0f / 1000.0f)
// distance between centerline of wheels on the same axle
#define AXLE_TRACK_METERS (141.0f / 1000.0f)

#define PULSES_PER_RPM    12
#define GEARBOX_RATIO     (25.0f * 34.0f * 37.0f * 35.0f * 38.0f / (12.0f * 10.0f * 10.0f * 14.0f * 10.0f))
#define TICKS_TO_RADIANS  (2.0f * M_PI / ((PULSES_PER_RPM) * (GEARBOX_RATIO)))
