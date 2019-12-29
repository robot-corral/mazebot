using System;
using System.Text.RegularExpressions;
using Windows.Devices.Enumeration;

namespace line_sensor.data_collector.ui
{
    public class SerialDeviceModel : IDeviceModel
    {
        public SerialDeviceModel(DeviceInformation deviceInformation)
        {
            this.deviceInformation = deviceInformation ?? throw new ArgumentNullException(nameof(deviceInformation));
            SetDisplayName(deviceInformation.Id, null);
        }

        private void SetDisplayName(string id, string portName)
        {
            Match match = DEVICE_UNIQUE_ID_REGEX.Match(id);

            if (match.Success)
            {
                DisplayName = "ST-Link / USB " + match.Groups[1].Value + (portName == null ? "" : $" ({portName})");
            }
            else
            {
                DisplayName = $"Invalid device: {id}";
            }
        }

        public string DisplayName { get; set; }

        public string Id { get { return this.deviceInformation.Id; } }

        public bool IsDefault { get { return this.deviceInformation.IsDefault; } }

        public void Update(DeviceInformationUpdate deviceInformationUpdate)
        {
            this.deviceInformation.Update(deviceInformationUpdate);
            // TODO update properties
        }

        private readonly DeviceInformation deviceInformation;

        private static readonly Regex DEVICE_UNIQUE_ID_REGEX = new Regex(".*MI_02#([^#]+)#\\{.*", RegexOptions.Compiled | RegexOptions.IgnoreCase | RegexOptions.Singleline);
    }
}
