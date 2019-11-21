#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    D_FORWARD,
    D_BACKWARD,
} diraction_t;

void initializeMotorControl();

bool isBusy();

/*
 * returns false if direction is invalid or motor control is busy controlling the motor.
 */
bool generatePulses(diraction_t direction, uint32_t pulseCount);

void emergencyStop();
