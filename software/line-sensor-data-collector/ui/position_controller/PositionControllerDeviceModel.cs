using System;
using System.ComponentModel;
using System.Threading.Tasks;

using Windows.UI.Core;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerDeviceModel : IPositionControllerDeviceModel, IUiComponent
    {
        public PositionControllerDeviceModel(IPositionController positionController)
        {
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));

            ConnectDisconnectTitle = TITLE_CONNECT;
            StatusDisplayName = STATUS_DISPLAY_NAME_NOT_AVAILABLE;
            DisconnectCommand = new PositionControllerDisconnectCommand(this.positionController);
            EmergencyStopCommand = new PositionControllerEmergencyStopCommand(this.positionController);
            ResetPositionControllerCommand = new PositionControllerResetCommand(this.positionController);
            CalibratePositionControllerCommand = new PositionControllerCalibrateCommand(this.positionController);
        }

        public void Initialize(MainModel mainModel, CoreDispatcher dispatcher)
        {
            Dispatcher = dispatcher;
            this.mainModel = mainModel ?? throw new ArgumentNullException(nameof(mainModel));
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void OnBusyChanged(bool isBusy)
        {
            DisconnectCommand.UpdateCanExecute();
            EmergencyStopCommand.UpdateCanExecute();
            ResetPositionControllerCommand.UpdateCanExecute();
            CalibratePositionControllerCommand.UpdateCanExecute();
        }

        public void Connected(string deviceId, PositionControllerResponse status)
        {
            Id = deviceId;

            if (deviceId != null && this.positionController.IsOkStatus(status.Status))
            {
                maxPosition = 0;
                IsConnected = true;
                PacketsTotalNumber = 0;
                PacketsFailuresNumber = 0;
                PacketsCrcFailuresNumber = 0;
                ConnectDisconnectTitle = TITLE_DISCONNECT;
            }
            else
            {
                maxPosition = 0;
                IsConnected = false;
                ConnectDisconnectTitle = TITLE_CONNECT;
            }

            SetStatus("Connection attempt", PositionControllerCommand.NONE, status);

            DisconnectCommand.UpdateCanExecute();
            EmergencyStopCommand.UpdateCanExecute();
            ResetPositionControllerCommand.UpdateCanExecute();
            CalibratePositionControllerCommand.UpdateCanExecute();
        }

        public UiComponent GetBusyUIComponents()
        {
            return this.mainModel.GetBusyUIComponents();
        }

        public UiComponent SetBusy(UiComponent uiComponent, bool isBusy)
        {
            return this.mainModel.SetBusy(uiComponent, isBusy);
        }

        public void SetStatus(PositionControllerCommand fromCommand, PositionControllerResponse status)
        {
            SetStatus(null, fromCommand, status);
        }

        private void SetStatus(string message, PositionControllerCommand fromCommand, PositionControllerResponse status)
        {
            if (status != null &&
                status.Status == PositionControllerStatus.PC_OK_IDLE && /* no errors should be present for calibration to be considered successful */
                fromCommand == PositionControllerCommand.CALIBRATE)
            {
                this.maxPosition = status.Position;
            }

            if (status == null)
            {
                Position = double.NaN;
                StatusDisplayName = STATUS_DISPLAY_NAME_NOT_AVAILABLE;
            }
            else
            {
                Position = ((double) status.Position) * 0.025d;
                StatusDisplayName = StatusToDisplayName(status.Status);

                ++PacketsTotalNumber;

                if ((status.Status & ~PositionControllerStatus.PC_OK_MASK) != 0)
                {
                    ++PacketsFailuresNumber;
                }
                if ((status.Status & PositionControllerStatus.PC_ERR_CRC) != 0 ||
                    (status.Status & PositionControllerStatus.CS_ERR_CRC) != 0)
                {
                    ++PacketsCrcFailuresNumber;
                }

                if (!this.positionController.IsOkStatus(status.Status))
                {
                    this.mainModel.LogEntries.Add(new PositionControllerErrorStatusLogEntry(message, fromCommand, status.Status));
                }
            }
        }

        private string StatusToDisplayName(PositionControllerStatus status)
        {
            PositionControllerStatus statusWithoutErrors = status & PositionControllerStatus.PC_OK_MASK;

            switch (statusWithoutErrors)
            {
                case PositionControllerStatus.PC_OK_RESET:          return "Reset";
                case PositionControllerStatus.PC_OK_IDLE:           return "Idle";
                case PositionControllerStatus.PC_OK_BUSY:           return "Busy";
                case PositionControllerStatus.PC_OK_EMERGENCY_STOP: return "Emergency Stop";
                default: return STATUS_DISPLAY_NAME_NOT_AVAILABLE;
            }
        }

        public Task Disconnect()
        {
            return this.positionController.Disconnect();
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

        public string StatusDisplayName
        {
            get { return this.statusDisplayName; }
            private set
            {
                if (this.statusDisplayName != value)
                {
                    this.statusDisplayName = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(StatusDisplayName)));
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

        public double Position
        {
            get { return this.position; }
            private set
            {
                if (this.position != value)
                {
                    this.position = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Position)));
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

        public BaseCommandWithParameter<IPositionControllerDeviceModel> DisconnectCommand { get; }

        public BaseCommandWithParameter<IPositionControllerDeviceModel> EmergencyStopCommand { get; }

        public BaseCommandWithParameter<IPositionControllerDeviceModel> ResetPositionControllerCommand { get; }

        public BaseCommandWithParameter<IPositionControllerDeviceModel> CalibratePositionControllerCommand { get; }

        private bool isConnected;

        private uint maxPosition;

        private string id;
        private string statusDisplayName;
        private string connectDisconnectTitle;

        private double position;
        private int packetsTotalNumber;
        private int packetsFailuresNumber;
        private int packetsCrcFailuresNumber;

        private MainModel mainModel;

        private readonly IPositionController positionController;

        private const string TITLE_CONNECT = "Connect";
        private const string TITLE_DISCONNECT = "Disconnect";

        private const string STATUS_DISPLAY_NAME_NOT_AVAILABLE = "N/A";
    }
}
