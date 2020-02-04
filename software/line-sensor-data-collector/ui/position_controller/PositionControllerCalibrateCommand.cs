using System;
using System.Threading.Tasks;

using Windows.UI.Core;

using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui.position_controller
{
    public class PositionControllerCalibrateCommand : BaseCommandWithParameter<IPositionControllerDeviceModel>
    {
        public PositionControllerCalibrateCommand(IPositionController positionController)
        {
            this.positionController = positionController ?? throw new ArgumentNullException(nameof(positionController));
        }

        public override bool CanExecute(IPositionControllerDeviceModel parameter)
        {
            return parameter != null &&
                   parameter.IsConnected &&
                   (parameter.GetBusyUIComponents() & UiComponent.POSITION_CONTROLLER) == 0;
        }

        public override void Execute(IPositionControllerDeviceModel parameter)
        {
            if (parameter == null)
            {
                return;
            }

            parameter.SetBusy(UiComponent.POSITION_CONTROLLER, true);

            Task.Run(() => ExecuteAsync(parameter))
                .ContinueWith(t =>
                              {
                                  parameter.SetStatus(PositionControllerCommand.CALIBRATE, t.Result);
                                  parameter.SetBusy(UiComponent.POSITION_CONTROLLER, false);
                              },
                              TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
        }

        /// <param name="parameter">method on model can only be called from UI thread</param>
        private async Task<PositionControllerResponse> ExecuteAsync(IPositionControllerDeviceModel parameter)
        {
            PositionControllerResponse result = await this.positionController.Calibrate()
                                                                             .ConfigureAwait(false);

            if (!positionController.IsOkStatus(result.Status))
            {
                return result;
            }

            DateTime start = DateTime.UtcNow;

            while (DateTime.UtcNow - start < MAX_CALIBRATION_TIME)
            {
                result = await this.positionController.GetPosition()
                                                      .ConfigureAwait(false);

                if (!positionController.IsOkStatus(result.Status) ||
                    (result.Status & PositionControllerStatus.PC_OK_MASK) == PositionControllerStatus.PC_OK_EMERGENCY_STOP)
                {
                    return result;
                }

                if ((result.Status & PositionControllerStatus.PC_OK_MASK) == PositionControllerStatus.PC_OK_IDLE)
                {
                    // idle means position controller finished calibration
                    return result;
                }
                else
                {
                    UpdateUi(parameter, result);
                }

                await Task.Delay(1000)
                          .ConfigureAwait(false);
            }

            return new PositionControllerResponse(PositionControllerStatus.CS_ERR_DEVICE_NOT_CONNTECTED);
        }

        private void UpdateUi(IPositionControllerDeviceModel parameter, PositionControllerResponse result)
        {
            CoreDispatcher dispatcher = parameter.Dispatcher;

            if (dispatcher == null)
            {
                return;
            }

            var ignore = dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                parameter.SetStatus(PositionControllerCommand.CALIBRATE, result);
            });
        }

        private readonly IPositionController positionController;

        private static readonly TimeSpan MAX_CALIBRATION_TIME = TimeSpan.FromMinutes(1);
    }
}
