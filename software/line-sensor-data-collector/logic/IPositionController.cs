using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public interface IPositionController
    {
        Task<PositionControllerStatus> TryToConnect(string serialDeviceId);

        void Disconnect();

        Task<PositionControllerResponse> Calibrate();

        Task<PositionControllerResponse> EmergencyStop();

        Task<PositionControllerResponse> MoveIfIdle(PositionControllerDirection direction, uint steps);

        Task<PositionControllerResponse> GetPosition();

        Task Reset();
    }
}
