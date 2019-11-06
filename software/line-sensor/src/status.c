#include "status.h"

#include "global_data.h"

#include <stdatomic.h>

lineSensorDetailedStatus_t getDetailedSensorStatus()
{
    return atomic_load(&g_statusDetailedInternal);
}

lineSensorDetailedStatus_t getCumulitiveDetailedSensorStatus()
{
    return atomic_load(&g_statusCumulativeDetailedInternal);
}

lineSensorDetailedStatus_t setSensorStatusFlags(lineSensorDetailedStatus_t flags)
{
    atomic_fetch_or(&g_statusCumulativeDetailedInternal, flags);
    return atomic_fetch_or(&g_statusDetailedInternal, flags);
}

lineSensorDetailedStatus_t resetSensorStatusFlags(lineSensorDetailedStatus_t flags)
{
    return atomic_fetch_and(&g_statusDetailedInternal, ~flags);
}
