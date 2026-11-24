namespace HOLLauncher;

/**
 * A change of game folder in the settings (L15). The new folder is created and receives a copy of the old folder's
 * servers.json (the kit's manager and access key, L11) when it has none, so that the launcher still reaches its manager;
 * the game already installed is not moved (a new folder starts as "not installed", the update downloads it).
 */
public static class GameFolder
{
    /** IOException / UnauthorizedAccessException / ArgumentException when the folder cannot be made ready. */
    public static void Prepare(string oldDir, string newDir)
    {
        Directory.CreateDirectory(newDir);
        var servers = Path.Combine(oldDir, GameServers.FileName);
        var target = Path.Combine(newDir, GameServers.FileName);
        if (!Same(oldDir, newDir) && File.Exists(servers) && !File.Exists(target))
        {
            File.Copy(servers, target);
        }
    }

    /** Same folder, ignoring case and a trailing separator (Windows). */
    public static bool Same(string a, string b)
        => string.Equals(Normalize(a), Normalize(b), StringComparison.OrdinalIgnoreCase);

    private static string Normalize(string dir) => Path.TrimEndingDirectorySeparator(Path.GetFullPath(dir));
}
