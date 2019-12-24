using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Storage.Streams;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace src_data_collector
{
    public sealed partial class MainPage : Page, INotifyPropertyChanged
    {
        public MainPage()
        {
            DataContext = this;
            this.InitializeComponent();
        }

        public event PropertyChangedEventHandler PropertyChanged = delegate { };

        public string ResultText
        {
            get { return this.resultText; }
            private set
            {
                if (this.resultText != value)
                {
                    this.resultText = value;
                    PropertyChanged(this, new PropertyChangedEventArgs(nameof(ResultText)));
                }
            }
        }

        private async void Button_ConnectToBle(object sender, RoutedEventArgs e)
        {
            if (this.bluetoothLeDevice == null)
            {
                this.bluetoothLeDevice = await BluetoothLEDevice.FromBluetoothAddressAsync(0x80E12607CDF5);
            }

            GattDeviceServicesResult result = await this.bluetoothLeDevice.GetGattServicesAsync(BluetoothCacheMode.Uncached);

            IEnumerable<GattDeviceService> services = result.Services;
            GattCharacteristicsResult characteristics = await services?.Where(x => x.Uuid == new Guid("0000fe40-cc7a-482a-984a-7f2ed5b3e58f"))
                                                                      ?.FirstOrDefault()
                                                                      ?.GetCharacteristicsAsync();

            if (characteristics != null)
            {
                GattReadResult readResult = await characteristics.Characteristics?.Where(x => x.Uuid == new Guid("0000fe43-8e22-4541-9d4c-21edae82ed19"))
                                                                                 ?.FirstOrDefault()
                                                                                 ?.ReadValueAsync();
                byte[] bytes = new byte[readResult.Value.Length];
                using (DataReader dr = DataReader.FromBuffer(readResult.Value))
                {
                    dr.ReadBytes(bytes);
                }

                ResultText = Encoding.ASCII.GetString(bytes);
            }
            return;
        }

        private string resultText;
        private BluetoothLEDevice bluetoothLeDevice;
    }
}
