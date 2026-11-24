using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace HOLLauncher;

/**
 * The launcher's home window (launcher 5, WPF), kept thin: what it shows and enables comes from pure, tested classes
 * (StatusView, ProgressView, PrimaryAction for the big button, HomeMenu for its ⚙ part and the title bar's icons,
 * FooterLine, HomeCards for the information panel), what it runs from the updaters as before.
 * It reads the status in the background (StatusLoader) at start, on "Refresh" and after the settings, and the home
 * content with it (IHomeSource: HomeClient). The big button installs, updates, plays, pauses or resumes a run;
 * "Running" while the game it started is open (MainWindow.Runs.cs: the launcher minimizes, watches the game, comes
 * back and offers a report if it crashed; or closes, closeOnPlay). The runs (game update, repair, launcher update,
 * uninstall) are in MainWindow.Runs.cs, the report and the folders in MainWindow.Report.cs, the panel in
 * MainWindow.Panel.cs.
 * Texts (L16): the fixed ones bound in XAML (UiTexts), the others set here, all in Texts.Language; a language change
 * refreshes both and reads the status again. Theme (L22): the settings' palette as brushes (WpfTheme), applied at start,
 * after "Settings", and when Windows' app theme changes while the setting follows it.
 */
public sealed partial class MainWindow : Window
{
    /** The game's name, the same in every language (title bar, logo, dialogs). */
    public const string GameName = "Heroes of Liberty";

    // Both send the key of servers.json to its manager (L11).
    private HttpClient http;
    private HttpClient downloads;
    private readonly string launcherDir;
    private readonly IHomeSource homeSource;
    private LauncherSettings settings;
    private readonly string? settingsProblem;
    private readonly ContextMenu menu = new();
    private readonly CancellationTokenSource closing = new();
    private StatusView current = StatusView.Loading();
    private ManagerServer? server;
    private PrimaryKind primaryKind = PrimaryKind.Loading;
    private Palette palette = Palette.Light;
    private bool closed;
    private bool started;

    public MainWindow(LauncherOptions options, string launcherDir, SettingsLoad settingsLoad, IHomeSource homeSource)
    {
        InitializeComponent();
        Options = options;
        this.launcherDir = launcherDir;
        this.homeSource = homeSource;
        settings = settingsLoad.Settings;
        settingsProblem = settingsLoad.Problem;
        http = VersionsClient.CreateHttpClient(options.GameDir);
        downloads = FileDownloader.CreateHttpClient(options.GameDir);
        autoUpdatePending = options.UpdateAtStart;
        autoLauncherUpdatePending = options.UpdateLauncherAtStart;
        WindowFrame.Attach(this);
        LogoTop.Text = "Heroes";
        LogoBottom.Text = "of Liberty";
        TitleText.Text = GameName;
        menu.Resources.MergedDictionaries.Add(Resources);
        menu.PlacementTarget = GearButton;
        menu.Placement = PlacementMode.Top;
        menu.VerticalOffset = -6;

        StateChanged += (_, _) => ShowWindowState();
        ContentRendered += async (_, _) => await StartAsync();
        Closing += (_, _) =>
        {
            updating?.Cancel(); // the downloaded part stays for the next try
            closing.Cancel();
        };
        Closed += (_, _) =>
        {
            closed = true;
            Microsoft.Win32.SystemEvents.UserPreferenceChanged -= OnUserPreferenceChanged;
            http.Dispose();
            downloads.Dispose();
        };
        Microsoft.Win32.SystemEvents.UserPreferenceChanged += OnUserPreferenceChanged;
        ApplyTexts();
        ApplyTheme();
        ShowWindowState();
        Display(StatusView.Loading());
    }

    public LauncherOptions Options { get; private set; }

    /** Once, when the window is first drawn: the unreadable settings warning, the crash offer (L20), the reading. */
    private async Task StartAsync()
    {
        if (started)
        {
            return;
        }
        started = true;
        if (settingsProblem is not null)
        {
            Warn(Texts.Format("SettingsUnreadable", settingsProblem));
        }
        await OfferReportAfterCrashAsync();
        await RefreshStatusAsync();
    }

    /** The texts set in code and the font, in Texts.Language (at start and after a language change). */
    private void ApplyTexts()
    {
        Title = Texts.Format("WindowTitle", LauncherInfo.Version);
        FontFamily = new FontFamily(Texts.FontFamily(Texts.Language));
        UiTexts.Instance.Refresh();
        ShowWindowState();
        shownNews = null;
        Render();
        ShowHome();
    }

    /** The settings' palette (L22); the brushes are DynamicResources, so the open window recolours itself. */
    private void ApplyTheme()
    {
        palette = Themes.Current(settings);
        WpfTheme.Apply(this, palette);
    }

    /** Windows' app theme changed (Settings > Personalisation > Colours): followed when the setting says so. */
    private void OnUserPreferenceChanged(object? sender, Microsoft.Win32.UserPreferenceChangedEventArgs e)
    {
        if (e.Category != Microsoft.Win32.UserPreferenceCategory.General || closed)
        {
            return;
        }
        Dispatcher.BeginInvoke(() =>
        {
            if (!closed && Themes.Current(settings) != palette)
            {
                ApplyTheme();
            }
        });
    }

    /** The maximize button's glyph and tooltip, and no border while maximized (the screen's edge is the frame). */
    private void ShowWindowState()
    {
        var maximized = WindowState == WindowState.Maximized;
        MaximizeGlyph.Data = (Geometry)FindResource(maximized ? "IconRestore" : "IconMaximize");
        MaximizeButton.ToolTip = Texts.Get(maximized ? "WindowRestore" : "WindowMaximize");
        Frame.BorderThickness = new Thickness(maximized ? 0 : 1);
    }

    private void Display(StatusView view)
    {
        current = view;
        Render();
    }

    /** Everything that depends on the state: texts, the big button, the icons, the run's bar, the footer, the notes. */
    private void Render()
    {
        StateText.Text = gameRunning ? Texts.Get("GameRunning") : current.StateText;
        LauncherNote.Text = current.LauncherText;
        LauncherNote.Visibility = current.UpdateLauncherEnabled && !gameRunning ? Visibility.Visible : Visibility.Collapsed;
        var action = PrimaryAction.From(current, progressView, gameRunning, busy);
        primaryKind = action.Kind;
        PrimaryButton.Content = action.Label;
        PrimaryButton.IsEnabled = action.Enabled;
        var running = Running;
        RefreshIcon.IsEnabled = HomeMenu.Enabled(MenuCommand.Refresh, current, running, gameRunning);
        SettingsIcon.IsEnabled = HomeMenu.Enabled(MenuCommand.Settings, current, running, gameRunning);
        ReportIcon.IsEnabled = HomeMenu.Enabled(MenuCommand.SendReport, current, running, gameRunning);
        if (progressView is { } line)
        {
            ProgressPanel.Visibility = Visibility.Visible;
            ProgressText.Text = line.Text;
            RunBar.Value = line.Percent;
            CancelButton.IsEnabled = line.CancelEnabled;
        }
        else
        {
            ProgressPanel.Visibility = Visibility.Collapsed;
        }
        var footer = FooterLine.From(current, server);
        FooterText.Text = footer.Text;
        FooterDot.SetResourceReference(System.Windows.Shapes.Shape.FillProperty, footer.Dot switch
        {
            ServerLink.Reached => "ServerReachedBrush",
            ServerLink.Unreachable => "ServerUnreachableBrush",
            _ => "ServerUnknownBrush",
        });
        FooterPanel.ToolTip = footer.Tooltip;
        ShowNews();
    }

    /** A run with or without a bar is under way. */
    private bool Running => progressView is not null || busy || updating is not null;

    private async Task RefreshStatusAsync()
    {
        Display(StatusView.Loading());
        var gameDir = Options.GameDir;
        StatusView view;
        try
        {
            view = await Task.Run(() => StatusLoader.LoadAsync(gameDir, new VersionsClient(http)));
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            // Version.txt or servers.json unreadable on disk: shown, the player can fix it and refresh.
            view = StatusView.Error(e.Message, null);
        }
        server = ReadServer(gameDir);
        if (closed)
        {
            return;
        }
        Display(view);
        _ = LoadHomeAsync();
        if (autoUpdatePending)
        {
            // --update: once, right after the first reading, if an update is there.
            autoUpdatePending = false;
            if (view.UpdateGameEnabled)
            {
                await UpdateGameAsync();
            }
        }
        if (autoLauncherUpdatePending && !closed)
        {
            // --update-launcher: likewise, once.
            autoLauncherUpdatePending = false;
            if (current.UpdateLauncherEnabled)
            {
                await UpdateLauncherAsync();
            }
        }
    }

    /** The first manager of servers.json, null when the file is unreadable (the state line says why). */
    private static ManagerServer? ReadServer(string gameDir)
    {
        try
        {
            return GameServers.ReadFirst(gameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return null;
        }
    }

    // Buttons.

    private async void OnPrimaryClick(object sender, RoutedEventArgs e)
    {
        switch (primaryKind)
        {
            case PrimaryKind.Install or PrimaryKind.Update:
                await UpdateGameAsync();
                break;
            case PrimaryKind.Play:
                Play();
                break;
            case PrimaryKind.Pause or PrimaryKind.Resume:
                await PauseOrResumeAsync();
                break;
        }
    }

    /** The ⚙ menu, built for the state at the moment it opens. */
    private void OnGearClick(object sender, RoutedEventArgs e)
    {
        menu.Items.Clear();
        foreach (var entry in HomeMenu.For(current, Running, gameRunning))
        {
            if (entry.SeparatorBefore)
            {
                menu.Items.Add(new Separator());
            }
            var item = new MenuItem { Header = entry.Text, IsEnabled = entry.Enabled };
            var command = entry.Command;
            item.Click += async (_, _) => await RunCommandAsync(command);
            menu.Items.Add(item);
        }
        menu.FontFamily = FontFamily;
        menu.IsOpen = true;
    }

    private async Task RunCommandAsync(MenuCommand command)
    {
        if (!HomeMenu.Enabled(command, current, Running, gameRunning))
        {
            return;
        }
        switch (command)
        {
            case MenuCommand.PlayAnyway:
                Play();
                break;
            case MenuCommand.Settings:
                await OpenSettingsAsync();
                break;
            case MenuCommand.Repair:
                await RepairGameAsync();
                break;
            case MenuCommand.OpenGameFolder:
                OpenGameFolder();
                break;
            case MenuCommand.OpenLogs:
                OpenLogsFolder();
                break;
            case MenuCommand.SendReport:
                await SendReportAsync();
                break;
            case MenuCommand.Uninstall:
                await UninstallGameAsync();
                break;
            case MenuCommand.UpdateLauncher:
                await UpdateLauncherAsync();
                break;
            case MenuCommand.Refresh:
                await RefreshStatusAsync();
                break;
        }
    }

    private async void OnRefreshClick(object sender, RoutedEventArgs e) => await RunCommandAsync(MenuCommand.Refresh);

    private async void OnSettingsClick(object sender, RoutedEventArgs e) => await RunCommandAsync(MenuCommand.Settings);

    private async void OnReportClick(object sender, RoutedEventArgs e) => await RunCommandAsync(MenuCommand.SendReport);

    private async void OnCancelClick(object sender, RoutedEventArgs e) => await CancelUpdateAsync();

    private void OnTitleBarMouseDown(object sender, MouseButtonEventArgs e) => WindowFrame.OnTitleBarMouseDown(this, e);

    private void OnMinimizeClick(object sender, RoutedEventArgs e) => WindowFrame.Minimize(this);

    private void OnMaximizeClick(object sender, RoutedEventArgs e) => WindowFrame.ToggleMaximize(this);

    private void OnCloseClick(object sender, RoutedEventArgs e) => WindowFrame.Close(this);

    /**
     * Only while nothing runs. The new game folder is made ready first (a failure keeps the old settings); a settings
     * file that cannot be written is said, the choice still applies until the launcher closes.
     */
    private async Task OpenSettingsAsync()
    {
        if (Running || !current.RefreshEnabled)
        {
            return;
        }
        var window = new SettingsWindow(settings, launcherDir, Options.GameDir, palette) { Owner = this };
        if (window.ShowDialog() != true || window.Result is not { } chosen)
        {
            return;
        }
        var gameDir = LauncherOptions.GameDirFrom(chosen, launcherDir);
        var folderChanged = !GameFolder.Same(gameDir, Options.GameDir);
        if (folderChanged)
        {
            try
            {
                GameFolder.Prepare(Options.GameDir, gameDir);
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException or ArgumentException or NotSupportedException)
            {
                Warn(Texts.Format("SettingsBadFolder", e.Message));
                return;
            }
        }
        try
        {
            chosen.Save(launcherDir);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            Warn(Texts.Format("SettingsNotSaved", e.Message));
        }
        settings = chosen;
        Texts.Use(Texts.Resolve(chosen.Language, System.Globalization.CultureInfo.CurrentUICulture));
        if (folderChanged)
        {
            Options = Options with { GameDir = gameDir };
            http.Dispose();
            downloads.Dispose();
            http = VersionsClient.CreateHttpClient(gameDir);
            downloads = FileDownloader.CreateHttpClient(gameDir);
        }
        ApplyTexts();
        ApplyTheme();
        await RefreshStatusAsync();
    }

    // Dialogs, in the language and theme in use.

    private void Inform(string message) => MessageWindow.Show(this, palette, message, DialogKind.Info);

    private void Warn(string message) => MessageWindow.Show(this, palette, message, DialogKind.Warning);

    private bool Ask(string message, bool defaultNo = false)
        => MessageWindow.Show(this, palette, message, DialogKind.Question, yesNo: true, defaultNo: defaultNo);
}
