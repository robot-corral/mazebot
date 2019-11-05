#include "status.h"

#include "global_data.h"

#include <stdatomic.h>

lineSensorStatus_t getSensorStatus()
{
    lineSensorStatus_t result = LSS_OK;
    lineSensorDetailedStatus_t detailedStatus = atomic_load(&g_statusDetailedInternal);
    if (detailedStatus & LSDS_ERR_FLAG_ALL)
    {
        result |= LSS_ERROR;
    }
    if (detailedStatus & LSDS_OK_FLAG_NEW_DATA_AVAILABLE)
    {
        result |= LSS_OK_FLAG_NEW_DATA_AVAILABLE;
    }
    return result;
}

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
