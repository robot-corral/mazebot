#include "det_os.h"

#include "global_data.h"

result_t osScheduleTask(task_t task, uint8_t priority, void* pTaskParameter)
{
    return R_OK;
}

result_t scheduleTask(task_t task, uint8_t priority, void* pTaskParameter)
{
    volatile osCallParameters_t osCallParameters;
    osCallParameters.task = task;
    osCallParameters.priority = priority;
    osCallParameters.pTaskParameter = pTaskParameter;
    volatile osCallParameters_t* pOsCallParameters = &osCallParameters;
    asm("ldr r0, %0" : : "m" (pOsCallParameters) : "r0");
    asm("svc %0" : : "I" (SVCC_SCHEDULE_TASK));
    return osCallParameters.result;
}
