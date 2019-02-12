/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

#define R_NO_TASKS_LEFT        1
#define R_NEW_TASK_FOUND       2
#define R_SUSPENDED_TASK_FOUND 3

extern void detOsIdle();

extern void executeTasks(void* pStackStartAddress);

extern void starkNewTask(void* pTaskParameter, task_t task, void* pStackStartAddress);

extern void resumeTask(void* pStackStartAddress, void* pRegisterStorage);

result_t moveToNextTaskSvc(scheduledTaskStatus_t oldTaskStatus, void** ppOutParam1, void** ppOutParam2, void** ppOutParam3);
