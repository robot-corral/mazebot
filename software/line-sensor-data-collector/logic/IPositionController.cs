using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public interface IPositionController
    {
        bool IsOkStatus(PositionControllerStatus status);

        Task<PositionControllerStatus> TryToConnect(string serialDeviceId);

        void Disconnect();

        Task<PositionControllerResponse> Calibrate();

        Task<PositionControllerResponse> EmergencyStop();

        Task<PositionControllerResponse> MoveIfIdle(PositionControllerDirection direction, uint steps);

        Task<PositionControllerResponse> GetPosition();

        Task Reset();

        /// <summary>
        /// This commands attempts to perform emergency stop, if it fails it attempts to reset
        /// position controller device, then try again.
        /// Each command execution takes no more than ~1 seconds.
        /// </summary>
        /// <param name="maxAttempts">
        ///   how many times to attempt to execute emergency stop command if one failed, 0 means skip
        /// </param>
        /// <returns>true if there is a good chance that emergency stop was completed, false otherwise</returns>
        Task<bool> StrongEmergencyStop(uint maxAttempts = 10);
    }
}
