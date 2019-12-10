#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    PCD_NONE     = 0,
    PCD_FORWARD  = 1,
    PCD_BACKWARD = 2,
} positionControllerDirection_t;

typedef enum
{
    PCS_FREE              = 0,
    PCS_MOVING_FORWARD    = 1,
    PCS_MOVING_BACKWARD   = 2,
    PCS_CALIBRATING       = 3,
    PCS_EMERGENCY_STOPPED = 4,
} positionControllerStatus_t;

typedef enum
{
    PCLST_NONE = 0,
    PCLST_MIN  = 1,
    PCLST_MAX  = 2,
} positionControllerLimitStopType_t;

void initializePositionController();

bool isPositionControllerBusy();

uint32_t getPosition();

/*
 * returns false if position control is busy controlling the motor.
 */
bool calibratePositionController();

/*
 * returns false if direction is invalid or position control is busy controlling the motor.
 */
bool setPosition(positionControllerDirection_t direction, uint32_t pulseCount);

/*
 * recalibration is required after emergency stop.
 */
void positionControllerEmergencyStop();

void positionControllerLimitStop(positionControllerLimitStopType_t limitStopType);
