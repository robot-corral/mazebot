using System;

namespace line_sensor.data_collector.logic
{
    [Flags]
    public enum PositionControllerStatus : uint
    {
        PC_OK_RESET          = 0x0000,
        PC_OK_IDLE           = 0x0001,
        PC_OK_BUSY           = 0x0002,
        PC_OK_EMERGENCY_STOP = 0x0003,

        /*
         * to check for PC_OK values you 1st need to mask status with PC_OK_MASK and then compare
         * desired ok value with the result, e.g.:
         * if ((result.Status & PositionControllerStatus.PC_OK_MASK) == PositionControllerStatus.PC_OK_RESET) { ... }
         */
        PC_OK_MASK = 0x0003,

        /*
         * these are errors experienced by position controller
         * e.g.
         */

        PC_ERR_CRC                 = 0x0004,
        PC_ERR_BUSY                = 0x0008,
        PC_ERR_COMMUNICATION_ERROR = 0x0010,
        PC_ERR_INVALID_STATE       = 0x0020,
        PC_ERR_INVALID_PARAMETER   = 0x0040,
        PC_ERR_UNKNOWN_COMMAND     = 0x0080,
        PC_ERR_UNEXPECTED          = 0x0100,
        PC_ERR_POSITION_ERROR      = 0x0200,

        /*
         * these are errors experienced by c# application
         */

        CS_ERR_CALIBRATION_TIMEOUT       = 0x0100_0000,
        CS_ERR_GAVE_UP_EXECUTING_COMMAND = 0x0200_0000,
        CS_ERR_DEVICE_NOT_CONNTECTED     = 0x0400_0000,
        CS_ERR_COMMUNICATION             = 0x0800_0000,
        CS_ERR_CRC                       = 0x1000_0000,
        CS_ERR_COMMUNICATION_TIMEOUT     = 0x2000_0000,
        CS_ERR_INVALID_ARGUMENT          = 0x4000_0000,
        CS_ERR_UNEXPECTED                = 0x8000_0000,
    }
}
