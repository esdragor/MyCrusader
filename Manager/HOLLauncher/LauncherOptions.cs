namespace HOLLauncher;

/** Command line: --game-dir <folder> (default: the launcher's folder, the launcher sits next to the game). */
public sealed record LauncherOptions(string GameDir)
{
    public static LauncherOptions Parse(IReadOnlyList<string> args, string launcherDir)
    {
        var gameDir = Path.GetFullPath(launcherDir);
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
            else
            {
                throw new ArgumentException($"unknown argument '{args[i]}' (only --game-dir <folder>)");
            }
        }
        return new LauncherOptions(gameDir);
    }
}
