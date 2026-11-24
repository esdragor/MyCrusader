using HOLUpdate;

namespace HOLLauncher;

/** The end of an uninstall: success, the message for the player, the number of game files deleted. */
public sealed record UninstallResult(bool Success, string Message, int Deleted);

/**
 * "Uninstall the game" (L17), after the window's confirmation. Deletes only the game's files: those of installed.json
 * (the list this launcher installed) and of the list given (the manager's latest list, for a game installed from a zip
 * without installed.json: its paths are still the game's), the version file (GameFiles.VersionFile) and the work
 * folder .holupdate. Never the launcher's files (UpdatePlan.LauncherOwned: launcher exe, hpatchz.exe, servers.json,
 * the launcher's settings) nor any file no list names (the player's own: HeroesOfLiberty\Saved, his notes...); the
 * folders this leaves empty are removed, the game folder itself stays (the launcher usually lives in it). No list at
 * all: nothing is deleted (the game's files cannot be told from the player's). A file in use (the game running): nothing
 * is deleted either, checked before the first deletion. Never throws to the window.
 */
public static class GameUninstaller
{
    /** No installed.json: the window then fetches the manager's latest list to give to Uninstall. */
    public static bool NeedsServerList(string gameDir) => UpdatePlan.ReadInstalledList(gameDir) is null;

    public static UninstallResult Uninstall(string gameDir, VersionManifest? serverList = null)
    {
        var root = Path.GetFullPath(gameDir);
        var work = Path.Combine(root, UpdatePlan.WorkDirName);
        var installedList = UpdatePlan.ReadInstalledList(root);
        var versionFile = GamePath(root, GameFiles.VersionFile);
        if (!File.Exists(versionFile) && installedList is null && !Directory.Exists(work))
        {
            return new UninstallResult(false, Texts.Format("UninstallNothing", gameDir), 0);
        }
        if (installedList is null && serverList is null)
        {
            return new UninstallResult(false, Texts.Get("UninstallNoList"), 0);
        }
        var paths = new[] { installedList, serverList }
            .SelectMany(list => list?.Files ?? [])
            .Select(f => f.Path)
            .Append(GameFiles.VersionFile)
            .Where(path => !UpdatePlan.IsLauncherOwned(path))
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Select(path => (Path: path, File: GamePath(root, path)))
            .Where(p => IsUnder(root, p.File))
            .ToList();
        try
        {
            foreach (var (path, file) in paths)
            {
                if (!IsFree(file))
                {
                    return new UninstallResult(false, Texts.Format("UninstallLocked", path), 0);
                }
            }
            var deleted = 0;
            // The version file last: a failure halfway still shows an installed (damaged) game, which "Repair" or a new
            // uninstall can handle.
            foreach (var (_, file) in paths.OrderBy(p => p.File == versionFile))
            {
                if (File.Exists(file))
                {
                    File.SetAttributes(file, FileAttributes.Normal);
                    File.Delete(file);
                    deleted++;
                }
            }
            if (Directory.Exists(work))
            {
                Directory.Delete(work, recursive: true);
            }
            foreach (var (_, file) in paths)
            {
                RemoveEmptyFolders(root, Path.GetDirectoryName(file)!);
            }
            return new UninstallResult(true, Texts.Format("UninstallDone", deleted), deleted);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            return new UninstallResult(false, Texts.Format("UninstallFailed", e.Message), 0);
        }
    }

    /** Missing, or openable alone (the running game keeps its exe and paks open). */
    private static bool IsFree(string file)
    {
        if (!File.Exists(file))
        {
            return true;
        }
        try
        {
            using var stream = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.None);
            return true;
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            return false;
        }
    }

    /** Removes dir and its parents while they are empty, up to the game folder (kept). */
    private static void RemoveEmptyFolders(string root, string dir)
    {
        while (IsUnder(root, dir) && !Same(root, dir) && Directory.Exists(dir) && !Directory.EnumerateFileSystemEntries(dir).Any())
        {
            Directory.Delete(dir);
            dir = Path.GetDirectoryName(dir)!;
        }
    }

    private static string GamePath(string root, string path) => Path.GetFullPath(Path.Combine(root, path.Replace('/', Path.DirectorySeparatorChar)));

    /** A list read from disk or the network cannot reach outside the game folder (VersionManifest already refuses ".."). */
    private static bool IsUnder(string root, string path)
        => path.StartsWith(Path.TrimEndingDirectorySeparator(root) + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase);

    private static bool Same(string a, string b)
        => string.Equals(Path.TrimEndingDirectorySeparator(a), Path.TrimEndingDirectorySeparator(b), StringComparison.OrdinalIgnoreCase);
}
