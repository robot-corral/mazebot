namespace line_sensor.data_collector.logic
{
    public enum PositionControllerCommand : byte
    {
        NONE           = 0,
        CALIBRATE      = 1,
        EMERGENCY_STOP = 2,
        MOVE_IF_IDLE   = 3,
        GET_STATUS     = 4,
        RESET          = 5,
    }
}
