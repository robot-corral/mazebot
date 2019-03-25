#pragma once

#define R_NO_TASKS_LEFT                              1
#define R_PREVIOUS_TASK_CONTINUES_EXECUTION          1

// the following constants values must not be changed without
// revewing code in PendSV_Handler as code there depends on those values
// (specifically we don't want to do 4 comparisons, we do only 2)

#define R_SUSPEND_PREVIOUS_TASK_AND_START_NEXT_TASK  2
#define R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK 3
#define R_START_NEXT_TASK                            4
#define R_RESUME_NEXT_TASK                           5
