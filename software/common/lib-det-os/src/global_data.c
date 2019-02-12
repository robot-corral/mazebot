/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

scheduledTaskIndex_t g_scheduledTasksRootIndex;

scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];

inProgressTaskStack_t g_inProgressTasksStacks[MAX_NUMBER_OF_SCHEDULED_TASKS] __attribute__((aligned(8)));

uint8_t g_detOsIdleStack[IDLE_TASK_STACK_SIZE] __attribute__((aligned(8)));
