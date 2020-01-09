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

        protected override void ExecuteImpl(MainModel parameter)
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
                SetIsBusy(false, parameter);

                this.positionController.TryToConnect(parameter.SelectedSerialDevice.Id)
                    .ContinueWith(t =>
                    {
                        try
                        {
                            if (t.Result)
                            {
                                parameter.ConnectedPositionControllerDeviceModel.IsConnected = true;
                            }
                            else
                            {
                                // TODO handle errors
                            }
                        }
                        finally
                        {
                            SetIsBusy(false, parameter);
                        }
                    }, TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
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
