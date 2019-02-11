/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

extern scheduledTaskIndex_t g_runningTaskIndex;
extern scheduledTaskIndex_t g_scheduledTasksTailIndex;

extern scheduledTaskNode_t g_scheduledTasks[MAX_NUMBER_OF_SCHEDULED_TASKS];
extern inProgressTaskData_t g_inProgressTaskData[MAX_NUMBER_OF_IN_ROGRESS_TASKS];
