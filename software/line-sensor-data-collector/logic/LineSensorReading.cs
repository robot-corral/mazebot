using System.Collections.Generic;

namespace line_sensor.data_collector.logic
{
    public class LineSensorReading
    {
        public double Position { get; set; }

        public IList<ushort[]> SensorValues { get; set; }
    }
}
