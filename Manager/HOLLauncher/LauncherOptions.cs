namespace HOLLauncher;

/**
 * Command line: --game-dir <folder> (default: the launcher's folder, the launcher sits next to the game); --update
 * starts the game update as soon as the window has read the status (tests); --update-launcher likewise for the
 * launcher's own update (L7, tests).
 */
public sealed record LauncherOptions(string GameDir, bool UpdateAtStart = false, bool UpdateLauncherAtStart = false)
{
    public static LauncherOptions Parse(IReadOnlyList<string> args, string launcherDir)
    {
        var gameDir = Path.GetFullPath(launcherDir);
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
}
