using System;
using System.Threading;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService.LuaBindings
{
    class OsExtensions
    {
        public static void Register(ExecutionConstrainedScript script, ManualResetEventSlim scriptThreadShutdownEvent)
        {
            RegisterSleepFunction(script, scriptThreadShutdownEvent);
        }

        private static void RegisterSleepFunction(ExecutionConstrainedScript script, ManualResetEventSlim scriptThreadShutdownEvent)
        {
            script.Globals.Get("os").Table["sleep"] = (Action<double>)((double ms) => 
            {
                if (ms <= 0)
                    throw ScriptRuntimeException.BadArgument(0, "sleep", "not a positive number");

                scriptThreadShutdownEvent.Wait(TimeSpan.FromMilliseconds(ms));
            });
        }
    }
}