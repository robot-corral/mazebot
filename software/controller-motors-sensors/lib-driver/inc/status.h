/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdbool.h>

typedef enum
{
    ERROR_MOTOR = 0x01,
    ERROR_DMA   = 0x02,
} Error_t;

typedef enum
{
    CS_OFF               = 0x0,
    CS_CALIBRATING_WHITE = 0x2,
    CS_CALIBRATING_BLACK = 0x3,
} CalibrationStatus_t;

typedef enum
{
    RS_OFF     = 0x0,
    RS_PAUSED  = 0x1,
    RS_RUNNING = 0x2,
} RunningStatus_t;

void setCriticalError(Error_t error);

void setCalibrationStatus(CalibrationStatus_t calibrationStatus);

void setRunningStatus(RunningStatus_t runningStatus);
