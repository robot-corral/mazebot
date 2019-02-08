/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include "det_os.h"

void startDetOsTimer();

void taskSchedulerTimerCallback();

void runDetOs(task_t startTask);
