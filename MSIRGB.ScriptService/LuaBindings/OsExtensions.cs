using MoonSharp.Interpreter;
using System.Threading;

namespace MSIRGB.ScriptService.LuaBindings
{
    class OsExtensions
    {
        [MoonSharpModuleMethod(Name = "sleep")]
        public static void Sleep(double ms)
        {
            if (ms <= 0)
                throw new ScriptRuntimeException("invalid value for parameter 'ms' (number must be positive)");

            Thread.Sleep(System.TimeSpan.FromMilliseconds(ms));
        }
    }
}