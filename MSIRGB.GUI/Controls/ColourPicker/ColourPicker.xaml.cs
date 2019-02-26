using System.Globalization;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace MSIRGB.Controls
{
    public partial class ColourPicker : UserControl
    {
        #region Properties
        public Color SelectedColour
        {
            get { return (Color)GetValue(SelectedColourProperty); }
            set { SetValue(SelectedColourProperty, value); }
        }

        protected static readonly DependencyProperty SelectedColourProperty =
            DependencyProperty.Register(
                "SelectedColour",
                typeof(Color),
                typeof(ColourPicker),
                new PropertyMetadata(Colors.White, Internal_OnSelectedColourChanged)
            );
        #endregion

        #region Events
        public event RoutedEventHandler SelectedColourChanged
        {
            add { AddHandler(SelectedColourChangedEvent, value); }
            remove { RemoveHandler(SelectedColourChangedEvent, value); }
        }

        public static readonly RoutedEvent SelectedColourChangedEvent =
            EventManager.RegisterRoutedEvent("SelectedColourChanged",
                RoutingStrategy.Direct,
                typeof(RoutedEventHandler),
                typeof(ColourPicker)
            );
        #endregion

        public ColourPicker()
        {
            InitializeComponent();
        }

        private void UpdateSliderPosition(Color c)
        {
            RedChannelSlider.Value = c.R / 0x11;
            GreenChannelSlider.Value = c.G / 0x11;
            BlueChannelSlider.Value = c.B / 0x11;
        }

        private void UpdateTextBoxes(Color c)
        {
            RedChannelTextBox.Text = string.Format("{0:x}", c.R / 0x11);
            GreenChannelTextBox.Text = string.Format("{0:x}", c.G / 0x11);
            BlueChannelTextBox.Text = string.Format("{0:x}", c.B / 0x11);
        }

        private Color ColourFromTextBoxes()
        {
            byte r = (byte)(byte.Parse(RedChannelTextBox.Text, NumberStyles.HexNumber) * 0x11);
            byte g = (byte)(byte.Parse(GreenChannelTextBox.Text, NumberStyles.HexNumber) * 0x11);
            byte b = (byte)(byte.Parse(BlueChannelTextBox.Text, NumberStyles.HexNumber) * 0x11);

            return Color.FromRgb(r, g, b);
        }

        private Color ColourFromSliders()
        {
            byte r = (byte)(RedChannelSlider.Value * 0x11);
            byte g = (byte)(GreenChannelSlider.Value * 0x11);
            byte b = (byte)(BlueChannelSlider.Value * 0x11);

            return Color.FromRgb(r, g, b);
        }
        
        private static void Internal_OnSelectedColourChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var cp = (ColourPicker)d;

            var c = ((Color)e.NewValue);
            cp.UpdateSliderPosition(c);
            cp.UpdateTextBoxes(c);

            var args = new SelectedColourChangedEventArgs(SelectedColourChangedEvent, c);
            cp.RaiseEvent(args);
        }

        private void TextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            if (!Regex.IsMatch(e.Text, "([0-9]|[a-f])"))
            {
                e.Handled = true;
            }
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            // We only want to catch events from after the UI is loaded
            // (events triggered before the UI is loaded are from setting initial values)
            if (this.IsLoaded)
            {
                {
                    var textBox = (TextBox)sender;

                    if (string.IsNullOrEmpty(textBox.Text))
                    {
                        textBox.Text = "0";
                    }
                    else if (!byte.TryParse(textBox.Text, NumberStyles.HexNumber, null, out byte value) || value > 0xf)
                    {
                        var caretIndex = textBox.CaretIndex;
                        textBox.Text = textBox.Text.Remove(caretIndex - 1, 1);
                        textBox.CaretIndex = caretIndex;
                    }
                }

                SelectedColour = ColourFromTextBoxes();
            }
        }

        private void Slider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            SelectedColour = ColourFromSliders();
        }
    }
}