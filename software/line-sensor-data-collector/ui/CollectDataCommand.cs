using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui
{
    public class CollectDataCommand : BaseCommandWithParameter<MainModel>
    {
        public CollectDataCommand(IPositionController positionController, IWirelessLineSensor wirelessLineSensor)
        {
            // TODO pkrupets
        }

        public override bool CanExecute(MainModel parameter)
        {
            // TODO pkrupets
            return false;
        }

        public override void Execute(MainModel parameter)
        {
            // TODO pkrupets
            throw new System.NotImplementedException();
        }
    }
}
