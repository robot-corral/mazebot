namespace line_sensor.data_collector.ui.ui_component
{
    public interface IUiComponent
    {
        /// <summary>
        /// This is called if this component busy status was changed
        /// </summary>
        void OnBusyChanged(bool isBusy);
    }
}
