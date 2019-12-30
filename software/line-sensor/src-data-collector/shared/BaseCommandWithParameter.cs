using System;
using System.Threading.Tasks;
using System.Windows.Input;

namespace line_sensor.data_collector.shared
{
    public abstract class BaseCommandWithParameter<T> : ICommand
    {
        public event EventHandler CanExecuteChanged;

        public virtual void UpdateCanExecute(T parameter)
        {
            CanExecuteChanged?.Invoke(parameter, null);
        }

        protected abstract bool CanExecuteImpl(T parameter);

        protected abstract Task ExecuteAsyncImpl(T parameter);

        public bool CanExecute(object parameter)
        {
            return CanExecuteImpl((T) parameter);
        }

        public void Execute(object parameter)
        {
            Task.Run(() => ExecuteAsyncImpl((T) parameter));
        }
    }
}
