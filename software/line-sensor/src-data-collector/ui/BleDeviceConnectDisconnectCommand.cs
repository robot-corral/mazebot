using System;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui
{
    public class BleDeviceConnectDisconnectCommand : BaseCommandWithParameter<MainModel>
    {
        public BleDeviceConnectDisconnectCommand(IWirelessLineSensor wirelessLineSensor)
        {
            this.wirelessLineSensor = wirelessLineSensor ?? throw new ArgumentNullException(nameof(wirelessLineSensor));
        }

        protected override bool CanExecuteImpl(MainModel parameter)
        {
            return parameter != null && parameter.SelectedBleDevice != null;
        }

        protected override void ExecuteImpl(MainModel parameter)
        {
            // TODO
        }

        private readonly IWirelessLineSensor wirelessLineSensor;
    }
}
