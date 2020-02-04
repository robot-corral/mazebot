using System;
using System.Threading.Tasks;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui
{
    public class BleDeviceToggleConnectionCommand : BaseCommandWithParameter<MainModel>
    {
        public BleDeviceToggleConnectionCommand(IWirelessLineSensor wirelessLineSensor)
        {
            this.wirelessLineSensor = wirelessLineSensor ?? throw new ArgumentNullException(nameof(wirelessLineSensor));
        }

        public override bool CanExecute(MainModel parameter)
        {
            return parameter != null &&
                   parameter.SelectedBleDevice != null &&
                   (parameter.GetBusyUIComponents() & (UiComponent.ALL_BLE_DEVICES | UiComponent.WIRELESS_LINE_SENSOR)) == 0;
        }

        public override void Execute(MainModel parameter)
        {
            if (parameter == null ||
                parameter.SelectedSerialDevice == null)
            {
                return;
            }

            if (parameter.WirelessLineSensorDeviceModel.IsConnected)
            {
                parameter.WirelessLineSensorDeviceModel.DisconnectCommand.Execute(parameter.WirelessLineSensorDeviceModel);
            }
            else
            {
                BleDeviceModel bleDeviceModel = parameter.SelectedBleDevice;

                bleDeviceModel.CanBeDeletedByWatcher = false;

                parameter.SetBusy(UiComponent.ALL_BLE_DEVICES | UiComponent.WIRELESS_LINE_SENSOR, true);

                this.wirelessLineSensor.TryToConnect(bleDeviceModel.Id)
                    .ContinueWith(t =>
                                  {
                                      parameter.WirelessLineSensorDeviceModel.Connected(bleDeviceModel.Id, t.Result);
                                      parameter.SetBusy(UiComponent.ALL_BLE_DEVICES | UiComponent.WIRELESS_LINE_SENSOR, false);
                                  },
                                  TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
            }
        }

        private readonly IWirelessLineSensor wirelessLineSensor;
    }
}
