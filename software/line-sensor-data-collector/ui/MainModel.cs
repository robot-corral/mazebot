using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;
using Windows.UI.Core;
using Windows.UI.Xaml;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui
{
    public class MainModel : INotifyPropertyChanged
    {
        public MainModel(CoreDispatcher dispatcher, ILogger logger)
        {
            this.positionController = new PositionController();
            this.wirelessLineSensor = new WirelessLineSensor(logger);

            this.serialDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.AllSupportedSerialDevices = new ObservableCollection<SerialDeviceModel>();
            this.ConnectedPositionControllerDeviceModel = new ConnectedPositionControllerDeviceModel();
            this.serialDeviceConnectDisconnectCommand = new SerialDeviceConnectDisconnectCommand(this.positionController);

            this.bleDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.AllSupportedBleDevices = new ObservableCollection<BleDeviceModel>();
            this.ConnectedWirelessLineSensorDeviceModel = new ConnectedWirelessLineSensorDeviceModel();
            this.bleDeviceConnectDisconnectCommand = new BleDeviceConnectDisconnectCommand(this.wirelessLineSensor);

            this.bleDeviceWatcher = new BleDeviceWatcher(dispatcher, this);
            this.serialDeviceWatcher = new SerialDeviceWatcher(dispatcher, this);
        }

        public void Initialize()
        {
            this.bleDeviceWatcher.Start();
            this.serialDeviceWatcher.Start();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public ObservableCollection<SerialDeviceModel> AllSupportedSerialDevices { get; }

        public SerialDeviceModel SelectedSerialDevice
        {
            get { return this.selectedSerialDevice; }
            set
            {
                if (this.selectedSerialDevice != value)
                {
                    this.selectedSerialDevice = value;
                    this.serialDeviceConnectDisconnectCommand.UpdateCanExecute(this);
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedSerialDevice)));
                }
            }
        }

        public Visibility SerialDeviceScanningIndicatorVisible
        {
            get { return this.serialDeviceScanningIndicatorVisible; }
            set
            {
                if (this.serialDeviceScanningIndicatorVisible != value)
                {
                    this.serialDeviceScanningIndicatorVisible = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SerialDeviceScanningIndicatorVisible)));
                }
            }
        }

        public ConnectedPositionControllerDeviceModel ConnectedPositionControllerDeviceModel { get; }

        public ICommand SerialDeviceConnectDisconnectCommand { get { return this.serialDeviceConnectDisconnectCommand; } }

        public ObservableCollection<BleDeviceModel> AllSupportedBleDevices { get; }

        public BleDeviceModel SelectedBleDevice
        {
            get { return this.selectedBleDevice; }
            set
            {
                if (this.selectedBleDevice != value)
                {
                    this.selectedBleDevice = value;
                    this.bleDeviceConnectDisconnectCommand.UpdateCanExecute(this);
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedBleDevice)));
                }
            }
        }

        public Visibility BleDeviceScanningIndicatorVisible
        {
            get { return this.bleDeviceScanningIndicatorVisible; }
            set
            {
                if (this.bleDeviceScanningIndicatorVisible != value)
                {
                    this.bleDeviceScanningIndicatorVisible = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(BleDeviceScanningIndicatorVisible)));
                }
            }
        }

        public ConnectedWirelessLineSensorDeviceModel ConnectedWirelessLineSensorDeviceModel { get; }

        public ICommand BleDeviceConnectDisconnectCommand { get { return this.bleDeviceConnectDisconnectCommand; } }

        private SerialDeviceModel selectedSerialDevice;
        private Visibility serialDeviceScanningIndicatorVisible;

        private BleDeviceModel selectedBleDevice;
        private Visibility bleDeviceScanningIndicatorVisible;

        private readonly IPositionController positionController;
        private readonly IWirelessLineSensor wirelessLineSensor;

        private readonly BleDeviceWatcher bleDeviceWatcher;
        private readonly BaseCommandWithParameter<MainModel> bleDeviceConnectDisconnectCommand;

        private readonly SerialDeviceWatcher serialDeviceWatcher;
        private readonly BaseCommandWithParameter<MainModel> serialDeviceConnectDisconnectCommand;
    }
}
