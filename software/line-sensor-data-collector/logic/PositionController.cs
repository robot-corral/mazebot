using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;

using Windows.Devices.SerialCommunication;
using Windows.Storage.Streams;

using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.logic
{
    public class PositionController : IPositionController
    {
        static PositionController()
        {
            byte[] getStatus = new byte[REQUEST_LENGTH];
            CreateRequest(PositionControllerCommand.GET_STATUS, PositionControllerDirection.NONE, 0, getStatus);
            commandsBuffers.Add(PositionControllerCommand.GET_STATUS, getStatus);

            byte[] calibrate = new byte[REQUEST_LENGTH];
            CreateRequest(PositionControllerCommand.CALIBRATE, PositionControllerDirection.NONE, 0, calibrate);
            commandsBuffers.Add(PositionControllerCommand.CALIBRATE, calibrate);

            byte[] emergencyStop = new byte[REQUEST_LENGTH];
            CreateRequest(PositionControllerCommand.EMERGENCY_STOP, PositionControllerDirection.NONE, 0, emergencyStop);
            commandsBuffers.Add(PositionControllerCommand.EMERGENCY_STOP, emergencyStop);

            byte[] reset = new byte[REQUEST_LENGTH];
            CreateRequest(PositionControllerCommand.RESET, PositionControllerDirection.NONE, 0, reset);
            commandsBuffers.Add(PositionControllerCommand.RESET, reset);
        }

        public PositionController(ILogger logger)
        {
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
        }

        public async Task<PositionControllerResponse> TryToConnect(string serialDeviceId)
        {
            if (string.IsNullOrWhiteSpace(serialDeviceId))
            {
                this.logger.Error($"'{nameof(serialDeviceId)}' is null, empty or whitespace");
                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_INVALID_ARGUMENT);
            }

            this.semaphore.WaitOne();

            try
            {
                this.deviceId = serialDeviceId;
                this.serialDevice = await SerialDevice.FromIdAsync(serialDeviceId);

                this.serialDevice.BaudRate = 230400;
                this.serialDevice.IsDataTerminalReadyEnabled = true;
                this.serialDevice.IsRequestToSendEnabled = true;
                this.serialDevice.Parity = SerialParity.None;
                this.serialDevice.StopBits = SerialStopBitCount.One;
                this.serialDevice.DataBits = 8;
                this.serialDevice.ReadTimeout = TimeSpan.FromSeconds(1);
                this.serialDevice.WriteTimeout = TimeSpan.FromSeconds(1);

                using (CancellationTokenSource cancellationTokenSource = new CancellationTokenSource(TimeSpan.FromSeconds(1)))
                {
                    Task<IBuffer> readTask = this.serialDevice.InputStream.ReadAsync(this.response.AsBuffer(), RESPONSE_LENGTH, InputStreamOptions.None)
                                                                          .AsTask(cancellationTokenSource.Token);
                    Task<uint> writeTask = this.serialDevice.OutputStream.WriteAsync(commandsBuffers[PositionControllerCommand.GET_STATUS].AsBuffer())
                                                                         .AsTask();

                    await Task.WhenAll(writeTask, readTask)
                              .ConfigureAwait(false);
                }

                return DecodeResponse(this.response);
            }
            catch (TaskCanceledException)
            {
                this.logger.Error($"'{nameof(serialDeviceId)}' get position controller status request timed out");
                DisconnectUnsafe();
                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_COMMUNICATION_TIMEOUT);
            }
            catch (Exception e)
            {
                this.logger.Error(e, $"unexpected exception while getting '{nameof(serialDeviceId)}' position controller status");
                DisconnectUnsafe();
                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_UNEXPECTED);
            }
            finally
            {
                this.semaphore.Release();
            }
        }

        public async Task Disconnect()
        {
            this.semaphore.WaitOne();

            try
            {
                await ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.EMERGENCY_STOP], false).ConfigureAwait(false);

                DisconnectUnsafe();
            }
            finally
            {
                this.semaphore.Release();
            }
        }

        private void DisconnectUnsafe()
        {
            if (this.serialDevice != null)
            {
                this.serialDevice.Dispose();
                this.serialDevice = null;
            }
        }

        public Task<PositionControllerResponse> Calibrate()
        {
            return ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.CALIBRATE]);
        }

        public Task<PositionControllerResponse> EmergencyStop()
        {
            return ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.EMERGENCY_STOP]);
        }

        public Task<PositionControllerResponse> MoveIfIdle(PositionControllerDirection direction, uint steps)
        {
            byte[] moveIfIdle = new byte[REQUEST_LENGTH];
            CreateRequest(PositionControllerCommand.MOVE_IF_IDLE, direction, steps, moveIfIdle);
            return ExecutePrebuiltCommand(moveIfIdle);
        }

        public Task<PositionControllerResponse> GetPosition()
        {
            return ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.GET_STATUS]);
        }

        public Task<PositionControllerResponse> Reset()
        {
            return ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.RESET]);
        }

        public async Task<PositionControllerResponse> StrongReset(uint maxAttempts = 10)
        {
            this.semaphore.WaitOne();

            try
            {
                if (this.serialDevice == null)
                {
                    return new PositionControllerResponse(PositionControllerStatus.CS_ERR_DEVICE_NOT_CONNTECTED);
                }

                for (uint i = 0; i < maxAttempts; ++i)
                {
                    try
                    {
                        await ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.RESET], false).ConfigureAwait(false);

                        await Task.Delay(50)
                                  .ConfigureAwait(false);

                        PositionControllerResponse result = await ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.GET_STATUS], false).ConfigureAwait(false);

                        if ((result.Status & PositionControllerStatus.PC_OK_MASK) == PositionControllerStatus.PC_OK_RESET)
                        {
                            return result;
                        }
                    }
                    catch (Exception e)
                    {
                        this.logger.Error(e, $"unexpected exception while trying to reset '{nameof(this.deviceId)}' position controller");
                    }
                }

                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_GAVE_UP_EXECUTING_COMMAND);
            }
            finally
            {
                this.semaphore.Release();
            }
        }

        public async Task<PositionControllerResponse> StrongEmergencyStop(uint maxAttempts = 10)
        {
            this.semaphore.WaitOne();

            try
            {
                if (this.serialDevice == null)
                {
                    return new PositionControllerResponse(PositionControllerStatus.CS_ERR_DEVICE_NOT_CONNTECTED);
                }

                for (uint i = 0; i < maxAttempts; ++i)
                {
                    try
                    {
                        PositionControllerResponse result = await ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.EMERGENCY_STOP], false).ConfigureAwait(false);

                        if ((result.Status & PositionControllerStatus.PC_OK_MASK) == PositionControllerStatus.PC_OK_EMERGENCY_STOP)
                        {
                            return result;
                        }

                        await ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.RESET], false).ConfigureAwait(false);

                        await Task.Delay(50)
                                  .ConfigureAwait(false);

                        result = await ExecutePrebuiltCommand(commandsBuffers[PositionControllerCommand.GET_STATUS], false).ConfigureAwait(false);

                        if ((result.Status & PositionControllerStatus.PC_OK_MASK) == PositionControllerStatus.PC_OK_RESET)
                        {
                            return result;
                        }
                    }
                    catch (Exception e)
                    {
                        this.logger.Error(e, $"unexpected exception while trying to emergency stop '{nameof(this.deviceId)}' position controller");
                    }
                }

                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_GAVE_UP_EXECUTING_COMMAND);
            }
            finally
            {
                this.semaphore.Release();
            }
        }

        public bool IsOkStatus(PositionControllerStatus status)
        {
            return (status & ~PositionControllerStatus.PC_OK_MASK) == 0;
        }

        private static IBuffer CreateRequest(PositionControllerCommand command, PositionControllerDirection direction, uint steps, byte[] buffer)
        {
            if (buffer == null || buffer.Length < REQUEST_LENGTH)
            {
                throw new ArgumentException($"'{nameof(buffer)}' parameter is null or too small");
            }

            using (Stream s = new MemoryStream(buffer))
            {
                using (BinaryWriter bw = new BinaryWriter(s))
                {
                    bw.Write((ushort) DATA_HEADER);
                    bw.Write((byte)   command);
                    bw.Write((byte)   direction);
                    bw.Write((uint)   steps);
                }
            }

            Stm32Crc32 crc32 = new Stm32Crc32(CRC32_INITIAL_VALUE, CRC32_POLYNOMIAL);

            using (Stream s = new MemoryStream(buffer))
            {
                using (BinaryReader br = new BinaryReader(s))
                {
                    crc32.Encode(br.ReadUInt32());
                    crc32.Encode(br.ReadUInt32());
                }
            }

            using (Stream s = new MemoryStream(buffer, 8, 4))
            {
                using (BinaryWriter bw = new BinaryWriter(s))
                {
                    bw.Write((uint) crc32.GetValue());
                }
            }

            return buffer.AsBuffer();
        }

        private PositionControllerResponse DecodeResponse(byte[] buffer)
        {
            using (Stream s = new MemoryStream(buffer))
            {
                using (BinaryReader br = new BinaryReader(s))
                {
                    ushort header = br.ReadUInt16();

                    if (header != DATA_HEADER)
                    {
                        return new PositionControllerResponse(PositionControllerStatus.CS_ERR_COMMUNICATION);
                    }

                    ushort status = br.ReadUInt16();
                    uint position = br.ReadUInt32();
                    uint crc = br.ReadUInt32();

                    s.Seek(0, SeekOrigin.Begin);

                    Stm32Crc32 crc32 = new Stm32Crc32(CRC32_INITIAL_VALUE, CRC32_POLYNOMIAL);

                    crc32.Encode(br.ReadUInt32());
                    crc32.Encode(br.ReadUInt32());

                    if (crc32.GetValue() != crc)
                    {
                        return new PositionControllerResponse(PositionControllerStatus.CS_ERR_CRC);
                    }

                    return new PositionControllerResponse((PositionControllerStatus) status, position);
                }
            }
        }

        private async Task<PositionControllerResponse> ExecutePrebuiltCommand(byte[] commandRequestBuffer, bool useLock = true)
        {
            if (commandRequestBuffer == null || commandRequestBuffer.Length != REQUEST_LENGTH)
            {
                this.logger.Error($"'{nameof(commandRequestBuffer)}' parameter is null or too small");
                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_INVALID_ARGUMENT);
            }

            if (useLock)
            {
                this.semaphore.WaitOne();
            }

            try
            {
                if (this.serialDevice == null)
                {
                    return new PositionControllerResponse(PositionControllerStatus.CS_ERR_DEVICE_NOT_CONNTECTED);
                }

                using (CancellationTokenSource cancellationTokenSource = new CancellationTokenSource(TimeSpan.FromSeconds(1)))
                {
                    Task<IBuffer> readTask = this.serialDevice.InputStream.ReadAsync(this.response.AsBuffer(), RESPONSE_LENGTH, InputStreamOptions.None)
                                                                          .AsTask(cancellationTokenSource.Token);
                    Task<uint> writeTask = this.serialDevice.OutputStream.WriteAsync(commandRequestBuffer.AsBuffer())
                                                                         .AsTask(cancellationTokenSource.Token);

                    await Task.WhenAll(writeTask, readTask)
                              .ConfigureAwait(false);
                }

                return DecodeResponse(this.response);
            }
            catch (TaskCanceledException)
            {
                this.logger.Error($"'{nameof(this.deviceId)}' position controller calibration request timed out");
                DisconnectUnsafe();
                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_COMMUNICATION_TIMEOUT);
            }
            catch (Exception e)
            {
                this.logger.Error(e, $"unexpected exception while calibrating '{nameof(this.deviceId)}' position controller");
                DisconnectUnsafe();
                return new PositionControllerResponse(PositionControllerStatus.CS_ERR_UNEXPECTED);
            }
            finally
            {
                if (useLock)
                {
                    this.semaphore.Release();
                }
            }
        }

        private string deviceId;
        private SerialDevice serialDevice;

        private readonly ILogger logger;

        private readonly Semaphore semaphore = new Semaphore(1, 1);

        private readonly byte[] response = new byte[RESPONSE_LENGTH];

        private const int REQUEST_LENGTH = 12;
        private const int RESPONSE_LENGTH = 12;

        private const ushort DATA_HEADER = 0x5441;

        private const uint CRC32_POLYNOMIAL = 0x4C11DB7;
        private const uint CRC32_INITIAL_VALUE = 0xFFFFFFFF;

        private static Dictionary<PositionControllerCommand, byte[]> commandsBuffers = new Dictionary<PositionControllerCommand, byte[]>();
    }
}
