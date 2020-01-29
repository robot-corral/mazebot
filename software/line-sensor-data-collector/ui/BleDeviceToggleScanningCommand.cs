using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui
{
    public class BleDeviceToggleScanningCommand : BaseCommandWithParameter<MainModel>
    {
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
