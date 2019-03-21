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
    for (uint32_t i = 0; i < MAX_NUMBER_OF_SCHEDULED_TASKS; ++i)
    {
        g_scheduledTasks[i].pTaskStackStartAddress = GET_TASK_STACK_START_ADDRESS(g_inProgressTasksStacks, i);
    }

    g_scheduledTasks[0].pTaskParameter = 0;
    g_scheduledTasks[0].pNextTask = nullptr;
    g_scheduledTasks[0].priority = TP_NORMAL_PRIORITY;
    g_scheduledTasks[0].status = STS_SCHEDULED;
    g_scheduledTasks[0].task = startTask;

    g_scheduledTaskPointers.pRootTask = &g_scheduledTasks[0];
    g_scheduledTaskPointers.pLastTaskInRootGroup = &g_scheduledTasks[0];
    g_scheduledTaskPointers.pLastTaskInCurrentlyRunningGroup = &g_scheduledTasks[0];
    g_scheduledTaskPointers.pCurrentlyRunningTask = nullptr;
    g_scheduledTaskPointers.pCurrentlyRunningParentTask = nullptr;

    LL_TIM_EnableCounter(TIM2); // start OS timer

    executeTasks(g_detOsIdleStack + IDLE_TASK_STACK_SIZE);
}
