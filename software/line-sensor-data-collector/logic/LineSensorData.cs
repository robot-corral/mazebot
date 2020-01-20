namespace line_sensor.data_collector.logic
{
    public class LineSensorData
    {
        public byte NumberOfCalls { get; set; }
        public byte NumberOfFailures { get; set; }
        public byte NumberOfCrcErrors { get; set; }

        public byte CurrentStatus { get; set; }
        public uint CurrentDetailedStatus { get; set; }
        public uint CumulativeDetailedStatusSinceLastReset { get; set; }

        public ushort[] SensorValues { get; set; }
    }
}
