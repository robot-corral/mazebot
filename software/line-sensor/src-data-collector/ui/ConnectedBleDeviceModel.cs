namespace line_sensor.data_collector.ui
{
    public class ConnectedBleDeviceModel
    {
        public ConnectedBleDeviceModel()
        {
            Title = "Connect";
        }

        public string Id { get; set; }

        public string Title { get; set; }

        public bool IsConnected { get; set; }
    }
}
