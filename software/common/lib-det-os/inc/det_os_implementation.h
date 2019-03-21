/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "det_os.h"

#include <stm32l496xx.h>

#define DET_OS_SVC_INTERRUT_PRIORITY 15

void runDetOs(task_t startTask);

#define runDetOsTaskScheduler() \
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

// we need these 2 declarations as linker otherwise ignores
// the following det-os functions:
//  * SVC_Handler
//  * PendSV_Handler

extern void(*SVC_Handler_fp)(void);
extern void(*PendSV_Handler_fp)(void);
