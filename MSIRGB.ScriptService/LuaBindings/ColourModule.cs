using System;
using MoonSharp.Interpreter;

namespace MSIRGB.ScriptService.LuaBindings
{
    [MoonSharpUserData]
    class ColourModule
    {
        public DynValue HSVtoRGB(double h, double s, double v)
        {
            if (h > 1.0)
                throw ScriptRuntimeException.BadArgument(0, "HSVtoRGB", "range is [0, 1]");

            if (s > 1.0)
                throw ScriptRuntimeException.BadArgument(1, "HSVtoRGB", "range is [0, 1]");

            if (v > 1.0)
                throw ScriptRuntimeException.BadArgument(2, "HSVtoRGB", "range is [0, 1]");

            if (s == 0.0)
                return DynValue.NewTuple(new DynValue[]
                {
                    DynValue.NewNumber(v),
                    DynValue.NewNumber(v),
                    DynValue.NewNumber(v),
                });

            var i = Math.Floor(h * 6.0);

            var f = (h * 6.0) - i;
            var p = v * (1.0 - s);
            var q = v * (1.0 - s * f);
            var t = v * (1.0 - s * (1.0 - f));

            i %= 6;

            if (i == 0)
                return DynValue.NewTuple(new DynValue[]
                {
                    DynValue.NewNumber(v),
                    DynValue.NewNumber(t),
                    DynValue.NewNumber(p),
                });

            if (i == 1)
                return DynValue.NewTuple(new DynValue[]
                {
                    DynValue.NewNumber(q),
                    DynValue.NewNumber(v),
                    DynValue.NewNumber(p),
                });
            else if (i == 2)
                return DynValue.NewTuple(new DynValue[]
                {
                    DynValue.NewNumber(p),
                    DynValue.NewNumber(v),
                    DynValue.NewNumber(t),
                });
            else if (i == 3)
                return DynValue.NewTuple(new DynValue[]
                {
                    DynValue.NewNumber(p),
                    DynValue.NewNumber(q),
                    DynValue.NewNumber(v),
                });
            else if (i == 4)
                return DynValue.NewTuple(new DynValue[]
                {
                    DynValue.NewNumber(t),
                    DynValue.NewNumber(p),
                    DynValue.NewNumber(v),
                });
            else // if (i == 5)
                return DynValue.NewTuple(new DynValue[]
                {
                    DynValue.NewNumber(v),
                    DynValue.NewNumber(p),
                    DynValue.NewNumber(q),
                });
        }
    }
}
