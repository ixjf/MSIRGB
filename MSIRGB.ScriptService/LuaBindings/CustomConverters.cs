using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService
{
    class CustomConverters
    {
        public static object NumberToByte(DynValue v)
        {
            double? n = v.CastToNumber();

            if (!n.HasValue)
                throw new ScriptRuntimeException(string.Format("cannot convert a {0} to byte", v.Type.ToLuaTypeString()));

            if (n.Value < 0 || n.Value > 255)
                throw new ScriptRuntimeException(string.Format("cannot convert number to byte (range is 0-255)"));

            return (byte)n.Value;
        }
    }
}
