/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "result_codes.h"

#define TP_LOWEST_PRIORITY  0x00
#define TP_NORMAL_PRIORITY  0x7F
#define TP_HIGHEST_PRIORITY 0xFF

typedef int32_t result_t;

typedef uint8_t taskPriority_t;

typedef void (*task_t)(void* pTaskParameter);

extern result_t scheduleTask(task_t task, taskPriority_t priority, void* pTaskParameter);

extern result_t scheduleSuspendedTask(task_t task, taskPriority_t priority, uint32_t suspendedDuration, void* pTaskParameter);

/*
 * suspend calling tasks for specified duration.
 */
extern result_t suspend(uint32_t suspendDuration);

/*
 * surrender remaining part of your time slice so other tasks can execute.
 */
extern result_t yield();
