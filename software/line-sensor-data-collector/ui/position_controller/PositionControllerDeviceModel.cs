using System;
using System.ComponentModel;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerDeviceModel : IPositionControllerDeviceModel, IUiComponent
    {
        public const string TITLE_CONNECT    = "Connect";
        public const string TITLE_DISCONNECT = "Disconnect";

        public PositionControllerDeviceModel(MainModel mainModel,
                                             IPositionController positionController)
        {
            this.mainModel = mainModel ?? throw new ArgumentNullException(nameof(mainModel));
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));

            ConnectDisconnectTitle = TITLE_CONNECT;
            DisconnectCommand = new PositionControllerDisconnectCommand(this.positionController);
            EmergencyStopCommand = new PositionControllerEmergencyStopCommand(this.positionController);
            ResetPositionControllerCommand = new PositionControllerResetCommand(this.positionController);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void OnBusyChanged(bool isBusy)
        {
            IsBusy = isBusy;
            DisconnectCommand.UpdateCanExecute(this);
            EmergencyStopCommand.UpdateCanExecute(this);
            ResetPositionControllerCommand.UpdateCanExecute(this);
        }

        public void Connected(string deviceId)
        {
            Id = deviceId;

            if (deviceId != null)
            {
                IsConnected = true;
                PacketsTotalNumber = 0;
                PacketsFailuresNumber = 0;
                PacketsCrcFailuresNumber = 0;
                ConnectDisconnectTitle = TITLE_DISCONNECT;
            }
            else
            {
                IsConnected = false;
                ConnectDisconnectTitle = TITLE_CONNECT;
            }

            DisconnectCommand.UpdateCanExecute(this);
            EmergencyStopCommand.UpdateCanExecute(this);
            ResetPositionControllerCommand.UpdateCanExecute(this);
        }

        public UiComponent GetBusyUIComponents()
        {
            return this.mainModel.GetBusyUIComponents();
        }

        public void SetBusy(UiComponent uiComponent, bool isBusy)
        {
            this.mainModel.SetBusy(uiComponent, isBusy);
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

        public bool IsBusy
        {
            get { return this.isBusy; }
            private set
            {
                if (this.isBusy != value)
                {
                    this.isBusy = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsBusy)));
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
            set
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
            set
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
            set
            {
                if (this.packetsFailuresNumber != value)
                {
                    this.packetsFailuresNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PacketsFailuresNumber)));
                }
            }
        }

        public BaseCommandWithParameter<IPositionControllerDeviceModel> DisconnectCommand { get; }

        public BaseCommandWithParameter<IPositionControllerDeviceModel> EmergencyStopCommand { get; }

        public BaseCommandWithParameter<IPositionControllerDeviceModel> ResetPositionControllerCommand { get; }

        private string id;
        private bool isBusy;
        private bool isConnected;
        private string connectDisconnectTitle;

        private int packetsTotalNumber;
        private int packetsFailuresNumber;
        private int packetsCrcFailuresNumber;

        private readonly MainModel mainModel;
        private readonly IPositionController positionController;
    }
}
