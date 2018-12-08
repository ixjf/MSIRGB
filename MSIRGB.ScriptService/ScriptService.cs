using System;
using System.IO;
using System.ServiceProcess;
using System.Threading;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService
{
    public partial class ScriptService : ServiceBase
    {
        private static ManualResetEventSlim _shutdownEvent = new ManualResetEventSlim(false);
        private Thread _scriptThread;

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

            _scriptThread = new Thread(() => 
            {
                ScriptThreadProc(logPath, scriptPath, ignoreMbCheck);
            });

            _scriptThread.Start();
        }

        protected override void OnStop()
        {
            if (_scriptThread != null)
            {
                _shutdownEvent.Set();

                if (!_scriptThread.Join(3000))
                {
                    _scriptThread.Abort();
                }
            }
        }

        private static void ScriptThreadProc(string logPath, string scriptPath, bool ignoreMbCheck)
        {
            var log = new Log(logPath);

            log.OutputInfo(String.Format("Initializing script thread (script file: '{0}')", Path.GetFileName(scriptPath)));

            // Add custom converters
            LuaBindings.CustomConverters.Register();

            // Create new Lua environment
            var script = new ExecutionConstrainedScript(CoreModules.Basic |
                                                        CoreModules.TableIterators |
                                                        CoreModules.String |
                                                        CoreModules.Table |
                                                        CoreModules.Math |
                                                        CoreModules.Bit32 |
                                                        CoreModules.OS_Time);

            script.Options.DebugPrint = s => log.OutputScriptPrint(s);

            // Bind modules & extensions
            LuaBindings.LightingModule.Register(script, ignoreMbCheck);
            LuaBindings.OsExtensions.Register(script);

            // Run the script while waiting for stop
            // If one of the script service Lua functions takes too long (they shouldn't), this won't work
            // but it's the best that can be done here
            script.LoadFile(scriptPath);

            try
            {
                script.Do(15000, () => 
                {
                    return !_shutdownEvent.IsSet;
                });
            }
            catch (InterpreterException exc)
            {
                log.OutputScriptError(exc.DecoratedMessage);
            }

            log.OutputInfo(String.Format("Finalizing script thread (script file: '{0}')", Path.GetFileName(scriptPath)));
        }
    }
}
