using System;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerDisconnectCommand : BaseCommandWithParameter<IPositionControllerDeviceModel>
    {
        public PositionControllerDisconnectCommand(IPositionController positionController)
        {
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));
        }

        public override bool CanExecute(IPositionControllerDeviceModel parameter)
        {
            return parameter.IsConnected && !parameter.IsBusy;
        }

        public override void Execute(IPositionControllerDeviceModel parameter)
        {
            this.positionController.Disconnect();
            parameter.Connected(null);
        }

        private readonly IPositionController positionController;
    }
}
