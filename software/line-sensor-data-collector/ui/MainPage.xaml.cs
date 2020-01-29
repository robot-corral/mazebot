using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace line_sensor.data_collector.ui
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            DataContext = App.MainModel;
            this.model = App.MainModel;
            InitializeComponent();
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            this.model.Initialize(Dispatcher);
        }

        private readonly MainModel model;
    }
}
