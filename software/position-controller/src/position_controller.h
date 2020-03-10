#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_PULSES_AT_CONSTANT_SPEED 5

typedef enum
{
    PCD_NONE     = 0,
    PCD_FORWARD  = 1,
    PCD_BACKWARD = 2,
} positionControllerDirection_t;

typedef enum
{
    PCS_RESET             = 0,
    PCS_IDLE              = 1,
    PCS_EMERGENCY_STOPPED = 2,

    PCS_BUSY              = 0x80, // make sure this doesn't intersect with states below

    PCS_BUSY_ACCELERATING                              = 3 | PCS_BUSY,
    PCS_BUSY_ACCELERATING_AND_MOVING_AT_CONSTANT_SPEED = 4 | PCS_BUSY,
    PCS_BUSY_SLOWING_DOWN                              = 5 | PCS_BUSY,
    PCS_BUSY_MOVING_AT_CONSTANT_SPEED                  = 6 | PCS_BUSY,
} positionControllerState_t;

typedef enum
{
    PCCS_NONE                    = 0,
    PCCS_CALIBRATING_MIN         = 1,
    PCCS_CALIBRATING_MAX         = 2,
    PCCS_CALIBRATING_CORRECT_MAX = 3,
} positionControllerCalibratingState_t;

typedef enum
{
    PCLST_NONE = 0,
    PCLST_MIN  = 1,
    PCLST_MAX  = 2,
} positionControllerLimitStopType_t;

typedef enum
{
    MRR_OK                = 0,
    MRR_BUSY              = 1,
    MRR_INVALID_STATE     = 2,
    MRR_INVALID_PARAMETER = 3,
    MRR_UNEXPECTED_ERROR  = 4,
} moveRequestResult_t;

void initializePositionController();

uint32_t getPosition();

uint32_t getAbsolutePositionError();

positionControllerState_t getState();

/*
 * returns false if position control is busy controlling the motor.
 */
moveRequestResult_t calibratePositionController(positionControllerState_t* pState);

/*
 * returns false if direction is invalid or position control is busy controlling the motor.
 */
moveRequestResult_t setPosition(positionControllerDirection_t direction, uint32_t pulseCount, positionControllerState_t* pState);

/*
 * recalibration is required after emergency stop.
 */
void positionControllerEmergencyStop();

void positionControllerLimitStop(positionControllerLimitStopType_t limitStopType);
