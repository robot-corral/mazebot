using System;
using System.Windows.Input;

namespace line_sensor.data_collector.shared
{
    public abstract class BaseCommandWithParameter<T> : ICommand
    {
        public event EventHandler CanExecuteChanged;

        public virtual void UpdateCanExecute()
        {
            CanExecuteChanged?.Invoke(this, null);
        }

        public abstract bool CanExecute(T parameter);

        public abstract void Execute(T parameter);

        bool ICommand.CanExecute(object parameter)
        {
            if (parameter == null)
            {
                return CanExecute(default(T));
            }
            else
            {
                if (parameter is T == false)
                {
                    return false;
                }
                else
                {
                    return CanExecute((T) parameter);
                }
            }
        }

        void ICommand.Execute(object parameter)
        {
            if (!((ICommand) this).CanExecute(parameter))
            {
                return;
            }
            Execute((T) parameter);
        }
    }
}
