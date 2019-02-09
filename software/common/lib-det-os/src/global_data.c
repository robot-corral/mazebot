/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

scheduledTaskIndex_t g_scheduledTasksTailIndex;

scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];

uint8_t g_activeTasksStack[(MAX_NUMBER_OF_SUSPENDED_TASKS + 1) * STACK_SIZE] __attribute__((aligned(8)));
uint8_t g_suspendedTasksRegisters[MAX_NUMBER_OF_SUSPENDED_TASKS * REGISTERS_STORAGE_SIZE];
