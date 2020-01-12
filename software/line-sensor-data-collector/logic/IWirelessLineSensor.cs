using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public delegate void DeviceRemovedEvent(WirelessLineSensor source, string removedDeviceId);

    public interface IWirelessLineSensor
    {
        event DeviceRemovedEvent DeviceRemovedEvent;

        bool IsConnected { get; }

        Task<bool> TryToConnect(string deviceId);

        Task<bool> StartQueryingLineSensor();

        Task<bool> StopQueryingLineSensor();
    }
}
