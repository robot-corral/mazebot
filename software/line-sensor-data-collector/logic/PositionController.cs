using System;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.SerialCommunication;
using Windows.Storage.Streams;

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2011/february/msdn-magazine-parallel-computing-it-s-all-about-the-synchronizationcontext
// https://docs.microsoft.com/en-us/dotnet/api/system.threading.tasks.taskscheduler?view=netframework-4.8

namespace line_sensor.data_collector.logic
{
    public class PositionController : IPositionController
    {
        public PositionController()
        {
        }

        public bool IsConnected { get; private set; }

        public async Task<bool> TryToConnect(string serialDeviceId)
        {
            if (string.IsNullOrWhiteSpace(serialDeviceId))
            {
                // TODO errors
                return false;
            }

            SerialDevice serialDevice = null;

            try
            {
                serialDevice = await SerialDevice.FromIdAsync(serialDeviceId);

                serialDevice.BaudRate = 230400;
                serialDevice.IsDataTerminalReadyEnabled = true;
                serialDevice.IsRequestToSendEnabled = true;
                serialDevice.Parity = SerialParity.None;
                serialDevice.StopBits = SerialStopBitCount.One;
                serialDevice.DataBits = 8;
                serialDevice.ReadTimeout = TimeSpan.FromSeconds(1);
                serialDevice.WriteTimeout = TimeSpan.FromSeconds(1);

                using (CancellationTokenSource cancellationTokenSource = new CancellationTokenSource(TimeSpan.FromSeconds(1)))
                {
                    Task<IBuffer> readTask = serialDevice.InputStream.ReadAsync(this.response.AsBuffer(), RESPONSE_LENGTH, InputStreamOptions.None)
                                                                     .AsTask(cancellationTokenSource.Token);
                    Task<uint> writeTask = serialDevice.OutputStream.WriteAsync(CreateGetStatusRequest())
                                                                    .AsTask();

                    await Task.WhenAll(writeTask, readTask)
                              .ConfigureAwait(true);
                }

                if (response[0] != 0x41 || response[1] != 0x54)
                {
                    // TODO errors
                    return false;
                }

                this.IsConnected = true;
                this.serialDevice = serialDevice;

                return true;
            }
            catch (TaskCanceledException)
            {
                // TODO errors
                serialDevice?.Dispose();
                return false;
            }
            catch
            {
                // TODO errors
                serialDevice?.Dispose();
                return false;
            }
        }

        private IBuffer CreateGetStatusRequest()
        {
            this.request[0] = 0x41;
            this.request[1] = 0x54;
            this.request[2] = 0x04;
            return this.request.AsBuffer();
        }

        private SerialDevice serialDevice;

        private readonly byte[] request = new byte[REQUEST_LENGTH];
        private readonly byte[] response = new byte[RESPONSE_LENGTH];

        private const int REQUEST_LENGTH = 8;
        private const int RESPONSE_LENGTH = 7;
    }
}
