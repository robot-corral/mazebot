/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

#include "det_os.h"

/*
 * max number of scheduled tasks should be less than maximum value of scheduledTaskIndex_t type.
 * this is needed to allow for NULL parent.
 */
#define MAX_NUMBER_OF_SCHEDULED_TASKS 10

#define STACK_SIZE 512
#define REGISTERS_STORAGE_SIZE (8 * 4)
#define MAX_NUMBER_OF_IDLE_TASKS 2

#define GET_STACK_START_ADDRESS(stackIndex) ((stackIndex + 1) * STACK_SIZE)

typedef enum
{
    STS_EMPTY     = 0x00,
    STS_SCHEDULED = 0x01,
    STS_RUNNING   = 0x02,
    STS_IDLE      = 0x03,
} scheduledTaskStatus_t;

typedef uint8_t activeTaskIndex_t;
typedef uint8_t scheduledTaskIndex_t;

typedef struct
{
    task_t task;

    void* pTaskParameter;

    uint8_t status;
    uint8_t priority;

    activeTaskIndex_t activeTaskIndex;
    scheduledTaskIndex_t parentTaskIdx;
} scheduledTaskNode_t;
