using System.ComponentModel;
using System.Threading.Tasks;

using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public interface IWirelessLineSensorDeviceModel : INotifyPropertyChanged
    {
        Task Disconnect();

        int PacketsTotalNumber { get; }

        int PacketsCrcFailuresNumber { get; }

        int PacketsFailuresNumber { get; }

        BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetWirelessLineSensorCommand { get; }

        BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetLineSensorCommand { get; }
    }
}
