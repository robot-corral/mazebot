using Windows.UI.Xaml;

using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui
{
    public class BleDeviceToggleScanningCommand : BaseCommandWithParameter<MainModel>
    {
        public override bool CanExecute(MainModel parameter)
        {
            return parameter != null &&
                   !parameter.WirelessLineSensorDeviceModel.IsConnected &&
                   (parameter.GetBusyUIComponents() & (UiComponent.ALL_BLE_DEVICES | UiComponent.WIRELESS_LINE_SENSOR)) == 0;
        }

        public override void Execute(MainModel parameter)
        {
            if (parameter.BleDeviceScanningIndicatorVisible == Visibility.Visible)
            {
                parameter.StopScanningBleDevices();
            }
            else
            {
                parameter.StartScanningBleDevices();
            }
        }
    }
}
