using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public interface IPositionController
    {
        bool IsConnected { get; }

        Task<bool> TryToConnect(string serialDeviceId);
    }
}
