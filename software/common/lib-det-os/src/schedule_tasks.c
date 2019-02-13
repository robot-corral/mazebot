/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "schedule_tasks.h"

#include "det_os.h"

#include "global_data.h"

static inline result_t executeRootScheduledTask(void** ppOutParam1, void** ppOutParam2, void** ppOutParam3)
{
    const scheduledTaskIndex_t newRunningTaskIndex = g_scheduledTaskRootIndex;

    if (newRunningTaskIndex == NULL_SCHEDULED_TASK_INDEX)
    {
        *ppOutParam2 = detOsIdle;
        *ppOutParam3 = g_detOsIdleStack + IDLE_TASK_STACK_SIZE;
        g_currentlyRunningTaskIndex = NULL_SCHEDULED_TASK_INDEX;
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
        result = R_START_NEW_TASK;
    }
    else if (oldStatus == STS_SUSPENDED)
    {
        *ppOutParam1 = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, newRunningTaskIndex);
        *ppOutParam2 = pNewRunningTask->pRegisterStorage;
        result = R_RESUME_NEW_TASK;
    }
    else
    {
        return R_ERR_STATE_CORRUPTED;
    }

    pNewRunningTask->status = STS_RUNNING;
    g_currentlyRunningTaskIndex = newRunningTaskIndex;
    // we always execute root task which doesn't have a parent
    g_currentlyRunningParentTaskIndex = NULL_SCHEDULED_TASK_INDEX;

    return result;
}

static inline void removeScheduledTask(scheduledTaskIndex_t taskIndex)
{
    scheduledTaskNode_t* pTaskToRemove = &g_scheduledTasks[taskIndex];

    pTaskToRemove->status = STS_EMPTY;

    if (taskIndex == g_scheduledTaskRootIndex)
    {
        g_scheduledTaskRootIndex = pTaskToRemove->nextTaskIdx;
    }
    else
    {
        g_scheduledTasks[g_currentlyRunningParentTaskIndex].nextTaskIdx = pTaskToRemove->nextTaskIdx;
    }
}

static inline bool moveTaskToTheEndOfTheGroupOfTasksWithTheSamePriority(scheduledTaskIndex_t taskIndex)
{
    scheduledTaskIndex_t indexOfLastScheduledTaskWithTheSamePriority = taskIndex;
    const taskPriority_t runningTaskPriority = g_scheduledTasks[taskIndex].priority;

    while (g_scheduledTasks[indexOfLastScheduledTaskWithTheSamePriority].nextTaskIdx != NULL_SCHEDULED_TASK_INDEX &&
           g_scheduledTasks[g_scheduledTasks[indexOfLastScheduledTaskWithTheSamePriority].nextTaskIdx].priority == runningTaskPriority)
    {
        indexOfLastScheduledTaskWithTheSamePriority = g_scheduledTasks[indexOfLastScheduledTaskWithTheSamePriority].nextTaskIdx;
    }

    const bool oldScheduledTaskIsTheOnlyScheduledTask = indexOfLastScheduledTaskWithTheSamePriority == NULL_SCHEDULED_TASK_INDEX;
    const bool oldScheduledTaskIsTheOnlyTaskWithItsPriority = indexOfLastScheduledTaskWithTheSamePriority == taskIndex;

    if (oldScheduledTaskIsTheOnlyScheduledTask || oldScheduledTaskIsTheOnlyTaskWithItsPriority)
    {
        return false;
    }
    else
    {
        if (taskIndex == g_scheduledTaskRootIndex)
        {
            g_scheduledTaskRootIndex = g_scheduledTasks[taskIndex].nextTaskIdx;
            g_scheduledTasks[taskIndex].nextTaskIdx = g_scheduledTasks[indexOfLastScheduledTaskWithTheSamePriority].nextTaskIdx;
            g_scheduledTasks[indexOfLastScheduledTaskWithTheSamePriority].nextTaskIdx = taskIndex;
        }
        else
        {
            g_scheduledTasks[g_currentlyRunningParentTaskIndex].nextTaskIdx = g_scheduledTasks[taskIndex].nextTaskIdx;
            g_scheduledTasks[taskIndex].nextTaskIdx = g_scheduledTasks[indexOfLastScheduledTaskWithTheSamePriority].nextTaskIdx;
            g_scheduledTasks[indexOfLastScheduledTaskWithTheSamePriority].nextTaskIdx = taskIndex;
        }

        return true;
    }
}

static inline result_t suspendOldTaskExecuteNewTask(scheduledTaskIndex_t oldRunningTaskIndex,
                                                    scheduledTaskIndex_t newRunningTaskIndex,
                                                    void** ppOutParam1,
                                                    void** ppOutParam2,
                                                    void** ppOutParam3,
                                                    void** ppOutParam4)
{
    result_t result;

    scheduledTaskNode_t* pOldRunningTask = &g_scheduledTasks[oldRunningTaskIndex];
    scheduledTaskNode_t* pNewRunningTask = &g_scheduledTasks[newRunningTaskIndex];

    const scheduledTaskStatus_t oldStatus = pNewRunningTask->status;

    if (oldStatus == STS_SCHEDULED)
    {
        *ppOutParam1 = pOldRunningTask->pRegisterStorage;
        *ppOutParam2 = pNewRunningTask->pTaskParameter;
        *ppOutParam3 = pNewRunningTask->task;
        *ppOutParam4 = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, newRunningTaskIndex);
        result = R_SUSPEND_OLD_TASK_START_NEW_TASK;
    }
    else if (oldStatus == STS_SUSPENDED)
    {
        *ppOutParam1 = pOldRunningTask->pRegisterStorage;
        *ppOutParam2 = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, newRunningTaskIndex);
        *ppOutParam3 = pNewRunningTask->pRegisterStorage;
        result = R_SUSPEND_OLD_TASK_RESUME_NEW_TASK;
    }
    else
    {
        return R_ERR_STATE_CORRUPTED;
    }

    pNewRunningTask->status = STS_RUNNING;
    pOldRunningTask->status = STS_SUSPENDED;
    g_currentlyRunningTaskIndex = newRunningTaskIndex;
    // we always execute root task which doesn't have a parent
    g_currentlyRunningParentTaskIndex = NULL_SCHEDULED_TASK_INDEX;

    return result;
}

result_t finishCurrentAndMoveToNextTaskSvc(void** ppOutParam1, void** ppOutParam2, void** ppOutParam3)
{
    // this function can be called if:
    //   * no tasks are running at all (not even idle)
    //      possible outcomes:
    //          - start executing root task (heppens when OS is 1st started)
    //   * there was a scheduled task running which just finished execution (idle task can never finish execution)
    //      possible outcomes:
    //          - if there are no scheduled tasks left then execute idle task
    //          - if there is a root task and it never executed then start executing it
    //          - if there is a root task and it was suspended then resume it

    const scheduledTaskIndex_t oldRunningTaskIndex = g_currentlyRunningTaskIndex;

    if (oldRunningTaskIndex != NULL_SCHEDULED_TASK_INDEX)
    {
        removeScheduledTask(oldRunningTaskIndex);
    }

    return executeRootScheduledTask(ppOutParam1, ppOutParam2, ppOutParam3);
}

result_t suspendCurrentAndMoveToNextTaskSvc(void** ppOutParam1, void** ppOutParam2, void** ppOutParam3, void** ppOutParam4)
{
    // this function can be called if:
    //   * idle task is running
    //      possible outcomes:
    //          - if there are no scheduled tasks left then execute idle task
    //          - if there is a root task and it never executed then start executing it
    //          - if there is a root task and it was suspended then resume it
    //   * root scheduled task is running
    //      possible outcomes:
    //          - there are no other tasks so current task continues execution
    //          - there are no tasks with priority which is higher or equal to current task then current task continues execution
    //          - there is a task with higher priority than the currently executing task then suspend current task and start executing task with the highest priority
    //          - there is at least one more task with the same priority then suspend current task and start executing next task with the same priority

    const scheduledTaskIndex_t scheduledTaskRootIndex = g_scheduledTaskRootIndex;
    const scheduledTaskIndex_t oldRunningTaskIndex = g_currentlyRunningTaskIndex;

    if (oldRunningTaskIndex == NULL_SCHEDULED_TASK_INDEX)
    {
        if (scheduledTaskRootIndex == NULL_SCHEDULED_TASK_INDEX)
        {
            return R_OLD_TASK_CONTINUES_EXECUTION;
        }
        else
        {
            return executeRootScheduledTask(ppOutParam1, ppOutParam2, ppOutParam3);
        }
    }
    else
    {
        if (scheduledTaskRootIndex != oldRunningTaskIndex)
        {
            moveTaskToTheEndOfTheGroupOfTasksWithTheSamePriority(oldRunningTaskIndex);
            return suspendOldTaskExecuteNewTask(oldRunningTaskIndex, scheduledTaskRootIndex, ppOutParam1, ppOutParam2, ppOutParam3, ppOutParam4);
        }
        else
        {
            if (moveTaskToTheEndOfTheGroupOfTasksWithTheSamePriority(oldRunningTaskIndex))
            {
                return suspendOldTaskExecuteNewTask(oldRunningTaskIndex, g_scheduledTaskRootIndex, ppOutParam1, ppOutParam2, ppOutParam3, ppOutParam4);
            }
            else
            {
                return R_OLD_TASK_CONTINUES_EXECUTION;
            }
        }
    }
}

result_t scheduleTaskSvc(task_t task, taskPriority_t priority, void* pTaskParameter)
{
    return R_OK; // TODO
}
