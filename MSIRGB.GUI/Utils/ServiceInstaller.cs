using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Threading;

namespace MSIRGB.Utils
{
    internal class ServiceInstaller
    {
        public static bool IsServiceInstalled(string svcName)
        {
            IntPtr scManager = OpenSCManager(null, null, SC_MANAGER_CONNECT);
            IntPtr svc = OpenService(scManager, svcName, SERVICE_ALL_ACCESS);

            if (svc == IntPtr.Zero)
            {
                CloseServiceHandle(scManager);

                return false;
            }
            else
            {
                CloseServiceHandle(svc);
                CloseServiceHandle(scManager);

                return true;
            }
        }

        public static void WaitStop(string svcName)
        {
            if (IsServiceInstalled(svcName))
            {
                IntPtr scManager = OpenSCManager(null, null, SC_MANAGER_CONNECT);
                IntPtr svc = OpenService(scManager, svcName, SERVICE_STOP | SERVICE_QUERY_STATUS);

                SERVICE_STATUS svcStatus = new SERVICE_STATUS();

                ControlService(svc, SERVICE_CONTROL_STOP, out svcStatus);

                while (svcStatus.dwCurrentState != SERVICE_STOPPED && QueryServiceStatus(svc, out svcStatus))
                {
                    Thread.Sleep(50);
                }

                CloseServiceHandle(svc);
                CloseServiceHandle(scManager);
            }
        }

        public static bool Install(string svcName, string displayName, string svcPath)
        {
            if (IsServiceInstalled(svcName))
            {
                return false;
            }
            else
            {
                IntPtr scManager = OpenSCManager(null, null, SC_MANAGER_CREATE_SERVICE);
                IntPtr svc = CreateService(scManager,
                                            svcName,
                                            displayName,
                                            SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS,
                                            SERVICE_WIN32_OWN_PROCESS,
                                            SERVICE_AUTO_START,
                                            SERVICE_ERROR_NORMAL,
                                            svcPath,
                                            null,
                                            0,
                                            null,
                                            null,
                                            null);

                if (svc == IntPtr.Zero)
                {
                    CloseServiceHandle(scManager);
                    return false;
                }

                CloseServiceHandle(svc);
                CloseServiceHandle(scManager);

                return true;
            }
        }

        public static void SetPermanentArguments(string svcName, string svcPath, string[] args)
        {
            string imagePath = "\"" + svcPath + "\"";

            foreach (string s in args)
            {
                imagePath += " \"" + s + "\"";
            }

            Registry.SetValue(@"HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\" + svcName,
                              "ImagePath",
                              imagePath);
        }

        public static string[] GetPermanentArguments(string svcName)
        {
            object imagePath = Registry.GetValue(@"HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\" + svcName,
                                              "ImagePath",
                                              null);

            if (imagePath == null)
            {
                return new string[0];
            }

            List<string> args = new List<string>();

            // The following regex matches all non-empty (i.e. no characters or just white spaces) data 
            // within quotation marks
            //   (?<=\") is a positive lookbehind, it matches the value after <= but doesn't
            // include it in the result, i.e won't include initial quotation mark
            //   ([^\"]*.\\S) matches all non-whitespace (\\S), non-line breaks (.), non-quotation marks ([^\"])
            // and includes it in the result
            //   (?=\") is a positive lookahead, it matches the value after = but doesn't include
            // it in the result, i.e. won't include the ending quotation mark for the match
            // 
            // Basically, Regex.Matches will transform "aa a" "b"    "c" "2320%$)" into matches 'aa a', 'b', 'c', '2320%$'
            foreach (Match match in Regex.Matches((string)imagePath, "(?<=\")([^\"]*.\\S)(?=\")"))
            {
                args.Add(match.ToString());
            }

            if (args.Count > 0)
            {
                args.RemoveAt(0); // Don't want the service EXE path
            }

            return args.ToArray();
        }

        public static bool Start(string svcName)
        {
            if (!IsServiceInstalled(svcName))
            {
                return false;
            }

            if (IsServiceNotStopped(svcName))
            {
                return false;
            }

            IntPtr scManager = OpenSCManager(null, null, SC_MANAGER_CONNECT);
            IntPtr svc = OpenService(scManager, svcName, SERVICE_START);

            if (!StartService(svc, 0, null))
            {
                CloseServiceHandle(svc);
                CloseServiceHandle(scManager);

                return false;
            }

            CloseServiceHandle(svc);
            CloseServiceHandle(scManager);

            return true;
        }

        public static void Uninstall(string svcName)
        {
            if (IsServiceInstalled(svcName))
            {
                if (IsServiceNotStopped(svcName))
                {
                    WaitStop(svcName);
                }

                IntPtr scManager = OpenSCManager(null, null, SC_MANAGER_CONNECT);
                IntPtr svc = OpenService(scManager, svcName, DELETE);

                DeleteService(svc);

                CloseServiceHandle(svc);
                CloseServiceHandle(scManager);
            }
        }

        private static bool IsServiceNotStopped(string svcName)
        {
            IntPtr scManager = OpenSCManager(null, null, SC_MANAGER_CONNECT);
            IntPtr svc = OpenService(scManager, svcName, SERVICE_QUERY_STATUS);

            if (svc == IntPtr.Zero)
            {
                return false;
            }
            else
            {
                SERVICE_STATUS svcStatus = new SERVICE_STATUS();
                QueryServiceStatus(svc, out svcStatus);

                CloseServiceHandle(svc);
                CloseServiceHandle(scManager);

                return svcStatus.dwCurrentState == SERVICE_RUNNING ||
                       svcStatus.dwCurrentState == SERVICE_STOP_PENDING ||
                       svcStatus.dwCurrentState == SERVICE_START_PENDING;
            }
        }

        #region Win32 Imports
        private const int SC_MANAGER_CONNECT = 0x0001;
        private const int SC_MANAGER_CREATE_SERVICE = 0x0002;
        private const int SERVICE_ALL_ACCESS = 0xF01FF;
        private const int SERVICE_START = 0x0010;
        private const int SERVICE_STOP = 0x0020;
        private const int SERVICE_QUERY_STATUS = 0x0004;
        private const int SERVICE_START_PENDING = 0x00000002;
        private const int SERVICE_STOP_PENDING = 0x00000003;
        private const int SERVICE_RUNNING = 0x00000004;
        private const int SERVICE_STOPPED = 0x00000001;
        private const int SERVICE_WIN32_OWN_PROCESS = 0x00000010;
        private const int SERVICE_AUTO_START = 0x00000002;
        private const int SERVICE_ERROR_NORMAL = 0x00000001;
        private const int SERVICE_CONTROL_STOP = 0x00000001;
        private const int DELETE = 0x10000;

        [StructLayout(LayoutKind.Sequential)]
        private struct SERVICE_STATUS
        {
            public int dwServiceType;
            public int dwCurrentState;
            public int dwControlsAccepted;
            public int dwWin32ExitCode;
            public int dwServiceSpecificExitCode;
            public int dwCheckPoint;
            public int dwWaitHint;
        }

        [DllImport("advapi32.dll")]
        private static extern IntPtr OpenSCManager(string lpMachineName, string lpDatabaseName, int dwDesiredAccess);

        [DllImport("advapi32.dll")]
        private static extern IntPtr OpenService(IntPtr hSCManager, string lpServiceName, int dwDesiredAccess);

        [DllImport("advapi32.dll")]
        private static extern bool StartService(IntPtr hService, int dwNumServiceArgs, string[] lpServiceArgVectors);

        [DllImport("advapi32.dll")]
        private static extern bool DeleteService(IntPtr hService);

        [DllImport("advapi32.dll")]
        private static extern bool ControlService(IntPtr hService, int dwControl, out SERVICE_STATUS lpServiceStatus);

        [DllImport("advapi32.dll")]
        private static extern bool CloseServiceHandle(IntPtr hSCObject);

        [DllImport("advapi32.dll")]
        private static extern bool QueryServiceStatus(IntPtr hService, out SERVICE_STATUS lpServiceStatus);

        [DllImport("advapi32.dll", SetLastError = true)]
        private static extern IntPtr CreateService(IntPtr hSCManager,
                                                    string lpServiceName,
                                                    string lpDisplayName,
                                                    int dwDesiredAccess,
                                                    int dwServiceType,
                                                    int dwStartType,
                                                    int dwErrorControl,
                                                    string lpBinaryPathName,
                                                    string lpLoadOrderGroup,
                                                    int lpdwTagId,
                                                    string lpDependencies,
                                                    string lpServiceStartName,
                                                    string lpPassword);
        #endregion
    }
}