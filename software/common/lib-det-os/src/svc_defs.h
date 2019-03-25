#pragma once

// svc calls which immediately affect tasks execution order

#define SVC_FINISH_CURRENT_AND_MOVE_TO_NEXT_TASK 0
#define SVC_YIELD                                1
#define SVC_SUSPEND                              2

// svc calls which do not immediately affect tasks execution order

#define SVC_SCHEDULE_TASK                        3
#define SVC_SCHEDULE_SUSPENDED_TASK              4
