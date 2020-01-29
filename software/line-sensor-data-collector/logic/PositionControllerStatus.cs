using System;

namespace line_sensor.data_collector.logic
{
    [Flags]
    public enum PositionControllerStatus : ushort
    {
        OK = 0x0000,

        /*
         * these are errors experienced by position controller
         * e.g.
         */

        PC_OK_BUSY           = 0x0001,
        PC_OK_EMERGENCY_STOP = 0x0002,

        PC_ERR_COMMUNICATION   = 0x0004,
        PC_ERR_UNKNOWN_COMMAND = 0x0008,
        PC_ERR_CRC             = 0x0010,

        /*
         * these are errors experienced by c# application
         */

        CS_ERR_DEVICE_NOT_CONNTECTED = 0x0400,
        CS_ERR_COMMUNICATION         = 0x0800,
        CS_ERR_CRC                   = 0x1000,
        CS_ERR_COMMUNICATION_TIMEOUT = 0x2000,
        CS_ERR_INVALID_ARGUMENT      = 0x4000,
        CS_ERR_UNEXPECTED            = 0x8000,
    }
}
