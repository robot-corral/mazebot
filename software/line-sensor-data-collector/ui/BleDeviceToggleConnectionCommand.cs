using System;
using System.Threading.Tasks;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

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
            return !this.isBusy && parameter != null && parameter.SelectedBleDevice != null;
        }

        public override void Execute(MainModel parameter)
        {
            //if (parameter == null ||
            //    parameter.SelectedSerialDevice == null)
            //{
            //    return;
            //}

            //if (parameter.WirelessLineSensorDeviceModel.IsConnected)
            //{
            //    this.wirelessLineSensor.Disconnect();
            //    parameter.WirelessLineSensorDeviceModel.IsConnected = false;
            //}
            //else
            //{
            //    SetIsBusy(true, parameter);

            //    BleDeviceModel bleDeviceModel = parameter.SelectedBleDevice;

            //    if (bleDeviceModel == null)
            //    {
            //        SetIsBusy(false, parameter);
            //        return;
            //    }

            //    bleDeviceModel.IsBusy = true;

            //    this.wirelessLineSensor.TryToConnect(bleDeviceModel.Id)
            //        .ContinueWith(t =>
            //        {
            //            try
            //            {
            //                if (t.Result)
            //                {
            //                    parameter.ConnectedWirelessLineSensorDeviceModel.IsConnected = true;
            //                }
            //                else
            //                {
            //                    // TODO handle errors
            //                }
            //            }
            //            finally
            //            {
            //                SetIsBusy(false, parameter);
            //            }
            //        }, TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
            //}
        }

        private void SetIsBusy(bool isBusy, MainModel parameter)
        {
            if (this.isBusy != isBusy)
            {
                this.isBusy = isBusy;
                UpdateCanExecute(parameter);
            }
        }

        private bool isBusy;

        private readonly IWirelessLineSensor wirelessLineSensor;
    }
}
