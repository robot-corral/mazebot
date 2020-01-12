using System;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using line_sensor.data_collector.shared;
using line_sensor.data_collector.ui;

namespace line_sensor.data_collector
{
    sealed partial class App : Application
    {
        static App()
        {
            logger.Initialize();
        }

        public App()
        {
            InitializeComponent();
            Resuming += OnResuming;
            Suspending += OnSuspending;
            UnhandledException += OnUnhandledException;
        }

        public static ILogger Logger { get { return logger;  } }

        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            Frame rootFrame = Window.Current.Content as Frame;

            if (rootFrame == null)
            {
                rootFrame = new Frame();
                rootFrame.NavigationFailed += OnNavigationFailed;

                if (e.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    //TODO: Load state from previously suspended application
                }

                Window.Current.Content = rootFrame;
            }

            if (e.PrelaunchActivated == false)
            {
                if (rootFrame.Content == null)
                {
                    rootFrame.Navigate(typeof(MainPage), e.Arguments);
                }
                Window.Current.Activate();
            }
        }

        private void OnResuming(object sender, object e)
        {
            //TODO: Load state from previously suspended application
        }

        void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            logger.Error(e.Exception, "unhandled navigation error");
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        private void OnUnhandledException(object sender, Windows.UI.Xaml.UnhandledExceptionEventArgs e)
        {
            logger.Error(e.Exception, "unhandled error");
            MessageDialog dialog = new MessageDialog(e.Exception.ToString(), "Error");
            dialog.ShowAsync().AsTask().Wait();
        }

        private void OnSuspending(object sender, SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();
            Task.WaitAll(logger.OnSuspending());
            deferral.Complete();
        }

        private static readonly Logger logger = new Logger();
    }
}
