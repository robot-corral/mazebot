/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "schedule_tasks.h"

#include "det_os.h"

#include "global_data.h"

result_t moveToNextTaskSvc(scheduledTaskStatus_t oldTaskStatus, void** ppOutParam1, void** ppOutParam2, void** ppOutParam3)
{
    if (oldTaskStatus == STS_EMPTY)
    {
        if (g_scheduledTasksRootIndex != NULL_SCHEDULED_TASK_INDEX && g_scheduledTasks[g_scheduledTasksRootIndex].status == STS_RUNNING)
        {
            g_scheduledTasks[g_scheduledTasksRootIndex].status = STS_EMPTY;
            g_scheduledTasksRootIndex = g_scheduledTasks[g_scheduledTasksRootIndex].nextTaskIdx;
        }
    }
    else
    {
        g_scheduledTasks[g_scheduledTasksRootIndex].status = STS_SUSPENDED;

        scheduledTaskIndex_t lastInGroupScheduledTaskIndex = g_scheduledTasksRootIndex;
        const taskPriority_t currentTaskPriority = g_scheduledTasks[g_scheduledTasksRootIndex].priority;

        while (g_scheduledTasks[lastInGroupScheduledTaskIndex].nextTaskIdx != NULL_SCHEDULED_TASK_INDEX &&
               g_scheduledTasks[g_scheduledTasks[lastInGroupScheduledTaskIndex].nextTaskIdx].priority == currentTaskPriority)
        {
            lastInGroupScheduledTaskIndex = g_scheduledTasks[lastInGroupScheduledTaskIndex].nextTaskIdx;
        }

        if (lastInGroupScheduledTaskIndex != g_scheduledTasksRootIndex)
        {
            const scheduledTaskIndex_t oldScheduledTasksRootIndex = g_scheduledTasksRootIndex;
            g_scheduledTasksRootIndex = g_scheduledTasks[oldScheduledTasksRootIndex].nextTaskIdx;
            g_scheduledTasks[oldScheduledTasksRootIndex].nextTaskIdx = g_scheduledTasks[lastInGroupScheduledTaskIndex].nextTaskIdx;
            g_scheduledTasks[lastInGroupScheduledTaskIndex].nextTaskIdx = oldScheduledTasksRootIndex;
        }
    }

    if (g_scheduledTasksRootIndex == NULL_SCHEDULED_TASK_INDEX)
    {
        *ppOutParam1 = 0;
        *ppOutParam2 = detOsIdle;
        *ppOutParam3 = g_detOsIdleStack + IDLE_TASK_STACK_SIZE;
        return R_NO_TASKS_LEFT;
    }

    result_t result;

    scheduledTaskNode_t* pCurrentScheduledTask = &g_scheduledTasks[g_scheduledTasksRootIndex];
    const scheduledTaskStatus_t oldStatus = pCurrentScheduledTask->status;

    if (oldStatus == STS_SCHEDULED)
    {
        *ppOutParam1 = pCurrentScheduledTask->pTaskParameter;
        *ppOutParam2 = pCurrentScheduledTask->task;
        *ppOutParam3 = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, g_scheduledTasksRootIndex);
        result = R_NEW_TASK_FOUND;
    }
    else if (oldStatus == STS_SUSPENDED)
    {
        *ppOutParam1 = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, g_scheduledTasksRootIndex);
        *ppOutParam2 = pCurrentScheduledTask->pRegisterStorage;
        result = R_SUSPENDED_TASK_FOUND;
    }
    else
    {
        return R_ERR_STATE_CORRUPTED;
    }

    g_scheduledTasks[g_scheduledTasksRootIndex].status = STS_RUNNING;

    return result;
}

result_t scheduleTaskSvc(task_t task, taskPriority_t priority, void* pTaskParameter)
{
    return R_OK; // TODO
}
