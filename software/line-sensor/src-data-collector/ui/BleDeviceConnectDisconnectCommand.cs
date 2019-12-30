using System;
using System.Threading.Tasks;
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

        protected override Task ExecuteAsyncImpl(MainModel parameter)
        {
            return Task.CompletedTask;
        }

        private readonly IWirelessLineSensor wirelessLineSensor;
    }
}
