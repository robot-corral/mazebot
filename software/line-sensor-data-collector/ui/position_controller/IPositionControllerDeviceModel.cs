using System.ComponentModel;
using System.Threading.Tasks;

using Windows.UI.Core;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.position_controller
{
    public interface IPositionControllerDeviceModel : INotifyPropertyChanged
    {
        string Id { get; }

        bool IsConnected { get; }

        string StatusDisplayName { get; }

        string ConnectDisconnectTitle { get; }

        double Position { get; }

        int PacketsTotalNumber { get; set; }

        int PacketsCrcFailuresNumber { get; set; }

        int PacketsFailuresNumber { get; set; }

        CoreDispatcher Dispatcher { get; }

        /*
         * commands below aren't guaranteed to finish execution when Execute method exits (they
         * can use busy UI flags to continue working without blocking UI thread).
         */

        BaseCommandWithParameter<IPositionControllerDeviceModel> CalibratePositionControllerCommand { get; }

        BaseCommandWithParameter<IPositionControllerDeviceModel> DisconnectCommand { get; }

        BaseCommandWithParameter<IPositionControllerDeviceModel> EmergencyStopCommand { get; }

        BaseCommandWithParameter<IPositionControllerDeviceModel> ResetPositionControllerCommand { get; }

        Task Disconnect();

        void SetStatus(PositionControllerCommand fromCommand, PositionControllerResponse status);

        void Connected(string deviceId, PositionControllerResponse status);

        UiComponent GetBusyUIComponents();

        /// <returns>previously busy components</returns>
        UiComponent SetBusy(UiComponent uiComponent, bool isBusy);
    }
}
