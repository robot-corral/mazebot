/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

extern volatile scheduledTasksPointers_t g_scheduledTaskPointers;

extern volatile scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];

extern volatile inProgressTaskStack_t g_inProgressTasksStacks[MAX_NUMBER_OF_SCHEDULED_TASKS];

extern volatile uint8_t g_detOsIdleStack[IDLE_TASK_STACK_SIZE];
