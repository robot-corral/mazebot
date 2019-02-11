/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "schedule_tasks.h"

#include "det_os.h"

#include "global_data.h"

scheduledTaskIndex_t findNextTaskSvc()
{
    return 0;
}

result_t markCurrentTaskAsCompletedAndStartNextTaskSvc()
{
    if (g_runningTaskIndex != NULL_SCHEDULED_TASK_INDEX)
    {
        g_scheduledTasks[g_runningTaskIndex].status = STS_EMPTY;
    }

    g_runningTaskIndex = findNextTaskSvc();

    // TODO need to use idle task if there is no task available at the moment

    scheduledTaskNode_t* pCurrentTask = &g_scheduledTasks[g_runningTaskIndex];

    pCurrentTask->status = STS_RUNNING;
    pCurrentTask->inProgressTaskIndex = 0; // TODO need to find available in progress task index

    starkNewTask(pCurrentTask->pTaskParameter, pCurrentTask->task, GET_TASK_STACK_START_ADDRESS(g_inProgressTaskData[pCurrentTask->inProgressTaskIndex]));

    return R_OK;
}

result_t scheduleTaskSvc(task_t task, uint8_t priority, void* pTaskParameter)
{
    return R_OK; // TODO
}
