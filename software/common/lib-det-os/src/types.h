/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "det_os.h"

#include <stdint.h>

typedef enum
{
    SVCC_SCHEDULE_TASK = 0,
} svcCode;

typedef struct
{
    task_t pTask;
    void* pTaskParameter;
    uint8_t taskPriority;
    uint8_t stateMask;
    uint16_t nextNodeIndex;
} scheduledTaskNode_t;

typedef struct
{
    task_t task;
    void* pTaskParameter;
    uint8_t priority;
    result_t result;
} osCallParameters_t;
