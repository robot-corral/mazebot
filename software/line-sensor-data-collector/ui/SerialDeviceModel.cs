using System;
using System.ComponentModel;
using System.Text.RegularExpressions;

using Windows.Devices.Enumeration;

namespace line_sensor.data_collector.ui
{
    public class SerialDeviceModel : IDeviceModel, INotifyPropertyChanged
    {
        public SerialDeviceModel(DeviceInformation deviceInformation)
        {
            this.deviceInformation = deviceInformation ?? throw new ArgumentNullException(nameof(deviceInformation));
            Id = deviceInformation.Id;
            IsDefault = deviceInformation.IsDefault;
            SetDisplayName(deviceInformation.Id);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void SetDisplayName(string id)
        {
            Match match = DEVICE_UNIQUE_ID_REGEX.Match(id);

            if (match.Success)
            {
                DisplayName = "ST-Link / USB " + match.Groups[1].Value;
            }
            else
            {
                DisplayName = $"Invalid device: {id}";
            }
        }

        public string Id { get; set; }

        public string DisplayName
        {
            get { return this.displayName; }
            set
            {
                if (this.displayName != value)
                {
                    this.displayName = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(DisplayName)));
                }
            }
        }

        public bool IsDefault { get; set; }

        public void Update(DeviceInformationUpdate deviceInformationUpdate)
        {
            this.deviceInformation.Update(deviceInformationUpdate);

            if (deviceInformationUpdate.Properties.ContainsKey(nameof(DeviceInformation.Id)))
            {
                Id = deviceInformation.Id;
            }
            if (deviceInformationUpdate.Properties.ContainsKey(nameof(DeviceInformation.Name)))
            {
                DisplayName = deviceInformation.Name;
            }
            if (deviceInformationUpdate.Properties.ContainsKey(nameof(DeviceInformation.IsDefault)))
            {
                IsDefault = deviceInformation.IsDefault;
            }
        }

        private string displayName;

        private readonly DeviceInformation deviceInformation;

        private static readonly Regex DEVICE_UNIQUE_ID_REGEX = new Regex(".*MI_02#([^#]+)#\\{.*", RegexOptions.Compiled | RegexOptions.IgnoreCase | RegexOptions.Singleline);
    }
}
