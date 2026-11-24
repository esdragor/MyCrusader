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
        LauncherOptions options;
        try
        {
            options = LauncherOptions.Parse(args, launcherDir, settings.Settings);
        }
        catch (ArgumentException e)
        {
            MessageBox.Show(e.Message, "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 2;
        }
        if (Environment.ProcessPath is { } exePath)
        {
            // L7: the launcher replaced by the last update, once the one that started this one has closed.
            _ = Task.Run(() => LauncherUpdater.DeleteOldAsync(exePath));
        }
        ApplicationConfiguration.Initialize();
        Application.Run(new MainForm(options, launcherDir, settings));
        return 0;
    }
}
