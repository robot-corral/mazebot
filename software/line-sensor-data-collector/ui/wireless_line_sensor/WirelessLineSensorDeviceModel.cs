using System.ComponentModel;
using System.Threading.Tasks;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public class WirelessLineSensorDeviceModel : IWirelessLineSensorDeviceModel
    {
        public WirelessLineSensorDeviceModel(MainModel mainModel, IWirelessLineSensor wirelessLineSensor)
        {
        }

        public event PropertyChangedEventHandler PropertyChanged;

        // TODO
        public string Id { get; private set; }

        public bool IsConnected
        {
            get { return this.isConnected; }
            private set
            {
                if (this.isConnected != value)
                {
                    this.isConnected = value;
                    PropertyChanged.Invoke(this, new PropertyChangedEventArgs(nameof(IsConnected)));
                }
            }
        }

        public int PacketsTotalNumber
        {
            get { return this.packetsTotalNumber; }
            private set
            {
                if (this.packetsTotalNumber != value)
                {
                    this.packetsTotalNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PacketsTotalNumber)));
                }
            }
        }

        public int PacketsCrcFailuresNumber
        {
            get { return this.packetsCrcFailuresNumber; }
            private set
            {
                if (this.packetsCrcFailuresNumber != value)
                {
                    this.packetsCrcFailuresNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PacketsCrcFailuresNumber)));
                }
            }
        }

        public int PacketsFailuresNumber
        {
            get { return this.packetsFailuresNumber; }
            private set
            {
                if (this.packetsFailuresNumber != value)
                {
                    this.packetsFailuresNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(PacketsFailuresNumber)));
                }
            }
        }

        public Task Disconnect()
        {
            // TODO pkrupets
            return Task.CompletedTask;
        }

        // TODO pkrupets
        public BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetLineSensorCommand { get; }

        // TODO pkrupets
        public BaseCommandWithParameter<IWirelessLineSensorDeviceModel> ResetWirelessLineSensorCommand { get; }

        private bool isConnected;
        private int packetsTotalNumber;
        private int packetsFailuresNumber;
        private int packetsCrcFailuresNumber;
    }
}
