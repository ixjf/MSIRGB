using System;
using System.Windows.Media;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService.LuaBindings
{
    [MoonSharpUserData]
    class LightingModule
    {
        private Sio _sio;

        public LightingModule(bool ignoreMbCheck)
        {
            _sio = new Sio(ignoreMbCheck); // This should just crash if it fails
        }

        ~LightingModule()
        {
            _sio.Dispose();
        }

        public void SetAllEnabled(bool enabled)
        {
            _sio.SetLedEnabled(enabled);
        }

        public void SetColour(byte index, byte r, byte g, byte b)
        {
            if (!Enum.IsDefined(typeof(Sio.ColourIndex), index))
                throw new ScriptRuntimeException("invalid value for parameter 'index' (range is 1-4)");

            _sio.SetColour((Sio.ColourIndex)index, Color.FromRgb(r, g, b));
        }

        public DynValue GetColour(byte index)
        {
            if (!Enum.IsDefined(typeof(Sio.ColourIndex), index))
                throw new ScriptRuntimeException("invalid value for parameter 'index' (range is 1-4)");

            Color c = _sio.GetColour((Sio.ColourIndex)index);

            return DynValue.NewTuple(new DynValue[] 
            {
                DynValue.NewNumber(c.R),
                DynValue.NewNumber(c.G),
                DynValue.NewNumber(c.B)
            });
        }

        public bool SetBreathingModeEnabled(bool enabled)
        {
            return _sio.SetBreathingModeEnabled(enabled);
        }

        public bool IsBreathingModeEnabled()
        {
            return _sio.IsBreathingModeEnabled();
        }

        public void SetFlashingSpeed(byte flashingSpeed)
        {
            if (!Enum.IsDefined(typeof(Sio.FlashingSpeed), flashingSpeed))
                throw new ScriptRuntimeException("invalid value for parameter 'flashingSpeed' (range is 0-6)");

            _sio.SetFlashingSpeed((Sio.FlashingSpeed)flashingSpeed);
        }

        public byte GetFlashingSpeed()
        {
            return (byte)_sio.GetFlashingSpeed();
        }

        public void SetStepDuration(ushort stepDuration)
        {
            if (stepDuration > Sio.STEP_DURATION_MAX_VALUE)
                throw new ScriptRuntimeException(String.Format("invalid value for parameter 'stepDuration' (range is 0-{0})", Sio.STEP_DURATION_MAX_VALUE));

            _sio.SetStepDuration(stepDuration);
        }

        public ushort GetStepDuration()
        {
            return _sio.GetStepDuration();
        }
    }
}