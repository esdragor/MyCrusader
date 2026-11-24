namespace HOLLauncher;

/**
 * Command line: --game-dir <folder> (default: the settings' game folder, else the launcher's folder, the launcher sits
 * next to the game; L15); --update starts the game update as soon as the window has read the status (tests);
 * --update-launcher likewise for the launcher's own update (L7, tests); --tray (L23) starts hidden in the notification
 * area (the Run key of "Start with Windows" passes it), the window opens from the icon.
 * GameDirArgument: the --game-dir given on this command line (null: none), which the Run key repeats (StartWithWindows).
 */
public sealed record LauncherOptions(string GameDir, bool UpdateAtStart = false, bool UpdateLauncherAtStart = false,
    bool StartInTray = false, string? GameDirArgument = null)
{
    public static LauncherOptions Parse(IReadOnlyList<string> args, string launcherDir, LauncherSettings? settings = null)
    {
        var gameDir = GameDirFrom(settings ?? LauncherSettings.Default, launcherDir);
        string? gameDirArgument = null;
        var update = false;
        var updateLauncher = false;
        var tray = false;
        for (var i = 0; i < args.Count; i++)
        {
            if (args[i] == "--game-dir")
            {
                if (i + 1 >= args.Count)
                {
                    throw new ArgumentException("--game-dir needs a folder");
                }
                gameDir = gameDirArgument = args[++i];
            }
            else if (args[i] == "--update")
            {
                update = true;
            }
            else if (args[i] == LauncherUpdater.UpdateLauncherOption)
            {
                updateLauncher = true;
            }
            else if (args[i] == StartWithWindows.TrayOption)
            {
                tray = true;
            }
            else
            {
                throw new ArgumentException(
                    $"unknown argument '{args[i]}' (only --game-dir <folder>, --update, --update-launcher and --tray)");
            }
        }
        return new LauncherOptions(gameDir, update, updateLauncher, tray, gameDirArgument);
    }

    /** The settings' game folder (made full against the launcher's folder), else the launcher's folder. */
    public static string GameDirFrom(LauncherSettings settings, string launcherDir)
        => settings.GameDir is { Length: > 0 } dir ? Path.GetFullPath(dir, Path.GetFullPath(launcherDir)) : Path.GetFullPath(launcherDir);
}
