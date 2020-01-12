using System;
using System.Threading;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

namespace line_sensor.data_collector.shared
{
    public class Logger : ILogger
    {
        public void Initialize()
        {
            this.lockObject.WaitOne();

            try
            {
                // to decode file use: tracerpt file.etl
                // log file is located at: %LOCALAPPDATA%\Packages\534ba322-edf8-4a0c-ae29-eb291114de44_j4k0b6t459tbp\LocalState\Logs
                this.session = new FileLoggingSession("LineSensorDataCollector_Session");
                this.session.LogFileGenerated += LogFileGeneratedHandler;
                this.channel = new LoggingChannel("MainChannel", null);
                this.session.AddLoggingChannel(this.channel);
            }
            finally
            {
                this.lockObject.Release();
            }
        }

        public async Task OnSuspending()
        {
            this.lockObject.WaitOne();

            try
            {
                this.channel.Dispose();
                await this.session.CloseAndSaveToFileAsync()
                                  .AsTask()
                                  .ConfigureAwait(false);
            }
            finally
            {
                this.lockObject.Release();
            }
        }

        public void Error(string message)
        {
            // TODO
        }

        public void Error(Exception exception, string message)
        {
            // TODO
        }

        public void Event(string eventName)
        {
            this.channel.LogEvent(eventName);
        }

        private void LogFileGeneratedHandler(IFileLoggingSession sender, LogFileGeneratedEventArgs args)
        {
            // TODO
        }

        private LoggingChannel channel;
        private FileLoggingSession session;

        private readonly Semaphore lockObject = new Semaphore(1, 1);
    }
}
