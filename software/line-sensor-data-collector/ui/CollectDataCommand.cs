using System;
using System.ComponentModel;
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
                   parameter.WirelessLineSensorDeviceModel.IsConnected;
        }

        public override void Execute(MainModel parameter)
        {
            if (parameter.CollectDataCommandTitle == MainModel.TITLE_COLLECT_DATA)
            {
                parameter.CollectDataCommandTitle = MainModel.TITLE_STOP_COLLECTING_DATA;

                UiComponent previousBusyComponents = parameter.SetBusy(UiComponent.POSITION_CONTROLLER | UiComponent.WIRELESS_LINE_SENSOR, true);

                this.cancellationTokenSource = new CancellationTokenSource();

                PropertyChangedEventHandler emergencyStoppedHandler = (s, e) => {
                    if (e.PropertyName == nameof(parameter.PositionControllerDeviceModel.IsEmergencyStopped) &&
                        parameter.PositionControllerDeviceModel.IsEmergencyStopped)
                    {
                        this.cancellationTokenSource.Cancel();
                    }
                };

                parameter.PositionControllerDeviceModel.PropertyChanged += emergencyStoppedHandler;

                int numberOfSamples = parameter.NumberOfSamples;
                string resultFilePathPrefix = parameter.FilePathPrefix;
                uint dataCollectionPositionDelta = (uint) (parameter.StepSize / PositionController.DISTANCE_PER_TICK_MM + 0.5);

                Task.Run(() => this.dataCollector.CollectData(dataCollectionPositionDelta,
                                                              numberOfSamples,
                                                              resultFilePathPrefix,
                                                              this.cancellationTokenSource.Token))
                    .ContinueWith(t =>
                    {
                        try
                        {
                            parameter.ResetBusy(previousBusyComponents);
                            parameter.CollectDataCommandTitle = MainModel.TITLE_COLLECT_DATA;
                        }
                        finally
                        {
                            parameter.PositionControllerDeviceModel.PropertyChanged -= emergencyStoppedHandler;
                        }
                    },
                    TaskScheduler.FromCurrentSynchronizationContext() /* make sure we continue on UI thread */);
            }
            else
            {
                this.cancellationTokenSource.Cancel();
            }
        }

        private readonly IDataCollector dataCollector;
        private CancellationTokenSource cancellationTokenSource;
    }
}
