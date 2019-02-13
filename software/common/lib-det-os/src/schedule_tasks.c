/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "schedule_tasks.h"

#include "det_os.h"

#include "global_data.h"

result_t finishCurrentAndMoveToNextTaskSvc(void** ppOutParam1, void** ppOutParam2, void** ppOutParam3)
{
    const scheduledTaskIndex_t previouslyRunningTaskIndex = g_scheduledTasksRootIndex;

    if (previouslyRunningTaskIndex != NULL_SCHEDULED_TASK_INDEX)
    {
        scheduledTaskNode_t* pPreviouslyRunningTask = &g_scheduledTasks[previouslyRunningTaskIndex];

        if (pPreviouslyRunningTask->status == STS_RUNNING)
        {
            pPreviouslyRunningTask->status = STS_EMPTY;
            g_scheduledTasksRootIndex = pPreviouslyRunningTask->nextTaskIdx;
        }
    }

    const scheduledTaskIndex_t newRunningTaskIndex = g_scheduledTasksRootIndex;

    if (newRunningTaskIndex == NULL_SCHEDULED_TASK_INDEX)
    {
        *ppOutParam2 = detOsIdle;
        *ppOutParam3 = g_detOsIdleStack + IDLE_TASK_STACK_SIZE;
        return R_NO_TASKS_LEFT;
    }

    result_t result;
    scheduledTaskNode_t* pNewRunningTask = &g_scheduledTasks[newRunningTaskIndex];
    const scheduledTaskStatus_t oldStatus = pNewRunningTask->status;

    if (oldStatus == STS_SCHEDULED)
    {
        *ppOutParam1 = pNewRunningTask->pTaskParameter;
        *ppOutParam2 = pNewRunningTask->task;
        *ppOutParam3 = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, newRunningTaskIndex);
        result = R_NEW_TASK_FOUND;
    }
    else if (oldStatus == STS_SUSPENDED)
    {
        *ppOutParam1 = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, newRunningTaskIndex);
        *ppOutParam2 = pNewRunningTask->pRegisterStorage;
        result = R_SUSPENDED_TASK_FOUND;
    }
    else
    {
        return R_ERR_STATE_CORRUPTED;
    }

    pNewRunningTask->status = STS_RUNNING;

    return result;
}

result_t scheduleTaskSvc(task_t task, taskPriority_t priority, void* pTaskParameter)
{
    return R_OK; // TODO
}
