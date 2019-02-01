/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

extern volatile uint16_t g_firstUsedScheduledTaskNodeIndex;
extern volatile uint16_t g_firstAvailableScheduledTaskNodeIndex;
extern volatile scheduledTaskNode_t g_scheduledTasks[MAX_SCHEDULED_TASKS];
