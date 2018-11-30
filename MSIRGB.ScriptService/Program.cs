using System.ServiceProcess;
using System.Diagnostics;

namespace MSIRGB.ScriptService
{
    static class Program
    {
        static void Main()
        {
#if DEBUG
            Debugger.Launch();
#endif

            ServiceBase[] ServicesToRun;
            ServicesToRun = new ServiceBase[]
            {
                new ScriptService()
            };
            ServiceBase.Run(ServicesToRun);
        }
    }
}
