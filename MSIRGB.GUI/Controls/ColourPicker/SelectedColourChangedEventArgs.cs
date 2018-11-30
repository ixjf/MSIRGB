using System.Windows;
using System.Windows.Media;

namespace MSIRGB.Controls
{
    public partial class ColourPicker
    {
        public partial class SelectedColourChangedEventArgs : RoutedEventArgs
        {
            public SelectedColourChangedEventArgs(RoutedEvent routedEvent, Color selectedColour)
                : base(routedEvent)
            {
                SelectedColour = selectedColour;
            }

            public Color SelectedColour { get; }
        }
    }
}
