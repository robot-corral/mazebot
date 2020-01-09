using System.ComponentModel;

namespace line_sensor.data_collector.ui
{
    public class ConnectedPositionControllerDeviceModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public string Id { get; set; }

        public string Title
        {
            get
            {
                return IsConnected ? "Disconnect" : "Connect";
            }
        }

        public bool IsConnected
        {
            get { return this.isConnected; }
            set
            {
                if (this.isConnected != value)
                {
                    this.isConnected = value;
                    PropertyChanged.Invoke(this, new PropertyChangedEventArgs(nameof(IsConnected)));
                    PropertyChanged.Invoke(this, new PropertyChangedEventArgs(nameof(Title)));
                }
            }
        }

        private bool isConnected;
    }
}
