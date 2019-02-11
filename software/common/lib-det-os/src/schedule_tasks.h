/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

extern void executeTasks(void* pStackStartAddress);

extern void starkNewTask(void* pTaskParameter, task_t task, void* pStackStartAddress);
