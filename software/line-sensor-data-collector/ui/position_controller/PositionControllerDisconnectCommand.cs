using System;
using System.Threading.Tasks;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

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
            return parameter.IsConnected && (parameter.GetBusyUIComponents() & UiComponent.POSITION_CONTROLLER) == 0;
        }

        public override void Execute(IPositionControllerDeviceModel parameter)
        {
            if (parameter == null)
            {
                return;
            }

            parameter.SetBusy(UiComponent.POSITION_CONTROLLER, true);

            this.positionController.Disconnect()
                .ContinueWith(t =>
                              {
                                  parameter.Connected(null, null);
                                  parameter.SetStatus(PositionControllerCommand.EMERGENCY_STOP, new PositionControllerResponse(PositionControllerStatus.OK));
                                  parameter.SetBusy(UiComponent.POSITION_CONTROLLER, false);
                              },
                              TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
        }

        private readonly IPositionController positionController;
    }
}
