/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"
#include "schedule_tasks_defs.h"

extern void detOsIdle();

extern void executeTasks(void* pStackStartAddress);

extern void starkNewTask(void* pTaskParameter, task_t task, void* pStackStartAddress);

extern void resumeTask(void* pStackStartAddress, void* pRegisterStorage);

result_t finishCurrentAndMoveToNextTaskSvc(void** ppOutParam1, void** ppOutParam2, void** ppOutParam3);

result_t suspendCurrentAndMoveToNextTaskSvc(void** ppOutParam1, void** ppOutParam2, void** ppOutParam3, void** ppOutParam4);
