using System;

namespace line_sensor.data_collector.logic
{
    [Flags]
    public enum LineSensorStatus : byte
    {
        LSS_ZERO                       = 0x00,
        LSS_OK_FLAG_DATA_AVAILABLE     = 0x01,
        LSS_OK_FLAG_NEW_DATA_AVAILABLE = 0x02,
        LSS_ERROR                      = 0x04,
    }
}
