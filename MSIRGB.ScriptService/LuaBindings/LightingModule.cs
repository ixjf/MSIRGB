using System;
using System.Windows.Media;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService.LuaBindings
{
    [MoonSharpUserData]
    [MoonSharpHideMember("Register")]
    class LightingModule
    {
        private Lighting _lighting;

        public static void Register(Script script, bool ignoreMbCheck)
        {
            UserData.RegisterType(typeof(LightingModule));

            script.Globals["Lighting"] = new LightingModule(ignoreMbCheck);
        }

        private LightingModule(bool ignoreMbCheck)
        {
            _lighting = new Lighting(ignoreMbCheck); // This should just crash if it fails
        }

        ~LightingModule()
        {
            _lighting.Dispose();
        }

        public void SetAllEnabled(bool enabled)
        {
            _lighting.SetLedEnabled(enabled);
        }

        public bool BatchBegin()
        {
            return _lighting.BatchBegin();
        }

        public void SetColour(byte index, byte r, byte g, byte b)
        {
            if (index < 1 || index > 8)
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("SetColour", 0);

            _lighting.SetColour(index, Color.FromRgb(r, g, b));
        }

        public DynValue GetColour(byte index)
        {
            if (index < 1 || index > 8)
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("GetColour", 0);

            Color c = _lighting.GetColour(index).Value;

            return DynValue.NewTuple(new DynValue[] 
            {
                DynValue.NewNumber(c.R),
                DynValue.NewNumber(c.G),
                DynValue.NewNumber(c.B)
            });
        }

        public bool SetBreathingModeEnabled(bool enabled)
        {
            return _lighting.SetBreathingModeEnabled(enabled);
        }

        public bool IsBreathingModeEnabled()
        {
            return _lighting.IsBreathingModeEnabled();
        }

        public void SetFlashingSpeed(byte flashingSpeed)
        {
            if (!Enum.IsDefined(typeof(Lighting.FlashingSpeed), flashingSpeed))
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("SetFlashingSpeed", 0);

            _lighting.SetFlashingSpeed((Lighting.FlashingSpeed)flashingSpeed);
        }

        public byte GetFlashingSpeed()
        {
            return (byte)_lighting.GetFlashingSpeed();
        }

        public void SetStepDuration(ushort stepDuration)
        {
            if (stepDuration > Lighting.STEP_DURATION_MAX_VALUE)
                throw ScriptRuntimeException.BadArgumentIndexOutOfRange("SetStepDuration", 0);

            _lighting.SetStepDuration(stepDuration);
        }

        public ushort GetStepDuration()
        {
            return _lighting.GetStepDuration();
        }

        public bool BatchEnd()
        {
            return _lighting.BatchEnd();
        }
    }
}