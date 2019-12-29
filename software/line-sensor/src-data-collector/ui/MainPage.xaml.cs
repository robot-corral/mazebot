using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace line_sensor.data_collector.ui
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            MainModel model = new MainModel(Dispatcher);
            DataContext = model;
            this.model = model;
            InitializeComponent();
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            this.model.Initialize();
        }

        private readonly MainModel model;
    }
}
