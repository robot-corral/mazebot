namespace line_sensor.data_collector.logic
{
    public class PositionControllerResponse
    {
        public PositionControllerResponse(PositionControllerStatus status)
            : this(status, 0)
        {
        }

        public PositionControllerResponse(PositionControllerStatus status, uint position)
        {
            Status = status;
            Position = position;
        }

        public PositionControllerStatus Status { get; }

        public uint Position { get; }
    }
}
