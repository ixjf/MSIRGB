using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows;
using System.Windows.Media;

namespace MSIRGB
{
    public class MainWindowModel
    {
        private Sio _sio;
        private bool _ignoredMbCheck;

        public static UInt16 STEP_DURATION_MAX_VALUE = Sio.STEP_DURATION_MAX_VALUE;

        public enum FlashingSpeed
        {
            Disabled = Sio.FlashingSpeed.Disabled,
            Speed1 = Sio.FlashingSpeed.Speed1,
            Speed2 = Sio.FlashingSpeed.Speed2,
            Speed3 = Sio.FlashingSpeed.Speed3,
            Speed4 = Sio.FlashingSpeed.Speed4,
            Speed5 = Sio.FlashingSpeed.Speed5,
            Speed6 = Sio.FlashingSpeed.Speed6,
        }

        public MainWindowModel()
        {
            TryInitializeSio();
        }

        ~MainWindowModel()
        {
            if (_sio != null)
                _sio.Dispose();
        }

        public void GetCurrentConfig(out Color c1, 
                                     out Color c2, 
                                     out Color c3, 
                                     out Color c4, 
                                     out UInt16 stepDuration, 
                                     out bool breathingEnabled, 
                                     out FlashingSpeed flashingSpeed)
        {
            c1 = _sio.GetColour(Sio.ColourIndex.Colour1);
            c2 = _sio.GetColour(Sio.ColourIndex.Colour2);
            c3 = _sio.GetColour(Sio.ColourIndex.Colour3);
            c4 = _sio.GetColour(Sio.ColourIndex.Colour4);

            stepDuration = _sio.GetStepDuration();

            breathingEnabled = _sio.IsBreathingModeEnabled();

            flashingSpeed = (FlashingSpeed)_sio.GetFlashingSpeed();
        }

        public void ApplyConfig(Color c1, 
                                Color c2, 
                                Color c3, 
                                Color c4, 
                                UInt16 stepDuration, 
                                bool breathingEnabled, 
                                FlashingSpeed flashingSpeed)
        {
            _sio.SetColour(Sio.ColourIndex.Colour1, c1);
            _sio.SetColour(Sio.ColourIndex.Colour2, c2);
            _sio.SetColour(Sio.ColourIndex.Colour3, c3);
            _sio.SetColour(Sio.ColourIndex.Colour4, c4);

            _sio.SetStepDuration(stepDuration);

            // Since breathing mode can't be enabled if flashing was previously enabled
            // we need to set the new flashing speed setting before trying to change breathing mode state
            _sio.SetFlashingSpeed((Sio.FlashingSpeed)flashingSpeed);

            _sio.SetBreathingModeEnabled(breathingEnabled);
        }

        public void DisableLighting()
        {
            ScriptService.StopAnyRunningScript();

            _sio.SetLedEnabled(false);
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

        private void TryInitializeSio(bool ignoreMbCheck = false)
        {
            string assemblyTitle = ((AssemblyTitleAttribute)Attribute.GetCustomAttribute(Assembly.GetExecutingAssembly(), typeof(AssemblyTitleAttribute), false))?.Title;

            try
            {
                _sio = new Sio(ignoreMbCheck);
            }
            catch (Sio.Exception exc)
            {
                if (exc.GetErrorCode() == Sio.ErrorCode.MotherboardNotSupported)
                {
                    if (!ignoreMbCheck)
                    {
                        if (MessageBox.Show("Your motherboard is not on the list of supported motherboards. " +
                                            "Attempting to use this program may cause irreversible damage to your hardware and/or personal data. " +
                                            "Are you sure you want to continue?",
                                            assemblyTitle,
                                            MessageBoxButton.YesNo,
                                            MessageBoxImage.Warning) == MessageBoxResult.Yes)
                        {
                            TryInitializeSio(true);
                            return;
                        }
                    }
                }
                else if (exc.GetErrorCode() == Sio.ErrorCode.DriverLoadFailed)
                {
                    MessageBox.Show("Failed to load driver.", assemblyTitle, MessageBoxButton.OK, MessageBoxImage.Error);
                }

                Application.Current.Shutdown();
            }

            _ignoredMbCheck = ignoreMbCheck;
        }
    }
}
