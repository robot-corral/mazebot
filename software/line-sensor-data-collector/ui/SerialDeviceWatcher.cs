using System;
using System.Collections.Generic;
using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;
using Windows.UI.Core;
using Windows.UI.Xaml;

namespace line_sensor.data_collector.ui
{
    public class SerialDeviceWatcher
    {
        public SerialDeviceWatcher(CoreDispatcher dispatcher, MainModel model)
        {
            this.model = model ?? throw new ArgumentNullException(nameof(model));
            this.dispatcher = dispatcher ?? throw new ArgumentNullException(nameof(dispatcher));
        }

        public void Start()
        {
            if (this.serialDeviceWatcher == null)
            {
                this.serialDeviceWatcher = DeviceInformation.CreateWatcher(SerialDevice.GetDeviceSelector());
                this.serialDeviceWatcher.Added += OnSerialDeviceAdded;
                this.serialDeviceWatcher.Updated += OnSerialDeviceUpdated;
                this.serialDeviceWatcher.Removed += OnSerialDeviceRemoved;
                this.serialDeviceWatcher.EnumerationCompleted += OnSerialDeviceEnumerationComplete;

                this.model.SerialDeviceScanningIndicatorVisible = Visibility.Visible;

                this.serialDeviceWatcher.Start();
            }
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
                        this.model.AllSupportedSerialDevices.Add(new SerialDeviceModel(deviceInformation));
                        if (this.model.AllSupportedSerialDevices.Count == 1)
                        {
                            this.model.SelectedSerialDevice = this.model.AllSupportedSerialDevices[0];
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
                this.model.SerialDeviceScanningIndicatorVisible = Visibility.Collapsed;
            });
        }

        private void OnSerialDeviceUpdated(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
            var ignoreContinuation = this.dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (sender == this.serialDeviceWatcher)
                {
                    for (int i = 0; i < this.model.AllSupportedSerialDevices.Count; ++i)
                    {
                        if (this.model.AllSupportedSerialDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            this.model.AllSupportedSerialDevices[i].Update(deviceInformationUpdate);
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
                    for (int i = 0; i < this.model.AllSupportedSerialDevices.Count; ++i)
                    {
                        if (this.model.AllSupportedSerialDevices[i].Id == deviceInformationUpdate.Id)
                        {
                            if (this.model.PositionControllerDeviceModel.Id == deviceInformationUpdate.Id)
                            {
                                this.model.PositionControllerDeviceModel.DisconnectCommand.Execute(this.model.PositionControllerDeviceModel);
                            }
                            if (this.model.SelectedSerialDevice != null && this.model.SelectedSerialDevice.Id == deviceInformationUpdate.Id)
                            {
                                this.model.SelectedSerialDevice = this.model.AllSupportedSerialDevices.Count <= 0 ? null : this.model.AllSupportedSerialDevices[0];
                            }
                            this.model.AllSupportedSerialDevices.RemoveAt(i);
                            break;
                        }
                    }
                }
            });
        }

        private DeviceWatcher serialDeviceWatcher;

        private readonly MainModel model;
        private readonly CoreDispatcher dispatcher;
        private readonly IDictionary<string, DeviceInformation> allSerialDevices = new Dictionary<string, DeviceInformation>();

        private const string STM32_ST_LINK_NAME = "stm32 stlink";
        private const string STM32_ST_LINK_USB_ID = "usb#vid_0483&pid_374b&mi_02";
    }
}
