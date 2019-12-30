using System;
using System.Threading.Tasks;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui
{
    public class SerialDeviceConnectDisconnectCommand : BaseCommandWithParameter<MainModel>
    {
        public SerialDeviceConnectDisconnectCommand(IPositionController positionController)
        {
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));
        }

        protected override bool CanExecuteImpl(MainModel parameter)
        {
            return !this.isBusy && parameter != null && parameter.SelectedSerialDevice != null;
        }

        protected override async Task ExecuteAsyncImpl(MainModel parameter)
        {
            SetIsBusy(true, parameter);

            try
            {
                if (parameter == null ||
                    parameter.SelectedSerialDevice == null)
                {
                    return;
                }

                if (this.positionController.IsConnected)
                {
                    // TODO add disconnect
                }
                else
                {
                    if (await this.positionController.TryToConnect(parameter.SelectedSerialDevice.Id))
                    {
                        parameter.ConnectedPositionControllerDeviceModel.IsConnected = true;
                    }
                    else
                    {
                        // TODO handle errors
                    }
                }
            }
            finally
            {
                SetIsBusy(false, parameter);
            }
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
        private readonly IPositionController positionController;
    }
}
