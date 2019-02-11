/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "det_os_implementation.h"

#include "schedule_tasks.h"

#include "global_data.h"

void runDetOs(task_t startTask)
{
    g_scheduledTasks[0].pTaskParameter = 0;
    g_scheduledTasks[0].inProgressTaskIndex = 0;
    g_scheduledTasks[0].parentTaskIdx = NULL_SCHEDULED_TASK_INDEX;
    g_scheduledTasks[0].priority = TP_NORMAL_PRIORITY;
    g_scheduledTasks[0].status = STS_SCHEDULED;
    g_scheduledTasks[0].task = startTask;

    g_scheduledTasksTailIndex = 0;
    g_runningTaskIndex = NULL_SCHEDULED_TASK_INDEX;

    executeTasks(GET_TASK_STACK_START_ADDRESS(g_inProgressTaskData[0]));
}
