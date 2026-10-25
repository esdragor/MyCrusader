using System.IO.Enumeration;
using System.Security.Cryptography;

namespace HOLUpdate;

/**
 * Builds the list of a folder (L1): every file under it, recursively, hashed as a stream (the game exe weighs 332 MB),
 * minus the excluded ones. An exclusion without '/' matches the file name ("*.pdb"), one with '/' the relative path
 * ("Saved/*"); both ignore case, like Windows.
 */
public static class ManifestBuilder
{
    public const string VersionFile = "Version.txt";

    /** Symbols are never sent to players (L0: HeroesOfLiberty.pdb weighs 2.5 GB). */
    public static readonly IReadOnlyList<string> DefaultExcludes = ["*.pdb"];

    private const int BufferSize = 1 << 20;

    /** version null: read from Version.txt at the folder's root (FileNotFoundException if missing). */
    public static VersionManifest Build(string dir, string? version = null, IEnumerable<string>? excludes = null)
    {
        var root = Path.GetFullPath(dir);
        version ??= File.ReadAllText(Path.Combine(root, VersionFile)).Trim();
        var patterns = (excludes ?? DefaultExcludes).ToList();
        var entries = new List<FileEntry>();
        foreach (var file in Directory.EnumerateFiles(root, "*", SearchOption.AllDirectories))
        {
            var path = Path.GetRelativePath(root, file).Replace('\\', '/');
            if (patterns.Any(pattern => IsExcluded(pattern, path)))
            {
                continue;
            }
            entries.Add(new FileEntry(path, new FileInfo(file).Length, Hash(file)));
        }
        return new VersionManifest(version, entries);
    }

    /** SHA-256 of a file in lowercase hex, read as a stream. */
    public static string Hash(string file)
    {
        using var stream = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read, BufferSize, FileOptions.SequentialScan);
        return Convert.ToHexString(SHA256.HashData(stream)).ToLowerInvariant();
    }

    private static bool IsExcluded(string pattern, string path)
    {
        var name = pattern.Contains('/') ? path : path[(path.LastIndexOf('/') + 1)..];
        return FileSystemName.MatchesSimpleExpression(pattern, name, ignoreCase: true);
    }
}
