using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Windows;
using System.Windows.Media;
using static System.Linq.Enumerable;
using MSIRGB.Utils;

namespace MSIRGB
{
    public class MainWindowModel
    {
        private Lighting _lighting;
        private bool _ignoredMbCheck;

        public static ushort STEP_DURATION_MAX_VALUE = Lighting.STEP_DURATION_MAX_VALUE;

        public enum FlashingSpeed
        {
            Disabled = Lighting.FlashingSpeed.Disabled,
            Speed1 = Lighting.FlashingSpeed.Speed1,
            Speed2 = Lighting.FlashingSpeed.Speed2,
            Speed3 = Lighting.FlashingSpeed.Speed3,
            Speed4 = Lighting.FlashingSpeed.Speed4,
            Speed5 = Lighting.FlashingSpeed.Speed5,
            Speed6 = Lighting.FlashingSpeed.Speed6,
        }

        public MainWindowModel()
        {
            CheckForRunningAC();
            CheckForRunningMSIApps();
            TryInitializeDll();
        }

        ~MainWindowModel()
        {
            if (_lighting != null)
            {
                _lighting.Dispose();
            }
        }

        public void GetCurrentConfig(ref List<Color> colours,
                                     out ushort stepDuration,
                                     out bool breathingEnabled,
                                     out bool invertedRChannel,
                                     out bool invertedGChannel,
                                     out bool invertedBChannel,
                                     out FlashingSpeed flashingSpeed)
        {
            foreach (byte index in Range(1, 8))
            {
                Color c = _lighting.GetColour(index).Value;
                c.R *= 0x11; // Colour is exposed as 12-bit depth, but colour picker expects 24-bit depth
                c.G *= 0x11;
                c.B *= 0x11;
                colours.Add(c);
            }

            stepDuration = _lighting.GetStepDuration();

            breathingEnabled = _lighting.IsBreathingModeEnabled();

            invertedRChannel = _lighting.IsRChannelInverted();
            invertedGChannel = _lighting.IsGChannelInverted();
            invertedBChannel = _lighting.IsBChannelInverted();

            flashingSpeed = (FlashingSpeed)_lighting.GetFlashingSpeed();
        }

        public void ApplyConfig(List<Color> colours,
                                ushort stepDuration,
                                bool breathingEnabled,
                                bool invertedRChannel,
                                bool invertedGChannel,
                                bool invertedBChannel,
                                FlashingSpeed flashingSpeed)
        {
            _lighting.BatchBegin();

            foreach (byte index in Range(1, 8))
            {
                Color c = colours[index - 1];
                c.R /= 0x11; // Colour must be passed with 12-bit depth
                c.G /= 0x11;
                c.B /= 0x11;

                if (invertedRChannel) // if inverting colour channels, transform colours from
                // colour picker appropriately (which are never inverted)
                {
                    c.R = (byte)(0x0F - c.R);
                }

                if (invertedGChannel)
                {
                    c.G = (byte)(0x0F - c.G);
                }

                if (invertedBChannel)
                { 
                    c.B = (byte)(0x0F - c.B);
                }

                _lighting.SetColour(index, c);
            }

            _lighting.SetStepDuration(stepDuration);

            // Since breathing mode can't be enabled if flashing was previously enabled
            // we need to set the new flashing speed setting before trying to change breathing mode state
            _lighting.SetFlashingSpeed((Lighting.FlashingSpeed)flashingSpeed);

            _lighting.SetBreathingModeEnabled(breathingEnabled);

            _lighting.SetRChannelInverted(invertedRChannel);
            _lighting.SetGChannelInverted(invertedGChannel);
            _lighting.SetBChannelInverted(invertedBChannel);

            _lighting.BatchEnd();
        }

        public void DisableLighting()
        {
            ScriptService.StopAnyRunningScript();

            _lighting.SetLedEnabled(false);
        }

        public string[] GetScripts()
        {
            return ScriptService.GetScripts();
        }

        public void RunScript(string scriptName)
        {
            ScriptService.RunScript(scriptName, _ignoredMbCheck);
        }

        public string GetRunningScript()
        {
            return ScriptService.GetRunningScript();
        }

        public void StopAnyRunningScript()
        {
            ScriptService.StopAnyRunningScript();
        }

        public void OpenScriptLog()
        {
            try
            {
                Process.Start(ScriptService.GetScriptLogFilePath());
            }
            catch (Exception)
            {
                // Do nothing - File may not exist
            }
        }

        private void TryInitializeDll(bool ignoreMbCheck = false)
        {
            string assemblyTitle = ((AssemblyTitleAttribute)Attribute.GetCustomAttribute(Assembly.GetExecutingAssembly(), typeof(AssemblyTitleAttribute), false))?.Title;

            try
            {
                _lighting = new Lighting(ignoreMbCheck);
            }
            catch (Lighting.Exception exc)
            {
                if (exc.GetErrorCode() == Lighting.ErrorCode.MotherboardModelNotSupported)
                {
                    if (MessageBox.Show("Your motherboard is not on the list of supported motherboards. " +
                                        "Attempting to use this program may cause irreversible damage to your hardware and/or personal data. " +
                                        "Are you sure you want to continue?",
                                        assemblyTitle,
                                        MessageBoxButton.YesNo,
                                        MessageBoxImage.Warning) == MessageBoxResult.Yes)
                    {
                        TryInitializeDll(true);
                        return;
                    }
                }
                else if (exc.GetErrorCode() == Lighting.ErrorCode.MotherboardModelMayOrMayNotBeSupported)
                {
                    if (MessageBox.Show("Your motherboard has been reported to work with MSIRGB but the maintainer of MSIRGB cannot guarantee it works fully and properly himself. " +
                                        "Attempting to use this program may cause irreversible damage to your hardware and/or personal data. " +
                                        "Are you sure you want to continue?",
                                        assemblyTitle,
                                        MessageBoxButton.YesNo,
                                        MessageBoxImage.Warning) == MessageBoxResult.Yes)
                    {
                        TryInitializeDll(true);
                        return;
                    }
                }
                else if (exc.GetErrorCode() == Lighting.ErrorCode.MotherboardVendorNotSupported)
                {
                    MessageBox.Show("Your motherboard's vendor was not detected to be MSI. MSIRGB only supports MSI motherboards. " +
                        "To avoid damage to your hardware, MSIRGB will shutdown. " + Environment.NewLine + Environment.NewLine + 
                        "If your motherboard's vendor is MSI, " + "" +
                        "please report this problem on the issue tracker at: https://github.com/ixjf/MSIRGB",
                        assemblyTitle,
                        MessageBoxButton.OK,
                        MessageBoxImage.Stop
                        );
                }
                else if (exc.GetErrorCode() == Lighting.ErrorCode.DriverLoadFailed)
                {
                    MessageBox.Show("Failed to load driver. This could be either due to some program interfering with MSIRGB's driver, " +
                                    "or it could be a bug. Please make sure you have no anti-cheats installed, like Riot Vanguard or FACEIT. " +
                                    "If the problem persists, report this on the issue tracker at: https://github.com/ixjf/MSIRGB",
                                    assemblyTitle,
                                    MessageBoxButton.OK,
                                    MessageBoxImage.Error
                                    );
                }
                else if (exc.GetErrorCode() == Lighting.ErrorCode.LoadFailed)
                {
                    MessageBox.Show("Failed to load. Please report this on the issue tracker at: https://github.com/ixjf/MSIRGB",
                                    assemblyTitle,
                                    MessageBoxButton.OK,
                                    MessageBoxImage.Error
                                    );
                }

                Application.Current.Shutdown();
            }

            _ignoredMbCheck = ignoreMbCheck;
        }

        private void CheckForRunningMSIApps()
        {
            string assemblyTitle = ((AssemblyTitleAttribute)Attribute.GetCustomAttribute(Assembly.GetExecutingAssembly(), typeof(AssemblyTitleAttribute), false))?.Title;

            Process[] runningMSIProcesses = Process.GetProcessesByName("LEDKeeper");

            if (runningMSIProcesses.Count() > 0)
            {
                switch (MessageBox.Show("MSIRGB detected that an MSI application that could potentially interfere is running. " +
                    "This application is likely either MSI Mystic Light or MSI Gaming App. " + Environment.NewLine + Environment.NewLine +
                    "In order to start MSIRGB, you must stop this application." + Environment.NewLine + Environment.NewLine +
                    "Please make sure that neither of these applications are running at any time simultaneously with MSIRGB. " + "" +
                    "If MSIRGB is set to autostart a script on Windows startup, please make sure neither of these MSI applications are autostarted as well.",
                    assemblyTitle,
                    MessageBoxButton.OKCancel,
                    MessageBoxImage.Warning))
                {
                    case MessageBoxResult.OK:

                        break;

                    case MessageBoxResult.Cancel:
                        Application.Current.Shutdown();
                        break;
                }
            }
        }

        private void CheckForRunningAC()
        {
            string assemblyTitle = ((AssemblyTitleAttribute)Attribute.GetCustomAttribute(Assembly.GetExecutingAssembly(), typeof(AssemblyTitleAttribute), false))?.Title;

            String s = "MSIRGB detected that {0} is running. Anti-cheats do not work well with MSIRGB. Please turn it off first.";
            String acName = null;

            if (ServiceInstaller.IsServiceNotStopped("vgk"))
            {
                acName = "Riot Vanguard";
            }
            else if (ServiceInstaller.IsServiceNotStopped("FACEIT"))
            {
                acName = "FACEIT Anti-Cheat";
            }
            
            if (acName != null)
            {
                MessageBox.Show(String.Format(s, acName), assemblyTitle, MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
            }
        }
    }
}
