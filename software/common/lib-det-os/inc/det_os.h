#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    R_OK = 0,
} result_t;

typedef enum
{
    TP_LOWEST_PRIORITY  = 0x00,
    TP_NORMAL_PRIORITY  = 0x7F,
    TP_HIGHEST_PRIORITY = 0xFF,
} taskPriority_t;

typedef void (*task_t)(void* pTaskParameter);

result_t scheduleTask(task_t task, uint8_t priority, void* pTaskParameter);
