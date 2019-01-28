using System;
using System.Linq;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Management;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService.LuaBindings
{
    [MoonSharpUserData]
    [MoonSharpHideMember("Register")]
    class Aida64Module
    {
        static readonly List<Regex> allowedLabels = new List<Regex>
        {
            new Regex("SCPUCLK", RegexOptions.Compiled), // CPU Clock
            new Regex("SCC-1-([1-9][0-9]*)", RegexOptions.Compiled), // CPU Core #{0} Clock
            new Regex("SMEMCLK", RegexOptions.Compiled), // Memory Clock
            new Regex("SCPUUTI", RegexOptions.Compiled), // CPU Utilization
            new Regex("SCPU([1-9][0-9]*)UTI", RegexOptions.Compiled), // CPU{0} Utilization
            new Regex("SMEMUTI", RegexOptions.Compiled), // Memory Utilization
            new Regex("SVIRTMEMUTI", RegexOptions.Compiled), // Virtual Memory Utilization
            new Regex("SDSK([1-9][0-9]*)ACT", RegexOptions.Compiled), // Disk {0} Activity
            new Regex("SGPU([1-9][0-9]*)CLK", RegexOptions.Compiled), // GPU {0} Clock
            new Regex("SGPU([1-9][0-9]*)MEMCLK", RegexOptions.Compiled), // GPU {0} Memory Clock
            new Regex("SGPU([1-9][0-9]*)UTI", RegexOptions.Compiled), // GPU {0} Utilization
            new Regex("SVMEMUSAGE", RegexOptions.Compiled), // Video Memory Utilization
            new Regex("SGPU([1-9][0-9]*)USEDDEMEM", RegexOptions.Compiled), // GPU {0} Used Dedicated Memory
            new Regex("SGPU([1-9][0-9]*)USEDDYMEM", RegexOptions.Compiled), // GPU {0} Used Dynamic Memory
            new Regex("TMOBO", RegexOptions.Compiled), // Motherboard (temperature)
            new Regex("TCPU", RegexOptions.Compiled), // CPU (temperature)
            new Regex("TCPUDIO", RegexOptions.Compiled), // CPU Diode (temperature)
            new Regex("TGPU([1-9][0-9]*)DIO", RegexOptions.Compiled), // GPU {0} Diode (temperature)
            new Regex("THDD([1-9][0-9]*)", RegexOptions.Compiled), // HDD (temperature) - not supported on trial versions
            new Regex("FCPU", RegexOptions.Compiled), // CPU (fan speed)
            new Regex("FCHA([1-9][0-9]*)", RegexOptions.Compiled), // Chassis #{0} (fan speed)
            new Regex("FGPU([1-9][0-9]*)", RegexOptions.Compiled), // GPU {0} (fan speed)
            new Regex("VCPU", RegexOptions.Compiled), // CPU Core (voltage) - not supported on trial versions
            new Regex("VGPU([1-9][0-9]*)", RegexOptions.Compiled), // GPU {0} Core (voltage)
            new Regex("PCPUPKG", RegexOptions.Compiled), // CPU Package (wattage)
            new Regex("PCPUVDD", RegexOptions.Compiled), // CPU VDD (wattage)
            new Regex("PCPUVDDNB", RegexOptions.Compiled), // CPU VDDNB (wattage)
            new Regex("PGPU([1-9][0-9]*)TDPP", RegexOptions.Compiled), // GPU {0} TDP%
        };

        public static void Register(Script script)
        {
            UserData.RegisterType(typeof(Aida64Module));

            script.Globals["Aida64"] = new Aida64Module();
        }

        public bool IsInstalledAndRunning()
        {
            try
            {
                using (var searcher = new ManagementObjectSearcher(@"root\wmi", "SELECT * FROM AIDA64_SensorValues"))
                {
                    return searcher.Get().Count != 0;
                }
            }
            catch (ManagementException e)
            {
                if (e.ErrorCode == ManagementStatus.InvalidClass)
                {
                    return false;
                }
                else
                {
                    throw e;
                }
            }
        }

        public DynValue GetSensorValue(string label)
        {
            if (allowedLabels.Find(x => x.Match(label).Success) == null)
            {
                throw ScriptRuntimeException.BadArgument(0, "GetSensorValue", "label is not valid");
            }

            using (var searcher = new ManagementObjectSearcher(@"root\wmi", string.Format("SELECT Value FROM AIDA64_SensorValues WHERE ID='{0}'", label)))
            {
                try
                {
                    var value = searcher.Get().Cast<ManagementBaseObject>().First();

                    try
                    {
                        return DynValue.NewNumber(Convert.ToDouble(value["Value"]));
                    }
                    catch (FormatException)
                    {
                        // Some values are not supported in trial versions
                        return DynValue.NewBoolean(false);
                    }
                }
                catch (InvalidOperationException)
                {
                    // Valid label, but may not apply with this hardware (e.g. specific cpu core data)
                    return DynValue.NewBoolean(false);
                }
                catch (ManagementException e)
                {
                    if (e.ErrorCode == ManagementStatus.InvalidClass)
                    {
                        throw new ScriptRuntimeException("GetSensorValue called but AIDA64 is not running");
                    }
                    else
                    {
                        throw e;
                    }
                }
            }
        }
    }
}