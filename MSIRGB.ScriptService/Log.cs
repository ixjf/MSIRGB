using System;
using System.IO;

namespace MSIRGB.ScriptService
{
    class Log
    {
        private StreamWriter _log;

        public Log(string logPath)
        {
            _log = File.CreateText(logPath); // This should just crash if it fails
            _log.AutoFlush = true;
        }

        public void OutputScriptError(string s)
        {
            _log.WriteLine(String.Format("[SCRIPT ERROR] {0}", s));
        }

        public void OutputScriptPrint(string s)
        {
            _log.WriteLine(String.Format("[SCRIPT OUTPUT] {0}", s));
        }

        public void OutputInfo(string s)
        {
            _log.WriteLine(String.Format("[INFO] {0}", s));
        }
    }
}
