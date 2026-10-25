using System.Globalization;
using System.Text.RegularExpressions;

namespace HOLPublish;

/**
 * The versions folder as laid out in LauncherInProgress.md (L2) and read by HOLManager's VersionStore, plus what only the
 * publication uses: <v>/published.txt (UTC date of the publication, never served) gives the order of the versions, since
 * names need not sort (a --version given by hand); a version without it (copied by hand) counts from its manifest.json's
 * last write. <v>.tmp is a publication being written; "launcher" holds the launcher's own versions.
 */
public static partial class VersionFolder
{
    public const string Latest = "latest.txt";
    public const string Manifest = "manifest.json";
    public const string Notes = "notes.txt";
    public const string PublishedFile = "published.txt";
    public const string Files = "files";
    public const string Patches = "patches";
    public const string TmpSuffix = ".tmp";

    [GeneratedRegex("^[A-Za-z0-9._-]+$")]
    private static partial Regex NamePattern();

    /** VersionStore.IsValidName ([A-Za-z0-9._-]+, not "." nor ".."), minus the names the folder uses for itself. */
    public static bool IsPublishable(string? name) =>
        name != null && NamePattern().IsMatch(name) && name.Trim('.').Length > 0
        && !name.EndsWith(TmpSuffix, StringComparison.OrdinalIgnoreCase)
        && !name.Equals("launcher", StringComparison.OrdinalIgnoreCase)
        && !name.Equals(Latest, StringComparison.OrdinalIgnoreCase);

    /** The published versions (those with a manifest.json), newest first. */
    public static IReadOnlyList<string> Published(string versionsDir)
    {
        if (!Directory.Exists(versionsDir))
        {
            return [];
        }
        return Directory.GetDirectories(versionsDir)
            .Select(d => Path.GetFileName(d)!)
            .Where(name => IsPublishable(name) && File.Exists(Path.Combine(versionsDir, name, Manifest)))
            .Select(name => (Name: name, Date: PublishedDate(Path.Combine(versionsDir, name))))
            .OrderByDescending(v => v.Date).ThenByDescending(v => v.Name, StringComparer.Ordinal)
            .Select(v => v.Name).ToList();
    }

    /** The name in latest.txt, null if none. */
    public static string? ReadLatest(string versionsDir)
    {
        var file = Path.Combine(versionsDir, Latest);
        return File.Exists(file) ? File.ReadAllText(file).Trim() : null;
    }

    /** Written beside, then moved over latest.txt: a reader sees the old name or the new one, never half a file. */
    public static void WriteLatest(string versionsDir, string version)
    {
        var file = Path.Combine(versionsDir, Latest);
        var tmp = file + TmpSuffix;
        File.WriteAllText(tmp, version + Environment.NewLine);
        File.Move(tmp, file, overwrite: true);
    }

    public static void WritePublishedDate(string versionDir, DateTime utc) =>
        File.WriteAllText(Path.Combine(versionDir, PublishedFile), utc.ToString("O", CultureInfo.InvariantCulture));

    /** A list path ('/') under a folder, with a suffix (".gz", ".patch"). */
    public static string Under(string dir, string listPath, string suffix = "") =>
        Path.Combine(dir, listPath.Replace('/', Path.DirectorySeparatorChar) + suffix);

    private static DateTime PublishedDate(string versionDir)
    {
        var file = Path.Combine(versionDir, PublishedFile);
        if (File.Exists(file) && DateTime.TryParse(File.ReadAllText(file).Trim(), CultureInfo.InvariantCulture,
                DateTimeStyles.RoundtripKind, out var date))
        {
            return date.ToUniversalTime();
        }
        return File.GetLastWriteTimeUtc(Path.Combine(versionDir, Manifest));
    }
}
