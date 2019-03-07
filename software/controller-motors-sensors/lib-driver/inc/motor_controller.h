/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

void enableLeftMotor();

/*
 * power -> motor power in the range [-MAX_MOTOR_POWER, MAX_MOTOR_POWER] (negative value means rotate backwards)
 */
void setLeftMotorPower(int16_t power);

uint32_t getLeftMotorEncoderTicks();

void disableLeftMotor();

void enableRightMotor();

/*
 * power -> motor power in the range [-MAX_MOTOR_POWER, MAX_MOTOR_POWER] (negative value means rotate backwards)
 */
void setRightMotorPower(int16_t power);

uint32_t getRightMotorEncoderTicks();

void disableRightMotor();
