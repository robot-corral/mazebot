/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"
#include "schedule_tasks_defs.h"

extern void detOsIdle();

extern void executeTasks(volatile void* pStackStartAddress);

extern result_t scheduleTaskSvc(task_t task, taskPriority_t priority, void* pTaskParameter);

extern result_t scheduleSuspendedTaskSvc(task_t task, taskPriority_t priority, uint32_t suspendedDuration, void* pTaskParameter);

result_t finishCurrentAndMoveToNextTaskSvc(volatile void** ppOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3, volatile void** ppOutParam4);

result_t suspendCurrentAndMoveToNextTaskSvc(volatile void** ppOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3, volatile void** ppOutParam4);

result_t currentTaskYieldsAndMoveToNextTaskSvc(volatile void** ppInOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3, volatile void** ppOutParam4);

result_t delayCurrentAndMoveToNextTaskSvc(volatile void** ppInOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3, volatile void** ppOutParam4);
