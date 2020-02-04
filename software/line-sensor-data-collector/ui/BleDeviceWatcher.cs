using System;
using System.Collections.Generic;

using Windows.Devices.Enumeration;
using Windows.UI.Core;
using Windows.UI.Xaml;

namespace line_sensor.data_collector.ui
{
    public class BleDeviceWatcher
    {
        public BleDeviceWatcher(CoreDispatcher dispatcher, MainModel model)
        {
            this.model = model ?? throw new ArgumentNullException(nameof(model));
            this.dispatcher = dispatcher ?? throw new ArgumentNullException(nameof(dispatcher));
        }

        public void Start()
        {
            // property strings are documented here https://msdn.microsoft.com/en-us/library/windows/desktop/ff521659(v=vs.85).aspx
            string[] requestedProperties = { "System.Devices.Aep.DeviceAddress", "System.Devices.Aep.IsConnected", "System.Devices.Aep.Bluetooth.Le.IsConnectable" };

            // guid in this expression is Bluetooth LE protocol ID
            string aqsAllBluetoothLEDevices = "(System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\")";

            this.bleDeviceWatcher = DeviceInformation.CreateWatcher(aqsAllBluetoothLEDevices,
                                                                    requestedProperties,
                                                                    DeviceInformationKind.AssociationEndpoint);

            this.allBleDevices.Clear();
            this.model.AllSupportedBleDevices.Clear();

            this.bleDeviceWatcher.Added += OnBleDeviceAdded;
            this.bleDeviceWatcher.Removed += OnBleDeviceRemoved;
            this.bleDeviceWatcher.Updated += OnBleDeviceUpdated;
            this.bleDeviceWatcher.EnumerationCompleted += OnBleDeviceEnumerationComplete;

            this.model.BleDeviceScanningIndicatorVisible = Visibility.Visible;

            this.bleDeviceWatcher.Start();
        }

        public void Stop()
        {
            if (this.bleDeviceWatcher != null && this.bleDeviceWatcher.Status == DeviceWatcherStatus.Started)
            {
                this.bleDeviceWatcher.Stop();
                this.bleDeviceWatcher = null;
                this.model.BleDeviceScanningIndicatorVisible = Visibility.Collapsed;
            }
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
                        this.model.AllSupportedBleDevices.Add(new BleDeviceModel(deviceInformation));
                        if (this.model.AllSupportedBleDevices.Count == 1)
                        {
                            this.model.SelectedBleDevice = this.model.AllSupportedBleDevices[0];
                        }
                    }
                });
            }
        }

        private static bool IsLooingLikeLineSensorDataDevice(DeviceInformation deviceInformation)
        {
            return deviceInformation != null &&
                   deviceInformation.Name?.ToLower() == DEVICE_NAME_LINE_SENSOR_DATA;
        }

        private void OnBleDeviceEnumerationComplete(DeviceWatcher sender, object args)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (sender == this.bleDeviceWatcher)
                {
                    this.model.BleDeviceScanningIndicatorVisible = Visibility.Collapsed;
                }
            });
        }

        private void OnBleDeviceUpdated(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (sender == this.bleDeviceWatcher)
                {
                    for (int i = 0; i < this.model.AllSupportedBleDevices.Count; ++i)
                    {
                        if (this.model.AllSupportedBleDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            this.model.AllSupportedBleDevices[i].Update(deviceInformationUpdate);
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
                if (sender == this.bleDeviceWatcher && this.allBleDevices.ContainsKey(deviceInformationUpdate.Id))
                {
                    bool removeDevice = true;

                    for (int i = 0; i < this.model.AllSupportedBleDevices.Count; ++i)
                    {
                        if (this.model.AllSupportedBleDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            if (this.model.AllSupportedBleDevices[i].CanBeDeletedByWatcher)
                            {
                                if (this.model.WirelessLineSensorDeviceModel.Id == deviceInformationUpdate.Id)
                                {
                                    this.model.WirelessLineSensorDeviceModel.DisconnectCommand.Execute(this.model.WirelessLineSensorDeviceModel);
                                }
                                this.model.AllSupportedBleDevices.RemoveAt(i);
                                if (this.model.SelectedBleDevice == null || this.model.SelectedBleDevice.Id == deviceInformationUpdate.Id)
                                {
                                    this.model.SelectedBleDevice = this.model.AllSupportedBleDevices.Count <= 0 ? null : this.model.AllSupportedBleDevices[0];
                                }
                            }
                            else
                            {
                                removeDevice = false;
                            }
                            break;
                        }
                    }
                    if (removeDevice)
                    {
                        this.allBleDevices.Remove(deviceInformationUpdate.Id);
                    }
                }
            });
        }

        private DeviceWatcher bleDeviceWatcher;

        private readonly MainModel model;
        private readonly CoreDispatcher dispatcher;
        private readonly IDictionary<string, DeviceInformation> allBleDevices = new Dictionary<string, DeviceInformation>();

        private const string DEVICE_NAME_LINE_SENSOR_DATA = "line_sensor_data";
    }
}
