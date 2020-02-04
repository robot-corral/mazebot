using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public class WirelessLineSensorResetWirelessLineSensorCommand : BaseCommandWithParameter<IWirelessLineSensorDeviceModel>
    {
        public WirelessLineSensorResetWirelessLineSensorCommand(IWirelessLineSensor wirelessLineSensor)
        {
            // TODO
        }

        public override bool CanExecute(IWirelessLineSensorDeviceModel parameter)
        {
            return false;
        }

        public override void Execute(IWirelessLineSensorDeviceModel parameter)
        {
            // TODO
        }
    }
}
