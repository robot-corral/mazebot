/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * System Clock                                                                *
 *******************************************************************************/

extern bool g_isSystemClockInitialized;

/*******************************************************************************
 * Motor                                                                       *
 *******************************************************************************/

extern uint16_t g_leftMotorPreviousEncoderTicksA;
extern uint16_t g_leftMotorPreviousEncoderTicksB;
extern uint32_t g_leftMotorEncoderTicks;

extern uint16_t g_rightMotorPreviousEncoderTicksA;
extern uint16_t g_rightMotorPreviousEncoderTicksB;
extern uint32_t g_rightMotorEncoderTicks;
