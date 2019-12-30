using System.Threading.Tasks;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui
{
    public class SerialDeviceConnectDisconnectCommand : BaseCommandWithParameter<MainModel>
    {
        protected override bool CanExecuteImpl(MainModel parameter)
        {
            return parameter != null && parameter.SelectedSerialDevice != null;
        }

        protected override Task ExecuteAsyncImpl(MainModel parameter)
        {
            return Task.CompletedTask;
        }
    }
}
