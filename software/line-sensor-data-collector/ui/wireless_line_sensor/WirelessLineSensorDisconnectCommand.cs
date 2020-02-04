using System;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public class WirelessLineSensorDisconnectCommand : BaseCommandWithParameter<IWirelessLineSensorDeviceModel>
    {
        public WirelessLineSensorDisconnectCommand(IWirelessLineSensor wirelessLineSensor)
        {
            this.wirelessLineSensor = wirelessLineSensor ?? throw new ArgumentNullException(nameof(wirelessLineSensor));
        }

        public override bool CanExecute(IWirelessLineSensorDeviceModel parameter)
        {
            return parameter.IsConnected && (parameter.GetBusyUIComponents() & UiComponent.WIRELESS_LINE_SENSOR) == 0;
        }

        public override void Execute(IWirelessLineSensorDeviceModel parameter)
        {
            if (parameter == null)
            {
                return;
            }

            this.wirelessLineSensor.Disconnect();
            parameter.Connected(null, false);
        }

        private readonly IWirelessLineSensor wirelessLineSensor;
    }
}
