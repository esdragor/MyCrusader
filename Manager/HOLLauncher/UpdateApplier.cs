using HOLUpdate;

namespace HOLLauncher;

/** A file of the game is in use (the game is running): nothing was replaced. */
public sealed class GameLockedException(string message, Exception inner) : Exception(message, inner);

/** A file ready to go in place: its entry in the target list and the checked file in the work folder. */
public sealed record ReadyFile(FileEntry File, string LocalPath);

/**
 * Puts an update in place (L6b), once every file is fetched and checked. First every file to replace or delete is
 * opened for exclusive writing: one in use stops everything before any change (GameLockedException). Then each file
 * is moved to its place (folders created, old one replaced), the removed ones deleted (UpdatePlan.ToDelete: only files
 * of the previous installed.json), Version.txt last, then installed.json, then the work folder is removed. If a move
 * fails anyway, Version.txt still names the old version: the next update starts again, and the files already moved
 * are seen as unchanged by the scan.
 */
public static class UpdateApplier
{
    public static void Apply(string gameDir, UpdatePlan plan, IReadOnlyList<ReadyFile> ready)
    {
        var root = Path.GetFullPath(gameDir);
        var touched = ready.Select(r => r.File.Path).Concat(plan.ToDelete.Select(f => f.Path)).Append(ManifestBuilder.VersionFile);
        foreach (var path in touched)
        {
            CheckWritable(GamePath(root, path), path);
        }
        var versionFile = ready.Where(r => IsVersionFile(r.File.Path)).ToList();
        foreach (var file in ready.Except(versionFile).OrderBy(r => r.File.Path, StringComparer.Ordinal))
        {
            MoveInPlace(file.LocalPath, GamePath(root, file.File.Path));
        }
        foreach (var file in plan.ToDelete)
        {
            File.Delete(GamePath(root, file.Path));
        }
        foreach (var file in versionFile)
        {
            MoveInPlace(file.LocalPath, GamePath(root, file.File.Path));
        }
        var list = UpdatePlan.InstalledListPath(root);
        Directory.CreateDirectory(Path.GetDirectoryName(list)!);
        plan.InstalledList.Save(list + ".tmp");
        File.Move(list + ".tmp", list, overwrite: true);
        var work = FileDownloader.TempDirFor(root, plan.Version);
        if (Directory.Exists(work))
        {
            Directory.Delete(work, recursive: true);
        }
    }

    private static bool IsVersionFile(string path) => path.Equals(ManifestBuilder.VersionFile, StringComparison.OrdinalIgnoreCase);

    private static string GamePath(string root, string path) => Path.Combine(root, path.Replace('/', Path.DirectorySeparatorChar));

    private static void CheckWritable(string file, string path)
    {
        if (!File.Exists(file))
        {
            return;
        }
        try
        {
            using var stream = new FileStream(file, FileMode.Open, FileAccess.ReadWrite, FileShare.None);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            throw new GameLockedException($"{path} est utilisé : fermez le jeu, puis relancez la mise à jour.", e);
        }
    }

    private static void MoveInPlace(string source, string target)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        File.Move(source, target, overwrite: true);
    }
}
