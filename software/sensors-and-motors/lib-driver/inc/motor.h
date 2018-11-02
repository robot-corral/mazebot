/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

void enableLeftMotor();

/*
 * power -> motor power in the range [-1000, 1000] (negative value means rotate backwards)
 */
void setLeftMotorPower(int16_t power);

/*
 * encoder counter can overflow so make sure to read this value at least once per second
 *
 * this is a single threaded method (or not-interrupt safe) so use it from one 'thread' only
 */
uint32_t getLeftMotorEncoderTicks();

void disableLeftMotor();

void enableRightMotor();

/*
 * power -> motor power in the range [-1000, 1000] (negative value means rotate backwards)
 */
void setRightMotorPower(int16_t power);

/*
 * encoder counter can overflow so make sure to read this value at least once per second
 *
 * this is a single threaded method (or not-interrupt safe) so use it from one 'thread' only
 */
uint32_t getRightMotorEncoderTicks();

void disableRightMotor();
