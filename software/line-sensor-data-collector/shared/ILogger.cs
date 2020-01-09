using System;

namespace line_sensor.data_collector.shared
{
    public interface ILogger
    {
        void Error(Exception exception, string message);

        void Event(string eventName);
    }
}
