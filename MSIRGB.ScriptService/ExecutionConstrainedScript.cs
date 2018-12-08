using System;
using System.Collections.Generic;
using System.IO;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService
{
    // Wraps MoonSharp.Interpreter.Script in a coroutine
    // allowing to yield execution of a script every n instructions
    class ExecutionConstrainedScript : Script
    {
        private List<string> _scriptChunks = new List<string>();

        public ExecutionConstrainedScript() : base()
        {

        }

        public ExecutionConstrainedScript(CoreModules coreModules) : base(coreModules)
        {

        }

        public void LoadFile(string filename)
        {
            _scriptChunks.Add(File.ReadAllText(filename));
        }

        public void LoadString(string code)
        {
            _scriptChunks.Add(code);
        }

        // Runs the stored chunks at once in a coroutine. Yields execution of the script
        // every yieldCounter instructions, calling yieldCallback. yieldCallback returns
        // whether to continue execution or not.
        public void Do(long yieldCounter, Func<bool> yieldCallback)
        {
            DynValue coroutine = base.CreateCoroutine(base.LoadString(string.Join("\n", _scriptChunks)));

            coroutine.Coroutine.AutoYieldCounter = yieldCounter;

            while (coroutine.Coroutine.State != CoroutineState.Dead)
            {
                coroutine.Coroutine.Resume();

                if (yieldCallback() == false) // We don't want to continue execution
                    break;
            }
        }
    }
}