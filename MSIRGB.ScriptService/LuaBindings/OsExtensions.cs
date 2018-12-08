namespace MSIRGB.ScriptService.LuaBindings
{
    class OsExtensions
    {
        public static void Register(ExecutionConstrainedScript script)
        {
            RegisterSleepFunction(script);
        }

        // I don't like this. Having to rely on os.clock and not being able to use ScriptRuntimeException.BadArgument
        // The problem is, in order to be able to stop the script thread cleanly, I can't have any binding function
        // take too long, else the instruction count hook doesn't work.
        // Still bad, tho, because MoonSharp fails to implement 'error' properly
        private static void RegisterSleepFunction(ExecutionConstrainedScript script)
        {
            script.LoadString(@"
                local _____os_clock = os.clock -- just in case someone decides to overwrite os.clock

                function os.sleep(ms)
                    if (ms <= 0) then error(""bad argument #1 to 'sleep' (not a positive number)"", 2) end
                    local initialTime = _____os_clock()
                    while ((_____os_clock() - initialTime) < (ms / 1000)) do end
                end
            ");
        }
    }
}