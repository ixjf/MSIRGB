using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace MSIRGB
{
    public partial class MainWindowView : Window
    {
        public MainWindowView()
        {
            InitializeComponent();
        }

        #region Window behaviour
        public void DragWindowEvent(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                DragMove();
            }
        }

        public void CloseWindowCommand(object sender, RoutedEventArgs e)
        {
            Close();
        }

        public void MinimizeWindowCommand(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }
        #endregion

        #region Routed events
        public new void LoadedEvent(object sender, RoutedEventArgs e)
        {
            ((MainWindowViewModel)DataContext).LoadedEvent();
        }

        public void ListSelectedColourChangedEvent(object sender, SelectionChangedEventArgs e)
        {
            ((MainWindowViewModel)DataContext).ListSelectedColourChangedEvent();
        }

        public void PickerSelectedColourChangedEvent(object sender, RoutedEventArgs e)
        {
            ((MainWindowViewModel)DataContext).PickerSelectedColourChangedEvent();
        }

        public void TextBoxIsDecNumberInput(object sender, TextCompositionEventArgs e)
        {
            if (!((MainWindowViewModel)DataContext).TextBoxIsDecNumberInput(e.Text))
            {
                e.Handled = true;
                return;
            }
        }

        public void ColourCodeTextBoxTextChangedEvent(object sender, TextChangedEventArgs e)
        {
            ((MainWindowViewModel)DataContext).ColourCodeTextBoxTextChangedEvent((string)((Control)sender).Tag);
        }

        public void StepDurationTextBoxTextChangedEvent(object sender, TextChangedEventArgs e)
        {
            ((MainWindowViewModel)DataContext).StepDurationTextBoxTextChangedEvent();
        }

        public void ScriptsListSelectedItemChangedEvent(object sender, SelectionChangedEventArgs e)
        {
            ((MainWindowViewModel)DataContext).ScriptsListSelectedItemChangedEvent();
        }

        public void ScriptsListItemPreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            // Disallow deselecting
            e.Handled = ((ListBoxItem)sender).IsSelected;
        }
        #endregion
    }
}
