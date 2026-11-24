using System.Globalization;

namespace HOLUpdate;

/**
 * The publication order of a versions folder (L4; shared since L21 so that it has one source): <v>/published.txt holds
 * the UTC date of the publication, never served; a version without it (copied by hand) counts from its manifest.json's
 * last write. Names need not sort (a --version given by hand, 0.3.10 after 0.3.9), so the publisher (patches, pruning)
 * and the manager (the notes history of /versions/latest, L21) both read the order here.
 */
public static class PublishedVersions
{
    public const string ManifestFile = "manifest.json";
    public const string DateFile = "published.txt";

    /** The folders of versionsDir accepted by isVersion (on the name) and holding a manifest.json, newest first. */
    public static IReadOnlyList<string> NewestFirst(string versionsDir, Func<string, bool> isVersion)
    {
        if (!Directory.Exists(versionsDir))
        {
            return [];
        }
        return Directory.GetDirectories(versionsDir)
            .Select(d => Path.GetFileName(d)!)
            .Where(name => isVersion(name) && File.Exists(Path.Combine(versionsDir, name, ManifestFile)))
            .Select(name => (Name: name, Date: Date(Path.Combine(versionsDir, name))))
            .OrderByDescending(v => v.Date).ThenByDescending(v => v.Name, StringComparer.Ordinal)
            .Select(v => v.Name).ToList();
    }

    /** When the version of that folder was published (UTC). */
    public static DateTime Date(string versionDir)
    {
        var file = Path.Combine(versionDir, DateFile);
        if (File.Exists(file) && DateTime.TryParse(File.ReadAllText(file).Trim(), CultureInfo.InvariantCulture,
                DateTimeStyles.RoundtripKind, out var date))
        {
            return date.ToUniversalTime();
        }
        return File.GetLastWriteTimeUtc(Path.Combine(versionDir, ManifestFile));
    }
}
