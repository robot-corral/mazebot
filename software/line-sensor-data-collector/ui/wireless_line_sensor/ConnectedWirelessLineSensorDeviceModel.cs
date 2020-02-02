using line_sensor.data_collector.logic;
using System.ComponentModel;
using System.Threading.Tasks;
using System.Windows.Input;

namespace line_sensor.data_collector.ui.wireless_line_sensor
{
    public class WirelessLineSensorDeviceModel : IWirelessLineSensorDeviceModel
    {
        public WirelessLineSensorDeviceModel(MainModel mainModel, IWirelessLineSensor wirelessLineSensor)
        {
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public string Id { get; set; }

        public bool IsConnected
        {
            get { return this.isConnected; }
            set
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
            set
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
            set
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
            set
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
            return Task.CompletedTask;
        }

        // TODO pkrupets
        public ICommand ResetLineSensorCommand { get; }

        // TODO pkrupets
        public ICommand ResetWirelessLineSensorCommand { get; }

        private bool isConnected;
        private int packetsTotalNumber;
        private int packetsFailuresNumber;
        private int packetsCrcFailuresNumber;
    }
}
