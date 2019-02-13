/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "det_os_implementation.h"

#include "schedule_tasks.h"

#include "global_data.h"

#include <stm32/stm32l4xx_ll_tim.h>

void runDetOs(task_t startTask)
{
    g_scheduledTasks[0].pTaskParameter = 0;
    g_scheduledTasks[0].nextTaskIdx = NULL_SCHEDULED_TASK_INDEX;
    g_scheduledTasks[0].priority = TP_NORMAL_PRIORITY;
    g_scheduledTasks[0].status = STS_SCHEDULED;
    g_scheduledTasks[0].task = startTask;

    g_scheduledTaskRootIndex = 0;
    g_currentlyRunningTaskIndex = NULL_SCHEDULED_TASK_INDEX;
    g_currentlyRunningParentTaskIndex = NULL_SCHEDULED_TASK_INDEX;

    LL_TIM_EnableCounter(TIM2); // start OS timer

    executeTasks(GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, 0));
}
