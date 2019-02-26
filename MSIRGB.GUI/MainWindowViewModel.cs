using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Media;
using static System.Linq.Enumerable;
using Prism.Commands;

namespace MSIRGB
{
    public class MainWindowViewModel : DependencyObject
    {
        #region Bindings
        public static readonly DependencyProperty ColourRTextProperty = DependencyProperty.Register(
            "ColourRText",
            typeof(string),
            typeof(MainWindowViewModel),
            new PropertyMetadata("255"));

        public static readonly DependencyProperty ColourGTextProperty = DependencyProperty.Register(
            "ColourGText",
            typeof(string),
            typeof(MainWindowViewModel),
            new PropertyMetadata("255"));

        public static readonly DependencyProperty ColourBTextProperty = DependencyProperty.Register(
            "ColourBText",
            typeof(string),
            typeof(MainWindowViewModel),
            new PropertyMetadata("255"));

        public static readonly DependencyProperty StepDurationTextProperty = DependencyProperty.Register(
            "StepDurationText",
            typeof(string),
            typeof(MainWindowViewModel),
            new PropertyMetadata("0"));

        public static readonly DependencyProperty BreathingModeEnabledProperty = DependencyProperty.Register(
            "BreathingModeEnabled",
            typeof(bool),
            typeof(MainWindowViewModel),
            new PropertyMetadata(false));

        public static readonly DependencyProperty FlashingSpeedIndexProperty = DependencyProperty.Register(
            "FlashingSpeedIndex",
            typeof(int),
            typeof(MainWindowViewModel),
            new PropertyMetadata(0));

        public static readonly DependencyProperty PickerSelectedColourProperty = DependencyProperty.Register(
            "PickerSelectedColour",
            typeof(Color),
            typeof(MainWindowViewModel));

        public static readonly DependencyProperty ListSelectedColourIndexProperty = DependencyProperty.Register(
            "ListSelectedColourIndex",
            typeof(int),
            typeof(MainWindowViewModel),
            new PropertyMetadata(-1));

        public static readonly DependencyProperty ScriptsSelectedIndexProperty = DependencyProperty.Register(
            "ScriptsSelectedIndex",
            typeof(int),
            typeof(MainWindowViewModel),
            new PropertyMetadata(-1));

        public static readonly DependencyProperty StopRunningScriptBtnEnabledProperty = DependencyProperty.Register(
            "StopRunningScriptBtnEnabled",
            typeof(bool),
            typeof(MainWindowViewModel),
            new PropertyMetadata(false));

        public string ColourRText
        {
            get { return (string)GetValue(ColourRTextProperty); }
            set { SetValue(ColourRTextProperty, value); }
        }

        public string ColourGText
        {
            get { return (string)GetValue(ColourGTextProperty); }
            set { SetValue(ColourGTextProperty, value); }
        }

        public string ColourBText
        {
            get { return (string)GetValue(ColourBTextProperty); }
            set { SetValue(ColourBTextProperty, value); }
        }

        public string StepDurationText
        {
            get { return (string)GetValue(StepDurationTextProperty); }
            set { SetValue(StepDurationTextProperty, value); }
        }

        public bool BreathingModeEnabled
        {
            get { return (bool)GetValue(BreathingModeEnabledProperty); }
            set { SetValue(BreathingModeEnabledProperty, value); }
        }

        public int FlashingSpeedSelectedIndex
        {
            get { return (int)GetValue(FlashingSpeedIndexProperty); }
            set { SetValue(FlashingSpeedIndexProperty, value); }
        }

        public Color PickerSelectedColour
        {
            get { return (Color)GetValue(PickerSelectedColourProperty); }
            set { SetValue(PickerSelectedColourProperty, value); }
        }

        public int ListSelectedColourIndex
        {
            get { return (int)GetValue(ListSelectedColourIndexProperty); }
            set { SetValue(ListSelectedColourIndexProperty, value); }
        }

        public int ScriptsSelectedIndex
        {
            get { return (int)GetValue(ScriptsSelectedIndexProperty); }
            set { SetValue(ScriptsSelectedIndexProperty, value); }
        }

        public bool StopRunningScriptBtnEnabled
        {
            get { return (bool)GetValue(StopRunningScriptBtnEnabledProperty); }
            set { SetValue(StopRunningScriptBtnEnabledProperty, value); }
        }

        public ObservableCollection<ColourItem> ColourList { get; set; }
        public ObservableCollection<FlashingSpeedItem> FlashingSpeedList { get; set; }
        public ObservableCollection<ScriptItem> ScriptsList { get; set; }
        #endregion

        private MainWindowModel _model;

        public MainWindowViewModel()
        {
            _model = new MainWindowModel();

            ColourList = new ObservableCollection<ColourItem>()
            {
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 1" },
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 2" },
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 3" },
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 4" },
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 5" },
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 6" },
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 7" },
                new ColourItem() { Background = new SolidColorBrush(Colors.White), DisplayName = "Colour 8" },
            };

            FlashingSpeedList = new ObservableCollection<FlashingSpeedItem>()
            {
                new FlashingSpeedItem() { Type = MainWindowModel.FlashingSpeed.Disabled, DisplayName = "Disabled" },
                new FlashingSpeedItem() { Type = MainWindowModel.FlashingSpeed.Speed1, DisplayName = "Fastest" },
                new FlashingSpeedItem() { Type = MainWindowModel.FlashingSpeed.Speed2, DisplayName = "Almost fastest" },
                new FlashingSpeedItem() { Type = MainWindowModel.FlashingSpeed.Speed3, DisplayName = "Fast" },
                new FlashingSpeedItem() { Type = MainWindowModel.FlashingSpeed.Speed4, DisplayName = "Somewhat fast" },
                new FlashingSpeedItem() { Type = MainWindowModel.FlashingSpeed.Speed5, DisplayName = "Not quite slow" },
                new FlashingSpeedItem() { Type = MainWindowModel.FlashingSpeed.Speed6, DisplayName = "Slow" }
            };

            ScriptsList = new ObservableCollection<ScriptItem>();

            ApplyCommand = new DelegateCommand(Apply);
            DisableLightingCommand = new DelegateCommand(DisableLighting);
            OpenScriptLogCommand = new DelegateCommand(OpenScriptLog);
            StopRunningScriptCommand = new DelegateCommand(StopRunningScript);
        }

        #region Commands
        public DelegateCommand ApplyCommand { get; private set; }
        public DelegateCommand DisableLightingCommand { get; private set; }
        public DelegateCommand OpenScriptLogCommand { get; private set; }
        public DelegateCommand StopRunningScriptCommand { get; private set; }

        public void Apply()
        {
            if (FlashingSpeedSelectedIndex != 0) // If flashing enabled
                BreathingModeEnabled = false;

            ScriptsSelectedIndex = -1;

            var colours = ColourList.Select(ci => ((SolidColorBrush)ci.Background).Color).ToList();

            _model.ApplyConfig(colours, 
                               Convert.ToUInt16(StepDurationText), 
                               BreathingModeEnabled, 
                               FlashingSpeedList[FlashingSpeedSelectedIndex].Type);
        }

        public void DisableLighting()
        {
            ScriptsSelectedIndex = -1;

            _model.DisableLighting();
        }

        public void OpenScriptLog()
        {
            _model.OpenScriptLog();
        }

        public void StopRunningScript()
        {
            ScriptsSelectedIndex = -1;
        }
        #endregion

        #region Events
        public void LoadedEvent()
        {
            var colours = new List<Color>();

            _model.GetCurrentConfig(ref colours,
                                    out ushort stepDuration,
                                    out bool breathingEnabled,
                                    out MainWindowModel.FlashingSpeed flashingSpeed);

            foreach(Byte index in Range(1, 8))
            {
                ColourList[index - 1].Background = new SolidColorBrush(colours[index - 1]);
            }

            StepDurationText = Convert.ToString(stepDuration);

            BreathingModeEnabled = breathingEnabled;

            FlashingSpeedSelectedIndex = FlashingSpeedList.IndexOf(FlashingSpeedList.Single(x => x.Type == flashingSpeed));

            ListSelectedColourIndex = 0;

            foreach(var s in _model.GetScripts())
            {
                ScriptsList.Add(new ScriptItem() { Name = s });
            }

            ScriptsSelectedIndex = ScriptsList.IndexOf(ScriptsList.FirstOrDefault(x => x.Name == _model.GetRunningScript()));

            if (ScriptsSelectedIndex != -1)
                StopRunningScriptBtnEnabled = true;
        }

        public void ListSelectedColourChangedEvent()
        {
            if (ListSelectedColourIndex != -1)
            {
                PickerSelectedColour = ((SolidColorBrush)ColourList[ListSelectedColourIndex].Background).Color;

                //ColourRText = Convert.ToDecimal(PickerSelectedColour.R).ToString();
                //ColourGText = Convert.ToDecimal(PickerSelectedColour.G).ToString();
                //ColourBText = Convert.ToDecimal(PickerSelectedColour.B).ToString();
            }
        }

        public void PickerSelectedColourChangedEvent()
        {
            if (ListSelectedColourIndex != -1)
            {
                ColourRText = Convert.ToDecimal(PickerSelectedColour.R).ToString();
                ColourGText = Convert.ToDecimal(PickerSelectedColour.G).ToString();
                ColourBText = Convert.ToDecimal(PickerSelectedColour.B).ToString();

                ColourList[ListSelectedColourIndex].Background = new SolidColorBrush(PickerSelectedColour);
            }
        }

        public bool TextBoxIsDecNumberInput(string text)
        {
            // Do not allow entering non decimals
            if (!Regex.IsMatch(text, "^[0-9]+$"))
            {
                return false;
            }

            return true;
        }

        public void StepDurationTextBoxTextChangedEvent()
        {
            // Is the value in valid range?
            if (String.IsNullOrEmpty(StepDurationText))
            {
                StepDurationText = "0";
                return;
            }

            if (!Int16.TryParse(StepDurationText, out short value) || value > MainWindowModel.STEP_DURATION_MAX_VALUE)
            {
                StepDurationText = Convert.ToString(MainWindowModel.STEP_DURATION_MAX_VALUE);
                return;
            }
        }

        public void ScriptsListSelectedItemChangedEvent()
        {
            if (ScriptsSelectedIndex != -1)
            {
                _model.RunScript(ScriptsList[ScriptsSelectedIndex].Name);
                StopRunningScriptBtnEnabled = true;
            }
            else
            {
                _model.StopAnyRunningScript();
                StopRunningScriptBtnEnabled = false;
            }
        }
        #endregion
    }
}
