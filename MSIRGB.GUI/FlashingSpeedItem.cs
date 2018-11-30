using System.Windows;

namespace MSIRGB
{
    public class FlashingSpeedItem : DependencyObject
    {
        #region Properties
        public MainWindowModel.FlashingSpeed Type { get; set; }
        public string DisplayName { get; set; }
        #endregion
    }
}