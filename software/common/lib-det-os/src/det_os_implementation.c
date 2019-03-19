/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "det_os_implementation.h"

#include "schedule_tasks.h"

#include "global_data.h"

#include <stm32/stm32l4xx_ll_tim.h>

extern void SVC_Handler(void);
void(*SVC_Handler_fp)(void) = &SVC_Handler;

extern void TIM2_IRQHandler(void);
void(*TIM2_IRQHandler_fp)(void) = &TIM2_IRQHandler;

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

    executeTasks(g_detOsIdleStack + IDLE_TASK_STACK_SIZE);
}
