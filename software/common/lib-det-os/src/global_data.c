/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

scheduledTaskIndex_t g_runningTasksIndex;
scheduledTaskIndex_t g_scheduledTasksRootIndex;

scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];

uint8_t g_activeTasksStack[(MAX_NUMBER_OF_IDLE_TASKS + 1) * STACK_SIZE];
uint8_t g_suspendedTasksRegisters[MAX_NUMBER_OF_IDLE_TASKS * REGISTERS_STORAGE_SIZE];
