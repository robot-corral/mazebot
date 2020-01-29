using System.ComponentModel;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.position_controller
{
    public interface IPositionControllerDeviceModel : INotifyPropertyChanged
    {
        string Id { get; }

        bool IsBusy { get; }

        bool IsConnected { get; }

        string ConnectDisconnectTitle { get; }

        int PacketsTotalNumber { get; set; }

        int PacketsCrcFailuresNumber { get; set; }

        int PacketsFailuresNumber { get; set; }

        BaseCommandWithParameter<IPositionControllerDeviceModel> DisconnectCommand { get; }

        BaseCommandWithParameter<IPositionControllerDeviceModel> EmergencyStopCommand { get; }

        BaseCommandWithParameter<IPositionControllerDeviceModel> ResetPositionControllerCommand { get; }

        void Connected(string deviceId);

        UiComponent GetBusyUIComponents();

        void SetBusy(UiComponent uiComponent, bool isBusy);
    }
}
