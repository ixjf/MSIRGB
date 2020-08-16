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

            if (!_thread.Join(5000))
            {
                _thread.Abort();
            }
        }

        private static void Proc(ManualResetEventSlim shutdownEvent, string logPath, string scriptPath, bool ignoreMbCheck)
        {
            //var log = new Log(logPath);

            //log.OutputInfo(string.Format("Initializing script thread (script file: '{0}')", Path.GetFileName(scriptPath)));

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

            //script.Options.DebugPrint = s => log.OutputScriptPrint(s);

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
                //log.OutputScriptError(exc.DecoratedMessage);
            }

            //log.OutputInfo(string.Format("Finalizing script thread (script file: '{0}')", Path.GetFileName(scriptPath)));
        }
    }
}
