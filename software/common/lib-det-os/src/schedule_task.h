/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "types.h"

result_t osScheduleTask(task_t task, uint8_t priority, void* pTaskParameter);
