/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

scheduledTaskIndex_t g_runningTaskIndex;
scheduledTaskIndex_t g_scheduledTasksTailIndex;

scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];

inProgressTaskData_t g_inProgressTaskData[MAX_NUMBER_OF_IN_ROGRESS_TASKS] __attribute__((aligned(8)));
