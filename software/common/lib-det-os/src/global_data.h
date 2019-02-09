/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

extern scheduledTaskIndex_t g_scheduledTasksTailIndex;

extern scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];

extern uint8_t g_activeTasksStack[(MAX_NUMBER_OF_SUSPENDED_TASKS + 1) * STACK_SIZE];
extern uint8_t g_suspendedTasksRegisters[MAX_NUMBER_OF_SUSPENDED_TASKS * REGISTERS_STORAGE_SIZE];
