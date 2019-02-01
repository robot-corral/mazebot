/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

volatile uint16_t g_firstUsedScheduledTaskNodeIndex = MAX_SCHEDULED_TASKS;

volatile uint16_t g_firstAvailableScheduledTaskNodeIndex = 0;

volatile scheduledTaskNode_t g_scheduledTasks[MAX_SCHEDULED_TASKS] = { 0 };
