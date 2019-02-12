/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "det_os_implementation.h"

#include "schedule_tasks.h"

#include "global_data.h"

void runDetOs(task_t startTask)
{
    g_scheduledTasks[0].pTaskParameter = 0;
    g_scheduledTasks[0].nextTaskIdx = NULL_SCHEDULED_TASK_INDEX;
    g_scheduledTasks[0].priority = TP_NORMAL_PRIORITY;
    g_scheduledTasks[0].status = STS_SCHEDULED;
    g_scheduledTasks[0].task = startTask;

    g_scheduledTasksRootIndex = 0;

    startDetOsTimer();
    executeTasks(GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, 0));
}
