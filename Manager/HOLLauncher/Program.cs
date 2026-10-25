namespace HOLLauncher;

internal static class Program
{
    [STAThread]
    private static int Main(string[] args)
    {
        LauncherOptions options;
        try
        {
            options = LauncherOptions.Parse(args, AppContext.BaseDirectory);
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
        Application.Run(new MainForm(options));
        return 0;
    }
}
