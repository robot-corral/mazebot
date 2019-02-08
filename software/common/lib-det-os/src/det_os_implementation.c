/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "det_os_implementation.h"

#include "types.h"
#include "global_data.h"

extern void executeTask(void* pTaskParameter, task_t task, void* pStackStartAddress);

void runDetOs(task_t startTask)
{
    g_runningTasksIndex = 0;
    g_scheduledTasksRootIndex = 0;

    g_scheduledTasks[0].activeTaskIndex = 0;
    g_scheduledTasks[0].pTaskParameter = 0;
    g_scheduledTasks[0].parentTaskIdx = MAX_NUMBER_OF_SCHEDULED_TASKS + 1;
    g_scheduledTasks[0].priority = TP_HIGHEST_PRIORITY;
    g_scheduledTasks[0].status = STS_RUNNING;
    g_scheduledTasks[0].task = startTask;

    startDetOsTimer();

    executeTask(g_scheduledTasks[0].pTaskParameter,
                g_scheduledTasks[0].task,
                &g_activeTasksStack[GET_STACK_START_ADDRESS(g_scheduledTasks[0].activeTaskIndex)]);
}

void taskSchedulerTimerCallback()
{
    // TODO
}
