#pragma once

#include "det_os.h"

void initializeOsHardware();

void taskSchedulerTimerCallback();

void runDetOs(task_t startTask);
