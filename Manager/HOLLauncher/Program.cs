using System.Windows;

namespace HOLLauncher;

internal static class Program
{
    [STAThread]
    private static int Main(string[] args)
    {
        // L15: settings beside the exe, then the language (setting, else Windows', else English), then the command line.
        var launcherDir = AppContext.BaseDirectory;
        var settings = LauncherSettings.Load(launcherDir);
        Texts.Use(Texts.Resolve(settings.Settings.Language, System.Globalization.CultureInfo.CurrentUICulture));
        // Launcher 5: WPF. The application exists before any window (the styles' pack URIs need it).
        var app = new Application { ShutdownMode = ShutdownMode.OnExplicitShutdown };
        LauncherOptions options;
        try
        {
            options = LauncherOptions.Parse(args, launcherDir, settings.Settings);
        }
        catch (ArgumentException e)
        {
            // L16: the reason stays technical (English), the sentence around it is the player's language.
            MessageWindow.Show(null, Themes.Current(settings.Settings), Texts.Format("BadCommandLine", e.Message), DialogKind.Error);
            return 2;
        }
        // L23: one launcher per game folder; a second start brings the first one's window to the front and ends here.
        if (Claim(options) is not { } instance)
        {
            return 0;
        }
        using (instance)
        {
            if (Environment.ProcessPath is { } exePath)
            {
                // L7: the launcher replaced by the last update, once the one that started this one has closed.
                _ = Task.Run(() => LauncherUpdater.DeleteOldAsync(exePath));
            }
            var window = new MainWindow(options, launcherDir, settings, new ManagerHomeSource(launcherDir), instance);
            app.MainWindow = window;
            app.ShutdownMode = ShutdownMode.OnMainWindowClose;
            if (options.StartInTray)
            {
                // L23: --tray (Start with Windows): hidden in the notification area, the window opens from the icon.
                window.StartInTray();
                app.Run();
            }
            else
            {
                app.Run(window);
            }
        }
        return 0;
    }

    /**
     * This start's claim on its game folder, or null when another launcher holds it (that one was asked to show its
     * window, unless this is a --tray start). Two tries: the other launcher may close between the claim and the signal.
     */
    private static SingleInstance? Claim(LauncherOptions options)
    {
        var name = SingleInstance.NameFor(options.GameDir);
        for (var attempt = 0; attempt < 2; attempt++)
        {
            if (SingleInstance.Claim(name) is { } claimed)
            {
                return claimed;
            }
            if (!SingleInstance.ShowsFirst(options) || SingleInstance.Signal(name))
            {
                return null;
            }
        }
        return null;
    }
}
