using System;
using System.IO;
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
        public const string TITLE_COLLECT_DATA = "Collect Data";
        public const string TITLE_STOP_COLLECTING_DATA = "Stop\nCollecting\nData";

        public MainModel(ILogger logger)
        {
            this.positionController = new PositionController(logger);
            this.wirelessLineSensor = new WirelessLineSensor(logger);
            this.dataCollector = new DataCollector(this.positionController, this.wirelessLineSensor, logger);

            this.serialDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.AllSupportedSerialDevices = new ObservableCollection<SerialDeviceModel>();
            this.AllSupportedSerialDevices.CollectionChanged += AllSupportedSerialDevicesCollectionChanged;
            this.SerialDeviceToggleConnectionCommand = new SerialDeviceToggleConnectionCommand(this.positionController);

            this.bleDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.AllSupportedBleDevices = new ObservableCollection<BleDeviceModel>();
            this.AllSupportedBleDevices.CollectionChanged += AllSupportedBleDevicesCollectionChanged;
            this.BleDeviceToggleScanningCommand = new BleDeviceToggleScanningCommand();
            this.BleDeviceToggleConnectionCommand = new BleDeviceToggleConnectionCommand(this.wirelessLineSensor);

            this.CollectDataCommand = new CollectDataCommand(this.dataCollector);

            this.positionControllerDeviceModel = new PositionControllerDeviceModel(this.positionController);
            this.positionControllerDeviceModel.PropertyChanged += PositionControllerDeviceModelChanged;

            this.wirelessLineSensorDeviceModel = new WirelessLineSensorDeviceModel(this.wirelessLineSensor);
            this.wirelessLineSensorDeviceModel.PropertyChanged += WirelessLineSensorDeviceModelChanged;

            this.LogEntries = new ObservableCollection<ILogEntryModel>();

            this.busyUIComponents = UiComponent.NONE;

            this.CollectDataCommandTitle = TITLE_COLLECT_DATA;

            this.StepSize = 0.5;
            this.NumberOfSamples = 1000;
            this.FilePathPrefix = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "line_sensor_data_" + Guid.NewGuid());
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void Initialize(CoreDispatcher dispatcher)
        {
            this.dispatcher = dispatcher;
            this.positionControllerDeviceModel.Initialize(this, dispatcher);
            this.wirelessLineSensorDeviceModel.Initialize(this, dispatcher);
            this.bleDeviceWatcher = new BleDeviceWatcher(dispatcher, this);
            this.serialDeviceWatcher = new SerialDeviceWatcher(dispatcher, this);
            this.bleDeviceWatcher.Start();
            this.serialDeviceWatcher.Start();
            this.wirelessLineSensor.DeviceRemovedEvent += BleDeviceRemoved;
        }

        private void BleDeviceRemoved(WirelessLineSensor source, string removedDeviceId)
        {
            var ignore = this.dispatcher?.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                for (int i = 0; i < AllSupportedBleDevices.Count; ++i)
                {
                    if (AllSupportedBleDevices[i].Id == removedDeviceId)
                    {
                        if (WirelessLineSensorDeviceModel.Id == removedDeviceId)
                        {
                            WirelessLineSensorDeviceModel.DisconnectCommand.Execute(WirelessLineSensorDeviceModel);
                        }
                        AllSupportedBleDevices.RemoveAt(i);
                        if (SelectedBleDevice == null || SelectedBleDevice.Id == removedDeviceId)
                        {
                            SelectedBleDevice = AllSupportedBleDevices.Count <= 0 ? null : AllSupportedBleDevices[0];
                        }
                        break;
                    }
                }
            });
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

        public void ResetBusy(UiComponent busyComponents)
        {
            UiComponent newBusyUIComponents = (~this.busyUIComponents) & busyComponents;
            UiComponent newNotBusyUIComponents = this.busyUIComponents & (~busyComponents);

            this.busyUIComponents = busyComponents;
            UpdateBusyComponents(newBusyUIComponents, true);
            UpdateBusyComponents(newNotBusyUIComponents, false);
        }

        private void UpdateBusyComponents(UiComponent changedUIComponents, bool isBusy)
        {
            if ((changedUIComponents & UiComponent.ALL_SERIAL_DEVICES) == UiComponent.ALL_SERIAL_DEVICES)
            {
                UpdateIsAllSupportedSerialDevicesEnabled();
            }
            if ((changedUIComponents & UiComponent.ALL_BLE_DEVICES) == UiComponent.ALL_BLE_DEVICES)
            {
                UpdateIsAllSupportedBleDevicesEnabled();
            }
            if ((changedUIComponents & UiComponent.POSITION_CONTROLLER) == UiComponent.POSITION_CONTROLLER)
            {
                this.positionControllerDeviceModel.OnBusyChanged(isBusy);
            }
            if ((changedUIComponents & UiComponent.WIRELESS_LINE_SENSOR) == UiComponent.WIRELESS_LINE_SENSOR)
            {
                this.wirelessLineSensorDeviceModel.OnBusyChanged(isBusy);
            }
            if ((changedUIComponents & UiComponent.ALL_SERIAL_DEVICES) == UiComponent.ALL_SERIAL_DEVICES ||
                (changedUIComponents & UiComponent.POSITION_CONTROLLER) == UiComponent.POSITION_CONTROLLER)
            {
                SerialDeviceToggleConnectionCommand.UpdateCanExecute();
            }
            if ((changedUIComponents & UiComponent.ALL_BLE_DEVICES) == UiComponent.ALL_BLE_DEVICES ||
                (changedUIComponents & UiComponent.WIRELESS_LINE_SENSOR) == UiComponent.WIRELESS_LINE_SENSOR)
            {
                BleDeviceToggleScanningCommand.UpdateCanExecute();
                BleDeviceToggleConnectionCommand.UpdateCanExecute();
            }
            if ((changedUIComponents & UiComponent.POSITION_CONTROLLER) == UiComponent.POSITION_CONTROLLER ||
                (changedUIComponents & UiComponent.WIRELESS_LINE_SENSOR) == UiComponent.WIRELESS_LINE_SENSOR)
            {
                CollectDataCommand.UpdateCanExecute();
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
                    this.SerialDeviceToggleConnectionCommand.UpdateCanExecute();
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
                    this.BleDeviceToggleConnectionCommand.UpdateCanExecute();
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

        private void UpdateIsAllSupportedBleDevicesEnabled()
        {
            IsAllSupportedBleDevicesEnabled = !WirelessLineSensorDeviceModel.IsConnected &&
                                               AllSupportedBleDevices.Count > 0 &&
                                               (GetBusyUIComponents() & UiComponent.ALL_BLE_DEVICES) == 0;
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

                    if (this.bleDeviceScanningIndicatorVisible == Visibility.Visible)
                    {
                        BleDeviceToggleScanningCommandTitle = TITLE_SCANNING_STOP;
                    }
                    else
                    {
                        BleDeviceToggleScanningCommandTitle = TITLE_SCANNING_START;
                    }
                }
            }
        }

        public string BleDeviceToggleScanningCommandTitle
        {
            get { return this.bleDeviceToggleScanningCommandTitle; }
            private set
            {
                if (this.bleDeviceToggleScanningCommandTitle != value)
                {
                    this.bleDeviceToggleScanningCommandTitle = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(BleDeviceToggleScanningCommandTitle)));
                }
            }
        }

        public BaseCommandWithParameter<MainModel> BleDeviceToggleScanningCommand { get; }

        public BaseCommandWithParameter<MainModel> BleDeviceToggleConnectionCommand { get; }

        public void StopScanningBleDevices()
        {
            this.bleDeviceWatcher.Stop();
        }

        public void StartScanningBleDevices()
        {
            this.bleDeviceWatcher.Start();
        }

        /*
         * 
         * Others
         * 
         */

        public IPositionControllerDeviceModel PositionControllerDeviceModel { get { return this.positionControllerDeviceModel; } }

        public IWirelessLineSensorDeviceModel WirelessLineSensorDeviceModel { get { return this.wirelessLineSensorDeviceModel; } }

        public BaseCommandWithParameter<MainModel> CollectDataCommand { get; }

        public ObservableCollection<ILogEntryModel> LogEntries { get; }

        public string CollectDataCommandTitle
        {
            get { return this.collectDataCommandTitle; }
            set
            {
                if (this.collectDataCommandTitle != value)
                {
                    this.collectDataCommandTitle = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(CollectDataCommandTitle)));
                }
            }
        }

        public double StepSize
        {
            get { return this.stepSize; }
            set
            {
                if (this.stepSize != value)
                {
                    this.stepSize = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(StepSize)));
                }
            }
        }

        public string FilePathPrefix
        {
            get { return this.filePathPrefix; }
            set
            {
                if (this.filePathPrefix != value)
                {
                    this.filePathPrefix = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(FilePathPrefix)));
                }
            }
        }

        public int NumberOfSamples
        {
            get { return this.numberOfSamples; }
            set
            {
                if (this.numberOfSamples != value)
                {
                    this.numberOfSamples = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(NumberOfSamples)));
                }
            }
        }

        private void PositionControllerDeviceModelChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(PositionControllerDeviceModel.IsConnected))
            {
                UpdateIsAllSupportedSerialDevicesEnabled();
                this.CollectDataCommand.UpdateCanExecute();
            }
        }

        private void AllSupportedSerialDevicesCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            UpdateIsAllSupportedSerialDevicesEnabled();
        }

        private void WirelessLineSensorDeviceModelChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(WirelessLineSensorDeviceModel.IsConnected))
            {
                UpdateIsAllSupportedBleDevicesEnabled();
                BleDeviceToggleScanningCommand.UpdateCanExecute();
                this.CollectDataCommand.UpdateCanExecute();
            }
        }

        private void AllSupportedBleDevicesCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            UpdateIsAllSupportedBleDevicesEnabled();
        }

        /* 
         * 
         * Fields
         * 
         */

        private CoreDispatcher dispatcher;

        private UiComponent busyUIComponents;

        private SerialDeviceModel selectedSerialDevice;
        private bool isAllSupportedSerialDevicesEnabled;
        private Visibility serialDeviceScanningIndicatorVisible;

        private BleDeviceModel selectedBleDevice;
        private bool isAllSupportedBleDevicesEnabled;
        private Visibility bleDeviceScanningIndicatorVisible;
        private string bleDeviceToggleScanningCommandTitle;

        private BleDeviceWatcher bleDeviceWatcher;
        private SerialDeviceWatcher serialDeviceWatcher;

        private double stepSize;
        private int numberOfSamples;
        private string filePathPrefix;
        private string collectDataCommandTitle;

        private readonly IDataCollector dataCollector;
        private readonly IPositionController positionController;
        private readonly IWirelessLineSensor wirelessLineSensor;

        private readonly PositionControllerDeviceModel positionControllerDeviceModel;
        private readonly WirelessLineSensorDeviceModel wirelessLineSensorDeviceModel;

        private const string TITLE_SCANNING_STOP = "Stop";
        private const string TITLE_SCANNING_START = "Rescan";
    }
}
