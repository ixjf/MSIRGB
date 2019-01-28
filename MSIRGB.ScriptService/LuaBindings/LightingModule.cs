using System;
using System.Windows.Media;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService.LuaBindings
{
    [MoonSharpUserData]
    [MoonSharpHideMember("Register")]
    class LightingModule
    {
        private Sio _sio;

        public static void Register(Script script, bool ignoreMbCheck)
        {
            UserData.RegisterType(typeof(LightingModule));

            script.Globals["Lighting"] = new LightingModule(ignoreMbCheck);
        }

        private LightingModule(bool ignoreMbCheck)
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
            if (index < 1 || index > 8)
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("SetColour", 0);

            _sio.SetColour(index, Color.FromRgb(r, g, b));
        }

        public DynValue GetColour(byte index)
        {
            if (index < 1 || index > 8)
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("GetColour", 0);

            Color c = _sio.GetColour(index).Value;

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
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("SetFlashingSpeed", 0);

            _sio.SetFlashingSpeed((Sio.FlashingSpeed)flashingSpeed);
        }

        public byte GetFlashingSpeed()
        {
            return (byte)_sio.GetFlashingSpeed();
        }

        public void SetStepDuration(ushort stepDuration)
        {
            if (stepDuration > Sio.STEP_DURATION_MAX_VALUE)
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("SetStepDuration", 0);

            _sio.SetStepDuration(stepDuration);
        }

        public ushort GetStepDuration()
        {
            return _sio.GetStepDuration();
        }
    }
}