using System;
using System.Linq;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.logic
{
    public class WirelessLineSensor : IWirelessLineSensor
    {
        public WirelessLineSensor(ILogger logger)
        {
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
        }

        public event DeviceRemovedEvent DeviceRemovedEvent;

        public bool IsConnected { get; private set; }

        public async Task<bool> TryToConnect(string deviceId)
        {
            if (string.IsNullOrWhiteSpace(deviceId))
            {
                this.logger.Error($"deviceId argument is null or empty");
                return false;
            }

            bool success = false;

            try
            {
                GattDeviceService service = null;
                GattCharacteristic responseCharacteristic = null;
                BluetoothLEDevice bluetoothLeDevice = await BluetoothLEDevice.FromIdAsync(deviceId)
                                                                             .AsTask()
                                                                             .ConfigureAwait(false);

                if (bluetoothLeDevice == null)
                {
                    this.logger.Error($"failed to create BLE device with id = '{deviceId}'");
                    return false;
                }

                this.bluetoothLeDevice = bluetoothLeDevice;
                bluetoothLeDevice.ConnectionStatusChanged += OnConnectionStatusChanged;

                DeviceAccessStatus deviceStatus = await bluetoothLeDevice.RequestAccessAsync()
                                                                         .AsTask()
                                                                         .ConfigureAwait(false);

                if (deviceStatus != DeviceAccessStatus.Allowed)
                {
                    this.logger.Error($"failed to get access to BLE device with id = '{deviceId}'");
                    return false;
                }

                // the following method connects to BLE device (and will call OnConnectionStatusChanged)

                GattDeviceServicesResult result = await bluetoothLeDevice.GetGattServicesAsync(BluetoothCacheMode.Uncached)
                                                                         .AsTask()
                                                                         .ConfigureAwait(false);

                if (result.Status == GattCommunicationStatus.Success)
                {
                    service = result.Services.Where(s => s.Uuid == SERVICE_GUID)
                                             .FirstOrDefault();

                    if (service == null)
                    {
                        this.logger.Error($"BLE device with id = '{deviceId}' doesn't have '{SERVICE_GUID}' service");
                        return false;
                    }

                    GattOpenStatus status = await service.OpenAsync(GattSharingMode.Exclusive)
                                                         .AsTask()
                                                         .ConfigureAwait(false);

                    if (status != GattOpenStatus.Success)
                    {
                        this.logger.Error($"failed to open '{SERVICE_GUID}' service on BLE device with id = '{deviceId}', result = '{status}'");
                        return false;
                    }

                    GattCharacteristicsResult characteristicsResult = characteristicsResult = await service.GetCharacteristicsForUuidAsync(RESPONSE_CHARACTERISTICS_GUID)
                                                                                                           .AsTask()
                                                                                                           .ConfigureAwait(false);

                    if (characteristicsResult.Status != GattCommunicationStatus.Success)
                    {
                        this.logger.Error($"failed to get '{RESPONSE_CHARACTERISTICS_GUID}' characteristic from '{SERVICE_GUID}' " +
                                          $"service on BLE device with id = '{deviceId}', result = '{characteristicsResult.Status}', protocol error = {characteristicsResult.ProtocolError}");
                        return false;
                    }

                    responseCharacteristic = characteristicsResult.Characteristics.FirstOrDefault();

                    if (responseCharacteristic == null)
                    {
                        this.logger.Error($"'{RESPONSE_CHARACTERISTICS_GUID}' characteristic doesn't seem to have any characteristics in '{SERVICE_GUID}' service on BLE device with id = '{deviceId}'");
                        return false;
                    }
                }
                else
                {
                    this.logger.Error($"failed to retreive services provided by BLE device with id = '{deviceId}', result = '{result.Status}', protocol error = '{result.ProtocolError}'");
                    return false;
                }

                this.service = service;
                this.responseCharacteristic = responseCharacteristic;
                this.responseCharacteristic.ValueChanged += OnCharacteristicValueChanged;

                IsConnected = true;

                success = true;

                return true;
            }
            catch (Exception e)
            {
                this.logger.Error(e, "unexpected exception while connecting to BLE device");
                return false;
            }
            finally
            {
                if (!success)
                {
                    // TODO move to disconnect

                    if (this.responseCharacteristic != null)
                    {
                        this.responseCharacteristic.ValueChanged -= OnCharacteristicValueChanged;
                        this.responseCharacteristic = null;
                    }

                    if (this.service != null)
                    {
                        this.service.Dispose();
                        this.service = null;
                    }

                    this.bluetoothLeDevice.ConnectionStatusChanged -= OnConnectionStatusChanged;
                    this.bluetoothLeDevice.Dispose();
                    this.bluetoothLeDevice = null;
                }
            }
        }

        public async Task<bool> StartQueryingLineSensor()
        {
            GattWriteResult result = await this.responseCharacteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(GattClientCharacteristicConfigurationDescriptorValue.Notify)
                                                                      .AsTask()
                                                                      .ConfigureAwait(false);
            if (result.Status != GattCommunicationStatus.Success)
            {
                this.logger.Error($"failed to enable notification for '{RESPONSE_CHARACTERISTICS_GUID}' charactersitic for '{SERVICE_GUID}' " +
                                  $"service on BLE device with id = '{this.bluetoothLeDevice.DeviceId}', result = '{result.Status}', protocol error = '{result.ProtocolError}'");
                return false;
            }

            return true;
        }

        public async Task<bool> StopQueryingLineSensor()
        {
            GattWriteResult result = await this.responseCharacteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(GattClientCharacteristicConfigurationDescriptorValue.None)
                                                                      .AsTask()
                                                                      .ConfigureAwait(false);
            if (result.Status != GattCommunicationStatus.Success)
            {
                this.logger.Error($"failed to disable notification for '{RESPONSE_CHARACTERISTICS_GUID}' charactersitic for '{SERVICE_GUID}' " +
                                  $"service on BLE device with id = '{this.bluetoothLeDevice.DeviceId}', result = '{result.Status}', protocol error = '{result.ProtocolError}'");
                return false;
            }

            return true;
        }

        private void OnConnectionStatusChanged(BluetoothLEDevice sender, object args)
        {
            // TODO
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args)
        {
            // TODO
        }

        private GattDeviceService service;
        private BluetoothLEDevice bluetoothLeDevice;
        private GattCharacteristic responseCharacteristic;

        private readonly ILogger logger;

        private static readonly Guid SERVICE_GUID                  = new Guid("0000fe40-cc7a-482a-984a-7f2ed5b3e58f");
        private static readonly Guid RESPONSE_CHARACTERISTICS_GUID = new Guid("0000fe42-8e22-4541-9d4c-21edae82ed19");
    }
}
