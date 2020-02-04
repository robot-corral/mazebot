using System;
using System.ComponentModel;
using System.Threading.Tasks;

using Windows.UI.Core;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public class WirelessLineSensorDeviceModel : IWirelessLineSensorDeviceModel
    {
        public WirelessLineSensorDeviceModel(IWirelessLineSensor wirelessLineSensor)
        {
            ConnectDisconnectTitle = TITLE_CONNECT;

            this.wirelessLineSensor = wirelessLineSensor ?? throw new ArgumentNullException(nameof(wirelessLineSensor));

            DisconnectCommand = new WirelessLineSensorDisconnectCommand(this.wirelessLineSensor);
            ResetLineSensorCommand = new WirelessLineSensorResetLineSensorCommand(this.wirelessLineSensor);
            ResetWirelessLineSensorCommand = new WirelessLineSensorResetWirelessLineSensorCommand(this.wirelessLineSensor);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void OnBusyChanged(bool isBusy)
        {
            DisconnectCommand.UpdateCanExecute();
            ResetLineSensorCommand.UpdateCanExecute();
            ResetWirelessLineSensorCommand.UpdateCanExecute();
        }

        public void Connected(string deviceId, bool isConnected)
        {
            Id = deviceId;

            if (isConnected)
            {
                IsConnected = true;
                ConnectDisconnectTitle = TITLE_DISCONNECT;
            }
            else
            {
                IsConnected = false;
                ConnectDisconnectTitle = TITLE_CONNECT;
            }

            DisconnectCommand.UpdateCanExecute();
            ResetLineSensorCommand.UpdateCanExecute();
            ResetWirelessLineSensorCommand.UpdateCanExecute();
        }

        public Task Disconnect()
        {
            this.wirelessLineSensor.Disconnect();
            return Task.CompletedTask;
        }

        public string Id
        {
            get { return this.id; }
            private set
            {
                if (this.id != value)
                {
                    this.id = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Id)));
                }
            }
        }

        public bool IsConnected
        {
            get { return this.isConnected; }
            private set
            {
                if (this.isConnected != value)
                {
                    this.isConnected = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsConnected)));
                }
            }
        }

        public void Initialize(MainModel mainModel, CoreDispatcher dispatcher)
        {
            Dispatcher = dispatcher;
            this.mainModel = mainModel;
        }

        public UiComponent GetBusyUIComponents()
        {
            return this.mainModel.GetBusyUIComponents();
        }

        public UiComponent SetBusy(UiComponent uiComponent, bool isBusy)
        {
            return this.mainModel.SetBusy(uiComponent, isBusy);
        }

        public string ConnectDisconnectTitle
        {
            get { return this.connectDisconnectTitle; }
            private set
            {
                if (this.connectDisconnectTitle != value)
                {
                    this.connectDisconnectTitle = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ConnectDisconnectTitle)));
                }
            }
        }

        public int PacketsTotalNumber
        {
            get { return this.packetsTotalNumber; }
            private set
            {
                if (this.packetsTotalNumber != value)
                {
                    this.packetsTotalNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PacketsTotalNumber)));
                }
            }
        }

        public int PacketsCrcFailuresNumber
        {
            get { return this.packetsCrcFailuresNumber; }
            private set
            {
                if (this.packetsCrcFailuresNumber != value)
                {
                    this.packetsCrcFailuresNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PacketsCrcFailuresNumber)));
                }
            }
        }

        public int PacketsFailuresNumber
        {
            get { return this.packetsFailuresNumber; }
            private set
            {
                if (this.packetsFailuresNumber != value)
                {
                    this.packetsFailuresNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PacketsFailuresNumber)));
                }
            }
        }

        public CoreDispatcher Dispatcher { get; private set; }

        public BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetLineSensorCommand { get; }

        public BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetWirelessLineSensorCommand { get; }

        public BaseCommandWithParameter<IWirelessLineSensorDeviceModel> DisconnectCommand { get; }

        private MainModel mainModel;

        private int packetsTotalNumber;
        private int packetsFailuresNumber;
        private int packetsCrcFailuresNumber;

        private bool isConnected;

        private string id;
        private string connectDisconnectTitle;

        private readonly IWirelessLineSensor wirelessLineSensor;

        private const string TITLE_CONNECT = "Connect";
        private const string TITLE_DISCONNECT = "Disconnect";
    }
}
