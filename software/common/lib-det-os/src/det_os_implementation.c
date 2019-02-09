/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "det_os_implementation.h"

#include "types.h"
#include "global_data.h"

extern void executeTask(void* pTaskParameter, task_t task, void* pStackStartAddress);

void runDetOs(task_t startTask)
{
    g_scheduledTasks[0].activeTaskIndex = 0;
    g_scheduledTasks[0].pTaskParameter = 0;
    g_scheduledTasks[0].parentTaskIdx = SCHEDULED_TASK_NULL_PARENT;
    g_scheduledTasks[0].priority = TP_NORMAL_PRIORITY;
    g_scheduledTasks[0].status = STS_RUNNING;
    g_scheduledTasks[0].task = startTask;

    g_scheduledTasksTailIndex = 0;

    startDetOsTimer();

    executeTask(g_scheduledTasks[0].pTaskParameter,
                g_scheduledTasks[0].task,
                &g_activeTasksStack[GET_STACK_START_ADDRESS(g_scheduledTasks[0].activeTaskIndex)]);
}
