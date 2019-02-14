/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "result_codes.h"

typedef int32_t result_t;

typedef enum
{
    TP_LOWEST_PRIORITY  = 0x00,
    TP_NORMAL_PRIORITY  = 0x7F,
    TP_HIGHEST_PRIORITY = 0xFF,
} taskPriority_t;

typedef void (*task_t)(void* pTaskParameter);

extern result_t scheduleTask(task_t task, taskPriority_t priority, void* pTaskParameter);
