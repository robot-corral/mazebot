/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"
#include "schedule_tasks_defs.h"

extern void detOsIdle();

extern void executeTasks(volatile void* pStackStartAddress);

extern void starkNewTask(void* pTaskParameter, task_t task, void* pStackStartAddress);

result_t finishCurrentAndMoveToNextTaskSvc(volatile void** ppOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3);

result_t suspendCurrentAndMoveToNextTaskSvc(volatile void** ppOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3, volatile void** ppOutParam4);
