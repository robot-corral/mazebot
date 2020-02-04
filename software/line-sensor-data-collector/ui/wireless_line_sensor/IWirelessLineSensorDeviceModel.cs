using System.ComponentModel;
using System.Threading.Tasks;

using Windows.UI.Core;

using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public interface IWirelessLineSensorDeviceModel : INotifyPropertyChanged
    {
        string Id { get; }

        bool IsConnected { get; }

        string ConnectDisconnectTitle { get; }

        int PacketsTotalNumber { get; }

        int PacketsCrcFailuresNumber { get; }

        int PacketsFailuresNumber { get; }

        CoreDispatcher Dispatcher { get; }

        /*
         * commands below aren't guaranteed to finish execution when Execute method exits (they
         * can use busy UI flags to continue working without blocking UI thread).
         */

        BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetWirelessLineSensorCommand { get; }

        BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetLineSensorCommand { get; }

        BaseCommandWithParameter<IWirelessLineSensorDeviceModel> DisconnectCommand { get; }

        Task Disconnect();

        void Connected(string deviceId, bool isConnected);

        UiComponent GetBusyUIComponents();

        /// <returns>previously busy components</returns>
        UiComponent SetBusy(UiComponent uiComponent, bool isBusy);
    }
}
