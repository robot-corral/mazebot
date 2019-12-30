using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public class PositionController : IPositionController
    {
        public bool IsConnected { get { return false; /* TODO */ } }

        public Task<bool> TryToConnect(string serialDeviceId)
        {
            // TODO
            return Task.FromResult(false);
        }
    }
}
