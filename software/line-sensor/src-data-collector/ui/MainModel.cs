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
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui
{
    public class MainModel : INotifyPropertyChanged
    {
        public MainModel(CoreDispatcher dispatcher)
        {
            this.dispatcher = dispatcher ?? throw new ArgumentNullException(nameof(dispatcher));

            this.serialDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.ConnectedSerialDeviceModel = new ConnectedSerialDeviceModel();
            this.AllSupportedSerialDevices = new ObservableCollection<SerialDeviceModel>();
            this.serialDeviceConnectDisconnectCommand = new SerialDeviceConnectDisconnectCommand();

            this.ConnectedBleDeviceModel = new ConnectedBleDeviceModel();
            this.bleDeviceScanningIndicatorVisible = Visibility.Collapsed;
            this.AllSupportedBleDevices = new ObservableCollection<BleDeviceModel>();
            this.bleDeviceConnectDisconnectCommand = new BleDeviceConnectDisconnectCommand();
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

        public ConnectedSerialDeviceModel ConnectedSerialDeviceModel { get; }

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

        public ConnectedBleDeviceModel ConnectedBleDeviceModel { get; }

        public ICommand BleDeviceConnectCommand { get { return this.bleDeviceConnectDisconnectCommand; } }

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
                if (sender == this.serialDeviceWatcher)
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
                if (sender == this.serialDeviceWatcher && this.allSerialDevices.Remove(deviceInformationUpdate.Id))
                {
                    for (int i = 0; i < AllSupportedSerialDevices.Count; ++i)
                    {
                        if (AllSupportedSerialDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            AllSupportedSerialDevices.RemoveAt(i);
                            if (this.ConnectedSerialDeviceModel.Id == deviceInformationUpdate.Id)
                            {
                                // disconnect
                                this.serialDeviceConnectDisconnectCommand.Execute(this);
                            }
                            if (SelectedSerialDevice.Id == deviceInformationUpdate.Id)
                            {
                                SelectedSerialDevice = GetNewSelectedItem(AllSupportedSerialDevices);
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
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (sender == this.bleDeviceWatcher)
                {
                    for (int i = 0; i < AllSupportedBleDevices.Count; ++i)
                    {
                        if (AllSupportedBleDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            AllSupportedBleDevices[i].Update(deviceInformationUpdate);
                            break;
                        }
                    }
                }
            });
        }

        private void OnBleDeviceRemoved(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (sender == this.bleDeviceWatcher && this.allBleDevices.Remove(deviceInformationUpdate.Id))
                {
                    for (int i = 0; i < AllSupportedBleDevices.Count; ++i)
                    {
                        if (AllSupportedBleDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            AllSupportedBleDevices.RemoveAt(i);
                            if (this.ConnectedBleDeviceModel.Id == deviceInformationUpdate.Id)
                            {
                                // disconnect
                                this.bleDeviceConnectDisconnectCommand.Execute(this);
                            }
                            if (SelectedBleDevice.Id == deviceInformationUpdate.Id)
                            {
                                SelectedBleDevice = GetNewSelectedItem(AllSupportedBleDevices);
                            }
                            break;
                        }
                    }
                }
            });
        }

        private DeviceWatcher serialDeviceWatcher;
        private SerialDeviceModel selectedSerialDevice;
        private Visibility serialDeviceScanningIndicatorVisible;

        private DeviceWatcher bleDeviceWatcher;
        private BleDeviceModel selectedBleDevice;
        private Visibility bleDeviceScanningIndicatorVisible;

        private readonly CoreDispatcher dispatcher;

        private readonly BaseCommandWithParameter<MainModel> bleDeviceConnectDisconnectCommand;
        private readonly IDictionary<string, DeviceInformation> allBleDevices = new Dictionary<string, DeviceInformation>();

        private readonly BaseCommandWithParameter<MainModel> serialDeviceConnectDisconnectCommand;
        private readonly IDictionary<string, DeviceInformation> allSerialDevices = new Dictionary<string, DeviceInformation>();

        private const string LINE_SENSOR_DATA = "line_sensor_data";

        private const string STM32_ST_LINK_NAME = "stm32 stlink";
        private const string STM32_ST_LINK_USB_ID = "usb#vid_0483&pid_374b&mi_02";
    }
}
