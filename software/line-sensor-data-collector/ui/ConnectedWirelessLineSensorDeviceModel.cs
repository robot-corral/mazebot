namespace line_sensor.data_collector.ui
{
    public class ConnectedWirelessLineSensorDeviceModel
    {
        public ConnectedWirelessLineSensorDeviceModel()
        {
            Title = "Connect";
        }

        public string Id { get; set; }

        public string Title { get; set; }

        public bool IsConnected { get; set; }
    }
}
