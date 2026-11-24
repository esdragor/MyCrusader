namespace HOLLauncher;

/**
 * Where the packaged game writes its logs (L17, "Open the logs folder"). Unreal writes them to
 * <project>\Saved\Logs: for the package of Tools\Package\package.ps1 (Development by default) that is
 * <game>\HeroesOfLiberty\Saved\Logs; a Shipping package counts as "installed" for Unreal (FApp::IsInstalled) and writes
 * to %LOCALAPPDATA%\HeroesOfLiberty\Saved\Logs instead. The playtest .bat files (-saveddirsuffix=_<name>) write to
 * Saved_<name>\Logs: not looked for, the folder of a normal start is the one the player sends.
 */
public static class GameLogs
{
    private const string Project = "HeroesOfLiberty";

    /** The folder beside the game, the usual one (Development package). */
    public static string ExpectedDir(string gameDir) => Path.Combine(gameDir, Project, "Saved", "Logs");

    /** The first existing of the folder beside the game and the one under localAppData; null when the game has none yet. */
    public static string? FindDir(string gameDir, string localAppData)
        => new[] { ExpectedDir(gameDir), Path.Combine(localAppData, Project, "Saved", "Logs") }.FirstOrDefault(Directory.Exists);

    /** The same with the player's %LOCALAPPDATA%. */
    public static string? FindDir(string gameDir)
        => FindDir(gameDir, Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
}
