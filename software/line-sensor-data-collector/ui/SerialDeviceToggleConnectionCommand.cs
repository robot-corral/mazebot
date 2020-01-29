using System;
using System.Threading.Tasks;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.position_controller;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui
{
    public class SerialDeviceToggleConnectionCommand : BaseCommandWithParameter<MainModel>
    {
        public SerialDeviceToggleConnectionCommand(IPositionController positionController)
        {
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));
        }

        public override bool CanExecute(MainModel parameter)
        {
            return parameter != null &&
                   parameter.SelectedSerialDevice != null &&
                   (parameter.GetBusyUIComponents() & (UiComponent.ALL_SERIAL_DEVICES | UiComponent.POSITION_CONTROLLER)) == 0;
        }

        public override void Execute(MainModel parameter)
        {
            if (parameter == null ||
                parameter.SelectedSerialDevice == null)
            {
                return;
            }

            if (parameter.PositionControllerDeviceModel.IsConnected)
            {
                parameter.PositionControllerDeviceModel.DisconnectCommand.Execute(parameter.PositionControllerDeviceModel);
            }
            else
            {
                SerialDeviceModel selectedSerialDevice = parameter.SelectedSerialDevice;

                parameter.SetBusy(UiComponent.ALL_SERIAL_DEVICES | UiComponent.POSITION_CONTROLLER, true);

                this.positionController.TryToConnect(selectedSerialDevice.Id)
                    .ContinueWith(t =>
                    {
                        try
                        {
                            if (this.positionController.IsOkStatus(t.Result))
                            {
                                parameter.PositionControllerDeviceModel.Connected(selectedSerialDevice.Id);
                            }
                            else
                            {
                                parameter.LogEntries.Add(new PositionControllerErrorStatusLogEntry(t.Result));
                            }
                        }
                        finally
                        {
                            parameter.SetBusy(UiComponent.ALL_SERIAL_DEVICES | UiComponent.POSITION_CONTROLLER, false);
                        }
                    }, TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
            }
        }

        private readonly IPositionController positionController;
    }
}
