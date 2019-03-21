/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

volatile scheduledTasksPointers_t g_scheduledTaskPointers __attribute__((aligned(4)));

volatile scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS] __attribute__((aligned(4)));

volatile inProgressTaskStack_t g_inProgressTasksStacks[MAX_NUMBER_OF_SCHEDULED_TASKS] __attribute__((aligned(8)));

volatile uint8_t g_detOsIdleStack[IDLE_TASK_STACK_SIZE] __attribute__((aligned(8)));
