using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;
using HOLUpdate;

namespace HOLLauncher;

/**
 * The launcher in the background (L23): the notification-area icon (TrayIcon) and its menu (TrayMenu, the launcher's
 * themed WPF menu), the close button that hides the window there (minimizeToTray; a one-time notification says where it
 * went; "Quit" really quits), the start hidden in it (--tray), the background update checks (CheckSchedule +
 * BackgroundUpdatePolicy: a notification, or the game update started by itself with its bar, "Pause" and "Cancel" in
 * the window), "Start with Windows" (StartWithWindows) and the second start that brings this window to the front
 * (SingleInstance).
 * A run's result is a dialog when the window is shown, a notification when it is hidden (TellResult).
 */
public sealed partial class MainWindow
{
    private readonly SingleInstance? instance;
    private readonly ContextMenu trayMenu = new();
    private readonly CheckSchedule schedule = new(() => DateTime.UtcNow);
    // Wakes up every minute; CheckSchedule decides whether a check is due (the interval may change in the settings).
    private readonly DispatcherTimer checkTimer = new() { Interval = TimeSpan.FromMinutes(1) };
    private TrayIcon tray = null!;
    private BackgroundMemory memory = BackgroundMemory.Empty;
    private WindowState shownState = WindowState.Normal;
    private bool quitting;
    private bool checking;

    /** Visible and not minimized: the player can see what the window says. */
    private bool WindowShown => IsVisible && WindowState != WindowState.Minimized;

    private void SetUpTray()
    {
        tray = new TrayIcon((Geometry)FindResource("IconEmblem"), GameName);
        tray.OpenRequested += ShowFromTray;
        tray.MenuRequested += ShowTrayMenu;
        trayMenu.Resources.MergedDictionaries.Add(Resources);
        trayMenu.Placement = PlacementMode.MousePoint;
        trayMenu.Opened += (_, _) =>
        {
            // A menu of a process that is not in the foreground would not close on a click elsewhere.
            if (PresentationSource.FromVisual(trayMenu) is HwndSource source)
            {
                SetForegroundWindow(source.Handle);
            }
        };
        checkTimer.Tick += async (_, _) => await BackgroundCheckAsync(manual: false);
        StateChanged += (_, _) =>
        {
            if (WindowState != WindowState.Minimized)
            {
                shownState = WindowState;
            }
        };
        if (Application.Current is { } app)
        {
            // Windows' logoff or shutdown: really quit, never hide. The icon goes whatever ends the application.
            app.SessionEnding += (_, _) => quitting = true;
            app.Exit += (_, _) => CloseTray();
        }
        instance?.Listen(() => Dispatcher.BeginInvoke(new Action(ShowFromTray)));
        ApplyStartWithWindows(warn: false);
    }

    /** --tray: the window stays hidden (its handle exists, so that it can close), the status is read at once. */
    public void StartInTray()
    {
        new WindowInteropHelper(this).EnsureHandle();
        _ = Dispatcher.InvokeAsync(StartReadingAsync);
    }

    /** Really closes the launcher (tray "Quit", closeOnPlay, the launcher's own update), whatever minimizeToTray says. */
    private void Quit()
    {
        quitting = true;
        Close();
    }

    private void CloseTray()
    {
        checkTimer.Stop();
        trayMenu.IsOpen = false;
        tray.Dispose();
        instance?.Dispose();
    }

    /** The close button with minimizeToTray: hidden, and the first time ever a notification says where it went. */
    private void HideToTray()
    {
        Hide();
        if (settings.TrayHintShown == true)
        {
            return;
        }
        tray.Notify(Texts.Get("TrayHiddenTitle"), Texts.Get("TrayHiddenText"), ShowFromTray);
        settings = settings with { TrayHintShown = true };
        if (settingsProblem is null)
        {
            try
            {
                settings.Save(launcherDir);
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException)
            {
                // A read-only launcher folder: the notification comes again at the next start, no more.
            }
        }
    }

    /** The window back in front: from the icon, a notification, a second start of the launcher. */
    private void ShowFromTray()
    {
        if (closed)
        {
            return;
        }
        if (!IsVisible)
        {
            Show();
        }
        if (WindowState == WindowState.Minimized)
        {
            WindowState = shownState;
        }
        Activate();
    }

    /** The icon's menu, built for the state at the moment it opens, where the mouse is. */
    private void ShowTrayMenu()
    {
        if (closed)
        {
            return;
        }
        trayMenu.Items.Clear();
        foreach (var entry in TrayMenu.For(current, Running, gameRunning))
        {
            if (entry.SeparatorBefore)
            {
                trayMenu.Items.Add(new Separator());
            }
            var item = new MenuItem
            {
                Header = entry.Text,
                IsEnabled = entry.Enabled,
                FontWeight = entry.Bold ? FontWeights.SemiBold : FontWeights.Normal,
            };
            var command = entry.Command;
            item.Click += async (_, _) => await RunTrayCommandAsync(command);
            trayMenu.Items.Add(item);
        }
        trayMenu.FontFamily = FontFamily;
        trayMenu.IsOpen = true;
    }

    private async Task RunTrayCommandAsync(TrayCommand command)
    {
        if (!TrayMenu.Enabled(command, current, Running, gameRunning))
        {
            return;
        }
        switch (command)
        {
            case TrayCommand.Open:
                ShowFromTray();
                break;
            case TrayCommand.Play:
                if (current.PlayConfirmation is not null)
                {
                    ShowFromTray(); // the question is asked over the window
                }
                Play();
                break;
            case TrayCommand.CheckForUpdates:
                if (WindowShown)
                {
                    await RefreshStatusAsync(); // the window shows the answer
                }
                else
                {
                    await BackgroundCheckAsync(manual: true);
                }
                break;
            case TrayCommand.Quit:
                Quit();
                break;
        }
    }

    /**
     * A background check: due by CheckSchedule (or asked, manual), never two at once nor during the window's own
     * reading. The window shows the new state when nothing runs (no "Loading" flash); then BackgroundUpdatePolicy says
     * whether to notify or install. The game counts as running even when started outside the launcher.
     */
    private async Task BackgroundCheckAsync(bool manual)
    {
        if (closed || checking || !readingStarted || current.IsLoading)
        {
            return;
        }
        if (!manual && (BackgroundUpdatePolicy.ModeOf(settings.AutoUpdate) == AutoUpdateMode.Off
            || !schedule.IsDue(settings.CheckEveryMinutes)))
        {
            return;
        }
        checking = true;
        schedule.Checked();
        StatusReading reading;
        bool gameOpen;
        try
        {
            var gameDir = Options.GameDir;
            var client = new VersionsClient(http);
            reading = await Task.Run(() => StatusLoader.ReadAsync(gameDir, client));
            gameOpen = gameRunning || await Task.Run(GameSession.IsGameRunning);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or ObjectDisposedException)
        {
            return; // the next check, or the player's "Refresh", says it
        }
        finally
        {
            checking = false;
        }
        if (closed)
        {
            return;
        }
        if (!Running && !gameRunning && !current.IsLoading)
        {
            Display(reading.View);
        }
        var decision = BackgroundUpdatePolicy.Decide(settings, reading.Status, gameOpen, Running, WindowShown, memory, manual);
        memory = decision.Memory;
        if (decision.LauncherVersion is { } launcher)
        {
            tray.Notify(Texts.Format("TrayLauncherAvailable", launcher), Texts.Get("TrayLauncherAvailableText"), ShowFromTray);
        }
        switch (decision.Game)
        {
            case BackgroundAction.Notify:
                var title = reading.Status?.Installed is null ? "TrayGameAvailable" : "TrayUpdateAvailable";
                tray.Notify(Texts.Format(title, VersionName.Display(decision.Version!)), Texts.Get("TrayClickToOpen"), ShowFromTray);
                break;
            case BackgroundAction.Install:
                if (!WindowShown)
                {
                    tray.Notify(Texts.Format("TrayUpdating", VersionName.Display(decision.Version!)), Texts.Get("TrayUpdatingText"),
                        ShowFromTray);
                }
                await UpdateGameAsync();
                break;
            default:
                if (manual && decision.LauncherVersion is null)
                {
                    tray.Notify(GameName, reading.View.StateText, ShowFromTray); // up to date, or why it could not tell
                }
                break;
        }
    }

    /** A run's result: a dialog over the shown window, a notification while it is hidden or minimized. */
    private void TellResult(bool success, string message)
    {
        if (WindowShown)
        {
            if (success)
            {
                Inform(message);
            }
            else
            {
                Warn(message);
            }
            return;
        }
        tray.Notify(Texts.Get(success ? "TrayUpdateDone" : "TrayUpdateFailed"), message, ShowFromTray);
    }

    /**
     * The Run key agrees with the setting (at start, quietly; after "Save" in the settings, a refusal is said). Not when
     * the launcher runs through dotnet.exe (a development start): the key would start dotnet.
     */
    private void ApplyStartWithWindows(bool warn)
    {
        if (Environment.ProcessPath is not { } exePath
            || string.Equals(Path.GetFileNameWithoutExtension(exePath), "dotnet", StringComparison.OrdinalIgnoreCase))
        {
            return;
        }
        try
        {
            StartWithWindows.Sync(new RegistryRunKey(), settings.StartWithWindows == true, exePath, Options.GameDirArgument);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or System.Security.SecurityException)
        {
            if (warn)
            {
                Warn(Texts.Format("SettingsStartWithWindowsFailed", e.Message));
            }
        }
    }

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool SetForegroundWindow(IntPtr window);
}
