using System;

namespace line_sensor.data_collector.shared
{
    // TODO
    // https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/concepts/caller-information

    public interface ILogger
    {
        void Error(string message);

        void Error(Exception exception, string message);

        void Event(string eventName);
    }
}
