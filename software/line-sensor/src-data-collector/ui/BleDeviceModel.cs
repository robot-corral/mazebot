using System;
using Windows.Devices.Enumeration;

namespace line_sensor.data_collector.ui
{
    public class BleDeviceModel : IDeviceModel
    {
        public BleDeviceModel(DeviceInformation deviceInformation)
        {
            this.deviceInformation = deviceInformation ?? throw new ArgumentNullException(nameof(deviceInformation));
            DisplayName = deviceInformation.Name;
        }

        public bool IsDefault { get { return this.deviceInformation.IsDefault; } }

        public string DisplayName { get; set; }

        private readonly DeviceInformation deviceInformation;
    }
}
