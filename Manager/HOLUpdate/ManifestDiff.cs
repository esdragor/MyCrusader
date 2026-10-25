namespace HOLUpdate;

/**
 * What changes between the installed list (Empty on a first install) and the target one (L1). Paths are compared
 * without case, like Windows: "Data/A.txt" and "data/a.TXT" are the same file. A file is changed when its size or
 * hash differs. Added, Changed, ToDownload and Unchanged hold the target's entries, ToDelete the installed ones.
 */
public sealed class ManifestDiff
{
    private ManifestDiff(List<FileEntry> added, List<FileEntry> changed, List<FileEntry> toDelete, List<FileEntry> unchanged)
    {
        Added = added;
        Changed = changed;
        ToDownload = added.Concat(changed).OrderBy(f => f.Path, StringComparer.Ordinal).ToList();
        ToDelete = toDelete;
        Unchanged = unchanged;
        DownloadBytes = ToDownload.Sum(f => f.Size);
    }

    public IReadOnlyList<FileEntry> Added { get; }
    public IReadOnlyList<FileEntry> Changed { get; }

    /** Added and changed, in ordinal order. */
    public IReadOnlyList<FileEntry> ToDownload { get; }

    public IReadOnlyList<FileEntry> ToDelete { get; }
    public IReadOnlyList<FileEntry> Unchanged { get; }
    public long DownloadBytes { get; }

    public static ManifestDiff Compare(VersionManifest installed, VersionManifest target)
    {
        var before = new Dictionary<string, FileEntry>(StringComparer.OrdinalIgnoreCase);
        foreach (var file in installed.Files)
        {
            before[file.Path] = file;
        }
        var targetPaths = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        List<FileEntry> added = [], changed = [], unchanged = [];
        foreach (var file in target.Files)
        {
            targetPaths.Add(file.Path);
            if (!before.TryGetValue(file.Path, out var old))
            {
                added.Add(file);
            }
            else if (old.Size != file.Size || old.Sha256 != file.Sha256)
            {
                changed.Add(file);
            }
            else
            {
                unchanged.Add(file);
            }
        }
        var toDelete = installed.Files.Where(f => !targetPaths.Contains(f.Path)).ToList();
        return new ManifestDiff(added, changed, toDelete, unchanged);
    }
}
