#include "schedule_task.h"

#include "det_os_implementation.h"

void systemCallHandler(osCallParameters_t* pOsCallParameters, uint8_t svcCode)
{
    switch (svcCode)
    {
        case SVCC_SCHEDULE_TASK:
        {
            pOsCallParameters->result = osScheduleTask(pOsCallParameters->task, pOsCallParameters->priority, pOsCallParameters->pTaskParameter);
            break;
        }
    }
}

void initializeDetOs()
{
}

void runDetOs(task_t startTask)
{
    initializeDetOs();
    initializeOsHardware();
}
