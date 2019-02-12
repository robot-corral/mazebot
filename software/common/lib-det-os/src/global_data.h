/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

extern scheduledTaskIndex_t g_scheduledTasksRootIndex;

extern scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];

extern inProgressTaskStack_t g_inProgressTasksStacks[MAX_NUMBER_OF_SCHEDULED_TASKS];

extern uint8_t g_detOsIdleStack[IDLE_TASK_STACK_SIZE];
