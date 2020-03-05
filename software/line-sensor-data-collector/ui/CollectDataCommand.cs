using System;
using System.ComponentModel;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using line_sensor.data_collector.logic;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui.ui_component;

namespace line_sensor.data_collector.ui
{
    public class CollectDataCommand : BaseCommandWithParameter<MainModel>
    {
        public CollectDataCommand(IDataCollector dataCollector)
        {
            this.dataCollector = dataCollector ?? throw new ArgumentNullException(nameof(dataCollector));
        }

        public override bool CanExecute(MainModel parameter)
        {
            return parameter != null &&
                   parameter.PositionControllerDeviceModel.IsConnected &&
                   parameter.WirelessLineSensorDeviceModel.IsConnected &&
                   (parameter.GetBusyUIComponents() & (UiComponent.POSITION_CONTROLLER | UiComponent.WIRELESS_LINE_SENSOR)) == 0;
        }

        public override void Execute(MainModel parameter)
        {
            UiComponent previousBusyComponents = parameter.SetBusy(UiComponent.POSITION_CONTROLLER | UiComponent.WIRELESS_LINE_SENSOR, true);

            CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();

            PropertyChangedEventHandler emergencyStoppedHandler = (s, e) => {
                if (e.PropertyName == nameof(parameter.PositionControllerDeviceModel.IsEmergencyStopped) &&
                    parameter.PositionControllerDeviceModel.IsEmergencyStopped)
                {
                    cancellationTokenSource.Cancel();
                }
            };

            parameter.PositionControllerDeviceModel.PropertyChanged += emergencyStoppedHandler;

            // TODO
            // TODO
            // TODO
            // TODO
            // TODO
            // TODO
            // TODO
            // TODO
            // TODO
            // TODO
            // TODO
            // TODO

            Task.Run(() => this.dataCollector.CollectData(20, 1000, Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "line_sensor_data"), cancellationTokenSource.Token))
                .ContinueWith(t =>
                              {
                                  try
                                  {
                                      parameter.ResetBusy(previousBusyComponents);
                                  }
                                  finally
                                  {
                                      parameter.PositionControllerDeviceModel.PropertyChanged -= emergencyStoppedHandler;
                                  }
                              },
                TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
        }

        private readonly IDataCollector dataCollector;
    }
}
