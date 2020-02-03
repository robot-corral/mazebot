using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public interface IPositionController
    {
        bool IsOkStatus(PositionControllerStatus status);

        Task<PositionControllerResponse> TryToConnect(string serialDeviceId);

        Task Disconnect();

        Task<PositionControllerResponse> Calibrate();

        Task<PositionControllerResponse> EmergencyStop();

        Task<PositionControllerResponse> MoveIfIdle(PositionControllerDirection direction, uint steps);

        Task<PositionControllerResponse> GetPosition();

        Task<PositionControllerResponse> Reset();

        /// <summary>
        /// Attempts to reset device until reset succeeds or max number of attempts was performed
        /// </summary>
        /// <param name="maxAttempts">
        ///   how many times to attempt to execute reset command if one failed, 0 means skip
        /// </param>
        Task<PositionControllerResponse> StrongReset(uint maxAttempts = 10);

        /// <summary>
        /// This commands attempts to perform emergency stop, if it fails it attempts to reset
        /// position controller device, then try again.
        /// Each command execution takes no more than ~1 seconds.
        /// </summary>
        /// <param name="maxAttempts">
        ///   how many times to attempt to execute emergency stop command if one failed, 0 means skip
        /// </param>
        Task<PositionControllerResponse> StrongEmergencyStop(uint maxAttempts = 10);
    }
}
