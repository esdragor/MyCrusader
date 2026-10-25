namespace HOLLauncher;

/**
 * Command line: --game-dir <folder> (default: the launcher's folder, the launcher sits next to the game); --update
 * starts the game update as soon as the window has read the status (tests, and later the launcher's own update, L10).
 */
public sealed record LauncherOptions(string GameDir, bool UpdateAtStart = false)
{
    public static LauncherOptions Parse(IReadOnlyList<string> args, string launcherDir)
    {
        var gameDir = Path.GetFullPath(launcherDir);
        var update = false;
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
            else
            {
                throw new ArgumentException($"unknown argument '{args[i]}' (only --game-dir <folder> and --update)");
            }
        }
        return new LauncherOptions(gameDir, update);
    }
}
