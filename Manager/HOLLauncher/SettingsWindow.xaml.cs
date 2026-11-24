using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace HOLLauncher;

/**
 * The "Settings" window (L15, L19, L22, launcher 5), modal and thin: SettingsModel gives the lists and what they show
 * for the stored settings, and makes the result of "Save" (Result: the old settings with these fields changed) or the
 * problem that keeps the window open. "Browse…" is WPF's folder dialog; "Launcher folder" puts the default back. The
 * window shows the main window's palette; a new theme or language applies once saved (MainWindow). L23: the
 * "Background" section (automatic updates and their interval, the notification area, start with Windows).
 */
public sealed partial class SettingsWindow : Window
{
    private readonly LauncherSettings settings;
    private readonly string launcherDir;
    private readonly Palette palette;
    private readonly string language = Texts.Language;

    public SettingsWindow(LauncherSettings settings, string launcherDir, string currentGameDir, Palette palette)
    {
        InitializeComponent();
        this.settings = settings;
        this.launcherDir = launcherDir;
        this.palette = palette;
        WpfTheme.Apply(this, palette);
        // L23: never taller than the screen's work area (the fields scroll); SizeToContent keeps it shorter when it fits.
        MaxHeight = Math.Max(400, SystemParameters.WorkArea.Height - 16);
        Title = Texts.Get("Settings");
        FontFamily = new FontFamily(Texts.FontFamily(language));
        TimesText.Text = "×";
        LanguageBox.ItemsSource = SettingsModel.Languages();
        ThemeBox.ItemsSource = SettingsModel.ThemeChoices(language);
        DisplayBox.ItemsSource = SettingsModel.DisplayChoices(language);
        ResolutionBox.ItemsSource = SettingsModel.ResolutionChoices(language);
        var fields = SettingsModel.Fields(settings, currentGameDir, language);
        LanguageBox.SelectedIndex = fields.LanguageIndex;
        ThemeBox.SelectedIndex = fields.ThemeIndex;
        GameDirBox.Text = fields.GameDir;
        DisplayBox.SelectedIndex = fields.DisplayIndex;
        ResolutionBox.SelectedIndex = fields.ResolutionIndex;
        WidthBox.Text = fields.Width;
        HeightBox.Text = fields.Height;
        ExtraBox.Text = fields.ExtraArguments;
        CloseOnPlayBox.IsChecked = fields.CloseOnPlay;
        AutoUpdateBox.ItemsSource = SettingsModel.AutoUpdateChoices(language);
        AutoUpdateBox.SelectedIndex = fields.AutoUpdateIndex;
        IntervalBox.Text = fields.CheckEveryMinutes;
        MinimizeToTrayBox.IsChecked = fields.MinimizeToTray;
        StartWithWindowsBox.IsChecked = fields.StartWithWindows;
        ShowResolution();
        ShowInterval();
    }

    /** L23: the interval matters only when the launcher checks ("off" greys it out, its value is kept). */
    private void ShowInterval()
        => IntervalBox.IsEnabled = AutoUpdateBox.SelectedItem is AutoUpdateChoice { Mode: not AutoUpdateMode.Off };

    private void OnAutoUpdateChanged(object sender, SelectionChangedEventArgs e) => ShowInterval();

    /** The settings chosen ("Save"); null if the window was cancelled. */
    public LauncherSettings? Result { get; private set; }

    /** The two sizes are typed only for "Custom"; otherwise they show the size chosen in the list. */
    private void ShowResolution()
    {
        if (ResolutionBox.SelectedItem is not ResolutionChoice choice)
        {
            return;
        }
        WidthBox.IsEnabled = HeightBox.IsEnabled = choice.Custom;
        if (choice.Size is var (w, h))
        {
            WidthBox.Text = w.ToString(System.Globalization.CultureInfo.InvariantCulture);
            HeightBox.Text = h.ToString(System.Globalization.CultureInfo.InvariantCulture);
        }
    }

    private void OnResolutionChanged(object sender, SelectionChangedEventArgs e) => ShowResolution();

    private void OnBrowseClick(object sender, RoutedEventArgs e)
    {
        var dialog = new Microsoft.Win32.OpenFolderDialog
        {
            Title = Texts.Get("SettingsBrowseDescription"),
            InitialDirectory = Directory.Exists(GameDirBox.Text) ? GameDirBox.Text : launcherDir,
        };
        if (dialog.ShowDialog(this) == true)
        {
            GameDirBox.Text = dialog.FolderName;
        }
    }

    private void OnLauncherDirClick(object sender, RoutedEventArgs e)
        => GameDirBox.Text = Path.TrimEndingDirectorySeparator(Path.GetFullPath(launcherDir));

    private void OnSaveClick(object sender, RoutedEventArgs e)
    {
        var fields = new SettingsFields(LanguageBox.SelectedIndex, ThemeBox.SelectedIndex, GameDirBox.Text, DisplayBox.SelectedIndex,
            ResolutionBox.SelectedIndex, WidthBox.Text, HeightBox.Text, ExtraBox.Text, CloseOnPlayBox.IsChecked == true,
            MinimizeToTrayBox.IsChecked == true, AutoUpdateBox.SelectedIndex, IntervalBox.Text, StartWithWindowsBox.IsChecked == true);
        var outcome = SettingsModel.Build(settings, fields, launcherDir, language);
        if (outcome.Problem is { } problem)
        {
            MessageWindow.Show(this, palette, problem, DialogKind.Warning);
            return;
        }
        Result = outcome.Result;
        DialogResult = true;
    }

    private void OnCancelClick(object sender, RoutedEventArgs e) => DialogResult = false;
}
