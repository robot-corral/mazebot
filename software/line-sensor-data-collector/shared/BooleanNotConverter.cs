using System;
using Windows.UI.Xaml.Data;

namespace line_sensor.data_collector.shared
{
    public class BooleanNotConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            if (targetType != typeof(bool))
            {
                throw new ArgumentException($"expected type is {typeof(bool)} but actual parameter is {targetType}");
            }
            if (!(value is bool))
            {
                throw new ArgumentException($"expected valye type is {typeof(bool)} but actual value type is {value?.GetType()}");
            }
            return ! ((bool) value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            throw new NotImplementedException();
        }
    }
}
