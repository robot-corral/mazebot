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
#define MAX_NUMBER_OF_SCHEDULED_TASKS 5

/*
 * as tasks data is statically allocated we need index which represents null scheduled task.
 */
#define NULL_SCHEDULED_TASK_INDEX (MAX_NUMBER_OF_SCHEDULED_TASKS + 1)

/*
 * task stack size (this includes 32 bytes required to store R0, R1, R2, R3, R12, LR, xPSR registers and ReturnAddress by ARM exception entry).
 */
#define TASK_STACK_SIZE 512

/*
 * stack to be used when there are no tasks to execute
 */
#define IDLE_TASK_STACK_SIZE (8 * 4)

/*
 * R4, R5, R6, R7, R8, R9, R10, R11, PSP
 * (the rest are saved to the stack by exception handler)
 */
#define TASK_REGISTERS_STORAGE_SIZE (9 * 4)

typedef enum
{
    STS_EMPTY     = 0x00,
    STS_SCHEDULED = 0x01,
    STS_RUNNING   = 0x02,
    STS_SUSPENDED = 0x03,
} scheduledTaskStatus_t;

typedef uint8_t scheduledTaskIndex_t;
typedef uint8_t inProgressTaskIndex_t;

typedef struct __attribute__((packed))
{
    task_t task;

    void* pTaskParameter;

    taskPriority_t priority;

    scheduledTaskStatus_t status;

    scheduledTaskIndex_t nextTaskIdx;

    /*
     * pRegisterStorage must be 4 byte aligned so we add this dummy field
     */
    uint8_t reserved;

    uint8_t pRegisterStorage[TASK_REGISTERS_STORAGE_SIZE];
} scheduledTaskNode_t;

/*
 * stack grows backwards and start address is never used, so need to point to next word after the end of stack memory
 */
#define GET_TASK_STACK_START_ADDRESS(inProgressTasksStacks, inProgressTaskStackIndex) (inProgressTasksStacks)[(inProgressTaskStackIndex) + 1].pStack

/*
 * stacks for all the scheduled tasks (they have to be separate from other data as stack start address should be 8 byte algigned to avoid wasting memory.
 */
typedef struct 
{
    uint8_t pStack[TASK_STACK_SIZE];
} inProgressTaskStack_t;
