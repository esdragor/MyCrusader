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
        ApplicationConfiguration.Initialize();
        Application.Run(new MainForm(options));
        return 0;
    }
}
