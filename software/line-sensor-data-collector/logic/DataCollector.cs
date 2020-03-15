using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using line_sensor.data_collector.shared;

namespace line_sensor.data_collector.logic
{
    public class DataCollector : IDataCollector
    {
        public DataCollector(IPositionController positionController, IWirelessLineSensor wirelessLineSensor, ILogger logger)
        {
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));
            this.wirelessLineSensor = wirelessLineSensor ?? throw new ArgumentNullException(nameof(wirelessLineSensor));
        }

        public async Task CollectData(uint dataCollectionPositionDelta, int numberOfSamples, string resultfolderPath, CancellationToken token)
        {
            PositionControllerResponse response = await this.positionController.GetPosition().ConfigureAwait(false);

            // we assume line sensor is positioned in the middle of the position controller and image is awlays in the same place

            uint startPosition = response.Position;
            uint currentPosition = startPosition;

            if (Directory.Exists(resultfolderPath))
            {
                Directory.Delete(resultfolderPath);
            }

            Directory.CreateDirectory(resultfolderPath);

            while (!token.IsCancellationRequested)
            {
                double currentRealPosition = currentPosition;
                currentRealPosition -= startPosition;
                currentRealPosition *= PositionController.DISTANCE_PER_TICK_MM;

                if (currentRealPosition > SENSOR_LENGTH)
                {
                    break;
                }

                LineSensorReading lineSensorReading = await getLineSensorData(currentRealPosition, numberOfSamples, token).ConfigureAwait(false);
                await saveLineSensorDataToFile(lineSensorReading, resultfolderPath).ConfigureAwait(false);
                currentPosition = await moveToNextPosition(dataCollectionPositionDelta).ConfigureAwait(false);
            }
        }

        private async Task<LineSensorReading> getLineSensorData(double currentPosition, int numberOfSamples, CancellationToken token)
        {
            bool callbackIsCalled = false;
            Semaphore semaphore = new Semaphore(0, 1);
            List<ushort[]> sensorValues = new List<ushort[]>();

            DeviceSentDataEvent callback = (source, data) =>
            {
                callbackIsCalled = true;

                if (token.IsCancellationRequested)
                {
                    try
                    {
                        semaphore.Release();
                    }
                    catch (SemaphoreFullException)
                    {
                        // ignore as callback might get called multiple times after being cancelled
                    }
                }
                else if ((data.CurrentStatus & LineSensorStatus.LSS_OK_FLAG_NEW_DATA_AVAILABLE) == LineSensorStatus.LSS_OK_FLAG_NEW_DATA_AVAILABLE &&
                         (data.CurrentStatus & LineSensorStatus.LSS_ERROR) == 0)
                {
                    lock (sensorValues)
                    {
                        if (sensorValues.Count >= numberOfSamples)
                        {
                            semaphore.Release();
                        }
                        else
                        {
                            sensorValues.Add(data.SensorValues);
                        }
                    }
                }
            };

            this.wirelessLineSensor.DeviceSentDataEvent += callback;

            try
            {
                // make sure we purge all old events before starting to receive new ones

                while (callbackIsCalled)
                {
                    callbackIsCalled = false;
                    await Task.Delay(500).ConfigureAwait(false);
                }

                bool success = false;

                for (int i = 0; i < START_QUERYING_LINE_SENSOR_RETRIES_COUNT && !token.IsCancellationRequested; ++i)
                {
                    if (await this.wirelessLineSensor.StartQueryingLineSensor().ConfigureAwait(false))
                    {
                        success = true;
                        break;
                    }

                    await Task.Delay(100).ConfigureAwait(false);
                }

                if (!success)
                {
                    throw new InvalidOperationException("failed to start querying line sensor, wireless device might have disconnected or powered down");
                }

#if DEBUG
                if (!semaphore.WaitOne())
#else
                if (!semaphore.WaitOne(TIMEOUT_TO_QUERY_LINE_SENSOR_MILLISECONDS))
#endif
                {
                    throw new TimeoutException("querying line sensor timed out");
                }

                for (int i = 0; i < STOP_QUERYING_LINE_SENSOR_RETRIES_COUNT && !token.IsCancellationRequested; ++i)
                {
                    if (await this.wirelessLineSensor.StopQueryingLineSensor().ConfigureAwait(false))
                    {
                        token.ThrowIfCancellationRequested();

                        if (sensorValues.Count < numberOfSamples)
                        {
                            throw new InvalidOperationException($"for some reason we collected {sensorValues.Count} sensor values which is less than {numberOfSamples} sensor values ");
                        }
                        else if (sensorValues.Count > numberOfSamples)
                        {
                            sensorValues = sensorValues.GetRange(0, numberOfSamples);
                        }

                        return new LineSensorReading { Position = currentPosition, SensorValues = sensorValues };
                    }

                    await Task.Delay(100).ConfigureAwait(false);
                }

                throw new InvalidOperationException("failed to stop querying line sensor, wireless device might have disconnected or powered down");
            }
            finally
            {
                this.wirelessLineSensor.DeviceSentDataEvent -= callback;
            }
        }

        private async Task saveLineSensorDataToFile(LineSensorReading lineSensorReading, string resultfolderPath)
        {
            string positionString = string.Format("{0:000.00}", lineSensorReading.Position);
            string resultFilePath = $"{resultfolderPath}\\{positionString}.csv";

            try
            {
                using (FileStream fs = new FileStream(resultFilePath, FileMode.Create, FileAccess.Write))
                {
                    using (TextWriter tw = new StreamWriter(fs))
                    {
                        foreach (ushort[] lsv in lineSensorReading.SensorValues)
                        {
                            await tw.WriteLineAsync(ConvertLineSensorValuesToString(lsv)).ConfigureAwait(false);
                        }
                    }
                }
            }
            catch
            {
                File.Delete(resultFilePath);
                throw;
            }
        }

        private string ConvertLineSensorValuesToString(ushort[] lineSensorValues)
        {
            if (lineSensorValues.Length != WirelessLineSensor.NUMBER_OF_SENSORS)
            {
                throw new InvalidOperationException($"sensor returned {lineSensorValues.Length} value when {WirelessLineSensor.NUMBER_OF_SENSORS} is expected");
            }

            StringBuilder sb = new StringBuilder();

            foreach (ushort v in lineSensorValues)
            {
                if (sb.Length > 0)
                {
                    sb.Append(',');
                }
                sb.Append(v);
            }

            return sb.ToString();
        }
 
        private async Task<uint> moveToNextPosition(uint dataCollectionPositionDelta)
        {
            PositionControllerResponse response = await this.positionController.MoveIfIdle(PositionControllerDirection.FORWARD, dataCollectionPositionDelta).ConfigureAwait(false);

            if ((response.Status & PositionControllerStatus.PC_OK_EMERGENCY_STOP) == PositionControllerStatus.PC_OK_EMERGENCY_STOP)
            {
                throw new InvalidOperationException("position controller is in emergency stop");
            }
            else
            {
                do
                {
                    response = await this.positionController.GetPosition().ConfigureAwait(false);

                    if ((response.Status & PositionControllerStatus.PC_OK_EMERGENCY_STOP) == PositionControllerStatus.PC_OK_EMERGENCY_STOP)
                    {
                        throw new InvalidOperationException("position controller is in emergency stop");
                    }
                } while (!positionController.IsOkStatus(response.Status) || (response.Status & PositionControllerStatus.PC_OK_BUSY) != 0);

                return response.Position;
            }
        }

        private readonly ILogger logger;
        private readonly IPositionController positionController;
        private readonly IWirelessLineSensor wirelessLineSensor;

        private const int STOP_QUERYING_LINE_SENSOR_RETRIES_COUNT = 10;
        private const int START_QUERYING_LINE_SENSOR_RETRIES_COUNT = 10;
        private const int TIMEOUT_TO_QUERY_LINE_SENSOR_MILLISECONDS = 10 * 60 * 1000;

        private const double SENSOR_LENGTH = 112.0 /* sensor width */ + 6.35 /* width of a line */;
    }
}
