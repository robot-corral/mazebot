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
 * task stack size (this includes 32 bytes required to store R0, R1, R2, R3, R12, LR, xPSR registers and ReturnAddress by ARM exception entry).
 */
#define TASK_STACK_SIZE 256

/*
 * stack to be used when there are no tasks to execute
 */
#define IDLE_TASK_STACK_SIZE (8 * 4)

/*
 * R4, R5, R6, R7, R8, R9, R10, R11, PSP
 * (the rest are saved to the stack by exception handler)
 */
#define TASK_REGISTERS_STORAGE_SIZE (9 * 4)

#define STS_EMPTY     0x00
#define STS_SCHEDULED 0x01
#define STS_RUNNING   0x02
#define STS_SUSPENDED 0x03

#define nullptr ((void*) 0)

typedef uint8_t scheduledTaskStatus_t;

struct _scheduledTaskNode_t;

typedef volatile struct _scheduledTaskNode_t* scheduledTaskNodePtr_t;

typedef struct _scheduledTaskNode_t
{
    task_t task;

    void* pTaskParameter;

    scheduledTaskNodePtr_t pNextTask;

    void* pTaskStackStartAddress;

    taskPriority_t priority;

    scheduledTaskStatus_t status;

    /*
     * pRegisterStorage must be 4 byte aligned to allow multiple register store / load operations so we add this dummy field
     */
    uint16_t reserved;

    uint8_t pRegisterStorage[TASK_REGISTERS_STORAGE_SIZE];
} scheduledTaskNode_t;

typedef struct __attribute__((packed))
{
    scheduledTaskNodePtr_t pRootTask;
    scheduledTaskNodePtr_t pLastTaskInRootGroup;
    scheduledTaskNodePtr_t pCurrentlyRunningTask;
    scheduledTaskNodePtr_t pCurrentlyRunningParentTask;
    scheduledTaskNodePtr_t pLastTaskInCurrentlyRunningGroup;
    scheduledTaskNodePtr_t pRootDelayedTask;
} scheduledTasksPointers_t;

/*
 * stack grows backwards and start address is never used, so need to point to next word after the end of stack memory
 */
#define GET_TASK_STACK_START_ADDRESS(tasksStacks, stackIndex) ((uint8_t*) (&tasksStacks[stackIndex]) + TASK_STACK_SIZE)

/*
 * stacks for all the scheduled tasks (they have to be separate from other data as stack start address should be 8 byte algigned to avoid wasting memory.
 *
 * stacks are separate from other data to allow for memory protection to be added later.
 */
typedef struct 
{
    uint8_t pStack[TASK_STACK_SIZE];
} inProgressTaskStack_t;
