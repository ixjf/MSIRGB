using System;
using System.Windows;
using System.Windows.Media;

namespace MSIRGB
{
    public class ColourItem : DependencyObject
    {
        #region Properties
        public static readonly DependencyProperty BackgroundProperty = DependencyProperty.Register(
            "Background",
            typeof(Brush),
            typeof(ColourItem));

        public static readonly DependencyProperty DisplayNameProperty = DependencyProperty.Register(
            "DisplayName",
            typeof(String),
            typeof(ColourItem));

        public Brush Background
        {
            get { return (Brush)GetValue(BackgroundProperty); }
            set { SetValue(BackgroundProperty, value); }
        }

        public String DisplayName
        {
            get { return (String)GetValue(DisplayNameProperty); }
            set { SetValue(DisplayNameProperty, value); }
        }
        #endregion
    }
}