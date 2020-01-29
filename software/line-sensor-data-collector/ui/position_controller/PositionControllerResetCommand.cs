using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerResetCommand : BaseCommandWithParameter<IPositionControllerDeviceModel>
    {
        public PositionControllerResetCommand(IPositionController positionController)
        {
        }

        public override bool CanExecute(IPositionControllerDeviceModel parameter)
        {
            // TODO pkrupets
            return false;
        }

        public override void Execute(IPositionControllerDeviceModel parameter)
        {
        }
    }
}
