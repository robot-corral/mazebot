using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public delegate void DeviceRemovedEvent(WirelessLineSensor source, string removedDeviceId);

    public delegate void DeviceSentDataEvent(WirelessLineSensor source, LineSensorData newData);

    public interface IWirelessLineSensor
    {
        event DeviceRemovedEvent DeviceRemovedEvent;

        event DeviceSentDataEvent DeviceSentDataEvent;

        bool IsConnected { get; }

        Task<bool> TryToConnect(string deviceId);

        Task<bool> StartQueryingLineSensor();

        Task<bool> StopQueryingLineSensor();
    }
}
