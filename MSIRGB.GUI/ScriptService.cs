using System;
using System.IO;

namespace MSIRGB
{
    class ScriptService
    {
        private const string SCRIPT_SVC_NAME = "MSIRGB.ScriptService";
        private const string SCRIPT_SVC_DISPLAY_NAME = "MSIRGB Script Service";
        private const string SCRIPT_SVC_EXE = SCRIPT_SVC_NAME + ".exe";
        private const string SCRIPT_LOG_FILE = "ScriptLog.txt";
        private const string SCRIPTS_FOLDER = @"Scripts\";

        public static string GetScriptLogFilePath()
        {
            return Path.GetFullPath(SCRIPT_LOG_FILE);
        }

        public static string[] GetScripts()
        {
            if (!Directory.Exists(SCRIPTS_FOLDER))
            {
                return new string[0];
            }

            var scripts = Directory.GetFiles(SCRIPTS_FOLDER, "*.lua");

            for (int i = 0; i < scripts.Length; i++)
            {
                scripts[i] = Path.GetFileNameWithoutExtension(scripts[i]);
            }

            return scripts;
        }

        public static string GetRunningScript()
        {
            string[] args = Utils.ServiceInstaller.GetPermanentArguments(SCRIPT_SVC_NAME);

            if (args.Length > 0)
            {
                return Path.GetFileNameWithoutExtension(args[1]);
            }
            else
            {
                return null;
            }
        }

        public static void RunScript(string scriptName, bool ignoreMbCheck)
        {
            var scriptPath = Path.Combine(Path.GetFullPath(SCRIPTS_FOLDER), scriptName + ".lua");

            if (!File.Exists(scriptPath))
                throw new Exception("Invalid script path");

            string svcFullPath = Path.GetFullPath(SCRIPT_SVC_EXE);

            if (Utils.ServiceInstaller.IsServiceInstalled(SCRIPT_SVC_NAME))
            {
                Utils.ServiceInstaller.WaitStop(SCRIPT_SVC_NAME);
            }
            else
            {
                Utils.ServiceInstaller.Install(SCRIPT_SVC_NAME, SCRIPT_SVC_DISPLAY_NAME, svcFullPath);
            }

            string[] args =
            {
                GetScriptLogFilePath(),
                scriptPath,
                Convert.ToString(ignoreMbCheck)
            };

            Utils.ServiceInstaller.SetPermanentArguments(SCRIPT_SVC_NAME, svcFullPath, args);
            Utils.ServiceInstaller.Start(SCRIPT_SVC_NAME);
        }

        public static void StopAnyRunningScript()
        {
            Utils.ServiceInstaller.Uninstall(SCRIPT_SVC_NAME);
        }
    }
}
