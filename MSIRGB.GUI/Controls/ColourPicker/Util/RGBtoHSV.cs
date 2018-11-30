using System;
using System.Windows.Media;

namespace MSIRGB.Controls.Util
{
    class ColorHSV
    {
        /////////////////////////////////////////////////////////
        /// Properties
        ////////////////////////////////////////////////////////
        /// 
        private double _hue;
        public double Hue
        {
            get => _hue;
            set
            {
                _hue = (value >= 0 && value < 360) ? value : 0;
            }
        }

        public double Saturation
        {
            get;
            set;
        }

        public double Brightness
        {
            get;
            set;
        }

        public ColorHSV(double h, double s, double b)
        {
            Hue = h;
            Saturation = s;
            Brightness = b;
        }

        public ColorHSV(Color c)
        {
            double r = ((double)c.R / 255.0);
            double g = ((double)c.G / 255.0);
            double b = ((double)c.B / 255.0);

            double max = Math.Max(r, Math.Max(g, b));
            double min = Math.Min(r, Math.Min(g, b));

            if (max == r && g >= b)
            {
                Hue = 60 * (g - b) / (max - min);
            }
            else if (max == r && g < b)
            {
                Hue = 60 * (g - b) / (max - min) + 360;
            }
            else if (max == g)
            {
                Hue = 60 * (b - r) / (max - min) + 120;
            }
            else if (max == b)
            {
                Hue = 60 * (r - g) / (max - min) + 240;
            }
            else
            {
                Hue = 0.0;
            }

            Saturation = (max == 0.0) ? 0.0 : (1.0 - (min / max));

            Brightness = max;
        }

        public Color ToRGB()
        {
            double r = 0;
            double g = 0;
            double b = 0;

            if (Saturation == 0)
            {
                r = g = b = Brightness;
            }
            else
            {
                // Calculate the color sector
                double sectorPos = Hue / 60.0;
                int sectorNumber = (int)Math.Floor(sectorPos);

                // Calculate the fractional
                double fractionalSector = sectorPos - sectorNumber;

                // Calculate the three axes of color
                double p = Brightness * (1.0 - Saturation);
                double q = Brightness * (1.0 - (Saturation * fractionalSector));
                double t = Brightness * (1.0 - (Saturation * (1 - fractionalSector)));

                // Assign fractional colors to RGB based on the sector angle
                switch (sectorNumber)
                {
                    case 0:
                        r = Brightness;
                        g = t;
                        b = p;
                        break;

                    case 1:
                        r = q;
                        g = Brightness;
                        b = p;
                        break;

                    case 2:
                        r = p;
                        g = Brightness;
                        b = t;
                        break;

                    case 3:
                        r = p;
                        g = q;
                        b = Brightness;
                        break;

                    case 4:
                        r = t;
                        g = p;
                        b = Brightness;
                        break;

                    case 5:
                        r = Brightness;
                        g = p;
                        b = q;
                        break;
                }
            }

            return Color.FromRgb((byte)Double.Parse(String.Format("{0:0.00}", r * 255.0)),
                                 (byte)Double.Parse(String.Format("{0:0.00}", g * 255.0)),
                                 (byte)Double.Parse(String.Format("{0:0.00}", b * 255.0)));
        }
    }
}
