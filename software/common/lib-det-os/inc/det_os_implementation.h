/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "det_os.h"

void runDetOs(task_t startTask);

// we need these 2 declarations as linker otherwise ignores
// the following det-os functions:
//  * SVC_Handler
//  * TIM2_IRQHandler

extern void(*SVC_Handler_fp)(void);
extern void(*TIM2_IRQHandler_fp)(void);
