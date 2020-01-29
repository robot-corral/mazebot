using System;
using System.Threading.Tasks;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerEmergencyStopCommand : BaseCommandWithParameter<IPositionControllerDeviceModel>
    {
        public PositionControllerEmergencyStopCommand(IPositionController positionController)
        {
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));
        }

        public override bool CanExecute(IPositionControllerDeviceModel parameter)
        {
            return parameter != null &&
                   parameter.IsConnected &&
                   parameter.IsBusy == false;
        }

        public override void Execute(IPositionControllerDeviceModel parameter)
        {
            if (parameter == null)
            {
                return;
            }

            parameter.SetBusy(UiComponent.POSITION_CONTROLLER, true);

            this.positionController.StrongEmergencyStop()
                .ContinueWith(t =>
                              {
                                  if (t.Result)
                                  {
                                      // TODO pkrupets report success
                                  }
                                  else
                                  {
                                      // TODO pkrupets report error
                                  }

                                  parameter.SetBusy(UiComponent.POSITION_CONTROLLER, false);
                              },
                              TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
        }

        private readonly IPositionController positionController;
    }
}
