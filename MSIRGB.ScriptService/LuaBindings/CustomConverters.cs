using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService.LuaBindings
{
    class CustomConverters
    {
        public static void Register()
        {
            Script.GlobalOptions.CustomConverters.SetScriptToClrCustomConversion(DataType.Number, typeof(byte), NumberToByte);
        }

        public static object NumberToByte(DynValue v)
        {
            double? n = v.CastToNumber();

            if (!n.HasValue)
                throw ScriptRuntimeException.ConvertToNumberFailed(0);

            if (n.Value < 0 || n.Value > 255)
                throw new ScriptRuntimeException(string.Format("number is out of range (0-255)"));

            return (byte)n.Value;
        }
    }
}