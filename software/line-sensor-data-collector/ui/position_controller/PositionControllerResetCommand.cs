using System;
using System.Threading.Tasks;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerResetCommand : BaseCommandWithParameter<IPositionControllerDeviceModel>
    {
        public PositionControllerResetCommand(IPositionController positionController)
        {
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));
        }

        public override bool CanExecute(IPositionControllerDeviceModel parameter)
        {
            return parameter != null &&
                   parameter.IsConnected &&
                   (parameter.GetBusyUIComponents() & UiComponent.POSITION_CONTROLLER) == 0;
        }

        public override void Execute(IPositionControllerDeviceModel parameter)
        {
            if (parameter == null)
            {
                return;
            }

            parameter.SetBusy(UiComponent.POSITION_CONTROLLER, true);

            this.positionController.Reset()
                .ContinueWith(t =>
                              {
                                  parameter.SetStatus(PositionControllerCommand.RESET, t.Result);
                                  parameter.SetBusy(UiComponent.POSITION_CONTROLLER, false);
                              },
                              TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
        }

        private readonly IPositionController positionController;
    }
}
