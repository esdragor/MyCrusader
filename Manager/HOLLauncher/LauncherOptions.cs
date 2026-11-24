namespace HOLLauncher;

/**
 * Command line: --game-dir <folder> (default: the settings' game folder, else the launcher's folder, the launcher sits
 * next to the game; L15); --update starts the game update as soon as the window has read the status (tests);
 * --update-launcher likewise for the launcher's own update (L7, tests).
 */
public sealed record LauncherOptions(string GameDir, bool UpdateAtStart = false, bool UpdateLauncherAtStart = false)
{
    public static LauncherOptions Parse(IReadOnlyList<string> args, string launcherDir, LauncherSettings? settings = null)
    {
        var gameDir = GameDirFrom(settings ?? LauncherSettings.Default, launcherDir);
        var update = false;
        var updateLauncher = false;
        for (var i = 0; i < args.Count; i++)
        {
            if (args[i] == "--game-dir")
            {
                if (i + 1 >= args.Count)
                {
                    throw new ArgumentException("--game-dir needs a folder");
                }
                gameDir = args[++i];
            }
            else if (args[i] == "--update")
            {
                update = true;
            }
            else if (args[i] == LauncherUpdater.UpdateLauncherOption)
            {
                updateLauncher = true;
            }
            else
            {
                throw new ArgumentException(
                    $"unknown argument '{args[i]}' (only --game-dir <folder>, --update and --update-launcher)");
            }
        }
        return new LauncherOptions(gameDir, update, updateLauncher);
    }

    /** The settings' game folder (made full against the launcher's folder), else the launcher's folder. */
    public static string GameDirFrom(LauncherSettings settings, string launcherDir)
        => settings.GameDir is { Length: > 0 } dir ? Path.GetFullPath(dir, Path.GetFullPath(launcherDir)) : Path.GetFullPath(launcherDir);
}
