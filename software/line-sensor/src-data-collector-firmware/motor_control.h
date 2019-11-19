#pragma once

#include <stdint.h>

typedef enum
{
    D_FORWARD,
    D_BACKWARD,
} diraction_t;

void initializeMotorControl();

void generatePulses(diraction_t direction, uint32_t pulseCount);
