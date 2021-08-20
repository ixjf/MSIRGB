using System.Threading;
using System.IO;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService
{
    internal class ScriptThread
    {
        private ManualResetEventSlim _shutdownEvent;
        private Thread _thread;

        public ScriptThread(string logPath, string scriptPath, bool ignoreMbCheck)
        {
            _shutdownEvent = new ManualResetEventSlim(false);

            _thread = new Thread(() =>
            {
                Proc(_shutdownEvent, logPath, scriptPath, ignoreMbCheck);
            });

            _thread.Start();
        }

        ~ScriptThread()
        {
            Terminate();
        }

        public void Terminate()
        {
            _shutdownEvent.Set();

            // FIXME: if this happens, it could stop execution of batch_commit, which leads
            // to messed up settings. But this usually doesn't happen. BUT if it does, just... don't abort?
            // if it freezes, let the end user terminate if he so wishes
            //if (!_thread.Join(5000))
            //{
                //_thread.Abort();
            //}
        }

        private static void Proc(ManualResetEventSlim shutdownEvent, string logPath, string scriptPath, bool ignoreMbCheck)
        {
            var log = new Log(logPath);

            log.OutputInfo(string.Format("Initializing script thread (script file: '{0}')", Path.GetFileName(scriptPath)));

            // Add custom converters
            LuaBindings.CustomConverters.Register();

            // Create new Lua environment
            var script = new ExecutionConstrainedScript(CoreModules.Preset_Complete);

            script.Options.DebugPrint = s => log.OutputScriptPrint(s);

            foreach (var p in ((MoonSharp.Interpreter.Loaders.ScriptLoaderBase)script.Options.ScriptLoader).ModulePaths)
            {
                log.OutputInfo("hello");
                log.OutputInfo(string.Format("Module path: {0}", p));
            }

            // Bind modules & extensions
            LuaBindings.LightingModule.Register(script, ignoreMbCheck);
            LuaBindings.Aida64Module.Register(script);
            LuaBindings.OsExtensions.Register(script, shutdownEvent);

            // Run the script while waiting for stop
            // If one of the script service Lua functions takes too long (they shouldn't), this won't work
            // but it's the best that can be done here
            script.LoadFile(scriptPath);

            try
            {
                script.Do(15000, () =>
                {
                    return !shutdownEvent.IsSet;
                });
            }
            catch (InterpreterException exc)
            {
                log.OutputScriptError(exc.DecoratedMessage);
            }

            log.OutputInfo(string.Format("Finalizing script thread (script file: '{0}')", Path.GetFileName(scriptPath)));
        }
    }
}
