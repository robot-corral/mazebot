/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdbool.h>

typedef void (*taskFunction_t)();

bool hasScheduledTasks();

bool scheduleTask(taskFunction_t task);

void runTasks();
