using System;
using System.IO;
using System.ServiceProcess;
using System.Threading;
using System.Threading.Tasks;
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
            Script.GlobalOptions.CustomConverters.SetScriptToClrCustomConversion(DataType.Number, typeof(byte), CustomConverters.NumberToByte);

            // Create new Lua environment
            Script script = new Script(CoreModules.Basic |
                                       CoreModules.TableIterators |
                                       CoreModules.String |
                                       CoreModules.Table |
                                       CoreModules.Math |
                                       CoreModules.Bit32 |
                                       CoreModules.OS_Time);

            script.Options.DebugPrint = s => log.OutputScriptPrint(s);

            // Bind modules & extensions
            UserData.RegisterType(typeof(LuaBindings.LightingModule));

            script.Globals["Lighting"] = new LuaBindings.LightingModule(ignoreMbCheck);

            script.Globals.Get("os").Table["sleep"] = (Action<double>)LuaBindings.OsExtensions.Sleep;

            // Run the script while waiting for stop
            // If one of the script service Lua functions takes too long (they shouldn't), this won't work
            // but it's the best that can be done here
            DynValue scriptCoroutine = script.CreateCoroutine(script.LoadFile(scriptPath));
            scriptCoroutine.Coroutine.AutoYieldCounter = 15000; // Yield execution of the Lua script every 15,000 instructions

            while (true)
            {
                try
                {
                    DynValue scriptCoroutineResult = scriptCoroutine.Coroutine.Resume();

                    if (scriptCoroutineResult.Type != DataType.YieldRequest) // Script has finished
                        break;
                }
                catch (InterpreterException exc)
                {
                    log.OutputScriptError(exc.DecoratedMessage);
                    break;
                }

                if (_shutdownEvent.IsSet)
                    break;
            }

            log.OutputInfo(String.Format("Finalizing script thread (script file: '{0}')", Path.GetFileName(scriptPath)));
        }
    }
}
