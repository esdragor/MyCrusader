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
        if (Environment.ProcessPath is { } exePath)
        {
            // L7: the launcher replaced by the last update, once the one that started this one has closed.
            _ = Task.Run(() => LauncherUpdater.DeleteOldAsync(exePath));
        }
        var window = new MainWindow(options, launcherDir, settings, new ManagerHomeSource(launcherDir));
        app.MainWindow = window;
        app.ShutdownMode = ShutdownMode.OnMainWindowClose;
        app.Run(window);
        return 0;
    }
}
