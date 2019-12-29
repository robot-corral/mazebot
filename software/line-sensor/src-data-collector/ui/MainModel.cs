using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows.Input;
using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;
using Windows.UI.Core;
using Windows.UI.Xaml;

namespace line_sensor.data_collector.ui
{
    public class MainModel : INotifyPropertyChanged
    {
        public MainModel(CoreDispatcher dispatcher)
        {
            this.dispatcher = dispatcher ?? throw new ArgumentNullException(nameof(dispatcher));

            AllSupportedSerialDevicesEnabled = true;
            SerialDeviceConnectCommandTitle = TITLE_CONNECT;
            SerialDeviceScanningIndicatorVisible = Visibility.Collapsed;

            AllSupportedBleDevicesEnabled = true;
            BleDeviceConnectCommandTitle = TITLE_CONNECT;
            BleDeviceScanningIndicatorVisible = Visibility.Collapsed;
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void Initialize()
        {
            if (this.serialDeviceWatcher == null)
            {
                this.serialDeviceWatcher = DeviceInformation.CreateWatcher(SerialDevice.GetDeviceSelector());
                this.serialDeviceWatcher.Added += OnSerialDeviceAdded;
                this.serialDeviceWatcher.Updated += OnSerialDeviceUpdated;
                this.serialDeviceWatcher.Removed += OnSerialDeviceRemoved;
                this.serialDeviceWatcher.EnumerationCompleted += OnSerialDeviceEnumerationComplete;

                SerialDeviceScanningIndicatorVisible = Visibility.Visible;

                this.serialDeviceWatcher.Start();
            }

            if (this.bleDeviceWatcher == null)
            {
                // property strings are documented here https://msdn.microsoft.com/en-us/library/windows/desktop/ff521659(v=vs.85).aspx
                string[] requestedProperties = { "System.Devices.Aep.DeviceAddress", "System.Devices.Aep.IsConnected", "System.Devices.Aep.Bluetooth.Le.IsConnectable" };

                // guid in this expression is Bluetooth LE protocol ID
                string aqsAllBluetoothLEDevices = "(System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\")";

                this.bleDeviceWatcher = DeviceInformation.CreateWatcher(aqsAllBluetoothLEDevices,
                                                                        requestedProperties,
                                                                        DeviceInformationKind.AssociationEndpoint);

                this.bleDeviceWatcher.Added += OnBleDeviceAdded;
                this.bleDeviceWatcher.Removed += OnBleDeviceRemoved;
                this.bleDeviceWatcher.Updated += OnBleDeviceUpdated;
                this.bleDeviceWatcher.EnumerationCompleted += OnBleDeviceEnumerationComplete;

                BleDeviceScanningIndicatorVisible = Visibility.Visible;

                this.bleDeviceWatcher.Start();
            }
        }

        public ObservableCollection<SerialDeviceModel> AllSupportedSerialDevices { get; } = new ObservableCollection<SerialDeviceModel>();

        public SerialDeviceModel SelectedSerialDevice
        {
            get { return this.selectedSerialDevice; }
            set
            {
                if (this.selectedSerialDevice != value)
                {
                    this.selectedSerialDevice = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedSerialDevice)));
                }
            }
        }

        public bool AllSupportedSerialDevicesEnabled
        {
            get { return this.allSupportedSerialDevicesEnabled; }
            set
            {
                if (this.allSupportedSerialDevicesEnabled != value)
                {
                    this.allSupportedSerialDevicesEnabled = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(AllSupportedSerialDevicesEnabled)));
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

        public string SerialDeviceConnectCommandTitle
        {
            get { return this.serialDeviceConnectCommandTitle; }
            set
            {
                if (this.serialDeviceConnectCommandTitle != value)
                {
                    this.serialDeviceConnectCommandTitle = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SerialDeviceConnectCommandTitle)));
                }
            }
        }

        public ICommand SerialDeviceConnectCommand { get; set; }

        public ObservableCollection<BleDeviceModel> AllSupportedBleDevices { get; } = new ObservableCollection<BleDeviceModel>();

        public BleDeviceModel SelectedBleDevice
        {
            get { return this.selectedBleDevice; }
            set
            {
                if (this.selectedBleDevice != value)
                {
                    this.selectedBleDevice = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedBleDevice)));
                }
            }
        }

        public bool AllSupportedBleDevicesEnabled
        {
            get { return this.allSupportedBleDevicesEnabled; }
            set
            {
                if (this.allSupportedBleDevicesEnabled != value)
                {
                    this.allSupportedBleDevicesEnabled = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(AllSupportedBleDevicesEnabled)));
                }
            }
        }

        public Visibility BleDeviceScanningIndicatorVisible
        {
            get { return this.bleDeviceLoadingVisible; }
            set
            {
                if (this.bleDeviceLoadingVisible != value)
                {
                    this.bleDeviceLoadingVisible = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(BleDeviceScanningIndicatorVisible)));
                }
            }
        }

        public string BleDeviceConnectCommandTitle
        {
            get { return this.bleDeviceConnectCommandTitle; }
            set
            {
                if (this.bleDeviceConnectCommandTitle != value)
                {
                    this.bleDeviceConnectCommandTitle = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(BleDeviceConnectCommandTitle)));
                }
            }
        }

        public ICommand BleDeviceConnectCommand { get; set; }

        private T GetNewSelectedItem<T>(ObservableCollection<T> allItems)
            where T : class, IDeviceModel
        {
            if (allItems.Count <= 0)
            {
                return null;
            }
            return allItems.Where(x => x.IsDefault).FirstOrDefault() ?? allItems[0];
        }

        private void OnSerialDeviceAdded(DeviceWatcher sender, DeviceInformation deviceInformation)
        {
            if (IsLooingLikeStmStLinkDevice(deviceInformation))
            {
                var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    if (sender == this.serialDeviceWatcher &&
                        this.allSerialDevices.TryAdd(deviceInformation.Id, deviceInformation))
                    {
                        AllSupportedSerialDevices.Add(new SerialDeviceModel(deviceInformation));
                        if (AllSupportedSerialDevices.Count == 1)
                        {
                            SelectedSerialDevice = AllSupportedSerialDevices[0];
                        }
                    }
                });
            }
        }

        private bool IsLooingLikeStmStLinkDevice(DeviceInformation deviceInformation)
        {
            return deviceInformation != null &&
                   deviceInformation.IsEnabled &&
                   deviceInformation.Name?.ToLower() == STM32_ST_LINK_NAME &&
                   deviceInformation.Id != null &&
                   deviceInformation.Id.ToLower().Contains(STM32_ST_LINK_USB_ID);
        }

        private void OnSerialDeviceEnumerationComplete(DeviceWatcher sender, object args)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SerialDeviceScanningIndicatorVisible = Visibility.Collapsed;
            });
        }

        private void OnSerialDeviceUpdated(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (sender == this.serialDeviceWatcher && AllSupportedSerialDevices != null)
                {
                    for (int i = 0; i < AllSupportedSerialDevices.Count; ++i)
                    {
                        if (AllSupportedSerialDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            AllSupportedSerialDevices[i].Update(deviceInformationUpdate);
                            break;
                        }
                    }
                }
            });
        }

        private void OnSerialDeviceRemoved(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (sender == this.serialDeviceWatcher && this.allSerialDevices.Remove(deviceInformationUpdate.Id) && AllSupportedSerialDevices != null)
                {
                    for (int i = 0; i < AllSupportedSerialDevices.Count; ++i)
                    {
                        if (AllSupportedSerialDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            AllSupportedSerialDevices.RemoveAt(i);
                            if (SelectedSerialDevice.Id == deviceInformationUpdate.Id)
                            {
                                // TODO disconnect
                                SelectedSerialDevice = GetNewSelectedItem(AllSupportedSerialDevices);
                                SerialDeviceConnectCommandTitle = TITLE_CONNECT;
                            }
                            break;
                        }
                    }
                }
            });
        }

        private void OnBleDeviceAdded(DeviceWatcher sender, DeviceInformation deviceInformation)
        {
            if (IsLooingLikeLineSensorDataDevice(deviceInformation))
            {
                var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    if (sender == this.bleDeviceWatcher &&
                        this.allBleDevices.TryAdd(deviceInformation.Id, deviceInformation))
                    {
                        AllSupportedBleDevices.Add(new BleDeviceModel(deviceInformation));
                        if (AllSupportedBleDevices.Count == 1)
                        {
                            SelectedBleDevice = AllSupportedBleDevices[0];
                        }
                    }
                });
            }
        }

        private bool IsLooingLikeLineSensorDataDevice(DeviceInformation deviceInformation)
        {
            return deviceInformation != null &&
                   deviceInformation.Name?.ToLower() == LINE_SENSOR_DATA;
        }

        private void OnBleDeviceEnumerationComplete(DeviceWatcher sender, object args)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                BleDeviceScanningIndicatorVisible = Visibility.Collapsed;
            });
        }

        private void OnBleDeviceUpdated(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
        }

        private void OnBleDeviceRemoved(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
        }

        private DeviceWatcher serialDeviceWatcher;
        private string serialDeviceConnectCommandTitle;
        private SerialDeviceModel selectedSerialDevice;
        private ObservableCollection<SerialDeviceModel> allSerialDeviceModels;

        private DeviceWatcher bleDeviceWatcher;
        private BleDeviceModel selectedBleDevice;
        private bool allSupportedBleDevicesEnabled;
        private Visibility bleDeviceLoadingVisible;
        private string bleDeviceConnectCommandTitle;
        private ObservableCollection<BleDeviceModel> allBleDeviceModels;
        private bool allSupportedSerialDevicesEnabled;
        private Visibility serialDeviceScanningIndicatorVisible;
        private readonly CoreDispatcher dispatcher;
        private readonly IDictionary<string, DeviceInformation> allBleDevices = new Dictionary<string, DeviceInformation>();
        private readonly IDictionary<string, DeviceInformation> allSerialDevices = new Dictionary<string, DeviceInformation>();

        private const string TITLE_CONNECT = "Connect";

        private const string LINE_SENSOR_DATA = "line_sensor_data";

        private const string STM32_ST_LINK_NAME = "stm32 stlink";
        private const string STM32_ST_LINK_USB_ID = "usb#vid_0483&pid_374b&mi_02";
    }
}
