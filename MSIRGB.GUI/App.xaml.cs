using System;
using System.Threading;
using System.Windows;

namespace MSIRGB
{
    public partial class App : Application
    {
        static Mutex mutex = new Mutex(true, @"Global\MSIRGB.GUI");

        App() : base()
        {
            if (!mutex.WaitOne(TimeSpan.Zero))
            {
                Current.Shutdown();
            }
        }
    }
}
