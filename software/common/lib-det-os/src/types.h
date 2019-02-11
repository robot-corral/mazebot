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

/*
 * as tasks data is statically allocated we need index which represents null scheduled task.
 */
#define NULL_SCHEDULED_TASK_INDEX (MAX_NUMBER_OF_SCHEDULED_TASKS + 1)

/*
 * task stack size (this includes 32 bytes required to store R0, R1, R2, R3, R12, LR, xPSR registers and ReturnAddress by ARM exception entry).
 */
#define TASK_STACK_SIZE 512

/*
 * R4, R5, R6, R7, R8, R9, R10, R11, SP, PC
 * (the rest are saved to the stack by exception handler)
 */
#define TASK_REGISTERS_STORAGE_SIZE (10 * 4)

/*
 * maximum number of tasks which can be in progress (one is running and the rest are suspended).
 */
#define MAX_NUMBER_OF_IN_ROGRESS_TASKS 2

typedef enum
{
    STS_EMPTY     = 0x00,
    STS_SCHEDULED = 0x01,
    STS_RUNNING   = 0x02,
    STS_SUSPENDED = 0x03,
} scheduledTaskStatus_t;

typedef uint8_t scheduledTaskIndex_t;
typedef uint8_t inProgressTaskIndex_t;

typedef struct
{
    task_t task;

    void* pTaskParameter;

    uint8_t status;
    uint8_t priority;

    scheduledTaskIndex_t parentTaskIdx;
    inProgressTaskIndex_t inProgressTaskIndex;
} scheduledTaskNode_t;

/*
 * stack grows backwards and start address is never used, so need to point to next word after the end of stack memory
 */
#define GET_TASK_STACK_START_ADDRESS(inProgressTaskData) (inProgressTaskData).savedRegisters

typedef struct __attribute__((packed))
{
    uint8_t stack[TASK_STACK_SIZE];
    uint8_t savedRegisters[TASK_REGISTERS_STORAGE_SIZE];
} inProgressTaskData_t;
