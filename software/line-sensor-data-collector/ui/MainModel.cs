using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

using Windows.UI.Core;
using Windows.UI.Xaml;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.log;
using line_sensor.data_collector.ui.position_controller;
using line_sensor.data_collector.ui.ui_component;
using line_sensor.data_collector.ui.wireless_line_sensor;

namespace line_sensor.data_collector.ui
{
    public class MainModel : INotifyPropertyChanged
    {
        public MainModel(ILogger logger)
        {
            this.positionController = new PositionController(logger);
            this.wirelessLineSensor = new WirelessLineSensor(logger);

            this.serialDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.AllSupportedSerialDevices = new ObservableCollection<SerialDeviceModel>();
            this.AllSupportedSerialDevices.CollectionChanged += AllSupportedSerialDevicesCollectionChanged;
            this.SerialDeviceToggleConnectionCommand = new SerialDeviceToggleConnectionCommand(this.positionController);

            this.bleDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.AllSupportedBleDevices = new ObservableCollection<BleDeviceModel>();
            this.BleDeviceToggleScanningCommand = new BleDeviceToggleScanningCommand();
            this.BleDeviceToggleConnectionCommand = new BleDeviceToggleConnectionCommand(this.wirelessLineSensor);

            this.CollectDataCommand = new CollectDataCommand(this.positionController, this.wirelessLineSensor);

            this.positionControllerDeviceModel = new PositionControllerDeviceModel(this.positionController);
            this.positionControllerDeviceModel.PropertyChanged += PositionControllerDeviceModelChanged;

            this.wirelessLineSensorDeviceModel = new WirelessLineSensorDeviceModel(this, this.wirelessLineSensor);

            this.LogEntries = new ObservableCollection<ILogEntryModel>();

            this.busyUIComponents = UiComponent.NONE;
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void Initialize(CoreDispatcher dispatcher)
        {
            this.positionControllerDeviceModel.Initialize(this, dispatcher);
            this.bleDeviceWatcher = new BleDeviceWatcher(dispatcher, this);
            this.serialDeviceWatcher = new SerialDeviceWatcher(dispatcher, this);
            this.bleDeviceWatcher.Start();
            this.serialDeviceWatcher.Start();
        }

        public UiComponent GetBusyUIComponents()
        {
            return this.busyUIComponents;
        }

        /// <returns>returns previously busy components</returns>
        public UiComponent SetBusy(UiComponent uiComponent, bool isBusy)
        {
            UiComponent oldBusyComponents = this.busyUIComponents;
            UiComponent changedUIComponents;

            if (isBusy)
            {
                // this.busyUIComponents     0011
                // uiComponent               0101
                // changedUIComponents       0100
                // new this.busyUIComponents 0111

                changedUIComponents = ~this.busyUIComponents & uiComponent;
                this.busyUIComponents |= uiComponent;
            }
            else
            {
                // this.busyUIComponents     0011
                // uiComponent               0101
                // changedUIComponents       0001
                // new this.busyUIComponents 0010

                changedUIComponents = this.busyUIComponents & uiComponent;
                this.busyUIComponents &= ~uiComponent;
            }

            UpdateBusyComponents(changedUIComponents, isBusy);

            return oldBusyComponents;
        }

        private void UpdateBusyComponents(UiComponent changedUIComponents, bool isBusy)
        {
            if ((changedUIComponents & UiComponent.ALL_SERIAL_DEVICES) == UiComponent.ALL_SERIAL_DEVICES)
            {
                UpdateIsAllSupportedSerialDevicesEnabled();
            }
            if ((changedUIComponents & UiComponent.POSITION_CONTROLLER) == UiComponent.POSITION_CONTROLLER)
            {
                this.positionControllerDeviceModel.OnBusyChanged(isBusy);
            }
            if ((changedUIComponents & UiComponent.ALL_SERIAL_DEVICES) == UiComponent.ALL_SERIAL_DEVICES ||
                (changedUIComponents & UiComponent.POSITION_CONTROLLER) == UiComponent.POSITION_CONTROLLER)
            {
                SerialDeviceToggleConnectionCommand.UpdateCanExecute(this);
            }
            if ((changedUIComponents & UiComponent.POSITION_CONTROLLER) == UiComponent.POSITION_CONTROLLER ||
                (changedUIComponents & UiComponent.WIRELESS_LINE_SENSOR) == UiComponent.WIRELESS_LINE_SENSOR)
            {
                CollectDataCommand.UpdateCanExecute(this);
            }
        }

        /*
         * 
         * Serial devices
         * 
         */

        public ObservableCollection<SerialDeviceModel> AllSupportedSerialDevices { get; }

        public SerialDeviceModel SelectedSerialDevice
        {
            get { return this.selectedSerialDevice; }
            set
            {
                if (this.selectedSerialDevice != value)
                {
                    this.selectedSerialDevice = value;
                    this.SerialDeviceToggleConnectionCommand.UpdateCanExecute(this);
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedSerialDevice)));
                }
            }
        }

        public bool IsAllSupportedSerialDevicesEnabled
        {
            get { return this.isAllSupportedSerialDevicesEnabled; }
            set
            {
                if (this.isAllSupportedSerialDevicesEnabled != value)
                {
                    this.isAllSupportedSerialDevicesEnabled = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsAllSupportedSerialDevicesEnabled)));
                }
            }
        }

        private void UpdateIsAllSupportedSerialDevicesEnabled()
        {
            IsAllSupportedSerialDevicesEnabled = !PositionControllerDeviceModel.IsConnected &&
                                                 AllSupportedSerialDevices.Count > 0 &&
                                                 (GetBusyUIComponents() & UiComponent.ALL_SERIAL_DEVICES) == 0;
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

        public BaseCommandWithParameter<MainModel> SerialDeviceToggleConnectionCommand { get; }

        /*
         * 
         * BLE devices
         * 
         */

        public ObservableCollection<BleDeviceModel> AllSupportedBleDevices { get; }

        public BleDeviceModel SelectedBleDevice
        {
            get { return this.selectedBleDevice; }
            set
            {
                if (this.selectedBleDevice != value)
                {
                    this.selectedBleDevice = value;
                    this.SerialDeviceToggleConnectionCommand.UpdateCanExecute(this);
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedBleDevice)));
                }
            }
        }

        public bool IsAllSupportedBleDevicesEnabled
        {
            get { return this.isAllSupportedBleDevicesEnabled; }
            set
            {
                if (this.isAllSupportedBleDevicesEnabled != value)
                {
                    this.isAllSupportedBleDevicesEnabled = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsAllSupportedBleDevicesEnabled)));
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

        public string BleDeviceToggleScanningCommandTitle
        {
            get { return this.bleDeviceToggleScanningCommandTitle; }
            set
            {
                if (this.bleDeviceToggleScanningCommandTitle != value)
                {
                    this.bleDeviceToggleScanningCommandTitle = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(BleDeviceToggleScanningCommandTitle)));
                }
            }
        }

        public BaseCommandWithParameter<MainModel> BleDeviceToggleScanningCommand { get; }

        public string BleDeviceToggleConnectionCommandTitle
        {
            get { return this.bleDeviceToggleConnectionCommandTitle; }
            set
            {
                if (this.bleDeviceToggleConnectionCommandTitle != value)
                {
                    this.bleDeviceToggleConnectionCommandTitle = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(BleDeviceToggleConnectionCommandTitle)));
                }
            }
        }

        public BaseCommandWithParameter<MainModel> BleDeviceToggleConnectionCommand { get; }

        /*
         * 
         * Others
         * 
         */

        public IPositionControllerDeviceModel PositionControllerDeviceModel { get { return this.positionControllerDeviceModel; } }

        public IWirelessLineSensorDeviceModel WirelessLineSensorDeviceModel { get { return this.wirelessLineSensorDeviceModel; } }

        public BaseCommandWithParameter<MainModel> CollectDataCommand { get; }

        public ObservableCollection<ILogEntryModel> LogEntries { get; }

        private void PositionControllerDeviceModelChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(PositionControllerDeviceModel.IsConnected))
            {
                UpdateIsAllSupportedSerialDevicesEnabled();
            }
        }

        private void AllSupportedSerialDevicesCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            UpdateIsAllSupportedSerialDevicesEnabled();
        }

        /* 
         * 
         * Fields
         * 
         */

        private UiComponent busyUIComponents;

        private SerialDeviceModel selectedSerialDevice;
        private bool isAllSupportedSerialDevicesEnabled;
        private Visibility serialDeviceScanningIndicatorVisible;

        private BleDeviceModel selectedBleDevice;
        private bool isAllSupportedBleDevicesEnabled;
        private Visibility bleDeviceScanningIndicatorVisible;
        private string bleDeviceToggleScanningCommandTitle;
        private string bleDeviceToggleConnectionCommandTitle;

        private BleDeviceWatcher bleDeviceWatcher;
        private SerialDeviceWatcher serialDeviceWatcher;

        private readonly IPositionController positionController;
        private readonly IWirelessLineSensor wirelessLineSensor;

        private readonly PositionControllerDeviceModel positionControllerDeviceModel;
        private readonly WirelessLineSensorDeviceModel wirelessLineSensorDeviceModel;
    }
}
