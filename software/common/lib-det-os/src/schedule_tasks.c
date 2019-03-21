/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "schedule_tasks.h"

#include "det_os.h"

#include "global_data.h"

static inline result_t executeRootScheduledTask(volatile void** ppOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3)
{
    if (g_scheduledTaskPointers.pRootTask == nullptr)
    {
        *ppOutParam2 = detOsIdle;
        *ppOutParam3 = g_detOsIdleStack + IDLE_TASK_STACK_SIZE;
        g_scheduledTaskPointers.pCurrentlyRunningTask = nullptr;
        g_scheduledTaskPointers.pCurrentlyRunningParentTask = nullptr;
        g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = nullptr;
        return R_NO_TASKS_LEFT;
    }

    result_t result;
    const scheduledTaskStatus_t oldStatus = g_scheduledTaskPointers.pRootTask->status;

    if (oldStatus == STS_SCHEDULED)
    {
        *ppOutParam1 = g_scheduledTaskPointers.pRootTask->pTaskParameter;
        *ppOutParam2 = g_scheduledTaskPointers.pRootTask->task;
        *ppOutParam3 = g_scheduledTaskPointers.pRootTask->pTaskStackStartAddress;
        result = R_START_NEW_TASK;
    }
    else if (oldStatus == STS_SUSPENDED)
    {
        *ppOutParam1 = g_scheduledTaskPointers.pRootTask->pRegisterStorage;
        result = R_RESUME_TASK;
    }
    else
    {
        return R_ERR_STATE_CORRUPTED;
    }

    g_scheduledTaskPointers.pRootTask->status = STS_RUNNING;

    g_scheduledTaskPointers.pCurrentlyRunningParentTask = nullptr;
    g_scheduledTaskPointers.pCurrentlyRunningTask = g_scheduledTaskPointers.pRootTask;
    g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = g_scheduledTaskPointers.pLastTaskInRootGroup;

    return result;
}

static inline bool moveCurrentlyRunningTaskToTheEndOfTheGroupOfTasksWithTheSamePriority()
{
    if (g_scheduledTaskPointers.pCurrentlyRunningTask == g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup)
    {
        return false;
    }

    if (g_scheduledTaskPointers.pCurrentlyRunningParentTask)
    {
        g_scheduledTaskPointers.pCurrentlyRunningParentTask->pNextTask = g_scheduledTaskPointers.pCurrentlyRunningTask->pNextTask;
    }
    else
    {
        g_scheduledTaskPointers.pRootTask = g_scheduledTaskPointers.pCurrentlyRunningTask->pNextTask;
    }

    g_scheduledTaskPointers.pCurrentlyRunningTask->pNextTask = g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup->pNextTask;
    g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup->pNextTask = g_scheduledTaskPointers.pCurrentlyRunningTask;

    if (g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup == g_scheduledTaskPointers.pLastTaskInRootGroup)
    {
        g_scheduledTaskPointers.pLastTaskInRootGroup = g_scheduledTaskPointers.pCurrentlyRunningTask;
        g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = g_scheduledTaskPointers.pCurrentlyRunningTask;
    }
    else
    {
        g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = g_scheduledTaskPointers.pCurrentlyRunningTask;
    }

    return true;
}

static inline result_t suspendCurrentlyRunningTaskAndExecuteRootTask(scheduledTaskNodePtr_t oldTask,
                                                                     volatile void** ppOutParam1,
                                                                     volatile void** ppOutParam2,
                                                                     volatile void** ppOutParam3,
                                                                     volatile void** ppOutParam4)
{
    result_t result;

    switch (g_scheduledTaskPointers.pRootTask->status)
    {
        case STS_SCHEDULED:
        {
            *ppOutParam1 = oldTask->pRegisterStorage;
            *ppOutParam2 = g_scheduledTaskPointers.pRootTask->pTaskParameter;
            *ppOutParam3 = g_scheduledTaskPointers.pRootTask->task;
            *ppOutParam4 = g_scheduledTaskPointers.pRootTask->pTaskStackStartAddress;
            result = R_SUSPEND_OLD_TASK_START_NEW_TASK;
            break;
        }
        case STS_SUSPENDED:
        {
            *ppOutParam1 = oldTask->pRegisterStorage;
            *ppOutParam2 = g_scheduledTaskPointers.pRootTask->pRegisterStorage;
            result = R_SUSPEND_OLD_TASK_RESUME_NEW_TASK;
            break;
        }
        default:
        {
            return R_ERR_STATE_CORRUPTED;
        }
    }

    oldTask->status = STS_SUSPENDED;
    g_scheduledTaskPointers.pRootTask->status = STS_RUNNING;

    g_scheduledTaskPointers.pCurrentlyRunningParentTask = nullptr;
    g_scheduledTaskPointers.pCurrentlyRunningTask = g_scheduledTaskPointers.pRootTask;
    g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = g_scheduledTaskPointers.pLastTaskInRootGroup;

    return result;
}

result_t finishCurrentAndMoveToNextTaskSvc(volatile void** ppOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3)
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

    if (g_scheduledTaskPointers.pCurrentlyRunningTask)
    {
        g_scheduledTaskPointers.pCurrentlyRunningTask->status = STS_EMPTY;

        if (g_scheduledTaskPointers.pCurrentlyRunningTask == g_scheduledTaskPointers.pRootTask)
        {
            g_scheduledTaskPointers.pRootTask = g_scheduledTaskPointers.pCurrentlyRunningTask->pNextTask;
        }
        else
        {
            g_scheduledTaskPointers.pCurrentlyRunningParentTask = g_scheduledTaskPointers.pCurrentlyRunningTask->pNextTask;
        }
    }

    return executeRootScheduledTask(ppOutParam1, ppOutParam2, ppOutParam3);
}

result_t suspendCurrentAndMoveToNextTaskSvc(volatile void** ppOutParam1, volatile void** ppOutParam2, volatile void** ppOutParam3, volatile void** ppOutParam4)
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

    if (g_scheduledTaskPointers.pCurrentlyRunningTask)
    {
        scheduledTaskNodePtr_t oldTask = g_scheduledTaskPointers.pCurrentlyRunningTask;

        if (g_scheduledTaskPointers.pCurrentlyRunningTask != g_scheduledTaskPointers.pRootTask)
        {
            moveCurrentlyRunningTaskToTheEndOfTheGroupOfTasksWithTheSamePriority();
            return suspendCurrentlyRunningTaskAndExecuteRootTask(oldTask, ppOutParam1, ppOutParam2, ppOutParam3, ppOutParam4);
        }
        else
        {
            if (moveCurrentlyRunningTaskToTheEndOfTheGroupOfTasksWithTheSamePriority())
            {
                return suspendCurrentlyRunningTaskAndExecuteRootTask(oldTask, ppOutParam1, ppOutParam2, ppOutParam3, ppOutParam4);
            }
            else
            {
                return R_OLD_TASK_CONTINUES_EXECUTION;
            }
        }
    }
    else
    {
        if (g_scheduledTaskPointers.pRootTask)
        {
            return executeRootScheduledTask(ppOutParam1, ppOutParam2, ppOutParam3);
        }
        else
        {
            return R_OLD_TASK_CONTINUES_EXECUTION;
        }
    }
}

result_t scheduleTaskSvc(task_t task, taskPriority_t priority, void* pTaskParameter)
{
    if (task == nullptr)
    {
        return R_ERR_INVALID_ARGUMENT;
    }

    for (uint32_t i = 0; i < MAX_NUMBER_OF_SCHEDULED_TASKS; ++i)
    {
        if (g_scheduledTasks[i].status == STS_EMPTY)
        {
            if (g_scheduledTaskPointers.pRootTask)
            {
                scheduledTaskNodePtr_t pNewTask = &g_scheduledTasks[i];

                pNewTask->task = task;
                pNewTask->priority = priority;
                pNewTask->status = STS_SCHEDULED;
                pNewTask->pTaskParameter = pTaskParameter;

                if (priority > g_scheduledTaskPointers.pRootTask->priority)
                {
                    pNewTask->pNextTask = g_scheduledTaskPointers.pRootTask;

                    if (g_scheduledTaskPointers.pRootTask == g_scheduledTaskPointers.pLastTaskInRootGroup)
                    {
                        g_scheduledTaskPointers.pLastTaskInRootGroup = pNewTask;
                    }

                    g_scheduledTaskPointers.pRootTask = pNewTask;
                }
                else
                {
                    scheduledTaskNodePtr_t pLastTaskWithHigherOrEqualPriority = g_scheduledTaskPointers.pRootTask;

                    while (pLastTaskWithHigherOrEqualPriority->pNextTask && pLastTaskWithHigherOrEqualPriority->pNextTask->priority >= priority)
                    {
                        pLastTaskWithHigherOrEqualPriority = pLastTaskWithHigherOrEqualPriority->pNextTask;
                    }

                    pNewTask->pNextTask = pLastTaskWithHigherOrEqualPriority->pNextTask;
                    pLastTaskWithHigherOrEqualPriority->pNextTask = pNewTask;

                    if (pLastTaskWithHigherOrEqualPriority->priority == pNewTask->priority)
                    {
                        if (pLastTaskWithHigherOrEqualPriority == g_scheduledTaskPointers.pLastTaskInRootGroup)
                        {
                            g_scheduledTaskPointers.pLastTaskInRootGroup = pNewTask;
                        }
                        if (pLastTaskWithHigherOrEqualPriority == g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup)
                        {
                            g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = pNewTask;
                        }
                    }
                }
            }
            else
            {
                g_scheduledTaskPointers.pRootTask = &g_scheduledTasks[i];
                g_scheduledTaskPointers.pLastTaskInRootGroup = g_scheduledTaskPointers.pRootTask;
                g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = g_scheduledTaskPointers.pRootTask;

                g_scheduledTaskPointers.pRootTask->task = task;
                g_scheduledTaskPointers.pRootTask->pNextTask = nullptr;
                g_scheduledTaskPointers.pRootTask->priority = priority;
                g_scheduledTaskPointers.pRootTask->status = STS_SCHEDULED;
                g_scheduledTaskPointers.pRootTask->pTaskParameter = pTaskParameter;
            }

            return R_OK;
        }
    }

    return R_ERR_STORAGE_FULL;
}
