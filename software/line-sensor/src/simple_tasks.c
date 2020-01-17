/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "simple_tasks.h"
#include "global_data.h"

#include <stdatomic.h>

bool hasScheduledTasks()
{
    return atomic_load(&g_taskSchedulerCurrentTask) != 0;
}

bool scheduleTask(taskFunction_t task)
{
    taskFunction_t expected = 0;
    return atomic_compare_exchange_strong(&g_taskSchedulerCurrentTask, &expected, task);
}

void runTasks()
{
    for (;;)
    {
        const taskFunction_t current = atomic_load(&g_taskSchedulerCurrentTask);

        if (current != 0)
        {
            current();
            g_taskSchedulerCurrentTask = 0;
            atomic_store(&g_taskSchedulerCurrentTask, 0);
        }
    }
}
