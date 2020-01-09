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

        public string Id { get { return this.deviceInformation.Id; } }

        public bool IsDefault { get { return this.deviceInformation.IsDefault; } }

        public string DisplayName { get; set; }

        public void Update(DeviceInformationUpdate deviceInformationUpdate)
        {
            this.deviceInformation.Update(deviceInformationUpdate);
            // TODO update properties
        }

        private readonly DeviceInformation deviceInformation;
    }
}
