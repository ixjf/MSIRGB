using System;
using System.IO;
using System.ServiceProcess;

namespace MSIRGB.ScriptService
{
    public partial class ScriptService : ServiceBase
    {
        private ScriptThread _scriptThread;

        public ScriptService()
        {
            InitializeComponent();
        }

        protected override void OnStart(string[] args)
        {
            // 'args' doesn't contain command line arguments that aren't passed by StartService
            // Since we pass these through the registry, we need to use Environment.GetCommandLineArgs
            args = Environment.GetCommandLineArgs();

            var logPath = args[1];
            var scriptPath = args[2];
            var ignoreMbCheck = Convert.ToBoolean(args[3]);

            if (!File.Exists(scriptPath))
            {
                Stop();
                return;
            }

            _scriptThread = new ScriptThread(logPath, scriptPath, ignoreMbCheck);
        }

        protected override void OnStop()
        {
            if (_scriptThread != null)
            {
                _scriptThread.Terminate();
            }
        }
    }
}
