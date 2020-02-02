using System.ComponentModel;
using System.Threading.Tasks;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public interface IWirelessLineSensorDeviceModel : INotifyPropertyChanged
    {
        Task Disconnect();
    }
}
