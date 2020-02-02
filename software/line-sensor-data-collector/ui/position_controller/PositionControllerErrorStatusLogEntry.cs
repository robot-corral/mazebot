using line_sensor.data_collector.logic;
using line_sensor.data_collector.ui.log;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerErrorStatusLogEntry : ILogEntryModel
    {
        public PositionControllerErrorStatusLogEntry(string message, PositionControllerCommand fromCommand, PositionControllerStatus status)
        {
            Title = message ?? fromCommand.ToString();
            Details = status.ToString();
        }

        public string Title { get; }

        public string Details { get; }
    }
}
