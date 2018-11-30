using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace MSIRGB.Controls
{
    // Colour map logic from: https://github.com/AndreasLill/ColorPickerWPF
    
    public partial class ColourPicker : UserControl
    {
        #region Control events
        public event RoutedEventHandler SelectedColourChanged
        {
            add { AddHandler(SelectedColourChangedEvent, value); }
            remove { RemoveHandler(SelectedColourChangedEvent, value); }
        }

        public static readonly RoutedEvent SelectedColourChangedEvent = 
            EventManager.RegisterRoutedEvent("SelectedColourChanged",
                RoutingStrategy.Direct,
                typeof(RoutedEventHandler),
                typeof(ColourPicker));
        #endregion


        #region Properties
        public double HandleSize
        {
            get { return (double)GetValue(HandleSizeProperty); }
            set { SetValue(HandleSizeProperty, value); }
        }

        public static readonly DependencyProperty HandleSizeProperty =
            DependencyProperty.Register("HandleSize",
                                        typeof(double),
                                        typeof(ColourPicker),
                                        new PropertyMetadata(15.0));


        public Brush HandleBrush
        {
            get { return (Brush)GetValue(HandleBrushProperty); }
            set { SetValue(HandleBrushProperty, value); }
        }

        public static readonly DependencyProperty HandleBrushProperty =
            DependencyProperty.Register("HandleBrush",
                                        typeof(Brush),
                                        typeof(ColourPicker),
                                        new PropertyMetadata(Brushes.Black));


        public double HandleThickness
        {
            get { return (double)GetValue(HandleThicknessProperty); }
            set { SetValue(HandleThicknessProperty, value); }
        }

        public static readonly DependencyProperty HandleThicknessProperty =
            DependencyProperty.Register("HandleThickness",
                                        typeof(double),
                                        typeof(ColourPicker),
                                        new PropertyMetadata(1.0));


        public Color SelectedColour
        {
            get { return (Color)GetValue(SelectedColourProperty); }
            set { SetValue(SelectedColourProperty, value); }
        }

        public static readonly DependencyProperty SelectedColourProperty =
            DependencyProperty.Register("SelectedColour",
                                        typeof(Color),
                                        typeof(ColourPicker),
                                        new PropertyMetadata(Colors.White, OnSelectedColourPropertyChanged));
        #endregion


        private bool _isMouseDown = false;
        private bool _selectedColourInternalPropertyChange = false; // Probably not the best idea, but I don't see any other way of achieving what I need

        public ColourPicker()
        {
            this.InitializeComponent();
            Loaded += new RoutedEventHandler(OnLoaded);
        }

        private static Color GetColourFromColourMap(Point pos)
        {
            double hue = (pos.X / 255) * 360;
            double saturation = 1 - (pos.Y / 255);

            var hsv = new Util.ColorHSV(hue, saturation, 1);

            return hsv.ToRGB();
        }

        private static Point GetPositionFromColourMap(Color c)
        {
            var hsv = new Util.ColorHSV(c);

            return new Point((hsv.Hue / 360) * 255, 255 - hsv.Saturation * 255);
        }

        private void UpdateHandlePosition(Point pos)
        {
            // Make as if handle has point of origin in its center
            pos.X -= Handle.Width / 2;
            pos.Y -= Handle.Height / 2;

            Handle.SetValue(Canvas.LeftProperty, pos.X);
            Handle.SetValue(Canvas.TopProperty, pos.Y);
        }

        private void SetColourPropertyDontCallback(Color c)
        {
            _selectedColourInternalPropertyChange = true;

            SelectedColour = c;

            _selectedColourInternalPropertyChange = false;
        }

        private void RaiseSelectedColourChangedEvent()
        {
            var args = new SelectedColourChangedEventArgs(SelectedColourChangedEvent, SelectedColour);
            RaiseEvent(args);
        }


        #region Events
        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            UpdateHandlePosition(GetPositionFromColourMap(SelectedColour));
        }

        private static void OnSelectedColourPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            // SelectedColour already changed, just update the handle position
            var cp = (ColourPicker)d;

            if (!cp._selectedColourInternalPropertyChange)
            {
                cp.UpdateHandlePosition(ColourPicker.GetPositionFromColourMap((Color)e.NewValue));

                cp.RaiseSelectedColourChangedEvent();
            }
        }

        private void Picker_MouseMove(object sender, MouseEventArgs e)
        {
            if (_isMouseDown)
            {
                var mousePos = Mouse.GetPosition(Canv);

                // Keep handle position within bounds if mouse is out of bounds
                if (mousePos.X > Canv.Width - 1)
                {
                    mousePos.X = Canv.Width - 1;
                }

                if (mousePos.X < 0)
                {
                    mousePos.X = 0;
                }

                if (mousePos.Y > Canv.Height - 1)
                {
                    mousePos.Y = Canv.Height - 1;
                }

                if (mousePos.Y < 0)
                {
                    mousePos.Y = 0;
                }

                SetColourPropertyDontCallback(GetColourFromColourMap(mousePos));

                UpdateHandlePosition(mousePos);

                RaiseSelectedColourChangedEvent();
            }
        }

        private void Picker_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _isMouseDown = true;
            Mouse.Capture(Canv);
        }

        private void Picker_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            _isMouseDown = false;
            Mouse.Capture(null);
        }
        #endregion
    }
}