/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

/*******************************************************************************
 * System Clock                                                                *
 *******************************************************************************/

bool g_isSystemClockInitialized = false;

/*******************************************************************************
 * Motor                                                                       *
 *******************************************************************************/

uint16_t g_leftMotorPreviousEncoderTicksA;
uint16_t g_leftMotorPreviousEncoderTicksB;
uint32_t g_leftMotorEncoderTicks;

uint16_t g_rightMotorPreviousEncoderTicksA;
uint16_t g_rightMotorPreviousEncoderTicksB;
uint32_t g_rightMotorEncoderTicks;
