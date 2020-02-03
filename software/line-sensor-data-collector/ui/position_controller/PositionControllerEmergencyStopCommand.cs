using System;
using System.Threading.Tasks;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

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
            // emergency stop can be executed even if ui component is busy
            return parameter != null && parameter.IsConnected;
        }

        public override void Execute(IPositionControllerDeviceModel parameter)
        {
            if (parameter == null)
            {
                return;
            }

            this.positionController.StrongEmergencyStop()
                .ContinueWith(t =>
                              {
                                  parameter.SetStatus(PositionControllerCommand.EMERGENCY_STOP, t.Result);
                              },
                              TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
        }

        private readonly IPositionController positionController;
    }
}
